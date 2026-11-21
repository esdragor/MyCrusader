// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLMoraleProcessor.h"

#include "Engine/World.h"
#include "Match/HOLMatchConfig.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLMoraleRules.h"
#include "Simulation/HOLMoraleTableRows.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLUnitTypes.h"

namespace
{
	// A source of an aura: where it stands, whose it is, how far it reaches and what it lays. Two kinds: a living hero (row DawnAura of Effects.csv,
	// SourceKind Hero) and a banner planted by a hero (row Banner, SourceKind Point: a place, not a unit, it stands even once its hero is dead; the row
	// gives the levers of the DawnAura row under that row's id, so a unit under both carries one effect).
	struct FAuraSource
	{
		FVector2D Location = FVector2D::ZeroVector;
		uint8 Owner = 0;
		double RadiusSquared = 0.0;
		HOLMorale::FHOLMoraleEffect Effect;
	};
}

UHOLMoraleProcessor::UHOLMoraleProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	// Before the combat of the step: the combat reads the levers as the time of this step has left them.
	ExecutionOrder.ExecuteBefore.Add(TEXT("HOLCombatProcessor"));
}

void UHOLMoraleProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FHOLMoraleFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHOLUnitStateFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FHOLUnitTypeConstants>(EMassFragmentPresence::All);
}

void UHOLMoraleProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// The rules of this step, integers only. First pass: the auras are taken off (HOLMorale::ClearAuras), the time of the step passes on every
	// unit's effects (HOLMorale::Tick), and the living heroes are noted (Dawn aura sources). Then the banners standing are noted (the hero subsystem:
	// a planted point per slot, 45 s). Second pass: each source's effect is laid again on the units of its own camp within its radius (D705: a bonus
	// goes to the owner alone; military and heroes, never a Peasant). The outcome does not depend on the chunks: a unit is covered or not by where
	// the sources stand, whatever the order it is visited in; the combat reads the fragment alone (one source of truth).
	const int32 StepMs = HOLCombat::ToMilliseconds(Context.GetDeltaTimeSeconds());
	TArray<FAuraSource> Sources;
	const HOLMoraleTableRows::FHOLMoraleTableRow* DawnRow = HOLMoraleTableRows::Find(HOLMoraleTableRows::HOLMoraleDawnAura);
	const HOLMoraleTableRows::FHOLMoraleTableRow* BannerRow = HOLMoraleTableRows::Find(HOLMoraleTableRows::HOLMoraleBanner);
	const HOLMorale::FHOLMoraleEffect Dawn = HOLMorale::MakeEffectFromRow(HOLMoraleTableRows::HOLMoraleDawnAura);
	const HOLMorale::FHOLMoraleEffect Banner = HOLMorale::MakeEffectFromRow(HOLMoraleTableRows::HOLMoraleBanner);
	const double DawnRadiusSquared = DawnRow ? static_cast<double>(DawnRow->RadiusCm) * static_cast<double>(DawnRow->RadiusCm) : 0.0;
	EntityQuery.ForEachEntityChunk(Context, [StepMs, DawnRow, DawnRadiusSquared, &Dawn, &Sources](FMassExecutionContext& Chunk)
	{
		const TArrayView<FHOLMoraleFragment> Fragments = Chunk.GetMutableFragmentView<FHOLMoraleFragment>();
		const TConstArrayView<FHOLUnitStateFragment> States = Chunk.GetFragmentView<FHOLUnitStateFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Chunk.GetFragmentView<FTransformFragment>();
		const FHOLUnitTypeConstants& Constants = Chunk.GetConstSharedFragment<FHOLUnitTypeConstants>();
		const bool bHero = HOLIsHero(Constants.ArmorCategoryMask);
		for (int32 Index = 0; Index < Fragments.Num(); ++Index)
		{
			HOLMorale::ClearAuras(Fragments[Index].State);
			HOLMorale::Tick(Fragments[Index].State, StepMs);
			if (DawnRow != nullptr && bHero && States[Index].CurrentHealth > 0.f)
			{
				const FVector Location = Transforms[Index].GetTransform().GetLocation();
				Sources.Add({ FVector2D(Location.X, Location.Y), States[Index].OwnerPlayer, DawnRadiusSquared, Dawn });
			}
		}
	});
	// The banners standing, even once their hero is dead (J8c-Q3).
	const UWorld* World = GetWorld();
	const UHOLHeroSubsystem* Heroes = World ? World->GetSubsystem<UHOLHeroSubsystem>() : nullptr;
	if (Heroes != nullptr && BannerRow != nullptr)
	{
		const double BannerRadiusSquared = static_cast<double>(BannerRow->RadiusCm) * static_cast<double>(BannerRow->RadiusCm);
		for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
		{
			FVector BannerLocation;
			if (Heroes->GetBanner(Slot, BannerLocation))
			{
				Sources.Add({ FVector2D(BannerLocation.X, BannerLocation.Y), static_cast<uint8>(Slot), BannerRadiusSquared, Banner });
			}
		}
	}
	if (Sources.IsEmpty())
	{
		return;
	}

	const int32 PeasantBit = HOLArmorCategoryBit(EHOLArmorCategory::Peasant);
	EntityQuery.ForEachEntityChunk(Context, [&Sources, PeasantBit](FMassExecutionContext& Chunk)
	{
		const FHOLUnitTypeConstants& Constants = Chunk.GetConstSharedFragment<FHOLUnitTypeConstants>();
		if ((Constants.ArmorCategoryMask & PeasantBit) != 0)
		{
			return; // never a Peasant (D705)
		}
		const TArrayView<FHOLMoraleFragment> Fragments = Chunk.GetMutableFragmentView<FHOLMoraleFragment>();
		const TConstArrayView<FHOLUnitStateFragment> States = Chunk.GetFragmentView<FHOLUnitStateFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Chunk.GetFragmentView<FTransformFragment>();
		for (int32 Index = 0; Index < Fragments.Num(); ++Index)
		{
			const FVector Location = Transforms[Index].GetTransform().GetLocation();
			const FVector2D Where(Location.X, Location.Y);
			for (const FAuraSource& Source : Sources)
			{
				if (Source.Owner == States[Index].OwnerPlayer && FVector2D::DistSquared(Source.Location, Where) <= Source.RadiusSquared)
				{
					HOLMorale::Apply(Fragments[Index].State, Source.Effect); // the same effect id from several sources is refreshed, never cumulated
				}
			}
		}
	});
}
