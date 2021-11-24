// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SelectionUnitHUD.h"
#include "UnitBase.h"
#include "BattleController.generated.h"
/**
 * 
 */
UCLASS()
class MYCRUSADER_API ABattleController : public APlayerController
{
	GENERATED_BODY()

public:
	ABattleController();
	void SelectUnitsEnd();

protected:
	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;
	uint32 SpeedZoom = 90;

	ASelectionUnitHUD *HUDSelection;

	TArray<AUnitBase *> UnitsSelected;

	// Begin PlayerController interface
	void BeginPlay();
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

	// End PlayerController interface

	/** Navigate player to the current mouse cursor location. */
	void MoveToMouseCursor();

	/** Navigate player to the current touch location. */
	void MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Navigate player to the given world location. */
	void SetNewMoveDestination(const FVector DestLocation);

	/** Input handlers for SetDestination action. */
	void OnSetDestinationPressed();
	void OnSetDestinationReleased();

	void ZoomIn();
	void ZoomOut();

	void SelectUnitsBegin();
	void MoveUnitsSelected();
	UFUNCTION(BlueprintImplementableEvent)
		bool MoveToL(AController *controller, FVector TargetLocation);
};
