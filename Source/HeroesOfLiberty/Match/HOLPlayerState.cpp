// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLPlayerState.h"

#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "Net/HOLUnitReceptionSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Simulation/HOLOrderChainRules.h"
#include "Simulation/HOLSiteSubsystem.h"
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
	DOREPLIFETIME_CONDITION(AHOLPlayerState, GatherIncome, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHOLPlayerState, OrderChains, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHOLPlayerState, HeroPrivate, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHOLPlayerState, Technologies, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AHOLPlayerState, SiteViews, COND_OwnerOnly);
}

bool AHOLPlayerState::SetSiteViews(const TArray<FHOLSiteView>& InSiteViews)
{
	if (SiteViews == InSiteViews)
	{
		return false;
	}
	SiteViews = InSiteViews;
	ForceNetUpdate();
	return true;
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

FHOLResources AHOLPlayerState::GetIncomePerMinute() const
{
	// D780, D788: its gatherers' income, and for the gold the sacred sites it holds (each pays hol.Site.GoldPerMinute).
	FHOLResources Income = GatherIncome;
	const int32 Slot = GetPlayerIndex();
	if (Slot != INDEX_NONE)
	{
		int32 Held = 0;
		for (const FHOLSiteView& View : SiteViews)
		{
			Held += View.Owner == Slot ? 1 : 0;
		}
		Income.Gold += Held * UHOLSiteSubsystem::GetGoldPerMinute();
	}
	return Income;
}

void AHOLPlayerState::SetGatherIncome(const FHOLResources& InIncome)
{
	if (GatherIncome == InIncome)
	{
		return;
	}
	GatherIncome = InIncome;
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

bool AHOLPlayerState::AddTechnology(const FName Technology)
{
	static const FName RamHealthStat(TEXT("RamHealth"));
	const int32 HealthBefore = GetTechnologyBonusPercent(RamHealthStat);
	if (!Technologies.Add(Technology))
	{
		return false;
	}
	TechnologyBonus.Reset();
	// S5 (Reinforced Ram): the Rams alive gain the new health at once, those born later are born with it (UHOLUnitSubsystem).
	const int32 HealthAfter = GetTechnologyBonusPercent(RamHealthStat);
	if (HealthAfter != HealthBefore && GetWorld() != nullptr)
	{
		if (UHOLUnitSubsystem* Units = GetWorld()->GetSubsystem<UHOLUnitSubsystem>())
		{
			const int32 Raised = Units->ApplyTechnologyHealth(PlayerIndex, HealthBefore, HealthAfter);
			UE_LOG(LogTemp, Display, TEXT("Technology: slot %d RamHealth +%d %%: %d units raised"), PlayerIndex, HealthAfter, Raised);
		}
	}
	OnTechnologiesChanged.Broadcast();
	ForceNetUpdate();
	return true;
}

void AHOLPlayerState::RestoreTechnologies(const FHOLTechnologyState& InTechnologies)
{
	Technologies = InTechnologies;
	TechnologyBonus.Reset();
	OnTechnologiesChanged.Broadcast();
	ForceNetUpdate();
}

void AHOLPlayerState::OnRep_Technologies()
{
	TechnologyBonus.Reset();
	OnTechnologiesChanged.Broadcast();
}

int32 AHOLPlayerState::GetTechnologyBonusPercent(const FName Stat) const
{
	if (Technologies.Done.IsEmpty())
	{
		return 0;
	}
	if (const int32* Known = TechnologyBonus.Find(Stat))
	{
		return *Known;
	}
	const int32 Percent = HOLTechnology::GetBonusPercent(Technologies.Done, Stat,
		[](const FName Name) { return UHOLTechnologySettings::FindTechnology(Name); });
	TechnologyBonus.Add(Stat, Percent);
	return Percent;
}
