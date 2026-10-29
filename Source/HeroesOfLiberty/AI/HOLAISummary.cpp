// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAISummary.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLProductionRules.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "Match/HOLPlayerState.h"
#include "Resources/HOLSourceSubsystem.h"
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
	return FString::Printf(TEXT("slot %d: %d idle; stock %s; sources%s; pop %d/%d (+%d queued); %d buildings; gatherers F %d W %d S %d G %d"),
		PlayerIndex, IdlePeasants.Num(), bHasStock ? *GetStock().ToString() : TEXT("none"), *Nearest, PopulationUsed, PopulationCap,
		PopulationQueued, Buildings.Num(), Food.Peasants, Wood.Peasants, Stone.Peasants, Gold.Peasants);
}

namespace HOLAISummary
{
	FHOLAISummary Make(const int32 PlayerIndex, TConstArrayView<FHOLUnitView> Units, TConstArrayView<FHOLSource> Sources,
		TConstArrayView<FHOLAIDeposit> Deposits, const TOptional<FHOLResources>& Stock, const int32 MaxSourcesPerResource,
		TConstArrayView<FHOLAIBuilding> Buildings, const TOptional<FHOLAIPlayerFigures>& Figures)
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

		// Sources: each standing one the slot may gather (not another slot's farm field, IA1.2: the gather would refuse it,
		// IA0.4 B4) measured from the slot's nearest deposit taking its resource. A few deposits per slot, a few hundred
		// sources: every pair is measured (2 to 4 summaries per second and AI slot, IA-Q2).
		for (int32 SourceIndex = 0; SourceIndex < Sources.Num(); ++SourceIndex)
		{
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

	void GatherDeposits(const UWorld& World, TArray<FHOLAIDeposit>& OutDeposits)
	{
		OutDeposits.Reset();
		for (TActorIterator<AHOLBuilding> It(&World); It; ++It)
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
		for (TActorIterator<AHOLBuilding> It(&World); It; ++It)
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
		const TConstArrayView<FHOLSource> Sources = SourceSystem ? TConstArrayView<FHOLSource>(SourceSystem->GetSources()) : TConstArrayView<FHOLSource>();
		TArray<FHOLAIBuilding> Buildings;
		GatherBuildings(World, Buildings);
		return Make(PlayerIndex, Units, Sources, Deposits, ReadStock(World, PlayerIndex), DefaultMaxSourcesPerResource, Buildings,
			ReadFigures(World, PlayerIndex));
	}
}

FHOLAISummary UHOLAISummaryLibrary::ReadAISummary(const UObject* WorldContextObject, const int32 PlayerIndex)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	return World ? HOLAISummary::Read(*World, PlayerIndex) : FHOLAISummary();
}
