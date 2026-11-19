// Copyright Epic Games, Inc. All Rights Reserved.

#include "HUD/HOLSelectionChipsReadLibrary.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HeroesOfLibertyUILog.h"
#include "HUD/HOLGameNames.h"
#include "Match/HOLFactionRules.h"
#include "Match/HOLPlayerColors.h"
#include "Match/HOLPlayerState.h"
#include "Player/HOLCommandGrid.h"
#include "Player/HOLPlayerController.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

#define LOCTEXT_NAMESPACE "HOLGameScreen"

namespace
{
	/** A chip's name is cut at this many characters (ChipNameChars of the C++ screen). */
	constexpr int32 ChipNameChars = 12;

	/** A chip's label (G.1e): "Paysan (5)", a long name cut short ("Homme d'arme. (3)"); Count 0: the name alone (MakeChipLabel of the C++ screen). */
	FText MakeChipLabel(const FText& Name, const int32 Count)
	{
		const FString Full = Name.ToString();
		const FText Shown = Full.Len() <= ChipNameChars ? Name : FText::AsCultureInvariant(Full.Left(ChipNameChars - 1).TrimEnd() + TEXT("."));
		return Count > 0 ? FText::Format(LOCTEXT("SelectionChip", "{0} ({1})"), Shown, Count) : Shown;
	}

	/** UHOLGameScreen::UnitNameFor: the unit type's name for its owner's faction. */
	FText UnitNameFor(const UWorld* World, const FName TypeName, const int32 OwnerPlayer)
	{
		const FName FactionKey = OwnerPlayer >= 0 ? HOLFactionRules::GetSlotFactionKey(World, OwnerPlayer) : NAME_None;
		const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
		const FHOLUnitTypeRow* Row = Units ? (OwnerPlayer >= 0 ? Units->FindTypeRowFor(TypeName, OwnerPlayer) : Units->FindTypeRow(TypeName)) : nullptr;
		return HOLGameNames::GetUnitName(TypeName, Row, FactionKey);
	}

	FText GetBuildingNameFor(const FName TypeName)
	{
		return HOLGameNames::GetBuildingName(TypeName, UHOLBuildingSettings::FindType(TypeName));
	}

	/** The subgroups of the controller's selection (units, else the group of buildings) and the active one's index (INDEX_NONE for none). */
	void GetSubgroupsOf(const AHOLPlayerController& Controller, TArray<FHOLSubgroup>& OutSubgroups, int32& OutFrom)
	{
		FHOLSubgroup Active;
		const bool bUnits = !Controller.GetSelectedUnits().IsEmpty();
		const bool bFound = bUnits ? Controller.GetActiveUnitSubgroup(Active) : Controller.GetActiveSubgroup(Active);
		if (bUnits)
		{
			Controller.GetUnitSubgroups(OutSubgroups);
		}
		else
		{
			Controller.GetSubgroups(OutSubgroups);
		}
		OutFrom = bFound ? OutSubgroups.IndexOfByKey(Active) : INDEX_NONE;
	}
}

void UHOLSelectionChipsReadLibrary::GetChipLabels(const APlayerController* OwningPlayer, TArray<FText>& OutLabels, int32& OutActive)
{
	OutLabels.Reset();
	OutActive = INDEX_NONE;
	const AHOLPlayerController* Controller = Cast<AHOLPlayerController>(OwningPlayer);
	const UWorld* World = Controller ? Controller->GetWorld() : nullptr;
	if (!Controller || !World)
	{
		return;
	}
	if (!Controller->GetSelectedUnits().IsEmpty())
	{
		// RefreshUnitSelection: the selected units as this machine knows them.
		const TArray<uint32>& UnitIds = Controller->GetSelectedUnits();
		TArray<FHOLUnitView> Views;
		FHOLUnitView One;
		if (UnitIds.Num() != 1)
		{
			HOLUnitView::Gather(*World, Views);
		}
		else if (HOLUnitView::Find(*World, UnitIds[0], One))
		{
			Views.Add(One);
		}
		Views.RemoveAll([&UnitIds](const FHOLUnitView& View) { return !UnitIds.Contains(View.UnitId); });
		if (Views.Num() <= 1)
		{
			return;
		}
		const AHOLPlayerState* PlayerState = Controller->GetPlayerState<AHOLPlayerState>();
		const int32 Owner = Views[0].OwnerPlayer;
		const bool bOwn = PlayerState && PlayerState->GetPlayerIndex() == Owner;
		TMap<FName, int32> PerType;
		TSet<FName> WorkerTypes;
		for (const FHOLUnitView& View : Views)
		{
			++PerType.FindOrAdd(View.TypeName);
			if (View.CarryCapacity > 0)
			{
				WorkerTypes.Add(View.TypeName);
			}
		}
		// Soldiers first, workers (the Peasants) last (J7a.5), each part by name.
		PerType.KeySort([&WorkerTypes](const FName A, const FName B)
		{
			const bool bWorkerA = WorkerTypes.Contains(A);
			return bWorkerA != WorkerTypes.Contains(B) ? !bWorkerA : A.LexicalLess(B);
		});
		FHOLSubgroup Subgroup;
		if (bOwn && Controller->GetActiveUnitSubgroup(Subgroup))
		{
			TArray<FHOLSubgroup> Subgroups;
			Controller->GetUnitSubgroups(Subgroups);
			for (const FHOLSubgroup& Each : Subgroups)
			{
				OutLabels.Add(Each.bAll ? FText::Format(LOCTEXT("SubgroupAll", "Tous ({0})"), Each.Members.Num())
					: MakeChipLabel(UnitNameFor(World, Each.TypeName, Owner), Each.Members.Num()));
			}
			OutActive = Subgroups.IndexOfByKey(Subgroup);
		}
		else
		{
			if (PerType.Num() > 1)
			{
				OutLabels.Add(FText::Format(LOCTEXT("SubgroupAll", "Tous ({0})"), Views.Num()));
			}
			for (const TPair<FName, int32>& Type : PerType)
			{
				OutLabels.Add(MakeChipLabel(UnitNameFor(World, Type.Key, Owner), Type.Value));
			}
			OutActive = bOwn ? 0 : INDEX_NONE;
		}
		return;
	}
	if (Controller->GetSelectedSource() != INDEX_NONE || !Controller->HasBuildingGroup())
	{
		return;
	}
	// RefreshBuildingGroup: the subgroups Tab goes through, the active one lit.
	FHOLSubgroup Subgroup;
	if (Controller->GetActiveSubgroup(Subgroup))
	{
		TArray<FHOLSubgroup> Subgroups;
		Controller->GetSubgroups(Subgroups);
		for (const FHOLSubgroup& Each : Subgroups)
		{
			OutLabels.Add(Each.bAll ? FText::Format(LOCTEXT("SubgroupAll", "Tous ({0})"), Each.Members.Num())
				: MakeChipLabel(GetBuildingNameFor(Each.TypeName), HOLCommandGrid::IsAloneInGroup(Each.TypeName) ? 0 : Each.Members.Num()));
		}
		OutActive = Subgroups.IndexOfByKey(Subgroup);
	}
}

bool UHOLSelectionChipsReadLibrary::GetBuildingGroupView(const APlayerController* OwningPlayer, FString& OutReport, FText& OutOwner, FLinearColor& OutColour,
	FString& OutActivityShown, FText& OutActivityText)
{
	OutReport.Reset();
	OutOwner = FText::GetEmpty();
	OutColour = FLinearColor::White;
	OutActivityShown.Reset();
	OutActivityText = FText::GetEmpty();
	const AHOLPlayerController* Controller = Cast<AHOLPlayerController>(OwningPlayer);
	if (!Controller || !Controller->GetSelectedUnits().IsEmpty() || Controller->GetSelectedSource() != INDEX_NONE || !Controller->HasBuildingGroup())
	{
		return false;
	}
	TArray<AHOLBuilding*> Buildings;
	Controller->GetSelectedBuildings(Buildings);
	if (Buildings.IsEmpty())
	{
		return false;
	}
	// In the group's order (the centre first), its ghosts and sites counted as "en construction".
	TMap<FName, int32> PerType;
	int32 Sites = 0;
	for (const AHOLBuilding* Building : Buildings)
	{
		++PerType.FindOrAdd(Building->GetTypeName());
		Sites += Building->IsFinished() ? 0 : 1;
	}
	TArray<FString> ShownTypes;
	for (const TPair<FName, int32>& Type : PerType)
	{
		ShownTypes.Add(FString::Printf(TEXT("%s x%d"), *Type.Key.ToString(), Type.Value));
	}
	OutReport = FString::Printf(TEXT("%d buildings (own): %s, subgroup %s"), Buildings.Num(), *FString::Join(ShownTypes, TEXT(" ")), *Controller->DescribeSubgroup());
	OutOwner = LOCTEXT("You", "Vous");
	OutColour = FLinearColor(HOLPlayerColors::GetColor(Buildings[0]->GetColorIndex()));
	if (Sites > 0)
	{
		OutActivityShown = FString::Printf(TEXT(", sites %d"), Sites);
	}
	OutActivityText = FText::Format(LOCTEXT("GroupBuilding", "{0} en construction"), Sites);
	return true;
}

bool UHOLSelectionChipsReadLibrary::ClickChip(APlayerController* OwningPlayer, const int32 Chip)
{
	// Tab as many times as it takes from the active subgroup to the chip's (the same path and logs as the key).
	AHOLPlayerController* Controller = Cast<AHOLPlayerController>(OwningPlayer);
	if (!Controller)
	{
		return false;
	}
	TArray<FHOLSubgroup> Subgroups;
	int32 From = INDEX_NONE;
	GetSubgroupsOf(*Controller, Subgroups, From);
	if (From == INDEX_NONE || !Subgroups.IsValidIndex(Chip))
	{
		return false;
	}
	UE_LOG(LogHOLUI, Display, TEXT("HUD: chip %d clicked"), Chip);
	for (int32 Step = (Chip - From + Subgroups.Num()) % Subgroups.Num(); Step > 0; --Step)
	{
		Controller->CycleSubgroup();
	}
	return true;
}

#undef LOCTEXT_NAMESPACE
