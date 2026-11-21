// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLMoraleProcessor.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLMoraleRules.h"
#include "Simulation/HOLMoraleTableRows.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLUnitTypes.h"

namespace
{
	// A living hero: where it stands and whose it is. The source of the Dawn aura (row DawnAura of Effects.csv, SourceKind Hero).
	struct FAuraSource
	{
		FVector2D Location = FVector2D::ZeroVector;
		uint8 Owner = 0;
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
	// unit's effects (HOLMorale::Tick), and the living heroes are noted. Second pass: the Dawn aura is laid again on the units of a hero's own
	// camp within the row's radius (D705: a bonus goes to the owner alone; military and heroes, never a Peasant). The outcome does not depend
	// on the chunks: a unit is covered or not by where the heroes stand, whatever the order it is visited in.
	const int32 StepMs = HOLCombat::ToMilliseconds(Context.GetDeltaTimeSeconds());
	TArray<FAuraSource> Sources;
	EntityQuery.ForEachEntityChunk(Context, [StepMs, &Sources](FMassExecutionContext& Chunk)
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
			if (bHero && States[Index].CurrentHealth > 0.f)
			{
				const FVector Location = Transforms[Index].GetTransform().GetLocation();
				Sources.Add({ FVector2D(Location.X, Location.Y), States[Index].OwnerPlayer });
			}
		}
	});
	if (Sources.IsEmpty())
	{
		return;
	}

	const HOLMoraleTableRows::FHOLMoraleTableRow* Row = HOLMoraleTableRows::Find(HOLMoraleTableRows::HOLMoraleDawnAura);
	if (Row == nullptr)
	{
		return;
	}
	const double RadiusSquared = static_cast<double>(Row->RadiusCm) * static_cast<double>(Row->RadiusCm);
	const HOLMorale::FHOLMoraleEffect Dawn = HOLMorale::MakeEffectFromRow(HOLMoraleTableRows::HOLMoraleDawnAura);
	const int32 PeasantBit = HOLArmorCategoryBit(EHOLArmorCategory::Peasant);
	EntityQuery.ForEachEntityChunk(Context, [&Sources, &Dawn, RadiusSquared, PeasantBit](FMassExecutionContext& Chunk)
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
				if (Source.Owner == States[Index].OwnerPlayer && FVector2D::DistSquared(Source.Location, Where) <= RadiusSquared)
				{
					HOLMorale::Apply(Fragments[Index].State, Dawn); // once however many heroes cover it (not cumulated)
					break;
				}
			}
		}
	});
}
