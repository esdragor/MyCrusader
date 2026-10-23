// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLForumPresence.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLConstructionRules.h"
#include "Engine/World.h"
#include "Match/HOLVictoryRules.h"
#include "Simulation/HOLUnitSubsystem.h"

AHOLGameState::FForumPresenceProvider FHOLForumPresenceCounter::MakeProvider(const TSharedRef<FHOLForumPresenceCounter>& Counter)
{
	return [Counter](const AHOLGameState& GameState, const int32 Slot) { return Counter->Get(GameState, Slot); };
}

FHOLForumPresence FHOLForumPresenceCounter::Get(const AHOLGameState& GameState, const int32 Slot)
{
	if (CountedStep != GameState.GetCurrentStep())
	{
		CountedStep = GameState.GetCurrentStep();
		Count(GameState);
	}
	return Presences.IsValidIndex(Slot) ? Presences[Slot] : FHOLForumPresence();
}

void FHOLForumPresenceCounter::Count(const AHOLGameState& GameState)
{
	Presences.Reset();
	Presences.SetNum(FHOLMatchConfig::MaxSlots);
	const UWorld* World = GameState.GetWorld();
	const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	if (!Units)
	{
		return;
	}

	// The zones of the forums down this step: none, no walk at all.
	struct FZone
	{
		int32 Slot = INDEX_NONE;
		int32 Camp = INDEX_NONE;
		FBox2D Body;
		FBox2D Reach;
		TArray<int32, TInlineAllocator<8>> Attackers;
		bool bDefenders = false;
	};
	TArray<FZone, TInlineAllocator<FHOLMatchConfig::MaxSlots>> Zones;
	const FHOLMatchConfig& Config = GameState.GetLaunchConfig();
	const double Radius = FHOLForumTuning::FromSettings().HoldRadius;
	for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		if (!GameState.GetForumState(Slot).IsDown() || GameState.IsSlotDefeated(Slot))
		{
			continue;
		}
		const AHOLBuilding* Forum = AHOLGameState::FindPrimaryForum(*World, Slot);
		if (!Forum)
		{
			continue;
		}
		FZone& Zone = Zones.AddDefaulted_GetRef();
		Zone.Slot = Slot;
		Zone.Camp = HOLVictoryRules::GetCamp(Config, Slot);
		Zone.Body = Forum->GetBodyBounds();
		Zone.Reach = Zone.Body.ExpandBy(Radius);
	}
	if (Zones.IsEmpty())
	{
		return;
	}

	++Walks;
	UnitsWalked += Units->ForEachLivingUnit([&Zones, &Config, Radius](const uint8 Owner, const FVector& Location)
	{
		const FVector2D Point(Location);
		for (FZone& Zone : Zones)
		{
			if (!Zone.Reach.IsInside(Point) || HOLConstruction::GetDistance(Zone.Body, Point) > Radius)
			{
				continue;
			}
			// D401: an owner out of the launch's slots (Neutral) is an adversary.
			const int32 Camp = Owner < FHOLMatchConfig::MaxSlots ? HOLVictoryRules::GetCamp(Config, Owner) : INDEX_NONE;
			if (Owner == Zone.Slot || (Camp != INDEX_NONE && Camp == Zone.Camp))
			{
				Zone.bDefenders = true;
				continue;
			}
			if (Zone.Attackers.Num() <= Owner)
			{
				Zone.Attackers.SetNumZeroed(Owner + 1);
			}
			++Zone.Attackers[Owner];
		}
	});

	for (const FZone& Zone : Zones)
	{
		FHOLForumPresence& Presence = Presences[Zone.Slot];
		Presence.bDefenders = Zone.bDefenders;
		int32 Most = 0;
		for (int32 Owner = 0; Owner < Zone.Attackers.Num(); ++Owner)
		{
			// The most units in the zone, the lowest slot on a tie (the first met).
			if (Zone.Attackers[Owner] > Most)
			{
				Most = Zone.Attackers[Owner];
				Presence.AttackerSlot = Owner;
			}
		}
		Presence.bAttackers = Most > 0;
	}
}
