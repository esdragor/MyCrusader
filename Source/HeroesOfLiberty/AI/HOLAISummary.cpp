// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAISummary.h"

#include "Algo/Unique.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLProductionRules.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "Map/HOLVisionSubsystem.h"
#include "Match/HOLFactionRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerState.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLUnitView.h"

FHOLAIResourceSummary& FHOLAISummary::Get(const EHOLResource Resource)
{
	switch (Resource)
	{
	case EHOLResource::Wood: return Wood;
	case EHOLResource::Stone: return Stone;
	case EHOLResource::Gold: return Gold;
	default: return Food;
	}
}

FString FHOLAISummary::ToString() const
{
	FString Nearest;
	for (const EHOLResource Resource : { EHOLResource::Food, EHOLResource::Wood, EHOLResource::Stone, EHOLResource::Gold })
	{
		const TArray<FHOLAISourceNear>& Near = Get(Resource).Sources;
		Nearest += FString::Printf(TEXT(" %c %d"), TEXT("FWSG")[static_cast<int32>(Resource)], Near.Num());
		Nearest += Near.IsEmpty() ? FString() : FString::Printf(TEXT(" (%.0f cm)"), Near[0].DepositDistance);
	}
	return FString::Printf(TEXT("slot %d: %d idle; stock %s; sources%s; pop %d/%d (+%d queued); %d buildings; gatherers F %d W %d S %d G %d; army %d, enemies %d"),
		PlayerIndex, IdlePeasants.Num(), bHasStock ? *GetStock().ToString() : TEXT("none"), *Nearest, PopulationUsed, PopulationCap,
		PopulationQueued, Buildings.Num(), Food.Peasants, Wood.Peasants, Stone.Peasants, Gold.Peasants, Army.Num(), Enemies.Num());
}

namespace
{
	/** A unit as the script reads it (IA2.1): TargetId only while it strikes. */
	FHOLAIUnit MakeAIUnit(const FHOLUnitView& Unit)
	{
		FHOLAIUnit Out;
		Out.Id = static_cast<int32>(Unit.UnitId);
		Out.OwnerSlot = Unit.OwnerPlayer;
		Out.Type = Unit.TypeName;
		Out.Location = Unit.Location;
		Out.Health = Unit.Health;
		Out.bIdle = Unit.Activity == EHOLUnitActivity::Idle;
		Out.TargetId = Unit.Activity == EHOLUnitActivity::Attacking && Unit.TargetUnitId != 0 ? static_cast<int32>(Unit.TargetUnitId) : INDEX_NONE;
		return Out;
	}

	bool IsSlotDefeated(const uint8 DefeatedMask, const int32 Slot)
	{
		return Slot >= 0 && Slot < 8 && (DefeatedMask & (1 << Slot)) != 0;
	}
}

namespace HOLAISummary
{
	FHOLAISummary Make(const int32 PlayerIndex, TConstArrayView<FHOLUnitView> Units, TConstArrayView<FHOLSource> Sources,
		TConstArrayView<FHOLAIDeposit> Deposits, const TOptional<FHOLResources>& Stock, const int32 MaxSourcesPerResource,
		TConstArrayView<FHOLAIBuilding> Buildings, const TOptional<FHOLAIPlayerFigures>& Figures, TConstArrayView<FHOLAISourceUse> Uses,
		TConstArrayView<FHOLUnitView> Enemies, const uint8 DefeatedMask, TConstArrayView<FHOLAIForum> Forums, const bool bRegicide, const FHOLCampTable& Camps,
		const TArray<int32>* CandidateSources)
	{
		FHOLAISummary Summary;
		Summary.PlayerIndex = PlayerIndex;

		// Idle Peasants: as the HUD's idle workers (AHOLPlayerController::UpdateIdleWorkers, D307).
		for (const FHOLUnitView& Unit : Units)
		{
			if (Unit.OwnerPlayer == PlayerIndex && Unit.CarryCapacity > 0 && Unit.Activity == EHOLUnitActivity::Idle)
			{
				Summary.IdlePeasants.Add(static_cast<int32>(Unit.UnitId));
			}
		}
		Summary.IdlePeasants.Sort();

		// IA1.5: all its Peasants, idle or not (the dying left out).
		for (const FHOLUnitView& Unit : Units)
		{
			if (Unit.OwnerPlayer == PlayerIndex && Unit.CarryCapacity > 0 && Unit.Activity != EHOLUnitActivity::Dying)
			{
				Summary.PeasantIds.Add(static_cast<int32>(Unit.UnitId));
			}
		}
		Summary.PeasantIds.Sort();

		// IA2.1: its army, the soldiers (a unit that does not gather), the dying, the King and the hero left out.
		for (const FHOLUnitView& Unit : Units)
		{
			if (Unit.OwnerPlayer == PlayerIndex && Unit.CarryCapacity == 0 && Unit.Activity != EHOLUnitActivity::Dying && !Unit.bKing && !Unit.bHero)
			{
				Summary.Army.Add(MakeAIUnit(Unit));
			}
		}
		Summary.Army.Sort([](const FHOLAIUnit& A, const FHOLAIUnit& B) { return A.Id < B.Id; });

		// IA2.6f: its Peasants as units (place and health), the dying left out.
		for (const FHOLUnitView& Unit : Units)
		{
			if (Unit.OwnerPlayer == PlayerIndex && Unit.CarryCapacity > 0 && Unit.Activity != EHOLUnitActivity::Dying)
			{
				Summary.Peasants.Add(MakeAIUnit(Unit));
			}
		}
		Summary.Peasants.Sort([](const FHOLAIUnit& A, const FHOLAIUnit& B) { return A.Id < B.Id; });

		// IA2.1: the enemies, as GatherEnemies read them (the one place; the fog will cut them there): another slot's, not
		// defeated, not dying, not of the slot's own team (IA4: Camps), by identifier.
		for (const FHOLUnitView& Unit : Enemies)
		{
			if (Unit.OwnerPlayer != PlayerIndex && Camps.AreHostile(PlayerIndex, Unit.OwnerPlayer) && !IsSlotDefeated(DefeatedMask, Unit.OwnerPlayer)
				&& Unit.Activity != EHOLUnitActivity::Dying)
			{
				Summary.Enemies.Add(MakeAIUnit(Unit));
			}
			// IA4 (D571): the units of the slot's teammates (another slot of its camp, not defeated, not dying) are what it may help.
			else if (Unit.OwnerPlayer != PlayerIndex && !Camps.AreHostile(PlayerIndex, Unit.OwnerPlayer) && !IsSlotDefeated(DefeatedMask, Unit.OwnerPlayer)
				&& Unit.Activity != EHOLUnitActivity::Dying)
			{
				Summary.Allies.Add(MakeAIUnit(Unit));
			}
		}
		Summary.Enemies.Sort([](const FHOLAIUnit& A, const FHOLAIUnit& B) { return A.Id < B.Id; });
		Summary.Allies.Sort([](const FHOLAIUnit& A, const FHOLAIUnit& B) { return A.Id < B.Id; });

		// Sources: each standing one the slot may gather (not another slot's farm field, IA1.2: the gather would refuse it,
		// IA0.4 B4) measured from the slot's nearest deposit taking its resource. A few deposits per slot, a few hundred
		// sources: every pair is measured (2 to 4 summaries per second and AI slot, IA-Q2). With CandidateSources (Read, through the
		// grid of the sources: a Forest Medium has 142 935) only those are measured: they hold the ones that are kept.
		const int32 NumToMeasure = CandidateSources ? CandidateSources->Num() : Sources.Num();
		for (int32 Measured = 0; Measured < NumToMeasure; ++Measured)
		{
			const int32 SourceIndex = CandidateSources ? (*CandidateSources)[Measured] : Measured;
			if (!Sources.IsValidIndex(SourceIndex))
			{
				continue;
			}
			const FHOLSource& Source = Sources[SourceIndex];
			if (!Source.IsStanding() || PlayerIndex < 0 || PlayerIndex > MAX_uint8
				|| !Source.IsGatherableBy(static_cast<uint8>(PlayerIndex)))
			{
				continue;
			}
			const FVector2D Position(Source.Location);
			double Nearest = TNumericLimits<double>::Max();
			for (const FHOLAIDeposit& Deposit : Deposits)
			{
				if (Deposit.OwnerPlayer == PlayerIndex && Deposit.Accepts.Contains(Source.Resource))
				{
					Nearest = FMath::Min(Nearest, HOLGather::GetDepositDistance(Position, Deposit.Footprint));
				}
			}
			if (Nearest == TNumericLimits<double>::Max())
			{
				continue;
			}
			FHOLAISourceNear& Near = Summary.Get(Source.Resource).Sources.AddDefaulted_GetRef();
			Near.SourceIndex = SourceIndex;
			Near.Location = Source.Location;
			Near.Remaining = Source.Remaining;
			Near.DepositDistance = static_cast<float>(Nearest);
			if (Uses.IsValidIndex(SourceIndex))
			{
				Near.Places = Uses[SourceIndex].Places;
				Near.Occupants = Uses[SourceIndex].Occupants;
				Near.bFull = Uses[SourceIndex].bFull;
			}
		}
		for (const EHOLResource Resource : { EHOLResource::Food, EHOLResource::Wood, EHOLResource::Stone, EHOLResource::Gold })
		{
			TArray<FHOLAISourceNear>& Near = Summary.Get(Resource).Sources;
			// By distance, then by index: the same summary on every run (the reproducible match, Seed).
			Near.Sort([](const FHOLAISourceNear& A, const FHOLAISourceNear& B)
			{
				return A.DepositDistance != B.DepositDistance ? A.DepositDistance < B.DepositDistance : A.SourceIndex < B.SourceIndex;
			});
			if (Near.Num() > FMath::Max(MaxSourcesPerResource, 0))
			{
				Near.SetNum(FMath::Max(MaxSourcesPerResource, 0));
			}
		}

		if (Stock.IsSet())
		{
			Summary.bHasStock = true;
			for (const EHOLResource Resource : { EHOLResource::Food, EHOLResource::Wood, EHOLResource::Stone, EHOLResource::Gold })
			{
				Summary.Get(Resource).Stock = Stock.GetValue()[Resource];
			}
		}

		// IA1.3: the slot's own buildings and sites, in an order that does not depend on the actors' (the same summary on
		// every run), the centre, the units in the queues.
		for (const FHOLAIBuilding& Building : Buildings)
		{
			if (Building.OwnerPlayer == PlayerIndex)
			{
				Summary.Buildings.Add(Building);
				Summary.PopulationQueued += Building.Queue.Num();
			}
		}
		Summary.Buildings.Sort([](const FHOLAIBuilding& A, const FHOLAIBuilding& B)
		{
			if (A.TypeName != B.TypeName)
			{
				return A.TypeName.LexicalLess(B.TypeName);
			}
			return A.Location.X != B.Location.X ? A.Location.X < B.Location.X : A.Location.Y < B.Location.Y;
		});
		for (const FHOLAIBuilding& Building : Summary.Buildings)
		{
			if (Building.TypeName == HOLBuildingTypeNames::MainCentre)
			{
				Summary.bHasCentre = true;
				Summary.CentreLocation = Building.Location;
				break;
			}
		}
		// IA2.1: the nearest main centre of another slot (not defeated) to its own, a tie to the lower slot, then the first
		// by position; without a centre of its own, the lowest slot's.
		{
			const FHOLAIBuilding* Best = nullptr;
			double BestDistance = 0.0;
			for (const FHOLAIBuilding& Building : Buildings)
			{
				if (Building.OwnerPlayer == PlayerIndex || Building.TypeName != HOLBuildingTypeNames::MainCentre
					|| IsSlotDefeated(DefeatedMask, Building.OwnerPlayer) || !Camps.AreHostile(PlayerIndex, Building.OwnerPlayer))
				{
					continue;
				}
				const double Distance = Summary.bHasCentre ? FVector::DistSquared2D(Building.Location, Summary.CentreLocation) : 0.0;
				const bool bBetter = !Best || Distance < BestDistance
					|| (Distance == BestDistance && (Building.OwnerPlayer < Best->OwnerPlayer
						|| (Building.OwnerPlayer == Best->OwnerPlayer && (Building.Location.X < Best->Location.X
							|| (Building.Location.X == Best->Location.X && Building.Location.Y < Best->Location.Y)))));
				if (bBetter)
				{
					Best = &Building;
					BestDistance = Distance;
				}
			}
			if (Best)
			{
				Summary.bHasEnemyCentre = true;
				Summary.EnemyCentreLocation = Best->Location;
			}
		}
		// IA3.1: the forums (the state is public, D353; the place comes from the slot's main centre, whatever its state, the
		// first by position), its own and the other slots' that are not defeated, by slot; its King; the Regicide flag.
		{
			const auto WithLocation = [&Buildings](FHOLAIForum Forum)
			{
				const FHOLAIBuilding* Centre = nullptr;
				for (const FHOLAIBuilding& Building : Buildings)
				{
					if (Building.OwnerPlayer == Forum.Slot && Building.TypeName == HOLBuildingTypeNames::MainCentre
						&& (!Centre || Building.Location.X < Centre->Location.X
							|| (Building.Location.X == Centre->Location.X && Building.Location.Y < Centre->Location.Y)))
					{
						Centre = &Building;
					}
				}
				Forum.bHasLocation = Centre != nullptr;
				Forum.Location = Centre ? Centre->Location : FVector::ZeroVector;
				return Forum;
			};
			for (const FHOLAIForum& Forum : Forums)
			{
				if (Forum.Slot == PlayerIndex)
				{
					Summary.bHasOwnForum = true;
					Summary.OwnForum = WithLocation(Forum);
				}
				else if (!IsSlotDefeated(DefeatedMask, Forum.Slot) && Camps.AreHostile(PlayerIndex, Forum.Slot))
				{
					Summary.EnemyForums.Add(WithLocation(Forum));
				}
				else if (!IsSlotDefeated(DefeatedMask, Forum.Slot))
				{
					Summary.AllyForums.Add(WithLocation(Forum));	// IA4 (D571): a teammate's (not hostile, not defeated)
				}
			}
			Summary.EnemyForums.Sort([](const FHOLAIForum& A, const FHOLAIForum& B) { return A.Slot < B.Slot; });
			Summary.AllyForums.Sort([](const FHOLAIForum& A, const FHOLAIForum& B) { return A.Slot < B.Slot; });
			for (const FHOLUnitView& Unit : Units)
			{
				if (Unit.OwnerPlayer == PlayerIndex && Unit.bKing && Unit.Activity != EHOLUnitActivity::Dying)
				{
					Summary.bHasKing = true;
					Summary.King = MakeAIUnit(Unit);
					break;
				}
			}
			Summary.bRegicide = bRegicide;
		}
		if (Figures.IsSet())
		{
			Summary.bHasPopulation = true;
			Summary.PopulationUsed = Figures->PopulationUsed;
			Summary.PopulationCap = Figures->PopulationCap;
			for (const EHOLResource Resource : { EHOLResource::Food, EHOLResource::Wood, EHOLResource::Stone, EHOLResource::Gold })
			{
				Summary.Get(Resource).Peasants = Figures->Gatherers[Resource];
			}
		}
		return Summary;
	}

	int32 FindCandidateSources(const FHOLSourceGrid& Grid, const TConstArrayView<FHOLSource> Sources, const int32 PlayerIndex,
		const TConstArrayView<FHOLAIDeposit> Deposits, const int32 MaxSourcesPerResource, const TFunctionRef<bool(const FHOLSource&)> IsKnown,
		TArray<int32>& OutCandidates)
	{
		OutCandidates.Reset();
		const int32 Count = FMath::Max(MaxSourcesPerResource, 0);
		if (PlayerIndex < 0 || PlayerIndex > MAX_uint8 || Count == 0)
		{
			return 0;
		}
		// What Make keeps: standing, gatherable by the slot, known to it.
		const auto Accept = [&Sources, PlayerIndex, &IsKnown](const int32 Index)
		{
			return Sources.IsValidIndex(Index) && Sources[Index].IsStanding() && Sources[Index].IsGatherableBy(static_cast<uint8>(PlayerIndex))
				&& IsKnown(Sources[Index]);
		};
		int32 Visits = 0;
		TArray<int32> Nearest;
		for (const EHOLResource Resource : { EHOLResource::Food, EHOLResource::Wood, EHOLResource::Stone, EHOLResource::Gold })
		{
			for (const FHOLAIDeposit& Deposit : Deposits)
			{
				if (Deposit.OwnerPlayer == PlayerIndex && Deposit.Accepts.Contains(Resource))
				{
					Visits += Grid.FindNearest(Deposit.Footprint, Resource, Count, Accept, Nearest);
					OutCandidates.Append(Nearest);
				}
			}
		}
		OutCandidates.Sort();
		OutCandidates.SetNum(Algo::Unique(OutCandidates));
		return Visits;
	}

	void GatherEnemies(const UWorld& World, const int32 PlayerIndex, TArray<FHOLUnitView>& OutEnemies)
	{
		// The whole world, as the server and a solo game see it (nothing is hidden before the fog, IA2-Q1 A): every unit of
		// another slot that is not defeated, the dying left out (Gather). THE place to cut to the slot's vision (M7, the fog
		// chantier): the rules in script and Make do not change.
		HOLUnitView::Gather(World, OutEnemies);
		const AHOLGameState* GameState = World.GetGameState<AHOLGameState>();
		const uint8 DefeatedMask = GameState ? GameState->GetDefeatedMask() : 0;
		OutEnemies.RemoveAll([PlayerIndex, DefeatedMask](const FHOLUnitView& Unit)
		{
			return Unit.OwnerPlayer == PlayerIndex || IsSlotDefeated(DefeatedMask, Unit.OwnerPlayer);
		});
	}

	void GatherDeposits(const UWorld& World, TArray<FHOLAIDeposit>& OutDeposits)
	{
		OutDeposits.Reset();
		for (FHOLBuildingIterator It(&World); It; ++It)
		{
			const FHOLBuildingTypeRow* Type = It->GetType();
			// As UHOLGatherProcessor::RefreshDeposits: a finished building only (D312), not a forum down (D351).
			if (!Type || Type->Accepts.IsEmpty() || !It->TakesDeliveries())
			{
				continue;
			}
			FHOLAIDeposit& Deposit = OutDeposits.AddDefaulted_GetRef();
			Deposit.Footprint.Centre = FVector2D(It->GetActorLocation());
			Deposit.Footprint.HalfExtent = UHOLBuildingSettings::GetBodySize(Type->Footprint) * 0.5;
			Deposit.Footprint.Yaw = static_cast<float>(It->GetActorRotation().Yaw);
			Deposit.OwnerPlayer = It->GetOwnerPlayer();
			Deposit.Accepts = Type->Accepts;
		}
	}

	void GatherBuildings(const UWorld& World, TArray<FHOLAIBuilding>& OutBuildings)
	{
		OutBuildings.Reset();
		for (FHOLBuildingIterator It(&World); It; ++It)
		{
			const AHOLBuilding* Building = *It;
			// The whole world, as the server and a solo game see it; Make keeps the slot's own (M7 will cut it to the
			// slot's vision).
			FHOLAIBuilding& Out = OutBuildings.AddDefaulted_GetRef();
			Out.OwnerPlayer = Building->GetOwnerPlayer();
			Out.TypeName = Building->GetTypeName();
			Out.State = Building->GetConstructionState();
			Out.Progress = Building->GetProgress();
			Out.Location = Building->GetActorLocation();
			for (const FHOLProductionItem& Item : Building->GetProductionQueue())
			{
				Out.Queue.Add(Item.TypeName);
			}
			Out.bProducing = !Building->GetProductionQueue().IsEmpty() && Building->GetProductionQueue()[0].IsStarted();
		}
	}

	void GatherForums(const UWorld& World, TArray<FHOLAIForum>& OutForums)
	{
		OutForums.Reset();
		const AHOLGameState* GameState = World.GetGameState<AHOLGameState>();
		if (!GameState)
		{
			return;
		}
		const FHOLForumTuning Tuning = FHOLForumTuning::FromSettings();
		const FHOLMatchConfig& Config = GameState->GetLaunchConfig();
		for (int32 Slot = 0; Slot < Config.Slots.Num() && Slot < FHOLMatchConfig::MaxSlots; ++Slot)
		{
			if (Config.Slots[Slot].State == EHOLSlotState::Closed)
			{
				continue;
			}
			const FHOLForumState& State = GameState->GetForumState(Slot);
			FHOLAIForum& Forum = OutForums.AddDefaulted_GetRef();
			Forum.Slot = Slot;
			Forum.Lives = HOLForumRules::GetLives(State, Tuning);
			Forum.bDown = State.IsDown();
			Forum.bRuin = State.Phase == EHOLForumPhase::Ruin;
			Forum.bRebuilding = State.Phase == EHOLForumPhase::Rebuilding;
			Forum.RuinSecondsLeft = Forum.bRuin ? State.RuinStepsLeft * Tuning.StepSeconds : 0.f;
			Forum.RebuildProgress = HOLForumRules::GetRebuildProgress(State, Tuning);
			Forum.HoldProgress = HOLForumRules::GetHoldProgress(State, Tuning);
			Forum.bContested = State.IsContested();
		}
	}

	TOptional<FHOLAIPlayerFigures> ReadFigures(const UWorld& World, const int32 PlayerIndex)
	{
		const AGameStateBase* GameState = World.GetGameState();
		if (!GameState || PlayerIndex == INDEX_NONE)
		{
			return {};
		}
		for (const APlayerState* PlayerState : GameState->PlayerArray)
		{
			const AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState);
			if (HOLPlayerState && HOLPlayerState->GetPlayerIndex() == PlayerIndex)
			{
				FHOLAIPlayerFigures Figures;
				Figures.PopulationUsed = HOLPlayerState->GetPopulationUsed();
				Figures.PopulationCap = HOLPlayerState->GetPopulationCap();
				Figures.Gatherers = HOLPlayerState->GetGatherers();
				return Figures;
			}
		}
		return {};
	}

	TOptional<FHOLResources> ReadStock(const UWorld& World, const int32 PlayerIndex)
	{
		// IA0.1 (a PlayerState for each AI slot) or R7 (AHOLGameState::GetSlotStock): only here.
		const AGameStateBase* GameState = World.GetGameState();
		if (!GameState || PlayerIndex == INDEX_NONE)
		{
			return {};
		}
		for (const APlayerState* PlayerState : GameState->PlayerArray)
		{
			const AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState);
			if (HOLPlayerState && HOLPlayerState->GetPlayerIndex() == PlayerIndex)
			{
				return HOLPlayerState->GetResources();
			}
		}
		return {};
	}

	FHOLAISummary Read(const UWorld& World, const int32 PlayerIndex)
	{
		TArray<FHOLUnitView> Units;
		HOLUnitView::Gather(World, Units);
		TArray<FHOLAIDeposit> Deposits;
		GatherDeposits(World, Deposits);
		const UHOLSourceSubsystem* SourceSystem = World.GetSubsystem<UHOLSourceSubsystem>();
		TConstArrayView<FHOLSource> Sources = SourceSystem ? TConstArrayView<FHOLSource>(SourceSystem->GetSources()) : TConstArrayView<FHOLSource>();
		// F-Q11 = B (D519): under the fog, the AI knows only the sources of the cells its camp explored, as a player; the
		// others count as used up here (their indices kept).
		TArray<FHOLSource> Known;
		const UHOLVisionSubsystem* Vision = World.GetSubsystem<UHOLVisionSubsystem>();
		const bool bFog = Vision && Vision->IsStarted() && Vision->GetMode() != EHOLFogMode::Disabled;
		TArray<FHOLAIBuilding> Buildings;
		GatherBuildings(World, Buildings);
		// 2026-11-05: only the sources Make can keep are found (through the grid of the sources) and measured; a Forest Medium has
		// 142 935 (Think 33 ms against 1.3). A client has no grid: every source, as before.
		TArray<int32> Candidates;
		const bool bCandidates = SourceSystem && SourceSystem->HasAuthority();
		if (bCandidates)
		{
			FindCandidateSources(SourceSystem->GetGrid(), Sources, PlayerIndex, Deposits, DefaultMaxSourcesPerResource, [Vision, bFog, PlayerIndex](const FHOLSource& Source)
			{
				return !bFog || Vision->GetVision(PlayerIndex, Source.Location) != EHOLCellVision::Unexplored;
			}, Candidates);
		}
		else if (bFog)
		{
			Known = Sources;
			for (FHOLSource& Source : Known)
			{
				if (Vision->GetVision(PlayerIndex, Source.Location) == EHOLCellVision::Unexplored)
				{
					Source.Remaining = 0;
				}
			}
			Sources = Known;
		}
		TArray<FHOLUnitView> Enemies;
		GatherEnemies(World, PlayerIndex, Enemies);
		const AHOLGameState* GameState = World.GetGameState<AHOLGameState>();
		TArray<FHOLAIForum> Forums;
		GatherForums(World, Forums);
		FHOLAISummary Summary = Make(PlayerIndex, Units, Sources, Deposits, ReadStock(World, PlayerIndex), DefaultMaxSourcesPerResource, Buildings,
			ReadFigures(World, PlayerIndex), {}, Enemies, GameState ? GameState->GetDefeatedMask() : 0, Forums,
			GameState && GameState->GetLaunchConfig().bRegicide, FHOLCampTable::FromWorld(&World), bCandidates ? &Candidates : nullptr);
		// IA1.7b: the places of each source of few places and the ones held (the processor's, published at each step), for the
		// sources kept only.
		if (SourceSystem)
		{
			for (const EHOLResource Resource : { EHOLResource::Food, EHOLResource::Wood, EHOLResource::Stone, EHOLResource::Gold })
			{
				for (FHOLAISourceNear& Near : Summary.Get(Resource).Sources)
				{
					const FHOLSourceTypeRow* Type = Sources.IsValidIndex(Near.SourceIndex) ? SourceSystem->GetType(Sources[Near.SourceIndex].TypeIndex) : nullptr;
					Near.Places = Type ? Type->Places : 0;
					Near.Occupants = Near.Places > 0 ? SourceSystem->GetOccupants(Near.SourceIndex) : 0;
					Near.bFull = SourceSystem->IsFull(Near.SourceIndex);
				}
			}
		}
		// IA4 (D579): the slot's level, from the launch configuration (Normal for a slot out of its table).
		if (GameState && GameState->GetLaunchConfig().Slots.IsValidIndex(PlayerIndex))
		{
			Summary.Level = GameState->GetLaunchConfig().Slots[PlayerIndex].Difficulty;
		}
		if (GameState)
		{
			Summary.Tier = GameState->GetTier(PlayerIndex);
		}
		// Palier 3 G14: the slot's faction, so that the script brain plans only what its faction makes.
		// Palier 3 G4: the key the RULES read (a faction without rules of its own, Fire / Dragon / Shadow / Templars, reads as the Dawn: the server accepts the
		// Dawn's Arquebusier for it), not the raw lobby key, so that the script filter and the server agree.
		Summary.Faction = HOLFactionRules::GetTable().ResolveKey(HOLFactionRules::GetSlotFactionKey(&World, PlayerIndex));
		return Summary;
	}
}

FHOLAISummary UHOLAISummaryLibrary::ReadAISummary(const UObject* WorldContextObject, const int32 PlayerIndex)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	return World ? HOLAISummary::Read(*World, PlayerIndex) : FHOLAISummary();
}
