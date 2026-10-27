// Copyright Epic Games, Inc. All Rights Reserved.

#include "Resources/HOLSourceSubsystem.h"

#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Resources/HOLSourceChunk.h"
#include "Resources/HOLSourceField.h"
#include "Resources/HOLSourceLayout.h"
#include "Resources/HOLSourceTypes.h"

namespace UE::HeroesOfLiberty::Resources
{
	/** Calls Function on the registry of every game world (the editor's server and a client may share a process). */
	void ForEachRegistry(TFunctionRef<void(UHOLSourceSubsystem&)> Function)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			UWorld* World = Context.World();
			if (UHOLSourceSubsystem* Registry = World && World->IsGameWorld() ? World->GetSubsystem<UHOLSourceSubsystem>() : nullptr)
			{
				Function(*Registry);
			}
		}
	}

	FAutoConsoleCommand CommandReport(TEXT("hol.Sources.Report"),
		TEXT("Resource sources of the match (J5, D297): standing, used up, left per type, shown, forbidden on the navmesh"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			ForEachRegistry([](UHOLSourceSubsystem& Registry)
			{
				UE_LOG(LogHOLMatch, Display, TEXT("%s"), *Registry.MakeReport());
			});
		}));

	FAutoConsoleCommand CommandNearest(TEXT("hol.Sources.Nearest"),
		TEXT("hol.Sources.Nearest X Y: logs the standing source nearest a ground point (cm): index, type, location, what is left"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Sources: usage hol.Sources.Nearest X Y"));
				return;
			}
			const FVector Point(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), 0.);
			ForEachRegistry([&Point](UHOLSourceSubsystem& Registry)
			{
				const int32 Index = Registry.FindNearestStanding(Point);
				if (Index == INDEX_NONE)
				{
					UE_LOG(LogHOLMatch, Display, TEXT("Sources: no standing source"));
					return;
				}
				const FHOLSource& Source = Registry.GetSources()[Index];
				UE_LOG(LogHOLMatch, Display, TEXT("Sources: nearest %d %s at (%.0f, %.0f), %d left, %.0f cm away"), Index,
					*Registry.GetTypeNames()[Source.TypeIndex].ToString(), Source.Location.X, Source.Location.Y, Source.Remaining,
					FVector::Dist2D(Point, Source.Location));
			});
		}));

#if !UE_BUILD_SHIPPING
	// A development tool, like the cheats of D291: empties a source before the gathering exists (J5a.3).
	FAutoConsoleCommand CommandTake(TEXT("hol.Sources.Take"),
		TEXT("hol.Sources.Take Index [Amount]: takes from a source on the server or in a solo game (all that is left by default)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 1)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Sources: usage hol.Sources.Take Index [Amount]"));
				return;
			}
			const int32 Index = FCString::Atoi(*Args[0]);
			const int32 Amount = Args.Num() > 1 ? FCString::Atoi(*Args[1]) : MAX_int32;
			ForEachRegistry([Index, Amount](UHOLSourceSubsystem& Registry)
			{
				if (Registry.HasAuthority())
				{
					const int32 Taken = Registry.Take(Index, Amount);
					UE_LOG(LogHOLMatch, Display, TEXT("Sources: took %d from source %d"), Taken, Index);
				}
			});
		}));

	// The same on the standing source nearest a point: the scripted checks of J5a.5 thin out the tree a client will
	// gather (the one nearest its start), so that its gathering uses it up within seconds.
	FAutoConsoleCommand CommandTakeNear(TEXT("hol.Sources.TakeNear"),
		TEXT("hol.Sources.TakeNear X Y [Amount]: hol.Sources.Take on the standing source nearest a ground point (cm)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Sources: usage hol.Sources.TakeNear X Y [Amount]"));
				return;
			}
			const FVector Point(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), 0.);
			const int32 Amount = Args.Num() > 2 ? FCString::Atoi(*Args[2]) : MAX_int32;
			ForEachRegistry([&Point, Amount](UHOLSourceSubsystem& Registry)
			{
				const int32 Index = Registry.HasAuthority() ? Registry.FindNearestStanding(Point) : INDEX_NONE;
				if (Index != INDEX_NONE)
				{
					const int32 Taken = Registry.Take(Index, Amount);
					UE_LOG(LogHOLMatch, Display, TEXT("Sources: took %d from source %d"), Taken, Index);
				}
			});
		}));
#endif
}

bool UHOLSourceSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLSourceSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	const ENetMode NetMode = InWorld.GetNetMode();
	bAuthority = NetMode != NM_Client;
	bReplicate = NetMode == NM_DedicatedServer || NetMode == NM_ListenServer;
	LoadTypes();

	FActorSpawnParameters Parameters;
	Parameters.Name = TEXT("HOLSourceField");
	Parameters.ObjectFlags |= RF_Transient;
	Field = InWorld.SpawnActor<AHOLSourceField>(Parameters);
	if (Field)
	{
		Field->Init(/*bDisplay*/ NetMode != NM_DedicatedServer, /*bNavigation*/ bAuthority);
	}
	bStarted = true;

	if (!bAuthority)
	{
		// Chunks received before the world began play.
		for (TActorIterator<AHOLSourceChunk> It(&InWorld); It; ++It)
		{
			ReadChunk(**It);
		}
		return;
	}

	int32 NumPlaced = 0;
	for (TActorIterator<AHOLSourceLayout> It(&InWorld); It; ++It)
	{
		for (const FHOLPlacedSource& Placed : It->GetSources())
		{
			// A linked type (a farm field, IA1.2) is born of its building, never of the map.
			const int32 PlacedType = TypeNames.IndexOfByKey(Placed.Type);
			if (PlacedType != INDEX_NONE && TypeRows[PlacedType] && TypeRows[PlacedType]->Linked)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Source %s placed by the map: linked to a building, skipped"), *Placed.Type.ToString());
				continue;
			}
			NumPlaced += AddSource(Placed.Type, Placed.Location) != INDEX_NONE ? 1 : 0;
		}
	}
	if (Field)
	{
		Field->RegisterNavigation();
	}
	if (NumPlaced > 0)
	{
		UE_LOG(LogHOLMatch, Log, TEXT("%d resource sources placed by the map"), NumPlaced);
	}
}

void UHOLSourceSubsystem::LoadTypes()
{
	TypesTable = GetDefault<UHOLSourceSettings>()->SourceTypesTable.LoadSynchronous();
	TypeNames.Reset();
	TypeRows.Reset();
	if (!TypesTable)
	{
		UE_LOG(LogHOLMatch, Error, TEXT("No source type table (HOLSourceSettings.SourceTypesTable in DefaultGame.ini): no sources"));
		return;
	}
	// The table's order is the same on every machine (the same asset): it numbers the types on the network.
	for (const FName& Name : TypesTable->GetRowNames())
	{
		if (TypeNames.Num() >= FHOLSource::NoType)
		{
			break;
		}
		TypeNames.Add(Name);
		TypeRows.Add(TypesTable->FindRow<FHOLSourceTypeRow>(Name, TEXT("source type")));
	}
}

const FHOLSourceTypeRow* UHOLSourceSubsystem::GetType(const uint8 TypeIndex) const
{
	return TypeRows.IsValidIndex(TypeIndex) ? TypeRows[TypeIndex] : nullptr;
}

int32 UHOLSourceSubsystem::AddSource(const FName TypeName, const FVector& Location, const int32 Amount, const uint8 OwnerSlot)
{
	if (!bStarted || !bAuthority)
	{
		UE_LOG(LogHOLMatch, Warning, TEXT("Source %s not added: %s"), *TypeName.ToString(), bStarted ? TEXT("a client") : TEXT("before the match"));
		return INDEX_NONE;
	}
	const int32 TypeIndex = TypeNames.IndexOfByKey(TypeName);
	const FHOLSourceTypeRow* Type = TypeIndex != INDEX_NONE ? TypeRows[TypeIndex] : nullptr;
	if (!Type)
	{
		UE_LOG(LogHOLMatch, Error, TEXT("No source type %s: source not added"), *TypeName.ToString());
		return INDEX_NONE;
	}
	FHOLSource& Source = Sources.AddDefaulted_GetRef();
	Source.TypeIndex = static_cast<uint8>(TypeIndex);
	Source.Resource = Type->Resource;
	Source.Location = Location;
	Source.Radius = Type->Radius;
	Source.Remaining = Amount != INDEX_NONE ? FMath::Max(Amount, 1) : Type->Amount;
	Source.OwnerSlot = OwnerSlot;
	const int32 SourceIndex = Sources.Num() - 1;
	OnSourceAdded(SourceIndex);
	return SourceIndex;
}

int32 UHOLSourceSubsystem::Take(const int32 SourceIndex, const int32 Amount)
{
	if (!bAuthority || !Sources.IsValidIndex(SourceIndex) || !Sources[SourceIndex].IsStanding() || Amount <= 0)
	{
		return 0;
	}
	FHOLSource& Source = Sources[SourceIndex];
	const FHOLSourceTypeRow* Type = GetType(Source.TypeIndex);
	if (Type && Type->Endless)
	{
		// A farm field (IA1.2): it gives what is asked and keeps what it holds.
		return Amount;
	}
	const int32 Taken = FMath::Min(Amount, Source.Remaining);
	Source.Remaining -= Taken;
	if (bReplicate)
	{
		GetOrAddChunk(SourceIndex / AHOLSourceChunk::NumSlots)->SetRemaining(SourceIndex % AHOLSourceChunk::NumSlots, Source.Remaining);
	}
	if (Source.Remaining == 0)
	{
		OnSourceUsedUp(SourceIndex);
	}
	return Taken;
}

bool UHOLSourceSubsystem::RemoveSource(const int32 SourceIndex)
{
	if (!bAuthority || !Sources.IsValidIndex(SourceIndex) || !Sources[SourceIndex].IsStanding())
	{
		return false;
	}
	// Like a used-up source: nothing left, the index kept (gatherers and chunks holding it see it gone).
	FHOLSource& Source = Sources[SourceIndex];
	Source.Remaining = 0;
	if (bReplicate)
	{
		GetOrAddChunk(SourceIndex / AHOLSourceChunk::NumSlots)->SetRemaining(SourceIndex % AHOLSourceChunk::NumSlots, 0);
	}
	OnSourceUsedUp(SourceIndex);
	return true;
}

bool UHOLSourceSubsystem::IsLinked(const int32 SourceIndex) const
{
	const FHOLSourceTypeRow* Type = Sources.IsValidIndex(SourceIndex) ? GetType(Sources[SourceIndex].TypeIndex) : nullptr;
	return Type && Type->Linked;
}

int32 UHOLSourceSubsystem::FindLinkedSourceAt(const FVector& Location, const float Tolerance) const
{
	// IA1.2 F7: a farm field stands at its farm's centre (AHOLBuilding::UpdateLinkedSource); a client holds its place as
	// floats (FHOLNetSource), hence the tolerance.
	const double ToleranceSquared = FMath::Square(static_cast<double>(Tolerance));
	for (int32 Index = 0; Index < Sources.Num(); ++Index)
	{
		const FHOLSource& Source = Sources[Index];
		if (Source.IsStanding() && IsLinked(Index) && FVector::DistSquared2D(Source.Location, Location) <= ToleranceSquared)
		{
			return Index;
		}
	}
	return INDEX_NONE;
}

void UHOLSourceSubsystem::OnSourceAdded(const int32 SourceIndex)
{
	const FHOLSource& Source = Sources[SourceIndex];
	if (!Source.IsStanding())
	{
		return;
	}
	++NumStanding;
	// A linked source (a farm field, IA1.2) has no shape and no hole in the navmesh: its building stands for it.
	if (Field && !IsLinked(SourceIndex))
	{
		Field->ShowSource(SourceIndex, Source.TypeIndex, *TypeRows[Source.TypeIndex], Source.Location);
	}
	if (bReplicate)
	{
		GetOrAddChunk(SourceIndex / AHOLSourceChunk::NumSlots)->SetSource(SourceIndex % AHOLSourceChunk::NumSlots, Source.TypeIndex,
			Source.Location, Source.Remaining);
	}
}

void UHOLSourceSubsystem::OnSourceUsedUp(const int32 SourceIndex)
{
	--NumStanding;
	if (Field && !IsLinked(SourceIndex))
	{
		Field->HideSource(SourceIndex);
	}
	UE_LOG(LogHOLMatch, Log, TEXT("Sources: source %d used up (%s)"), SourceIndex, bAuthority ? TEXT("server") : TEXT("client"));
}

AHOLSourceChunk* UHOLSourceSubsystem::GetOrAddChunk(const int32 ChunkIndex)
{
	while (Chunks.Num() <= ChunkIndex)
	{
		FActorSpawnParameters Parameters;
		Parameters.Name = *FString::Printf(TEXT("HOLSourceChunk_%d"), Chunks.Num());
		AHOLSourceChunk* Chunk = GetWorld()->SpawnActor<AHOLSourceChunk>(Parameters);
		Chunk->InitSlots(Chunks.Num());
		Chunks.Add(Chunk);
	}
	return Chunks[ChunkIndex];
}

void UHOLSourceSubsystem::RecordDeposit(const uint8 OwnerPlayer, const EHOLResource Resource, const int32 Amount)
{
	Deposited.FindOrAdd(OwnerPlayer)[Resource] += Amount;
}

int32 UHOLSourceSubsystem::FindNearestStanding(const FVector& Location, const TOptional<EHOLResource> Resource) const
{
	int32 Nearest = INDEX_NONE;
	double NearestDistanceSquared = TNumericLimits<double>::Max();
	for (int32 Index = 0; Index < Sources.Num(); ++Index)
	{
		if (!Sources[Index].IsStanding() || (Resource.IsSet() && Sources[Index].Resource != Resource.GetValue()))
		{
			continue;
		}
		const double DistanceSquared = FVector::DistSquared2D(Location, Sources[Index].Location);
		if (DistanceSquared < NearestDistanceSquared)
		{
			Nearest = Index;
			NearestDistanceSquared = DistanceSquared;
		}
	}
	return Nearest;
}

void UHOLSourceSubsystem::AddChunk(AHOLSourceChunk* Chunk)
{
	if (Chunk)
	{
		ReadChunk(*Chunk);
	}
}

void UHOLSourceSubsystem::RemoveChunk(AHOLSourceChunk* Chunk)
{
	// The sources stay: a chunk leaves only with the match.
}

void UHOLSourceSubsystem::ReadChunk(const AHOLSourceChunk& Chunk)
{
	if (!bStarted || bAuthority || !Chunk.HasAllSlots() || Chunk.GetChunkIndex() == INDEX_NONE)
	{
		return;
	}
	const int32 First = Chunk.GetChunkIndex() * AHOLSourceChunk::NumSlots;
	if (Sources.Num() < First + AHOLSourceChunk::NumSlots)
	{
		Sources.SetNum(First + AHOLSourceChunk::NumSlots);
	}
	for (int32 Slot = 0; Slot < AHOLSourceChunk::NumSlots; ++Slot)
	{
		const FHOLNetSource& Received = Chunk.GetSource(Slot);
		const FHOLSourceTypeRow* Type = GetType(Received.TypeIndex);
		if (!Type)
		{
			continue;
		}
		const int32 SourceIndex = First + Slot;
		FHOLSource& Source = Sources[SourceIndex];
		const bool bNew = Source.TypeIndex == FHOLSource::NoType;
		const bool bWasStanding = Source.IsStanding();
		if (bNew)
		{
			Source.TypeIndex = Received.TypeIndex;
			Source.Resource = Type->Resource;
			Source.Location = FVector(Received.Location);
			Source.Radius = Type->Radius;
		}
		Source.Remaining = Chunk.GetRemaining(Slot);
		if (bNew)
		{
			// A source used up before the client joined never enters its map.
			OnSourceAdded(SourceIndex);
		}
		else if (bWasStanding && !Source.IsStanding())
		{
			OnSourceUsedUp(SourceIndex);
		}
	}
}

FString UHOLSourceSubsystem::MakeReport() const
{
	TArray<int32> StandingPerType;
	TArray<int32> LeftPerType;
	StandingPerType.SetNumZeroed(TypeNames.Num());
	LeftPerType.SetNumZeroed(TypeNames.Num());
	int32 NumKnown = 0;
	for (const FHOLSource& Source : Sources)
	{
		if (Source.TypeIndex == FHOLSource::NoType || !TypeNames.IsValidIndex(Source.TypeIndex))
		{
			continue;
		}
		++NumKnown;
		if (Source.IsStanding())
		{
			++StandingPerType[Source.TypeIndex];
			LeftPerType[Source.TypeIndex] += Source.Remaining;
		}
	}
	TArray<FString> PerType;
	for (int32 TypeIndex = 0; TypeIndex < TypeNames.Num(); ++TypeIndex)
	{
		if (StandingPerType[TypeIndex] > 0)
		{
			const bool bEndless = TypeRows[TypeIndex] && TypeRows[TypeIndex]->Endless;
			PerType.Add(bEndless
				? FString::Printf(TEXT("%s %d: endless"), *TypeNames[TypeIndex].ToString(), StandingPerType[TypeIndex])
				: FString::Printf(TEXT("%s %d: %d left"), *TypeNames[TypeIndex].ToString(), StandingPerType[TypeIndex],
					LeftPerType[TypeIndex]));
		}
	}
	return FString::Printf(TEXT("Sources (%s): %d standing of %d (%s); shown %d, navmesh %d"),
		bAuthority ? TEXT("server") : TEXT("client"), NumStanding, NumKnown, *FString::Join(PerType, TEXT(", ")),
		Field ? Field->GetNumShown() : 0, Field ? Field->GetNumNavCylinders() : 0);
}
