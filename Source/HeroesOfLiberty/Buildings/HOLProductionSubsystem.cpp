// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLProductionSubsystem.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLPlayerState.h"
#include "Net/HOLUnitReceptionSubsystem.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

namespace UE::HeroesOfLiberty::Production
{
	TAutoConsoleVariable<float> CVarBuildTime(
		TEXT("hol.Prod.BuildTime"),
		0.f,
		TEXT("Production time (s) of every unit type in place of the table's (0: the table). For the scripted checks (D284): a Peasant in 2 s, not 20."));

	/** Gap (cm) between the building's footprint and the first row of units born there. */
	constexpr float BirthGap = 400.f;
	/** Angle (deg) between two neighbours of the arc. */
	constexpr float BirthArcStep = 12.f;
	/** Units in one arc (the middle one, then alternately right and left); the next ones stand a row further. */
	constexpr int32 BirthsPerRow = 9;
	/** Distance (cm) between two rows. */
	constexpr float BirthRowGap = 150.f;

	const TCHAR* DescribeRefusal(const EHOLProductionRefusal Refusal)
	{
		switch (Refusal)
		{
		case EHOLProductionRefusal::NotProducible: return TEXT("not produced here");
		case EHOLProductionRefusal::QueueFull: return TEXT("queue full");
		case EHOLProductionRefusal::NotEnoughResources: return TEXT("not enough resources");
		default: return TEXT("none");
		}
	}
}

bool UHOLProductionSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLProductionSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// The server (or a solo match) decides alone (D282); a client only shows what it receives.
	if (InWorld.GetNetMode() == NM_Client)
	{
		return;
	}
	StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
	if (UHOLSimulationSubsystem* Simulation = InWorld.GetSubsystem<UHOLSimulationSubsystem>())
	{
		StepStartedHandle = Simulation->GetOnStepStarted().AddUObject(this, &UHOLProductionSubsystem::OnStepStarted);
	}
}

void UHOLProductionSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		if (UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>())
		{
			Simulation->GetOnStepStarted().Remove(StepStartedHandle);
		}
	}
	Super::Deinitialize();
}

double UHOLProductionSubsystem::GetDisplayStep(const UWorld* World)
{
	if (!World)
	{
		return 0.0;
	}
	if (World->GetNetMode() == NM_Client)
	{
		const UHOLUnitReceptionSubsystem* Reception = World->GetSubsystem<UHOLUnitReceptionSubsystem>();
		return Reception && Reception->GetClock().IsValid() ? Reception->GetClock().GetStep(FPlatformTime::Seconds()) : 0.0;
	}
	// The step being run is GetStepIndex() - 1 once it is over: an item started at step S is done when step
	// S + BuildSteps runs, which this shows as a full bar.
	const UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	return Simulation ? static_cast<double>(Simulation->GetStepIndex() - 1) + Simulation->GetInterpolationAlpha() : 0.0;
}

int32 UHOLProductionSubsystem::GetBuildSteps(const FHOLUnitTypeRow& Type, const float InStepSeconds)
{
	const float Override = UE::HeroesOfLiberty::Production::CVarBuildTime.GetValueOnGameThread();
	const float BuildTime = Override > 0.f ? Override : Type.BuildTime;
	return FMath::Max(1, FMath::RoundToInt(BuildTime / FMath::Max(InStepSeconds, UE_KINDA_SMALL_NUMBER)));
}

FHOLProductionCost UHOLProductionSubsystem::GetCost(const FName TypeName) const
{
	FHOLProductionCost Cost;
	const UHOLUnitSubsystem* Units = GetWorld()->GetSubsystem<UHOLUnitSubsystem>();
	if (const FHOLUnitTypeRow* Row = Units ? Units->FindTypeRow(TypeName) : nullptr)
	{
		Cost.Resources = Row->Cost;
		Cost.BuildSteps = GetBuildSteps(*Row, StepSeconds);
		Cost.Population = Row->Population;
	}
	return Cost;
}

void UHOLProductionSubsystem::AddCheatPopulation(const int32 PlayerIndex, const int32 Places)
{
	CheatPopulation.FindOrAdd(PlayerIndex) += Places;
}

AHOLPlayerState* UHOLProductionSubsystem::FindPlayerState(const int32 PlayerIndex) const
{
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!GameState || PlayerIndex == INDEX_NONE)
	{
		return nullptr;
	}
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState);
		if (HOLPlayerState && HOLPlayerState->GetPlayerIndex() == PlayerIndex)
		{
			return HOLPlayerState;
		}
	}
	return nullptr;
}

void UHOLProductionSubsystem::OnStepStarted(const int64 StepIndex)
{
	ApplyOrders();
	AdvanceQueues(StepIndex);
}

void UHOLProductionSubsystem::ApplyOrders()
{
	using namespace UE::HeroesOfLiberty::Production;

	UHOLCommandSubsystem* Commands = GetWorld()->GetSubsystem<UHOLCommandSubsystem>();
	if (!Commands)
	{
		return;
	}
	TArray<FHOLProductionCommand> Orders;
	Commands->ConsumeProductionOrders(Orders);
	for (const FHOLProductionCommand& Order : Orders)
	{
		AHOLBuilding* Building = Order.Building.Get();
		AHOLPlayerState* PlayerState = FindPlayerState(Order.PlayerIndex);
		// Only the owner gives its buildings orders: a client could send anything.
		if (!Building || !PlayerState || Building->GetOwnerPlayer() != Order.PlayerIndex)
		{
			UE_LOG(LogHOLCommands, Display, TEXT("Production order of slot %d ignored: not its building"), Order.PlayerIndex);
			continue;
		}
		const FHOLBuildingTypeRow* Type = Building->GetType();
		if (!Type)
		{
			continue;
		}
		// A building under construction neither produces nor rallies (D312).
		if (!Building->IsFinished())
		{
			UE_LOG(LogHOLCommands, Display, TEXT("Production order of slot %d ignored: %s not finished"), Order.PlayerIndex, *Building->GetTypeName().ToString());
			continue;
		}

		if (Order.Kind == FHOLProductionCommand::EKind::Rally)
		{
			// On the building itself: no rally point any more (the units are born at its foot, D283).
			if (Building->IsOnFootprint(Order.Location))
			{
				Building->ClearRallyPoint();
				UE_LOG(LogHOLCommands, Display, TEXT("Rally: slot %d cleared the rally point of %s"), Order.PlayerIndex, *Building->GetTypeName().ToString());
			}
			else
			{
				// On one of the player's own sites not finished (the user's trial of J6a.4): the units born here build it; on one
				// of its camps, finished or not: once it is, they gather the nearest source it accepts (D312, J6b.1); the flag
				// on its middle (as on a source); a client could name any building (D288).
				AHOLBuilding* Site = Order.RallySite.Get();
				if (Site && (Site == Building || Site->GetOwnerPlayer() != Order.PlayerIndex || (Site->IsFinished() && !Site->IsCamp())))
				{
					Site = nullptr;
				}
				const FVector Point = Site ? Site->GetActorLocation() : Order.Location;
				const int32 SourceIndex = Site ? INDEX_NONE : Order.SourceIndex;
				Building->SetRallyPoint(Point, SourceIndex, Site);
				UE_LOG(LogHOLCommands, Display, TEXT("Rally: slot %d set the rally point of %s at (%.0f, %.0f)%s"), Order.PlayerIndex,
					*Building->GetTypeName().ToString(), Point.X, Point.Y,
					Site ? *FString::Printf(TEXT(", on site %s"), *Site->GetTypeName().ToString())
					: SourceIndex != INDEX_NONE ? *FString::Printf(TEXT(", on source %d"), SourceIndex) : TEXT(""));
			}
			continue;
		}

		FHOLResources Stock = PlayerState->GetResources();
		TArray<FHOLProductionItem>& Queue = Building->GetMutableProductionQueue();
		if (Order.Kind == FHOLProductionCommand::EKind::Queue)
		{
			const EHOLProductionRefusal Refusal = Type->Produces.Contains(Order.TypeName)
				? HOLProduction::TryEnqueue(Queue, Order.TypeName, GetCost(Order.TypeName), Type->MaxQueue, Stock)
				: EHOLProductionRefusal::NotProducible;
			if (Refusal != EHOLProductionRefusal::None)
			{
				UE_LOG(LogHOLCommands, Display, TEXT("Production: slot %d cannot queue %s (%s)"), Order.PlayerIndex,
					*Order.TypeName.ToString(), DescribeRefusal(Refusal));
				continue;
			}
			UE_LOG(LogHOLCommands, Display, TEXT("Production: slot %d queued %s (%d in the queue, %s)"), Order.PlayerIndex,
				*Order.TypeName.ToString(), Queue.Num(), *Stock.ToString());
		}
		else
		{
			const FName TypeName = Queue.IsValidIndex(Order.Index) ? Queue[Order.Index].TypeName : NAME_None;
			if (!HOLProduction::Cancel(Queue, Order.Index, GetCost(TypeName), Stock))
			{
				continue;
			}
			UE_LOG(LogHOLCommands, Display, TEXT("Production: slot %d cancelled %s (%d in the queue, %s)"), Order.PlayerIndex,
				*TypeName.ToString(), Queue.Num(), *Stock.ToString());
		}
		PlayerState->SetResources(Stock);
		Building->NotifyProductionChanged();
	}
}

void UHOLProductionSubsystem::AdvanceQueues(const int64 StepIndex)
{
	UWorld* World = GetWorld();
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	if (!Units)
	{
		return;
	}
	const auto GetCostOf = [this](const FName TypeName) { return GetCost(TypeName); };

	// Each player's cap (its buildings) and the places its units in production hold.
	TArray<AHOLBuilding*> Buildings;
	TMap<int32, int32> Caps;
	TMap<int32, int32> Started;
	for (TActorIterator<AHOLBuilding> It(World); It; ++It)
	{
		AHOLBuilding* Building = *It;
		const FHOLBuildingTypeRow* Type = Building->GetType();
		Buildings.Add(Building);
		// Only a finished building gives places (D312): a house's foundation gives none.
		Caps.FindOrAdd(Building->GetOwnerPlayer()) += Type ? HOLConstruction::GetPopulationProvided(Type->PopulationProvided, Building->IsFinished()) : 0;
		Started.FindOrAdd(Building->GetOwnerPlayer()) += HOLProduction::GetStartedPopulation(Building->GetProductionQueue(), GetCostOf);
	}
	for (const TPair<int32, int32>& Cheat : CheatPopulation)
	{
		if (int32* Cap = Caps.Find(Cheat.Key))
		{
			*Cap += Cheat.Value;
		}
	}
	// No building (the stress test's maps): nothing produces, no population to count.
	if (Buildings.IsEmpty())
	{
		return;
	}

	TArray<int32> UnitPopulation;
	Units->GetPopulationByOwner(UnitPopulation);
	TMap<int32, int32> FreePopulation;
	for (const TPair<int32, int32>& Cap : Caps)
	{
		const int32 Used = (UnitPopulation.IsValidIndex(Cap.Key) ? UnitPopulation[Cap.Key] : 0) + Started.FindRef(Cap.Key);
		FreePopulation.Add(Cap.Key, HOLProduction::GetPopulationCap(Cap.Value) - Used);
	}

	for (AHOLBuilding* Building : Buildings)
	{
		TArray<FHOLProductionItem>& Queue = Building->GetMutableProductionQueue();
		if (Queue.Num() == 0)
		{
			continue;
		}
		const TArray<FHOLProductionItem> Before = Queue;
		TArray<FName> Done;
		HOLProduction::Advance(Queue, StepIndex, GetCostOf, FreePopulation.FindOrAdd(Building->GetOwnerPlayer()), Done);
		for (const FName TypeName : Done)
		{
			int32& NumBorn = NumBirths.FindOrAdd(Building);
			// A tiny radius: the unit is put on the navmesh right under the point (FHOLUnitSpawnRequest).
			FHOLUnitSpawnRequest Request{ TypeName, 1, GetBirthLocation(*Building, NumBorn), 1.f, Building->GetOwnerPlayer() };
			// Born at its foot, then to the rally point if it has one (J4, D288).
			Request.bHasMoveTarget = Building->HasRallyPoint();
			Request.MoveTarget = Building->GetRallyPoint();
			Request.GatherSource = Building->HasRallyPoint() ? Building->GetRallySource() : INDEX_NONE;
			Request.BuildSite = Building->HasRallyPoint() ? Building->GetRallySite() : nullptr;
			Units->RequestUnits(Request);
			++NumBorn;
			UE_LOG(LogHOLCommands, Display, TEXT("Production: %s of slot %d done at step %lld (%d born here)"), *TypeName.ToString(),
				Building->GetOwnerPlayer(), StepIndex, NumBorn);
		}
		if (Queue != Before)
		{
			Building->NotifyProductionChanged();
		}
	}

	// The population each player sees: its units (the births just asked for included) and its units in production.
	Units->GetPopulationByOwner(UnitPopulation);
	for (const TPair<int32, int32>& Cap : Caps)
	{
		if (AHOLPlayerState* PlayerState = FindPlayerState(Cap.Key))
		{
			int32 InProduction = 0;
			for (const AHOLBuilding* Building : Buildings)
			{
				if (Building->GetOwnerPlayer() == Cap.Key)
				{
					InProduction += HOLProduction::GetStartedPopulation(Building->GetProductionQueue(), GetCostOf);
				}
			}
			const int32 Used = (UnitPopulation.IsValidIndex(Cap.Key) ? UnitPopulation[Cap.Key] : 0) + InProduction;
			PlayerState->SetPopulation(Used, HOLProduction::GetPopulationCap(Cap.Value));
		}
	}
}

FVector UHOLProductionSubsystem::GetBirthLocation(const AHOLBuilding& Building, const int32 Index) const
{
	using namespace UE::HeroesOfLiberty::Production;

	const FVector Origin = Building.GetActorLocation();
	const FHOLBuildingTypeRow* Type = Building.GetType();
	const float HalfFootprint = Type ? 0.5f * FMath::Max(Type->Footprint.X, Type->Footprint.Y) : 600.f;

	// On the side of the rally point (user's choice at the J5a trial): the unit walks off at once, so one arc is enough.
	// Without one, below the building on the screen: the camera never turns and looks along +X for every player (D254),
	// so this side is never hidden behind the building (user's choice in J3).
	FVector Forward(-1.0, 0.0, 0.0);
	const FVector ToRally = (Building.GetRallyPoint() - Origin) * FVector(1.0, 1.0, 0.0);
	const bool bTowardRally = Building.HasRallyPoint() && ToRally.Size() > HalfFootprint;
	if (bTowardRally)
	{
		Forward = ToRally.GetSafeNormal();
	}
	const int32 Row = bTowardRally ? 0 : Index / BirthsPerRow;
	const int32 InRow = Index % BirthsPerRow;
	// 0, then +1, -1, +2, -2...: the arc fills from the middle.
	const int32 Side = (InRow + 1) / 2 * (InRow % 2 == 1 ? 1 : -1);
	const float Distance = HalfFootprint + BirthGap + Row * BirthRowGap;
	const FVector Direction = Forward.RotateAngleAxis(Side * BirthArcStep, FVector::UpVector);
	return Origin + Direction * Distance;
}
