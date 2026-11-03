#include "NpcActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"

namespace
{
	UStaticMesh* Mesh(const TCHAR* Path) { return LoadObject<UStaticMesh>(nullptr, Path); }
	UStaticMesh* Cube() { UStaticMesh* M = Mesh(TEXT("/Engine/BasicShapes/Cube.Cube")); return M; }
	UStaticMesh* Sphere() { UStaticMesh* M = Mesh(TEXT("/Engine/BasicShapes/Sphere.Sphere")); return M; }
	UStaticMesh* Cylinder() { UStaticMesh* M = Mesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder")); return M; }
	UMaterialInterface* BaseMat() { UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")); return M; }

	const FLinearColor BoneColor(0.85f, 0.85f, 0.8f);
	const FLinearColor MetalColor(0.3f, 0.38f, 0.46f);
	const FLinearColor BloodColor(0.18f, 0.0f, 0.0f);
}

ANpcActor::ANpcActor()
{
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Movable);
	RootComponent = Root;
}

UStaticMeshComponent* ANpcActor::AddPart(UStaticMesh* InMesh, const FVector& Loc, const FVector& Scale, const FLinearColor& Color, EPart Kind, bool bSkeleton)
{
	UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
	C->SetMobility(EComponentMobility::Movable);
	C->SetStaticMesh(InMesh);
	C->SetupAttachment(Root);
	C->SetRelativeLocation(Loc);
	C->SetRelativeScale3D(Scale);
	C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	C->SetCastShadow(true);
	C->RegisterComponent();
	AddInstanceComponent(C);

	UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(BaseMat(), C);
	M->SetVectorParameterValue(TEXT("Color"), Color);
	C->SetMaterial(0, M);

	FPart P; P.Comp = C; P.Mat = M; P.Base = Color; P.Kind = Kind;
	if (bSkeleton)
	{
		C->SetVisibility(false);
		Skeleton.Add(P);
	}
	else
	{
		Body.Add(P);
	}
	return C;
}

void ANpcActor::Init(const FNpcProfile& InProfile, bool bInGore)
{
	Profile = InProfile;
	bGore = bInGore;
	SetActorScale3D(FVector(Profile.HeightScale));

	const FLinearColor White(0.9f, 0.9f, 0.9f), Black(0.01f, 0.01f, 0.01f);

	// 몸 (정면 = +X, 왼쪽 = -Y)
	for (float Y : { -10.f, 10.f }) AddPart(Cube(), FVector(0, Y, 42), FVector(0.16f, 0.14f, 0.84f), Profile.Pants, EPart::Limb, false);
	AddPart(Cube(), FVector(0, 0, 113), FVector(0.26f, 0.46f, 0.6f), Profile.Clothes, EPart::Torso, false);
	for (float Y : { -30.f, 30.f })
	{
		AddPart(Cube(), FVector(0, Y, 111), FVector(0.13f, 0.12f, 0.6f), Profile.Clothes, EPart::Limb, false);
		AddPart(Sphere(), FVector(0, Y, 77), FVector(0.1f), Profile.Skin, EPart::Limb, false);
	}
	AddPart(Cylinder(), FVector(0, 0, 148), FVector(0.11f, 0.11f, 0.1f), Profile.Skin, EPart::Head, false);
	AddPart(Sphere(), FVector(0, 0, 165), FVector(0.24f, 0.22f, 0.28f), Profile.Skin, EPart::Head, false);
	AddPart(Sphere(), FVector(-2, 0, 172), FVector(0.25f, 0.235f, 0.22f), Profile.Hair, EPart::Hair, false);
	AddPart(Cube(), FVector(11, 0, 162), FVector(0.04f, 0.03f, 0.05f), Profile.Skin, EPart::Head, false);
	for (float Y : { -5.f, 5.f })
	{
		AddPart(Sphere(), FVector(10.5f, Y, 167), FVector(0.05f), White, EPart::Eye, false);
		Pupils.Add(AddPart(Sphere(), FVector(12.8f, Y, 167), FVector(0.025f), Black, EPart::Pupil, false));
	}

	// 엑스레이용 골격
	const bool bMetal = Profile.Xray == EXray::MetalFrame;
	const FLinearColor Bone = bMetal ? MetalColor : BoneColor;
	AddPart(Sphere(), FVector(0, 0, 165), FVector(0.2f, 0.19f, 0.24f), Bone, EPart::Head, true);
	AddPart(Cylinder(), FVector(0, 0, 113), FVector(0.05f, 0.05f, 0.62f), Bone, EPart::Torso, true);
	for (float Z : { 100.f, 113.f, 126.f }) AddPart(Cube(), FVector(0, 0, Z), FVector(0.2f, 0.36f, 0.025f), Bone, EPart::Torso, true);
	AddPart(Cube(), FVector(0, 0, 82), FVector(0.14f, 0.32f, 0.08f), Bone, EPart::Torso, true);
	for (float Y : { -10.f, 10.f }) AddPart(Cylinder(), FVector(0, Y, 42), FVector(0.045f, 0.045f, 0.84f), Bone, EPart::Limb, true);
	for (float Y : { -30.f, 30.f })
	{
		const bool bProstheticArm = Profile.Xray == EXray::Prosthetic && Y < 0.f; // 왼팔
		AddPart(Cylinder(), FVector(0, Y, 111), FVector(0.04f, 0.04f, 0.6f), bProstheticArm ? MetalColor : Bone, EPart::Limb, true);
	}
}

FLinearColor ANpcActor::ThermalColor(EPart Kind) const
{
	const bool bHead = Kind == EPart::Head || Kind == EPart::Hair || Kind == EPart::Eye;
	switch (Profile.Thermal)
	{
	case EThermal::Flat:
		return FLinearColor(1.f, 0.45f, 0.03f); // 온몸이 똑같은 온도 → 기계적
	case EThermal::Fever:
		if (bHead) return FLinearColor(1.f, 0.02f, 0.02f);
		return Kind == EPart::Torso ? FLinearColor(1.f, 0.12f, 0.03f) : FLinearColor(1.f, 0.45f, 0.03f);
	default:
		if (bHead) return FLinearColor(1.f, 0.2f, 0.03f);
		return Kind == EPart::Torso ? FLinearColor(1.f, 0.55f, 0.03f) : FLinearColor(0.35f, 0.8f, 0.1f);
	}
}

void ANpcActor::SetViewMode(EViewMode M)
{
	if (bDead) M = EViewMode::Normal;
	Mode = M;
	for (FPart& P : Body)
	{
		P.Comp->SetVisibility(M != EViewMode::XRay);
		FLinearColor C = P.Base;
		if (M == EViewMode::Thermal && P.Kind != EPart::Pupil) C = ThermalColor(P.Kind);
		P.Mat->SetVectorParameterValue(TEXT("Color"), C);
	}
	for (FPart& P : Skeleton) P.Comp->SetVisibility(M == EViewMode::XRay);
}

void ANpcActor::WalkPath(const TArray<FVector>& Points, float Speed, float EndYaw)
{
	Path = Points;
	PathIndex = 0;
	WalkSpeed = Speed;
	FinalYaw = EndYaw;
}

void ANpcActor::PupilReact(int32 DelayMs)
{
	PupilDelay = DelayMs / 1000.f;
	PupilTimer = 0.f;
}

FVector ANpcActor::GetHeadWorld() const
{
	return GetActorLocation() + FVector(0, 0, 165.f * Profile.HeightScale);
}

void ANpcActor::Tick(float Dt)
{
	Super::Tick(Dt);

	if (!bDead && PathIndex < Path.Num())
	{
		const FVector Pos = GetActorLocation();
		FVector To = Path[PathIndex] - Pos;
		To.Z = 0.f;
		const float Dist = To.Size();
		const float Step = WalkSpeed * Dt;
		if (Dist <= Step)
		{
			SetActorLocation(FVector(Path[PathIndex].X, Path[PathIndex].Y, Pos.Z));
			PathIndex++;
			if (PathIndex >= Path.Num()) SetActorRotation(FRotator(0.f, FinalYaw, 0.f));
		}
		else
		{
			const FVector Dir = To / Dist;
			SetActorLocation(Pos + Dir * Step);
			const FRotator Want(0.f, Dir.Rotation().Yaw, 0.f);
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), Want, Dt, 8.f));
		}
	}

	if (PupilTimer >= 0.f)
	{
		PupilTimer += Dt;
		float K = 1.f;
		if (PupilTimer >= PupilDelay)
		{
			const float T = PupilTimer - PupilDelay;
			K = T < 0.08f ? FMath::Lerp(1.f, 0.45f, T / 0.08f) : 0.45f;
		}
		if (PupilTimer > PupilDelay + 1.8f) { K = 1.f; PupilTimer = -1.f; }
		for (UStaticMeshComponent* P : Pupils) P->SetRelativeScale3D(FVector(0.025f * K));
	}
}

void ANpcActor::Kill(const FVector& ShotDir)
{
	if (bDead) return;
	SetViewMode(EViewMode::Normal);
	bDead = true;
	Path.Reset();

	const FVector Chest = GetActorLocation() + FVector(0, 0, 113.f * Profile.HeightScale);
	for (FPart& P : Body)
	{
		if (P.Kind == EPart::Eye || P.Kind == EPart::Pupil) { P.Comp->SetVisibility(false); continue; }
		P.Comp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		P.Comp->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
		P.Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		P.Comp->SetSimulatePhysics(true);
		const FVector Impulse = ShotDir * FMath::FRandRange(180.f, 380.f) + FVector(0, 0, FMath::FRandRange(40.f, 140.f));
		P.Comp->AddImpulse(Impulse, NAME_None, true);
	}

	if (bGore) SpawnBlood(Chest, ShotDir);
}

void ANpcActor::SpawnBlood(const FVector& Origin, const FVector& Dir)
{
	UWorld* W = GetWorld();
	if (!W) return;

	auto SpawnPool = [&](const FVector& At, float Size)
	{
		AStaticMeshActor* A = W->SpawnActor<AStaticMeshActor>(FVector(At.X, At.Y, 1.2f), FRotator::ZeroRotator);
		if (!A) return;
		UStaticMeshComponent* C = A->GetStaticMeshComponent();
		C->SetMobility(EComponentMobility::Movable);
		C->SetStaticMesh(Cylinder());
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->SetCastShadow(false);
		C->SetWorldScale3D(FVector(Size, Size * FMath::FRandRange(0.7f, 1.2f), 0.004f));
		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(BaseMat(), C);
		M->SetVectorParameterValue(TEXT("Color"), BloodColor);
		C->SetMaterial(0, M);
		A->Tags.Add(TEXT("Blood"));
	};

	SpawnPool(Origin + Dir * 60.f, FMath::FRandRange(1.0f, 1.5f));
	for (int32 i = 0; i < 6; ++i)
	{
		const FVector Off = Dir * FMath::FRandRange(80.f, 260.f) + FVector(0, FMath::FRandRange(-90.f, 90.f), 0);
		SpawnPool(Origin + Off, FMath::FRandRange(0.08f, 0.3f));
	}
}
