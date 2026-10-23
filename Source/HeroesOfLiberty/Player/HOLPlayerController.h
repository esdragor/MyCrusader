// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Match/HOLResources.h"
#include "Player/HOLCommandGrid.h"
#include "Player/HOLUnitPicking.h"
#include "HOLPlayerController.generated.h"

class AHOLBuilding;
class AHOLPlayerState;
class AHOLRTSCamera;
class AHOLSelectionMarkers;
class UInputAction;
struct FHOLPickCandidate;
struct FHOLUnitView;
struct FInputActionValue;

/**
 * What the player's own unit selected alone carries (D303), sent by the server to its owner at every change: the states
 * sent to everyone only say whether a unit carries (FHOLNetUnitState).
 */
USTRUCT()
struct FHOLWatchedLoad
{
	GENERATED_BODY()

	/** 0: no unit watched. */
	UPROPERTY()
	uint32 UnitId = 0;
	UPROPERTY()
	EHOLResource Resource = EHOLResource::Food;
	UPROPERTY()
	uint8 Amount = 0;

	bool operator==(const FHOLWatchedLoad& Other) const = default;
};

/**
 * One unload at a deposit, shown to the owner alone as a « +10 » rising above its Peasant (J5c.3, D298): sent by the
 * server to the owner's controller, the step's unloads in one unreliable call (UHOLGatherProcessor). ~9 bytes.
 */
USTRUCT()
struct FHOLDepositPopup
{
	GENERATED_BODY()

	/** Where the Peasant unloaded, on the ground (cm). */
	UPROPERTY()
	FVector_NetQuantize Location = FVector::ZeroVector;
	UPROPERTY()
	EHOLResource Resource = EHOLResource::Food;
	/** What it unloaded: its load, 10 at most for a Peasant. */
	UPROPERTY()
	uint8 Amount = 0;
};

/**
 * Controller of a player in a match map (a map with start slots, D255): no pawn, a visible cursor and, on the
 * player's own machine, the RTS camera (D254), placed above the player's start slot once the slot is known. Camera
 * keys come from IMC_Game (Tools/Editor/make_game_input.py); speed and edge scrolling from the Controls tab (D253).
 * Selection (D262, D288) lives here, on the player's machine only: units (their identifiers, D223), one building or one
 * resource source (J5b, D298). A left click selects the unit under the cursor (aimed at on the screen: light units have
 * no collision), else the building under it (its own or another player's), else the source under it, else nothing; a box (a press dragged past a few pixels) selects the
 * player's own units in it; Shift adds or removes; Escape selects nothing; "." selects the next idle worker of the
 * player and centres the camera on it (D30, J5b); the HUD shows it (J3, D281). A right click
 * sends the selected units of the player to the ground point, or puts the selected building's rally point there (J4);
 * on one of the player's sites not finished, its selected Peasants go and help build it (J6a.4, D312; Shift: queued).
 * Production (J3, D282): the cells of the command grid (keys of D30, or a click on the HUD) and the HUD's cancel
 * clicks become requests to the server (RPCs), which queues them in the order queue and applies them at its next step.
 * With builders (Peasants) selected, the grid shows the buildings they build, directly on A, Z, E (J6a.5, D313): a cell
 * starts the placement; a site selected shows « Annuler » on V. F1, F2, F3 select all the player's military, economic,
 * technological buildings (D30, the user's trial of J6a.5), their ghosts included; pressed again, the camera goes from
 * one to the next. Tab goes through the group's subgroups (« Tous », each type, each main centre alone), whose cells the
 * grid shows; a unit asked of several is spread evenly among them (the user's 2nd trial of J6a.5).
 * Placement (J6a.3, D311): in placement mode a ghost of the building follows the cursor tile by tile, green where the
 * server would accept it from what this machine knows (the rules of HOLConstruction) and the stock covers its cost,
 * red otherwise; a left click places it (an RPC, checked again and paid by the server) and leaves the mode, Shift +
 * click places it and stays; Escape or a right click leave the mode. In the mode, clicks never select nor order. The
 * player's selected units are sent to build what is placed (J6a.4, D312): with Shift, each foundation at the end of
 * their queue, built in that order.
 */
UCLASS()
class HEROESOFLIBERTY_API AHOLPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AHOLPlayerController();

	AHOLRTSCamera* GetRTSCamera() const { return RTSCamera; }
	AHOLSelectionMarkers* GetSelectionMarkers() const { return SelectionMarkers; }

	/** The building under a point of the viewport (0 to 1 each way), on the selection channel, or null. */
	AHOLBuilding* FindBuildingAt(const FVector2D& ViewportFraction) const;
	/** The unit drawn under a point of the viewport, the one in front (D288), or 0. */
	uint32 FindUnitAt(const FVector2D& ViewportFraction) const;
	/**
	 * Another player's unit a right click there takes as its target (J7c, retour d'essai 8, loop 3): the one drawn under
	 * the point (bOutDirect true), or else the one drawn nearest to it within hol.Select.TargetMargin (pixels at 1080
	 * high, HOLUnitPicking::PickNear); 0 when none.
	 */
	uint32 FindTargetAt(const FVector2D& ViewportFraction, bool& bOutDirect) const;
	/** The standing resource source drawn under a point of the viewport, the one in front (J5, D298), or INDEX_NONE. */
	int32 FindSourceAt(const FVector2D& ViewportFraction) const;
	/**
	 * A left click at a point of the viewport: the unit under it, else the building, else the source, else nothing. With bAdd (Shift), the
	 * first side of the selection and the unit wins (D.2, HOLUnitPicking::CombineClick): of the selection's side, the unit is added, or
	 * removed when it already is one; an enemy never joins the player's own units; the selection stays when nothing is under the cursor.
	 */
	void SelectAt(const FVector2D& ViewportFraction, bool bAdd = false);
	/**
	 * A double click or Ctrl + click there (A.1): on a unit, all the units of its type and of its owner on the screen
	 * (HOLUnitPicking::PickSameTypeOnScreen; D.2: an enemy's too), added with bAdd to the selection of their side (the first
	 * side of the whole kept); elsewhere, as SelectAt.
	 */
	void SelectSameTypeAt(const FVector2D& ViewportFraction, bool bAdd = false);
	/**
	 * A box between two points of the viewport: the units in it of the first side present, own > ally > enemy > neutral
	 * (D.1: an enemy group too, its panel without commands); with bAdd, the first side of the selection and the box wins,
	 * of the same side added, or removed when all already are (HOLUnitPicking::CombineBox).
	 */
	void SelectInBox(const FVector2D& CornerA, const FVector2D& CornerB, bool bAdd = false);
	/**
	 * A right click at a point of the viewport: the selected own units gather the source under it (J5, D298), build the
	 * player's site under it (J6a.4) or go there; or the selected own building's rally point, on the source or the site
	 * under it if there is one. With Shift held (or bQueue, scripts), a site or a source goes at the end of the units'
	 * queue (the user's trial of J6a.4).
	 */
	void OrderAt(const FVector2D& ViewportFraction, bool bQueue = false);
	/**
	 * A right-button drag from Start to End (viewport fractions; J7a.4, J7-Q3): the selected own units, two or more, go
	 * in formation with their front on that line (its middle the target, facing away from the group; the user's 1st
	 * trial: the default width, or with Ctrl, bWidth, the line's length); Shift (bQueue): after their current move. A
	 * press on a source or one of the player's sites, a single unit or a building's selection: a right click at Start
	 * (OrderAt).
	 */
	void OrderLineAt(const FVector2D& Start, const FVector2D& End, bool bWidth = false, bool bQueue = false);
	/** Ctrl held (the user's 1st trial of J7a.4): a right-button drag sets the front's width too. */
	bool IsOrderWidthHeld() const;
	/**
	 * The right button held and dragged past a few pixels with two or more own units selected (J7a.4): the front line
	 * on the ground, for its preview (the local player only).
	 */
	bool GetOrderLine(FVector& OutStart, FVector& OutEnd) const;
	/** Selects a building, or nothing (null). */
	void SetSelection(AHOLBuilding* Building);
	/**
	 * Selects several of the player's own buildings (an F key, D30): one alone is selected as SetSelection does; several
	 * make a group, GetSelection() then null.
	 */
	void SetBuildingSelection(TArray<AHOLBuilding*> Buildings);
	/** The selected buildings: the one of GetSelection(), or a group's (the player's own), or none. */
	void GetSelectedBuildings(TArray<AHOLBuilding*>& OutBuildings) const;
	/** Several of the player's buildings are selected together (an F key). */
	bool HasBuildingGroup() const { return !SelectedBuildings.IsEmpty(); }
	/**
	 * A group's subgroups (the user's 2nd trial of J6a.5, HOLCommandGrid::SplitSubgroups): « Tous », then one per type in
	 * the group's order, each main centre alone; indices in GetSelectedBuildings' order. Empty without a group.
	 */
	void GetSubgroups(TArray<FHOLSubgroup>& OutSubgroups) const;
	/** The subgroup Tab made active (0: « Tous », a new group's), whose cells the grid shows; false without a group. */
	bool GetActiveSubgroup(FHOLSubgroup& OutSubgroup) const;
	/**
	 * Tab (AoE4's way): the next subgroup of the group, back to « Tous » after the last one; the same through a selection
	 * of units of several types (GetUnitSubgroups); nothing else.
	 */
	void CycleSubgroup();
	/**
	 * The subgroups of the player's own selected units (the user's trial of J7b), when they are of several types: « Tous »,
	 * then one per type, the soldiers first, the Peasants last (each by name); OutUnits (optional): their ids, the indices
	 * of the subgroups' members. Empty for one type, or no own unit.
	 */
	void GetUnitSubgroups(TArray<FHOLSubgroup>& OutSubgroups, TArray<uint32>* OutUnits = nullptr) const;
	/** The units' subgroup Tab made active (« Tous » for a new selection); false without unit subgroups. */
	bool GetActiveUnitSubgroup(FHOLSubgroup& OutSubgroup) const;
	/** Whether a building of the group is in its active subgroup (its frame drawn brighter); true outside a group. */
	bool IsInActiveSubgroup(const AHOLBuilding* Building) const;
	/** "All (7)", "House x6", "MainCentre", "Peasant x3": the active subgroup, for the logs; empty without one. */
	FString DescribeSubgroup() const;
	/**
	 * The buildings the grid's commands go to: the player's own building selected alone, or the active subgroup's (the
	 * whole group for « Tous »).
	 */
	void GetCommandBuildings(TArray<AHOLBuilding*>& OutBuildings) const;
	/**
	 * Selects units (their identifiers), or nothing (empty), on the subgroup of type ActiveType (« Tous » by default; a
	 * type none of them has reads as « Tous », GetActiveUnitSubgroup).
	 */
	void SetUnitSelection(TArray<uint32> UnitIds, FName ActiveType = NAME_None);
	/** Selects a standing resource source (its index in UHOLSourceSubsystem), or nothing (INDEX_NONE). */
	void SetSourceSelection(int32 SourceIndex);
	AHOLBuilding* GetSelection() const { return Selection.Get(); }
	const TArray<uint32>& GetSelectedUnits() const { return SelectedUnits; }
	/** The selected resource source, or INDEX_NONE (J5b, D298). */
	int32 GetSelectedSource() const { return SelectedSource; }

	/**
	 * The player's idle workers known here (J5b, D307): its own units that gather, standing with nothing to do, in
	 * increasing identifiers; read again four times a second.
	 */
	const TArray<uint32>& GetIdleWorkers() const { return IdleWorkers; }
	/** The "." key or a click on the HUD's counter (D30): the next idle worker selected alone, the camera above it. */
	void SelectNextIdleWorker();
	/** The box the player is drawing, in viewport fractions; false when none (J4.6 draws it). */
	bool GetSelectionBox(FVector2D& OutMin, FVector2D& OutMax) const;
	/** "MainCentre of slot 3 (own)", "none": for the logs of the scripted checks. */
	FString DescribeBuilding(const AHOLBuilding* Building) const;
	/**
	 * The building as DescribeBuilding, "3 buildings (own): MainCentre x1, House x2", "Peasant #12 of slot 3 (own)", "4 units
	 * (own): Peasant x4", "source 12 GoldVein (2500 left)" or "none".
	 */
	FString DescribeSelection() const;

	/** The selection changed (on this machine only, D262). For the HUD. */
	DECLARE_MULTICAST_DELEGATE(FOnSelectionChanged);
	FOnSelectionChanged OnSelectionChanged;
	/** The player gave an order on the ground (units or rally point): the HUD marks the point briefly (J4.6). */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnOrderGiven, const FVector& /*Point*/);
	FOnOrderGiven OnOrderGiven;
	/** An attack order (J7b.3): the target's place marked as an order's point, in red. */
	FOnOrderGiven OnAttackOrderGiven;

	AHOLPlayerState* GetHOLPlayerState() const;
	/** The selected building if it is the player's own, else null: only the owner gives it orders. */
	AHOLBuilding* GetOwnSelection() const;

	/** Number of cells of the command grid (D30: A Z E R / Q S D F / W X C V). */
	static constexpr int32 NumGridCells = HOLCommandGrid::NumCells;
	/**
	 * Unit type a cell of the grid produces at the selected building (its own), or None: the building's Produces
	 * list, in order, fills the cells.
	 */
	FName GetGridCellUnitType(int32 Cell) const;
	/**
	 * What each cell of the grid does with the selection (D30, D281, D313): the selected building's production, or its
	 * « Annuler » when it is one of the player's sites; a group of buildings, the cells of its active subgroup's buildings
	 * merged (HOLCommandGrid::MergeCommands: in « Tous », a key two of them use goes to the main centre, then to the
	 * group's order; a subgroup of one site alone has its « Annuler »); with builders (Peasants) selected, the buildings
	 * they build, directly on A, Z, E; NumGridCells commands.
	 */
	void GetGridCommands(TArray<FHOLGridCommand>& OutCommands) const;
	/**
	 * The building (of the player's own, finished) that would take a unit of TypeName: the selected one, or, of the
	 * active subgroup, the next one of an even spread (HOLCommandGrid::PickEvenly); null when none produces it or every
	 * queue is full. With bOutAny, whether one of them produces it at all.
	 */
	AHOLBuilding* FindProductionBuilding(FName TypeName, bool* bOutAny = nullptr) const;
	/**
	 * A cell of the grid, by its key or a click on the HUD: asks the server to produce its unit type, starts the
	 * placement of its building (D313), or asks the server to cancel the selected site (D311). What does not go
	 * through (not enough resources, queue full) or waits (population full) is told (OnCommandFeedback).
	 */
	void ActivateGridCell(int32 Cell);
	/**
	 * The orders' panel left of the grid (the user's choice of 2026-10-21, AoE4's places): with own units selected and no
	 * building, Q attack-move (soldiers among them), S stop, V hold; NumGridCells commands.
	 */
	void GetOrderCommands(TArray<FHOLGridCommand>& OutCommands) const;
	/** A cell of the orders' panel, by its key or a click on the HUD. */
	void ActivateOrderCell(int32 Cell);
	/**
	 * A key of the grid (A Z E R / Q S D F / W X C V, HOLCommandGrid::PressKey): to the grid on the right (production),
	 * the builders' age panels (A Z E R choose an age, then all 12 keys start its buildings' ghosts) or the orders' panel.
	 */
	void PressGridKey(int32 Cell);
	/** A click on the builders' panel of Age: its header (Cell INDEX_NONE) or its building at Cell (HOLCommandGrid::ClickAge). */
	void ClickAge(int32 Age, int32 Cell);
	/** The age chosen to the keys (0 to 3; INDEX_NONE: none), kept until its ghost is placed or cancelled. */
	int32 GetChosenAge() const { return ChosenAge; }
	/** The context of the selected units (no building): builders, soldiers, what they build, the age reached (HOLCommandGrid). */
	void GetUnitsContext(FHOLGridContext& OutContext) const;

	/** A command did not go through as asked (D313): the HUD shows a short message above the grid. */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnCommandFeedback, const FHOLCommandFeedback& /*Feedback*/);
	FOnCommandFeedback OnCommandFeedback;

	/**
	 * The F1, F2, F3 keys (D30, the user's trial of J6a.5): all the player's buildings of Category selected (finished,
	 * under construction and its ghosts, HOLCommandGrid::IsSelectedByCategory), in the order of their cycle (the centre,
	 * then by type), « Tous » active, the camera where it is; pressed again while that selection stays, the camera goes to
	 * the next one (HOLCommandGrid::NextInCycle), the active subgroup kept. None: a short message.
	 */
	void SelectBuildingCategory(EHOLBuildingCategory Category);

	/**
	 * Control groups (D30, J7a.5), 1 to 9 and 0, kept on this machine only, as the selection they come from (D262): the
	 * server needs none, every order still names its units. A group holds the player's own units (identifiers, D223) or
	 * its own buildings, one kind at a time, as the selection; dead units and buildings gone drop out when it is read.
	 */
	static constexpr int32 NumControlGroups = 10;
	/**
	 * The digit: the group replaces the selection; pressed again within ControlGroupDoublePress seconds, the camera goes
	 * to its centroid as well (F1's camera move). An empty group changes nothing.
	 */
	void SelectControlGroup(int32 Group);
	/**
	 * Shift + digit: the group made of the selection's own units (or buildings) when empty, otherwise the selection added
	 * to it; another kind than the group's replaces it. Ctrl + digit alone does nothing (the user's trial of J7a.5).
	 */
	void AddToControlGroup(int32 Group);
	/** Ctrl + Shift + digit: the selection's own units and buildings leave the group. */
	void RemoveFromControlGroup(int32 Group);
	/** The group's members still here: units (identifiers) and buildings; false when it is empty. */
	bool GetControlGroup(int32 Group, TArray<uint32>& OutUnits, TArray<AHOLBuilding*>& OutBuildings) const;
	/** Ctrl + A (D30): all the player's soldiers known here (every unit type but the Peasant), no building. */
	void SelectArmy();
	/** Two presses of a group's key this close (s, real time): the camera goes to the group. */
	static constexpr double ControlGroupDoublePress = 0.3;
	/** The group last selected (its key, a click on its slot or the command), INDEX_NONE before any, and when (real time). */
	int32 GetLastSelectedGroup(double& OutTime) const { OutTime = LastGroupSelectTime; return LastSelectedGroup; }

	/**
	 * Placement mode (J6a.3, D311) for a type the builders build (the Peasant's grid of J6a.5 starts it; a development
	 * command meanwhile): the ghost follows the cursor until a click places it or Escape / a right click ends it.
	 */
	void BeginPlacement(FName TypeName);
	void EndPlacement();
	bool IsPlacing() const { return !PlacementType.IsNone(); }
	FName GetPlacementType() const { return PlacementType; }
	/**
	 * A left click in placement mode: places the ghost's building if it is green, staying in the mode with bKeep (Shift,
	 * several foundations in a row) or leaving it; nothing on a red ghost.
	 */
	void ClickPlacement(bool bKeep);
	/** The ghost to draw: its type, footprint, ground height and colour; false when none is shown. */
	bool GetPlacementGhost(FName& OutType, FBox2D& OutFootprint, double& OutGroundZ, bool& bOutValid) const;
	/**
	 * Whether this machine sees a foundation of TypeName on Footprint as possible (the ghost's colour): the rules of
	 * UHOLConstructionSubsystem::CheckPlacement, then the stock; OutReason says why not ("" when possible).
	 */
	bool CheckPlacementHere(FName TypeName, const FBox2D& Footprint, FString& OutReason, FHOLCommandFeedback* OutFeedback = nullptr) const;
	/**
	 * Asks the server to place a foundation of TypeName around a ground point (snapped to the tiles); it checks everything
	 * again. The player's selected units are sent to build it (J6a.4), at the end of their queue with bQueue (Shift).
	 */
	void RequestPlaceBuilding(FName TypeName, const FVector& Point, bool bQueue = false);
	/** Asks the server to have units build (help) one of the player's sites not finished (J6a.4, D312); bQueue: Shift. */
	void RequestBuildUnits(TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, bool bQueue = false);
	/** Asks the server to cancel one of the player's buildings not yet finished (refunded, D311). */
	void RequestCancelBuilding(AHOLBuilding* Building);

	/** Asks the server to queue one unit of TypeName at the selected building (its own). */
	void RequestProduction(FName TypeName);
	/** Asks the server to cancel the item at Index of the selected building's queue (its own). */
	void RequestCancelProduction(int32 Index);
	/**
	 * Asks the server to move units to a ground point, as one group (J4, D288); it moves only the player's own. bQueue
	 * (Shift, the user's 1st trial of J7a.4): after their current move.
	 */
	void RequestMoveUnits(TConstArrayView<uint32> UnitIds, const FVector& Target, bool bQueue = false);
	/** Asks the server to have units attack an enemy unit (J7b.3, a right click on it); it checks the units and the target. */
	void RequestAttackUnit(TConstArrayView<uint32> UnitIds, uint32 TargetUnitId, bool bQueue = false);
	/**
	 * Asks the server to have units strike an enemy building (D345.2b, a right click on it); it checks the units and the building.
	 * bQueue (Shift, D345): at the end of each unit's queue, as an attack on a unit.
	 */
	void RequestAttackBuilding(TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, bool bQueue = false);
	/** Asks the server for an attack-move of units to a ground point (J7b.3), as one group. */
	void RequestAttackMove(TConstArrayView<uint32> UnitIds, const FVector& Target, bool bQueue = false);
	/** Asks the server to stop units (J7b.3), or to stop them holding their position (bHold). */
	void RequestStopUnits(TConstArrayView<uint32> UnitIds, bool bHold);
	/**
	 * Attack-move mode (J7b.3, the grid's A): the next left click on the ground sends the selected soldiers there (Shift:
	 * queued); a right click or Escape leaves it.
	 */
	bool IsAttackMoveArmed() const { return bAttackMoveArmed; }
	/** The attack-move of the mode at a viewport point (a left click, or hol.Select.AttackMove). */
	void AttackMoveAt(const FVector2D& ViewportFraction, bool bQueue);
	/** The selected units that are the player's own (an order moves only them). */
	TArray<uint32> GetOwnSelectedUnits() const;
	/** The player's own soldiers among the selection (every type but those that gather). */
	TArray<uint32> GetOwnSelectedSoldiers() const;
	/**
	 * The player's own selected units the grid's commands go to (the user's trial of J7b): those of the active unit
	 * subgroup (Tab), all of them for « Tous » or a selection of one type.
	 */
	TArray<uint32> GetCommandUnits() const;
	/**
	 * Asks the server to have units gather a resource source (J5, D298); it sends only the player's own. bQueue (Shift): at
	 * the end of their queue, after their sites (the user's trial of J6a.4).
	 */
	void RequestGatherUnits(TConstArrayView<uint32> UnitIds, int32 SourceIndex, bool bQueue = false);
	/**
	 * Asks the server to put the selected building's rally point (own) at a ground point, on a resource source (J5) when
	 * SourceIndex is one, on one of the player's sites not finished when Site is one (the units born there build it, the
	 * user's trial of J6a.4), or to clear it (on the building).
	 */
	void RequestRallyPoint(const FVector& Point, int32 SourceIndex = INDEX_NONE, AHOLBuilding* Site = nullptr);

	/**
	 * Development cheats for the trials (hol.Cheat.*; never in a Shipping build: no command registers them and the
	 * server refuses them). Resources added to the player's stock (never below 0), instant production for every
	 * player, places added to the player's population cap (still 175 at most, D03), every unit walking Scale times as
	 * fast (hol.Units.SpeedScale, for every player), every gatherer gathering Scale times as fast (hol.Gather.SpeedScale,
	 * for every player).
	 */
	void RequestCheatResources(const FHOLResources& Amount);
	void RequestCheatFastBuild(bool bOn);
	void RequestCheatPopulation(int32 Places);
	void RequestCheatUnitSpeed(float Scale);
	void RequestCheatGatherSpeed(float Scale);
	/**
	 * D345.2a, until the units attack buildings: Amount of damage of AttackType (EHOLAttackType as a byte) on the building
	 * nearest Point on the server (any player's), its armour taken off; destroyed at 0 (hol.Cheat.DamageNear, DamageSelected).
	 */
	void RequestCheatDamageBuilding(const FVector& Point, int32 Amount, uint8 AttackType);
	/** D345.2b, scenarios: a finished building of TypeName for Slot, on the tile grid around Point (hol.Cheat.SpawnBuilding). */
	void RequestCheatSpawnBuilding(FName TypeName, int32 Slot, const FVector& Point);
	/** V1.1: any slot loses at once, an AI's too (hol.Victory.Defeat), as a surrender would but with the reason Cheat. */
	void RequestCheatDefeat(int32 Slot);

	/**
	 * D356: the player gives up, an immediate defeat (hol.Victory.Surrender until the in-game menu of V1.3). The server
	 * defeats the player's own slot only, not already defeated, the match not over.
	 */
	void RequestSurrender();

	/** D356: Escape with nothing to undo asks the interface to open the in-game menu (on this machine only). */
	DECLARE_MULTICAST_DELEGATE(FOnGameMenuRequested);
	FOnGameMenuRequested OnGameMenuRequested;
	/** The interface says when its in-game menu opens or closes (D356: Escape then closes it). */
	void SetGameMenuOpen(bool bOpen) { bGameMenuOpen = bOpen; }
	bool IsGameMenuOpen() const { return bGameMenuOpen; }
	/**
	 * D357: the player's slot is defeated, it only watches: camera and selection kept (the units' details), no order,
	 * no grid, no placement sent from this machine (the server refuses them anyway, V1.2).
	 */
	bool IsSpectator() const;
	/** Units, a building, a group of buildings or a source selected. */
	bool HasSelection() const;

	/** A « +10 » shown on this machine (J5c.3): the unload, when it arrived, and its line over others at the same place. */
	struct FShownDepositPopup
	{
		FHOLDepositPopup Popup;
		float BornSeconds = 0.f;
		int32 Stack = 0;
	};
	/** Seconds a « +10 » shows (it rises, then fades), and how many show at once at most (the oldest go first). */
	static constexpr float DepositPopupSeconds = 1.2f;
	static constexpr int32 MaxDepositPopups = 30;
	/** The « +10 » of the player's own unloads still showing, oldest first; the HUD draws them. */
	TConstArrayView<FShownDepositPopup> GetDepositPopups() const { return DepositPopups; }

	/**
	 * From the server (or the simulation, in solo): the player's unloads of one step (J5c.3, D298), to this player alone
	 * (a client call reaches the owning connection only). Unreliable: a « +10 » lost is not missed.
	 */
	UFUNCTION(Client, Unreliable)
	void ClientShowDepositPopups(const TArray<FHOLDepositPopup>& Popups);

	/** Swaps the engine's default HUD for the match HUD of the UI module (D281), on the player's machine. */
	virtual void ClientSetHUD_Implementation(TSubclassOf<AHUD> NewHUDClass) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void Tick(float DeltaSeconds) override;

private:
	/** Places the camera above the player's start slot, once its slot has reached this machine. */
	void TryPlaceCamera();
	/** Cursor position in the viewport, 0 to 1 each way; the test cursor (hol.Camera.TestCursor) when set. */
	bool GetCursorFraction(FVector2D& OutFraction) const;
	FVector2D GetEdgeScroll() const;
	void UpdateDrag();
	/** Pixels of a point of the viewport given in fractions. */
	FVector2D ToPixels(const FVector2D& ViewportFraction) const;
	/**
	 * Units known here as drawn on the screen (D288); with bOwnOnly, the player's own only; with bOthersOnly, the other
	 * players' only. Ids match the candidates.
	 */
	void GatherPickCandidates(bool bOwnOnly, TArray<FHOLPickCandidate>& OutCandidates, TArray<uint32>& OutUnitIds, bool bOthersOnly = false) const;
	/**
	 * Every unit known here by its side for the local player (D.1), and the selected units known here with their first
	 * side (OutSelectedSide untouched when none is).
	 */
	void GatherSelectionSides(TMap<uint32, EHOLSelectionSide>& OutSideOf, TArray<uint32>& OutSelected, EHOLSelectionSide& OutSelectedSide) const;
	/** Units that left this machine (dead, out of view of a client) leave the selection, a used-up source too. */
	void PurgeSelection(TConstArrayView<FHOLUnitView> Views);
	/** The player's idle workers among the units known here (Views). */
	void UpdateIdleWorkers(TConstArrayView<FHOLUnitView> Views);
	void BroadcastSelection();
	/** On a client: tells the server which unit the player watches (its own unit selected alone, D303), when it changes. */
	void UpdateWatchedUnit();
	/** On the server: the watched unit's load, sent to the owner when it changed. */
	void SendWatchedLoad();
	UFUNCTION()
	void OnRep_WatchedLoad();

	void HandlePan(const FInputActionValue& Value, FVector2D Direction);
	void HandleZoom(const FInputActionValue& Value);
	void HandleDragStarted();
	void HandleDragCompleted();
	void HandleSelectStarted();
	void HandleSelectCompleted();
	void HandleSelectAdd(bool bHeld);
	void HandleOrder();
	void HandleOrderCompleted();
	void HandleDeselect();
	void HandleNextIdle();
	/** Enter: a test scenario waiting on a waitkey step goes on (HOLTestHooks::PressContinueKey). */
	void HandleContinueKey();
	void HandleCategoryKey(EHOLBuildingCategory Category);
	void HandleSubgroupKey();
	/** The group's buildings as the pure rules read them (their type, a main centre alone). */
	void GetGroupMembers(TConstArrayView<AHOLBuilding*> Group, TArray<FHOLGroupMember>& OutMembers) const;
	/** After the group changed (a building gone, an F key again): the subgroup of that building active again, else « Tous ». */
	void RestoreSubgroup(uint32 FirstKey);
	/** Logs and broadcasts a command's feedback (OnCommandFeedback). */
	void NotifyFeedback(const FHOLCommandFeedback& Feedback);
	void HandleGridKey(const FInputActionValue& Value, int32 Cell);
	/** A control group's digit key (IA_Group_N): Ctrl, Shift or both read here, never the character the layout gives. */
	void HandleGroupKey(const FInputActionValue& Value, int32 Group);
	/** The selection's own units and own buildings (the group keys' source). */
	void GetOwnSelectionForGroup(TArray<uint32>& OutUnits, TArray<AHOLBuilding*>& OutBuildings) const;
	bool IsCtrlHeld() const;
	bool IsShiftHeld() const;
	/** The ghost under the cursor, its colour checked again (placement mode, every frame). */
	void UpdatePlacementGhost();

	UFUNCTION(Server, Reliable)
	void ServerQueueProduction(AHOLBuilding* Building, FName TypeName);

	UFUNCTION(Server, Reliable)
	void ServerCancelProduction(AHOLBuilding* Building, int32 Index);

	/** The units' identifiers (D223) and the ground point: ~4 bytes a unit, a player's selection at most (D288). */
	UFUNCTION(Server, Reliable)
	void ServerMoveUnits(const TArray<uint32>& UnitIds, FVector_NetQuantize Target, bool bQueue);

	/** The combat orders (J7b.3): attack a unit, attack-move, stop or hold (EHOLOrderKind as a byte); the server checks them. */
	UFUNCTION(Server, Reliable)
	void ServerAttackUnit(const TArray<uint32>& UnitIds, uint32 TargetUnitId, bool bQueue);
	UFUNCTION(Server, Reliable)
	void ServerAttackBuilding(const TArray<uint32>& UnitIds, AHOLBuilding* Building, bool bQueue);
	UFUNCTION(Server, Reliable)
	void ServerAttackMove(const TArray<uint32>& UnitIds, FVector_NetQuantize Target, bool bQueue);
	UFUNCTION(Server, Reliable)
	void ServerStopUnits(const TArray<uint32>& UnitIds, bool bHold);

	/**
	 * A right-button drag (J7a.4): the units and the two ends of the front line on the ground (~4 bytes a unit and two
	 * points); the server makes the formation from them (facing; width with bWidth, Ctrl), as for ServerMoveUnits.
	 */
	UFUNCTION(Server, Reliable)
	void ServerMoveUnitsInLine(const TArray<uint32>& UnitIds, FVector_NetQuantize Start, FVector_NetQuantize End, bool bWidth, bool bQueue);

	/** The units' identifiers and the source's index (J5, D298); Shift (the user's trial of J6a.4). */
	UFUNCTION(Server, Reliable)
	void ServerGatherUnits(const TArray<uint32>& UnitIds, int32 SourceIndex, bool bQueue);

	/** A type and a ground point (J6a.3, D311): ~16 bytes a foundation; the units sent to build it (J6a.4), Shift. */
	UFUNCTION(Server, Reliable)
	void ServerPlaceBuilding(FName TypeName, FVector_NetQuantize Point, const TArray<uint32>& UnitIds, bool bQueue);

	/** The units' identifiers and the site they build (J6a.4, D312). */
	UFUNCTION(Server, Reliable)
	void ServerBuildUnits(const TArray<uint32>& UnitIds, AHOLBuilding* Building, bool bQueue);

	UFUNCTION(Server, Reliable)
	void ServerCancelBuilding(AHOLBuilding* Building);

	/** The building, the ground point, the source there and the site there (the user's trial of J6a.4), both checked. */
	UFUNCTION(Server, Reliable)
	void ServerSetRallyPoint(AHOLBuilding* Building, FVector_NetQuantize Point, int32 SourceIndex, AHOLBuilding* Site);

	/** The player's own unit selected alone (D303), or 0: the server sends its load at every change. */
	UFUNCTION(Server, Reliable)
	void ServerWatchUnit(uint32 UnitId);

	UFUNCTION(Server, Reliable)
	void ServerCheatResources(FHOLResources Amount);

	UFUNCTION(Server, Reliable)
	void ServerCheatFastBuild(bool bOn);

	UFUNCTION(Server, Reliable)
	void ServerCheatPopulation(int32 Places);

	UFUNCTION(Server, Reliable)
	void ServerCheatUnitSpeed(float Scale);

	UFUNCTION(Server, Reliable)
	void ServerCheatGatherSpeed(float Scale);

	UFUNCTION(Server, Reliable)
	void ServerCheatDamageBuilding(FVector_NetQuantize Point, int32 Amount, uint8 AttackType);

	UFUNCTION(Server, Reliable)
	void ServerCheatSpawnBuilding(FName TypeName, int32 Slot, FVector_NetQuantize Point);

	UFUNCTION(Server, Reliable)
	void ServerCheatDefeat(int32 Slot);

	UFUNCTION(Server, Reliable)
	void ServerSurrender();

	UPROPERTY(Transient)
	TObjectPtr<AHOLRTSCamera> RTSCamera;

	/** What the ground shows of the selection (J4.6), local only like the camera. */
	UPROPERTY(Transient)
	TObjectPtr<AHOLSelectionMarkers> SelectionMarkers;

	/** Sum of the held arrow keys this frame. */
	FVector2D KeyPan = FVector2D::ZeroVector;
	bool bCameraPlaced = false;
	bool bDragging = false;
	/** Ground point under the cursor when the drag began: the drag keeps it under the cursor. */
	FVector DragAnchor = FVector::ZeroVector;

	TWeakObjectPtr<AHOLBuilding> Selection;
	/** Selected units (D223 identifiers), never with a building: several of the player's own, or one of anyone's. */
	TArray<uint32> SelectedUnits;
	/** Selected resource source, never with a building or units; INDEX_NONE: none. */
	int32 SelectedSource = INDEX_NONE;
	/** The interface's in-game menu is open (D356). */
	bool bGameMenuOpen = false;
	TArray<uint32> IdleWorkers;
	/** The idle worker the last "." selected: the next press goes on from it. */
	uint32 LastIdleWorker = 0;
	/** Several of the player's buildings selected together (an F key), never with Selection, units or a source: 2 at least. */
	TArray<TWeakObjectPtr<AHOLBuilding>> SelectedBuildings;
	/**
	 * The last F key (D30): its category, the buildings it selected (unique ids on this machine) and the one the camera
	 * went to last; a press again while they are still the selection goes on from it.
	 */
	TOptional<EHOLBuildingCategory> LastCategory;
	TArray<uint32> LastCategorySelection;
	uint32 LastCategoryBuilding = 0;
	/** The group's active subgroup (Tab), an index of GetSubgroups: 0, « Tous », for a new group. */
	int32 ActiveSubgroup = 0;
	/** The units' active subgroup (Tab, the user's trial of J7b): its type, None for « Tous »; kept while units of it are. */
	FName ActiveUnitType;
	/** By unit type, the building of a group that took the last one (unique id here): the next press goes on from it. */
	TMap<FName, uint32> LastProductionBuilding;
	/** Left button held: where it was pressed (viewport fractions); its release clicks or ends the box. */
	bool bSelectPressed = false;
	FVector2D SelectPressFraction = FVector2D::ZeroVector;
	/** When it was pressed (real seconds); the last single click's press and pixels, the first of a double click (A.1). */
	double SelectPressTime = -1.;
	double LastClickTime = -1.;
	FVector2D LastClickPixels = FVector2D::ZeroVector;
	/** Right button held (J7a.4): where it was pressed; its release orders there, or along the drag. */
	bool bOrderPressed = false;
	/** Attack-move mode (J7b.3): the next left click on the ground gives the order. */
	bool bAttackMoveArmed = false;
	/** The builders' age chosen to the keys (the user's 1st trial of G.1; INDEX_NONE: none). */
	int32 ChosenAge = INDEX_NONE;
	void SetChosenAge(int32 Age);
	/** Does what a key or a click of the builders' panels gives (HOLCommandGrid::PressKey, ClickAge), the chosen age after it. */
	void ApplyAgeKey(const FHOLGridKeyResult& Result, int32 Cell);
	/** Does a command of the grid or of the orders' panel. */
	void ExecuteGridCommand(const FHOLGridCommand& Command);
	FVector2D OrderPressFraction = FVector2D::ZeroVector;
	/** Shift held (IA_SelectAdd). */
	bool bSelectAddHeld = false;
	float SincePurge = 0.f;

	/** The unit this client last asked the server to watch (D303). */
	uint32 SentWatchedUnitId = 0;
	/** On the server: the unit the player watches. */
	uint32 WatchedUnitId = 0;
	/** Replicated to the owner alone. */
	UPROPERTY(ReplicatedUsing = OnRep_WatchedLoad)
	FHOLWatchedLoad WatchedLoad;

	/** The « +10 » showing (J5c.3), MaxDepositPopups at most. */
	TArray<FShownDepositPopup> DepositPopups;

	/** A control group (J7a.5): the player's own units, or its own buildings, never both. */
	struct FControlGroup
	{
		TArray<uint32> Units;
		TArray<TWeakObjectPtr<AHOLBuilding>> Buildings;
	};
	FControlGroup ControlGroups[NumControlGroups];
	/** The last group selected by its key and when (real time): a press again soon centres the camera. */
	int32 LastGroupPressed = INDEX_NONE;
	double LastGroupPressTime = -1.0;
	/** The group last selected, double press or not (the HUD lights its slot). */
	int32 LastSelectedGroup = INDEX_NONE;
	double LastGroupSelectTime = -1.0;

	/** Placement mode (J6a.3): the type placed, None outside the mode; the ghost under the cursor and its colour. */
	FName PlacementType;
	bool bGhostShown = false;
	bool bGhostValid = false;
	FBox2D GhostFootprint = FBox2D(ForceInit);
	double GhostGroundZ = 0.0;
	FString GhostReason;
	/** ", moved beside the building (ring 1)" when the ghost left the cursor for a free spot (the user's trial of J7b, feedback 3). */
	FString GhostMoved;
	/** Why the ghost is red, as told to the player when it is clicked (D313). */
	FHOLCommandFeedback GhostFeedback;
};
