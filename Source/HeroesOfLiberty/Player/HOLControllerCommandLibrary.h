// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLControllerCommandLibrary.generated.h"

class AHOLPlayerController;
class AHOLBuilding;

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

	// ---- C9 (SAURON-1119ab): the "test pilot" commands hol.Match.Pause / Resume, hol.Victory.Surrender, hol.Units.*, hol.Prod.RallySite, hol.Select.Stop / Hold / NextIdle / Own / Type, hol.Build.CancelSelected ----
	// Wrappers only: each one is the controller's own call, the lookups are the C++ ones of the removed commands (same loops, same predicates), the script keeps the arguments, the journal lines and the loop over the local controllers.

	/** Controller->RequestPause() / RequestResume() / RequestSurrender() (hol.Match.Pause, hol.Match.Resume, hol.Victory.Surrender). False without a controller. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestMatchPause(AHOLPlayerController* Controller);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestMatchResume(AHOLPlayerController* Controller);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestMatchSurrender(AHOLPlayerController* Controller);

	/** The ids (sorted) of every unit of the controller's player known here, on the screen or not (HOLUnitView::Gather filtered on the player index; OwnUnitIdsForProof, hol.Select.Own). False without a controller. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool OwnUnitIds(AHOLPlayerController* Controller, TArray<int32>& OutIds);

	/** The same, of one unit type only (hol.Select.Type). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool OwnUnitIdsOfType(AHOLPlayerController* Controller, FName TypeName, TArray<int32>& OutIds);

	/** Controller->SetUnitSelection(Ids) (hol.Select.Own, hol.Select.Type). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool SelectUnitIds(AHOLPlayerController* Controller, const TArray<int32>& Ids);

	/** Controller->ActivateOrderCell(StopCell or HoldCell) (hol.Select.Stop, hol.Select.Hold). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool PressStopOrHold(AHOLPlayerController* Controller, bool bHold);

	/** Controller->SelectNextIdleWorker() (hol.Select.NextIdle). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool SelectNextIdleWorkerOf(AHOLPlayerController* Controller);

	/** Controller->RequestCancelBuilding(Controller->GetOwnSelection()) (hol.Build.CancelSelected). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestCancelSelectedBuilding(AHOLPlayerController* Controller);

	/** The main centre of the player (bEnemy false) or of the first other player (true) known here, or null (the file-local FindMainCentre of the removed hol.Units.* commands). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static AHOLBuilding* FindMainCentreOf(AHOLPlayerController* Controller, bool bEnemy);

	/** The first own building (registry order, foundations included) of a type, or null (FindOwnBuilding). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static AHOLBuilding* FindOwnBuildingByType(AHOLPlayerController* Controller, FName TypeName);

	/** The first own building that is not finished and waits for builders (a site), or null (hol.Units.Build, hol.Prod.RallySite). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static AHOLBuilding* FindOwnSite(AHOLPlayerController* Controller);

	/** The first own finished building with places (of that type when TypeName is not none), or null (hol.Units.Garrison / UngarrisonAll). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static AHOLBuilding* FindOwnGarrison(AHOLPlayerController* Controller, FName TypeName);

	/** Controller->DescribeBuilding(Building): the text the "Order: ..." lines carry. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static FString DescribeBuildingOf(AHOLPlayerController* Controller, AHOLBuilding* Building);

	/** The standing source nearest the player's start plus (Dx, Dy) cm (hol.Units.GatherOrder): 0 found (index, type name and place filled), 1 no start slot or no source subsystem, 2 no standing source. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static int32 FindGatherSource(AHOLPlayerController* Controller, double Dx, double Dy, int32& OutIndex, FString& OutTypeName, double& OutX, double& OutY);

	/** The controller's Request<X> of the hol.Units.* commands, on the given unit ids (what OwnUnitIds returned). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestAttackMoveUnits(AHOLPlayerController* Controller, const TArray<int32>& Ids, double X, double Y, bool bShift);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestMoveUnitsInLineOf(AHOLPlayerController* Controller, const TArray<int32>& Ids, double X1, double Y1, double X2, double Y2, bool bWidth, bool bShift);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestAttackBuildingUnits(AHOLPlayerController* Controller, const TArray<int32>& Ids, AHOLBuilding* Building, bool bShift);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestGatherSourceUnits(AHOLPlayerController* Controller, const TArray<int32>& Ids, int32 SourceIndex, bool bShift);
	/** Drop off at the building's own place (Building->GetActorLocation(), as the command did). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestDropOffUnitsAt(AHOLPlayerController* Controller, const TArray<int32>& Ids, AHOLBuilding* Building, bool bShift);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestBuildSiteUnits(AHOLPlayerController* Controller, const TArray<int32>& Ids, AHOLBuilding* Site, bool bShift);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestGarrisonUnitsIn(AHOLPlayerController* Controller, const TArray<int32>& Ids, AHOLBuilding* Building);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestUngarrisonAllOf(AHOLPlayerController* Controller, AHOLBuilding* Building);
	/** Controller->RequestSetRallyPointOf(Producer, Site->GetActorLocation(), INDEX_NONE, Site, bShift) (hol.Prod.RallySite). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestRallyOnSite(AHOLPlayerController* Controller, AHOLBuilding* Producer, AHOLBuilding* Site, bool bShift);

	// ---- C10a (SAURON-1119ba): pointing by viewport fraction and by ground point (hol.Select.Click / ClickUnit / Box / BoxGround / OrderGround / OrderLineGround / ClickGround / OrderNear) ----

	/** Height (cm) of a unit's body drawn at scale 1 (the mannequin): what a click aims at (D288). The controller's PickBodyHeight is this. */
	static constexpr double PickBodyHeight = 180.0;

	/** Where a ground point (cm) is drawn, as a fraction of the viewport: true only when it is ON the screen (0 to 1 each way). The fraction is filled whenever the point projects. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool ProjectGroundFraction(AHOLPlayerController* Controller, double X, double Y, double& OutFx, double& OutFy);
	/** The same for the C++ callers (the controller's ProjectGroundPoint): ONE rule. */
	static bool ProjectGroundFractionOf(const AHOLPlayerController& Controller, double X, double Y, FVector2D& OutFraction);

	/** Controller->SelectAt (bDouble: SelectSameTypeAt) at that viewport fraction. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static void SelectAtFraction(AHOLPlayerController* Controller, double Fx, double Fy, bool bAdd, bool bDouble);
	/** Controller->SelectInBox between two viewport fractions. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static void SelectInBoxFraction(AHOLPlayerController* Controller, double X1, double Y1, double X2, double Y2, bool bAdd);
	/** Controller->OrderAt (a right click) at that viewport fraction. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static void OrderAtFraction(AHOLPlayerController* Controller, double Fx, double Fy, bool bQueue);
	/** Controller->OrderLineAt (a right-button drag) between two viewport fractions. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static void OrderLineAtFractions(AHOLPlayerController* Controller, double X1, double Y1, double X2, double Y2, bool bWidth, bool bQueue);

	/**
	 * Where the Wanted-th unit (by identifier, negative from the last) of the chosen side is drawn (hol.Select.ClickUnit): the units known here, those of slot Slot when it is not
	 * INDEX_NONE, else the ally ones (bAlly), else the other players' (bEnemy), else the player's own. OutN is the resolved index (also set on failure); false when there is no such
	 * unit or it does not project (the fraction is not range checked, as before).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool UnitScreenPoint(AHOLPlayerController* Controller, int32 Wanted, bool bEnemy, bool bAlly, int32 Slot, int32& OutUnitId, double& OutFx, double& OutFy, int32& OutN);

	/** The ground point (Dx, Dy) cm in the frame of the player's main centre (or an enemy's), Z 0 (hol.Select.OrderNear, AttackMoveNear, Camera.FocusNear); false without a centre. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool PointNearOf(AHOLPlayerController* Controller, double Dx, double Dy, bool bEnemy, double& OutX, double& OutY);
	static bool PointNearFor(const AHOLPlayerController& Controller, double Dx, double Dy, bool bEnemy, FVector& OutPoint);

	// ---- C10b (SAURON-1119ca): sources, heal / attack by identifier, units order, placement near / at a source ----

	/**
	 * Where the standing resource source nearest a ground point (cm) is drawn, aimed at the middle of its trunk (the former controller AimAtSource, ONE rule).
	 * Returns 0 = found and on screen (OutIndex, OutFx, OutFy filled), 1 = no standing source, 2 = the source is not on the screen (OutIndex filled).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static int32 SourceScreenPoint(AHOLPlayerController* Controller, double X, double Y, int32& OutIndex, double& OutFx, double& OutFy);

	/** The lowest identifier among the units of slot Slot known here (HOLUnitView::Gather), 0 when there is none (hol.Select.Heal / Attack slot=<S>). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static int32 FirstUnitIdOfSlot(AHOLPlayerController* Controller, int32 Slot);

	/** Every unit known here, whoever owns it (hol.Units.Order ... all). False without a controller. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool AllUnitIds(AHOLPlayerController* Controller, TArray<int32>& OutIds);

	/** Controller->GetCommandUnits(): the units the selected orders go to (hol.Select.Heal / Attack). False without a controller. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool CommandUnitIds(AHOLPlayerController* Controller, TArray<int32>& OutIds);

	/** Controller->RequestHealUnit(Ids, TargetId, bQueue). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestHealUnitIds(AHOLPlayerController* Controller, const TArray<int32>& Ids, int32 TargetId, bool bQueue);

	/** Controller->RequestAttackUnit(Ids, TargetId, bQueue), then OnAttackOrderGiven.Broadcast(the target's place) when the target is known here. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestAttackUnitIds(AHOLPlayerController* Controller, const TArray<int32>& Ids, int32 TargetId, bool bQueue);

	/** Controller->RequestMoveUnits(Ids, (X, Y, 0)) (hol.Units.Order). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestMoveUnitIdsTo(AHOLPlayerController* Controller, const TArray<int32>& Ids, double X, double Y);

	/** The middle of the main centre of the player's own start slot (OutX, OutY); false without a player state or a start (hol.Select.OrderSourceNear, hol.Units.OrderNear). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool StartCentreOf(AHOLPlayerController* Controller, double& OutX, double& OutY);

	/**
	 * Logs what this machine thinks of a placement ("Build request: ..." at Display) then asks the server (the former controller RequestPlacementAt) for a foundation at the main centre
	 * of the player (or of an enemy) plus (Dx, Dy) cm in its frame. False without such a centre (nothing logged, nothing sent).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestPlacementNear(AHOLPlayerController* Controller, FName TypeName, double Dx, double Dy, bool bEnemy);

	/** The standing source nearest a ground point: its index (INDEX_NONE when none) and its place. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static int32 StandingSourceNear(AHOLPlayerController* Controller, double X, double Y, double& OutX, double& OutY);

	/** The same request as RequestPlacementNear, on the place of source SourceIndex. False when the index is not a source. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ControllerCommand")
	static bool RequestPlacementOnSource(AHOLPlayerController* Controller, FName TypeName, int32 SourceIndex);

private:
	static void RequestPlacementAtPoint(AHOLPlayerController& Controller, FName TypeName, const FVector& Point);
};
