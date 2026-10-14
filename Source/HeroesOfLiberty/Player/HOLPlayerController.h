// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HOLPlayerController.generated.h"

class AHOLBuilding;
class AHOLRTSCamera;
class UInputAction;
struct FInputActionValue;

/**
 * Controller of a player in a match map (a map with start slots, D255): no pawn, a visible cursor and, on the
 * player's own machine, the RTS camera (D254), placed above the player's start slot once the slot is known. Camera
 * keys come from IMC_Game (Tools/Editor/make_game_input.py); speed and edge scrolling from the Controls tab (D253).
 * Selection (D262) lives here, on the player's machine only: a left click selects the building under the cursor (its
 * own or another player's) or nothing, Escape selects nothing; nothing shows it yet (the panel comes in J3).
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
