// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLVisionSubsystem.h"

#include "Buildings/HOLBuildingTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"
#include "HeroesOfLibertyLog.h"
#include "Map/HOLMapBounds.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLVictoryRules.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

namespace
{
	/** D522: the fog of the matches started from now on; Disabled until the match's configuration carries it (F8). */
	TAutoConsoleVariable<int32> CVarFogMode(
		TEXT("hol.Fog.Mode"),
		0,
		TEXT("Fog of war of the next match (F3, D522): 0 none (everything in sight), 1 explored map, 2 normal (black map)."),
		ECVF_Default);

	FAutoConsoleCommandWithWorldAndArgs FogReportCommand(TEXT("hol.Fog.Report"),
		TEXT("hol.Fog.Report: logs the fog of each camp (cells in sight, explored), its observers and the cost of its last step (server or solo)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* InWorld)
		{
			for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
			{
				const UWorld* World = WorldContext.World();
				const UHOLVisionSubsystem* Vision = World && World->IsGameWorld() ? World->GetSubsystem<UHOLVisionSubsystem>() : nullptr;
				if (Vision)
				{
					UE_LOG(LogHOLMatch, Display, TEXT("%s"), *Vision->MakeReport());
				}
			}
		}));
}

bool UHOLVisionSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLVisionSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	// A client computes its fog from what it receives (F6e, D525).
	UHOLSimulationSubsystem* Simulation = InWorld.GetNetMode() != NM_Client ? InWorld.GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	if (Simulation && !StepHandle.IsValid())
	{
		StepHandle = Simulation->GetOnStepEnded().AddUObject(this, &UHOLVisionSubsystem::OnStepEnded);
	}
}

void UHOLVisionSubsystem::Deinitialize()
{
	if (UHOLSourceSubsystem* Sources = GetWorld() ? GetWorld()->GetSubsystem<UHOLSourceSubsystem>() : nullptr)
	{
		Sources->OnSourceStanding.Remove(SourceHandle);
	}
	if (UHOLSimulationSubsystem* Simulation = GetWorld() ? GetWorld()->GetSubsystem<UHOLSimulationSubsystem>() : nullptr)
	{
		Simulation->GetOnStepEnded().Remove(StepHandle);
	}
	Super::Deinitialize();
}

void UHOLVisionSubsystem::Start(const FHOLMatchConfig& Config, const EHOLFogMode InMode)
{
	Mode = InMode;
	UnitObservers.Reset();
	Stamp = 0;

	// A camp per team, or per slot alone (HOLVictoryRules::GetCamp), packed: Map's camps are only the match's.
	CampOfSlot.Init(INDEX_NONE, Config.Slots.Num());
	TArray<int32> Camps;
	for (int32 Slot = 0; Slot < Config.Slots.Num(); ++Slot)
	{
		const int32 Camp = HOLVictoryRules::GetCamp(Config, Slot);
		if (Camp != INDEX_NONE)
		{
			CampOfSlot[Slot] = Camps.AddUnique(Camp);
		}
	}
	Map.Init(FHOLVisionGrid::MakeCentered(AHOLMapBounds::MaxSideTiles, UHOLBuildingSettings::GetTileSize()), Camps.Num());
	if (Mode == EHOLFogMode::Explored)
	{
		for (int32 Camp = 0; Camp < Camps.Num(); ++Camp)
		{
			Map.ExploreAll(Camp);
		}
	}

	// The forest: the trees standing now, then each one told (a client's too, F6e).
	UHOLSourceSubsystem* Sources = GetWorld() ? GetWorld()->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
	if (Sources)
	{
		Sources->OnSourceStanding.Remove(SourceHandle);
		SourceHandle.Reset();
		if (Mode != EHOLFogMode::Disabled)
		{
			for (int32 Index = 0; Index < Sources->GetSources().Num(); ++Index)
			{
				if (Sources->GetSources()[Index].IsStanding())
				{
					OnSourceStanding(Index, true);
				}
			}
			SourceHandle = Sources->OnSourceStanding.AddUObject(this, &UHOLVisionSubsystem::OnSourceStanding);
		}
	}
	bStarted = true;
	UE_LOG(LogHOLMatch, Log, TEXT("Fog: started, mode %s, camps %d, grid %d x %d cells of %.0f cm"), GetModeName(Mode), Camps.Num(),
		Map.GetGrid().Side, Map.GetGrid().Side, Map.GetGrid().CellSize);
}

void UHOLVisionSubsystem::Update()
{
	const UHOLUnitSubsystem* Units = GetWorld() ? GetWorld()->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	if (!bStarted || Mode == EHOLFogMode::Disabled || !Units)
	{
		return;
	}
	const double StartSeconds = FPlatformTime::Seconds();
	++Stamp;
	Units->ForEachLivingUnitWithType([this](const uint32 UnitId, const FName TypeName, const uint8 Owner, const FVector& Location)
	{
		const int32 Camp = GetCampIndex(Owner);
		if (Camp == INDEX_NONE)
		{
			return;
		}
		const FVector2D Position(Location);
		if (FTracked* Tracked = UnitObservers.Find(UnitId))
		{
			Map.MoveObserver(Tracked->Handle, Position);
			Tracked->Stamp = Stamp;
		}
		else
		{
			UnitObservers.Add(UnitId, FTracked{ Map.AddObserver(Camp, Position, GetUnitSight(TypeName)), Stamp });
		}
	});
	// Dead or gone since the last step: their cells counted off, still explored.
	for (auto It = UnitObservers.CreateIterator(); It; ++It)
	{
		if (It.Value().Stamp != Stamp)
		{
			Map.RemoveObserver(It.Value().Handle);
			It.RemoveCurrent();
		}
	}
	LastUpdateMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.;
}

int32 UHOLVisionSubsystem::GetCampIndex(const int32 Slot) const
{
	return CampOfSlot.IsValidIndex(Slot) ? CampOfSlot[Slot] : INDEX_NONE;
}

EHOLCellVision UHOLVisionSubsystem::GetVision(const int32 Slot, const FVector& Location) const
{
	if (Mode == EHOLFogMode::Disabled)
	{
		return EHOLCellVision::Visible;
	}
	const int32 Camp = GetCampIndex(Slot);
	return Camp == INDEX_NONE ? EHOLCellVision::Unexplored : Map.GetVision(Camp, Map.GetGrid().ToCell(FVector2D(Location)));
}

FString UHOLVisionSubsystem::MakeReport() const
{
	FString Report = FString::Printf(TEXT("Fog: mode %s, camps %d, observers %d"), GetModeName(Mode), Map.GetNumCamps(), Map.GetNumObservers());
	for (int32 Camp = 0; Camp < Map.GetNumCamps(); ++Camp)
	{
		TArray<FString> Slots;
		for (int32 Slot = 0; Slot < CampOfSlot.Num(); ++Slot)
		{
			if (CampOfSlot[Slot] == Camp)
			{
				Slots.Add(FString::FromInt(Slot));
			}
		}
		Report += FString::Printf(TEXT(", camp %d (slots %s) visible %d explored %d"), Camp, *FString::Join(Slots, TEXT(" ")),
			Map.CountVisible(Camp), Map.CountExplored(Camp));
	}
	return Report + FString::Printf(TEXT("; step %.3f ms"), LastUpdateMs);
}

const TCHAR* UHOLVisionSubsystem::GetModeName(const EHOLFogMode InMode)
{
	switch (InMode)
	{
	case EHOLFogMode::Explored: return TEXT("Explored");
	case EHOLFogMode::Normal: return TEXT("Normal");
	default: return TEXT("Disabled");
	}
}

void UHOLVisionSubsystem::OnSourceStanding(const int32 SourceIndex, const bool bStanding)
{
	const UHOLSourceSubsystem* Sources = GetWorld() ? GetWorld()->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
	if (Sources && Sources->BlocksSight(SourceIndex))
	{
		const FVector2D Position(Sources->GetSources()[SourceIndex].Location);
		bStanding ? Map.AddBlocker(Position) : Map.RemoveBlocker(Position);
	}
}

void UHOLVisionSubsystem::OnStepEnded(const int64 StepIndex)
{
	if (!bStarted)
	{
		const AHOLGameState* State = GetWorld() ? GetWorld()->GetGameState<AHOLGameState>() : nullptr;
		Start(State ? State->GetLaunchConfig() : FHOLMatchConfig::MakeDefault(), static_cast<EHOLFogMode>(FMath::Clamp(CVarFogMode.GetValueOnGameThread(), 0, 2)));
	}
	Update();
}

double UHOLVisionSubsystem::GetUnitSight(const FName TypeName)
{
	if (const double* Sight = UnitSights.Find(TypeName))
	{
		return *Sight;
	}
	const UHOLUnitSubsystem* Units = GetWorld() ? GetWorld()->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	const FHOLUnitTypeRow* Row = Units ? Units->FindTypeRow(TypeName) : nullptr;
	return UnitSights.Add(TypeName, Row ? HOLVisionRules::GetSightRadius(Row->SightRadius, 0., FHOLVisionTuning()) : 0.);
}
