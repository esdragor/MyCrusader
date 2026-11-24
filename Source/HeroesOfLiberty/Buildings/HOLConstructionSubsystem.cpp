// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLConstructionSubsystem.h"

#include "Match/HOLMatchStatsSubsystem.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLConstructionBoard.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Buildings/HOLRubble.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "HeroesOfLibertyLog.h"
#include "Map/HOLMapBounds.h"
#include "Match/HOLFactionRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerState.h"
#include "NavigationSystem.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLGarrisonSubsystem.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

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
	// D898 G4: a wall piece goes on the fine grid of hol.Wall.SubDivision; every other building (and a wall at SubDivision 1) keeps the 4 m grid.
	const int32 SubDivision = HOLConstruction::GetWallSubDivision();
	if (SubDivision > 1 && (&Type == UHOLBuildingSettings::FindType(FName(TEXT("Palisade"))) || &Type == UHOLBuildingSettings::FindType(FName(TEXT("StoneWall")))))
	{
		return HOLConstruction::SnapWallPiece(FVector2D(Point), Type.Footprint, UHOLBuildingSettings::GetTileSize(), SubDivision);
	}
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
			for (FHOLBuildingIterator It(&World); It; ++It)
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

namespace UE::HeroesOfLiberty::Construction
{
	/** Whether Footprint may go with Planned built too: every rule of CheckPlacement, the navmesh included. */
	bool IsFreeWith(FPlacementWorld& Placement, const FBox2D& Footprint, TConstArrayView<FBox2D> Planned)
	{
		const int32 Num = Placement.Buildings.Num();
		Placement.Buildings.Append(Planned.GetData(), Planned.Num());
		const FHOLPlacementQuery Query = Placement.MakeQuery(Footprint);
		bool bFree = HOLConstruction::CheckPlacement(Query) == EHOLPlacementRefusal::None;
		if (bFree && Placement.NavSystem)
		{
			bFree = Placement.IsOnNavMesh(Footprint);
		}
		Placement.Buildings.SetNum(Num, EAllowShrinking::No);
		return bFree;
	}

	/** The footprints of Type's size in rings of tiles around Centre (ring 0 the snapped one), ring by ring, nearest Centre first. */
	void EnumerateRings(const FHOLBuildingTypeRow& Type, const FVector2D& Centre, const int32 MaxRings, TArray<FBox2D>& OutBoxes)
	{
		const double Grid = UHOLBuildingSettings::GetTileSize();
		const FBox2D Start = HOLConstruction::SnapToGrid(Centre, Type.Footprint, Grid);
		for (int32 Ring = 0; Ring <= MaxRings; ++Ring)
		{
			TArray<FBox2D> InRing;
			for (int32 Y = -Ring; Y <= Ring; ++Y)
			{
				for (int32 X = -Ring; X <= Ring; ++X)
				{
					if (FMath::Max(FMath::Abs(X), FMath::Abs(Y)) == Ring)
					{
						InRing.Add(Start.ShiftBy(FVector2D(X * Grid, Y * Grid)));
					}
				}
			}
			// Stable: the candidates were listed Y then X, a tie keeps that order.
			InRing.StableSort([&Centre](const FBox2D& A, const FBox2D& B)
			{
				return FVector2D::DistSquared(A.GetCenter(), Centre) < FVector2D::DistSquared(B.GetCenter(), Centre);
			});
			OutBoxes.Append(InRing);
		}
	}
}

bool UHOLConstructionSubsystem::FindRingFootprint(const UWorld& World, const FHOLBuildingTypeRow& Type, const FVector2D& Centre, const int32 PlayerIndex,
	const int32 MaxRings, const TConstArrayView<FBox2D> Planned, FBox2D& OutFootprint)
{
	using namespace UE::HeroesOfLiberty::Construction;

	if (MaxRings < 0)
	{
		return false;
	}
	FPlacementWorld Placement;
	Placement.Gather(World, PlayerIndex);
	TArray<FBox2D> Candidates;
	EnumerateRings(Type, Centre, MaxRings, Candidates);
	for (const FBox2D& Candidate : Candidates)
	{
		if (IsFreeWith(Placement, Candidate, Planned))
		{
			OutFootprint = Candidate;
			return true;
		}
	}
	return false;
}

bool UHOLConstructionSubsystem::FindFarmBlock(const UWorld& World, const FHOLBuildingTypeRow& MillType, const FHOLBuildingTypeRow& FarmType,
	const FVector2D& Centre, const int32 PlayerIndex, const int32 MaxRings, FBox2D& OutMill, TArray<FBox2D>& OutFarms)
{
	using namespace UE::HeroesOfLiberty::Construction;

	if (MaxRings < 0)
	{
		return false;
	}
	FPlacementWorld Placement;
	Placement.Gather(World, PlayerIndex);
	TArray<FBox2D> Candidates;
	EnumerateRings(MillType, Centre, MaxRings, Candidates);
	for (const FBox2D& MillBox : Candidates)
	{
		if (!IsFreeWith(Placement, MillBox, {}))
		{
			continue;
		}
		// The 3 x 3 cells: the mill in the middle, a farm in each of the 8 others, touching it.
		TArray<FBox2D> Planned;
		Planned.Add(MillBox);
		const FVector2D Step = (FVector2D(MillType.Footprint) + FVector2D(FarmType.Footprint)) * 0.5;
		bool bAll = true;
		for (int32 Y = -1; Y <= 1 && bAll; ++Y)
		{
			for (int32 X = -1; X <= 1 && bAll; ++X)
			{
				if (X == 0 && Y == 0)
				{
					continue;
				}
				const FBox2D Farm = HOLConstruction::MakeFootprint(MillBox.GetCenter() + FVector2D(X * Step.X, Y * Step.Y), FVector2D(FarmType.Footprint));
				if (IsFreeWith(Placement, Farm, Planned))
				{
					Planned.Add(Farm);
				}
				else
				{
					bAll = false;
				}
			}
		}
		if (bAll)
		{
			OutMill = MillBox;
			OutFarms = TArray<FBox2D>(Planned.GetData() + 1, Planned.Num() - 1);
			return true;
		}
	}
	return false;
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
			// IA0.1 E2/E4: no stock to pay with (an AI slot before IA0.1): dropped, no longer silently.
			UE_LOG(LogHOLCommands, Warning, TEXT("Construction order of slot %d ignored: no player state"), Order.PlayerIndex);
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
	// K1a: the script board decides the order of the gates (HOLConstructionBoard.as: buildable, faction, age, placement, payment); the journal lines and the
	// refusals are the C++ ones. -1 (no board, or an answer out of range) refuses the order (error logged once). The stock is only
	// COMPUTED here: it is written after the foundation is born (below), never before.
	FBox2D Footprint;
	FVector2D Centre;
	FHOLResources Cost;
	FHOLResources Stock;
	int32 Verdict = -1;
	int32 RawVerdict = -1; // the board's answer as given, before an out-of-range one is folded into -1
	if (UHOLConstructionBoard::Find(World))
	{
		const bool bBuildable = Type && UHOLBuildingSettings::IsBuildableBy(World, Order.TypeName, Order.PlayerIndex);
		const AHOLGameState* BoardState = World->GetGameState<AHOLGameState>();
		const int32 BoardTier = BoardState ? BoardState->GetTier(Order.PlayerIndex) : 0;
		int32 Refusal = 0;
		if (bBuildable)
		{
			Footprint = SnapFootprint(*Type, Order.Location);
			Centre = Footprint.GetCenter();
			Refusal = static_cast<int32>(CheckPlacement(*World, Footprint, Order.PlayerIndex));
			Cost = UHOLBuildingSettings::GetBuildCost(World, Order.TypeName, Order.PlayerIndex);
		}
		Stock = PlayerState.GetResources();
		Verdict = UHOLConstructionBoard::AskPlace(World, bBuildable, bBuildable && HOLFactionRules::IsBuildingOpenTo(World, Order.TypeName, Order.PlayerIndex),
			Type ? Type->Age : 0, BoardTier, Refusal, Stock, Cost);
		switch (Verdict)
		{
		case 0:
			Stock = UHOLConstructionBoard::AskStockAfterPlace(World, Stock, Cost);
			break;
		case 1:
			UE_LOG(LogHOLCommands, Display, TEXT("Construction: slot %d cannot place %s: not buildable"), Order.PlayerIndex, *Order.TypeName.ToString());
			return;
		case 2:
			UE_LOG(LogHOLCommands, Display, TEXT("Construction: slot %d cannot place %s: not a building of its faction"), Order.PlayerIndex, *Order.TypeName.ToString());
			return;
		case 3:
			UE_LOG(LogHOLCommands, Display, TEXT("Construction: slot %d cannot place %s: age %d not reached (tier %d)"), Order.PlayerIndex,
				*Order.TypeName.ToString(), Type->Age, BoardTier);
			return;
		case 4:
			UE_LOG(LogHOLCommands, Display, TEXT("Construction: slot %d cannot place %s at (%.0f, %.0f): %s (%s)"), Order.PlayerIndex,
				*Order.TypeName.ToString(), Centre.X, Centre.Y, DescribeRefusal(static_cast<EHOLPlacementRefusal>(Refusal)), *Stock.ToString());
			return;
		case 5:
			UE_LOG(LogHOLCommands, Display, TEXT("Construction: slot %d cannot place %s at (%.0f, %.0f): not enough resources (%s; cost %s)"),
				Order.PlayerIndex, *Order.TypeName.ToString(), Centre.X, Centre.Y, *Stock.ToString(), *Cost.ToString());
			return;
		default:
			RawVerdict = Verdict;
			Verdict = -1;
			break;
		}
	}
	if (Verdict < 0)
	{
		// A verdict beyond 0-5 comes from a board that IS there: said as such, not as an absent board.
		static bool bLogged = false;
		static bool bLoggedRange = false;
		if (RawVerdict > 5 || RawVerdict < -1)
		{
			if (!bLoggedRange)
			{
				bLoggedRange = true;
				UE_LOG(LogHOLCommands, Error, TEXT("Construction: Place refused, verdict out of range %d"), RawVerdict);
			}
		}
		else if (!bLogged)
		{
			bLogged = true;
			UE_LOG(LogHOLCommands, Error, TEXT("Construction: Place refused, script board absent"));
		}
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
		*Order.TypeName.ToString(), Centre.X, Centre.Y, *Cost.ToString(), *Stock.ToString());

	// The Peasants selected when it was placed go and build it (J6a.4, D312), at the end of their queue with Shift (D311).
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	if (Commands && !Order.UnitIds.IsEmpty() && Order.bQueue)
	{
		// With Shift, through the queue of moves as Shift + a site (EtapeDefini M.1, the user's 3rd trial of L: Shift + P then
		// Shift + a placement went to build at once, P dropped): it waits for their walks; sorted there by each unit's queue
		// of work (UHOLUnitOrderProcessor). Only a player places a foundation with builders: their chain is drawn (D328).
		const UHOLCommandSubsystem::FPlayerOrderScope PlayerOrder(*Commands);
		Commands->IssueQueuedWorkOrder(Order.UnitIds, INDEX_NONE, Foundation, Foundation->GetActorLocation(), Order.PlayerIndex);
	}
	else if (Commands && !Order.UnitIds.IsEmpty())
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
	UWorld* World = GetWorld();
	// K1a: the script board decides who cancels what (HOLConstructionBoard.as); no board: the cancel is ignored.
	const int32 State = Building ? static_cast<int32>(Building->GetConstructionState()) : 0;
	int32 Verdict = UHOLConstructionBoard::AskCancel(World, Building != nullptr, Building ? static_cast<int32>(Building->GetOwnerPlayer()) : INDEX_NONE, Order.PlayerIndex, State);
	if (Verdict < 0)
	{
		static bool bLogged = false;
		if (!bLogged)
		{
			bLogged = true;
			UE_LOG(LogHOLCommands, Error, TEXT("Construction: Cancel refused, script board absent"));
		}
		// Still ignored (as before), but said for what it is: not "not its building under construction".
		UE_LOG(LogHOLCommands, Display, TEXT("Construction: cancel of slot %d ignored: no script board"), Order.PlayerIndex);
		return;
	}
	if (Verdict != 0 || !Building)
	{
		UE_LOG(LogHOLCommands, Display, TEXT("Construction: cancel of slot %d ignored: not its building under construction"), Order.PlayerIndex);
		return;
	}
	const FHOLBuildingTypeRow* Type = Building->GetType();
	const FHOLResources BuildCost = Type ? UHOLBuildingSettings::GetBuildCost(World, Building->GetTypeName(), Building->GetOwnerPlayer()) : FHOLResources();
	const FHOLResources Refund = UHOLConstructionBoard::AskCancelRefund(World, Type != nullptr, BuildCost, State, Building->GetConstructionWork(), Building->GetTotalWork());
	FHOLResources Stock = PlayerState.GetResources();
	Stock += Refund;
	PlayerState.SetResources(Stock);
	const FVector Location = Building->GetActorLocation();
	UE_LOG(LogHOLCommands, Display, TEXT("Construction: slot %d cancelled %s at (%.0f, %.0f), refund %s (%s)"), Order.PlayerIndex,
		*Building->GetTypeName().ToString(), Location.X, Location.Y, *Refund.ToString(), *Stock.ToString());
	Building->Destroy();
}

int32 UHOLConstructionSubsystem::DamageBuilding(AHOLBuilding& Building, const int32 Damage, const EHOLAttackType AttackType)
{
	const FHOLBuildingTypeRow* Type = Building.GetType();
	// A main forum in ruin or rebuilding is unbreakable (D351).
	if (!Building.HasAuthority() || !Type || !IsValid(&Building) || Building.GetHealth() <= 0 || Building.IsForumDown())
	{
		return 0;
	}
	const int32 Before = Building.GetHealth();
	const int32 Left = Building.TakeDamagePoints(HOLConstruction::GetBuildingDamage(Damage, AttackType, Type->RangedArmor));
	const int32 Dealt = Before - Left;
	UE_LOG(LogHOLCommands, Display, TEXT("Building damaged: %s of slot %d took %d (%d %s), health %d/%d"), *Building.GetTypeName().ToString(),
		Building.GetOwnerPlayer(), Dealt, Damage, *UEnum::GetValueAsString(AttackType), Left, Type->MaxHealth);
	if (Left <= 0)
	{
		// V2 (D350): a main forum falls into ruin instead (a life less); destroyed only when it is not one.
		AHOLGameState* GameState = Building.IsPrimaryForum() && Building.GetWorld() ? Building.GetWorld()->GetGameState<AHOLGameState>() : nullptr;
		if (!GameState || !GameState->HandleForumFall(Building))
		{
			DestroyBuilding(Building);
		}
	}
	return Dealt;
}

FHOLResources UHOLConstructionSubsystem::RefundProductionQueue(AHOLBuilding& Building)
{
	FHOLResources Refund;
	UWorld* World = Building.GetWorld();
	TArray<FHOLProductionItem>& Queue = Building.GetMutableProductionQueue();
	// K1b: the script board decides whether the queue is refunded and adds up its costs (HOLConstructionBoard.as); no board: nothing is refunded.
	// Not a per-hit path: reached once per forum fall or building death (the combat processor runs on the game thread, bRequiresGameThreadExecution).
	const int32 QueueVerdict = UHOLConstructionBoard::AskQueueRefund(World, World != nullptr, Building.HasAuthority(), Queue.Num());
	if (QueueVerdict < 0)
	{
		static bool bLogged = false;
		if (!bLogged)
		{
			bLogged = true;
			UE_LOG(LogHOLCommands, Error, TEXT("Construction: RefundProductionQueue refused, script board absent"));
		}
		return Refund;
	}
	if (QueueVerdict != 0)
	{
		return Refund;
	}
	const UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>();
	TArray<FHOLResources> ItemCosts;
	for (int32 Index = Queue.Num() - 1; Index >= 0; --Index)
	{
		const FHOLProductionCost ItemCost = Production ? Production->GetCost(Queue[Index].TypeName, Building.GetOwnerPlayer()) : FHOLProductionCost();
		ItemCosts.Add(ItemCost.Resources);
	}
	while (!Queue.IsEmpty())
	{
		const FName TypeName = Queue.Last().TypeName;
		HOLProduction::Cancel(Queue, Queue.Num() - 1, Production ? Production->GetCost(TypeName, Building.GetOwnerPlayer()) : FHOLProductionCost(), Refund);
	}
	UHOLConstructionBoard::AskQueueTotal(World, ItemCosts, Refund);
	Building.NotifyProductionChanged();
	// Its owner's stock, or the one kept for him while he is away (M6-Q16).
	FHOLResources Stock;
	AHOLGameState::AddSlotStock(*World, Building.GetOwnerPlayer(), Refund, Stock);
	return Refund;
}

void UHOLConstructionSubsystem::DestroyBuilding(AHOLBuilding& Building)
{
	UWorld* World = Building.GetWorld();
	// K1b: the script board decides whether it is destroyed, the site's share and the rubble (HOLConstructionBoard.as). No board: a dead building must not stay up, so it is
	// still destroyed (the engine's own guards only), with no refund and no rubble, and the error says so once. Once per building death (DamageBuilding at 0 health, the forum fall).
	const int32 DestroyVerdict = UHOLConstructionBoard::AskDestroy(World, World != nullptr, Building.HasAuthority(), IsValid(&Building));
	if (DestroyVerdict < 0)
	{
		static bool bLogged = false;
		if (!bLogged)
		{
			bLogged = true;
			UE_LOG(LogHOLCommands, Error, TEXT("Construction: Destroy kept without rules, script board absent (no refund, no rubble)"));
		}
		if (!World || !Building.HasAuthority() || !IsValid(&Building))
		{
			return;
		}
	}
	else if (DestroyVerdict != 0)
	{
		return;
	}
	// Its owner's stock: every item of its queue back in full (as a cancel, the head too), a site the share not built (D345).
	const int32 QueueLength = Building.GetProductionQueue().Num();
	FHOLResources Refund = RefundProductionQueue(Building);
	const FHOLBuildingTypeRow* Type = Building.GetType();
	FHOLResources SiteRefund;
	if (Type)
	{
		const FHOLResources SiteCost = UHOLBuildingSettings::GetBuildCost(Building.GetWorld(), Building.GetTypeName(), Building.GetOwnerPlayer());
		SiteRefund = UHOLConstructionBoard::AskCancelRefund(World, true, SiteCost, static_cast<int32>(Building.GetConstructionState()), Building.GetConstructionWork(), Building.GetTotalWork());
		Refund += SiteRefund;
	}
	// Its owner's stock, or the one kept for him while he is away (M6-Q16).
	FHOLResources Stock;
	const bool bHasStock = AHOLGameState::AddSlotStock(*World, Building.GetOwnerPlayer(), SiteRefund, Stock);
	const FVector Location = Building.GetActorLocation();
	UE_LOG(LogHOLCommands, Display, TEXT("Building destroyed: %s of slot %d at (%.0f, %.0f), %s, queue of %d refunded, refund %s (%s)"),
		*Building.GetTypeName().ToString(), Building.GetOwnerPlayer(), Location.X, Location.Y,
		Building.IsFinished() ? TEXT("finished") : Building.IsFoundation() ? TEXT("foundation") : TEXT("under construction"), QueueLength,
		*Refund.ToString(), bHasStock ? *Stock.ToString() : TEXT("no stock"));

	// Rubble for 10 s where it stood (D345.2a), seen by everyone; none for a bare foundation, which the others never saw.
	const int32 RubbleVerdict = UHOLConstructionBoard::AskRubble(World, Building.IsFoundation(), Type != nullptr);
	if (RubbleVerdict > 0)
	{
		const FTransform Transform = Building.GetActorTransform();
		if (AHOLRubble* Rubble = World->SpawnActorDeferred<AHOLRubble>(AHOLRubble::StaticClass(), Transform, nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
		{
			Rubble->Setup(Building.GetTypeName(), Building.GetColorIndex());
			Rubble->FinishSpawning(Transform);
		}
	}
	// Its footprint reopens at once (the walkers' kept paths across it forgotten), then it leaves the world: everything
	// that reads the buildings at each step (population, deposits, production, builders, selection) no longer finds it.
	if (UHOLMatchStatsSubsystem* MatchStats = World ? World->GetSubsystem<UHOLMatchStatsSubsystem>() : nullptr)
	{
		MatchStats->RecordBuildingLost(Building.GetOwnerPlayer());
	}
	// IA1.2 F6a (IA1.2-Q2): a farm's field leaves the map with it; its Peasant goes to a free farm of the massif, or idles.
	Building.ReleaseLinkedSource();
	// F7 lot 3c (D693): its garrison back on the field at its foot, health unchanged, before it leaves the world.
	UHOLGarrisonSubsystem::EjectAll(Building);
	Building.ReleaseFootprint();
	Building.Destroy();
}
