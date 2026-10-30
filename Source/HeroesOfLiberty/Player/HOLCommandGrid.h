// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/ICursor.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Match/HOLResources.h"
#include "Simulation/HOLHeroAbilityRules.h"

/** What a cell of the command grid does (D30, D313). */
enum class EHOLGridCommand : uint8
{
	None,
	/** Queues a unit at the player's selected building (J3, D282). */
	Produce,
	/** Starts the placement of a building (J6a.5, D313): the builders' grid, A house, Z lumber camp, E mining camp. */
	Build,
	/** Cancels the player's selected site, refunded (D311). */
	CancelConstruction,
	/** Attack-move (J7b.3): the next left click on the ground sends the soldiers there, taking on the enemies on the way. */
	AttackMove,
	/** Stop (J7b.3): the units stand, their orders, queue and fight forgotten. */
	Stop,
	/** Hold position (J7b.3): stop, then only enemies in reach, never a chase, until a move order. */
	Hold,
	/**
	 * A hero's ability (J8c.4, J8c-Q4): A Serrez les rangs (cast at once), Z Bannière de l'Aube (the next left click on the
	 * ground plants it, 16 m at most from the hero).
	 */
	HeroAbility,
};

/** A cell's command and the type it names (the unit produced, the building placed). */
struct FHOLGridCommand
{
	EHOLGridCommand Command = EHOLGridCommand::None;
	FName TypeName;
	/** Build: the age of its panel (0 to 3); bGreyed: an age the player has not reached, shown greyed, a press does nothing. */
	int32 Age = INDEX_NONE;
	bool bGreyed = false;
	/** HeroAbility: which one; bGreyed while it is not open at the hero's level or cooling down (CooldownSeconds left). */
	EHOLHeroAbility Ability = EHOLHeroAbility::Aura;
	int32 CooldownSeconds = 0;

	bool IsNone() const { return Command == EHOLGridCommand::None; }
	bool operator==(const FHOLGridCommand& Other) const = default;
};

/** What the player is told when a command does not go through as asked (D313): a short message above the grid. */
enum class EHOLCommandFeedback : uint8
{
	None,
	/** The stock does not cover the cost: nothing done. */
	NotEnoughResources,
	/** The building's queue is full (D283): nothing queued. */
	QueueFull,
	/** No room in the population: queued, but the production waits (« Population maximale — construisez des maisons »). */
	PopulationFull,
	/** A red ghost clicked: nothing placed. */
	PlacementRefused,
	/** An F key (D30) while the player has no building of its category (« Aucun bâtiment militaire »): nothing selected. */
	NoBuilding,
};

struct FHOLCommandFeedback
{
	EHOLCommandFeedback Kind = EHOLCommandFeedback::None;
	/** NotEnoughResources: the first resource missing, in the order Food, Wood, Stone, Gold. */
	EHOLResource Resource = EHOLResource::Food;
	/** PlacementRefused: why. */
	EHOLPlacementRefusal Refusal = EHOLPlacementRefusal::None;
	/** NoBuilding: the category of the F key. */
	EHOLBuildingCategory Category = EHOLBuildingCategory::Economic;

	bool IsNone() const { return Kind == EHOLCommandFeedback::None; }
	bool operator==(const FHOLCommandFeedback& Other) const = default;
};

/** What the command grid is read from: the player's selection as this machine knows it. */
struct FHOLGridContext
{
	/**
	 * The player's own building is selected: finished, it produces; not finished, its site may be cancelled. One of
	 * several shown together (bBuildingGroup: « Tous » or a subgroup of several, an F key, D30): its cells are read the
	 * same, never « Annuler » (a group is never cancelled at once), then merged with the others' (MergeCommands).
	 */
	bool bOwnBuilding = false;
	bool bBuildingGroup = false;
	bool bBuildingFinished = false;
	/** V2.3a: the main forum down (ruin or rebuilding): it produces nothing, nothing to cancel; its cells are empty. */
	bool bForumDown = false;
	TArray<FName> Produces;
	/**
	 * The player's own selected units count at least one builder (a unit that gathers, CarryCapacity > 0: what the
	 * server checks for a build order, J6a.4).
	 */
	bool bBuilders = false;
	/**
	 * The player's own units are selected (J7b.3: stop, hold); among them soldiers (every type but the Peasant: attack-move,
	 * on Q, the builders' grid hidden: the user's trial of J7b).
	 */
	bool bOwnUnits = false;
	bool bSoldiers = false;
	/**
	 * Every selected unit is a King (V5.3, Regicide): no attack-move nor hold, which would have it take on enemies by itself;
	 * stop only (the move and the attack on an enemy unit are the right click's). A mixed selection keeps them for the
	 * others: the server then gives the King a plain move (HOLCombat::GetSearchRadius).
	 */
	bool bOnlyKings = false;
	/**
	 * The player's own hero is among the units commanded (J8c.4): his abilities on the grid on the right (HeroAbilityCell),
	 * read from his level and the seconds left of each ability's cool-down (indexed by EHOLHeroAbility; the owner's private
	 * sheet, J8-Q5).
	 */
	bool bHero = false;
	int32 HeroLevel = 0;
	int32 HeroCooldownSeconds[3] = {};
	/** The types builders build, their cell and their age (BuildGridSlot, Age of the building type table). */
	TArray<FHOLBuildable> Buildable;
	/** The last age the player has reached (HOLCommandGrid::GetReachedAge): the panels after it greyed. */
	int32 ReachedAge = 0;
};

/** Where a key of the grid goes (the user's choice of 2026-10-21, AoE4's way): the grid on the right, the orders on the left. */
enum class EHOLGridKeyTarget : uint8
{
	/** Nothing (an age not reached, an empty cell of the chosen age). */
	None,
	/** The grid on the right: a building's production or its site's « Annuler ». */
	Grid,
	/** The orders' panel on the left: attack-move, stop, hold. */
	Orders,
	/** An age chosen (A Z E R, or a click on its header): its keys build. */
	Age,
	/** The ghost of TypeName, a building of the chosen age (a ghost shown before: replaced). */
	Build,
};

struct FHOLGridKeyResult
{
	EHOLGridKeyTarget Target = EHOLGridKeyTarget::None;
	/** The age chosen after the press (INDEX_NONE: none). */
	int32 ChosenAge = INDEX_NONE;
	/** Build: the building whose ghost starts. */
	FName TypeName;

	bool operator==(const FHOLGridKeyResult& Other) const = default;
};

/** What closes the chosen age (the user's 1st trial of G.1): the ghost placed or cancelled, Escape, another selection. */
enum class EHOLAgeEvent : uint8
{
	/** The ghost placed (a left click): the placement ends. */
	Placed,
	/** The ghost placed with Shift: the ghost kept for the next one, and the age. */
	PlacedKept,
	/** The ghost cancelled (Escape, a right click). */
	Cancelled,
	/** Escape with no ghost. */
	Escape,
	SelectionChanged,
};

/** One of the player's buildings in the cycle of an F key (D30): its rank (its type) and a key stable on this machine. */
struct FHOLCycleBuilding
{
	int32 Rank = 0;
	uint32 Key = 0;
};

/** One of the selected buildings that could take a unit (a group, D30): its queue, its limit, its key. */
struct FHOLQueueChoice
{
	int32 QueueLength = 0;
	int32 MaxQueue = 0;
	uint32 Key = 0;
};

/** One building of a group (an F key, D30), for its subgroups: its type, and whether it is a subgroup to itself. */
struct FHOLGroupMember
{
	FName TypeName;
	/** A main centre (the user's 2nd trial of J6a.5: « un forum n'a pas de catégorie ; deux forums sont distincts »). */
	bool bAlone = false;
};

/**
 * A subgroup of a group of buildings, the one Tab makes active (the user's 2nd trial of J6a.5, AoE4's way): « Tous »
 * (every building of the group), the buildings of one type, or one main centre alone. The selection stays the group.
 */
struct FHOLSubgroup
{
	bool bAll = false;
	/** None for « Tous ». */
	FName TypeName;
	/** Its buildings: their indices in the group, in the group's order. */
	TArray<int32> Members;

	bool operator==(const FHOLSubgroup& Other) const = default;
};

/**
 * What a cell of the grid shows of its command (the user's trial of J6a.5): its icon, its name while it has none; never
 * its cost, which only its tooltip gives (what is missing in red there), a greyed cell telling it cannot be paid.
 */
struct FHOLGridCellFace
{
	bool bIcon = false;
	bool bName = false;
	bool bCost = false;
};

/**
 * Pure rules of the command grid (D30, D281, D313), tested on their own: what each cell does for the selection, what
 * a cell shows, what a press tells the player when it does not go through, and the F keys: the player's buildings of a
 * category selected together, then the camera from one to the next (the user's trial of J6a.5); a group's subgroups
 * (Tab), whose cells merge, and the even spread of its production (the user's 2nd trial of J6a.5).
 */
namespace HOLCommandGrid
{
	/** Cells of the grid (A Z E R / Q S D F / W X C V), and of the orders' panel on its left (the same keys). */
	inline constexpr int32 NumCells = 12;
	/** The cell of a selected site's « Annuler » (V, bottom right: far from the keys that build). */
	inline constexpr int32 CancelCell = 11;
	/**
	 * The builders' panels (the user's trials of G.1, AoE4's way): the 4 ages side by side (I to IV, ages 0 to 3: Fondation,
	 * Essor, Puissance, Légende), each a medallion (its numeral, its key A Z E R) over its buildings at the places of their
	 * keys, all 12 cells (BuildGridSlot), one row per category (the user's 3rd trial of G.1): A Z E R economic, Q S D F
	 * military, W X C V technology (the F keys' F2 / F1 / F3); an age not reached greyed.
	 */
	inline constexpr int32 NumAges = 4;
	inline constexpr int32 GridColumns = 4;
	inline constexpr int32 NumGridRows = NumCells / GridColumns;

	/**
	 * The row of the builders' panels where the buildings of Category stand: 0 economic, 1 military, 2 technology. One
	 * exception, written in the table (BuildGridSlot): the Farm, economic, on W (IA1.2-Q1: the economic row of age I is full).
	 */
	HEROESOFLIBERTY_API int32 GetCategoryRow(EHOLBuildingCategory Category);
	/**
	 * The units' orders, in the panel left of the grid at AoE4's places (the user's choice of 2026-10-21): Q attack-move
	 * (soldiers among the selection: a Peasant alone has none), S stop, V hold position.
	 */
	inline constexpr int32 AttackMoveCell = 4;
	inline constexpr int32 StopCell = 5;
	inline constexpr int32 HoldCell = 11;

	/**
	 * The hero's abilities on the grid on the right (J8c-Q4, the top row: the units' orders are on the left): A Serrez les
	 * rangs, Z Bannière de l'Aube. INDEX_NONE for the passive aura (its box is the hero's block's alone).
	 */
	HEROESOFLIBERTY_API int32 GetHeroAbilityCell(EHOLHeroAbility Ability);

	/**
	 * The mouse cursor (the user's trial of A.2, AoE4's way): the red attack cursor (Crosshairs, SHOLAttackCursor) while
	 * the attack-move waits for its point (Q), the default arrow otherwise, placement mode included (its ghost shows it).
	 */
	HEROESOFLIBERTY_API EMouseCursor::Type GetMouseCursor(bool bAttackMoveArmed, bool bPlacing);

	/**
	 * What Cell of the grid on the right does: the selected building's production (finished) or its cancel cell (a site);
	 * the hero's abilities when he is among the units commanded (greyed below their level or cooling down); else nothing (the builders' are the age panels': GetAgeBuilding; the units' orders the orders' panel's: GetOrder).
	 */
	HEROESOFLIBERTY_API FHOLGridCommand GetCommand(const FHOLGridContext& Context, int32 Cell);

	/** What Cell of the orders' panel does: with own units and no building, attack-move with soldiers (Q), stop (S), hold (V). */
	HEROESOFLIBERTY_API FHOLGridCommand GetOrder(const FHOLGridContext& Context, int32 Cell);

	/** The builders' age panels are shown: builders selected, no soldier among them, no building. */
	HEROESOFLIBERTY_API bool IsBuildGrid(const FHOLGridContext& Context);

	/** The last age the player has reached: 0 (Fondation) until the ages come (GDD § 11.1). */
	HEROESOFLIBERTY_API int32 GetReachedAge();

	/** Age (0 to 3) is reached: its panel's buildings may be placed. */
	HEROESOFLIBERTY_API bool IsAgeReached(const FHOLGridContext& Context, int32 Age);

	/**
	 * The building at Cell (0 to 11) of the panel of Age, the builders' (IsBuildGrid): Build, its type, its age, greyed
	 * when the age is not reached; else nothing.
	 */
	HEROESOFLIBERTY_API FHOLGridCommand GetAgeBuilding(const FHOLGridContext& Context, int32 Age, int32 Cell);

	/**
	 * Where a key of Cell goes, ChosenAge the age chosen before it (the user's 1st trial of G.1, AoE4's way). A building
	 * selected: the grid, no age. The builders' panels, no age chosen: A Z E R choose their age (one not reached: nothing),
	 * Q S D F / W X C V the orders; an age chosen, every key of the grid starts the ghost of its building there (the user's
	 * 3rd trial of G.1: A then A the house; an empty cell: nothing, never another age), the age kept until the ghost is
	 * placed or cancelled (GetChosenAgeAfter): another key of it changes the ghost. Else: the grid for the hero's
	 * cells (A Z, the hero among the units commanded), the orders for the others.
	 */
	HEROESOFLIBERTY_API FHOLGridKeyResult PressKey(const FHOLGridContext& Context, int32 ChosenAge, int32 Cell);

	/**
	 * A click on the builders' panel of Age: on its header (Cell INDEX_NONE) the age chosen, as its key; on its building at
	 * Cell its ghost, the age chosen. An age not reached: nothing, the chosen age kept.
	 */
	HEROESOFLIBERTY_API FHOLGridKeyResult ClickAge(const FHOLGridContext& Context, int32 ChosenAge, int32 Age, int32 Cell);

	/** The age chosen after Event: kept only by a ghost placed with Shift (the next one); else closed. */
	HEROESOFLIBERTY_API int32 GetChosenAgeAfter(int32 ChosenAge, EHOLAgeEvent Event);

	/** The first resource of Cost the stock does not cover (Food, Wood, Stone, Gold); false when it covers it all. */
	HEROESOFLIBERTY_API bool FindMissingResource(const FHOLResources& Stock, const FHOLResources& Cost, EHOLResource& OutResource);

	/**
	 * A unit asked at a building: queue full first, then the cost (both refused by the server, D282), then the population
	 * (queued, the production waits for room: PopulationFull, which does not block). None: it goes through.
	 */
	HEROESOFLIBERTY_API FHOLCommandFeedback CheckProduction(const FHOLResources& Stock, const FHOLResources& Cost, int32 QueueLength, int32 MaxQueue,
		int32 PopulationUsed, int32 PopulationCap, int32 Places);

	/** A building asked (D313): its cost, taken at its placement (D311). */
	HEROESOFLIBERTY_API FHOLCommandFeedback CheckBuild(const FHOLResources& Stock, const FHOLResources& Cost);

	/** The command is not sent (the population's feedback is only told: the unit is queued). */
	HEROESOFLIBERTY_API bool Blocks(const FHOLCommandFeedback& Feedback);

	/** A unit of Places waits for room in the population (the production stops, D313). */
	HEROESOFLIBERTY_API bool IsPopulationBlocked(int32 PopulationUsed, int32 PopulationCap, int32 Places);

	/** "not enough Wood", "queue full", "population full", "placement refused (on a building)", "no Military building", "none": for the logs. */
	HEROESOFLIBERTY_API FString Describe(const FHOLCommandFeedback& Feedback);

	/** What a cell shows (the user's trial of J6a.5): the icon when the command has one, else its name; the cost never. */
	HEROESOFLIBERTY_API FHOLGridCellFace GetCellFace(bool bHasIcon);

	/** The rank of a type in the cycle of its F key: the main centre first (no grid slot), then by its slot in the builders' grid. */
	HEROESOFLIBERTY_API int32 GetCycleRank(const FHOLBuildingTypeRow& Type);

	/**
	 * Whether the F key of Category selects a building of Type (D30, the user's trial of J6a.5): its category, whatever
	 * its state: finished, under construction, or a foundation not started (a ghost, its owner's alone: the user's 2nd
	 * trial of J6a.5).
	 */
	HEROESOFLIBERTY_API bool IsSelectedByCategory(const FHOLBuildingTypeRow& Type, EHOLBuildingCategory Category);

	/**
	 * A press of an F key repeats the last one (the camera goes to the next building, the selection stays) when it is the
	 * same key and the selection is still what that key selected (the same buildings, in any order); else it selects the
	 * category's buildings again, the camera where it is.
	 */
	HEROESOFLIBERTY_API bool IsRepeatPress(TConstArrayView<uint32> Selected, TConstArrayView<uint32> LastSelected, bool bSameKey);

	/**
	 * The cycle of an F key (D30): of the category's buildings, sorted by rank then key, the index (in Buildings) of the
	 * first one after LastKey, or of the first of all after the last one or when LastKey is not one of them; INDEX_NONE
	 * when there is none. Each repeated press goes to the next one.
	 */
	HEROESOFLIBERTY_API int32 NextInCycle(TConstArrayView<FHOLCycleBuilding> Buildings, uint32 LastKey);

	/** A building of this type is a subgroup to itself in a group: the main centre. */
	HEROESOFLIBERTY_API bool IsAloneInGroup(FName TypeName);

	/**
	 * The subgroups Tab goes through (the user's 2nd trial of J6a.5): « Tous » first, then one per type in the group's
	 * order (the first building of each type places it), except each main centre, a subgroup to itself at its place. A
	 * group of one type and no centre still has « Tous » then its type. None for an empty group.
	 */
	HEROESOFLIBERTY_API TArray<FHOLSubgroup> SplitSubgroups(TConstArrayView<FHOLGroupMember> Members);

	/** The order in which the buildings of « Tous » give their cells: the main centres first, then the group's order. */
	HEROESOFLIBERTY_API TArray<int32> GetCommandPriority(TConstArrayView<FHOLGroupMember> Members);

	/**
	 * The cells of several buildings shown together (« Tous », a subgroup): PerBuilding holds each building's cells
	 * (NumCells each, in the group's order), Priority the order they are read in; a cell two of them fill goes to the
	 * first one (« Tous »: the forum's Peasant on A before a mill's technology on A).
	 */
	HEROESOFLIBERTY_API TArray<FHOLGridCommand> MergeCommands(TConstArrayView<TArray<FHOLGridCommand>> PerBuilding, TConstArrayView<int32> Priority);

	/**
	 * The building of a group (or subgroup) that takes a unit asked, spread evenly (the user's 2nd trial of J6a.5): of
	 * those with room, the fewest queued; between equals, the first after the one LastKey names in the group's order
	 * (each press goes to the next one: 4 presses on 2 centres give 2 + 2, even before their queues reach this machine),
	 * or the first of them when LastKey is none of them. INDEX_NONE when every queue is full.
	 */
	HEROESOFLIBERTY_API int32 PickEvenly(TConstArrayView<FHOLQueueChoice> Choices, uint32 LastKey);
}
