// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Player/HOLCommandGrid.h"
#include "Player/HOLCommandGridReadLibrary.h"
#include "HOLCommandGridBoard.generated.h"

/**
 * The command grid's rules where the game runs (MIG-CPP, lot G1 bridge; D839: a binding to expose). The cells' content is the script's
 * (Script/Player/HOLCommandGridBoard.as calls namespace HOLCommandGridRules of Script/Player/HOLCommandGridRules.as, tested against the C++ by the
 * agreement tests); this world subsystem asks its script subclass. Abstract: only a script subclass is born. HOLCommandGrid::GetCommand and GetOrder
 * ask it first (Ask*); with no script board in any game world they keep their C++ body (HOLCommandGrid::GetCommandNative / GetOrderNative, the twin
 * kept until the proof, removed in a later lot). The read library the agreement tests compare against calls the Native bodies, never this bridge.
 * Same pattern as UHOLPlayerOrdersBoard and UHOLConstructionBoard. Game thread only (the script runs there; another thread keeps the C++ body).
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLCommandGridBoard : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The board of the first game or PIE world that has one (the grid's functions have no world argument), or null. */
	static UHOLCommandGridBoard* FindAny();

	/** True and Out filled when a script board answered; false: the caller keeps its C++ body. */
	static bool AskGetCommand(const FHOLGridContext& Context, int32 Cell, FHOLGridCommand& Out);
	static bool AskGetOrder(const FHOLGridContext& Context, int32 Cell, FHOLGridCommand& Out);

	/** FHOLGridContext as the view the script reads (the inverse of the read library's ToRule). */
	static FHOLGridContextView MakeContextView(const FHOLGridContext& Context);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** The script subclass overrides these (HOLCommandGridRules::GetCommand / GetOrder); the default is an empty command (the board existing is the answer). */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	FHOLGridCommandView GetCommandAnswer(const FHOLGridContextView& Context, int32 Cell);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	FHOLGridCommandView GetOrderAnswer(const FHOLGridContextView& Context, int32 Cell);
	virtual FHOLGridCommandView GetCommandAnswer_Implementation(const FHOLGridContextView& Context, int32 Cell);
	virtual FHOLGridCommandView GetOrderAnswer_Implementation(const FHOLGridContextView& Context, int32 Cell);
};
