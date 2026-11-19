// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLControllerCommandLibrary.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLProductionRules.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerState.h"
#include "Player/HOLPlayerController.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

namespace
{
	/** The player's own building of a type (foundations included, registry order), or null: the rule of the controller file's FindOwnBuilding (same loop, kept here because that helper is file-local). */
	AHOLBuilding* FindOwnBuildingOfType(const AHOLPlayerController& Controller, const FName TypeName)
	{
		const AHOLPlayerState* PlayerState = Controller.GetHOLPlayerState();
		for (FHOLBuildingIterator It(Controller.GetWorld()); PlayerState && It; ++It)
		{
			if (It->GetOwnerPlayer() == PlayerState->GetPlayerIndex() && It->GetTypeName() == TypeName)
			{
				return *It;
			}
		}
		return nullptr;
	}

	AHOLGameState* GameStateOf(const UObject* WorldContextObject)
	{
		const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
		return World ? World->GetGameState<AHOLGameState>() : nullptr;
	}
}

bool UHOLControllerCommandLibrary::OwnSelectedUnitIds(AHOLPlayerController* Controller, TArray<int32>& OutIds)
{
	OutIds.Reset();
	if (!Controller)
	{
		return false;
	}
	for (const uint32 Id : Controller->GetOwnSelectedUnits())
	{
		OutIds.Add(static_cast<int32>(Id));
	}
	return true;
}

bool UHOLControllerCommandLibrary::HasOwnSelection(AHOLPlayerController* Controller)
{
	return Controller && Controller->GetOwnSelection() != nullptr;
}

bool UHOLControllerCommandLibrary::SelectOwnBuilding(AHOLPlayerController* Controller, const FName TypeName)
{
	if (!Controller)
	{
		return false;
	}
	AHOLBuilding* Building = FindOwnBuildingOfType(*Controller, TypeName);
	Controller->SetSelection(Building);
	return Building != nullptr;
}

bool UHOLControllerCommandLibrary::EnsureOwnSelection(AHOLPlayerController* Controller, const FName FallbackType)
{
	if (!Controller)
	{
		return false;
	}
	if (!Controller->GetOwnSelection())
	{
		Controller->SetSelection(FindOwnBuildingOfType(*Controller, FallbackType));
	}
	return Controller->GetOwnSelection() != nullptr;
}

bool UHOLControllerCommandLibrary::IsSpectatorController(AHOLPlayerController* Controller)
{
	return Controller && Controller->IsSpectator();
}

bool UHOLControllerCommandLibrary::RequestCastRageOfSelection(AHOLPlayerController* Controller)
{
	if (!Controller)
	{
		return false;
	}
	Controller->RequestCastRage(Controller->GetOwnSelectedUnits());
	return true;
}

bool UHOLControllerCommandLibrary::RequestRallyPointAt(AHOLPlayerController* Controller, const double X, const double Y)
{
	if (!Controller)
	{
		return false;
	}
	Controller->RequestRallyPoint(FVector(X, Y, 0.));
	return true;
}

bool UHOLControllerCommandLibrary::RequestProduction(AHOLPlayerController* Controller, const FName TypeName)
{
	if (!Controller)
	{
		return false;
	}
	Controller->RequestProduction(TypeName);
	return true;
}

bool UHOLControllerCommandLibrary::RequestCancelProduction(AHOLPlayerController* Controller, const int32 Index)
{
	if (!Controller)
	{
		return false;
	}
	Controller->RequestCancelProduction(Index);
	return true;
}

bool UHOLControllerCommandLibrary::RequestMarketTrade(AHOLPlayerController* Controller, const int32 Resource, const int32 Lots, const bool bBuy)
{
	if (!Controller)
	{
		return false;
	}
	Controller->RequestMarketTrade(Resource, Lots, bBuy);
	return true;
}

bool UHOLControllerCommandLibrary::RequestTribute(AHOLPlayerController* Controller, const int32 Receiver, const int32 Resource, const int32 Amount)
{
	if (!Controller)
	{
		return false;
	}
	Controller->RequestTribute(Receiver, Resource, Amount);
	return true;
}

bool UHOLControllerCommandLibrary::ReceivedCounts(AHOLPlayerController* Controller, int32& OutSignals, int32& OutChats, int32& OutSlot)
{
	OutSignals = 0;
	OutChats = 0;
	OutSlot = -1;
	if (!Controller)
	{
		return false;
	}
	const AHOLPlayerState* PlayerState = Controller->GetHOLPlayerState();
	OutSignals = Controller->GetNumReceivedSignals();
	OutChats = Controller->GetNumReceivedChats();
	OutSlot = PlayerState ? PlayerState->GetPlayerIndex() : -1;
	return true;
}

TArray<FString> UHOLControllerCommandLibrary::DescribeSpecializationStates(AHOLPlayerController* Controller)
{
	TArray<FString> Lines;
	const AHOLPlayerState* PlayerState = Controller ? Controller->GetHOLPlayerState() : nullptr;
	const AGameStateBase* GameState = Controller && Controller->GetWorld() ? Controller->GetWorld()->GetGameState() : nullptr;
	if (!PlayerState || !GameState)
	{
		return Lines;
	}
	for (const APlayerState* Other : GameState->PlayerArray)
	{
		if (const AHOLPlayerState* OtherState = Cast<AHOLPlayerState>(Other))
		{
			const FHOLSpecializationView& View = OtherState->GetSpecialization();
			Lines.Add(FString::Printf(TEXT("Specialization state slot %d %s: pending tier %d option %d, %d ms left, chosen %d %d %d"),
				OtherState->GetPlayerIndex(), OtherState == PlayerState ? TEXT("local") : TEXT("other"), View.PendingTier, View.PendingOption,
				View.RemainingMs, View.GetChosen(1), View.GetChosen(2), View.GetChosen(3)));
		}
	}
	return Lines;
}

FString UHOLControllerCommandLibrary::DescribeProductionReport(AHOLPlayerController* Controller)
{
	if (!Controller || !Controller->GetWorld())
	{
		return FString();
	}
	const AHOLPlayerState* PlayerState = Controller->GetHOLPlayerState();
	const AHOLBuilding* Building = Controller->GetOwnSelection();
	TArray<FString> Items;
	if (Building)
	{
		const UHOLUnitSubsystem* Units = Controller->GetWorld()->GetSubsystem<UHOLUnitSubsystem>();
		const double Step = UHOLProductionSubsystem::GetDisplayStep(Controller->GetWorld());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		for (const FHOLProductionItem& Item : Building->GetProductionQueue())
		{
			const FHOLUnitTypeRow* Row = Units ? Units->FindTypeRow(Item.TypeName) : nullptr;
			Items.Add(Item.IsStarted() && Row
				? FString::Printf(TEXT("%s %.2f"), *Item.TypeName.ToString(),
					HOLProduction::GetProgress(Item, Step, UHOLProductionSubsystem::GetBuildSteps(*Row, StepSeconds)))
				: FString::Printf(TEXT("%s waiting"), *Item.TypeName.ToString()));
		}
	}
	// The other players' economy as this machine sees it: a client is sent its own only (D282).
	TArray<FString> Others;
	if (const AGameStateBase* GameState = Controller->GetWorld()->GetGameState())
	{
		for (const APlayerState* Other : GameState->PlayerArray)
		{
			const AHOLPlayerState* OtherState = Cast<AHOLPlayerState>(Other);
			if (OtherState && OtherState != PlayerState)
			{
				Others.Add(FString::Printf(TEXT("slot %d %s"), OtherState->GetPlayerIndex(), *OtherState->DescribeEconomy()));
			}
		}
	}
	return FString::Printf(TEXT("Production report: %s, selected %s, queue %d [%s]; others [%s]"),
		PlayerState ? *PlayerState->DescribeEconomy() : TEXT("no player state"),
		*Controller->DescribeBuilding(Controller->GetSelection()), Items.Num(), *FString::Join(Items, TEXT(", ")),
		*FString::Join(Others, TEXT("; ")));
}

TArray<FString> UHOLControllerCommandLibrary::DescribeMarketReport(const UObject* WorldContextObject)
{
	TArray<FString> Lines;
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	if (!GameState)
	{
		Lines.Add(TEXT("Market report: no game state"));
		return Lines;
	}
	const HOLMarket::FBook Book = GameState->GetMarketBook();
	Lines.Add(FString::Printf(TEXT("Market report: net mode %d, book %s, prices %d %d %d %d"), static_cast<int32>(World->GetNetMode()),
		GameState->HasMarketBook() ? TEXT("set") : TEXT("not set"), Book.Prices[0], Book.Prices[1], Book.Prices[2], Book.Prices[3]));
	int32 Markets = 0;
	for (FHOLBuildingIterator It(World); It; ++It)
	{
		if (It->GetTypeName() == FName(TEXT("Market")))
		{
			++Markets;
			Lines.Add(FString::Printf(TEXT("Market report: a Market of slot %d, %s"), static_cast<int32>(It->GetOwnerPlayer()),
				It->IsFinished() ? TEXT("finished") : TEXT("not finished")));
		}
	}
	Lines.Add(FString::Printf(TEXT("Market report: %d Markets"), Markets));
	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (const AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState))
		{
			const FHOLResources Stock = HOLPlayerState->GetResources();
			Lines.Add(FString::Printf(TEXT("Market stock slot %d: food %d wood %d stone %d gold %d"), HOLPlayerState->GetPlayerIndex(), Stock.Food, Stock.Wood,
				Stock.Stone, Stock.Gold));
		}
	}
	return Lines;
}

bool UHOLControllerCommandLibrary::ForceStance(const UObject* WorldContextObject, const int32 FromSlot, const int32 ToSlot, const int32 Stance)
{
	AHOLGameState* GameState = GameStateOf(WorldContextObject);
	if (!GameState)
	{
		return false;
	}
	GameState->ApplyStance(FromSlot, ToSlot, Stance);
	return true;
}

bool UHOLControllerCommandLibrary::ForceVision(const UObject* WorldContextObject, const int32 FromSlot, const int32 ToSlot, const bool bShare)
{
	AHOLGameState* GameState = GameStateOf(WorldContextObject);
	if (!GameState)
	{
		return false;
	}
	GameState->ApplyVision(FromSlot, ToSlot, bShare);
	return true;
}

bool UHOLControllerCommandLibrary::ReportDiplomacy(const UObject* WorldContextObject)
{
	const AHOLGameState* GameState = GameStateOf(WorldContextObject);
	if (!GameState)
	{
		return false;
	}
	GameState->ReportDiplomacy();
	return true;
}

bool UHOLControllerCommandLibrary::NoteTributeNotice(const UObject* WorldContextObject, const int32 GiverSlot, const int32 Resource, const int32 Arrives, const int32 Tax)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	const APlayerController* Local = World ? World->GetFirstPlayerController() : nullptr;
	const AHOLPlayerState* LocalState = Local ? Local->GetPlayerState<AHOLPlayerState>() : nullptr;
	if (!GameState || !LocalState)
	{
		return false;
	}
	GameState->NoteTributeReceived(GiverSlot, LocalState->GetPlayerIndex(), Resource, Arrives, Tax);
	return true;
}
