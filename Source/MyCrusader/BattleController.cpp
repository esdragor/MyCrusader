// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "UnitBase.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/World.h"
#include "GameFramework/SpringArmComponent.h"
#include "AI/NavigationSystemBase.h"
#include "SelectionUnitHUD.h"




ABattleController::ABattleController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void ABattleController::BeginPlay()
{
	Super::BeginPlay();

	HUDSelection = Cast<ASelectionUnitHUD>(GetHUD());
}

void ABattleController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Blue, FString::Printf(TEXT("%i"), HUDSelection->UnitSelection.Num()));


	// keep updating the destination every tick while desired
	if (bMoveToMouseCursor)
	{
		MoveToMouseCursor();
	}
}

void ABattleController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("SetDestination", IE_Pressed, this, &ABattleController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &ABattleController::OnSetDestinationReleased);

	InputComponent->BindAction("ZoomIn", IE_Pressed, this, &ABattleController::ZoomIn);
	InputComponent->BindAction("ZoomOut", IE_Pressed, this, &ABattleController::ZoomOut);


	InputComponent->BindAction("LeftMouse", IE_Pressed, this, &ABattleController::SelectUnitsBegin);
	InputComponent->BindAction("LeftMouse", IE_Released, this, &ABattleController::SelectUnitsEnd);
	InputComponent->BindAction("RightMouse", IE_Released, this, &ABattleController::MoveUnitsSelected);
}

void ABattleController::MoveToMouseCursor()
{

}

void ABattleController::MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	FVector2D ScreenSpaceLocation(Location);

	// Trace to see what is under the touch location
	FHitResult HitResult;
	GetHitResultAtScreenPosition(ScreenSpaceLocation, CurrentClickTraceChannel, true, HitResult);
	if (HitResult.bBlockingHit)
	{
		// We hit something, move there
		SetNewMoveDestination(HitResult.ImpactPoint);
	}
}

void ABattleController::SetNewMoveDestination(const FVector DestLocation)
{

}

void ABattleController::OnSetDestinationPressed()
{
	// set flag to keep updating destination until released
	bMoveToMouseCursor = true;
}

void ABattleController::OnSetDestinationReleased()
{
	// clear flag to indicate we should stop updating the destination
	bMoveToMouseCursor = false;
}


void ABattleController::ZoomIn()
{

	GetPawn()->FindComponentByClass<USpringArmComponent>()->TargetArmLength -= SpeedZoom;
}

void ABattleController::ZoomOut()
{

	GetPawn()->FindComponentByClass<USpringArmComponent>()->TargetArmLength += SpeedZoom;
}

void ABattleController::SelectUnitsBegin()
{
	HUDSelection->GetUnitsDeselected();
	HUDSelection->StartSelectionSquare = HUDSelection->GetMousePosOnScreen();
	HUDSelection->onSelection = true;
}

void ABattleController::SelectUnitsEnd()
{
	if (!HUDSelection)
		HUDSelection = Cast<ASelectionUnitHUD>(GetHUD());
	UnitsSelected = HUDSelection->UnitSelection;

	HUDSelection->onSelection = false;
}

void ABattleController::GetUnitsDeselected()
{
	HUDSelection->GetUnitsDeselected();
}

void ABattleController::FUnitsSelected()
{
	HUDSelection->UnitsSelected();

}

void ABattleController::MoveUnitsSelected()
{
	SelectUnitsEnd();
	for (int i = 0; i < UnitsSelected.Num(); i++)
	{
		FHitResult hit;

		GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, hit);
		FVector TargetLocation = hit.Location;
		TargetLocation += FVector(i % 2 * 100, i / 2 * 100, 0); //basic formation
		if (UnitsSelected[i])
		{
			AController *controller = UnitsSelected[i]->GetController();
			if (controller)
			{
				//UAIBlueprintHelperLibrary::SimpleMoveToLocation(controller, TargetLocation);
				MoveToL(controller, TargetLocation);
				//UAIBlueprintHelperLibrary::SimpleMoveToLocation(controller, TargetLocation);
			}

		}
	}
}

