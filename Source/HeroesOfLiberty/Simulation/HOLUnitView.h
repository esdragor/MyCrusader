// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLResources.h"
#include "Simulation/HOLSimulationFragments.h"

class UWorld;
struct FHOLHeroSheet;

/** A unit as this machine knows it (J4): what the player can see, select and order. */
struct FHOLUnitView
{
	uint32 UnitId = 0;
	uint8 OwnerPlayer = 0;
	FName TypeName;
	/**
	 * Where it is drawn this frame: the simulation in solo, between its last two fixed steps (D219), the display entity
	 * on a client (D271, D272).
	 */
	FVector Location = FVector::ZeroVector;
	/** Body radius (cm), from the type table. */
	float Radius = 60.f;
	/** Drawing scale of its mesh, from the type table: the selection aims at the body as drawn (D288). */
	float MeshScale = 1.f;
	/** What it is doing and carrying (J5, D298): the selection block shows it. A client has them from the last state received. */
	EHOLUnitActivity Activity = EHOLUnitActivity::Idle;
	/** What it gathers, while Activity is Gathering (J5c.1: on a client, received with the activity). */
	EHOLResource GatherResource = EHOLResource::Food;
	bool bCarrying = false;
	EHOLResource LoadResource = EHOLResource::Food;
	/**
	 * How much it carries: always known in solo and on the server; on a client, only for the player's own unit selected
	 * alone (D303, AHOLPlayerController's watched unit), once the server has answered.
	 */
	int32 LoadAmount = 0;
	bool bLoadAmountKnown = false;
	/** What its type carries at most (0: it does not gather). */
	int32 CarryCapacity = 0;
	/** Its type's class in a group's formation (HOLFormation::GetRowPriority): a shooter (HOLCombat::IsRangedAttack), a siege unit. */
	bool bRanged = false;
	bool bSiege = false;
	/** The King of Regicide (V5.3): moves, stops and attacks on order only, no attack-move nor hold (HOLCommandGrid::GetOrder). */
	bool bKing = false;
	/** Unhurt (J7b.4): its health is its type's maximum. */
	bool bFullHealth = true;
	/**
	 * Its health points (J7b.5) and its type's maximum (0: type unknown): exact everywhere since J7-Q5 revised (a client
	 * gets a wounded unit's exact health, HOLUnitView::GetReceivedHealth); bHealthExact false for a client's unit of
	 * unknown type, and for a client's hero before his sheet arrives (J8a.3: his health comes as a share, Health is drawn
	 * back from it; once his public hero sheet is there and alive, Health is the sheet's, exact). What the selection
	 * block and the health bar show.
	 */
	int32 Health = 0;
	int32 MaxHealth = 0;
	bool bHealthExact = false;
	/** The share received, 0 to 1, for a client's hero (HOLUnitView::GetReceivedHealthShare); below 0 otherwise. */
	float HealthShare = -1.f;

	/** What its health bar shows filled, 0 to 1: the share received for a client's hero, Health of MaxHealth otherwise. */
	float GetHealthShare() const
	{
		if (HealthShare >= 0.f)
		{
			return FMath::Min(HealthShare, 1.f);
		}
		if (MaxHealth > 0)
		{
			return FMath::Clamp(static_cast<float>(Health) / static_cast<float>(MaxHealth), 0.f, 1.f);
		}
		return bFullHealth ? 1.f : 0.f;
	}
	/** While Activity is Attacking: its target (0: none, or not known to this client yet). */
	uint32 TargetUnitId = 0;
};

namespace HOLUnitView
{
	/** FHOLNetUnitState::MaxWireHealth, whose header this one does not include: a hero's whole share (J8a.3). */
	constexpr int32 MaxWireHealthShare = (1 << 8) - 1;

	/**
	 * The units of a game world: simulated (solo, server) or displayed (client). A unit Dying (J7b.4) cannot be selected,
	 * ordered or aimed at: it is left out unless bIncludeDying (the scripted checks).
	 */
	HEROESOFLIBERTY_API void Gather(const UWorld& World, TArray<FHOLUnitView>& OutUnits, bool bIncludeDying = false);
	/** One unit of a game world, without gathering them all (the selection block, every frame); false when it is gone. */
	HEROESOFLIBERTY_API bool Find(const UWorld& World, uint32 UnitId, FHOLUnitView& OutUnit, bool bIncludeDying = false);
	/**
	 * A client's health from the state received (J7-Q5 revised): the exact health points a wounded unit sends
	 * (HOLUnitReplication::GetWireHealth), MaxHealth when unhurt. Back to levels: the level's health here.
	 * bHealthShare (a hero, J8a.3): the wire holds a share (HOLUnitReplication::GetWireHealthShare), drawn back to the
	 * nearest health points, held within 1 and MaxHealth - 1 while wounded and alive.
	 */
	inline int32 GetReceivedHealth(const bool bFullHealth, const uint16 WireHealth, const int32 MaxHealth, const bool bHealthShare = false)
	{
		if (bFullHealth || !bHealthShare || WireHealth == 0)
		{
			return bFullHealth ? MaxHealth : static_cast<int32>(WireHealth);
		}
		const int64 Nearest = (2 * static_cast<int64>(WireHealth) * MaxHealth + MaxWireHealthShare) / (2 * MaxWireHealthShare);
		return static_cast<int32>(FMath::Clamp<int64>(Nearest, 1, FMath::Max(1, MaxHealth - 1)));
	}
	/** What a client's hero health bar shows filled (J8a.3), 0 to 1, from the state received with bHealthShare. */
	inline float GetReceivedHealthShare(const bool bFullHealth, const uint16 WireHealth)
	{
		return bFullHealth ? 1.f : FMath::Clamp(static_cast<float>(WireHealth) / static_cast<float>(MaxWireHealthShare), 0.f, 1.f);
	}
	/**
	 * A client's view health from the state received (UHOLUnitReceptionSubsystem): MaxHealth, Health, HealthShare and
	 * bHealthExact. A troop's exact; a hero's (bHero, J8a.3) share drawn back, the share filling his bar, unless HeroSheet
	 * (his slot's public sheet, null when not received) is Alive: its exact HP then, the share still filling the bar.
	 */
	HEROESOFLIBERTY_API void SetReceivedHealth(FHOLUnitView& View, bool bFullHealth, uint16 WireHealth, int32 MaxHealth, bool bHero,
		const FHOLHeroSheet* HeroSheet = nullptr);
}
