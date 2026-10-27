// Copyright Epic Games, Inc. All Rights Reserved.

#include "Presentation/HOLArrowSubsystem.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "MassCommonFragments.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"

namespace
{
	/** The fallback projectile (AR9): a shooter type whose Projectile column is empty shoots an arrow. */
	const TCHAR* ArrowMeshPath = TEXT("/Game/Units/Props/SM_Arrow.SM_Arrow");
	/** Where the arrow leaves the shooter (his drawing shoulder) and reaches its target (its chest), above their feet (cm). */
	constexpr double ReleaseHeight = 150.0;
	constexpr double ReleaseForward = 30.0;
	constexpr double ImpactHeight = 120.0;
	/** The light arc (Q4): its top over the middle of the flight, as a share of the ground distance. */
	constexpr double ArcShare = 0.06;
	/** The arrow drawn larger than life to be seen from the RTS camera (J7c.2b, trial): at its true 1.2 cm shaft it was
	 *  a pixel or two wide in the close-ups. 1.5 times as long (~117 cm), 4 times as thick (shaft ~5 cm). */
	const FVector ArrowScale(1.5, 4.0, 4.0);

	FAutoConsoleCommandWithWorld ArrowReportCommand(
		TEXT("hol.Combat.ArrowReport"),
		TEXT("Logs the arrows drawn on this machine (J7c.2b): in flight, launched, landed. Nothing on a dedicated server."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			const UHOLArrowSubsystem* ArrowSubsystem = World ? World->GetSubsystem<UHOLArrowSubsystem>() : nullptr;
			if (!ArrowSubsystem)
			{
				UE_LOG(LogHOLPresentation, Display, TEXT("Arrow report: no arrow display on this machine"));
				return;
			}
			UE_LOG(LogHOLPresentation, Display, TEXT("Arrow report: in flight %d launched %lld landed %lld gaps %lld mean %.0f min %.0f max %.0f ms"),
				ArrowSubsystem->GetInFlightCount(), ArrowSubsystem->GetLaunchedCount(), ArrowSubsystem->GetLandedCount(), ArrowSubsystem->GetGapCount(),
				ArrowSubsystem->GetGapMeanMs(), ArrowSubsystem->GetGapMinMs(), ArrowSubsystem->GetGapMaxMs());
		}));
}

bool UHOLArrowSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->GetNetMode() != NM_DedicatedServer && !IsRunningDedicatedServer();
}

bool UHOLArrowSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UHOLArrowSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UHOLArrowSubsystem, STATGROUP_Tickables);
}

UStaticMesh* UHOLArrowSubsystem::GetProjectileMesh(const FName ShooterType)
{
	// The shooter type's Projectile column (AR9); the arrow when it is empty or the type unknown.
	const UHOLSimulationSettings* Settings = GetDefault<UHOLSimulationSettings>();
	const UDataTable* UnitTypes = Settings ? Settings->UnitTypesTable.LoadSynchronous() : nullptr;
	const FHOLUnitTypeRow* Row = UnitTypes && !ShooterType.IsNone() ? UnitTypes->FindRow<FHOLUnitTypeRow>(ShooterType, TEXT("Arrows"), false) : nullptr;
	if (Row && !Row->Projectile.IsNull())
	{
		if (UStaticMesh* Mesh = Row->Projectile.LoadSynchronous())
		{
			return Mesh;
		}
	}
	return LoadObject<UStaticMesh>(nullptr, ArrowMeshPath);
}

const UInstancedStaticMeshComponent* UHOLArrowSubsystem::GetInstances(const UStaticMesh* Mesh) const
{
	const int32 Group = Meshes.IndexOfByKey(Mesh);
	return Group != INDEX_NONE ? Instances[Group].Get() : nullptr;
}

int32 UHOLArrowSubsystem::FindOrAddGroup(const FName ShooterType)
{
	if (const int32* Known = GroupByType.Find(ShooterType))
	{
		return *Known;
	}
	int32& Group = GroupByType.Add(ShooterType, INDEX_NONE);
	UWorld* World = GetWorld();
	UStaticMesh* Mesh = GetProjectileMesh(ShooterType);
	if (!World || !Mesh)
	{
		UE_LOG(LogHOLPresentation, Warning, TEXT("Arrows: no projectile mesh for %s (nor %s), its shots not drawn"), *ShooterType.ToString(), ArrowMeshPath);
		return INDEX_NONE;
	}
	Group = Meshes.IndexOfByKey(Mesh);
	if (Group != INDEX_NONE)
	{
		return Group;
	}
	if (!InstancesOwner)
	{
		FActorSpawnParameters Parameters;
		Parameters.ObjectFlags |= RF_Transient;
		InstancesOwner = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Parameters);
		if (!InstancesOwner)
		{
			return INDEX_NONE;
		}
	}
	UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(InstancesOwner, *FString::Printf(TEXT("Projectiles_%s"), *Mesh->GetName()));
	Component->SetStaticMesh(Mesh);
	Component->SetMobility(EComponentMobility::Movable);
	Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Component->SetCastShadow(false);
	Component->SetCanEverAffectNavigation(false);
	if (!InstancesOwner->GetRootComponent())
	{
		InstancesOwner->SetRootComponent(Component);
	}
	else
	{
		Component->SetupAttachment(InstancesOwner->GetRootComponent());
	}
	Component->RegisterComponent();
	Group = Meshes.Add(Mesh);
	Instances.Add(Component);
	return Group;
}

void UHOLArrowSubsystem::Launch(const FHOLArrowLaunch& Launch)
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	FVector Ahead = Launch.To - Launch.From;
	Ahead.Z = 0.0;
	Ahead = Ahead.GetSafeNormal();
	FArrow& Arrow = Arrows.AddDefaulted_GetRef();
	Arrow.From = Launch.From + Ahead * ReleaseForward + FVector(0.0, 0.0, ReleaseHeight);
	Arrow.LastTarget = Launch.To + FVector(0.0, 0.0, ImpactHeight);
	Arrow.Target = Launch.Target;
	Arrow.Start = World->GetTimeSeconds();
	Arrow.Duration = FMath::Max(Launch.FlightMs, 1) / 1000.0;
	Arrow.Group = FindOrAddGroup(Launch.ShooterType);
	++Launched;
}

void UHOLArrowSubsystem::NoteHealthDrop(const FMassEntityHandle& Target)
{
	const UWorld* World = GetWorld();
	if (!World || !Target.IsSet())
	{
		return;
	}
	const double Now = World->GetTimeSeconds();
	double GapSeconds = 0.0;
	bool bFound = false;
	for (const FArrow& Arrow : Arrows)
	{
		if (Arrow.Target == Target)
		{
			// The earliest to land: the largest (least negative) gap.
			const double Gap = Now - (Arrow.Start + Arrow.Duration);
			GapSeconds = bFound ? FMath::Max(GapSeconds, Gap) : Gap;
			bFound = true;
		}
	}
	if (const double* Landing = !bFound ? LastLanding.Find(Target) : nullptr; Landing && Now - *Landing <= MaxGapSeconds)
	{
		GapSeconds = Now - *Landing;
		bFound = true;
		LastLanding.Remove(Target);
	}
	if (!bFound)
	{
		return;
	}
	const double GapMs = GapSeconds * 1000.0;
	GapMinMs = GapCount == 0 ? GapMs : FMath::Min(GapMinMs, GapMs);
	GapMaxMs = GapCount == 0 ? GapMs : FMath::Max(GapMaxMs, GapMs);
	GapSumMs += GapMs;
	++GapCount;
}

void UHOLArrowSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	const UWorld* World = GetWorld();
	if (!World || (Arrows.IsEmpty() && !Instances.ContainsByPredicate([](const UInstancedStaticMeshComponent* Component)
		{
			return Component && Component->GetInstanceCount() > 0;
		})))
	{
		return;
	}
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	FMassEntityManager* EntityManager = EntitySubsystem ? &EntitySubsystem->GetMutableEntityManager() : nullptr;
	const double Now = World->GetTimeSeconds();
	for (auto It = LastLanding.CreateIterator(); It; ++It)
	{
		if (Now - It.Value() > MaxGapSeconds)
		{
			It.RemoveCurrent();
		}
	}

	// Each mesh's instances (AR9), in its group's order.
	TArray<TArray<FTransform>> GroupTransforms;
	GroupTransforms.SetNum(Instances.Num());
	for (int32 Index = Arrows.Num() - 1; Index >= 0; --Index)
	{
		FArrow& Arrow = Arrows[Index];
		// The target followed while it lives (its drawn position); gone, its last one.
		if (EntityManager && Arrow.Target.IsSet() && EntityManager->IsEntityValid(Arrow.Target))
		{
			if (const FTransformFragment* Transform = EntityManager->GetFragmentDataPtr<FTransformFragment>(Arrow.Target))
			{
				Arrow.LastTarget = Transform->GetTransform().GetLocation() + FVector(0.0, 0.0, ImpactHeight);
			}
		}
		const double Alpha = (Now - Arrow.Start) / Arrow.Duration;
		if (Alpha >= 1.0)
		{
			++Landed;
			if (Arrow.Target.IsSet())
			{
				LastLanding.Add(Arrow.Target, Now);
			}
			Arrows.RemoveAtSwap(Index, EAllowShrinking::No);
			continue;
		}
		if (!GroupTransforms.IsValidIndex(Arrow.Group))
		{
			continue;
		}
		const double T = FMath::Max(Alpha, 0.0);
		const FVector Span = Arrow.LastTarget - Arrow.From;
		const double Arc = ArcShare * FVector2D(Span.X, Span.Y).Size();
		const FVector Position = Arrow.From + Span * T + FVector(0.0, 0.0, Arc * 4.0 * T * (1.0 - T));
		const FVector Velocity = Span + FVector(0.0, 0.0, Arc * (4.0 - 8.0 * T));
		GroupTransforms[Arrow.Group].Emplace(FRotationMatrix::MakeFromX(Velocity.GetSafeNormal()).ToQuat(), Position, ArrowScale);
	}

	for (int32 Group = 0; Group < Instances.Num(); ++Group)
	{
		UInstancedStaticMeshComponent* Component = Instances[Group];
		const TArray<FTransform>& Transforms = GroupTransforms[Group];
		if (!Component)
		{
			continue;
		}
		const int32 Have = Component->GetInstanceCount();
		if (Have > Transforms.Num())
		{
			TArray<int32> Excess;
			for (int32 Index = Have - 1; Index >= Transforms.Num(); --Index)
			{
				Excess.Add(Index);
			}
			Component->RemoveInstances(Excess);
		}
		else if (Have < Transforms.Num())
		{
			TArray<FTransform> Added;
			Added.Init(FTransform::Identity, Transforms.Num() - Have);
			Component->AddInstances(Added, false, true);
		}
		if (!Transforms.IsEmpty())
		{
			Component->BatchUpdateInstancesTransforms(0, Transforms, true, true, true);
		}
	}
}
