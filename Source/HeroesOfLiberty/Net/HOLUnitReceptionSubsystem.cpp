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
#include "MassRepresentationFragments.h"
#include "MassRepresentationSubsystem.h"
#include "GameFramework/GameStateBase.h"
#include "Match/HOLPlayerState.h"
#include "Net/HOLNetUnitDisplay.h"
#include "Net/HOLNetUnitStateNetSerializer.h"
#include "Net/HOLUnitReplicationSettings.h"
#include "Net/HOLUnitReplicator.h"
#include "Presentation/HOLArrowSubsystem.h"
#include "Presentation/HOLCorpse.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

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
	UE::HeroesOfLiberty::Net::ResetUnitStateBitProbe();
}

FString UHOLUnitReceptionSubsystem::MakeBitProbeReport(const double Seconds)
{
	const UE::HeroesOfLiberty::Net::FHOLUnitStateBitProbe& Probe = UE::HeroesOfLiberty::Net::GetUnitStateBitProbe();
	const double PerSecond = 1.0 / FMath::Max(Seconds, 1.0e-3);
	TArray<int32> Gaps;
	for (int32 Bucket = 0; Bucket < Probe.NumBuckets; ++Bucket)
	{
		if (Probe.GapHistogram[Bucket] > 0)
		{
			Gaps.Add(Bucket);
		}
	}
	Gaps.Sort([&Probe](const int32 A, const int32 B) { return Probe.GapHistogram[A] > Probe.GapHistogram[B]; });
	FString Frequent;
	for (int32 Index = 0; Index < FMath::Min(Gaps.Num(), 8); ++Index)
	{
		Frequent += FString::Printf(TEXT(" %d bits x%d"), Gaps[Index] * Probe.BucketBits, Probe.GapHistogram[Gaps[Index]]);
	}
	return FString::Printf(TEXT("Net (bits): %.0f states per second, %.0f right after the previous one; %.0f gaps per second of %.1f bits on average (median %d, 10 %% %d, 90 %% %d; most frequent:%s); %.0f packet starts per second, first state at %.1f bits; %.0f larger gaps per second"),
		Probe.NumStates * PerSecond, Probe.NumAdjacent * PerSecond,
		Probe.NumGaps * PerSecond, Probe.NumGaps > 0 ? double(Probe.GapBits) / Probe.NumGaps : 0.0,
		Probe.GapPercentile(0.5), Probe.GapPercentile(0.1), Probe.GapPercentile(0.9), *Frequent,
		Probe.NumPacketStarts * PerSecond, Probe.NumPacketStarts > 0 ? double(Probe.PacketStartBits) / Probe.NumPacketStarts : 0.0,
		Probe.NumOther * PerSecond);
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
			UE_LOG(LogHOLNet, Display, TEXT("%s"), *MakeBitProbeReport(Now - Stats.StartSeconds));
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
			const FHOLNetUnitIdentity& Identity = Chunk->GetIdentity(Index);
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
		// The bodies this client keeps (J7c, bodies kept 30 s): read off the dying display entities (their received activity)
		// before they go, laid once gone.
		TArray<FHOLCorpseSeed> CorpseSeeds;
		HOLCorpse::CollectSeeds(EntityManager, Departures, GetWorld()->GetTimeSeconds(), CorpseSeeds);
		EntityManager.BatchDestroyEntities(Departures);
		if (CorpseSeeds.Num() > 0)
		{
			if (UHOLUnitSubsystem* Units = GetWorld()->GetSubsystem<UHOLUnitSubsystem>())
			{
				HOLCorpse::Lay(EntityManager, *Units, CorpseSeeds);
			}
		}
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
			Display.GroundZ = GetGroundHeight(Slot.State.GetPosition(), 0.0);
			Display.Yaw = Display.Velocity.SizeSquared2D() > 1.f ? FMath::RadiansToDegrees(FMath::Atan2(Display.Velocity.Y, Display.Velocity.X)) : 0.f;

			const FTransform Drawn(FRotator(0.f, Display.Yaw, 0.f), Display.GetDisplayedPosition(ClockStep, Now, StepSeconds));
			EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity).SetTransform(Drawn);
			EntityManager.GetFragmentDataChecked<FHOLPreviousTransformFragment>(Entity).Transform = Drawn;

			FHOLUnitStateFragment& UnitState = EntityManager.GetFragmentDataChecked<FHOLUnitStateFragment>(Entity);
			UnitState.UnitId = Slot.UnitId;
			UnitState.OwnerPlayer = Slot.OwnerPlayer;
			UnitState.Activity = Slot.State.Activity;
			UnitState.GatherResource = Slot.State.GatherResource;
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
	// Its health went down (J7c.2b2): measured against the arrow this client draws on it, if any.
	const bool bHealthDown = !Slot.State.bFullHealth && (Display.State.bFullHealth || Slot.State.Health < Display.State.Health);
	if (bHealthDown)
	{
		if (UHOLArrowSubsystem* ArrowDisplay = GetWorld()->GetSubsystem<UHOLArrowSubsystem>())
		{
			ArrowDisplay->NoteHealthDrop(Slot.Entity);
		}
	}
	Display.State = Slot.State;
	Display.StateStep = HOLUnitReplication::UnwrapStep(Slot.State.Step, FMath::RoundToInt64(ClockStep));
	Display.Velocity = Slot.State.GetVelocity();
	Display.GroundZ = GetGroundHeight(Slot.State.GetPosition(), Display.GroundZ);
	Display.Correction = FHOLDisplayCorrection();
	const FVector Predicted = Display.GetDisplayedPosition(ClockStep, Now, StepSeconds);
	Display.Correction.Start(Displayed, Predicted, Now);
	FHOLUnitStateFragment& UnitState = EntityManager.GetFragmentDataChecked<FHOLUnitStateFragment>(Slot.Entity);
	UnitState.Activity = Slot.State.Activity;
	UnitState.GatherResource = Slot.State.GatherResource;

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

double UHOLUnitReceptionSubsystem::GetGroundHeight(const FVector& Position, const double Fallback) const
{
	// The lowest static surface under the unit: a unit walks under a tree's crown (the grove of Plain), never on it. An
	// object type query reports every surface along the ray, not only the first one.
	TArray<FHitResult> Hits;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(HOLUnitGround), false);
	GetWorld()->LineTraceMultiByObjectType(Hits, FVector(Position.X, Position.Y, GroundTraceHalfHeight),
		FVector(Position.X, Position.Y, -GroundTraceHalfHeight), FCollisionObjectQueryParams(ECC_WorldStatic), Params);
	double Height = TNumericLimits<double>::Max();
	for (const FHitResult& Hit : Hits)
	{
		Height = FMath::Min(Height, Hit.ImpactPoint.Z);
	}
	return Hits.Num() > 0 ? Height : Fallback;
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

bool UHOLUnitReceptionSubsystem::MakeView(const FSlot& Slot, FHOLUnitView& OutView) const
{
	const UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	const FTransformFragment* Transform = EntitySubsystem && Slot.Entity.IsSet()
		? EntitySubsystem->GetEntityManager().GetFragmentDataPtr<FTransformFragment>(Slot.Entity) : nullptr;
	if (!Transform)
	{
		return false;
	}
	OutView = FHOLUnitView();
	OutView.UnitId = Slot.UnitId;
	OutView.OwnerPlayer = Slot.OwnerPlayer;
	OutView.TypeName = TypeNames.IsValidIndex(Slot.TypeIndex) ? TypeNames[Slot.TypeIndex] : NAME_None;
	OutView.Location = Transform->GetTransform().GetLocation();
	OutView.Activity = Slot.State.Activity;
	OutView.GatherResource = Slot.State.GatherResource;
	OutView.bCarrying = Slot.State.bCarrying;
	OutView.LoadResource = Slot.State.LoadResource;
	// J7b.4: health (exact, J7-Q5 revised; its maximum from the type below), and the target's identifier from its slot.
	OutView.bFullHealth = Slot.State.bFullHealth;
	OutView.Health = Slot.State.bFullHealth ? 0 : Slot.State.Health;
	OutView.TargetUnitId = GetTargetUnitId(Slot.State);
	// The amount only for the watched unit (D303): the server sends it at every change, sooner than the state.
	if (Slot.UnitId == WatchedUnitId)
	{
		OutView.bCarrying = WatchedAmount > 0;
		OutView.LoadResource = WatchedResource;
		OutView.LoadAmount = WatchedAmount;
		OutView.bLoadAmountKnown = true;
	}
	const UHOLUnitSubsystem* Units = GetWorld()->GetSubsystem<UHOLUnitSubsystem>();
	if (const FHOLUnitTypeRow* Row = Units ? Units->FindTypeRow(OutView.TypeName) : nullptr)
	{
		OutView.Radius = Row->Radius;
		OutView.MeshScale = Row->MeshScale;
		OutView.CarryCapacity = Row->CarryCapacity;
		OutView.bRanged = HOLCombat::IsRangedAttack(Row->AttackRange);
		OutView.bSiege = Row->ArmorCategory == EHOLArmorCategory::Siege;
		// J7b.5: the health received, exact (J7-Q5 revised).
		OutView.MaxHealth = Row->MaxHealth;
		OutView.Health = HOLUnitView::GetReceivedHealth(OutView.bFullHealth, Slot.State.Health, Row->MaxHealth);
		OutView.bHealthExact = true;
	}
	return true;
}

uint32 UHOLUnitReceptionSubsystem::GetTargetUnitId(const FHOLNetUnitState& State) const
{
	return State.HasTargetSlot() && Slots.IsValidIndex(State.TargetSlot) ? Slots[State.TargetSlot].UnitId : 0;
}

FMassEntityHandle UHOLUnitReceptionSubsystem::GetTargetEntity(const FHOLNetUnitState& State) const
{
	return State.HasTargetSlot() && Slots.IsValidIndex(State.TargetSlot) ? Slots[State.TargetSlot].Entity : FMassEntityHandle();
}

void UHOLUnitReceptionSubsystem::GetDisplayedUnits(TArray<FHOLUnitView>& OutUnits) const
{
	FHOLUnitView View;
	for (const FSlot& Slot : Slots)
	{
		if (MakeView(Slot, View))
		{
			OutUnits.Add(View);
		}
	}
}

bool UHOLUnitReceptionSubsystem::GetDisplayedUnit(const uint32 UnitId, FHOLUnitView& OutUnit) const
{
	const FSlot* Slot = UnitId != 0 ? Slots.FindByPredicate([UnitId](const FSlot& Each) { return Each.UnitId == UnitId && Each.Entity.IsSet(); }) : nullptr;
	return Slot && MakeView(*Slot, OutUnit);
}

void UHOLUnitReceptionSubsystem::SetWatchedLoad(const uint32 UnitId, const EHOLResource Resource, const int32 Amount)
{
	WatchedUnitId = UnitId;
	WatchedResource = Resource;
	WatchedAmount = Amount;
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

FString UHOLUnitReceptionSubsystem::DescribeLooks() const
{
	const UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem)
	{
		return TEXT("no entities");
	}
	TMap<int32, int32> ColorIndices;
	if (const AGameStateBase* GameState = GetWorld()->GetGameState())
	{
		for (const APlayerState* PlayerState : GameState->PlayerArray)
		{
			if (const AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState))
			{
				ColorIndices.Add(HOLPlayerState->GetPlayerIndex(), HOLPlayerState->GetColorIndex());
			}
		}
	}
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	TMap<FString, int32> Counts;
	for (const FSlot& Slot : Slots)
	{
		if (!Slot.Entity.IsSet())
		{
			continue;
		}
		const FName TypeName = TypeNames.IsValidIndex(Slot.TypeIndex) ? TypeNames[Slot.TypeIndex] : NAME_None;
		FString Mesh = TEXT("no mesh");
		const FMassRepresentationFragment* Representation = EntityManager.GetFragmentDataPtr<FMassRepresentationFragment>(Slot.Entity);
		const FMassRepresentationSubsystemSharedFragment* Shared = EntityManager.GetSharedFragmentDataPtr<FMassRepresentationSubsystemSharedFragment>(Slot.Entity);
		if (Representation && Shared && Shared->RepresentationSubsystem && Representation->SkinnedMeshDescHandle.IsValid())
		{
			FMassInstancedSkinnedMeshInfoArrayView Infos = Shared->RepresentationSubsystem->GetMutableInstancedSkinnedMeshInfos();
			const int32 Index = Representation->SkinnedMeshDescHandle.ToIndex();
			if (Infos.IsValidIndex(Index) && !Infos[Index].GetDesc().Meshes.IsEmpty() && Infos[Index].GetDesc().Meshes[0].Asset)
			{
				Mesh = Infos[Index].GetDesc().Meshes[0].Asset->GetName();
			}
		}
		const TCHAR* Drawn = Representation && Representation->CurrentRepresentation == EMassRepresentationType::SkinnedMeshInstance ? TEXT("skinned")
			: Representation && Representation->CurrentRepresentation == EMassRepresentationType::StaticMeshInstance ? TEXT("static") : TEXT("none");
		const int32* Color = ColorIndices.Find(Slot.OwnerPlayer);
		++Counts.FindOrAdd(FString::Printf(TEXT("%s of slot %d %s %s colour %d"), *TypeName.ToString(), Slot.OwnerPlayer, Drawn, *Mesh,
			Color ? *Color : static_cast<int32>(Slot.OwnerPlayer)));
	}
	Counts.KeySort(TLess<FString>());
	TArray<FString> Lines;
	for (const TPair<FString, int32>& Count : Counts)
	{
		Lines.Add(FString::Printf(TEXT("%s x%d"), *Count.Key, Count.Value));
	}
	return FString::Join(Lines, TEXT(", "));
}
