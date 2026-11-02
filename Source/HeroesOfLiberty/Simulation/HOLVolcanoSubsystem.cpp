// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLVolcanoSubsystem.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Commands/HOLPlayerOrders.h"
#include "EngineUtils.h"
#include "Match/HOLGameState.h"
#include "Simulation/HOLDuelSubsystem.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

DEFINE_LOG_CATEGORY_STATIC(LogHOLVolcano, Log, All);

bool UHOLVolcanoSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLVolcanoSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	BindSteps();
}

void UHOLVolcanoSubsystem::BindSteps()
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client || StepStartedHandle.IsValid())
	{
		return;
	}
	if (UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>())
	{
		StepStartedHandle = Simulation->GetOnStepStarted().AddUObject(this, &UHOLVolcanoSubsystem::OnStepStarted);
	}
}

void UHOLVolcanoSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		if (UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>())
		{
			Simulation->GetOnStepStarted().Remove(StepStartedHandle);
		}
	}
	StepStartedHandle.Reset();
	Super::Deinitialize();
}

float UHOLVolcanoSubsystem::GetClockOf(const int64 StepIndex) const
{
	return static_cast<float>(StepIndex) * GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
}

void UHOLVolcanoSubsystem::OnStepStarted(const int64 StepIndex)
{
	UWorld* World = GetWorld();
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	if (Sites.IsEmpty() || !GameState || !GameState->GetLaunchConfig().bCatastrophe || !Units)
	{
		return;
	}
	// The Man-at-arms' speed, read from the unit type table (never typed here): the lava walks at a share of it (D575).
	const FHOLUnitTypeRow* ManAtArms = Units->FindTypeRow(TEXT("ManAtArms"));
	if (!ManAtArms)
	{
		return;
	}
	const float Now = GetClockOf(StepIndex);
	for (int32 Index = 0; Index < Sites.Num(); ++Index)
	{
		const HOLVolcano::FSiteState State = HOLVolcano::SiteStateOfMatch(GameState->GetLaunchConfig(), Index, Sites[Index], Now, Settings, ManAtArms->Speed);
		Apply(Index, Sites[Index], State);
	}
}

int32 UHOLVolcanoSubsystem::GetEvacuations(const int32 Slot, TArray<FEvacuation>& Out) const
{
	Out.Reset();
	UWorld* World = GetWorld();
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	const UHOLSimulationSubsystem* Simulation = World ? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	if (Sites.IsEmpty() || !GameState || !GameState->GetLaunchConfig().bCatastrophe || !Units || !Simulation)
	{
		return 0;
	}
	const FHOLUnitTypeRow* ManAtArms = Units->FindTypeRow(TEXT("ManAtArms"));
	if (!ManAtArms)
	{
		return 0;
	}
	const float Now = GetClockOf(Simulation->GetStepIndex());
	// The flows now threatening: the site's drawn flow while it warns or erupts.
	TArray<const HOLVolcano::FFlow*> Threats;
	for (int32 Index = 0; Index < Sites.Num(); ++Index)
	{
		const HOLVolcano::FSiteState State = HOLVolcano::SiteStateOfMatch(GameState->GetLaunchConfig(), Index, Sites[Index], Now, Settings, ManAtArms->Speed);
		if ((State.Phase == EHOLVolcanoPhase::Warning || State.Phase == EHOLVolcanoPhase::Erupting) && Sites[Index].Flows.IsValidIndex(State.FlowIndex))
		{
			Threats.Add(&Sites[Index].Flows[State.FlowIndex]);
		}
	}
	if (Threats.IsEmpty())
	{
		return 0;
	}
	UHOLDuelSubsystem* Duels = UHOLDuelSubsystem::Find(World);
	const bool bHeroInDuel = Duels && Duels->GetAdversaryNow(Slot) >= 0;
	const uint32 Hero = Units->FindHeroUnit(Slot);
	TArray<FHOLUnitView> Views;
	Units->GetUnitViews(Views);
	for (const FHOLUnitView& View : Views)
	{
		if (View.OwnerPlayer != Slot || View.Health <= 0 || (bHeroInDuel && View.UnitId == Hero))
		{
			continue;
		}
		for (const HOLVolcano::FFlow* Flow : Threats)
		{
			if (HOLVolcano::IsInCorridor(*Flow, Settings.LavaHalfWidth, View.Location, View.Radius))
			{
				FEvacuation& Entry = Out.AddDefaulted_GetRef();
				Entry.UnitId = View.UnitId;
				Entry.Target = HOLVolcano::EvacuationPoint(*Flow, Settings.LavaHalfWidth, View.Location, View.Radius);
				break;
			}
		}
	}
	return Out.Num();
}

int32 UHOLVolcanoSubsystem::EvacuateOwner(const int32 Slot)
{
	UWorld* World = GetWorld();
	TArray<FEvacuation> Entries;
	if (!World || GetEvacuations(Slot, Entries) == 0)
	{
		return 0;
	}
	int32 Ordered = 0;
	for (const FEvacuation& Entry : Entries)
	{
		const uint32 Ids[1] = { Entry.UnitId };
		Ordered += HOLPlayerOrders::Move(*World, Slot, MakeArrayView(Ids, 1), Entry.Target) ? 1 : 0;
	}
	UE_LOG(LogHOLVolcano, Display, TEXT("Volcano evacuation: slot %d, %d of %d units leave the corridor (margin = LavaHalfWidth %.0f cm + each body's radius)"),
		Slot, Ordered, Entries.Num(), Settings.LavaHalfWidth);
	return Ordered;
}

void UHOLVolcanoSubsystem::Apply(const int32 SiteIndex, const HOLVolcano::FSite& Site, const HOLVolcano::FSiteState& State)
{
	Burn(Site, State);
	SyncObstacle(SiteIndex, Site, State);
}

void UHOLVolcanoSubsystem::SyncObstacle(const int32 SiteIndex, const HOLVolcano::FSite& Site, const HOLVolcano::FSiteState& State)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const bool bWanted = State.Phase == EHOLVolcanoPhase::Cooling && Site.Flows.IsValidIndex(State.FlowIndex);
	AHOLLavaObstacle* Existing = FindObstacle(SiteIndex);
	if (Existing && (!bWanted || Existing->GetCycleIndex() != State.CycleIndex))
	{
		Existing->Destroy();
		Obstacles.Remove(SiteIndex);
		Existing = nullptr;
	}
	if (bWanted && !Existing)
	{
		AHOLLavaObstacle* Obstacle = World->SpawnActor<AHOLLavaObstacle>();
		if (Obstacle)
		{
			Obstacle->SetCycleIndex(State.CycleIndex);
			Obstacle->Build(HOLVolcano::GetObstacleBoxes(Site.Flows[State.FlowIndex], Settings.LavaHalfWidth), ObstacleHeight);
			Obstacles.Add(SiteIndex, Obstacle);
		}
	}
}

int32 UHOLVolcanoSubsystem::Burn(const HOLVolcano::FSite& Site, const HOLVolcano::FSiteState& State)
{
	UWorld* World = GetWorld();
	if (!World || State.Phase != EHOLVolcanoPhase::Erupting || !Site.Flows.IsValidIndex(State.FlowIndex))
	{
		return 0;
	}
	const HOLVolcano::FFlow& Flow = Site.Flows[State.FlowIndex];
	int32 Burnt = 0;

	if (UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>())
	{
		UHOLDuelSubsystem* Duels = UHOLDuelSubsystem::Find(World);
		const int32 Killed = Units->KillUnitsWhere([&](const uint32 UnitId, FName, const uint8 Owner, const FVector& Location, const float Radius)
		{
			if (!HOLVolcano::IsInLava(Flow, State.FrontDistance, Settings.LavaHalfWidth, Location, Radius))
			{
				return false;
			}
			const bool bHero = Units->FindHeroUnit(Owner) == UnitId;
			return HOLVolcano::LavaDestroys(bHero, bHero && Duels && Duels->GetAdversaryNow(Owner) >= 0);
		});
		NumUnitsBurnt += Killed;
		Burnt += Killed;
	}

	for (TActorIterator<AHOLBuilding> It(World); It; ++It)
	{
		AHOLBuilding* Building = *It;
		if (!IsValid(Building) || Building->GetHealth() <= 0)
		{
			continue;
		}
		const FBox2D Footprint = Building->GetFootprintBounds();
		const FVector2D Centre = Footprint.GetCenter();
		const float Radius = static_cast<float>(0.5 * Footprint.GetSize().GetMax());
		if (HOLVolcano::IsInLava(Flow, State.FrontDistance, Settings.LavaHalfWidth, FVector(Centre.X, Centre.Y, 0.0), Radius))
		{
			// As any blow: a main forum falls into ruin and costs a life (V5-Q2 = A), another building is destroyed.
			if (UHOLConstructionSubsystem::DamageBuilding(*Building, Building->GetMaxHealth() + 1, EHOLAttackType::Siege) > 0)
			{
				++NumBuildingsBurnt;
				++Burnt;
			}
		}
	}
	if (Burnt > 0)
	{
		UE_LOG(LogHOLVolcano, Display, TEXT("Lava front at %.0f cm of flow %d burnt %d (units and buildings)"), State.FrontDistance, State.FlowIndex, Burnt);
	}
	return Burnt;
}