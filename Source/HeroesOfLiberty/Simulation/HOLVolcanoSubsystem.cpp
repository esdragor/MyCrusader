// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLVolcanoSubsystem.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Commands/HOLPlayerOrders.h"
#include "EngineUtils.h"
#include "Map/HOLVolcanoLayout.h"
#include "Match/HOLGameState.h"
#include "Match/HOLVictoryRules.h"
#include "Resources/HOLSourceSubsystem.h"
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
	LoadLayout();
}

void UHOLVolcanoSubsystem::LoadLayout()
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return;
	}
	TArray<FHOLPlacedVolcano> Placed;
	bool bAnyLayout = false;
	for (TActorIterator<AHOLVolcanoLayout> It(World); It; ++It)
	{
		bAnyLayout = true;
		Placed.Append(It->GetSites());
	}
	if (!bAnyLayout)
	{
		return;
	}
	SetSites(FHOLVolcanoLayoutRules::ToSites(Placed));
	UE_LOG(LogHOLVolcano, Log, TEXT("%d volcano site(s) placed by the map"), Sites.Num());
}

void UHOLVolcanoSubsystem::SetSites(const TArray<HOLVolcano::FSite>& InSites)
{
	Sites = InSites;
	Wakes.Reset();
	Wakes.SetNum(Sites.Num());
	AltarHeld.Init(0.f, Sites.Num());
	AltarHolds.Init(HOLVolcano::EAltarHold::Idle, Sites.Num());
	AltarCycle.Init(INDEX_NONE, Sites.Num());
	AltarHolder.Init(INDEX_NONE, Sites.Num());
	FreshPlacedCycle.Init(INDEX_NONE, Sites.Num());
	FreshSources.Reset();
	FreshSources.SetNum(Sites.Num());
}

HOLVolcano::FSiteState UHOLVolcanoSubsystem::StateOf(const int32 SiteIndex, const float Now) const
{
	UWorld* World = GetWorld();
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	const FHOLUnitTypeRow* ManAtArms = Units ? Units->FindTypeRow(TEXT("ManAtArms")) : nullptr;
	if (!Sites.IsValidIndex(SiteIndex) || !GameState || !ManAtArms)
	{
		return HOLVolcano::FSiteState();
	}
	return HOLVolcano::SiteStateOfMatch(GameState->GetLaunchConfig(), SiteIndex, Sites[SiteIndex], Now, Settings, ManAtArms->Speed,
		Wakes.IsValidIndex(SiteIndex) ? &Wakes[SiteIndex] : nullptr);
}

HOLVolcano::FSiteState UHOLVolcanoSubsystem::GetSiteState(const int32 SiteIndex) const
{
	const UWorld* World = GetWorld();
	const UHOLSimulationSubsystem* Simulation = World ? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	return Simulation ? StateOf(SiteIndex, GetClockOf(Simulation->GetStepIndex())) : HOLVolcano::FSiteState();
}

bool UHOLVolcanoSubsystem::StepAltarOf(const int32 SiteIndex, const float Now, const float DeltaSeconds)
{
	UWorld* World = GetWorld();
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	if (!Sites.IsValidIndex(SiteIndex) || !AltarHeld.IsValidIndex(SiteIndex) || !GameState || !Units)
	{
		return false;
	}
	const HOLVolcano::FSite& Site = Sites[SiteIndex];
	const HOLVolcano::FSiteState State = StateOf(SiteIndex, Now);
	// Each cycle has its own count: the altar lights again at the re-arming (D400), from zero.
	if (AltarCycle[SiteIndex] != State.CycleIndex)
	{
		AltarCycle[SiteIndex] = State.CycleIndex;
		AltarHeld[SiteIndex] = 0.f;
		AltarHolder[SiteIndex] = INDEX_NONE;
	}
	if (!State.bAltarLit)
	{
		AltarHeld[SiteIndex] = 0.f;
		AltarHolder[SiteIndex] = INDEX_NONE;
		AltarHolds[SiteIndex] = HOLVolcano::EAltarHold::Idle;
		return false;
	}
	// The camps with a living unit in the zone (a Neutral owner is a camp of its own, an adversary: D401).
	const FHOLMatchConfig& Config = GameState->GetLaunchConfig();
	TArray<int32, TInlineAllocator<8>> Camps;
	const FVector2D Altar(Site.Altar.Location);
	const double RadiusSquared = FMath::Square(static_cast<double>(Site.Altar.Radius));
	// D6 (D369, D401): in the Diplomacy mode allies (both said Ally) in the zone are one camp, a Neutral is another one.
	const FHOLCampTable Relations = FHOLCampTable::FromWorld(GetWorld());
	TArray<int32, TInlineAllocator<8>> SeenOwners;
	Units->ForEachLivingUnit([&](const uint8 Owner, const FVector& Location)
	{
		if (FVector2D::DistSquared(FVector2D(Location), Altar) > RadiusSquared)
		{
			return;
		}
		if (Relations.IsDiplomacy() && Owner < FHOLMatchConfig::MaxSlots)
		{
			for (const int32 Seen : SeenOwners)
			{
				if (Seen == Owner || !Relations.AreHostile(Seen, Owner))
				{
					return; // an owner already counted, or an ally of one: the same camp
				}
			}
			SeenOwners.Add(Owner);
		}
		const int32 Camp = Owner < FHOLMatchConfig::MaxSlots ? HOLVictoryRules::GetCamp(Config, Owner) : INDEX_NONE;
		Camps.AddUnique(Camp != INDEX_NONE ? Camp : 1000 + Owner);
	});
	const int32 PreviousHolder = AltarHolder[SiteIndex];
	const float PreviousCount = AltarHeld[SiteIndex];
	AltarHolds[SiteIndex] = HOLVolcano::StepAltarOwned(AltarHeld[SiteIndex], AltarHolder[SiteIndex], Camps.Num(), Camps.Num() == 1 ? Camps[0] : INDEX_NONE, DeltaSeconds, Settings);
	if (PreviousHolder != INDEX_NONE && AltarHolder[SiteIndex] != PreviousHolder)
	{
		UE_LOG(LogHOLVolcano, Display, TEXT("Volcano altar of site %d changes holder at %.1f s: camp %d (was camp %d at %.0f%%) starts again from 0%%"),
			SiteIndex, Now, AltarHolder[SiteIndex], PreviousHolder, 100.f * PreviousCount / FMath::Max(Settings.AltarHoldSeconds, UE_KINDA_SMALL_NUMBER));
	}
	if (AltarHeld[SiteIndex] < Settings.AltarHoldSeconds)
	{
		return false;
	}
	Wakes[SiteIndex].Add(State.CycleIndex, Now);
	AltarHeld[SiteIndex] = 0.f;
	AltarHolder[SiteIndex] = INDEX_NONE;
	UE_LOG(LogHOLVolcano, Display, TEXT("Volcano altar of site %d held for %.0f s at %.1f s: the alert of cycle %d starts now"), SiteIndex, Settings.AltarHoldSeconds, Now, State.CycleIndex);
	return true;
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
	const float Now = GetClockOf(StepIndex);
	const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
	for (int32 Index = 0; Index < Sites.Num(); ++Index)
	{
		// The altar first (V8): a hold completed this step starts the alert in this very step's state.
		if (Sites[Index].Altar.bPresent)
		{
			StepAltarOf(Index, Now, StepSeconds);
		}
		// The Man-at-arms' speed is read from the unit type table (never typed here): the lava walks at a share of it (D575).
		Apply(Index, Sites[Index], StateOf(Index, Now));
	}
	PublishViews(Now);
}

FHOLVolcanoSiteNet UHOLVolcanoSubsystem::BuildNetView(const int32 SiteIndex, const float Now) const
{
	FHOLVolcanoSiteNet View;
	if (!Sites.IsValidIndex(SiteIndex))
	{
		return View;
	}
	const HOLVolcano::FSite& Site = Sites[SiteIndex];
	const HOLVolcano::FSiteState State = StateOf(SiteIndex, Now);
	View.Phase = static_cast<uint8>(State.Phase);
	View.Cycle = State.CycleIndex;
	View.FlowIndex = State.FlowIndex;
	View.SecondsToErupt = State.SecondsToErupt >= 0.f ? FMath::CeilToInt(State.SecondsToErupt) : -1;
	View.FrontDistance = FMath::RoundToInt(State.FrontDistance);
	if (!Site.Flows.IsEmpty() && !Site.Flows[0].Points.IsEmpty())
	{
		View.CraterX = Site.Flows[0].Points[0].X;
		View.CraterY = Site.Flows[0].Points[0].Y;
	}
	View.FlowLength = Site.Flows.IsValidIndex(State.FlowIndex) ? FMath::RoundToInt(Site.Flows[State.FlowIndex].Length()) : 0;
	View.bAltarPresent = Site.Altar.bPresent;
	View.bAltarLit = State.bAltarLit;
	View.bWoken = State.bWoken;
	View.AltarX = Site.Altar.Location.X;
	View.AltarY = Site.Altar.Location.Y;
	View.AltarRadius = Site.Altar.Radius;
	const float HoldSeconds = FMath::Max(Settings.AltarHoldSeconds, UE_KINDA_SMALL_NUMBER);
	const float Held = GetAltarHeldSeconds(SiteIndex);
	View.AltarPercent = static_cast<uint8>(FMath::Clamp(FMath::FloorToInt(100.f * Held / HoldSeconds), 0, 100));
	View.AltarSecondsLeft = FMath::Max(0, FMath::CeilToInt(HoldSeconds - Held));
	View.AltarHold = static_cast<uint8>(GetAltarHold(SiteIndex));
	return View;
}

void UHOLVolcanoSubsystem::PublishViews(const float Now) const
{
	const UWorld* World = GetWorld();
	AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	if (!GameState)
	{
		return;
	}
	TArray<FHOLVolcanoSiteNet> Views;
	for (int32 Index = 0; Index < Sites.Num(); ++Index)
	{
		Views.Add(BuildNetView(Index, Now));
	}
	GameState->SetVolcanoViews(Views);
}

bool UHOLVolcanoSubsystem::MeasureAltar(const int32 SiteIndex, const int32 Slot, FAltarMeasure& Out) const
{
	Out = FAltarMeasure();
	const UWorld* World = GetWorld();
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	const UHOLSimulationSubsystem* Simulation = World ? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	if (!Sites.IsValidIndex(SiteIndex) || !Sites[SiteIndex].Altar.bPresent || !GameState || !GameState->GetLaunchConfig().bCatastrophe || !Units || !Simulation)
	{
		return false;
	}
	if (!StateOf(SiteIndex, GetClockOf(Simulation->GetStepIndex())).bAltarLit)
	{
		return false;
	}
	const HOLVolcano::FSite& Site = Sites[SiteIndex];
	const FHOLMatchConfig& Config = GameState->GetLaunchConfig();
	const FHOLCampTable Relations = FHOLCampTable::FromWorld(GetWorld());
	const int32 MyCamp = Slot >= 0 && Slot < FHOLMatchConfig::MaxSlots ? HOLVictoryRules::GetCamp(Config, static_cast<uint8>(Slot)) : INDEX_NONE;
	Out.SiteIndex = SiteIndex;
	Out.Location = Site.Altar.Location;
	Out.Radius = Site.Altar.Radius;
	bool bOwn = false;
	const FVector2D Altar(Site.Altar.Location);
	const double RadiusSquared = FMath::Square(static_cast<double>(Site.Altar.Radius));
	Units->ForEachLivingUnitWithType([&](const uint32, const FName TypeName, const uint8 Owner, const FVector& Location)
	{
		if (FVector2D::DistSquared(FVector2D(Location), Altar) > RadiusSquared)
		{
			return;
		}
		const int32 Camp = Owner < FHOLMatchConfig::MaxSlots ? HOLVictoryRules::GetCamp(Config, Owner) : INDEX_NONE;
		const bool bMine = Owner == Slot || (Relations.IsDiplomacy() ? (Owner < FHOLMatchConfig::MaxSlots && !Relations.AreHostile(Slot, Owner))
			: (MyCamp != INDEX_NONE && Camp == MyCamp));
		if (bMine)
		{
			bOwn = true;
			return;
		}
		Out.bAdversary = true;
		Out.AdversaryStrength += TypeName == TEXT("Peasant") ? Settings.AiAltarWorkerWeight : 1.f;
	});
	Out.bOwnCampAlone = bOwn && !Out.bAdversary;
	Out.TraceFromAltar = TNumericLimits<float>::Max();
	Out.TraceFromBuildings = TNumericLimits<float>::Max();
	for (const HOLVolcano::FFlow& Flow : Site.Flows)
	{
		Out.TraceFromAltar = FMath::Min(Out.TraceFromAltar, HOLVolcano::DistanceToFlow(Flow, Site.Altar.Location));
	}
	for (TActorIterator<AHOLBuilding> It(const_cast<UWorld*>(World)); It; ++It)
	{
		const AHOLBuilding* Building = *It;
		if (!IsValid(Building) || Building->GetHealth() <= 0 || Building->GetOwnerPlayer() != Slot)
		{
			continue;
		}
		const FVector2D Centre = Building->GetFootprintBounds().GetCenter();
		for (const HOLVolcano::FFlow& Flow : Site.Flows)
		{
			Out.TraceFromBuildings = FMath::Min(Out.TraceFromBuildings, HOLVolcano::DistanceToFlow(Flow, FVector(Centre.X, Centre.Y, 0.0)));
		}
	}
	return true;
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
	const float Now = GetClockOf(Simulation->GetStepIndex());
	// The flows now threatening: the site's drawn flow while it warns or erupts.
	TArray<const HOLVolcano::FFlow*> Threats;
	for (int32 Index = 0; Index < Sites.Num(); ++Index)
	{
		const HOLVolcano::FSiteState State = StateOf(Index, Now);
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
	SyncFreshDeposit(SiteIndex, Site, State);
}

void UHOLVolcanoSubsystem::SyncFreshDeposit(const int32 SiteIndex, const HOLVolcano::FSite& Site, const HOLVolcano::FSiteState& State)
{
	// V7 (D379): once per cycle, on the step the basalt becomes passable. The cycle is marked placed even if the source could not be added (no log at every step).
	if (!HOLVolcano::FreshDepositDue(State) || !FreshPlacedCycle.IsValidIndex(SiteIndex) || FreshPlacedCycle[SiteIndex] >= State.CycleIndex || !Site.Flows.IsValidIndex(State.FlowIndex))
	{
		return;
	}
	UWorld* World = GetWorld();
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	UHOLSourceSubsystem* Sources = World ? World->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
	if (!GameState || !Sources || !Sources->HasAuthority())
	{
		return;
	}
	FreshPlacedCycle[SiteIndex] = State.CycleIndex;
	const HOLVolcano::FFreshDeposit Deposit = HOLVolcano::FreshDeposit(HOLVolcano::SiteSeed(GameState->GetLaunchConfig().Seed, SiteIndex), State.CycleIndex, Site.Flows[State.FlowIndex], Settings);
	if (!Deposit.bPresent)
	{
		return;
	}
	const int32 SourceIndex = Sources->AddSource(Deposit.TypeName, Deposit.Location);
	if (SourceIndex != INDEX_NONE)
	{
		FreshSources[SiteIndex].Add(SourceIndex);
	}
	UE_LOG(LogHOLVolcano, Display, TEXT("Volcano site %d cycle %d: the flow has cooled, a fresh %s %s vein (%s) appears %.0f %% along the flow, at (%.0f, %.0f), source %d"),
		SiteIndex, State.CycleIndex, Deposit.bBig ? TEXT("big") : TEXT("small"), Deposit.Kind == HOLVolcano::EFreshKind::Gold ? TEXT("gold") : TEXT("stone"), *Deposit.TypeName.ToString(),
			100.f * Deposit.Along, Deposit.Location.X, Deposit.Location.Y, SourceIndex);
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