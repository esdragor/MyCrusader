// Copyright Epic Games, Inc. All Rights Reserved.
#include "Simulation/HOLMoraleProcessor.h"

#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Misc/ScopeLock.h"
#include "Match/HOLFactionRules.h"
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

namespace
{
	// The duel's morale orders waiting for the next step (S2-b): written by the script bridge, drained by Execute (one lock, no per-entity cost).
	struct FPendingAround
	{
		TWeakObjectPtr<const UWorld> World;
		int32 EffectId = -1;
		int32 CenterXCm = 0;
		int32 CenterYCm = 0;
		int32 Slot = 0;
		int32 DurationMs = 0;
		int32 RadiusCm = 0;
	};
	// A clearing order (S2-c, step 8, D404): the malus of the camp's units within the radius are taken off and the immunity granted (HOLMorale::ClearMalusAndProtect).
		struct FPendingClear
		{
			TWeakObjectPtr<const UWorld> World;
			int32 CenterXCm = 0;
			int32 CenterYCm = 0;
			int32 RadiusCm = 0;
			int32 Slot = 0;
		};
FCriticalSection PendingAroundLock;
	TArray<FPendingAround> PendingAround;
		TArray<FPendingClear> PendingClear; // under PendingAroundLock too

	// PROVISIONAL (step 7, S2-c): the attenuation percent each camp gives (stand-in for the Luminous Monk's aura, A3). Only the rule HOLMorale::Apply
	// decides what it does to an effect; this table is just the value handed to it.
	int32 SlotAttenuationPercent[FHOLMatchConfig::MaxSlots] = {};

	FAutoConsoleCommand AttenuateCommand(
		TEXT("hol.Morale.Attenuate"),
		TEXT("hol.Morale.Attenuate <slot> <percent> : the camp <slot> takes every morale malus attenuated by <percent> % (75 at most, 0 = none). Stand-in for the Luminous Monk's aura (D125, D706)."),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() >= 2)
			{
				UHOLMoraleProcessor::SetSlotAttenuation(FCString::Atoi(*Args[0]), FCString::Atoi(*Args[1]));
			}
		}));
}

void UHOLMoraleProcessor::QueueEffectAround(const UWorld* World, const int32 EffectId, const int32 CenterXCm, const int32 CenterYCm, const int32 OrderSlot, const int32 DurationMs, const int32 RadiusCm)
{
	if (World == nullptr)
	{
		return;
	}
	FScopeLock Lock(&PendingAroundLock);
	FPendingAround& Order = PendingAround.AddDefaulted_GetRef();
	Order.World = World;
	Order.EffectId = EffectId;
	Order.CenterXCm = CenterXCm;
	Order.CenterYCm = CenterYCm;
	Order.Slot = OrderSlot;
	Order.DurationMs = DurationMs;
	Order.RadiusCm = RadiusCm;
}

void UHOLMoraleProcessor::QueueClearMalusAround(const UWorld* World, const int32 CenterXCm, const int32 CenterYCm, const int32 RadiusCm, const int32 OrderSlot)
{
	if (World == nullptr || RadiusCm <= 0 || OrderSlot < 0 || OrderSlot >= FHOLMatchConfig::MaxSlots)
	{
		return;
	}
	FScopeLock Lock(&PendingAroundLock);
	FPendingClear& Order = PendingClear.AddDefaulted_GetRef();
	Order.World = World;
	Order.CenterXCm = CenterXCm;
	Order.CenterYCm = CenterYCm;
	Order.RadiusCm = RadiusCm;
	Order.Slot = OrderSlot;
}
void UHOLMoraleProcessor::SetSlotAttenuation(const int32 Slot, const int32 Percent)
{
	if (Slot >= 0 && Slot < FHOLMatchConfig::MaxSlots)
	{
		SlotAttenuationPercent[Slot] = FMath::Clamp(Percent, 0, 100);
	}
}

int32 UHOLMoraleProcessor::GetSlotAttenuation(const int32 Slot)
{
	return (Slot >= 0 && Slot < FHOLMatchConfig::MaxSlots) ? SlotAttenuationPercent[Slot] : 0;
}

int32 UHOLMoraleProcessor::NumPendingOrders()
{
	FScopeLock Lock(&PendingAroundLock);
	return PendingAround.Num() + PendingClear.Num();
}

void UHOLMoraleProcessor::ResetForWorld(const UWorld* World)
{
	// STUB (red test first, REVUE-1123pm G1): does nothing yet.
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
	TArray<FAuraSource> HeroPlaces; // where each living hero stands: the cry is laid from there (one source with the Dawn aura's, kept apart)
	// D880: the passive Dawn aura is laid by the heroes of a player of the DAWN faction only (the faction of the PLAYER, not the unit type). The cry
	// and the banner are not concerned (HeroPlaces keeps every hero). The faction is read once per slot per step here, never per entity.
	const UWorld* World = GetWorld();
	bool bSlotLaysDawnAura[FHOLMatchConfig::MaxSlots];
	for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		bSlotLaysDawnAura[Slot] = HOLFactionRules::GetSlotFactionKey(World, Slot) == FName(TEXT("Dawn"));
	}
	EntityQuery.ForEachEntityChunk(Context, [StepMs, DawnRow, DawnRadiusSquared, &Dawn, &Sources, &HeroPlaces, &bSlotLaysDawnAura](FMassExecutionContext& Chunk)
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
				HeroPlaces.Add({ FVector2D(Location.X, Location.Y), States[Index].OwnerPlayer, 0.0, Dawn });
			}
			if (DawnRow != nullptr && bHero && States[Index].CurrentHealth > 0.f
				&& (States[Index].OwnerPlayer >= FHOLMatchConfig::MaxSlots || bSlotLaysDawnAura[States[Index].OwnerPlayer])) // a slot out of the table reads as the Dawn's, like GetSlotFactionKey
			{
				const FVector Location = Transforms[Index].GetTransform().GetLocation();
				Sources.Add({ FVector2D(Location.X, Location.Y), States[Index].OwnerPlayer, DawnRadiusSquared, Dawn });
			}
		}
	});
	// The banners standing, even once their hero is dead (J8c-Q3).
	UHOLHeroSubsystem* Heroes = World ? World->GetSubsystem<UHOLHeroSubsystem>() : nullptr;
	// The cries cast since the last step (J8c-Q2; the cry is consumed here, the combat no longer reads it): from each crying hero's place, the row CloseRanks
	// effect (a timed effect: it stays its whole duration, then the Tick takes it off) is laid on the units of his camp within the row's radius.
	const HOLMoraleTableRows::FHOLMoraleTableRow* CryRow = HOLMoraleTableRows::Find(HOLMoraleTableRows::HOLMoraleCloseRanks);
	if (Heroes != nullptr && CryRow != nullptr)
	{
		const HOLMorale::FHOLMoraleEffect Cry = HOLMorale::MakeEffectFromRow(HOLMoraleTableRows::HOLMoraleCloseRanks);
		const double CryRadiusSquared = static_cast<double>(CryRow->RadiusCm) * static_cast<double>(CryRow->RadiusCm);
		for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
		{
			if (Heroes->ConsumeCloseRanks(Slot))
			{
				for (const FAuraSource& Place : HeroPlaces)
				{
					if (Place.Owner == Slot)
					{
						Sources.Add({ Place.Location, static_cast<uint8>(Slot), CryRadiusSquared, Cry });
					}
				}
			}
		}
	}
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
	// The duel's orders waiting (S2-b): each is one source of its camp at its place, laid once, a timed effect (Triumph, Demoralised, Hesitation).
	{
		TArray<FPendingAround> Orders;
		{
			FScopeLock Lock(&PendingAroundLock);
			for (int32 Index = PendingAround.Num() - 1; Index >= 0; --Index)
			{
				if (!PendingAround[Index].World.IsValid())
				{
					PendingAround.RemoveAt(Index); // its world is gone
				}
				else if (PendingAround[Index].World.Get() == World)
				{
					Orders.Insert(PendingAround[Index], 0); // keeps the order they came in
					PendingAround.RemoveAt(Index);
				}
			}
		}
		for (const FPendingAround& Order : Orders)
		{
			const HOLMoraleTableRows::FHOLMoraleTableRow* Row = HOLMoraleTableRows::Find(Order.EffectId);
			if (Row == nullptr || Order.Slot < 0 || Order.Slot >= FHOLMatchConfig::MaxSlots)
			{
				continue;
			}
			HOLMorale::FHOLMoraleEffect Effect = HOLMorale::MakeEffectFromRow(Order.EffectId);
			Effect.DurationMs = Order.DurationMs > 0 ? Order.DurationMs : Effect.DurationMs;
			const double Radius = static_cast<double>(Order.RadiusCm > 0 ? Order.RadiusCm : Row->RadiusCm);
			Sources.Add({ FVector2D(Order.CenterXCm, Order.CenterYCm), static_cast<uint8>(Order.Slot), Radius * Radius, Effect });
		}
	}
	// The clearing orders waiting (S2-c, step 8): taken here, applied in the second pass BEFORE the sources, so a malus laid in the same step meets the immunity.
	TArray<FPendingClear> Clears;
	{
		FScopeLock Lock(&PendingAroundLock);
		for (int32 Index = PendingClear.Num() - 1; Index >= 0; --Index)
		{
			if (!PendingClear[Index].World.IsValid())
			{
				PendingClear.RemoveAt(Index); // its world is gone
			}
			else if (PendingClear[Index].World.Get() == World)
			{
				Clears.Insert(PendingClear[Index], 0); // keeps the order they came in
				PendingClear.RemoveAt(Index);
			}
		}
	}
if (Sources.IsEmpty() && Clears.IsEmpty())
	{
		return;
	}

	const int32 PeasantBit = HOLArmorCategoryBit(EHOLArmorCategory::Peasant);
	EntityQuery.ForEachEntityChunk(Context, [&Sources, &Clears, PeasantBit](FMassExecutionContext& Chunk)
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
				// Lumiere sacree (D404): the rule decides what is cleared and for how long the unit is immune; here only who is reached (camp and radius).
				for (const FPendingClear& Clear : Clears)
				{
					if (Clear.Slot == States[Index].OwnerPlayer
						&& FVector2D::DistSquared(FVector2D(Clear.CenterXCm, Clear.CenterYCm), Where) <= static_cast<double>(Clear.RadiusCm) * static_cast<double>(Clear.RadiusCm))
					{
						HOLMorale::ClearMalusAndProtect(Fragments[Index].State);
					}
				}
			for (const FAuraSource& Source : Sources)
			{
				if (Source.Owner == States[Index].OwnerPlayer && FVector2D::DistSquared(Source.Location, Where) <= Source.RadiusSquared)
				{
					// The camp's attenuation (a malus only: the rule leaves a bonus alone); the same effect id from several sources is refreshed, never cumulated.
					const int32 OwnerSlot = States[Index].OwnerPlayer;
					HOLMorale::Apply(Fragments[Index].State, Source.Effect, OwnerSlot < FHOLMatchConfig::MaxSlots ? SlotAttenuationPercent[OwnerSlot] : 0);
				}
			}
		}
	});
}
