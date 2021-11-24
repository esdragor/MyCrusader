// Fill out your copyright notice in the Description page of Project Settings.


#include "SelectionUnitHUD.h"

void ASelectionUnitHUD::DrawHUD()
{
	if (onSelection)
	{
		CurrentSelectionSquare = GetMousePosOnScreen();

		DrawRect(FLinearColor(0.8f, 0.8f, 0.8f, 0.1f),
			StartSelectionSquare.X, StartSelectionSquare.Y,
			CurrentSelectionSquare.X - StartSelectionSquare.X, CurrentSelectionSquare.Y - StartSelectionSquare.Y);
		UnitsSelected();
	}
}

FVector2D ASelectionUnitHUD::GetMousePosOnScreen()
{
	FVector2D pos;

	if (PC == nullptr)
		PC = GetOwningPlayerController();
	PC->GetMousePosition(pos.X, pos.Y);

	return pos;
}


void ASelectionUnitHUD::UnitsSelected()
{
	GetUnitsDeselected();
	GetActorsInSelectionRectangle<AUnitBase>(StartSelectionSquare, CurrentSelectionSquare, TmpSelection, false, false);
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("%i"), UnitSelection.Num()));

	for (int i = 0; i < TmpSelection.Num(); i++)
	{
		//if (TmpSelection[i]->authority)
		//{
		//	SelectSquadron(i);
		//}
		if (!UnitSelection.Contains(TmpSelection[i]))
		{
			UnitSelection.Add(TmpSelection[i]);
			UnitSelection.Last()->SetSelected();
			//TmpSelection[0][i]->SetSelected();
		}

	}
}

void ASelectionUnitHUD::GetUnitsDeselected()
{
	int a = UnitSelection.Num();
	for (int i = 0; i < UnitSelection.Num(); i++)
	{
		if (UnitSelection[i]->getHealthPercent() > 0)
			UnitSelection[i]->SetDeselected(); // you select a unit
	}
	UnitSelection.Empty();
	TmpSelection.Empty();
	a = UnitSelection.Num();
}

//void ASelectionUnitHUD::SelectSquadron(int index)
//{
//	TArray<AUnitBase*> *units = TmpSelection[index]->squadron->GetUnits();
//	for (int i = 0; i < units->Num(); i++)
//	{
//		if (!UnitSelection.Contains(units[0][i]))
//			UnitSelection.Add(units[0][i]);
//		units[0][i]->SetSelected();
//	}
//}