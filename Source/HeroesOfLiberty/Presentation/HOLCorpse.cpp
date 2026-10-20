// Copyright Epic Games, Inc. All Rights Reserved.

#include "Presentation/HOLCorpse.h"

#include "Engine/World.h"
#include "Misc/ScopeLock.h"
#include "UObject/ObjectKey.h"
#include "MassCommonFragments.h"
#include "MassEntityManager.h"
#include "MassEntityTemplate.h"
#include "MassEntityUtils.h"
#include "MassRepresentationFragments.h"
#include "Net/HOLNetUnitDisplay.h"
#include "Presentation/HOLUnitAnimation.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

float HOLCorpse::GetResumePosition(const float StartPosition, const double ElapsedSeconds, const float PlayRate, const float Length)
{
	const double Position = static_cast<double>(StartPosition) + FMath::Max(ElapsedSeconds, 0.) * PlayRate;
	return static_cast<float>(FMath::Max(FMath::Min(Position, static_cast<double>(Length)), 0.));
}

float HOLCorpse::GetSinkOffset(const double Age)
{
	return SinkDepth * static_cast<float>(FMath::Clamp((Age - HoldSeconds) / SinkSeconds, 0., 1.));
}

bool HOLCorpse::IsExpired(const double Age)
{
	return Age >= static_cast<double>(HoldSeconds + SinkSeconds);
}

namespace
{
	/** The bodies that went, per world (a new world starts at none); guarded, the processor may run off the game thread. */
	TMap<TObjectKey<UWorld>, HOLCorpse::FRemovedStats> RemovedStatsByWorld;
	FCriticalSection RemovedStatsLock;
}

void HOLCorpse::NoteRemoved(const UWorld& World, const double Age)
{
	FScopeLock Lock(&RemovedStatsLock);
	FRemovedStats& Stats = RemovedStatsByWorld.FindOrAdd(TObjectKey<UWorld>(&World));
	++Stats.NumRemoved;
	Stats.LongestKept = FMath::Max(Stats.LongestKept, Age);
}

HOLCorpse::FRemovedStats HOLCorpse::GetRemovedStats(const UWorld& World)
{
	FScopeLock Lock(&RemovedStatsLock);
	const FRemovedStats* Stats = RemovedStatsByWorld.Find(TObjectKey<UWorld>(&World));
	return Stats ? *Stats : FRemovedStats();
}

bool HOLCorpse::LeavesCorpse(const FHOLUnitTypeConstants& Type, const EHOLUnitActivity Activity, const int32 CurrentHealth)
{
	return (Activity == EHOLUnitActivity::Dying || CurrentHealth <= 0) && Type.FallSequence != INDEX_NONE;
}

void HOLCorpse::CollectSeeds(const FMassEntityManager& EntityManager, const TConstArrayView<FMassEntityHandle> Entities, const double Now,
	TArray<FHOLCorpseSeed>& Out)
{
	const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
	for (const FMassEntityHandle Entity : Entities)
	{
		if (!EntityManager.IsEntityValid(Entity))
		{
			continue;
		}
		const FHOLUnitTypeConstants* Type = EntityManager.GetConstSharedFragmentDataPtr<FHOLUnitTypeConstants>(Entity);
		const FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity);
		const FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);
		if (Type == nullptr || State == nullptr || Transform == nullptr)
		{
			continue;
		}
		// A client's display entity holds the received activity (Dying at 0 health, UHOLUnitReplicationSubsystem), not the
		// health: its CurrentHealth is never set, so only its activity counts.
		const bool bDisplayEntity = EntityManager.GetFragmentDataPtr<FHOLNetUnitDisplayFragment>(Entity) != nullptr;
		if (!LeavesCorpse(*Type, State->Activity, bDisplayEntity ? 1 : State->CurrentHealth))
		{
			continue;
		}

		const FHOLUnitAnimationStateFragment* AnimationState = EntityManager.GetFragmentDataPtr<FHOLUnitAnimationStateFragment>(Entity);
		const FMassRepresentationFragment* Representation = EntityManager.GetFragmentDataPtr<FMassRepresentationFragment>(Entity);
		const bool bFalling = AnimationState && AnimationState->bFalling && AnimationState->DrawnFrame != 0 && Representation;

		// The fall as Choose plays it for Dying (its play rate, its prop: the Peasant's knife).
		FHOLUnitAnimationInput Input;
		Input.Activity = EHOLUnitActivity::Dying;
		const FHOLUnitAnimationChoice Choice = HOLUnitAnimation::Choose(*Type, Input);

		FHOLCorpseSeed& Seed = Out.AddDefaulted_GetRef();
		Seed.TypeName = Type->TypeName;
		FHOLCorpseFragment& Corpse = Seed.Corpse;
		// As it was last drawn (its fall's yaw and its mesh's scale), else (never drawn falling: off screen) its place at its
		// mesh's scale, as UHOLInterpolatedInstancesProcessor draws it.
		const FTransform& Place = Transform->GetTransform();
		Corpse.Base = bFalling ? Representation->PrevTransform
			: FTransform(Place.GetRotation(), Place.GetLocation(), Place.GetScale3D() * Type->MeshScale);
		Corpse.LaidAt = Now;
		Corpse.Sequence = Type->FallSequence;
		Corpse.PlayRate = Choice.PlayRate;
		Corpse.Length = Type->SequenceLengths.IsValidIndex(Corpse.Sequence) ? Type->SequenceLengths[Corpse.Sequence] : 0.f;
		Corpse.StartPosition = bFalling
			? GetResumePosition(0.f, Now - AnimationState->FallStartSeconds, Corpse.PlayRate, Corpse.Length)
			: FMath::Min(Corpse.Length, HOLCombat::DyingSteps * StepSeconds * Corpse.PlayRate);
		Corpse.OwnerPlayer = State->OwnerPlayer;
		Corpse.Prop = bFalling ? AnimationState->Prop : Choice.Prop;
		Corpse.TypeName = Type->TypeName;
	}
}

void HOLCorpse::Lay(FMassEntityManager& EntityManager, UHOLUnitSubsystem& Units, const TConstArrayView<FHOLCorpseSeed> Seeds)
{
	TMap<FName, TArray<int32>> SeedsByType;
	for (int32 Index = 0; Index < Seeds.Num(); ++Index)
	{
		SeedsByType.FindOrAdd(Seeds[Index].TypeName).Add(Index);
	}
	for (const TPair<FName, TArray<int32>>& TypeSeeds : SeedsByType)
	{
		const FMassEntityTemplate* Template = Units.FindTemplate(TypeSeeds.Key, EHOLUnitTemplateKind::Corpse);
		if (Template == nullptr)
		{
			continue;
		}

		TArray<FMassEntityHandle> Created;
		{
			// Values the traits pose at build time, as the Mass spawner does; the observers run when the creation context dies.
			TSharedRef<FMassEntityManager::FEntityCreationContext> CreationContext =
				EntityManager.BatchCreateEntities(Template->GetArchetype(), Template->GetSharedFragmentValues(), TypeSeeds.Value.Num(), Created);
			TArray<FMassArchetypeEntityCollection> Collections;
			UE::Mass::Utils::CreateEntityCollections(EntityManager, Created, FMassArchetypeEntityCollection::NoDuplicates, Collections);
			EntityManager.BatchSetEntityFragmentValues(Collections, Template->GetInitialFragmentValues());
		}

		for (int32 Index = 0; Index < Created.Num(); ++Index)
		{
			const FHOLCorpseSeed& Seed = Seeds[TypeSeeds.Value[Index]];
			const FMassEntityHandle Entity = Created[Index];
			EntityManager.GetFragmentDataChecked<FHOLCorpseFragment>(Entity) = Seed.Corpse;
			EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity).SetTransform(Seed.Corpse.Base);
			// Its first frame drawn where it lies: no motion vector from the origin.
			if (FMassRepresentationFragment* Representation = EntityManager.GetFragmentDataPtr<FMassRepresentationFragment>(Entity))
			{
				Representation->PrevTransform = Seed.Corpse.Base;
			}
		}
	}
}
