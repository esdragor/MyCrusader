// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLUnitReplicationSubsystem.h"

#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"
#include "HeroesOfLibertyLog.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "Net/HOLNetUnitStateNetSerializer.h"
#include "Net/HOLUnitReceptionSubsystem.h"
#include "Net/HOLUnitReplicationSettings.h"
#include "Net/HOLUnitReplicator.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

namespace UE::HeroesOfLiberty::Net
{
	float ErrorThresholdOverride = 0.f;
	FAutoConsoleVariableRef CVarErrorThreshold(TEXT("hol.Net.ErrorThreshold"), ErrorThresholdOverride,
		TEXT("Prediction error (cm) past which the server sends a unit again, 0 = UHOLUnitReplicationSettings value"));

	float UpdateFrequencyOverride = 0.f;
	FAutoConsoleVariableRef CVarUpdateFrequency(TEXT("hol.Net.UpdateFrequency"), UpdateFrequencyOverride,
		TEXT("Network updates per second of the shared unit actor and its chunk actors, 0 = UHOLUnitReplicationSettings value"));

	int32 RefreshStepsOverride = 0;
	FAutoConsoleVariableRef CVarRefreshSteps(TEXT("hol.Net.RefreshSteps"), RefreshStepsOverride,
		TEXT("Cycle of the slow refresh in steps (S5), 0 = UHOLUnitReplicationSettings value"));

	float ReportInterval = 0.f;
	FAutoConsoleVariableRef CVarReportInterval(TEXT("hol.Net.ReportInterval"), ReportInterval,
		TEXT("Seconds between two unit replication reports (server: sends; client: reception and display), then reset; 0 = none"));

	int32 SampleEvery = 0;
	FAutoConsoleVariableRef CVarSample(TEXT("hol.Net.Sample"), SampleEvery,
		TEXT("S5 displayed error: logs the positions of the units whose identifier is a multiple of this (server: true ones at each step; client: displayed ones 4 times a second), 0 = none"));

	/** 1: the velocity sent is the step's whole move, the separation's pushes included (D294); 0: the walk's velocity. */
	int32 StepVelocity = 1;
	FAutoConsoleVariableRef CVarStepVelocity(TEXT("hol.Net.StepVelocity"), StepVelocity,
		TEXT("1: the velocity sent with a unit's state is its whole move over the last step, the separation's pushes included (D294); 0: the velocity it walks at"));

	FAutoConsoleCommandWithWorld CommandReport(TEXT("hol.Net.Report"),
		TEXT("Unit replication report: on the server what it sent since the last report, on a client what it received"),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (UHOLUnitReplicationSubsystem* Replication = World ? World->GetSubsystem<UHOLUnitReplicationSubsystem>() : nullptr)
			{
				UE_LOG(LogHOLNet, Display, TEXT("%s"), *Replication->MakeReport());
				Replication->ResetStats();
			}
			UHOLUnitReceptionSubsystem* Reception = World ? World->GetSubsystem<UHOLUnitReceptionSubsystem>() : nullptr;
			if (Reception && Reception->IsTickable())
			{
				UE_LOG(LogHOLNet, Display, TEXT("%s"), *Reception->MakeReport());
				Reception->ResetStats();
			}
		}));

	/** S5 (D273): S2 starts once the clients are in the match, so the measure never runs with a client still loading. */
	FAutoConsoleCommand CommandWhenClients(TEXT("hol.Net.WhenClients"),
		TEXT("hol.Net.WhenClients <NumClients> <Command...>: runs a console command once that many clients have joined (server)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				UE_LOG(LogHOLNet, Warning, TEXT("Usage: hol.Net.WhenClients <NumClients> <Command...>"));
				return;
			}
			const int32 NumClients = FCString::Atoi(*Args[0]);
			const FString Command = FString::Join(TArrayView<const FString>(Args).RightChop(1), TEXT(" "));
			FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([NumClients, Command](float)
			{
				UWorld* GameWorld = nullptr;
				for (const FWorldContext& Context : GEngine->GetWorldContexts())
				{
					if (Context.World() && Context.World()->IsGameWorld())
					{
						GameWorld = Context.World();
					}
				}
				const UNetDriver* NetDriver = GameWorld ? GameWorld->GetNetDriver() : nullptr;
				int32 NumJoined = 0;
				for (const UNetConnection* Connection : NetDriver ? NetDriver->ClientConnections : TArray<TObjectPtr<UNetConnection>>())
				{
					NumJoined += Connection && Connection->PlayerController ? 1 : 0;
				}
				if (NumJoined < NumClients)
				{
					return true;
				}
				UE_LOG(LogHOLNet, Display, TEXT("%d clients joined: %s"), NumJoined, *Command);
				GEngine->Exec(GameWorld, *Command);
				return false;
			}), 0.25f);
		}));

	const TCHAR* GetSendReasonName(const int32 Reason)
	{
		switch (static_cast<EHOLUnitSendReason>(Reason))
		{
		case EHOLUnitSendReason::None: return TEXT("birth");
		case EHOLUnitSendReason::Activity: return TEXT("activity");
		case EHOLUnitSendReason::Error: return TEXT("error");
		case EHOLUnitSendReason::Refresh: return TEXT("refresh");
		}
		return TEXT("?");
	}
}

bool UHOLUnitReplicationSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLUnitReplicationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// The server of a networked match only: a client receives the shared actor, the solo game does not replicate (D272).
	const ENetMode NetMode = InWorld.GetNetMode();
	if (NetMode != NM_DedicatedServer && NetMode != NM_ListenServer)
	{
		return;
	}
	UHOLSimulationSubsystem* Simulation = InWorld.GetSubsystem<UHOLSimulationSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = InWorld.GetSubsystem<UMassEntitySubsystem>();
	if (!Simulation || !EntitySubsystem)
	{
		UE_LOG(LogHOLNet, Error, TEXT("Simulation or Mass missing: no unit replication"));
		return;
	}

	FActorSpawnParameters Parameters;
	Parameters.Name = TEXT("HOLUnitReplicator");
	Replicator = InWorld.SpawnActor<AHOLUnitReplicator>(Parameters);

	EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
	UnitQuery = FMassEntityQuery(EntityManager);
	UnitQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
	UnitQuery.AddRequirement<FHOLUnitStateFragment>(EMassFragmentAccess::ReadOnly);
	UnitQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	UnitQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	UnitQuery.AddRequirement<FHOLPreviousTransformFragment>(EMassFragmentAccess::ReadOnly);
	UnitQuery.AddConstSharedRequirement<FHOLUnitTypeConstants>();

	TypeIndices.Reset();
	if (const UHOLUnitSubsystem* Units = InWorld.GetSubsystem<UHOLUnitSubsystem>())
	{
		TArray<FName> TypeNames;
		Units->GetTypeNames(TypeNames);
		for (int32 Index = 0; Index < TypeNames.Num() && Index <= MAX_uint8; ++Index)
		{
			TypeIndices.Add(TypeNames[Index], static_cast<uint8>(Index));
		}
	}

	StepSeconds = Simulation->GetStepSeconds();
	StepEndedHandle = Simulation->GetOnStepEnded().AddUObject(this, &UHOLUnitReplicationSubsystem::OnStepEnded);
	ResetStats();
	LastReportSeconds = Stats.StartSeconds;

	const UHOLUnitReplicationSettings* Settings = GetDefault<UHOLUnitReplicationSettings>();
	UE_LOG(LogHOLNet, Log, TEXT("Unit replication on: threshold %.0f cm, refresh every %d steps, %.0f updates per second, %d unit types, chunks of %d slots"),
		Settings->ErrorThreshold, Settings->RefreshSteps, Settings->NetUpdateFrequency, TypeIndices.Num(), AHOLUnitChunk::NumSlots);
}

void UHOLUnitReplicationSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		if (UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>())
		{
			Simulation->GetOnStepEnded().Remove(StepEndedHandle);
		}
	}
	StepEndedHandle.Reset();
	Records.Reset();
	FreeSlots.Reset();
	EntityManager.Reset();
	Replicator = nullptr;

	Super::Deinitialize();
}

void UHOLUnitReplicationSubsystem::ResetStats()
{
	Stats = FHOLUnitReplicationStats();
	Stats.StartSeconds = FPlatformTime::Seconds();
	Stats.SerializedStates = UE::HeroesOfLiberty::Net::GetNumSerializedUnitStates();
	const UWorld* World = GetWorld();
	if (const UNetDriver* NetDriver = World ? World->GetNetDriver() : nullptr)
	{
		for (const UNetConnection* Connection : NetDriver->ClientConnections)
		{
			if (Connection)
			{
				Stats.OutTotals.Add(Connection, TPair<int32, int32>(Connection->OutTotalBytes, Connection->OutTotalPackets));
			}
		}
	}
}

void UHOLUnitReplicationSubsystem::OnStepEnded(const int64 Step)
{
	if (!Replicator || !EntityManager.IsValid())
	{
		return;
	}
	const double StartSeconds = FPlatformTime::Seconds();

	const UHOLUnitReplicationSettings* Settings = GetDefault<UHOLUnitReplicationSettings>();
	FHOLUnitSendRules Rules = Settings->GetSendRules();
	if (UE::HeroesOfLiberty::Net::ErrorThresholdOverride > 0.f)
	{
		Rules.ErrorThreshold = UE::HeroesOfLiberty::Net::ErrorThresholdOverride;
	}
	if (UE::HeroesOfLiberty::Net::RefreshStepsOverride > 0)
	{
		Rules.RefreshSteps = UE::HeroesOfLiberty::Net::RefreshStepsOverride;
	}
	const float UpdateFrequency = UE::HeroesOfLiberty::Net::UpdateFrequencyOverride > 0.f
		? UE::HeroesOfLiberty::Net::UpdateFrequencyOverride : Settings->NetUpdateFrequency;
	if (Replicator->GetNetUpdateFrequency() != UpdateFrequency)
	{
		Replicator->SetUpdateFrequency(UpdateFrequency);
		UE_LOG(LogHOLNet, Display, TEXT("Unit replicator: %.0f updates per second"), UpdateFrequency);
	}
	const int32 SampleEvery = UE::HeroesOfLiberty::Net::SampleEvery;
	FString Sample;

	Replicator->SetServerStep(Step);
	const auto WriteState = [this, Step](FUnitRecord& Record, const FVector& Position, const FVector& Velocity, const EHOLUnitActivity Activity)
	{
		Record.Sent.State = FHOLNetUnitState::Make(Position, Velocity, Activity, Step);
		Record.Sent.Step = Step;
		Replicator->GetOrAddChunk(Record.Slot / AHOLUnitChunk::NumSlots)->SetState(Record.Slot % AHOLUnitChunk::NumSlots, Record.Sent.State);
	};

	FMassExecutionContext ExecutionContext(*EntityManager, StepSeconds);
	UnitQuery.ForEachEntityChunk(ExecutionContext, [this, Step, &Rules, &WriteState, SampleEvery, &Sample](FMassExecutionContext& Context)
	{
		const TConstArrayView<FHOLUnitStateFragment> States = Context.GetFragmentView<FHOLUnitStateFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMassVelocityFragment> Velocities = Context.GetFragmentView<FMassVelocityFragment>();
		const TConstArrayView<FHOLPreviousTransformFragment> PreviousTransforms = Context.GetFragmentView<FHOLPreviousTransformFragment>();
		const FHOLUnitTypeConstants& Constants = Context.GetConstSharedFragment<FHOLUnitTypeConstants>();
		const uint8 TypeIndex = TypeIndices.FindRef(Constants.TypeName);

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FHOLUnitStateFragment& State = States[EntityIt];
			const FVector Position = Transforms[EntityIt].GetTransform().GetLocation();
			Stats.NumUnitSteps++;

			// The instant velocity: S5 measured more sends with the move over the last 4 steps (it lags in turns). The
			// move of the last step, not the walk's velocity: in a crowd, the separation's pushes (D292) are part of it,
			// and a client extrapolating the walk alone drifted away (S5: 1.4 -> 2.0 sends per unit and per second).
			const FVector Velocity = UE::HeroesOfLiberty::Net::StepVelocity != 0 && StepSeconds > 0.f
				? (Position - PreviousTransforms[EntityIt].Transform.GetLocation()) / StepSeconds : Velocities[EntityIt].Value;

			FUnitRecord* Record = Records.Find(State.UnitId);
			if (Record == nullptr)
			{
				// A new unit: a slot, its identity once, its first state.
				Record = &Records.Add(State.UnitId);
				Record->Slot = TakeSlot();
				Record->OwnerPlayer = State.OwnerPlayer;
				Replicator->GetOrAddChunk(Record->Slot / AHOLUnitChunk::NumSlots)
					->SetIdentity(Record->Slot % AHOLUnitChunk::NumSlots, State.UnitId, TypeIndex, State.OwnerPlayer);
				WriteState(*Record, Position, Velocity, State.Activity);
				Stats.Sends[static_cast<int32>(EHOLUnitSendReason::None)]++;
			}
			else
			{
				if (Record->OwnerPlayer != State.OwnerPlayer)
				{
					// A conversion (D37): the identity is sent again.
					Record->OwnerPlayer = State.OwnerPlayer;
					Replicator->GetOrAddChunk(Record->Slot / AHOLUnitChunk::NumSlots)
						->SetIdentity(Record->Slot % AHOLUnitChunk::NumSlots, State.UnitId, TypeIndex, State.OwnerPlayer);
				}
				const EHOLUnitSendReason Reason = HOLUnitReplication::GetSendReason(Record->Sent, Position, State.Activity,
					State.UnitId, Step, StepSeconds, Rules);
				if (Reason != EHOLUnitSendReason::None)
				{
					WriteState(*Record, Position, Velocity, State.Activity);
					Stats.Sends[static_cast<int32>(Reason)]++;
				}
			}
			Record->SeenStep = Step;
			if (SampleEvery > 0 && State.UnitId % SampleEvery == 0)
			{
				Sample += FString::Printf(TEXT(" %u:%.0f,%.0f"), State.UnitId, Position.X, Position.Y);
			}
		}
	});

	// The units not seen at this step are gone: their slots are freed.
	for (auto It = Records.CreateIterator(); It; ++It)
	{
		if (It->Value.SeenStep != Step)
		{
			FreeSlot(It->Value.Slot);
			It.RemoveCurrent();
			++Stats.NumDepartures;
		}
	}

	const double Ms = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;
	++Stats.NumSteps;
	Stats.TotalMs += Ms;
	Stats.MaxMs = FMath::Max(Stats.MaxMs, Ms);

	if (!Sample.IsEmpty())
	{
		// The true positions at this step (taken when it ended), against what the clients display (S5, D273).
		UE_LOG(LogHOLNet, Display, TEXT("NetSample server step=%lld t=%.4f%s"), Step, StartSeconds, *Sample);
	}

	const double Now = FPlatformTime::Seconds();
	if (UE::HeroesOfLiberty::Net::ReportInterval > 0.f && Now - LastReportSeconds >= UE::HeroesOfLiberty::Net::ReportInterval)
	{
		LastReportSeconds = Now;
		UE_LOG(LogHOLNet, Display, TEXT("%s"), *MakeReport());
		ResetStats();
	}
}

int32 UHOLUnitReplicationSubsystem::TakeSlot()
{
	if (FreeSlots.IsEmpty())
	{
		// A new chunk: its slots are free, the lowest taken first.
		const int32 ChunkIndex = Replicator->GetChunks().Num();
		Replicator->GetOrAddChunk(ChunkIndex);
		for (int32 Slot = AHOLUnitChunk::NumSlots - 1; Slot >= 0; --Slot)
		{
			FreeSlots.Add(ChunkIndex * AHOLUnitChunk::NumSlots + Slot);
		}
	}
	return FreeSlots.Pop(EAllowShrinking::No);
}

void UHOLUnitReplicationSubsystem::FreeSlot(const int32 Slot)
{
	Replicator->GetOrAddChunk(Slot / AHOLUnitChunk::NumSlots)->SetIdentity(Slot % AHOLUnitChunk::NumSlots, 0, 0, 0);
	FreeSlots.Add(Slot);
}

FString UHOLUnitReplicationSubsystem::MakeReport() const
{
	const UWorld* World = GetWorld();
	const UNetDriver* NetDriver = World ? World->GetNetDriver() : nullptr;

	if (Replicator == nullptr)
	{
		// A client: what the shared actor and the chunk actors brought.
		const AHOLUnitReplicator* Received = nullptr;
		for (TActorIterator<AHOLUnitReplicator> It(const_cast<UWorld*>(World)); It; ++It)
		{
			Received = *It;
		}
		if (Received == nullptr)
		{
			return TEXT("Net (client): no unit replicator received");
		}
		int32 NumUnits = 0;
		int32 NumChunks = 0;
		const UHOLUnitReceptionSubsystem* Reception = World->GetSubsystem<UHOLUnitReceptionSubsystem>();
		for (const TWeakObjectPtr<AHOLUnitChunk>& ChunkPtr : Reception ? Reception->GetChunks() : TArray<TWeakObjectPtr<AHOLUnitChunk>>())
		{
			const AHOLUnitChunk* Chunk = ChunkPtr.Get();
			if (Chunk == nullptr)
			{
				continue;
			}
			++NumChunks;
			for (int32 Slot = 0; Slot < AHOLUnitChunk::NumSlots && Chunk->HasAllSlots(); ++Slot)
			{
				NumUnits += Chunk->GetIdentity(Slot).UnitId != 0 ? 1 : 0;
			}
		}
		return FString::Printf(TEXT("Net (client): %d units in %d chunks, server step %d"), NumUnits, NumChunks, Received->GetServerStep());
	}

	const double Seconds = FMath::Max(FPlatformTime::Seconds() - Stats.StartSeconds, 1.0e-3);
	const double UnitSeconds = FMath::Max(static_cast<double>(Stats.NumUnitSteps) * StepSeconds, 1.0e-3);
	int64 TotalSends = 0;
	FString Reasons;
	for (int32 Reason = 0; Reason < UE_ARRAY_COUNT(Stats.Sends); ++Reason)
	{
		TotalSends += Stats.Sends[Reason];
		Reasons += FString::Printf(TEXT("%s%s %lld"), Reasons.IsEmpty() ? TEXT("") : TEXT(", "),
			UE::HeroesOfLiberty::Net::GetSendReasonName(Reason), Stats.Sends[Reason]);
	}
	// Averages over the report's span, from the connections' running totals (OutBytesPerSecond is the last second only).
	FString Clients;
	FString Packets;
	int32 NumClients = 0;
	if (NetDriver)
	{
		for (const UNetConnection* Connection : NetDriver->ClientConnections)
		{
			if (Connection)
			{
				const TPair<int32, int32> AtReset = Stats.OutTotals.FindRef(Connection);
				Clients += FString::Printf(TEXT(" %.0f"), (Connection->OutTotalBytes - AtReset.Key) / Seconds);
				Packets += FString::Printf(TEXT(" %.0f"), (Connection->OutTotalPackets - AtReset.Value) / Seconds);
				++NumClients;
			}
		}
	}
	// States really written, against the ones the send rule wrote into the slots (a state rewritten before the next
	// update leaves once; a lost one leaves again).
	const double WireStates = (UE::HeroesOfLiberty::Net::GetNumSerializedUnitStates() - Stats.SerializedStates) / Seconds / FMath::Max(NumClients, 1);
	return FString::Printf(TEXT("Net (server): %d units, %d steps in %.1f s, %.2f sends per unit and per second (%s), %d gone; %.3f ms per step (max %.3f); %.0f updates per second; %.0f states on the wire per client and per second; out B/s per client:%s; packets/s:%s"),
		Records.Num(), Stats.NumSteps, Seconds, TotalSends / UnitSeconds, *Reasons, Stats.NumDepartures,
		Stats.NumSteps > 0 ? Stats.TotalMs / Stats.NumSteps : 0.0, Stats.MaxMs, Replicator->GetNetUpdateFrequency(), WireStates,
		Clients.IsEmpty() ? TEXT(" none") : *Clients, Packets.IsEmpty() ? TEXT(" none") : *Packets);
}
