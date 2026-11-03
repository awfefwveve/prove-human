#pragma once

#include "CoreMinimal.h"
#include "PangaeaTypes.h"

namespace PangaeaRules
{
	constexpr int32 LastDay = 30;
	FDayRule Get(int32 Day, int32 RunSeed);
	int32 FacilityCost(EFacility F);
	int32 FacilityUnlockDay(EFacility F);
}

namespace PangaeaGen
{
	TArray<FNpcProfile> Generate(const FDayRule& Rule, int32 Seed, bool bKennel);
}
