// Copyright Epic Games, Inc. All Rights Reserved.

#include "HUD/HOLSelectionUnitReadLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/IConsoleManager.h"
#include "HUD/HOLGameNames.h"
#include "Match/HOLFactionRules.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerColors.h"
#include "Match/HOLPlayerState.h"
#include "Player/HOLPlayerController.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
#include "Simulation/HOLUnitView.h"
#include "Technologies/HOLTechnologyRules.h"

// Unique namespace: unity builds merge the .cpp files.
namespace HOLSelectionUnitReadPrivate
{
	const UWorld* GetWorldOf(const UObject* WorldContextObject)
	{
		return GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	}

	/** 0 Idle, 1 Moving, 2 Gathering (and its round trip, D307), 3 Building, 4 Attacking. */
	int32 ToActivityKind(const EHOLUnitActivity Activity)
	{
		switch (Activity)
		{
		case EHOLUnitActivity::Gathering:
		case EHOLUnitActivity::GatherTrip: return 2;
		case EHOLUnitActivity::Building: return 3;
		case EHOLUnitActivity::Attacking: return 4;
		case EHOLUnitActivity::Moving: return 1;
		default: return 0;
		}
	}

	int32 ToResourceNumber(const EHOLResource Resource)
	{
		switch (Resource)
		{
		case EHOLResource::Wood: return 1;
		case EHOLResource::Stone: return 2;
		case EHOLResource::Gold: return 3;
		default: return 0;
		}
	}

	/** GetOwnerNameIn of the C++ screen (same LOCTEXT namespace and keys: the translations stay). */
	FText GetOwnerNameIn(const UWorld* World, const int32 Owner, const bool bOwn)
	{
		const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
		if (bOwn || !GameState)
		{
			return bOwn ? NSLOCTEXT("HOLGameScreen", "You", "Vous") : FText::Format(NSLOCTEXT("HOLGameScreen", "PlayerN", "Joueur {0}"), Owner + 1);
		}
		return HOLForumMessages::GetPlayerName(GameState->GetLaunchConfig(), Owner, INDEX_NONE);
	}

	const FHOLUnitTypeRow* FindRowFor(const UWorld* World, const FName Type, const int32 OwnerSlot)
	{
		const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
		return Units ? (OwnerSlot >= 0 ? Units->FindTypeRowFor(Type, OwnerSlot) : Units->FindTypeRow(Type)) : nullptr;
	}

	int32 ReadCVar(const TCHAR* Name)
	{
		const TConsoleVariableData<int32>* Variable = IConsoleManager::Get().FindTConsoleVariableDataInt(Name);
		return Variable ? Variable->GetValueOnGameThread() : 1;
	}
}

bool UHOLSelectionUnitReadLibrary::GetUnitSelectionView(const UObject* WorldContextObject, const int32 ViewerSlot, FHOLUnitSelView& Out)
{
	using namespace HOLSelectionUnitReadPrivate;

	Out = FHOLUnitSelView();
	const UWorld* World = GetWorldOf(WorldContextObject);
	const AHOLPlayerController* Controller = World ? Cast<AHOLPlayerController>(World->GetFirstPlayerController()) : nullptr;
	if (!World || !Controller || Controller->GetSelectedUnits().IsEmpty())
	{
		return false;
	}
	const TArray<uint32>& UnitIds = Controller->GetSelectedUnits();

	// RefreshUnitSelection: the selected units as this machine knows them (one: Find, without gathering them all).
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
	if (Views.IsEmpty())
	{
		return false;
	}

	const FHOLUnitView& First = Views[0];
	Out.UnitCount = Views.Num();
	Out.OwnerSlot = First.OwnerPlayer;
	Out.bOwn = ViewerSlot != INDEX_NONE && ViewerSlot == First.OwnerPlayer;
	Out.OwnerColour = FLinearColor(HOLPlayerColors::GetColor(AHOLPlayerState::FindColorIndex(*World, First.OwnerPlayer)));
	Out.OwnerName = GetOwnerNameIn(World, First.OwnerPlayer, Out.bOwn);
	for (const FHOLUnitView& View : Views)
	{
		const int32 Index = Out.TypeNames.IndexOfByKey(View.TypeName);
		if (Index == INDEX_NONE)
		{
			Out.TypeNames.Add(View.TypeName);
			Out.TypeCounts.Add(1);
			Out.TypeWorkers.Add(View.CarryCapacity > 0);
		}
		else
		{
			++Out.TypeCounts[Index];
			Out.TypeWorkers[Index] = Out.TypeWorkers[Index] || View.CarryCapacity > 0;
		}
		if (View.CarryCapacity > 0)
		{
			Out.WorkerActivities.Add(ToActivityKind(View.Activity));
		}
	}
	Out.bHero = First.bHero;
	Out.Health = First.Health;
	Out.MaxHealth = First.MaxHealth;
	Out.bHealthExact = First.bHealthExact;
	Out.ActivityKind = ToActivityKind(First.Activity);
	Out.ActivityName = UEnum::GetValueAsString(First.Activity);
	Out.LoadResourceName = UEnum::GetValueAsString(First.LoadResource);
	Out.LoadResource = ToResourceNumber(First.LoadResource);
	Out.LoadAmount = First.LoadAmount;
	Out.bLoadAmountKnown = First.bLoadAmountKnown;
	Out.CarryCapacity = First.CarryCapacity;
	return true;
}

bool UHOLSelectionUnitReadLibrary::GetUnitStatsRow(const UObject* WorldContextObject, const FName Type, const int32 OwnerSlot, const int32 ViewerSlot, FHOLUnitStatsRow& Out)
{
	using namespace HOLSelectionUnitReadPrivate;

	Out = FHOLUnitStatsRow();
	const UWorld* World = GetWorldOf(WorldContextObject);
	const FHOLUnitTypeRow* Row = FindRowFor(World, Type, OwnerSlot);
	if (!Row)
	{
		return false;
	}
	Out.Attack = Row->Attack;
	Out.BuildingAttack = Row->BuildingAttack;
	Out.MeleeArmor = Row->MeleeArmor;
	Out.RangedArmor = Row->RangedArmor;
	Out.HeroArmor = Row->HeroArmor;
	Out.SiegeArmor = Row->SiegeArmor;
	Out.Speed = Row->Speed;
	// RefreshUnitStats: the bonus is the player's own technologies' (a client does not know the others').
	const FName AttackStat = HOLTechnology::GetAttackStat(Row->AttackType);
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	if (GameState && ViewerSlot != INDEX_NONE && ViewerSlot == OwnerSlot && !AttackStat.IsNone())
	{
		for (const APlayerState* Each : GameState->PlayerArray)
		{
			const AHOLPlayerState* PlayerState = Cast<AHOLPlayerState>(Each);
			if (PlayerState && PlayerState->GetPlayerIndex() == ViewerSlot)
			{
				Out.BonusPercent = PlayerState->GetTechnologyBonusPercent(AttackStat);
				break;
			}
		}
	}
	return true;
}

int32 UHOLSelectionUnitReadLibrary::GetUnitStatsMode()
{
	return HOLSelectionUnitReadPrivate::ReadCVar(TEXT("hol.HUD.UnitStats"));
}

int32 UHOLSelectionUnitReadLibrary::GetUnitStatsBuildingMode()
{
	return HOLSelectionUnitReadPrivate::ReadCVar(TEXT("hol.HUD.UnitStatsBuilding"));
}

FText UHOLSelectionUnitReadLibrary::GetUnitName(const UObject* WorldContextObject, const FName Type, const int32 OwnerSlot)
{
	using namespace HOLSelectionUnitReadPrivate;

	const UWorld* World = GetWorldOf(WorldContextObject);
	const FName FactionKey = World && OwnerSlot >= 0 ? HOLFactionRules::GetSlotFactionKey(World, OwnerSlot) : NAME_None;
	return HOLGameNames::GetUnitName(Type, FindRowFor(World, Type, OwnerSlot), FactionKey);
}

FText UHOLSelectionUnitReadLibrary::FormatStat(const float Value)
{
	FNumberFormattingOptions Options;
	Options.MinimumFractionalDigits = 0;
	Options.MaximumFractionalDigits = 1;
	return FText::AsNumber(Value, &Options);
}
