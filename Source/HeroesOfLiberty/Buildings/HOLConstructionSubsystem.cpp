// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLConstructionSubsystem.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "HeroesOfLibertyLog.h"
#include "Map/HOLMapBounds.h"
#include "Match/HOLPlayerState.h"
#include "NavigationSystem.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLSimulationSubsystem.h"

namespace UE::HeroesOfLiberty::Construction
{
	/** The corners of a footprint probed this far inside (cm, HOLConstruction::GetNavProbes). */
	constexpr double NavProbeCornerInset = 100.0;
	/** A probe is on the navmesh when the navmesh is this near it on the ground (cm), within NavProbeHeight up or down. */
	constexpr double NavProbeTolerance = 50.0;
	constexpr double NavProbeHeight = 500.0;
	/** A refused footprint is looked for this many tiles around the obstacle at most (the user's trial of J7b, feedback 3). */
	constexpr int32 FreeFootprintRings = 4;
	/** The ground looked for this far above and below the default height (cm). */
	constexpr double GroundTraceHalfLength = 5000.0;
}

bool UHOLConstructionSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLConstructionSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// The server (or a solo match) decides alone (D288); a client only shows what it receives.
	if (InWorld.GetNetMode() == NM_Client)
	{
		return;
	}
	if (UHOLSimulationSubsystem* Simulation = InWorld.GetSubsystem<UHOLSimulationSubsystem>())
	{
		StepStartedHandle = Simulation->GetOnStepStarted().AddUObject(this, &UHOLConstructionSubsystem::OnStepStarted);
	}
}

void UHOLConstructionSubsystem::Deinitialize()
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

FBox2D UHOLConstructionSubsystem::SnapFootprint(const FHOLBuildingTypeRow& Type, const FVector& Point)
{
	return HOLConstruction::SnapToGrid(FVector2D(Point), Type.Footprint, UHOLBuildingSettings::GetTileSize());
}

namespace UE::HeroesOfLiberty::Construction
{
	/** What a placement is checked against on this machine, gathered once (CheckPlacement, FindFreeFootprint). */
	struct FPlacementWorld
	{
		TArray<FBox2D> Buildings;
		TArray<FVector2D> EnemyCentres;
		TArray<FHOLPlacementCircle> Sources;
		FBox2D MapArea = FBox2D(ForceInit);
		/** Height of the player's own main centre: where the navmesh is probed. */
		double GroundZ = 0.0;
		/** Only the server (and a solo match) has one; a client leaves the navmesh to the server. */
		const UNavigationSystemV1* NavSystem = nullptr;

		void Gather(const UWorld& World, const int32 PlayerIndex)
		{
			for (TActorIterator<AHOLBuilding> It(&World); It; ++It)
			{
				if (!It->GetType())
				{
					continue;
				}
				// Every player's buildings and foundations (D311: touching is fine).
				Buildings.Add(It->GetFootprintBounds());
				if (It->GetTypeName() == HOLBuildingTypeNames::MainCentre)
				{
					if (It->GetOwnerPlayer() != PlayerIndex)
					{
						// Every other player is an enemy until the teams reach the match (the match configuration has them, the
						// machines do not share it yet).
						EnemyCentres.Add(FVector2D(It->GetActorLocation()));
					}
					else
					{
						GroundZ = It->GetActorLocation().Z;
					}
				}
			}
			if (const UHOLSourceSubsystem* SourceSubsystem = World.GetSubsystem<UHOLSourceSubsystem>())
			{
				for (const FHOLSource& Source : SourceSubsystem->GetSources())
				{
					if (Source.IsStanding())
					{
						Sources.Add({ FVector2D(Source.Location), static_cast<double>(Source.Radius) });
					}
				}
			}
			if (const AHOLMapBounds* Bounds = AHOLMapBounds::Find(&World))
			{
				MapArea = Bounds->GetArea();
			}
			NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(const_cast<UWorld*>(&World));
		}

		FHOLPlacementQuery MakeQuery(const FBox2D& Footprint) const
		{
			FHOLPlacementQuery Query;
			Query.Footprint = Footprint;
			Query.Buildings = Buildings;
			Query.Sources = Sources;
			Query.MapArea = MapArea;
			Query.EnemyMainCentres = EnemyCentres;
			Query.AntiRushRadius = GetDefault<UHOLBuildingSettings>()->AntiRushRadius;
			return Query;
		}

		/** The footprint lies on the navmesh (its probes, HOLConstruction::GetNavProbes); always true without a navmesh. */
		bool IsOnNavMesh(const FBox2D& Footprint) const
		{
			if (!NavSystem)
			{
				return true;
			}
			TArray<FVector2D> Probes;
			HOLConstruction::GetNavProbes(Footprint, UHOLBuildingSettings::GetTileSize(), NavProbeCornerInset, Probes);
			for (const FVector2D& Probe : Probes)
			{
				FNavLocation OnNav;
				const FVector Point(Probe, GroundZ);
				if (!NavSystem->ProjectPointToNavigation(Point, OnNav, FVector(NavProbeTolerance, NavProbeTolerance, NavProbeHeight))
					|| FVector::Dist2D(OnNav.Location, Point) > NavProbeTolerance)
				{
					return false;
				}
			}
			return true;
		}
	};
}

EHOLPlacementRefusal UHOLConstructionSubsystem::CheckPlacement(const UWorld& World, const FBox2D& Footprint, const int32 PlayerIndex,
	bool* bOutNavChecked)
{
	using namespace UE::HeroesOfLiberty::Construction;

	FPlacementWorld Placement;
	Placement.Gather(World, PlayerIndex);
	FHOLPlacementQuery Query = Placement.MakeQuery(Footprint);

	// The navmesh under the footprint: only the server (and a solo match) has one; a client leaves it to the server.
	if (bOutNavChecked)
	{
		*bOutNavChecked = Placement.NavSystem != nullptr;
	}
	if (Placement.NavSystem && HOLConstruction::CheckPlacement(Query) == EHOLPlacementRefusal::None)
	{
		Query.bOnNavMesh = Placement.IsOnNavMesh(Footprint);
	}
	return HOLConstruction::CheckPlacement(Query);
}

bool UHOLConstructionSubsystem::FindFreeFootprint(const UWorld& World, const FBox2D& Footprint, const FVector2D& Aim, const int32 PlayerIndex,
	FBox2D& OutFootprint, int32& OutRing, EHOLPlacementRefusal& OutObstacle)
{
	using namespace UE::HeroesOfLiberty::Construction;

	FPlacementWorld Placement;
	Placement.Gather(World, PlayerIndex);
	const FHOLPlacementQuery Query = Placement.MakeQuery(Footprint);
	// Only a footprint on a building or a source is moved (the user's trial of J7b, feedback 3); out of the map, near an
	// enemy centre or off the navmesh, the ghost stays red under the cursor.
	OutObstacle = HOLConstruction::CheckPlacement(Query);
	if (OutObstacle != EHOLPlacementRefusal::Building && OutObstacle != EHOLPlacementRefusal::Source)
	{
		return false;
	}
	const double Grid = UHOLBuildingSettings::GetTileSize();
	FBox2D Obstacle;
	if (!HOLConstruction::FindObstacle(Query, Aim, Grid, Obstacle))
	{
		return false;
	}
	return HOLConstruction::FindFreeFootprint(Query, Obstacle, Aim, Grid, FreeFootprintRings,
		[&Placement](const FBox2D& Candidate) { return Placement.IsOnNavMesh(Candidate); }, OutFootprint, &OutRing);
}

const TCHAR* UHOLConstructionSubsystem::DescribeRefusal(const EHOLPlacementRefusal Refusal)
{
	switch (Refusal)
	{
	case EHOLPlacementRefusal::OutOfMap: return TEXT("out of the map");
	case EHOLPlacementRefusal::Building: return TEXT("on a building");
	case EHOLPlacementRefusal::Source: return TEXT("on a resource source");
	case EHOLPlacementRefusal::EnemyCentre: return TEXT("too near an enemy main centre");
	case EHOLPlacementRefusal::NavMesh: return TEXT("off the navmesh");
	default: return TEXT("none");
	}
}

double UHOLConstructionSubsystem::FindGroundZ(const UWorld& World, const FVector2D& Point, const double DefaultZ)
{
	using namespace UE::HeroesOfLiberty::Construction;

	TArray<FHitResult> Hits;
	const FVector Top(Point, DefaultZ + GroundTraceHalfLength);
	const FVector Bottom(Point, DefaultZ - GroundTraceHalfLength);
	World.LineTraceMultiByObjectType(Hits, Top, Bottom, FCollisionObjectQueryParams(ECC_WorldStatic),
		FCollisionQueryParams(SCENE_QUERY_STAT(HOLGround), /*bTraceComplex*/ false));
	for (const FHitResult& Hit : Hits)
	{
		if (!Cast<AHOLBuilding>(Hit.GetActor()))
		{
			return Hit.ImpactPoint.Z;
		}
	}
	return DefaultZ;
}

AHOLPlayerState* UHOLConstructionSubsystem::FindPlayerState(const int32 PlayerIndex) const
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

void UHOLConstructionSubsystem::OnStepStarted(const int64 StepIndex)
{
	ApplyOrders();
}

void UHOLConstructionSubsystem::ApplyOrders()
{
	UHOLCommandSubsystem* Commands = GetWorld()->GetSubsystem<UHOLCommandSubsystem>();
	if (!Commands)
	{
		return;
	}
	TArray<FHOLConstructionCommand> Orders;
	Commands->ConsumeConstructionOrders(Orders);
	for (const FHOLConstructionCommand& Order : Orders)
	{
		AHOLPlayerState* PlayerState = FindPlayerState(Order.PlayerIndex);
		if (!PlayerState)
		{
			continue;
		}
		if (Order.Kind == FHOLConstructionCommand::EKind::Place)
		{
			Place(Order, *PlayerState);
		}
		else
		{
			Cancel(Order, *PlayerState);
		}
	}
}

void UHOLConstructionSubsystem::Place(const FHOLConstructionCommand& Order, AHOLPlayerState& PlayerState)
{
	UWorld* World = GetWorld();
	// Only a type the builders build (D313): a client could name any row, the main centre too.
	const FHOLBuildingTypeRow* Type = Order.TypeName.IsNone() ? nullptr : UHOLBuildingSettings::FindType(Order.TypeName);
	if (!Type || !Type->IsBuildable())
	{
		UE_LOG(LogHOLCommands, Display, TEXT("Construction: slot %d cannot place %s: not buildable"), Order.PlayerIndex, *Order.TypeName.ToString());
		return;
	}
	const FBox2D Footprint = SnapFootprint(*Type, Order.Location);
	const FVector2D Centre = Footprint.GetCenter();
	const EHOLPlacementRefusal Refusal = CheckPlacement(*World, Footprint, Order.PlayerIndex);
	if (Refusal != EHOLPlacementRefusal::None)
	{
		UE_LOG(LogHOLCommands, Display, TEXT("Construction: slot %d cannot place %s at (%.0f, %.0f): %s (%s)"), Order.PlayerIndex,
			*Order.TypeName.ToString(), Centre.X, Centre.Y, DescribeRefusal(Refusal), *PlayerState.GetResources().ToString());
		return;
	}
	FHOLResources Stock = PlayerState.GetResources();
	if (!HOLConstruction::TryPay(Stock, Type->Cost))
	{
		UE_LOG(LogHOLCommands, Display, TEXT("Construction: slot %d cannot place %s at (%.0f, %.0f): not enough resources (%s; cost %s)"),
			Order.PlayerIndex, *Order.TypeName.ToString(), Centre.X, Centre.Y, *Stock.ToString(), *Type->Cost.ToString());
		return;
	}

	// On the ground, along the grid (no rotation, D311), owned by the player's controller: its production queue and rally
	// point will reach it alone (D282), like its main centre's.
	const double GroundZ = FindGroundZ(*World, Centre, Order.Location.Z);
	const FTransform Transform(FRotator::ZeroRotator, FVector(Centre, GroundZ));
	AHOLBuilding* Foundation = World->SpawnActorDeferred<AHOLBuilding>(AHOLBuilding::StaticClass(), Transform, PlayerState.GetPlayerController(),
		nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Foundation)
	{
		return;
	}
	PlayerState.SetResources(Stock);
	Foundation->Setup(Order.TypeName, static_cast<uint8>(Order.PlayerIndex), static_cast<uint8>(PlayerState.GetColorIndex()),
		EHOLConstructionState::Foundation);
	Foundation->FinishSpawning(Transform);
	Foundation->ForceNetUpdate();
	UE_LOG(LogHOLCommands, Display, TEXT("Construction: slot %d placed %s foundation at (%.0f, %.0f), paid %s (%s)"), Order.PlayerIndex,
		*Order.TypeName.ToString(), Centre.X, Centre.Y, *Type->Cost.ToString(), *Stock.ToString());

	// The Peasants selected when it was placed go and build it (J6a.4, D312), at the end of their queue with Shift (D311).
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	if (Commands && !Order.UnitIds.IsEmpty())
	{
		FHOLBuildOrder BuildOrder;
		BuildOrder.UnitIds = Order.UnitIds;
		BuildOrder.Building = Foundation;
		BuildOrder.PlayerIndex = Order.PlayerIndex;
		BuildOrder.bQueue = Order.bQueue;
		// Only a player places a foundation with builders (its controller): their chain is drawn (D328).
		BuildOrder.bChain = true;
		Commands->IssueBuildOrder(BuildOrder);
	}
}

void UHOLConstructionSubsystem::Cancel(const FHOLConstructionCommand& Order, AHOLPlayerState& PlayerState)
{
	AHOLBuilding* Building = Order.Building.Get();
	// Only the owner cancels, and only what is not finished (D311): a client could name any building.
	if (!Building || Building->GetOwnerPlayer() != Order.PlayerIndex || !HOLConstruction::CanCancel(Building->GetConstructionState()))
	{
		UE_LOG(LogHOLCommands, Display, TEXT("Construction: cancel of slot %d ignored: not its building under construction"), Order.PlayerIndex);
		return;
	}
	const FHOLBuildingTypeRow* Type = Building->GetType();
	const FHOLResources Refund = Type ? HOLConstruction::GetCancelRefund(Type->Cost, Building->GetConstructionState(), Building->GetConstructionWork())
		: FHOLResources();
	FHOLResources Stock = PlayerState.GetResources();
	Stock += Refund;
	PlayerState.SetResources(Stock);
	const FVector Location = Building->GetActorLocation();
	UE_LOG(LogHOLCommands, Display, TEXT("Construction: slot %d cancelled %s at (%.0f, %.0f), refund %s (%s)"), Order.PlayerIndex,
		*Building->GetTypeName().ToString(), Location.X, Location.Y, *Refund.ToString(), *Stock.ToString());
	Building->Destroy();
}
