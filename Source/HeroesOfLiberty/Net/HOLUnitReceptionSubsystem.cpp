// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLUnitReceptionSubsystem.h"

#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/PlatformTime.h"
#include "HeroesOfLibertyLog.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassEntityTemplate.h"
#include "MassEntityUtils.h"
#include "Net/HOLNetUnitDisplay.h"
#include "Net/HOLNetUnitStateNetSerializer.h"
#include "Net/HOLUnitReplicationSettings.h"
#include "Net/HOLUnitReplicator.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"

bool UHOLUnitReceptionSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLUnitReceptionSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// A client only: the server and the solo game display the simulated units themselves (D272).
	bActive = InWorld.GetNetMode() == NM_Client;
	if (!bActive)
	{
		return;
	}
	StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
	Clock.Reset(StepSeconds);
	if (const UHOLUnitSubsystem* Units = InWorld.GetSubsystem<UHOLUnitSubsystem>())
	{
		Units->GetTypeNames(TypeNames);
	}
	ResetStats();
}

void UHOLUnitReceptionSubsystem::Deinitialize()
{
	bActive = false;
	Slots.Reset();
	Chunks.Reset();
	NumDisplayed = 0;
	Super::Deinitialize();
}

TStatId UHOLUnitReceptionSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UHOLUnitReceptionSubsystem, STATGROUP_Tickables);
}

void UHOLUnitReceptionSubsystem::ResetStats()
{
	Stats = FStats();
	Stats.StartSeconds = FPlatformTime::Seconds();
	const UNetDriver* NetDriver = GetWorld() ? GetWorld()->GetNetDriver() : nullptr;
	const UNetConnection* Connection = NetDriver ? NetDriver->ServerConnection.Get() : nullptr;
	Stats.InTotalBytes = Connection ? Connection->InTotalBytes : 0;
	Stats.DeserializedStates = UE::HeroesOfLiberty::Net::GetNumDeserializedUnitStates();
}

void UHOLUnitReceptionSubsystem::AddChunk(AHOLUnitChunk* Chunk)
{
	const int32 ChunkIndex = Chunk->GetChunkIndex();
	if (ChunkIndex < 0)
	{
		UE_LOG(LogHOLNet, Error, TEXT("Unit chunk %s arrived without its index"), *Chunk->GetName());
		return;
	}
	if (Chunks.Num() <= ChunkIndex)
	{
		Chunks.SetNum(ChunkIndex + 1);
	}
	Chunks[ChunkIndex] = Chunk;
}

void UHOLUnitReceptionSubsystem::RemoveChunk(const AHOLUnitChunk* Chunk)
{
	// Its units stay displayed with their last state until the slots are read again (a chunk leaves with the match).
	if (Chunks.IsValidIndex(Chunk->GetChunkIndex()) && Chunks[Chunk->GetChunkIndex()] == Chunk)
	{
		Chunks[Chunk->GetChunkIndex()] = nullptr;
	}
}

void UHOLUnitReceptionSubsystem::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	const double Now = FPlatformTime::Seconds();
	if (Stats.FrameSeconds.Num() < MaxFrames)
	{
		Stats.FrameSeconds.Add(DeltaTime);
	}
	const float ReportInterval = UE::HeroesOfLiberty::Net::ReportInterval;
	if (ReportInterval > 0.f && Now - LastReportSeconds >= ReportInterval)
	{
		LastReportSeconds = Now;
		if (Replicator.IsValid())
		{
			UE_LOG(LogHOLNet, Display, TEXT("%s"), *MakeReport());
		}
		ResetStats();
	}

	if (!Replicator.IsValid())
	{
		for (TActorIterator<AHOLUnitReplicator> It(GetWorld()); It; ++It)
		{
			Replicator = *It;
		}
		if (!Replicator.IsValid())
		{
			return;
		}
	}
	ReadClock(Now);
	if (Clock.IsValid())
	{
		ReadSlots(Now);
		if (UE::HeroesOfLiberty::Net::SampleEvery > 0 && Now - LastSampleSeconds >= 0.25)
		{
			LastSampleSeconds = Now;
			LogSample(Now);
		}
	}
}

void UHOLUnitReceptionSubsystem::LogSample(const double Now) const
{
	const FMassEntityManager& EntityManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetEntityManager();
	const int32 SampleEvery = UE::HeroesOfLiberty::Net::SampleEvery;
	const double ClockStep = Clock.GetStep(Now);
	FString Sample;
	for (const FSlot& Slot : Slots)
	{
		if (Slot.UnitId != 0 && Slot.UnitId % SampleEvery == 0 && Slot.Entity.IsSet())
		{
			// The position UHOLNetUnitDisplayProcessor draws at this instant.
			const FHOLNetUnitDisplayFragment& Display = EntityManager.GetFragmentDataChecked<FHOLNetUnitDisplayFragment>(Slot.Entity);
			const FVector Position = Display.GetDisplayedPosition(ClockStep, Now, StepSeconds);
			Sample += FString::Printf(TEXT(" %u:%.0f,%.0f"), Slot.UnitId, Position.X, Position.Y);
		}
	}
	if (!Sample.IsEmpty())
	{
		UE_LOG(LogHOLNet, Display, TEXT("NetSample client t=%.4f clock=%.2f%s"), Now, ClockStep, *Sample);
	}
}

void UHOLUnitReceptionSubsystem::ReadClock(const double Now)
{
	const int32 ServerStep = Replicator->GetServerStep();
	if (ServerStep == LastServerStep)
	{
		return;
	}
	LastServerStep = ServerStep;
	// The step left the server half a round trip ago.
	const UNetDriver* NetDriver = GetWorld()->GetNetDriver();
	const UNetConnection* Connection = NetDriver ? NetDriver->ServerConnection.Get() : nullptr;
	const double OneWaySeconds = Connection ? 0.5 * Connection->AvgLag : 0.0;
	Clock.AddSample(ServerStep, Now, OneWaySeconds);
}

void UHOLUnitReceptionSubsystem::ReadSlots(const double Now)
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (EntitySubsystem == nullptr)
	{
		return;
	}
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();

	TArray<int32> Arrivals;
	TArray<FMassEntityHandle> Departures;
	for (const TWeakObjectPtr<AHOLUnitChunk>& ChunkPtr : Chunks)
	{
		const AHOLUnitChunk* Chunk = ChunkPtr.Get();
		if (Chunk == nullptr || !Chunk->HasAllSlots())
		{
			continue;
		}
		const int32 FirstSlot = Chunk->GetChunkIndex() * AHOLUnitChunk::NumSlots;
		if (Slots.Num() < FirstSlot + AHOLUnitChunk::NumSlots)
		{
			Slots.SetNum(FirstSlot + AHOLUnitChunk::NumSlots);
		}
		for (int32 Index = 0; Index < AHOLUnitChunk::NumSlots; ++Index)
		{
			FSlot& Slot = Slots[FirstSlot + Index];
			const FHOLNetUnitIdentityItem& Identity = Chunk->GetIdentity(Index);
			const FHOLNetUnitState& State = Chunk->GetState(Index);
			if (Identity.UnitId != Slot.UnitId)
			{
				// The slot changed hands: the old unit leaves, the new one (if any) arrives with the slot's state.
				if (Slot.Entity.IsSet())
				{
					Departures.Add(Slot.Entity);
				}
				Slot = FSlot();
				Slot.UnitId = Identity.UnitId;
				Slot.TypeIndex = Identity.TypeIndex;
				Slot.OwnerPlayer = Identity.OwnerPlayer;
				Slot.State = State;
				if (Slot.UnitId != 0)
				{
					Arrivals.Add(FirstSlot + Index);
				}
				continue;
			}
			if (Slot.UnitId == 0 || !Slot.Entity.IsSet())
			{
				continue;
			}
			if (Identity.OwnerPlayer != Slot.OwnerPlayer)
			{
				// A conversion (D37): the colour follows the owner.
				Slot.OwnerPlayer = Identity.OwnerPlayer;
				EntityManager.GetFragmentDataChecked<FHOLUnitStateFragment>(Slot.Entity).OwnerPlayer = Slot.OwnerPlayer;
			}
			if (!(State == Slot.State))
			{
				Slot.State = State;
				ApplyState(Slot, Now);
			}
		}
	}

	if (Departures.Num() > 0)
	{
		EntityManager.BatchDestroyEntities(Departures);
		NumDisplayed -= Departures.Num();
		Stats.NumDepartures += Departures.Num();
	}
	if (Arrivals.Num() > 0)
	{
		CreateEntities(Arrivals, Now);
	}
}

void UHOLUnitReceptionSubsystem::CreateEntities(const TConstArrayView<int32> SlotIndices, const double Now)
{
	UWorld* World = GetWorld();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	if (EntitySubsystem == nullptr || Units == nullptr)
	{
		return;
	}
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	const double ClockStep = Clock.GetStep(Now);

	TMap<uint8, TArray<int32>> SlotsByType;
	for (const int32 SlotIndex : SlotIndices)
	{
		SlotsByType.FindOrAdd(Slots[SlotIndex].TypeIndex).Add(SlotIndex);
	}
	for (const TPair<uint8, TArray<int32>>& TypeSlots : SlotsByType)
	{
		const FMassEntityTemplate* Template = TypeNames.IsValidIndex(TypeSlots.Key)
			? Units->FindTemplate(TypeNames[TypeSlots.Key], EHOLUnitTemplateKind::Display) : nullptr;
		if (Template == nullptr)
		{
			UE_LOG(LogHOLNet, Error, TEXT("Unit type %d unknown to this client (%d types): %d units not displayed"),
				TypeSlots.Key, TypeNames.Num(), TypeSlots.Value.Num());
			continue;
		}

		TArray<FMassEntityHandle> Created;
		{
			TSharedRef<FMassEntityManager::FEntityCreationContext> CreationContext =
				EntityManager.BatchCreateEntities(Template->GetArchetype(), Template->GetSharedFragmentValues(), TypeSlots.Value.Num(), Created);
			TArray<FMassArchetypeEntityCollection> Collections;
			UE::Mass::Utils::CreateEntityCollections(EntityManager, Created, FMassArchetypeEntityCollection::NoDuplicates, Collections);
			EntityManager.BatchSetEntityFragmentValues(Collections, Template->GetInitialFragmentValues());
		}

		for (int32 Index = 0; Index < Created.Num(); ++Index)
		{
			FSlot& Slot = Slots[TypeSlots.Value[Index]];
			const FMassEntityHandle Entity = Created[Index];
			Slot.Entity = Entity;

			FHOLNetUnitDisplayFragment& Display = EntityManager.GetFragmentDataChecked<FHOLNetUnitDisplayFragment>(Entity);
			Display.State = Slot.State;
			Display.StateStep = HOLUnitReplication::UnwrapStep(Slot.State.Step, FMath::RoundToInt64(ClockStep));
			Display.Velocity = Slot.State.GetVelocity();
			Display.Yaw = Display.Velocity.SizeSquared2D() > 1.f ? FMath::RadiansToDegrees(FMath::Atan2(Display.Velocity.Y, Display.Velocity.X)) : 0.f;

			const FTransform Drawn(FRotator(0.f, Display.Yaw, 0.f), Display.GetDisplayedPosition(ClockStep, Now, StepSeconds));
			EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity).SetTransform(Drawn);
			EntityManager.GetFragmentDataChecked<FHOLPreviousTransformFragment>(Entity).Transform = Drawn;

			FHOLUnitStateFragment& UnitState = EntityManager.GetFragmentDataChecked<FHOLUnitStateFragment>(Entity);
			UnitState.UnitId = Slot.UnitId;
			UnitState.OwnerPlayer = Slot.OwnerPlayer;
			UnitState.Activity = Slot.State.Activity;
		}
		NumDisplayed += Created.Num();
		Stats.NumArrivals += Created.Num();
	}
}

void UHOLUnitReceptionSubsystem::ApplyState(FSlot& Slot, const double Now)
{
	FMassEntityManager& EntityManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
	FHOLNetUnitDisplayFragment& Display = EntityManager.GetFragmentDataChecked<FHOLNetUnitDisplayFragment>(Slot.Entity);
	const double ClockStep = Clock.GetStep(Now);

	// Where the unit is drawn now, from the previous state and its correction, against where the new state puts it.
	const FVector Displayed = Display.GetDisplayedPosition(ClockStep, Now, StepSeconds);
	Display.State = Slot.State;
	Display.StateStep = HOLUnitReplication::UnwrapStep(Slot.State.Step, FMath::RoundToInt64(ClockStep));
	Display.Velocity = Slot.State.GetVelocity();
	Display.Correction = FHOLDisplayCorrection();
	const FVector Predicted = Display.GetDisplayedPosition(ClockStep, Now, StepSeconds);
	Display.Correction.Start(Displayed, Predicted, Now);
	EntityManager.GetFragmentDataChecked<FHOLUnitStateFragment>(Slot.Entity).Activity = Slot.State.Activity;

	const double Gap = FVector::Dist(Displayed, Predicted);
	++Stats.NumStates;
	if (Gap > FHOLDisplayCorrection::SnapDistance)
	{
		++Stats.NumSnaps;
	}
	else
	{
		++Stats.NumCorrections;
		Stats.TotalCorrectionCm += Gap;
		Stats.MaxCorrectionCm = FMath::Max(Stats.MaxCorrectionCm, Gap);
	}
}

FString UHOLUnitReceptionSubsystem::MakeReport() const
{
	const double Seconds = FMath::Max(FPlatformTime::Seconds() - Stats.StartSeconds, 1.0e-3);
	const double ClockStep = Clock.IsValid() ? Clock.GetStep(FPlatformTime::Seconds()) : 0.0;

	// Received bytes averaged over the report's span (InBytesPerSecond is the last second only).
	const UNetDriver* NetDriver = GetWorld()->GetNetDriver();
	const UNetConnection* Connection = NetDriver ? NetDriver->ServerConnection.Get() : nullptr;
	const double InBytesPerSecond = Connection ? (Connection->InTotalBytes - Stats.InTotalBytes) / Seconds : -1.0;
	// States really read off the wire (a state rewritten twice between two updates is read once).
	const double WireStates = (UE::HeroesOfLiberty::Net::GetNumDeserializedUnitStates() - Stats.DeserializedStates) / Seconds;

	// Frame rate: the mean, and the 1 % low (mean of the slowest 1 % of the frames), as in the stress test's report.
	double TotalFrameSeconds = 0.0;
	for (const float FrameSeconds : Stats.FrameSeconds)
	{
		TotalFrameSeconds += FrameSeconds;
	}
	TArray<float> SlowestFirst = Stats.FrameSeconds;
	SlowestFirst.Sort(TGreater<float>());
	const int32 NumSlowest = FMath::Max(1, SlowestFirst.Num() / 100);
	double SlowestSeconds = 0.0;
	for (int32 Index = 0; Index < NumSlowest && Index < SlowestFirst.Num(); ++Index)
	{
		SlowestSeconds += SlowestFirst[Index];
	}
	const double MeanFps = TotalFrameSeconds > 0.0 ? Stats.FrameSeconds.Num() / TotalFrameSeconds : 0.0;
	const double LowFps = SlowestSeconds > 0.0 ? NumSlowest / SlowestSeconds : 0.0;

	return FString::Printf(TEXT("Net (display): %d units displayed (+%d -%d), %.0f states per second (%.0f read off the wire), correction mean %.1f cm max %.1f cm, %d snaps; clock %.1f steps ahead of the last step received; in %.0f B/s; %.1f fps (1%% low %.1f)"),
		NumDisplayed, Stats.NumArrivals, Stats.NumDepartures, Stats.NumStates / Seconds, WireStates,
		Stats.NumCorrections > 0 ? Stats.TotalCorrectionCm / Stats.NumCorrections : 0.0, Stats.MaxCorrectionCm, Stats.NumSnaps,
		ClockStep - LastServerStep, InBytesPerSecond, MeanFps, LowFps);
}

FString UHOLUnitReceptionSubsystem::DescribeDisplayed() const
{
	TMap<FString, int32> Counts;
	for (const FSlot& Slot : Slots)
	{
		if (Slot.Entity.IsSet())
		{
			const FName TypeName = TypeNames.IsValidIndex(Slot.TypeIndex) ? TypeNames[Slot.TypeIndex] : NAME_None;
			++Counts.FindOrAdd(FString::Printf(TEXT("%s of slot %d"), *TypeName.ToString(), Slot.OwnerPlayer));
		}
	}
	Counts.KeySort(TLess<FString>());
	TArray<FString> Lines;
	for (const TPair<FString, int32>& Count : Counts)
	{
		Lines.Add(FString::Printf(TEXT("%s x%d"), *Count.Key, Count.Value));
	}
	return FString::Printf(TEXT("%d displayed: %s"), NumDisplayed, *FString::Join(Lines, TEXT(", ")));
}
