// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mass/EntityHandle.h"
#include "MassEntityTypes.h"
#include "Simulation/HOLFormationRules.h"
#include "Simulation/HOLGatherRules.h"
#include "HOLSimulationFragments.generated.h"

/** Transform at the start of the last fixed step: the presentation interpolates from it to FTransformFragment. */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLPreviousTransformFragment : public FMassFragment
{
	GENERATED_BODY()

	FTransform Transform = FTransform::Identity;
};

/** Every light unit (D29): filters the presentation processors. */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLLightUnitTag : public FMassTag
{
	GENERATED_BODY()
};

/**
 * What the RTS layer sees a unit doing (D226): standing, walking to its current order, gathering at its source, on its
 * gathering's round trip, or building. Sent to every client in 3 bits (FHOLNetUnitState::ActivityBits), Gathering with
 * its resource (J5c.1): it takes values 4 to 7 of the wire (HOLUnitReplication::EncodeActivity); Building (J6a.4) takes
 * value 2, Gathering's own, never written, which was the last free value of the wire: a sixth activity needs one bit more.
 */
UENUM()
enum class EHOLUnitActivity : uint8
{
	/** Standing with nothing to do: an idle worker for the HUD and the "." key (J5b, D307). */
	Idle,
	Moving,
	/**
	 * Standing at its source, taking from it (J5, D298): the client shows it, the presentation plays the gesture of the
	 * resource (J5c.1, FHOLUnitStateFragment::GatherResource).
	 */
	Gathering,
	/**
	 * On its gathering's round trip (J5b, D307): walking to its source or to a deposit, or waiting for a free spot. It
	 * walks as Moving does, but is never idle: a gatherer between two walks no longer shows Idle for a step.
	 */
	GatherTrip,
	/**
	 * At its spot around a site, working on it (J6a.4, D312): never idle (the HUD's count, the "." key). Written on the
	 * wire as value 2 (HOLUnitReplication::BuildingWireValue): its enum value is above the wire's first gathering value.
	 */
	Building,
	/**
	 * In reach of its target, striking (J7b.4, J7-Q5: FHOLCombatFragment::bAttacking): the client plays the strike and
	 * turns it towards its target, whose identifier comes with the state (FHOLNetUnitState::TargetSlot).
	 */
	Attacking,
	/**
	 * At 0 health (J7b.4, J7-Q5): kept ~1 s (HOLCombat::DyingSteps) for its fall before its deferred destruction (D224),
	 * no corpse (D86). Nobody takes it on, it obeys no order, it cannot be selected (HOLUnitView).
	 */
	Dying,
};

/** True while the unit walks: to a player's order, or on its gathering's round trip. */
inline bool IsWalkingActivity(const EHOLUnitActivity Activity)
{
	return Activity == EHOLUnitActivity::Moving || Activity == EHOLUnitActivity::GatherTrip;
}

/** Where a unit is in its move order (D286, D287). */
UENUM()
enum class EHOLMovePhase : uint8
{
	/** No order, or the order is over. */
	None,
	/** Ordered: its cluster's path is not computed yet (a budget of path requests per step). */
	WaitingPath,
	/** Following its cluster's path (Mass' short path along the shared corridor). */
	Path,
	/** From the end of the path straight to its own slot, then it stands. */
	FinalLeg,
	/**
	 * A group's block march (J7a.4): taking its slot on the move (playtest note 1 of J7b), never holding it; ahead of its
	 * slot, walking on slower along the march to its slot's file until its slot catches it up; behind it, to it.
	 */
	FormUp,
	/** A group's block march (J7a.4): at its slot in the block marching along the group's path (Slot moves with it). */
	Block,
};

/** A unit's part of its move order (J4, D286, D287), on the server. */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLMoveOrderFragment : public FMassFragment
{
	GENERATED_BODY()

	/** Its slot around the target, on the navmesh. */
	FVector Slot = FVector::ZeroVector;

	/**
	 * Its slot in its block march, set each step of the march (J7a.4): Slot is where it is steered, a point ahead along the
	 * march while it is ahead of its slot (the user's 4th trial of playtest note 1 of J7b: it never turns back, it slows
	 * down and lets its slot come back to it). A member's gap, and the formation report's, are read from here.
	 */
	FVector2f MarchSlot = FVector2f::ZeroVector;

	/**
	 * The march's direction where it is steered on a block's march (SteerOnMarch; zero otherwise, a new order clears it):
	 * the avoidance's turn never sends it against the march (playtest note 1, loop 20: a unit forming up going round
	 * one of its kind took a step back).
	 */
	FVector2f MarchDirection = FVector2f::ZeroVector;

	/** Distance to the slot at the last step of the final leg, to tell a unit that no longer gets closer. */
	float LastSlotDistance = 0.f;

	/** Where it stood when it last moved on along its path: a unit that no longer leaves that spot is stuck on its path. */
	FVector2f PathAnchor = FVector2f::ZeroVector;

	/** Sideways place (cm, to the right of the march) along its cluster's corridor, so the units do not walk in a file. */
	float Lateral = 0.f;

	/** Order the unit follows: a path computed for an older order is not applied to it. */
	uint32 OrderSerial = 0;

	/**
	 * The slowest speed (cm/s, the table's) of the units of its order still on their way, while it walks in a group's
	 * formation (J7a.4): it walks no faster (HOLFormation::GetMarchSpeed). 0: its own speed (one unit, a task's walk, a
	 * walk back to its place). Set by the order, then each step from the order's walkers of the step before.
	 */
	float MarchSpeed = 0.f;

	/** Its hold on its formation place (J7a.4): pushed off it, it walks back (HOLFormation::WatchPlace). */
	HOLFormation::FPlaceHold PlaceHold;

	/**
	 * Facing (deg) of its formation's rows (J7a.4: the march's way, or across a right-button drag's line), set by a group's
	 * formation move. Standing at the end of the walk (bFacesFront), it is held turned this way (UHOLInstantOrientationProcessor).
	 */
	float FrontYaw = 0.f;

	/**
	 * Steps of the final leg without getting closer to the slot (pushed by others, or the slot taken); along its path, steps
	 * without leaving its anchor (held at a corner of the navmesh, its move target gone round it).
	 */
	uint16 StuckSteps = 0;

	/**
	 * Steps in a row a building's footprint put it back while it walked (D310, UHOLFootprintKeepOutProcessor): its way
	 * crosses a building the navmesh did not show yet when its path was found; the order processor finds it a new one.
	 */
	uint8 BuildingPushes = 0;

	EHOLMovePhase Phase = EHOLMovePhase::None;

	/** Its order is a group's move in formation (J7a.4): it ends holding its place (PlaceHold), the group marches at MarchSpeed. */
	bool bFormation = false;

	/**
	 * Its final leg is a block's re-forming at its arrival (the user's 3rd trial of J7a.4): it goes through the others of
	 * its order re-forming too, walking or standing (GetPassThroughSerial), until the arrival is
	 * settled (UHOLUnitOrderProcessor::WatchArrivals). A new order clears it.
	 */
	bool bReforming = false;

	/**
	 * A straggler of its order's block march (the user's trial of playtest note 1 of J7b): out of the block, it walks at
	 * its own speed (not its order's slowest); steered straight to its slot (Block phase), it goes through the units of
	 * its order, until it is one of the block. A new order, or its joining the block, clears it.
	 */
	bool bStraggler = false;

	/**
	 * It reached its formation place (or the end of its walk back there, J7a.4): standing, it faces FrontYaw. A new order,
	 * or an activity other than Idle, lets it go.
	 */
	bool bFacesFront = false;

	/**
	 * Its path ended short of its slot (a partial path: the slot cannot be reached from where it stood, J6b.2: shut in by
	 * buildings); it walks to the path's end. A task that sent it there takes that end for no arrival.
	 */
	bool bShortOfSlot = false;

	/**
	 * Its kind of row in its block march, plus one (HOLFormation::GetRowPriority + 1, set as the block starts); 0: none.
	 * Two units taking their slots (FormUp) go through each other unless they share it (GetPassThroughKind). A new order
	 * clears it.
	 */
	uint8 RowKind = 0;

	/**
	 * Its block in its order's block march (UHOLUnitOrderProcessor::FBlockMarch::Index, playtest note 1 of J7b: small
	 * blocks): 0 the body (and its stragglers), 1 and on a separate packet's own block. A new order clears it.
	 */
	uint8 BlockIndex = 0;
};

/**
 * A member of its order's block march (Block phase, neither a straggler nor re-forming) with its kind of row (RowKind): as
 * one forming up, it goes through the units of its order of other kinds of rows only (the user's trial of playtest note 1
 * of J7b: members never going through pushed the units forming up of another kind ahead of their slots, and held the
 * block). One with no kind (RowKind 0) keeps clear of everyone.
 */
inline bool IsBlockMemberWithKind(const FHOLMoveOrderFragment& Order)
{
	return Order.Phase == EHOLMovePhase::Block && !Order.bStraggler && !Order.bReforming && Order.RowKind != 0;
}

/**
 * The order whose walkers a unit goes through (J7a.4): its order's while it forms up for a block march, marches in the
 * block with its kind of row (IsBlockMemberWithKind, J7b note 1), re-forms at the block's arrival (until all stand), or, a
 * straggler, walks straight into its slot (J7b note 1); 0 otherwise (it keeps clear of everyone). Forming up or marching in
 * the block, it goes only through the units of other kinds of rows (GetPassThroughKind, CanPassThrough).
 */
inline uint32 GetPassThroughSerial(const FHOLMoveOrderFragment& Order)
{
	return Order.Phase == EHOLMovePhase::FormUp || Order.bReforming || (Order.bStraggler && Order.Phase == EHOLMovePhase::Block)
		|| IsBlockMemberWithKind(Order)
		? Order.OrderSerial : 0u;
}

/**
 * Its kind of row while it forms up for a block march or marches in the block (RowKind: the kind plus one), 0 otherwise
 * (a straggler, or re-forming: through all of its order). Two units of one order with the same one keep clear of each
 * other (the user's trial of playtest note 1 of J7b: two groups of one kind gathering walked into each other and
 * overlapped), their places given without crossings within a kind; two kinds go through each other (their rows cross, as
 * at the block's arrival), a member and one forming up as two forming up.
 */
inline uint8 GetPassThroughKind(const FHOLMoveOrderFragment& Order)
{
	return Order.Phase == EHOLMovePhase::FormUp || IsBlockMemberWithKind(Order) ? Order.RowKind : uint8(0);
}

/**
 * Two units go through each other: of one order (GetPassThroughSerial, not 0), and not two forming up or marching in the
 * block in the same kind of row (GetPassThroughKind).
 */
inline bool CanPassThrough(const uint32 Serial, const uint8 Kind, const uint32 OtherSerial, const uint8 OtherKind)
{
	return Serial != 0u && Serial == OtherSerial && (Kind == 0 || Kind != OtherKind);
}

/** Where a gatherer is in its round trip (J5, D296). */
UENUM()
enum class EHOLGatherPhase : uint8
{
	/** Not gathering: idle, or walking where a player sent it (its load kept). */
	None,
	/** Walking to its spot at its source. */
	ToSource,
	/** At its source, taking from it. */
	Gathering,
	/** Carrying its load to the nearest deposit. */
	ToDeposit,
};

/** A unit's gathering (J5, D296), on the server: its source, its load, its place in the round trip. */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLGatherFragment : public FMassFragment
{
	GENERATED_BODY()

	/**
	 * The source it gathers (an index of UHOLSourceSubsystem), INDEX_NONE: none. Kept while it carries its load to the
	 * deposit, so it walks back to the same source; INDEX_NONE on the way to the deposit: it stands there once unloaded.
	 */
	int32 SourceIndex = INDEX_NONE;

	/** Where it walks: its spot at the source, or against the deposit. */
	FVector Spot = FVector::ZeroVector;

	/** Where its current walk began, and how far it has walked since (hol.Gather.LogWalks: detours, D308). */
	FVector WalkStart = FVector::ZeroVector;
	FVector LastLocation = FVector::ZeroVector;
	float Walked = 0.f;
	/** Farthest it went from the straight line to Spot, and the steps its walk took (hol.Gather.LogWalks). */
	float MaxLateral = 0.f;
	uint16 WalkSteps = 0;

	/** Its move order's serial when it gave itself the walk to Spot: a new serial, then no order, is the arrival. */
	uint32 OrderSerialBefore = 0;

	/** What it carries; kept by a player's move order, lost when it gathers another resource (D296). */
	FHOLGatherLoad Load;

	/** Arrivals in a row that missed their spot (taken, or pushed away). */
	uint8 Misses = 0;

	/** Walks to its source in a row whose path ended short of the spot (out of reach: shut in by buildings, J6b.2). */
	uint8 UnreachableWalks = 0;

	/** The walk to Spot is asked for (it starts at the next step, through the order queue). */
	bool bWalkIssued = false;

	/**
	 * It started gathering this step (D309): having walked through every unit to its spot, it pushes a standing unit it
	 * landed on apart, down to the spots' spacing (UHOLUnitSeparationProcessor: a gatherer beside it on its own spot is
	 * left); the next step clears it.
	 */
	bool bSettling = false;

	/**
	 * Steps it keeps settling after this one (J7a.6): stopped idle where its round trip left it (at a deposit, beside
	 * others stopped there, against the building's wall that puts back a body pushed into it), one look left two
	 * Peasants 83 cm apart; it looks again for a few steps.
	 */
	uint8 SettleSteps = 0;

	EHOLGatherPhase Phase = EHOLGatherPhase::None;

	bool IsGathering() const { return Phase != EHOLGatherPhase::None; }

	/**
	 * Sent to gather a source (a gather order, or the head of its queue: the user's trial of J6a.4): its round trip starts
	 * there; what it carries of another resource is kept until its first unit of this one (D306).
	 */
	void StartTask(const int32 InSourceIndex, const EHOLResource Resource)
	{
		HOLGather::StartGathering(Load, Resource);
		SourceIndex = InSourceIndex;
		Phase = EHOLGatherPhase::ToSource;
		bWalkIssued = false;
		Misses = 0;
		UnreachableWalks = 0;
	}
};

/** Per-entity state of a unit (D222), the values that change: identifier, health, activity. */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLUnitStateFragment : public FMassFragment
{
	GENERATED_BODY()

	/** Stable identifier (D223), attributed by the server and never recycled during a match. */
	UPROPERTY()
	uint32 UnitId = 0;

	UPROPERTY()
	int32 CurrentHealth = 0;

	UPROPERTY()
	EHOLUnitActivity Activity = EHOLUnitActivity::Idle;

	/**
	 * What it gathers, read only while Activity is Gathering (J5c.1: the presentation plays the axe, the pick or the
	 * picking of berries). Sent with the activity, in its 3 bits (HOLUnitReplication::EncodeActivity).
	 */
	UPROPERTY()
	EHOLResource GatherResource = EHOLResource::Food;

	/** Player the unit belongs to (D233): a conversion (D37) rewrites it. The client turns it into a colour. */
	UPROPERTY()
	uint8 OwnerPlayer = 0;
};

/** How a unit takes on enemies (J7b.2): the plan's three modes (HOLCombat::ESeekMode), orders set them from J7b.3. */
UENUM()
enum class EHOLCombatMode : uint8
{
	/** Takes on the enemies within its aggression radius (none for the Peasant). */
	Normal,
	/** Attack-move: as Normal, a unit with no aggression of its own looks within HOLCombat::AttackMoveRadius. */
	AttackMove,
	/** Hold position: only enemies in reach, never a chase. */
	Hold,
};

/**
 * J7c.4 loop 21 (hol.Combat.RearFlank): a side's line of contact, made each step by UHOLUnitSeparationProcessor from its
 * units of the front: Origin their mean place, Charge the way to the other armies' middle, Axis across it (left of it),
 * Min/Max the span of the front's units along Axis from Origin.
 */
struct FHOLFrontLine
{
	FVector2D Origin = FVector2D::ZeroVector;
	FVector2D Axis = FVector2D::ZeroVector;
	FVector2D Charge = FVector2D::ZeroVector;
	double Min = 0.;
	double Max = 0.;
	int32 Count = 0;
};

/**
 * A unit's fight (J7b.2, J7-Q4), on the server: its target, its cool-down, the last enemy that hit it (riposte), and
 * what the combat report shows. The health is FHOLUnitStateFragment::CurrentHealth (the table's MaxHealth at birth).
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLCombatFragment : public FMassFragment
{
	GENERATED_BODY()

	/** The enemy it takes on; unset: none. */
	FMassEntityHandle Target;

	/** The last enemy that hit it: taken on by a riposte (HOLCombat::FCandidate::bAttacksMe) even beyond its search radius. */
	FMassEntityHandle LastAttacker;

	/** Where its target stood when it last walked towards it: a new walk once the target moved away from there. */
	FVector2D ApproachPoint = FVector2D::ZeroVector;

	/** Where an attack-move takes it (its slot of the order, J7b.3): once no enemy is left, it walks on there. */
	FVector AttackMoveGoal = FVector::ZeroVector;

	/**
	 * Changed by every player order (J7b.3, Disengage): a combat walk given before carries the old one (FHOLMoveOrder::
	 * CombatStamp) and is dropped, never overwriting the newer order. Never 0.
	 */
	uint32 EngageSerial = 1;

	/** Milliseconds to its next hit (HOLCombat::TickAttack). */
	int32 CooldownMs = 0;

	/** Others of its side taking it on, counted at the step's start (HOLCombat::FCandidate::AttackerCount, the report). */
	int32 AttackerCount = 0;

	/** Hits it struck, damage dealt, kills, and the step of its first hit (the combat report); -1: none yet. */
	int32 Hits = 0;
	int32 DamageDealt = 0;
	int32 Kills = 0;
	int64 FirstHitStep = -1;

	EHOLCombatMode Mode = EHOLCombatMode::Normal;

	/** On an attack-move, a fight took it off its way (J7b.3): once no enemy is left, it walks on to AttackMoveGoal. */
	bool bAttackMoveDetour = false;

	/** Step it fell to 0 health at (J7b.4): destroyed HOLCombat::DyingSteps later; -1: alive. */
	int64 DeathStep = -1;

	/** Its target was ordered (J7b.3): kept, never replaced by its own choice, until it dies. */
	bool bOrderedTarget = false;

	/** In reach of its target, striking (HOLCombat::StartAttack was applied): out of reach, it starts again. */
	bool bAttacking = false;

	/** It walks towards its target on a walk of its own (a task's move order): stopped once in reach or the target gone. */
	bool bApproaching = false;

	/**
	 * J7c.4 loop 20 (hol.Combat.RearSlideKeep): the way (world, unit length) it slides along the friends barring its way,
	 * kept while it slides; zero: none. SlideShutSeconds: how long that way has been shut; SlideFreeSeconds: how long it has
	 * walked on unbarred (the way forgotten past a while). Written by UHOLFullSpeedProcessor only.
	 */
	FVector2f SlideWay = FVector2f::ZeroVector;
	float SlideShutSeconds = 0.f;
	float SlideFreeSeconds = 0.f;

	/**
	 * J7c.4 loop 21 (hol.Combat.RearFlank): a melee of an attack-move held behind its own line goes round its end.
	 * FlankWaitSeconds: how long it has been held (then, flanking: how long it has flanked); FlankSide: the end it goes to
	 * (+1/-1 along FHOLFrontLine::Axis); FlankWalkGoal: the point its last walk was given (zero: none yet).
	 */
	float FlankWaitSeconds = 0.f;
	bool bFlanking = false;
	int8 FlankSide = 0;
	FVector2f FlankWalkGoal = FVector2f::ZeroVector;
	/** J7c.4 loop 22 (hol.Combat.RearFlankBack): how far behind its line (cm) its first point, at the end's level, lies; 0: passed or none. */
	float FlankBack = 0.f;

	/**
	 * A player's order prevails (J7b.2): its target, its approach, its strikes and its last attacker forgotten (no riposte
	 * after the order). Its walk is the order's: nothing stopped here. Idle again, it looks for enemies as before.
	 */
	void Disengage()
	{
		Target = FMassEntityHandle();
		LastAttacker = FMassEntityHandle();
		bOrderedTarget = false;
		bAttacking = false;
		bApproaching = false;
		bAttackMoveDetour = false;
		EngageSerial = EngageSerial == MAX_uint32 ? 1u : EngageSerial + 1u;
	}
};
