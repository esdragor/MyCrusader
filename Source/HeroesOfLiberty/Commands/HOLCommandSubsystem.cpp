// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLCommandSubsystem.h"

#include "Buildings/HOLBuilding.h"
#include "Engine/World.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "NavigationSystem.h"
#include "Simulation/HOLSimulationSettings.h"

bool UHOLCommandSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

bool UHOLCommandSubsystem::IsRefused(const int32 PlayerIndex)
{
	const UWorld* World = GetWorld();
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	// R3.10: no order while the match waits for its named players.
	if (GameState && GameState->IsWaitingForPlayers())
	{
		UE_LOG(LogHOLCommands, Display, TEXT("Match waits: order of slot %d refused"), PlayerIndex);
		return true;
	}
	if (!GameState || PlayerIndex == INDEX_NONE || !GameState->IsSlotDefeated(PlayerIndex))
	{
		return false;
	}
	const uint32 Bit = 1u << (PlayerIndex & 31);
	if (!(RefusalsLogged & Bit))
	{
		RefusalsLogged |= Bit;
		UE_LOG(LogHOLCommands, Display, TEXT("Victory: orders of defeated slot %d refused (D358)"), PlayerIndex);
	}
	return true;
}

void UHOLCommandSubsystem::DropOrdersOf(const int32 PlayerIndex)
{
	const auto OfPlayer = [PlayerIndex](const auto& Order) { return Order.PlayerIndex == PlayerIndex; };
	PendingMoveOrders.RemoveAll(OfPlayer);
	PendingGatherOrders.RemoveAll(OfPlayer);
	PendingProductionOrders.RemoveAll(OfPlayer);
	PendingConstructionOrders.RemoveAll(OfPlayer);
	PendingBuildOrders.RemoveAll(OfPlayer);
	PendingHeroReturnOrders.Remove(PlayerIndex);
}

void UHOLCommandSubsystem::IssueHeroReturnOrder(const int32 PlayerIndex)
{
	if (PlayerIndex < 0 || PlayerIndex >= FHOLMatchConfig::MaxSlots || IsRefused(PlayerIndex))
	{
		return;
	}
	PendingHeroReturnOrders.Add(PlayerIndex);
}

void UHOLCommandSubsystem::ConsumeHeroReturnOrders(TArray<int32>& OutPlayerIndices)
{
	OutPlayerIndices.Reset();
	Swap(OutPlayerIndices, PendingHeroReturnOrders);
}

void UHOLCommandSubsystem::IssueMoveOrder(const TConstArrayView<uint32> UnitIds, const FVector& Target, const int32 PlayerIndex, const bool bFromTask,
	const FVector2D& FrontLine, const bool bFrontWidth, const bool bQueue)
{
	if (UnitIds.Num() == 0 || IsRefused(PlayerIndex))
	{
		return;
	}

	// Destination brought back to the navigable area (D228): one point query per order, not per unit, the way
	// a real time strategy game keeps a click next to the map walkable instead of walking into the void.
	// A task's walk (J5: a gatherer's spot, already on the navmesh) is taken as it is.
	FVector Destination = Target;
	UWorld* World = GetWorld();
	if (UNavigationSystemV1* NavSystem = World && !bFromTask ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr)
	{
		const UHOLSimulationSettings* Settings = GetDefault<UHOLSimulationSettings>();
		const FVector Extent(Settings ? Settings->OrderTargetProjectionExtent : 1000.f);
		FNavLocation Projected;
		if (NavSystem->ProjectPointToNavigation(Target, Projected, Extent))
		{
			Destination = Projected.Location;
		}
		else
		{
			UE_LOG(LogHOLSimulation, Warning, TEXT("Move order to %s is not on the navmesh: kept as given (D228)"), *Target.ToString());
		}
	}

	FHOLMoveOrder& Order = PendingMoveOrders.AddDefaulted_GetRef();
	Order.UnitIds = UnitIds;
	Order.Target = Destination;
	Order.PlayerIndex = PlayerIndex;
	Order.bFromTask = bFromTask;
	Order.FrontLine = FrontLine;
	Order.bFrontWidth = bFrontWidth;
	Order.bQueue = bQueue;
	Order.bChain = bPlayerOrders && !bFromTask;
	Order.bRally = bRallyOrders && !bFromTask;
}

void UHOLCommandSubsystem::IssueCombatOrder(const TConstArrayView<uint32> UnitIds, const EHOLOrderKind Kind, const uint32 TargetUnitId, const int32 PlayerIndex,
	const bool bQueue)
{
	if (UnitIds.Num() == 0 || Kind == EHOLOrderKind::Move || Kind == EHOLOrderKind::AttackMove || IsRefused(PlayerIndex))
	{
		return;
	}
	FHOLMoveOrder& Order = PendingMoveOrders.AddDefaulted_GetRef();
	Order.UnitIds = UnitIds;
	Order.PlayerIndex = PlayerIndex;
	Order.Kind = Kind;
	Order.TargetUnitId = TargetUnitId;
	Order.bChain = bPlayerOrders && Kind == EHOLOrderKind::Attack;
	// Stop and hold are at once (AoE4): never queued.
	Order.bQueue = bQueue && Kind == EHOLOrderKind::Attack;
}

void UHOLCommandSubsystem::IssueAttackMoveOrder(const TConstArrayView<uint32> UnitIds, const FVector& Target, const int32 PlayerIndex, const bool bQueue)
{
	const int32 Before = PendingMoveOrders.Num();
	IssueMoveOrder(UnitIds, Target, PlayerIndex, /*bFromTask*/ false, FVector2D::ZeroVector, /*bFrontWidth*/ false, bQueue);
	if (PendingMoveOrders.Num() > Before)
	{
		PendingMoveOrders.Last().Kind = EHOLOrderKind::AttackMove;
	}
}

void UHOLCommandSubsystem::IssueCombatWalk(const uint32 UnitId, const FVector& Target, const uint32 CombatStamp, const bool bAttackMove)
{
	const uint32 UnitIds[] = { UnitId };
	const int32 Before = PendingMoveOrders.Num();
	IssueMoveOrder(UnitIds, Target, INDEX_NONE, /*bFromTask*/ true);
	if (PendingMoveOrders.Num() > Before)
	{
		PendingMoveOrders.Last().CombatStamp = CombatStamp;
		PendingMoveOrders.Last().Kind = bAttackMove ? EHOLOrderKind::AttackMove : EHOLOrderKind::Move;
	}
}

void UHOLCommandSubsystem::IssueAttackBuildingOrder(const TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, const int32 PlayerIndex, const bool bQueue)
{
	if (UnitIds.Num() == 0 || !Building || IsRefused(PlayerIndex))
	{
		return;
	}
	FHOLMoveOrder& Order = PendingMoveOrders.AddDefaulted_GetRef();
	Order.UnitIds = UnitIds;
	Order.PlayerIndex = PlayerIndex;
	Order.Kind = EHOLOrderKind::AttackBuilding;
	Order.TargetBuilding = Building;
	Order.bChain = bPlayerOrders;
	Order.bQueue = bQueue;
}

void UHOLCommandSubsystem::IssueGatherOrder(const TConstArrayView<uint32> UnitIds, const int32 SourceIndex, const int32 PlayerIndex, const bool bQueue)
{
	if (UnitIds.Num() == 0 || SourceIndex == INDEX_NONE || IsRefused(PlayerIndex))
	{
		return;
	}
	if (bQueue)
	{
		// In the workers' queue, among the build orders: kept in the order given (Shift + place, Shift + right click...).
		FHOLBuildOrder& Order = PendingBuildOrders.AddDefaulted_GetRef();
		Order.UnitIds = UnitIds;
		Order.SourceIndex = SourceIndex;
		Order.PlayerIndex = PlayerIndex;
		Order.bQueue = true;
		Order.bChain = bPlayerOrders;
		return;
	}
	FHOLGatherOrder& Order = PendingGatherOrders.AddDefaulted_GetRef();
	Order.UnitIds = UnitIds;
	Order.SourceIndex = SourceIndex;
	Order.PlayerIndex = PlayerIndex;
	Order.bChain = bPlayerOrders;
	Order.bRally = bRallyOrders;
}

void UHOLCommandSubsystem::IssueDropOffOrder(const TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, const int32 PlayerIndex, const bool bQueue)
{
	if (UnitIds.Num() == 0 || !Building || IsRefused(PlayerIndex))
	{
		return;
	}
	if (bQueue)
	{
		// In the workers' queue, among the build orders, as a source with Shift (D485, EtapeDefini L.3): their load is judged
		// at its turn.
		FHOLBuildOrder& QueuedOrder = PendingBuildOrders.AddDefaulted_GetRef();
		QueuedOrder.UnitIds = UnitIds;
		QueuedOrder.Building = Building;
		QueuedOrder.bDeposit = true;
		QueuedOrder.PlayerIndex = PlayerIndex;
		QueuedOrder.bQueue = true;
		QueuedOrder.bChain = bPlayerOrders;
		return;
	}
	FHOLGatherOrder& Order = PendingGatherOrders.AddDefaulted_GetRef();
	Order.UnitIds = UnitIds;
	Order.Deposit = Building;
	Order.PlayerIndex = PlayerIndex;
	Order.bChain = bPlayerOrders;
}

void UHOLCommandSubsystem::IssueDropOffMoveOrder(const TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, const FVector& Point,
	const int32 PlayerIndex, const bool bQueue)
{
	if (!Building)
	{
		return;
	}
	// As a move to the point clicked (brought back to the navmesh, D228; refused as a move is), then made a drop-off.
	const int32 Before = PendingMoveOrders.Num();
	IssueMoveOrder(UnitIds, Point, PlayerIndex, /*bFromTask*/ false, FVector2D::ZeroVector, /*bFrontWidth*/ false, bQueue);
	if (PendingMoveOrders.Num() > Before)
	{
		PendingMoveOrders.Last().Kind = EHOLOrderKind::DropOff;
		PendingMoveOrders.Last().TargetBuilding = Building;
	}
}

void UHOLCommandSubsystem::IssueQueuedWorkOrder(const TConstArrayView<uint32> UnitIds, const int32 SourceIndex, AHOLBuilding* Building,
	const FVector& Location, const int32 PlayerIndex)
{
	if (UnitIds.Num() == 0 || (SourceIndex == INDEX_NONE && !Building) || IsRefused(PlayerIndex))
	{
		return;
	}
	// Not brought back to the navmesh as a move: no walk of its own, the gathering and the builders walk the units.
	FHOLMoveOrder& Order = PendingMoveOrders.AddDefaulted_GetRef();
	Order.UnitIds = UnitIds;
	Order.Target = Location;
	Order.PlayerIndex = PlayerIndex;
	Order.Kind = SourceIndex != INDEX_NONE ? EHOLOrderKind::Gather : EHOLOrderKind::Build;
	Order.SourceIndex = SourceIndex;
	if (SourceIndex == INDEX_NONE)
	{
		Order.TargetBuilding = Building;
	}
	Order.bQueue = true;
	Order.bChain = bPlayerOrders;
}

void UHOLCommandSubsystem::ConsumeGatherOrders(TArray<FHOLGatherOrder>& OutOrders)
{
	OutOrders.Reset();
	Swap(OutOrders, PendingGatherOrders);
}

void UHOLCommandSubsystem::ConsumeMoveOrders(TArray<FHOLMoveOrder>& OutOrders)
{
	OutOrders.Reset();
	Swap(OutOrders, PendingMoveOrders);
}

void UHOLCommandSubsystem::IssueProductionOrder(const FHOLProductionCommand& Command)
{
	if (!IsRefused(Command.PlayerIndex))
	{
		PendingProductionOrders.Add(Command);
	}
}

void UHOLCommandSubsystem::ConsumeProductionOrders(TArray<FHOLProductionCommand>& OutCommands)
{
	OutCommands.Reset();
	Swap(OutCommands, PendingProductionOrders);
}

void UHOLCommandSubsystem::IssueConstructionOrder(const FHOLConstructionCommand& Command)
{
	if (!IsRefused(Command.PlayerIndex))
	{
		PendingConstructionOrders.Add(Command);
	}
}

void UHOLCommandSubsystem::ConsumeConstructionOrders(TArray<FHOLConstructionCommand>& OutCommands)
{
	OutCommands.Reset();
	Swap(OutCommands, PendingConstructionOrders);
}

void UHOLCommandSubsystem::IssueBuildOrder(const FHOLBuildOrder& Order)
{
	if (Order.UnitIds.Num() > 0 && (Order.Building.IsValid() || Order.SourceIndex != INDEX_NONE) && !IsRefused(Order.PlayerIndex))
	{
		FHOLBuildOrder& Added = PendingBuildOrders.Add_GetRef(Order);
		Added.bChain |= bPlayerOrders;
		Added.bRally |= bRallyOrders;
	}
}

void UHOLCommandSubsystem::ConsumeBuildOrders(TArray<FHOLBuildOrder>& OutOrders)
{
	OutOrders.Reset();
	Swap(OutOrders, PendingBuildOrders);
}

void UHOLCommandSubsystem::NotifyNavigationChanged(const FBox& Area)
{
	// Only the simulation consumes them: a client would only pile them up.
	const UWorld* World = GetWorld();
	if (World && World->GetNetMode() != NM_Client && Area.IsValid)
	{
		PendingNavigationChanges.Add(Area);
	}
}

void UHOLCommandSubsystem::ConsumeNavigationChanges(TArray<FBox>& OutAreas)
{
	OutAreas.Reset();
	Swap(OutAreas, PendingNavigationChanges);
}

void UHOLCommandSubsystem::NotifyFootprintBlocked(const FHOLBlockedFootprint& Footprint)
{
	// As the changes of the navmesh: only the simulation consumes them.
	const UWorld* World = GetWorld();
	if (World && World->GetNetMode() != NM_Client && Footprint.Area.IsValid)
	{
		PendingBlockedFootprints.Add(Footprint);
	}
}

void UHOLCommandSubsystem::ConsumeBlockedFootprints(TArray<FHOLBlockedFootprint>& OutFootprints)
{
	OutFootprints.Reset();
	Swap(OutFootprints, PendingBlockedFootprints);
}
