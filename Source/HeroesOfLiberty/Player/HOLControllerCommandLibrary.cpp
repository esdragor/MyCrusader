// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLControllerCommandLibrary.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLConstructionSubsystem.h"
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
#include "Player/HOLControllerReadLibrary.h"
#include "Player/HOLUnitPicking.h"
#include "Player/HOLCommandGrid.h"
#include "Commands/HOLPlayerOrders.h"
#include "Map/HOLStartSlot.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLUnitView.h"
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

// ---- C9 (SAURON-1119ab) ----

namespace
{
	TArray<uint32> ToUnitIds(const TArray<int32>& Ids)
	{
		TArray<uint32> Result;
		for (const int32 Id : Ids)
		{
			Result.Add(static_cast<uint32>(Id));
		}
		return Result;
	}

	/** The first own building (the registry's order) the predicate takes, foundations included (the removed FindOwnBuildingWhere), or null. */
	AHOLBuilding* FindOwnBuildingMatching(const AHOLPlayerController& Controller, const TFunctionRef<bool(const AHOLBuilding&)> Predicate)
	{
		const AHOLPlayerState* PlayerState = Controller.GetHOLPlayerState();
		for (FHOLBuildingIterator It(Controller.GetWorld()); PlayerState && It; ++It)
		{
			if (It->GetOwnerPlayer() == PlayerState->GetPlayerIndex() && Predicate(**It))
			{
				return *It;
			}
		}
		return nullptr;
	}

	/** The sorted ids of the player's units known here (the removed OwnUnitIdsForProof), of one type when TypeName is not none. */
	void GatherOwnUnitIds(const AHOLPlayerController& Controller, const FName TypeName, TArray<int32>& OutIds)
	{
		OutIds.Reset();
		const AHOLPlayerState* PlayerState = Controller.GetHOLPlayerState();
		TArray<FHOLUnitView> Views;
		HOLUnitView::Gather(*Controller.GetWorld(), Views);
		TArray<uint32> UnitIds;
		for (const FHOLUnitView& View : Views)
		{
			if (PlayerState && View.OwnerPlayer == PlayerState->GetPlayerIndex() && (TypeName.IsNone() || View.TypeName == TypeName))
			{
				UnitIds.Add(View.UnitId);
			}
		}
		UnitIds.Sort();
		for (const uint32 Id : UnitIds)
		{
			OutIds.Add(static_cast<int32>(Id));
		}
	}
}

bool UHOLControllerCommandLibrary::RequestMatchPause(AHOLPlayerController* Controller)
{
	if (Controller) { Controller->RequestPause(); }
	return Controller != nullptr;
}

bool UHOLControllerCommandLibrary::RequestMatchResume(AHOLPlayerController* Controller)
{
	if (Controller) { Controller->RequestResume(); }
	return Controller != nullptr;
}

bool UHOLControllerCommandLibrary::RequestMatchSurrender(AHOLPlayerController* Controller)
{
	if (Controller) { Controller->RequestSurrender(); }
	return Controller != nullptr;
}

bool UHOLControllerCommandLibrary::OwnUnitIds(AHOLPlayerController* Controller, TArray<int32>& OutIds)
{
	OutIds.Reset();
	if (Controller) { GatherOwnUnitIds(*Controller, NAME_None, OutIds); }
	return Controller != nullptr;
}

bool UHOLControllerCommandLibrary::OwnUnitIdsOfType(AHOLPlayerController* Controller, const FName TypeName, TArray<int32>& OutIds)
{
	OutIds.Reset();
	if (Controller) { GatherOwnUnitIds(*Controller, TypeName, OutIds); }
	return Controller != nullptr;
}

bool UHOLControllerCommandLibrary::SelectUnitIds(AHOLPlayerController* Controller, const TArray<int32>& Ids)
{
	if (Controller) { Controller->SetUnitSelection(ToUnitIds(Ids)); }
	return Controller != nullptr;
}

bool UHOLControllerCommandLibrary::PressStopOrHold(AHOLPlayerController* Controller, const bool bHold)
{
	if (Controller) { Controller->ActivateOrderCell(bHold ? HOLCommandGrid::HoldCell : HOLCommandGrid::StopCell); }
	return Controller != nullptr;
}

bool UHOLControllerCommandLibrary::SelectNextIdleWorkerOf(AHOLPlayerController* Controller)
{
	if (Controller) { Controller->SelectNextIdleWorker(); }
	return Controller != nullptr;
}

bool UHOLControllerCommandLibrary::RequestCancelSelectedBuilding(AHOLPlayerController* Controller)
{
	if (Controller) { Controller->RequestCancelBuilding(Controller->GetOwnSelection()); }
	return Controller != nullptr;
}

AHOLBuilding* UHOLControllerCommandLibrary::FindMainCentreOf(AHOLPlayerController* Controller, const bool bEnemy)
{
	return Controller ? const_cast<AHOLBuilding*>(UHOLControllerReadLibrary::FindMainCentre(*Controller, bEnemy)) : nullptr;
}

AHOLBuilding* UHOLControllerCommandLibrary::FindOwnBuildingByType(AHOLPlayerController* Controller, const FName TypeName)
{
	return Controller ? FindOwnBuildingOfType(*Controller, TypeName) : nullptr;
}

AHOLBuilding* UHOLControllerCommandLibrary::FindOwnSite(AHOLPlayerController* Controller)
{
	return Controller ? FindOwnBuildingMatching(*Controller, [](const AHOLBuilding& Building) { return Building.NeedsBuilders(); }) : nullptr;
}

AHOLBuilding* UHOLControllerCommandLibrary::FindOwnGarrison(AHOLPlayerController* Controller, const FName TypeName)
{
	if (!Controller)
	{
		return nullptr;
	}
	const int32 PlayerIndex = Controller->GetHOLPlayerState() ? Controller->GetHOLPlayerState()->GetPlayerIndex() : INDEX_NONE;
	return FindOwnBuildingMatching(*Controller, [PlayerIndex, TypeName](const AHOLBuilding& Building)
	{
		return (TypeName.IsNone() || Building.GetTypeName() == TypeName) && HOLPlayerOrders::TakesGarrisonOrder(Building, PlayerIndex);
	});
}

FString UHOLControllerCommandLibrary::DescribeBuildingOf(AHOLPlayerController* Controller, AHOLBuilding* Building)
{
	return Controller ? Controller->DescribeBuilding(Building) : FString();
}

int32 UHOLControllerCommandLibrary::FindGatherSource(AHOLPlayerController* Controller, const double Dx, const double Dy, int32& OutIndex, FString& OutTypeName, double& OutX, double& OutY)
{
	const AHOLPlayerState* PlayerState = Controller ? Controller->GetHOLPlayerState() : nullptr;
	const AHOLStartSlot* Start = PlayerState ? AHOLStartSlot::FindForSlot(Controller->GetWorld(), PlayerState->GetPlayerIndex()) : nullptr;
	const UHOLSourceSubsystem* Sources = Controller ? Controller->GetWorld()->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
	if (!Start || !Sources)
	{
		return 1;
	}
	OutIndex = Sources->FindNearestStanding(Start->GetMainCentreLocation() + FVector(Dx, Dy, 0.));
	if (OutIndex == INDEX_NONE)
	{
		return 2;
	}
	const FHOLSource& Source = Sources->GetSources()[OutIndex];
	OutTypeName = Sources->GetTypeNames()[Source.TypeIndex].ToString();
	OutX = Source.Location.X;
	OutY = Source.Location.Y;
	return 0;
}

bool UHOLControllerCommandLibrary::RequestAttackMoveUnits(AHOLPlayerController* Controller, const TArray<int32>& Ids, const double X, const double Y, const bool bShift)
{
	if (Controller) { Controller->RequestAttackMove(ToUnitIds(Ids), FVector(X, Y, 0.), bShift); }
	return Controller != nullptr;
}

bool UHOLControllerCommandLibrary::RequestMoveUnitsInLineOf(AHOLPlayerController* Controller, const TArray<int32>& Ids, const double X1, const double Y1, const double X2, const double Y2, const bool bWidth, const bool bShift)
{
	if (Controller) { Controller->RequestMoveUnitsInLine(ToUnitIds(Ids), FVector(X1, Y1, 0.), FVector(X2, Y2, 0.), bWidth, bShift); }
	return Controller != nullptr;
}

bool UHOLControllerCommandLibrary::RequestAttackBuildingUnits(AHOLPlayerController* Controller, const TArray<int32>& Ids, AHOLBuilding* Building, const bool bShift)
{
	if (Controller && Building) { Controller->RequestAttackBuilding(ToUnitIds(Ids), Building, bShift); }
	return Controller && Building;
}

bool UHOLControllerCommandLibrary::RequestGatherSourceUnits(AHOLPlayerController* Controller, const TArray<int32>& Ids, const int32 SourceIndex, const bool bShift)
{
	if (Controller) { Controller->RequestGatherUnits(ToUnitIds(Ids), SourceIndex, bShift); }
	return Controller != nullptr;
}

bool UHOLControllerCommandLibrary::RequestDropOffUnitsAt(AHOLPlayerController* Controller, const TArray<int32>& Ids, AHOLBuilding* Building, const bool bShift)
{
	if (Controller && Building) { Controller->RequestDropOffUnits(ToUnitIds(Ids), Building, Building->GetActorLocation(), bShift); }
	return Controller && Building;
}

bool UHOLControllerCommandLibrary::RequestBuildSiteUnits(AHOLPlayerController* Controller, const TArray<int32>& Ids, AHOLBuilding* Site, const bool bShift)
{
	if (Controller && Site) { Controller->RequestBuildUnits(ToUnitIds(Ids), Site, bShift); }
	return Controller && Site;
}

bool UHOLControllerCommandLibrary::RequestGarrisonUnitsIn(AHOLPlayerController* Controller, const TArray<int32>& Ids, AHOLBuilding* Building)
{
	if (Controller && Building) { Controller->RequestGarrisonUnits(ToUnitIds(Ids), Building); }
	return Controller && Building;
}

bool UHOLControllerCommandLibrary::RequestUngarrisonAllOf(AHOLPlayerController* Controller, AHOLBuilding* Building)
{
	if (Controller && Building) { Controller->RequestUngarrisonAll(Building); }
	return Controller && Building;
}

bool UHOLControllerCommandLibrary::RequestUngarrisonUnitOf(AHOLPlayerController* Controller, const int32 UnitId)
{
	return false; // stub (red): the body comes with the green commit
}

bool UHOLControllerCommandLibrary::RequestUngarrisonAllSelected(AHOLPlayerController* Controller)
{
	return false; // stub (red)
}

bool UHOLControllerCommandLibrary::RequestRallyOnSite(AHOLPlayerController* Controller, AHOLBuilding* Producer, AHOLBuilding* Site, const bool bShift)
{
	if (Controller && Producer && Site) { Controller->RequestSetRallyPointOf(Producer, Site->GetActorLocation(), INDEX_NONE, Site, bShift); }
	return Controller && Producer && Site;
}

// ---- C10a (SAURON-1119ba): pointing by viewport fraction and by ground point ----

bool UHOLControllerCommandLibrary::ProjectGroundFractionOf(const AHOLPlayerController& Controller, const double X, const double Y, FVector2D& OutFraction)
{
	int32 SizeX = 0, SizeY = 0;
	Controller.GetViewportSize(SizeX, SizeY);
	FVector2D Pixel;
	if (SizeX <= 0 || SizeY <= 0 || !Controller.ProjectWorldLocationToScreen(FVector(X, Y, 0.), Pixel))
	{
		return false;
	}
	OutFraction = FVector2D(Pixel.X / SizeX, Pixel.Y / SizeY);
	return OutFraction.X >= 0. && OutFraction.X <= 1. && OutFraction.Y >= 0. && OutFraction.Y <= 1.;
}

bool UHOLControllerCommandLibrary::ProjectGroundFraction(AHOLPlayerController* Controller, const double X, const double Y, double& OutFx, double& OutFy)
{
	FVector2D Fraction(0., 0.);
	const bool bOnScreen = Controller && ProjectGroundFractionOf(*Controller, X, Y, Fraction);
	OutFx = Fraction.X;
	OutFy = Fraction.Y;
	return bOnScreen;
}

void UHOLControllerCommandLibrary::SelectAtFraction(AHOLPlayerController* Controller, const double Fx, const double Fy, const bool bAdd, const bool bDouble)
{
	if (Controller) { bDouble ? Controller->SelectSameTypeAt(FVector2D(Fx, Fy), bAdd) : Controller->SelectAt(FVector2D(Fx, Fy), bAdd); }
}

void UHOLControllerCommandLibrary::SelectInBoxFraction(AHOLPlayerController* Controller, const double X1, const double Y1, const double X2, const double Y2, const bool bAdd)
{
	if (Controller) { Controller->SelectInBox(FVector2D(X1, Y1), FVector2D(X2, Y2), bAdd); }
}

void UHOLControllerCommandLibrary::OrderAtFraction(AHOLPlayerController* Controller, const double Fx, const double Fy, const bool bQueue)
{
	if (Controller) { Controller->OrderAt(FVector2D(Fx, Fy), bQueue); }
}

void UHOLControllerCommandLibrary::OrderLineAtFractions(AHOLPlayerController* Controller, const double X1, const double Y1, const double X2, const double Y2, const bool bWidth, const bool bQueue)
{
	if (Controller) { Controller->OrderLineAt(FVector2D(X1, Y1), FVector2D(X2, Y2), bWidth, bQueue); }
}

bool UHOLControllerCommandLibrary::UnitScreenPoint(AHOLPlayerController* Controller, const int32 Wanted, const bool bEnemy, const bool bAlly, const int32 Slot, int32& OutUnitId, double& OutFx, double& OutFy, int32& OutN)
{
	OutN = Wanted;
	if (!Controller || !Controller->GetWorld())
	{
		return false;
	}
	const AHOLPlayerState* PlayerState = Controller->GetHOLPlayerState();
	TArray<FHOLUnitView> Views;
	HOLUnitView::Gather(*Controller->GetWorld(), Views);
	const AHOLGameState* GameState = Controller->GetWorld()->GetGameState<AHOLGameState>();
	Views.RemoveAll([PlayerState, GameState, bEnemy, bAlly, Slot](const FHOLUnitView& View)
	{
		if (Slot != INDEX_NONE)
		{
			return View.OwnerPlayer != Slot;
		}
		if (bAlly)
		{
			return !PlayerState || !GameState
				|| HOLUnitPicking::GetSide(GameState->GetLaunchConfig(), View.OwnerPlayer, PlayerState->GetPlayerIndex()) != EHOLSelectionSide::Ally;
		}
		const bool bOwn = PlayerState && View.OwnerPlayer == PlayerState->GetPlayerIndex();
		return bEnemy ? bOwn || !PlayerState : !bOwn;
	});
	Views.Sort([](const FHOLUnitView& A, const FHOLUnitView& B) { return A.UnitId < B.UnitId; });
	// Negative: from the last one (-1), whatever units the steps before made.
	const int32 N = Wanted < 0 ? Views.Num() + Wanted : Wanted;
	OutN = N;
	int32 SizeX = 0, SizeY = 0;
	Controller->GetViewportSize(SizeX, SizeY);
	FVector2D Middle;
	if (!Views.IsValidIndex(N) || SizeX <= 0 || SizeY <= 0 || !Controller->ProjectWorldLocationToScreen(
		Views[N].Location + FVector(0.0, 0.0, PickBodyHeight * 0.5 * Views[N].MeshScale), Middle))
	{
		return false;
	}
	OutUnitId = static_cast<int32>(Views[N].UnitId);
	OutFx = Middle.X / SizeX;
	OutFy = Middle.Y / SizeY;
	return true;
}

bool UHOLControllerCommandLibrary::PointNearFor(const AHOLPlayerController& Controller, const double Dx, const double Dy, const bool bEnemy, FVector& OutPoint)
{
	const AHOLBuilding* Centre = UHOLControllerReadLibrary::FindMainCentre(Controller, bEnemy);
	if (!Centre)
	{
		return false;
	}
	OutPoint = Centre->GetActorLocation() + Centre->GetActorRotation().RotateVector(FVector(Dx, Dy, 0.));
	OutPoint.Z = 0.;
	return true;
}

bool UHOLControllerCommandLibrary::PointNearOf(AHOLPlayerController* Controller, const double Dx, const double Dy, const bool bEnemy, double& OutX, double& OutY)
{
	FVector Point = FVector::ZeroVector;
	const bool bFound = Controller && PointNearFor(*Controller, Dx, Dy, bEnemy, Point);
	OutX = Point.X;
	OutY = Point.Y;
	return bFound;
}


// ---- C10b (SAURON-1119ca): sources, heal / attack by identifier, units order, placement near / at a source ----

int32 UHOLControllerCommandLibrary::SourceScreenPoint(AHOLPlayerController* Controller, const double X, const double Y, int32& OutIndex, double& OutFx, double& OutFy)
{
	OutIndex = INDEX_NONE;
	const UHOLSourceSubsystem* Sources = Controller && Controller->GetWorld() ? Controller->GetWorld()->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
	const int32 Index = Sources ? Sources->FindNearestStanding(FVector(X, Y, 0.)) : INDEX_NONE;
	if (Index == INDEX_NONE)
	{
		return 1;
	}
	OutIndex = Index;
	const FHOLSource& Source = Sources->GetSources()[Index];
	const FHOLSourceTypeRow* Type = Sources->GetType(Source.TypeIndex);
	int32 SizeX = 0, SizeY = 0;
	Controller->GetViewportSize(SizeX, SizeY);
	FVector2D Pixel;
	if (!Type || SizeX <= 0 || SizeY <= 0 || !Controller->ProjectWorldLocationToScreen(Source.Location + FVector(0., 0., Type->Height * 0.3), Pixel))
	{
		return 2;
	}
	OutFx = Pixel.X / SizeX;
	OutFy = Pixel.Y / SizeY;
	return 0;
}

int32 UHOLControllerCommandLibrary::FirstUnitIdOfSlot(AHOLPlayerController* Controller, const int32 Slot)
{
	if (!Controller || !Controller->GetWorld())
	{
		return 0;
	}
	TArray<FHOLUnitView> Views;
	HOLUnitView::Gather(*Controller->GetWorld(), Views);
	uint32 TargetId = 0;
	for (const FHOLUnitView& View : Views)
	{
		TargetId = View.OwnerPlayer == Slot && (TargetId == 0 || View.UnitId < TargetId) ? View.UnitId : TargetId;
	}
	return static_cast<int32>(TargetId);
}

bool UHOLControllerCommandLibrary::AllUnitIds(AHOLPlayerController* Controller, TArray<int32>& OutIds)
{
	OutIds.Reset();
	if (!Controller || !Controller->GetWorld())
	{
		return false;
	}
	TArray<FHOLUnitView> Views;
	HOLUnitView::Gather(*Controller->GetWorld(), Views);
	for (const FHOLUnitView& View : Views)
	{
		OutIds.Add(static_cast<int32>(View.UnitId));
	}
	return true;
}

bool UHOLControllerCommandLibrary::CommandUnitIds(AHOLPlayerController* Controller, TArray<int32>& OutIds)
{
	OutIds.Reset();
	if (!Controller)
	{
		return false;
	}
	for (const uint32 UnitId : Controller->GetCommandUnits())
	{
		OutIds.Add(static_cast<int32>(UnitId));
	}
	return true;
}

bool UHOLControllerCommandLibrary::RequestHealUnitIds(AHOLPlayerController* Controller, const TArray<int32>& Ids, const int32 TargetId, const bool bQueue)
{
	if (!Controller)
	{
		return false;
	}
	TArray<uint32> Units;
	for (const int32 Id : Ids)
	{
		Units.Add(static_cast<uint32>(Id));
	}
	Controller->RequestHealUnit(Units, static_cast<uint32>(TargetId), bQueue);
	return true;
}

bool UHOLControllerCommandLibrary::RequestAttackUnitIds(AHOLPlayerController* Controller, const TArray<int32>& Ids, const int32 TargetId, const bool bQueue)
{
	if (!Controller)
	{
		return false;
	}
	TArray<uint32> Units;
	for (const int32 Id : Ids)
	{
		Units.Add(static_cast<uint32>(Id));
	}
	Controller->RequestAttackUnit(Units, static_cast<uint32>(TargetId), bQueue);
	FHOLUnitView View;
	if (Controller->GetWorld() && HOLUnitView::Find(*Controller->GetWorld(), static_cast<uint32>(TargetId), View))
	{
		Controller->OnAttackOrderGiven.Broadcast(View.Location);
	}
	return true;
}

bool UHOLControllerCommandLibrary::RequestMoveUnitIdsTo(AHOLPlayerController* Controller, const TArray<int32>& Ids, const double X, const double Y)
{
	if (!Controller)
	{
		return false;
	}
	TArray<uint32> Units;
	for (const int32 Id : Ids)
	{
		Units.Add(static_cast<uint32>(Id));
	}
	Controller->RequestMoveUnits(Units, FVector(X, Y, 0.));
	return true;
}

bool UHOLControllerCommandLibrary::StartCentreOf(AHOLPlayerController* Controller, double& OutX, double& OutY)
{
	const AHOLPlayerState* PlayerState = Controller && Controller->GetWorld() ? Controller->GetHOLPlayerState() : nullptr;
	const AHOLStartSlot* Start = PlayerState ? AHOLStartSlot::FindForSlot(Controller->GetWorld(), PlayerState->GetPlayerIndex()) : nullptr;
	if (!Start)
	{
		return false;
	}
	const FVector Centre = Start->GetMainCentreLocation();
	OutX = Centre.X;
	OutY = Centre.Y;
	return true;
}

void UHOLControllerCommandLibrary::RequestPlacementAtPoint(AHOLPlayerController& Controller, const FName TypeName, const FVector& Point)
{
	const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(TypeName);
	if (!Type)
	{
		return;
	}
	const FBox2D Footprint = UHOLConstructionSubsystem::SnapFootprint(*Type, Point);
	FString Reason;
	const bool bValid = Controller.CheckPlacementHere(TypeName, Footprint, Reason);
	UE_LOG(LogHOLMatch, Display, TEXT("Build request: %s at (%.0f, %.0f), here %s"), *TypeName.ToString(), Footprint.GetCenter().X,
		Footprint.GetCenter().Y, bValid ? TEXT("valid") : *FString::Printf(TEXT("refused (%s)"), *Reason));
	Controller.RequestPlaceBuilding(TypeName, Point);
}

bool UHOLControllerCommandLibrary::RequestPlacementNear(AHOLPlayerController* Controller, const FName TypeName, const double Dx, const double Dy, const bool bEnemy)
{
	const AHOLBuilding* Centre = Controller ? UHOLControllerReadLibrary::FindMainCentre(*Controller, bEnemy) : nullptr;
	if (!Centre)
	{
		return false;
	}
	const FVector Offset = Centre->GetActorRotation().RotateVector(FVector(Dx, Dy, 0.));
	RequestPlacementAtPoint(*Controller, TypeName, Centre->GetActorLocation() + Offset);
	return true;
}

int32 UHOLControllerCommandLibrary::StandingSourceNear(AHOLPlayerController* Controller, const double X, const double Y, double& OutX, double& OutY)
{
	const UHOLSourceSubsystem* Sources = Controller && Controller->GetWorld() ? Controller->GetWorld()->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
	const int32 Index = Sources ? Sources->FindNearestStanding(FVector(X, Y, 0.)) : INDEX_NONE;
	if (Index != INDEX_NONE)
	{
		OutX = Sources->GetSources()[Index].Location.X;
		OutY = Sources->GetSources()[Index].Location.Y;
	}
	return Index;
}

bool UHOLControllerCommandLibrary::RequestPlacementOnSource(AHOLPlayerController* Controller, const FName TypeName, const int32 SourceIndex)
{
	const UHOLSourceSubsystem* Sources = Controller && Controller->GetWorld() ? Controller->GetWorld()->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
	if (!Sources || !Sources->GetSources().IsValidIndex(SourceIndex))
	{
		return false;
	}
	RequestPlacementAtPoint(*Controller, TypeName, Sources->GetSources()[SourceIndex].Location);
	return true;
}
