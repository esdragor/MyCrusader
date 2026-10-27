// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Match/HOLResources.h"
#include "Simulation/HOLHeroSheet.h"
#include "Engine/NetSerialization.h"
#include "HOLPlayerState.generated.h"

/**
 * One step of a chain of queued orders as its owner receives it (D328, J7b.5b-1b): its kind (HOLOrderChain::EStepKind)
 * and where it is: the point of a move, the site, the source (fixed); an attack: the unit taken on (D223 identifier, the
 * same on every machine), placed by the receiver where it shows it, Location being where it stood when the chain changed.
 */
USTRUCT()
struct FHOLNetChainStep
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 Kind = 0;

	/** Whole centimetres, packed (FVector_NetQuantize). */
	UPROPERTY()
	FVector_NetQuantize Location = FVector::ZeroVector;

	/** Attack only: the unit taken on; 0 otherwise. */
	UPROPERTY()
	uint32 TargetId = 0;

	/** An attack's Location is only a fallback: its target walking does not make the chain change (nothing resent). */
	bool operator==(const FHOLNetChainStep& Other) const
	{
		return Kind == Other.Kind && TargetId == Other.TargetId && (TargetId != 0 || Location == Other.Location);
	}
};

/** A chain of queued orders as its owner receives it (D328): its units (D223 identifiers), its steps, head first. */
USTRUCT()
struct FHOLNetOrderChain
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<uint32> UnitIds;

	UPROPERTY()
	TArray<FHOLNetChainStep> Steps;

	bool operator==(const FHOLNetOrderChain& Other) const { return UnitIds == Other.UnitIds && Steps == Other.Steps; }
};

/**
 * A player of the match: the slot it holds in the match configuration (D244), which is the index its units carry
 * as OwnerPlayer (D233), and the colour the slot chose. Its economy (J3, D282): the stock of resources and the
 * population (places used, cap), set by the server alone and replicated to this player only (D203: nobody sees
 * another player's resources); and how many of its units gather each resource (J5b, D307), for the top bar of its HUD.
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

	/** The player's units gathering each resource (J5b, D307): on their round trip or at their source. */
	const FHOLResources& GetGatherers() const { return Gatherers; }

	/** Server only: the stock (the game mode at the start, then the production). */
	void SetResources(const FHOLResources& InResources);
	/** Server only: the population, counted by the production at each step. */
	void SetPopulation(int32 InUsed, int32 InCap);
	/** Server only: the gatherers by resource, counted by the gathering at each step. */
	void SetGatherers(const FHOLResources& InGatherers);

	/** The economy changed: on the server when it is set, on the owner's machine when it arrives. For the HUD. */
	DECLARE_MULTICAST_DELEGATE(FOnEconomyChanged);
	FOnEconomyChanged OnEconomyChanged;

	/**
	 * The chains of queued orders (Shift) of the player's units (D328, J7b.5b-1b): set by the server only when they change
	 * (UHOLUnitOrderProcessor, at most every 0.25 s and only while the player has some or just lost them), replicated to
	 * this player only. Solo and host: the same array, read where it is set. Empty without a queue: nothing is sent.
	 */
	const TArray<FHOLNetOrderChain>& GetOrderChains() const { return OrderChains; }
	/** Server only: true when the chains changed (and are then sent). */
	bool SetOrderChains(TArray<FHOLNetOrderChain>&& InChains);
	/**
	 * Where a received step is on this machine: an attack at its target as shown here (client: the displayed unit; solo or
	 * server: the simulated one), else at its Location (a target no longer known: its fallback, false returned).
	 */
	static bool LocateChainStep(const UWorld& World, const FHOLNetChainStep& Step, FVector& OutLocation);

	/** The chains changed: on the server when set, on the owner's machine when they arrive. For the drawing (J7b.5b-2). */
	DECLARE_MULTICAST_DELEGATE(FOnOrderChainsChanged);
	FOnOrderChainsChanged OnOrderChainsChanged;

	/**
	 * J8a.3 (J8-Q5): the private part of the player's hero sheet (XP, cooldowns, death step), set by the server only when
	 * it changes, replicated to this player only (like the stock, D270).
	 */
	const FHOLHeroPrivateSheet& GetHeroPrivate() const { return HeroPrivate; }
	/** Server only: true when it changed (and is then sent at once). */
	bool SetHeroPrivate(const FHOLHeroPrivateSheet& InHeroPrivate);

	/** The private sheet changed: on the server when set, on the owner's machine when it arrives. */
	DECLARE_MULTICAST_DELEGATE(FOnHeroPrivateChanged);
	FOnHeroPrivateChanged OnHeroPrivateChanged;

	/** "F 200 W 200 S 200 G 200, population 0/10": for the logs of the scripted checks. */
	FString DescribeEconomy() const;

private:
	UFUNCTION()
	void OnRep_Economy();

	UFUNCTION()
	void OnRep_OrderChains();

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

	UPROPERTY(ReplicatedUsing = OnRep_Economy)
	FHOLResources Gatherers;

	UPROPERTY(ReplicatedUsing = OnRep_OrderChains)
	TArray<FHOLNetOrderChain> OrderChains;

	UFUNCTION()
	void OnRep_HeroPrivate();

	UPROPERTY(ReplicatedUsing = OnRep_HeroPrivate)
	FHOLHeroPrivateSheet HeroPrivate;
};
