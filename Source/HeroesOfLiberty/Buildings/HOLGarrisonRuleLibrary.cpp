// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLGarrisonRuleLibrary.h"

#include "Buildings/HOLGarrisonRules.h"

namespace UE::HeroesOfLiberty::GarrisonRuleLibrary
{
	/** The script's kind numbers; one outside 0..4 becomes a kind that never enters (the script's KindMayEnter answers false). */
	TArray<HOLGarrison::EKind> ToKinds(const TArray<int32>& UnitKinds)
	{
		TArray<HOLGarrison::EKind> Kinds;
		Kinds.Reserve(UnitKinds.Num());
		for (const int32 Kind : UnitKinds)
		{
			Kinds.Add(Kind >= 0 && Kind <= static_cast<int32>(HOLGarrison::EKind::Hero) ? static_cast<HOLGarrison::EKind>(Kind) : HOLGarrison::EKind::Siege);
		}
		return Kinds;
	}
}

TArray<int32> UHOLGarrisonRuleLibrary::AdmittedWith(const TArray<int32>& UnitSlots, const TArray<int32>& UnitKinds, const int32 BuildingSlot,
	const int32 Slots, const int32 Occupants, const bool bMeleeInfantry, const bool bHero, const bool bCavalry)
{
	return HOLGarrison::Admitted(UnitSlots, UE::HeroesOfLiberty::GarrisonRuleLibrary::ToKinds(UnitKinds), BuildingSlot, Slots, Occupants,
		bMeleeInfantry, bHero, bCavalry);
}

TArray<int32> UHOLGarrisonRuleLibrary::AdmittedBySettings(const TArray<int32>& UnitSlots, const TArray<int32>& UnitKinds, const int32 BuildingSlot,
	const int32 Slots, const int32 Occupants)
{
	return HOLGarrison::AdmittedBySettings(UnitSlots, UE::HeroesOfLiberty::GarrisonRuleLibrary::ToKinds(UnitKinds), BuildingSlot, Slots, Occupants);
}
