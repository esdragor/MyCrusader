// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLPlayerState.h"

#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "Net/HOLUnitReceptionSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Simulation/HOLOrderChainRules.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

void AHOLPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHOLPlayerState, PlayerIndex);
	DOREPLIFETIME(AHOLPlayerState, ColorIndex);
	DOREPLIFETIME_CONDITION(AHOLPlayerState, Resources, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHOLPlayerState, PopulationUsed, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHOLPlayerState, PopulationCap, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHOLPlayerState, Gatherers, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHOLPlayerState, OrderChains, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHOLPlayerState, HeroPrivate, COND_OwnerOnly);
}

int32 AHOLPlayerState::FindColorIndex(const UWorld& World, const int32 Slot)
{
	if (const AGameStateBase* GameState = World.GetGameState())
	{
		for (const APlayerState* PlayerState : GameState->PlayerArray)
		{
			const AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState);
			if (HOLPlayerState && HOLPlayerState->GetPlayerIndex() == Slot)
			{
				return HOLPlayerState->GetColorIndex();
			}
		}
	}
	return Slot;
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

void AHOLPlayerState::SetGatherers(const FHOLResources& InGatherers)
{
	if (Gatherers == InGatherers)
	{
		return;
	}
	Gatherers = InGatherers;
	OnEconomyChanged.Broadcast();
	ForceNetUpdate();
}

bool AHOLPlayerState::SetOrderChains(TArray<FHOLNetOrderChain>&& InChains)
{
	if (OrderChains == InChains)
	{
		return false;
	}
	OrderChains = MoveTemp(InChains);
	OnOrderChainsChanged.Broadcast();
	// As the economy: a player state sends once a second by default; a queue given is shown at once.
	ForceNetUpdate();
	return true;
}

bool AHOLPlayerState::LocateChainStep(const UWorld& World, const FHOLNetChainStep& Step, FVector& OutLocation)
{
	OutLocation = Step.Location;
	if (Step.Kind != static_cast<uint8>(HOLOrderChain::EStepKind::Attack) || Step.TargetId == 0)
	{
		return true;
	}
	if (const UHOLUnitReceptionSubsystem* Reception = World.GetNetMode() == NM_Client ? World.GetSubsystem<UHOLUnitReceptionSubsystem>() : nullptr)
	{
		FHOLUnitView View;
		if (Reception->GetDisplayedUnit(Step.TargetId, View))
		{
			OutLocation = View.Location;
			return true;
		}
		return false;
	}
	const UHOLUnitSubsystem* Units = World.GetSubsystem<UHOLUnitSubsystem>();
	const UMassEntitySubsystem* EntitySubsystem = World.GetSubsystem<UMassEntitySubsystem>();
	if (Units && EntitySubsystem)
	{
		const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
		const FMassEntityHandle Entity = Units->FindEntity(Step.TargetId);
		if (const FTransformFragment* Transform = EntityManager.IsEntityValid(Entity) ? EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity) : nullptr)
		{
			OutLocation = Transform->GetTransform().GetLocation();
			return true;
		}
	}
	return false;
}

void AHOLPlayerState::OnRep_OrderChains()
{
	OnOrderChainsChanged.Broadcast();
}

FString AHOLPlayerState::DescribeEconomy() const
{
	return FString::Printf(TEXT("%s, population %d/%d"), *Resources.ToString(), PopulationUsed, PopulationCap);
}

void AHOLPlayerState::OnRep_Economy()
{
	OnEconomyChanged.Broadcast();
}

bool AHOLPlayerState::SetHeroPrivate(const FHOLHeroPrivateSheet& InHeroPrivate)
{
	if (HeroPrivate == InHeroPrivate)
	{
		return false;
	}
	HeroPrivate = InHeroPrivate;
	OnHeroPrivateChanged.Broadcast();
	// As the economy: a player state sends once a second by default.
	ForceNetUpdate();
	return true;
}

void AHOLPlayerState::OnRep_HeroPrivate()
{
	OnHeroPrivateChanged.Broadcast();
}
