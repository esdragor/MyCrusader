// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLPlayerState.h"

#include "Net/UnrealNetwork.h"

void AHOLPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHOLPlayerState, PlayerIndex);
	DOREPLIFETIME(AHOLPlayerState, ColorIndex);
	DOREPLIFETIME_CONDITION(AHOLPlayerState, Resources, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHOLPlayerState, PopulationUsed, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHOLPlayerState, PopulationCap, COND_OwnerOnly);
}

void AHOLPlayerState::SetMatchSlot(const int32 InPlayerIndex, const int32 InColorIndex)
{
	PlayerIndex = InPlayerIndex;
	ColorIndex = InColorIndex;
}

void AHOLPlayerState::SetResources(const FHOLResources& InResources)
{
	if (Resources == InResources)
	{
		return;
	}
	Resources = InResources;
	OnEconomyChanged.Broadcast();
	// A player state sends once a second by default (APlayerState): the bar lagged behind the production queue.
	ForceNetUpdate();
}

void AHOLPlayerState::SetPopulation(const int32 InUsed, const int32 InCap)
{
	if (PopulationUsed == InUsed && PopulationCap == InCap)
	{
		return;
	}
	PopulationUsed = InUsed;
	PopulationCap = InCap;
	OnEconomyChanged.Broadcast();
	ForceNetUpdate();
}

FString AHOLPlayerState::DescribeEconomy() const
{
	return FString::Printf(TEXT("%s, population %d/%d"), *Resources.ToString(), PopulationUsed, PopulationCap);
}

void AHOLPlayerState::OnRep_Economy()
{
	OnEconomyChanged.Broadcast();
}
