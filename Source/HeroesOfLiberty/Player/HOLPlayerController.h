// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Match/HOLResources.h"
#include "HOLPlayerController.generated.h"

class AHOLBuilding;
class AHOLPlayerState;
class AHOLRTSCamera;
class AHOLSelectionMarkers;
class UInputAction;
struct FHOLPickCandidate;
struct FInputActionValue;

/**
 * Controller of a player in a match map (a map with start slots, D255): no pawn, a visible cursor and, on the
 * player's own machine, the RTS camera (D254), placed above the player's start slot once the slot is known. Camera
 * keys come from IMC_Game (Tools/Editor/make_game_input.py); speed and edge scrolling from the Controls tab (D253).
 * Selection (D262, D288) lives here, on the player's machine only: units (their identifiers, D223) or one building.
 * A left click selects the unit under the cursor (aimed at on the screen: light units have no collision), else the
 * building under it (its own or another player's), else nothing; a box (a press dragged past a few pixels) selects the
 * player's own units in it; Shift adds or removes; Escape selects nothing; the HUD shows it (J3, D281). A right click
 * sends the selected units of the player to the ground point, or puts the selected building's rally point there (J4).
 * Production (J3, D282): the cells of the command grid (keys of D30, or a click on the HUD) and the HUD's cancel
 * clicks become requests to the server (RPCs), which queues them in the order queue and applies them at its next step.
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
	/** The standing resource source drawn under a point of the viewport, the one in front (J5, D298), or INDEX_NONE. */
	int32 FindSourceAt(const FVector2D& ViewportFraction) const;
	/**
	 * A left click at a point of the viewport: the unit under it, else the building, else nothing. With bAdd (Shift), one
	 * of the player's own units is added to its selected units, or removed when it already is one; the selection stays
	 * when nothing is under the cursor.
	 */
	void SelectAt(const FVector2D& ViewportFraction, bool bAdd = false);
	/** A box between two points of the viewport: the player's own units in it; with bAdd, added, or removed when all already are. */
	void SelectInBox(const FVector2D& CornerA, const FVector2D& CornerB, bool bAdd = false);
	/**
	 * A right click at a point of the viewport: the selected own units gather the source under it (J5, D298) or go there;
	 * or the selected own building's rally point, on the source under it if there is one.
	 */
	void OrderAt(const FVector2D& ViewportFraction);
	/** Selects a building, or nothing (null). */
	void SetSelection(AHOLBuilding* Building);
	/** Selects units (their identifiers), or nothing (empty). */
	void SetUnitSelection(TArray<uint32> UnitIds);
	AHOLBuilding* GetSelection() const { return Selection.Get(); }
	const TArray<uint32>& GetSelectedUnits() const { return SelectedUnits; }
	/** The box the player is drawing, in viewport fractions; false when none (J4.6 draws it). */
	bool GetSelectionBox(FVector2D& OutMin, FVector2D& OutMax) const;
	/** "MainCentre of slot 3 (own)", "none": for the logs of the scripted checks. */
	FString DescribeBuilding(const AHOLBuilding* Building) const;
	/** The building as DescribeBuilding, "Peasant #12 of slot 3 (own)", "4 units (own): Peasant x4" or "none". */
	FString DescribeSelection() const;

	/** The selection changed (on this machine only, D262). For the HUD. */
	DECLARE_MULTICAST_DELEGATE(FOnSelectionChanged);
	FOnSelectionChanged OnSelectionChanged;
	/** The player gave an order on the ground (units or rally point): the HUD marks the point briefly (J4.6). */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnOrderGiven, const FVector& /*Point*/);
	FOnOrderGiven OnOrderGiven;

	AHOLPlayerState* GetHOLPlayerState() const;
	/** The selected building if it is the player's own, else null: only the owner gives it orders. */
	AHOLBuilding* GetOwnSelection() const;

	/** Number of cells of the command grid (D30: A Z E R / Q S D F / W X C V). */
	static constexpr int32 NumGridCells = 12;
	/**
	 * Unit type a cell of the grid produces at the selected building (its own), or None: the building's Produces
	 * list, in order, fills the cells.
	 */
	FName GetGridCellUnitType(int32 Cell) const;
	/** A cell of the grid, by its key or a click on the HUD: asks the server to produce its unit type. */
	void ActivateGridCell(int32 Cell);

	/** Asks the server to queue one unit of TypeName at the selected building (its own). */
	void RequestProduction(FName TypeName);
	/** Asks the server to cancel the item at Index of the selected building's queue (its own). */
	void RequestCancelProduction(int32 Index);
	/** Asks the server to move units to a ground point, as one group (J4, D288); it moves only the player's own. */
	void RequestMoveUnits(TConstArrayView<uint32> UnitIds, const FVector& Target);
	/** Asks the server to have units gather a resource source (J5, D298); it sends only the player's own. */
	void RequestGatherUnits(TConstArrayView<uint32> UnitIds, int32 SourceIndex);
	/**
	 * Asks the server to put the selected building's rally point (own) at a ground point, on a resource source (J5) when
	 * SourceIndex is one, or to clear it (on the building).
	 */
	void RequestRallyPoint(const FVector& Point, int32 SourceIndex = INDEX_NONE);

	/**
	 * Development cheats for the trials (hol.Cheat.*; never in a Shipping build: no command registers them and the
	 * server refuses them). Resources added to the player's stock (never below 0), instant production for every
	 * player, places added to the player's population cap (still 175 at most, D03).
	 */
	void RequestCheatResources(const FHOLResources& Amount);
	void RequestCheatFastBuild(bool bOn);
	void RequestCheatPopulation(int32 Places);

	/** Swaps the engine's default HUD for the match HUD of the UI module (D281), on the player's machine. */
	virtual void ClientSetHUD_Implementation(TSubclassOf<AHUD> NewHUDClass) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

private:
	/** Places the camera above the player's start slot, once its slot has reached this machine. */
	void TryPlaceCamera();
	/** Cursor position in the viewport, 0 to 1 each way; the test cursor (hol.Camera.TestCursor) when set. */
	bool GetCursorFraction(FVector2D& OutFraction) const;
	FVector2D GetEdgeScroll() const;
	void UpdateDrag();
	/** Pixels of a point of the viewport given in fractions. */
	FVector2D ToPixels(const FVector2D& ViewportFraction) const;
	/** Units known here as drawn on the screen (D288); with bOwnOnly, the player's own only. Ids match the candidates. */
	void GatherPickCandidates(bool bOwnOnly, TArray<FHOLPickCandidate>& OutCandidates, TArray<uint32>& OutUnitIds) const;
	/** The selected units that are the player's own (an order moves only them). */
	TArray<uint32> GetOwnSelectedUnits() const;
	/** Units that left this machine (dead, out of view of a client) leave the selection. */
	void PurgeSelectedUnits();
	void BroadcastSelection();

	void HandlePan(const FInputActionValue& Value, FVector2D Direction);
	void HandleZoom(const FInputActionValue& Value);
	void HandleDragStarted();
	void HandleDragCompleted();
	void HandleSelectStarted();
	void HandleSelectCompleted();
	void HandleSelectAdd(bool bHeld);
	void HandleOrder();
	void HandleDeselect();
	void HandleGridKey(const FInputActionValue& Value, int32 Cell);

	UFUNCTION(Server, Reliable)
	void ServerQueueProduction(AHOLBuilding* Building, FName TypeName);

	UFUNCTION(Server, Reliable)
	void ServerCancelProduction(AHOLBuilding* Building, int32 Index);

	/** The units' identifiers (D223) and the ground point: ~4 bytes a unit, a player's selection at most (D288). */
	UFUNCTION(Server, Reliable)
	void ServerMoveUnits(const TArray<uint32>& UnitIds, FVector_NetQuantize Target);

	/** The units' identifiers and the source's index (J5, D298). */
	UFUNCTION(Server, Reliable)
	void ServerGatherUnits(const TArray<uint32>& UnitIds, int32 SourceIndex);

	UFUNCTION(Server, Reliable)
	void ServerSetRallyPoint(AHOLBuilding* Building, FVector_NetQuantize Point, int32 SourceIndex);

	UFUNCTION(Server, Reliable)
	void ServerCheatResources(FHOLResources Amount);

	UFUNCTION(Server, Reliable)
	void ServerCheatFastBuild(bool bOn);

	UFUNCTION(Server, Reliable)
	void ServerCheatPopulation(int32 Places);

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
	/** Left button held: where it was pressed (viewport fractions); its release clicks or ends the box. */
	bool bSelectPressed = false;
	FVector2D SelectPressFraction = FVector2D::ZeroVector;
	/** Shift held (IA_SelectAdd). */
	bool bSelectAddHeld = false;
	float SincePurge = 0.f;
};
