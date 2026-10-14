// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HOLPlayerController.generated.h"

class AHOLBuilding;
class AHOLPlayerState;
class AHOLRTSCamera;
class UInputAction;
struct FInputActionValue;

/**
 * Controller of a player in a match map (a map with start slots, D255): no pawn, a visible cursor and, on the
 * player's own machine, the RTS camera (D254), placed above the player's start slot once the slot is known. Camera
 * keys come from IMC_Game (Tools/Editor/make_game_input.py); speed and edge scrolling from the Controls tab (D253).
 * Selection (D262) lives here, on the player's machine only: a left click selects the building under the cursor (its
 * own or another player's) or nothing, Escape selects nothing; the HUD shows it (J3, D281).
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

	/** The building under a point of the viewport (0 to 1 each way), on the selection channel, or null. */
	AHOLBuilding* FindBuildingAt(const FVector2D& ViewportFraction) const;
	/** Selects the building under a point of the viewport, or nothing (a left click there). */
	void SelectAt(const FVector2D& ViewportFraction);
	/** Selects a building, or nothing (null). */
	void SetSelection(AHOLBuilding* Building);
	AHOLBuilding* GetSelection() const { return Selection.Get(); }
	/** "MainCentre of slot 3 (own)", "none": for the logs of the scripted checks. */
	FString DescribeBuilding(const AHOLBuilding* Building) const;

	/** The selection changed (on this machine only, D262). For the HUD. */
	DECLARE_MULTICAST_DELEGATE(FOnSelectionChanged);
	FOnSelectionChanged OnSelectionChanged;

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
	/** Asks the server to put the selected building's rally point (own) at a ground point, or to clear it (on the building). */
	void RequestRallyPoint(const FVector& Point);

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

	void HandlePan(const FInputActionValue& Value, FVector2D Direction);
	void HandleZoom(const FInputActionValue& Value);
	void HandleDragStarted();
	void HandleDragCompleted();
	void HandleSelect();
	void HandleDeselect();
	void HandleGridKey(const FInputActionValue& Value, int32 Cell);

	UFUNCTION(Server, Reliable)
	void ServerQueueProduction(AHOLBuilding* Building, FName TypeName);

	UFUNCTION(Server, Reliable)
	void ServerCancelProduction(AHOLBuilding* Building, int32 Index);

	/** The units' identifiers (D223) and the ground point: ~4 bytes a unit, a player's selection at most (D288). */
	UFUNCTION(Server, Reliable)
	void ServerMoveUnits(const TArray<uint32>& UnitIds, FVector_NetQuantize Target);

	UFUNCTION(Server, Reliable)
	void ServerSetRallyPoint(AHOLBuilding* Building, FVector_NetQuantize Point);

	UPROPERTY(Transient)
	TObjectPtr<AHOLRTSCamera> RTSCamera;

	/** Sum of the held arrow keys this frame. */
	FVector2D KeyPan = FVector2D::ZeroVector;
	bool bCameraPlaced = false;
	bool bDragging = false;
	/** Ground point under the cursor when the drag began: the drag keeps it under the cursor. */
	FVector DragAnchor = FVector::ZeroVector;

	TWeakObjectPtr<AHOLBuilding> Selection;
};
