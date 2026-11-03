#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PangaeaTypes.h"
#include "NpcActor.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

// 생존자 1명의 외형과 연출. 지금은 기본 도형으로 만든 임시 몸이고, 나중에 메타휴먼으로 교체한다.
UCLASS()
class PANGAEA_API ANpcActor : public AActor
{
	GENERATED_BODY()

public:
	ANpcActor();

	void Init(const FNpcProfile& InProfile, bool bInGore);
	void SetViewMode(EViewMode M);
	EViewMode GetViewMode() const { return Mode; }

	// 경로를 따라 걷는다. 끝에서 EndYaw 방향을 바라본다.
	void WalkPath(const TArray<FVector>& Points, float Speed, float EndYaw);
	bool IsWalking() const { return PathIndex < Path.Num(); }

	void PupilReact(int32 DelayMs);
	void Kill(const FVector& ShotDir);
	bool IsDead() const { return bDead; }
	FVector GetHeadWorld() const;

	virtual void Tick(float DeltaSeconds) override;

private:
	enum class EPart : uint8 { Head, Torso, Limb, Eye, Hair, Pupil };

	struct FPart
	{
		UStaticMeshComponent* Comp = nullptr;
		UMaterialInstanceDynamic* Mat = nullptr;
		FLinearColor Base;
		EPart Kind = EPart::Torso;
	};

	UPROPERTY() TObjectPtr<USceneComponent> Root;

	TArray<FPart> Body;
	TArray<FPart> Skeleton;
	TArray<UStaticMeshComponent*> Pupils;
	FNpcProfile Profile;
	EViewMode Mode = EViewMode::Normal;
	bool bGore = true;
	bool bDead = false;

	TArray<FVector> Path;
	int32 PathIndex = 0;
	float WalkSpeed = 160.f;
	float FinalYaw = 180.f;
	float BobTime = 0.f;

	float PupilTimer = -1.f;
	float PupilDelay = 0.2f;

	UStaticMeshComponent* AddPart(UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale, const FLinearColor& Color, EPart Kind, bool bSkeleton);
	void SpawnBlood(const FVector& Origin, const FVector& Dir);
	FLinearColor ThermalColor(EPart Kind) const;
};
