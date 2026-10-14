// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLUnitReplicationSubsystem.h"

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
#include "Net/HOLUnitReceptionSubsystem.h"
#include "Net/HOLUnitReplicationSettings.h"
#include "Net/HOLUnitReplicator.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

namespace UE::HeroesOfLiberty::Net
{
	/** S5 knob (D269): overrides the settings' threshold when > 0. */
	float ErrorThresholdOverride = 0.f;
	FAutoConsoleVariableRef CVarErrorThreshold(TEXT("hol.Net.ErrorThreshold"), ErrorThresholdOverride,
		TEXT("Prediction error (cm) past which the server sends a unit again, 0 = UHOLUnitReplicationSettings value"));

	/** Server: a report line every this many seconds, 0 = none (S5). */
	float ReportInterval = 0.f;
	FAutoConsoleVariableRef CVarReportInterval(TEXT("hol.Net.ReportInterval"), ReportInterval,
		TEXT("Seconds between two unit replication reports of the server (sends then reset), 0 = none"));

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
		Settings->ErrorThreshold, Settings->RefreshSteps, Settings->NetUpdateFrequency, TypeIndices.Num(), UHOLUnitChunk::NumSlots);
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

	Replicator->SetServerStep(Step);
	const auto WriteState = [this, Step](FUnitRecord& Record, const FVector& Position, const FVector& Velocity, const EHOLUnitActivity Activity)
	{
		Record.Sent.State = FHOLNetUnitState::Make(Position, Velocity, Activity, Step);
		Record.Sent.Step = Step;
		Replicator->GetOrAddChunk(Record.Slot / UHOLUnitChunk::NumSlots)->SetState(Record.Slot % UHOLUnitChunk::NumSlots, Record.Sent.State);
	};

	FMassExecutionContext ExecutionContext(*EntityManager, StepSeconds);
	UnitQuery.ForEachEntityChunk(ExecutionContext, [this, Step, &Rules, &WriteState](FMassExecutionContext& Context)
	{
		const TConstArrayView<FHOLUnitStateFragment> States = Context.GetFragmentView<FHOLUnitStateFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMassVelocityFragment> Velocities = Context.GetFragmentView<FMassVelocityFragment>();
		const FHOLUnitTypeConstants& Constants = Context.GetConstSharedFragment<FHOLUnitTypeConstants>();
		const uint8 TypeIndex = TypeIndices.FindRef(Constants.TypeName);

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FHOLUnitStateFragment& State = States[EntityIt];
			const FVector Position = Transforms[EntityIt].GetTransform().GetLocation();
			const FVector& Velocity = Velocities[EntityIt].Value;
			Stats.NumUnitSteps++;

			FUnitRecord* Record = Records.Find(State.UnitId);
			if (Record == nullptr)
			{
				// A new unit: a slot, its identity once, its first state.
				Record = &Records.Add(State.UnitId);
				Record->Slot = TakeSlot();
				Record->OwnerPlayer = State.OwnerPlayer;
				Replicator->GetOrAddChunk(Record->Slot / UHOLUnitChunk::NumSlots)
					->SetIdentity(Record->Slot % UHOLUnitChunk::NumSlots, State.UnitId, TypeIndex, State.OwnerPlayer);
				WriteState(*Record, Position, Velocity, State.Activity);
				Stats.Sends[static_cast<int32>(EHOLUnitSendReason::None)]++;
			}
			else
			{
				if (Record->OwnerPlayer != State.OwnerPlayer)
				{
					// A conversion (D37): the identity is sent again.
					Record->OwnerPlayer = State.OwnerPlayer;
					Replicator->GetOrAddChunk(Record->Slot / UHOLUnitChunk::NumSlots)
						->SetIdentity(Record->Slot % UHOLUnitChunk::NumSlots, State.UnitId, TypeIndex, State.OwnerPlayer);
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
		for (int32 Slot = UHOLUnitChunk::NumSlots - 1; Slot >= 0; --Slot)
		{
			FreeSlots.Add(ChunkIndex * UHOLUnitChunk::NumSlots + Slot);
		}
	}
	return FreeSlots.Pop(EAllowShrinking::No);
}

void UHOLUnitReplicationSubsystem::FreeSlot(const int32 Slot)
{
	Replicator->GetOrAddChunk(Slot / UHOLUnitChunk::NumSlots)->SetIdentity(Slot % UHOLUnitChunk::NumSlots, 0, 0, 0);
	FreeSlots.Add(Slot);
}

FString UHOLUnitReplicationSubsystem::MakeReport() const
{
	const UWorld* World = GetWorld();
	const UNetDriver* NetDriver = World ? World->GetNetDriver() : nullptr;

	if (Replicator == nullptr)
	{
		// A client: what the shared actor brought (M2.5.4 will display it).
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
		for (const UHOLUnitChunk* Chunk : Received->GetChunks())
		{
			if (Chunk == nullptr)
			{
				continue;
			}
			++NumChunks;
			for (const FHOLNetUnitIdentityItem& Identity : Chunk->GetIdentities())
			{
				NumUnits += Identity.UnitId != 0 ? 1 : 0;
			}
		}
		const UNetConnection* Connection = NetDriver ? NetDriver->ServerConnection.Get() : nullptr;
		return FString::Printf(TEXT("Net (client): %d units in %d chunks, server step %d, in %d B/s"),
			NumUnits, NumChunks, Received->GetServerStep(), Connection ? Connection->InBytesPerSecond : -1);
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
	FString Clients;
	if (NetDriver)
	{
		for (const UNetConnection* Connection : NetDriver->ClientConnections)
		{
			if (Connection)
			{
				Clients += FString::Printf(TEXT(" %d"), Connection->OutBytesPerSecond);
			}
		}
	}
	return FString::Printf(TEXT("Net (server): %d units, %d steps in %.1f s, %.2f sends per unit and per second (%s), %d gone; %.3f ms per step (max %.3f); out B/s per client:%s"),
		Records.Num(), Stats.NumSteps, Seconds, TotalSends / UnitSeconds, *Reasons, Stats.NumDepartures,
		Stats.NumSteps > 0 ? Stats.TotalMs / Stats.NumSteps : 0.0, Stats.MaxMs, Clients.IsEmpty() ? TEXT(" none") : *Clients);
}
