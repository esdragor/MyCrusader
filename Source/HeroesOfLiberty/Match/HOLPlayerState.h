// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Match/HOLResources.h"
#include "HOLPlayerState.generated.h"

/**
 * A player of the match: the slot it holds in the match configuration (D244), which is the index its units carry
 * as OwnerPlayer (D233), and the colour the slot chose. Its economy (J3, D282): the stock of resources and the
 * population (places used, cap), set by the server alone and replicated to this player only (D203: nobody sees
 * another player's resources).
 */
UCLASS()
class HEROESOFLIBERTY_API AHOLPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	int32 GetPlayerIndex() const { return PlayerIndex; }
	int32 GetColorIndex() const { return ColorIndex; }
	/** The colour the player of a slot chose, as this machine knows it; the slot's own index until its state arrives. */
	static int32 FindColorIndex(const UWorld& World, int32 Slot);

	/** Server only, by the game mode. */
	void SetMatchSlot(int32 InPlayerIndex, int32 InColorIndex);

	const FHOLResources& GetResources() const { return Resources; }
	/** Places taken by the player's units and by the units in production (D282). */
	int32 GetPopulationUsed() const { return PopulationUsed; }
	/** Places the player's buildings give, 175 at most (D03, D83). */
	int32 GetPopulationCap() const { return PopulationCap; }

	/** Server only: the stock (the game mode at the start, then the production). */
	void SetResources(const FHOLResources& InResources);
	/** Server only: the population, counted by the production at each step. */
	void SetPopulation(int32 InUsed, int32 InCap);

	/** The economy changed: on the server when it is set, on the owner's machine when it arrives. For the HUD. */
	DECLARE_MULTICAST_DELEGATE(FOnEconomyChanged);
	FOnEconomyChanged OnEconomyChanged;

	/** "F 200 W 200 S 200 G 200, population 0/10": for the logs of the scripted checks. */
	FString DescribeEconomy() const;

private:
	UFUNCTION()
	void OnRep_Economy();

	/** Slot index in the match configuration, INDEX_NONE for a player with no slot. */
	UPROPERTY(Replicated)
	int32 PlayerIndex = INDEX_NONE;

	UPROPERTY(Replicated)
	int32 ColorIndex = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Economy)
	FHOLResources Resources;

	UPROPERTY(ReplicatedUsing = OnRep_Economy)
	int32 PopulationUsed = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Economy)
	int32 PopulationCap = 0;
};
