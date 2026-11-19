// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLControllerCommandLibrary.generated.h"

class AHOLPlayerController;

/**
 * What the development console commands that stayed in C++ do on the controller and on the game state, for their script version (C2 step 5,
 * SAURON-1119u, D839 "liaison a exposer"; script namespace HOLControllerCommand). Wrappers only, no rule: every Request<X> is the controller's own
 * (its guards, its spectator check, its selection reads stay the C++ ones, so the script cannot drift from them), every Describe<X> gives the
 * EXACT text of the C++ line the command logged (the script writes it with HOLScriptLog::LogDisplay(LogHOLMatch, ...)).
 * The loop over the local controllers stays the script's (GetLocalControllers is a C++ helper of the controller's file).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLControllerCommandLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ---- Selection (hol.Hero.Rage, hol.Prod.Rally / Queue / Cancel, hol.Market.Trade, hol.Tribute) ----

	/** The ids of the units the controller has selected AND owns (GetOwnSelectedUnits), cast to int32. False without a controller. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool OwnSelectedUnitIds(AHOLPlayerController* Controller, TArray<int32>& OutIds);

	/** Whether the controller has a selected building of its own (GetOwnSelection() not null). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool HasOwnSelection(AHOLPlayerController* Controller);

	/** SetSelection(the player's first own building of that type, foundations included, registry order): null clears the selection. True when one was found. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool SelectOwnBuilding(AHOLPlayerController* Controller, FName TypeName);

	/** If GetOwnSelection() is null, SelectOwnBuilding(FallbackType) (the "its main centre selected first" of Prod.Rally / Prod.Queue). True when an own building is selected afterwards. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool EnsureOwnSelection(AHOLPlayerController* Controller, FName FallbackType);

	/** AHOLPlayerController::IsSpectator() (a pure spectator: its player state IsOnlyASpectator). The C++ Request<X> of the hero, duel, diplomacy, signal and specialization commands returned silently for it; the script console commands (C2) ask this before sending (SAURON-1119w). False without a controller. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool IsSpectatorController(AHOLPlayerController* Controller);

	// ---- The controller's own Request<X> (not UFUNCTION in the controller) ----

	/** Controller->RequestCastRage(Controller->GetOwnSelectedUnits()) (hol.Hero.Rage). False without a controller. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestCastRageOfSelection(AHOLPlayerController* Controller);

	/** Controller->RequestRallyPoint(FVector(X, Y, 0)) (hol.Prod.Rally): the selected own building, the point in cm. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestRallyPointAt(AHOLPlayerController* Controller, double X, double Y);

	/** Controller->RequestProduction(TypeName) (hol.Prod.Queue). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestProduction(AHOLPlayerController* Controller, FName TypeName);

	/** Controller->RequestCancelProduction(Index) (hol.Prod.Cancel): the item of the selected own building's queue. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestCancelProduction(AHOLPlayerController* Controller, int32 Index);

	/** Controller->RequestMarketTrade(Resource, Lots, bBuy) (hol.Market.Trade); Resource 0 food, 1 wood, 2 stone. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestMarketTrade(AHOLPlayerController* Controller, int32 Resource, int32 Lots, bool bBuy);

	/** Controller->RequestTribute(Receiver, Resource, Amount) (hol.Tribute); Resource 0 food, 1 wood, 2 stone, 3 gold. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestTribute(AHOLPlayerController* Controller, int32 Receiver, int32 Resource, int32 Amount);

	// ---- Reports (hol.Signal.Report, hol.Chat.Report, hol.Spec.Report, hol.Prod.Report, hol.Market.Report) ----

	/** The counts of hol.Signal.Report and hol.Chat.Report: received signals, received chats, and the slot of the controller's player state (-1 when it has none). False without a controller. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool ReceivedCounts(AHOLPlayerController* Controller, int32& OutSignals, int32& OutChats, int32& OutSlot);

	/** The lines of hol.Spec.Report for this controller ("Specialization state slot N local|other: pending tier ..."), one per player state known here; empty without player state or game state. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static TArray<FString> DescribeSpecializationStates(AHOLPlayerController* Controller);

	/** The line of hol.Prod.Report for this controller ("Production report: <economy>, selected <building>, queue N [...]; others [...]"). Empty without a controller. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static FString DescribeProductionReport(AHOLPlayerController* Controller);

	/** The lines of hol.Market.Report in order ("Market report: no game state" alone when there is none), of the world of WorldContextObject (a dedicated server too). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand", meta = (WorldContext = "WorldContextObject"))
	static TArray<FString> DescribeMarketReport(const UObject* WorldContextObject);

	// ---- Game state (hol.Diplomacy.Force / ForceVision / Report, hol.Tribute.Notice) ----

	/** AHOLGameState::ApplyStance(From, To, Stance) at once (hol.Diplomacy.Force); Stance 0 enemy, 1 neutral, 2 ally (HOLDiplomacy). False when there is no AHOLGameState here (the "Usage" Warning case). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand", meta = (WorldContext = "WorldContextObject"))
	static bool ForceStance(const UObject* WorldContextObject, int32 FromSlot, int32 ToSlot, int32 Stance);

	/** AHOLGameState::ApplyVision(From, To, bShare) at once (hol.Diplomacy.ForceVision). False when there is no AHOLGameState here. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand", meta = (WorldContext = "WorldContextObject"))
	static bool ForceVision(const UObject* WorldContextObject, int32 FromSlot, int32 ToSlot, bool bShare);

	/** AHOLGameState::ReportDiplomacy() (hol.Diplomacy.Report: the game state logs its own lines). False when there is no AHOLGameState here (the C++ command then did nothing). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand", meta = (WorldContext = "WorldContextObject"))
	static bool ReportDiplomacy(const UObject* WorldContextObject);

	/** AHOLGameState::NoteTributeReceived(Giver, <slot of the first local player controller's state>, Resource, Arrives, Tax) (hol.Tribute.Notice; Resource 0..3, the C++ command mapped an unknown name to 3). False when there is no game state or no local player state (the "Usage" Warning case). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand", meta = (WorldContext = "WorldContextObject"))
	static bool NoteTributeNotice(const UObject* WorldContextObject, int32 GiverSlot, int32 Resource, int32 Arrives, int32 Tax);
};
