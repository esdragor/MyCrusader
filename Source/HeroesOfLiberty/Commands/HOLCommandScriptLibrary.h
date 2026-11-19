// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLCommandScriptLibrary.generated.h"

class AHOLBuilding;
class UHOLCommandSubsystem;

/**
 * One order read back from the command queue (lot M1 of PLAN-ENVELOPPE-1118.md, D851): the fields a script test or rule needs, whatever the queue it
 * came from (moves, gathers, builds, productions, constructions, rage, garrison). Fields a queue does not have stay at their default.
 */
USTRUCT(BlueprintType)
struct FHOLScriptOrderRead
{
	GENERATED_BODY()

	/** The kind's name: Move, Attack, AttackMove, Stop, Hold, AttackBuilding, DropOff, Gather, Build, ToggleDeploy, Other (a move-queue order); Queue, Cancel, Rally, Trade, Tribute, Place (production and construction); Garrison, UngarrisonAll; Rage; Gather, Build (their own queues). */
	UPROPERTY(BlueprintReadWrite) FName Kind;
	UPROPERTY(BlueprintReadWrite) int32 PlayerIndex = -1;
	UPROPERTY(BlueprintReadWrite) TArray<int32> UnitIds;
	UPROPERTY(BlueprintReadWrite) FVector Target = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite) int32 TargetUnitId = 0;
	UPROPERTY(BlueprintReadWrite) int32 SourceIndex = -1;
	UPROPERTY(BlueprintReadWrite) bool bQueue = false;
	UPROPERTY(BlueprintReadWrite) bool bFromTask = false;
	/** A move-queue order: its chain is drawn (D328; set when it was issued by a player-order emitter, Issue*Player). */
	UPROPERTY(BlueprintReadWrite) bool bChain = false;
	/** A building is named and still alive (TargetBuilding, Building, Deposit, RallySite). */
	UPROPERTY(BlueprintReadWrite) bool bHasBuilding = false;
	/** Production / construction: the type asked. */
	UPROPERTY(BlueprintReadWrite) FName TypeName;
	/** Production: the index (queue slot, cancel). */
	UPROPERTY(BlueprintReadWrite) int32 Index = -1;
};

/**
 * The order queue (UHOLCommandSubsystem, which stays C++: Mass reads it) for Angelscript (lot M1, D851, D839: a binding to expose). Emitters send each
 * kind of order the script's rules will give (the player's Do<X>, the AI, the rally points); readers take the queue as the simulation does (Consume*),
 * for a test. Every function takes the subsystem: GetCommands(World) in the game, MakeDetachedForTest() in a script test (no world needed: with no
 * world nothing is projected on the navmesh, clamped to the map or refused for a defeated slot). Unit ids are int32 here (script).
 * A rule has no copy: UHOLCommandSubsystem decides, this only carries.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLCommandScriptLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** The world's command subsystem (nullptr outside a game or PIE world). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Commands", meta = (WorldContext = "WorldContextObject"))
	static UHOLCommandSubsystem* GetCommands(const UObject* WorldContextObject);

	/** A subsystem of its own, outside any world, for a script test (a fresh empty queue each call). The caller keeps the reference. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static UHOLCommandSubsystem* MakeDetachedForTest();

	// ---- Emitters (the move queue) ----
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueMove(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, const FVector& Target, int32 PlayerIndex, bool bQueue = false, bool bFromTask = false);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueAttack(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, int32 TargetUnitId, int32 PlayerIndex, bool bQueue = false);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueAttackMove(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, const FVector& Target, int32 PlayerIndex, bool bQueue = false);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueStop(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, int32 PlayerIndex);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueHold(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, int32 PlayerIndex);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueAttackBuilding(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, AHOLBuilding* Building, int32 PlayerIndex, bool bQueue = false);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueDropOffMove(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, AHOLBuilding* Building, const FVector& Point, int32 PlayerIndex, bool bQueue = false);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueQueuedWork(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, int32 SourceIndex, AHOLBuilding* Building, const FVector& Location, int32 PlayerIndex);

	// ---- Emitters, player's orders (lot P1a, D328): the FPlayerOrderScope is opened INSIDE the C++ function (no state leaks to the script), so the order draws its chain (bChain). The rule of refusal (IsAccepted) is script (HOLPlayerOrders.as), not here. ----
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueMovePlayer(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, const FVector& Target, int32 PlayerIndex, bool bQueue, const FVector2D& FrontLine, bool bFrontWidth);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueAttackPlayer(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, int32 TargetUnitId, int32 PlayerIndex, bool bQueue);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueAttackMovePlayer(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, const FVector& Target, int32 PlayerIndex, bool bQueue);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueAttackBuildingPlayer(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, AHOLBuilding* Building, int32 PlayerIndex, bool bQueue);
	/** bHold: Hold, else Stop. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueStopPlayer(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, int32 PlayerIndex, bool bHold);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueToggleDeployPlayer(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, int32 PlayerIndex);
	/** FHOLMatchConfig::MaxSlots (the number of player slots), for the refusal rule of the script. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Commands")
	static int32 GetMaxSlots();

	// ---- Emitters (gather, build) ----
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueGather(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, int32 SourceIndex, int32 PlayerIndex, bool bQueue = false);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueDropOff(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, AHOLBuilding* Building, int32 PlayerIndex, bool bQueue = false);
	/** Build: Building a site to help (or SourceIndex a source to gather after, -1 for none). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueBuild(UHOLCommandSubsystem* Commands, const TArray<int32>& UnitIds, AHOLBuilding* Building, int32 SourceIndex, int32 PlayerIndex, bool bQueue = false);

	// ---- Emitters (production, construction) ----
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueProductionQueue(UHOLCommandSubsystem* Commands, int32 PlayerIndex, AHOLBuilding* Building, FName TypeName, bool bQueue = false);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueProductionCancel(UHOLCommandSubsystem* Commands, int32 PlayerIndex, AHOLBuilding* Building, int32 Index);
	/** The building's rally point: Location, SourceIndex (a source to gather, -1 none), RallySite (a site to help, nullptr none); Location on the building itself clears it. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueRally(UHOLCommandSubsystem* Commands, int32 PlayerIndex, AHOLBuilding* Building, const FVector& Location, int32 SourceIndex, AHOLBuilding* RallySite);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssuePlace(UHOLCommandSubsystem* Commands, int32 PlayerIndex, FName TypeName, const FVector& Location, const TArray<int32>& UnitIds, bool bQueue = false);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueConstructionCancel(UHOLCommandSubsystem* Commands, int32 PlayerIndex, AHOLBuilding* Building);

	// ---- Emitters (garrison, rage) ----
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueGarrison(UHOLCommandSubsystem* Commands, int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueUngarrisonAll(UHOLCommandSubsystem* Commands, int32 PlayerIndex, AHOLBuilding* Building);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static void IssueRage(UHOLCommandSubsystem* Commands, int32 PlayerIndex, const TArray<int32>& UnitIds);

	// ---- Readers (take and empty the queue, as the simulation does) ----
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static int32 GetNumPendingMoveOrders(const UHOLCommandSubsystem* Commands);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static TArray<FHOLScriptOrderRead> ConsumeMoves(UHOLCommandSubsystem* Commands);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static TArray<FHOLScriptOrderRead> ConsumeGathers(UHOLCommandSubsystem* Commands);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static TArray<FHOLScriptOrderRead> ConsumeBuilds(UHOLCommandSubsystem* Commands);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static TArray<FHOLScriptOrderRead> ConsumeProductions(UHOLCommandSubsystem* Commands);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static TArray<FHOLScriptOrderRead> ConsumeConstructions(UHOLCommandSubsystem* Commands);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static TArray<FHOLScriptOrderRead> ConsumeGarrisons(UHOLCommandSubsystem* Commands);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Commands")
	static TArray<FHOLScriptOrderRead> ConsumeRages(UHOLCommandSubsystem* Commands);
};
