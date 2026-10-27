// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAISummary.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
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
	return FString::Printf(TEXT("slot %d: %d idle; stock %s; sources%s"), PlayerIndex, IdlePeasants.Num(),
		bHasStock ? *GetStock().ToString() : TEXT("none"), *Nearest);
}

namespace HOLAISummary
{
	FHOLAISummary Make(const int32 PlayerIndex, TConstArrayView<FHOLUnitView> Units, TConstArrayView<FHOLSource> Sources,
		TConstArrayView<FHOLAIDeposit> Deposits, const TOptional<FHOLResources>& Stock, const int32 MaxSourcesPerResource)
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
		return Make(PlayerIndex, Units, Sources, Deposits, ReadStock(World, PlayerIndex));
	}
}

FHOLAISummary UHOLAISummaryLibrary::ReadAISummary(const UObject* WorldContextObject, const int32 PlayerIndex)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	return World ? HOLAISummary::Read(*World, PlayerIndex) : FHOLAISummary();
}
