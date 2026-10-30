// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLVisionSubsystem.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"
#include "GameFramework/PlayerController.h"
#include "HeroesOfLibertyLog.h"
#include "Map/HOLMapBounds.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"
#include "Match/HOLVictoryRules.h"
#include "Net/HOLFogNetSubsystem.h"
#include "Net/HOLUnitReceptionSubsystem.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
#include "Simulation/HOLUnitView.h"

namespace
{
	/** D522: the fog of the matches started from now on; Disabled until the match's configuration carries it (F8). */
	TAutoConsoleVariable<int32> CVarFogMode(
		TEXT("hol.Fog.Mode"),
		0,
		TEXT("Fog of war of the next match (F3, D522): 0 none (everything in sight), 1 explored map, 2 normal (black map)."),
		ECVF_Default);

	FAutoConsoleCommandWithWorldAndArgs FogReportCommand(TEXT("hol.Fog.Report"),
		TEXT("hol.Fog.Report: logs the fog of each camp (cells in sight, explored, ghosts), its observers, the cost of its last step and, on a server, its network groups"),
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
				if (const UHOLFogNetSubsystem* FogNet = Vision && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLFogNetSubsystem>() : nullptr)
				{
					UE_LOG(LogHOLMatch, Display, TEXT("%s"), *FogNet->MakeReport());
					// F6f: what the server sent of the sources to each camp, for the leak test to set against the client's.
					TArray<FString> Sent;
					for (int32 Camp = 0; Camp < Vision->GetNumCamps(); ++Camp)
					{
						int32 NumSent = 0;
						int32 NumUnexplored = 0;
						FogNet->CountSentSources(Camp, NumSent, NumUnexplored);
						Sent.Add(FString::Printf(TEXT("camp %d sent %d (on unexplored cells %d)"), Camp, NumSent, NumUnexplored));
					}
					UE_LOG(LogHOLMatch, Display, TEXT("Fog sources: %s"), *FString::Join(Sent, TEXT(", ")));
				}
				const FString Leaks = Vision ? Vision->MakeLeakReport() : FString();
				if (!Leaks.IsEmpty())
				{
					UE_LOG(LogHOLMatch, Display, TEXT("%s"), *Leaks);
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
	// A client computes its fog from what it receives (F6e, D525), each frame (Tick).
	bClient = InWorld.GetNetMode() == NM_Client;
	UHOLSimulationSubsystem* Simulation = !bClient ? InWorld.GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
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
	BuildingObservers.Reset();
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
	Ghosts.Reset();
	Ghosts.SetNum(Camps.Num());
	GhostVersions.Reset();
	GhostVersions.SetNumZeroed(Camps.Num());
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
		// D389: the resources revealed from the start, remembered as they are now.
		if (Mode == EHOLFogMode::Explored)
		{
			for (int32 Index = 0; Index < Sources->GetSources().Num(); ++Index)
			{
				const FHOLSource& Source = Sources->GetSources()[Index];
				if (Source.IsStanding() && !Sources->IsLinked(Index))
				{
					const FHOLGhost Ghost{ HOLVisionRules::MakeGhostKey(true, Index), Sources->GetTypeNames()[Source.TypeIndex], FHOLSource::NoOwner,
						FVector2D(Source.Location), EHOLBuildingSight::Standing, Source.Remaining };
					for (TMap<uint64, FHOLGhost>& Memory : Ghosts)
					{
						Memory.Add(Ghost.Key, Ghost);
					}
				}
			}
		}
	}
	bStarted = true;
	UE_LOG(LogHOLMatch, Log, TEXT("Fog: started, mode %s, camps %d, grid %d x %d cells of %.0f cm"), GetModeName(Mode), Camps.Num(),
		Map.GetGrid().Side, Map.GetGrid().Side, Map.GetGrid().CellSize);
}

void UHOLVisionSubsystem::BeginPass()
{
	++Stamp;
	Present.Reset();
}

void UHOLVisionSubsystem::ObserveUnit(const uint32 UnitId, const FName TypeName, const uint8 Owner, const FVector& Location)
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
}

void UHOLVisionSubsystem::ObserveBuilding(const AHOLBuilding& Building, const bool bObserve)
{
	const FHOLBuildingTypeRow* Type = Building.GetType();
	const int32 Camp = Type ? GetCampIndex(Building.GetOwnerPlayer()) : INDEX_NONE;
	if (Camp == INDEX_NONE)
	{
		return;
	}
	EHOLBuildingSight State = Building.IsFinished() ? EHOLBuildingSight::Standing : EHOLBuildingSight::Site;
	if (Building.IsPrimaryForum() && Building.IsForumDown())
	{
		State = Building.GetShownLook() == EHOLForumLook::Rubble ? EHOLBuildingSight::Rubble : EHOLBuildingSight::Site;
	}
	const double Radius = HOLVisionRules::GetBuildingSight(Type->SightRadius, State);
	Present.Add(FHOLGhost{ HOLVisionRules::MakeGhostKey(false, Building.GetUniqueID()), Building.GetTypeName(), Building.GetOwnerPlayer(),
		FVector2D(Building.GetActorLocation()), State, 0 });
	if (!bObserve)
	{
		return;
	}
	if (FTracked* Tracked = BuildingObservers.Find(Building.GetUniqueID()))
	{
		if (Tracked->Radius != Radius)
		{
			Map.SetObserverRadius(Tracked->Handle, Radius);
			Tracked->Radius = Radius;
		}
		Tracked->Stamp = Stamp;
	}
	else
	{
		BuildingObservers.Add(Building.GetUniqueID(), FTracked{ Map.AddObserver(Camp, FVector2D(Building.GetActorLocation()), Radius), Stamp, Radius });
	}
}

void UHOLVisionSubsystem::EndPass(const bool bGhosts, const int32 OnlyCamp)
{
	// Dead, destroyed or gone since the last pass: their cells counted off, still explored.
	for (TMap<uint32, FTracked>* Observers : { &UnitObservers, &BuildingObservers })
	{
		for (auto It = Observers->CreateIterator(); It; ++It)
		{
			if (It.Value().Stamp != Stamp)
			{
				Map.RemoveObserver(It.Value().Handle);
				It.RemoveCurrent();
			}
		}
	}
	if (!bGhosts)
	{
		return;
	}
	// F4: each camp's memory of the buildings and sources (a farm's field stands with its farm, not on its own).
	if (const UHOLSourceSubsystem* Sources = GetWorld()->GetSubsystem<UHOLSourceSubsystem>())
	{
		for (int32 Index = 0; Index < Sources->GetSources().Num(); ++Index)
		{
			const FHOLSource& Source = Sources->GetSources()[Index];
			if (Source.IsStanding() && !Sources->IsLinked(Index))
			{
				Present.Add(FHOLGhost{ HOLVisionRules::MakeGhostKey(true, Index), Sources->GetTypeNames()[Source.TypeIndex], FHOLSource::NoOwner,
					FVector2D(Source.Location), EHOLBuildingSight::Standing, Source.Remaining });
			}
		}
	}
	for (int32 Camp = 0; Camp < Ghosts.Num(); ++Camp)
	{
		if (OnlyCamp != INDEX_NONE && Camp != OnlyCamp)
		{
			continue;
		}
		if (HOLVisionRules::UpdateGhosts(Ghosts[Camp], Present,
			[this, Camp](const FVector2D& Position) { return Map.GetCount(Camp, Map.GetGrid().ToCell(Position)) > 0; }))
		{
			++GhostVersions[Camp];
		}
	}
}

void UHOLVisionSubsystem::Update()
{
	const UHOLUnitSubsystem* Units = GetWorld() ? GetWorld()->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	if (!bStarted || Mode == EHOLFogMode::Disabled || !Units)
	{
		return;
	}
	const double StartSeconds = FPlatformTime::Seconds();
	BeginPass();
	Units->ForEachLivingUnitWithType([this](const uint32 UnitId, const FName TypeName, const uint8 Owner, const FVector& Location)
	{
		ObserveUnit(UnitId, TypeName, Owner, Location);
	});
	// F3.2: the buildings, a site half as far, a forum in ruin blind (its sight counted again only when it changes).
	for (TActorIterator<AHOLBuilding> It(GetWorld()); It; ++It)
	{
		if (IsValid(*It))
		{
			ObserveBuilding(**It, true);
		}
	}
	EndPass(true);
	LastUpdateMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.;
}

bool UHOLVisionSubsystem::MakeMemoryFor(const int32 Slot, TArray<uint32>& OutRuns, TArray<FHOLFogGhostNet>& OutGhosts) const
{
	const int32 Camp = bStarted && Mode != EHOLFogMode::Disabled ? GetCampIndex(Slot) : INDEX_NONE;
	if (Camp == INDEX_NONE || !Ghosts.IsValidIndex(Camp) || Map.CountExplored(Camp) == 0)
	{
		return false;
	}
	OutRuns = Map.GetExploredRuns(Camp);
	OutGhosts.Reset();
	for (const TPair<uint64, FHOLGhost>& Pair : Ghosts[Camp])
	{
		if ((Pair.Key >> 32) == 0)
		{
			OutGhosts.Add(FHOLFogGhostNet{ Pair.Value.TypeName, Pair.Value.Owner, Pair.Value.Position, static_cast<uint8>(Pair.Value.State) });
		}
	}
	return true;
}

void UHOLVisionSubsystem::ReceiveMemory(const TArray<uint32>& Runs, const TArray<FHOLFogGhostNet>& BuildingGhosts)
{
	PendingRuns = Runs;
	PendingGhosts = BuildingGhosts;
	bMemoryPending = true;
}

bool UHOLVisionSubsystem::UpdateClient()
{
	const UHOLUnitReceptionSubsystem* Reception = GetWorld() ? GetWorld()->GetSubsystem<UHOLUnitReceptionSubsystem>() : nullptr;
	if (!bStarted || Mode == EHOLFogMode::Disabled || !Reception)
	{
		return false;
	}
	// Its own camp: the slot of the local player (INDEX_NONE until its state has come).
	const APlayerController* Controller = GetWorld()->GetFirstPlayerController();
	const AHOLPlayerState* PlayerState = Controller ? Controller->GetPlayerState<AHOLPlayerState>() : nullptr;
	ClientCamp = PlayerState ? GetCampIndex(PlayerState->GetPlayerIndex()) : INDEX_NONE;
	if (ClientCamp == INDEX_NONE)
	{
		return false;
	}
	if (bMemoryPending)
	{
		// F6e.3: what the server kept for this player, who comes back: the cells explored, and the buildings seen (keys of their own, made here).
		bMemoryPending = false;
		const bool bRead = Map.AddExploredRuns(ClientCamp, PendingRuns);
		for (int32 Index = 0; bRead && Index < PendingGhosts.Num(); ++Index)
		{
			const FHOLFogGhostNet& Net = PendingGhosts[Index];
			const FHOLGhost Ghost{ HOLVisionRules::MakeGhostKey(false, 0x40000000u + Index), Net.TypeName, Net.Owner, Net.Position,
				static_cast<EHOLBuildingSight>(Net.State), 0 };
			Ghosts[ClientCamp].Add(Ghost.Key, Ghost);
		}
		++GhostVersions[ClientCamp];
		UE_LOG(LogHOLMatch, Display, TEXT("Fog: memory received, %s, %d explored cells, %d building ghosts"), bRead ? TEXT("taken") : TEXT("refused"),
			Map.CountExplored(ClientCamp), PendingGhosts.Num());
		PendingRuns.Reset();
		PendingGhosts.Reset();
	}
	const double StartSeconds = FPlatformTime::Seconds();
	BeginPass();
	// The units of its camp only (an ally's too): the others it holds are the ones it sees, and they see nothing for it.
	TArray<FHOLUnitView> Views;
	Reception->GetDisplayedUnits(Views);
	for (const FHOLUnitView& View : Views)
	{
		if (GetCampIndex(View.OwnerPlayer) == ClientCamp)
		{
			ObserveUnit(View.UnitId, View.TypeName, View.OwnerPlayer, View.Location);
		}
	}
	// All the buildings it holds are present for its memory (F6e.2: an enemy's it sees now, until the server stops sending it); its camp's observe.
	for (TActorIterator<AHOLBuilding> It(GetWorld()); It; ++It)
	{
		if (IsValid(*It))
		{
			ObserveBuilding(**It, GetCampIndex(It->GetOwnerPlayer()) == ClientCamp);
		}
	}
	EndPass(true, ClientCamp);
	LastUpdateMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.;
	return true;
}

void UHOLVisionSubsystem::Tick(const float DeltaTime)
{
	// D521: a client's observers are walked 20 times a second (the cells change slowly).
	ClientSeconds += DeltaTime;
	if (ClientSeconds >= 0.05f && TryStart())
	{
		ClientSeconds = 0.f;
		UpdateClient();
	}
}

TStatId UHOLVisionSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UHOLVisionSubsystem, STATGROUP_Tickables);
}

const TMap<uint64, FHOLGhost>* UHOLVisionSubsystem::GetGhosts(const int32 Slot) const
{
	const int32 Camp = Mode == EHOLFogMode::Disabled ? INDEX_NONE : GetCampIndex(Slot);
	return Ghosts.IsValidIndex(Camp) ? &Ghosts[Camp] : nullptr;
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

bool UHOLVisionSubsystem::IsUnitShown(const int32 ViewerSlot, const int32 OwnerSlot, const FVector& Location) const
{
	const int32 ViewerCamp = GetCampIndex(ViewerSlot);
	if (!bStarted || Mode == EHOLFogMode::Disabled || ViewerCamp == INDEX_NONE || ViewerCamp == GetCampIndex(OwnerSlot))
	{
		return true;
	}
	return Map.GetVision(ViewerCamp, Map.GetGrid().ToCell(FVector2D(Location))) == EHOLCellVision::Visible;
}

bool UHOLVisionSubsystem::IsAreaVisible(const int32 Slot, const FVector& Center, const FVector2D& Extent) const
{
	if (Mode == EHOLFogMode::Disabled)
	{
		return true;
	}
	const int32 Camp = GetCampIndex(Slot);
	if (Camp == INDEX_NONE)
	{
		return false;
	}
	const FHOLVisionGrid& Grid = Map.GetGrid();
	const FIntPoint Low = Grid.ToCell(FVector2D(Center) - Extent);
	const FIntPoint High = Grid.ToCell(FVector2D(Center) + Extent);
	for (int32 Y = Low.Y; Y <= High.Y; ++Y)
	{
		for (int32 X = Low.X; X <= High.X; ++X)
		{
			if (Map.GetCount(Camp, FIntPoint(X, Y)) > 0)
			{
				return true;
			}
		}
	}
	return false;
}

uint32 UHOLVisionSubsystem::GetCampsSeeing(const FVector& Center, const FVector2D& Extent) const
{
	if (Mode == EHOLFogMode::Disabled)
	{
		return HOLVisionRules::AllCamps;
	}
	const FHOLVisionGrid& Grid = Map.GetGrid();
	const FIntPoint Low = Grid.ToCell(FVector2D(Center) - Extent);
	const FIntPoint High = Grid.ToCell(FVector2D(Center) + Extent);
	uint32 Camps = 0u;
	for (int32 Camp = 0; Camp < Map.GetNumCamps() && Camp < 32; ++Camp)
	{
		for (int32 Y = Low.Y; Y <= High.Y && !(Camps & (1u << Camp)); ++Y)
		{
			for (int32 X = Low.X; X <= High.X; ++X)
			{
				if (Map.GetCount(Camp, FIntPoint(X, Y)) > 0)
				{
					Camps |= 1u << Camp;
					break;
				}
			}
		}
	}
	return Camps;
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
		// F6e.2: among its ghosts, the buildings of other camps (what it remembers of the enemy's).
		int32 Others = 0;
		if (Ghosts.IsValidIndex(Camp))
		{
			for (const TPair<uint64, FHOLGhost>& Pair : Ghosts[Camp])
			{
				Others += (Pair.Key >> 32) == 0 && GetCampIndex(Pair.Value.Owner) != Camp ? 1 : 0;
			}
		}
		Report += FString::Printf(TEXT(", camp %d (slots %s) visible %d explored %d ghosts %d (buildings of others %d)"), Camp, *FString::Join(Slots, TEXT(" ")),
			Map.CountVisible(Camp), Map.CountExplored(Camp), Ghosts.IsValidIndex(Camp) ? Ghosts[Camp].Num() : 0, Others);
	}
	return Report + FString::Printf(TEXT("; step %.3f ms"), LastUpdateMs);
}

FString UHOLVisionSubsystem::MakeLeakReport() const
{
	const UHOLUnitReceptionSubsystem* Reception = GetWorld() ? GetWorld()->GetSubsystem<UHOLUnitReceptionSubsystem>() : nullptr;
	const UHOLSourceSubsystem* Sources = GetWorld() ? GetWorld()->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
	if (!bClient || !bStarted || ClientCamp == INDEX_NONE || !Reception || !Sources)
	{
		return FString();
	}
	const FHOLVisionGrid& Grid = Map.GetGrid();
	int32 NumSources = 0;
	int32 SourcesOut = 0;
	for (const FHOLSource& Source : Sources->GetSources())
	{
		if (Source.IsStanding())
		{
			++NumSources;
			SourcesOut += Map.GetVision(ClientCamp, Grid.ToCell(FVector2D(Source.Location))) == EHOLCellVision::Unexplored ? 1 : 0;
		}
	}
	TArray<FHOLUnitView> Views;
	Reception->GetDisplayedUnits(Views);
	int32 NumUnits = 0;
	int32 UnitsOut = 0;
	for (const FHOLUnitView& View : Views)
	{
		if (GetCampIndex(View.OwnerPlayer) != ClientCamp)
		{
			++NumUnits;
			UnitsOut += Map.GetVision(ClientCamp, Grid.ToCell(FVector2D(View.Location))) == EHOLCellVision::Visible ? 0 : 1;
		}
	}
	int32 NumBuildings = 0;
	int32 BuildingsOut = 0;
	for (TActorIterator<AHOLBuilding> It(GetWorld()); It; ++It)
	{
		const FHOLBuildingTypeRow* Type = IsValid(*It) && GetCampIndex(It->GetOwnerPlayer()) != ClientCamp ? It->GetType() : nullptr;
		if (!Type)
		{
			continue;
		}
		++NumBuildings;
		const FVector2D Center(It->GetActorLocation());
		const FVector2D Extent = 0.5 * Type->Footprint;
		const FIntPoint Low = Grid.ToCell(Center - Extent);
		const FIntPoint High = Grid.ToCell(Center + Extent);
		bool bSeen = false;
		for (int32 Y = Low.Y; Y <= High.Y && !bSeen; ++Y)
		{
			for (int32 X = Low.X; X <= High.X && !bSeen; ++X)
			{
				bSeen = Map.GetCount(ClientCamp, FIntPoint(X, Y)) > 0;
			}
		}
		BuildingsOut += bSeen ? 0 : 1;
	}
	return FString::Printf(TEXT("Fog leaks: camp %d, sources %d held %d on unexplored cells, units of others %d held %d out of sight, buildings of others %d held %d out of sight"),
		ClientCamp, NumSources, SourcesOut, NumUnits, UnitsOut, NumBuildings, BuildingsOut);
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

bool UHOLVisionSubsystem::TryStart()
{
	if (!bStarted)
	{
		// A match only (not the menu's world): its launch configuration set (InitVictory).
		const AHOLGameState* State = GetWorld() ? GetWorld()->GetGameState<AHOLGameState>() : nullptr;
		if (!State || !State->HasStartPlaces())
		{
			return false;
		}
		// The server decides the mode and tells its clients (F6e); a client waits for it.
		int32 ModeValue = State->GetFogMode();
		if (!bClient)
		{
			ModeValue = FMath::Clamp(CVarFogMode.GetValueOnGameThread(), 0, 2);
			GetWorld()->GetGameState<AHOLGameState>()->SetFogMode(ModeValue);
		}
		else if (ModeValue < 0)
		{
			return false;
		}
		Start(State->GetLaunchConfig(), static_cast<EHOLFogMode>(FMath::Clamp(ModeValue, 0, 2)));
	}
	return true;
}

void UHOLVisionSubsystem::OnStepEnded(const int64 StepIndex)
{
	if (TryStart())
	{
		Update();
	}
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
