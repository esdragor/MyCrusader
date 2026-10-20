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

namespace
{
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
			UE_LOG(LogHOLPresentation, Display, TEXT("Arrow report: in flight %d launched %lld landed %lld"), ArrowSubsystem->GetInFlightCount(),
				ArrowSubsystem->GetLaunchedCount(), ArrowSubsystem->GetLandedCount());
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

bool UHOLArrowSubsystem::EnsureComponent()
{
	if (Instances)
	{
		return true;
	}
	if (bMeshMissing)
	{
		return false;
	}
	UWorld* World = GetWorld();
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, ArrowMeshPath);
	if (!World || !Mesh)
	{
		bMeshMissing = true;
		UE_LOG(LogHOLPresentation, Warning, TEXT("Arrows: no mesh %s, arrows not drawn"), ArrowMeshPath);
		return false;
	}
	FActorSpawnParameters Parameters;
	Parameters.ObjectFlags |= RF_Transient;
	AActor* Owner = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Parameters);
	if (!Owner)
	{
		return false;
	}
	Instances = NewObject<UInstancedStaticMeshComponent>(Owner, TEXT("Arrows"));
	Instances->SetStaticMesh(Mesh);
	Instances->SetMobility(EComponentMobility::Movable);
	Instances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Instances->SetCastShadow(false);
	Instances->SetCanEverAffectNavigation(false);
	Owner->SetRootComponent(Instances);
	Instances->RegisterComponent();
	return true;
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
	++Launched;
}

void UHOLArrowSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	const UWorld* World = GetWorld();
	if (!World || (Arrows.IsEmpty() && (!Instances || Instances->GetInstanceCount() == 0)))
	{
		return;
	}
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	FMassEntityManager* EntityManager = EntitySubsystem ? &EntitySubsystem->GetMutableEntityManager() : nullptr;
	const double Now = World->GetTimeSeconds();

	TArray<FTransform> Transforms;
	Transforms.Reserve(Arrows.Num());
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
			Arrows.RemoveAtSwap(Index, EAllowShrinking::No);
			continue;
		}
		const double T = FMath::Max(Alpha, 0.0);
		const FVector Span = Arrow.LastTarget - Arrow.From;
		const double Arc = ArcShare * FVector2D(Span.X, Span.Y).Size();
		const FVector Position = Arrow.From + Span * T + FVector(0.0, 0.0, Arc * 4.0 * T * (1.0 - T));
		const FVector Velocity = Span + FVector(0.0, 0.0, Arc * (4.0 - 8.0 * T));
		Transforms.Emplace(FRotationMatrix::MakeFromX(Velocity.GetSafeNormal()).ToQuat(), Position, ArrowScale);
	}

	if (!EnsureComponent())
	{
		return;
	}
	const int32 Have = Instances->GetInstanceCount();
	if (Have > Transforms.Num())
	{
		TArray<int32> Excess;
		for (int32 Index = Have - 1; Index >= Transforms.Num(); --Index)
		{
			Excess.Add(Index);
		}
		Instances->RemoveInstances(Excess);
	}
	else if (Have < Transforms.Num())
	{
		TArray<FTransform> Added;
		Added.Init(FTransform::Identity, Transforms.Num() - Have);
		Instances->AddInstances(Added, false, true);
	}
	if (!Transforms.IsEmpty())
	{
		Instances->BatchUpdateInstancesTransforms(0, Transforms, true, true, true);
	}
}
