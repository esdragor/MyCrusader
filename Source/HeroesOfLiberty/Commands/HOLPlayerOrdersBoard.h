// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLPlayerOrdersBoard.generated.h"

class AHOLBuilding;

/**
 * The rule of the player's orders where the match runs (MIG-CPP step 5, lot P1b; D839: a binding to expose; the queue and the scope of the player's
 * order stay in UHOLCommandSubsystem). The verdict is the script's (Script/Commands/HOLPlayerOrdersBoard.as calls namespace HOLPlayerOrders of
 * Script/Commands/HOLPlayerOrders.as, tested in Script/Commands/HOLPlayerOrders_Test.as); this world subsystem asks its script subclass
 * (UHOLPlayerOrdersBoardScript). Abstract: only a script subclass is born. Every Ask* answers 1 (order accepted and queued), 0 (refused), or -1 when
 * the world has no script board (HOLPlayerOrders.cpp then REFUSES the order and logs it once; the C++ body is gone, the rule has one source). Same pattern as UHOLMatchFlowSubsystem (pause) and UHOLConstructionBoard (construction).
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLPlayerOrdersBoard : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The world's board (its script subclass), or null (no script compiled, or no world). */
	static UHOLPlayerOrdersBoard* Find(const UWorld* World);
	/** The board of the first game or PIE world that has one (for IsAccepted, which has no world argument), or null. Game thread only. */
	static UHOLPlayerOrdersBoard* FindAny();

	static int32 AskIsAccepted(const UWorld* World, int32 PlayerIndex, int32 NumUnits);
	/** IsAccepted without a world: the board of any game world, or -1 (no board: HOLPlayerOrders::IsAccepted refuses). */
	static int32 AskIsAcceptedAnywhere(int32 PlayerIndex, int32 NumUnits);
	static int32 AskMove(const UWorld* World, int32 PlayerIndex, const TArray<int32>& UnitIds, const FVector& Target, bool bQueue, const FVector2D& FrontLine,
		bool bFrontWidth);
	static int32 AskAttack(const UWorld* World, int32 PlayerIndex, const TArray<int32>& UnitIds, int32 TargetUnitId, bool bQueue);
	static int32 AskAttackBuilding(const UWorld* World, int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building, bool bQueue);
	static int32 AskAttackMove(const UWorld* World, int32 PlayerIndex, const TArray<int32>& UnitIds, const FVector& Target, bool bQueue);
	static int32 AskStop(const UWorld* World, int32 PlayerIndex, const TArray<int32>& UnitIds, bool bHold);
	static int32 AskToggleDeploy(const UWorld* World, int32 PlayerIndex, const TArray<int32>& UnitIds);

	// Lot P2 (second half): the workers of Gather / Build / DropOff are already filtered by C++ (SendNonWorkersTo, HOLUnitView); bSource, SourceLocation, bDeposit, bSourceStanding are game data computed by C++.
	static int32 AskGather(const UWorld* World, int32 PlayerIndex, const TArray<int32>& UnitIds, int32 SourceIndex, bool bQueue, bool bSource, FVector SourceLocation);
	static int32 AskBuild(const UWorld* World, int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building, bool bQueue);
	static int32 AskDropOff(const UWorld* World, int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building, FVector Point, bool bQueue, bool bDeposit);
	static int32 AskPlace(const UWorld* World, int32 PlayerIndex, FName TypeName, FVector Point, const TArray<int32>& UnitIds, bool bQueue);
	static int32 AskQueueProduction(const UWorld* World, int32 PlayerIndex, AHOLBuilding* Building, FName TypeName);
	static int32 AskMarketTrade(const UWorld* World, int32 PlayerIndex, AHOLBuilding* Building, int32 Resource, int32 Lots, bool bBuy);
	static int32 AskSetRally(const UWorld* World, int32 PlayerIndex, AHOLBuilding* Building, FVector Point, int32 SourceIndex, bool bSourceStanding, AHOLBuilding* Site, bool bQueue);
	static int32 AskGarrison(const UWorld* World, int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building);
	static int32 AskUngarrisonAll(const UWorld* World, int32 PlayerIndex, AHOLBuilding* Building);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** The script subclass overrides these (HOLPlayerOrders::...); the default is -1 (no rule here). */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Commands")
	int32 IsAcceptedAnswer(int32 PlayerIndex, int32 NumUnits);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Commands")
	int32 MoveAnswer(int32 PlayerIndex, const TArray<int32>& UnitIds, FVector Target, bool bQueue, FVector2D FrontLine, bool bFrontWidth);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Commands")
	int32 AttackAnswer(int32 PlayerIndex, const TArray<int32>& UnitIds, int32 TargetUnitId, bool bQueue);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Commands")
	int32 AttackBuildingAnswer(int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building, bool bQueue);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Commands")
	int32 AttackMoveAnswer(int32 PlayerIndex, const TArray<int32>& UnitIds, FVector Target, bool bQueue);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Commands")
	int32 StopAnswer(int32 PlayerIndex, const TArray<int32>& UnitIds, bool bHold);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Commands")
	int32 ToggleDeployAnswer(int32 PlayerIndex, const TArray<int32>& UnitIds);
	virtual int32 IsAcceptedAnswer_Implementation(int32 PlayerIndex, int32 NumUnits);
	virtual int32 MoveAnswer_Implementation(int32 PlayerIndex, const TArray<int32>& UnitIds, FVector Target, bool bQueue, FVector2D FrontLine, bool bFrontWidth);
	virtual int32 AttackAnswer_Implementation(int32 PlayerIndex, const TArray<int32>& UnitIds, int32 TargetUnitId, bool bQueue);
	virtual int32 AttackBuildingAnswer_Implementation(int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building, bool bQueue);
	virtual int32 AttackMoveAnswer_Implementation(int32 PlayerIndex, const TArray<int32>& UnitIds, FVector Target, bool bQueue);
	virtual int32 StopAnswer_Implementation(int32 PlayerIndex, const TArray<int32>& UnitIds, bool bHold);
	virtual int32 ToggleDeployAnswer_Implementation(int32 PlayerIndex, const TArray<int32>& UnitIds);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Commands")
	int32 GatherAnswer(int32 PlayerIndex, const TArray<int32>& UnitIds, int32 SourceIndex, bool bQueue, bool bSource, FVector SourceLocation);
	virtual int32 GatherAnswer_Implementation(int32 PlayerIndex, const TArray<int32>& UnitIds, int32 SourceIndex, bool bQueue, bool bSource, FVector SourceLocation);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Commands")
	int32 BuildAnswer(int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building, bool bQueue);
	virtual int32 BuildAnswer_Implementation(int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building, bool bQueue);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Commands")
	int32 DropOffAnswer(int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building, FVector Point, bool bQueue, bool bDeposit);
	virtual int32 DropOffAnswer_Implementation(int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building, FVector Point, bool bQueue, bool bDeposit);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Commands")
	int32 PlaceAnswer(int32 PlayerIndex, FName TypeName, FVector Point, const TArray<int32>& UnitIds, bool bQueue);
	virtual int32 PlaceAnswer_Implementation(int32 PlayerIndex, FName TypeName, FVector Point, const TArray<int32>& UnitIds, bool bQueue);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Commands")
	int32 QueueProductionAnswer(int32 PlayerIndex, AHOLBuilding* Building, FName TypeName);
	virtual int32 QueueProductionAnswer_Implementation(int32 PlayerIndex, AHOLBuilding* Building, FName TypeName);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Commands")
	int32 MarketTradeAnswer(int32 PlayerIndex, AHOLBuilding* Building, int32 Resource, int32 Lots, bool bBuy);
	virtual int32 MarketTradeAnswer_Implementation(int32 PlayerIndex, AHOLBuilding* Building, int32 Resource, int32 Lots, bool bBuy);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Commands")
	int32 SetRallyAnswer(int32 PlayerIndex, AHOLBuilding* Building, FVector Point, int32 SourceIndex, bool bSourceStanding, AHOLBuilding* Site, bool bQueue);
	virtual int32 SetRallyAnswer_Implementation(int32 PlayerIndex, AHOLBuilding* Building, FVector Point, int32 SourceIndex, bool bSourceStanding, AHOLBuilding* Site, bool bQueue);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Commands")
	int32 GarrisonAnswer(int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building);
	virtual int32 GarrisonAnswer_Implementation(int32 PlayerIndex, const TArray<int32>& UnitIds, AHOLBuilding* Building);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Commands")
	int32 UngarrisonAllAnswer(int32 PlayerIndex, AHOLBuilding* Building);
	virtual int32 UngarrisonAllAnswer_Implementation(int32 PlayerIndex, AHOLBuilding* Building);
};
