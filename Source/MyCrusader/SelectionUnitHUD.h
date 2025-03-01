// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UnitBase.h"
#include "SelectionUnitHUD.generated.h"

class ABattleController;

/**
 * 
 */
UCLASS()
class MYCRUSADER_API ASelectionUnitHUD : public AHUD
{
	GENERATED_BODY()

public:
	FVector2D StartSelectionSquare;
	FVector2D CurrentSelectionSquare;
	bool onSelection = false;
	ABattleController *BattleCont = nullptr;
	TArray<AUnitBase *> TmpSelection;
	TArray<AUnitBase*> UnitSelection;

	FVector2D GetMousePosOnScreen();
	virtual void DrawHUD() override;
	void UnitsSelected();
	void GetUnitsDeselected();
	//void SelectSquadron(int index);
};
