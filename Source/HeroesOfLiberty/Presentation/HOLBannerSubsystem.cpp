// Copyright Epic Games, Inc. All Rights Reserved.

#include "Presentation/HOLBannerSubsystem.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLMatchConfig.h"
#include "Simulation/HOLHeroLibrary.h"

namespace
{
	const TCHAR* BannerMeshPath = TEXT("/Game/Units/Props/SM_Banner.SM_Banner");
	/** The trace for the ground: from high to low over the whole map (as the aura circles'). */
	constexpr double GroundTraceHalfHeight = 100000.0;
	/** The sway: the standard turns this far about its pole either way (degrees), once in this many seconds, each slot out
	 *  of step with the others by this fraction of a turn. */
	constexpr double SwayDegrees = 5.0;
	constexpr double SwayPeriodSeconds = 5.0;
	constexpr double SwaySlotPhase = 0.37;

	FAutoConsoleCommandWithWorld BannerReportCommand(
		TEXT("hol.Hero.BannerDisplay"),
		TEXT("Logs the banners drawn on this machine (J8c.4 loop 4). Nothing on a dedicated server."),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			const UHOLBannerSubsystem* Display = World ? World->GetSubsystem<UHOLBannerSubsystem>() : nullptr;
			if (!Display)
			{
				UE_LOG(LogHOLPresentation, Display, TEXT("Banner display: none on this machine"));
				return;
			}
			FString Line = FString::Printf(TEXT("Banner display: %d shown"), Display->GetShown().Num());
			for (const UHOLBannerSubsystem::FShown& Banner : Display->GetShown())
			{
				Line += FString::Printf(TEXT(", slot %d (%.0f, %.0f, %.0f)"), Banner.Slot, Banner.Location.X, Banner.Location.Y, Banner.Location.Z);
			}
			UE_LOG(LogHOLPresentation, Display, TEXT("%s"), *Line);
		}));
}

bool UHOLBannerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->GetNetMode() != NM_DedicatedServer && !IsRunningDedicatedServer();
}

bool UHOLBannerSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UHOLBannerSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UHOLBannerSubsystem, STATGROUP_Tickables);
}

double UHOLBannerSubsystem::GroundAt(const FVector2D& Point, const double Fallback) const
{
	const UWorld* World = GetWorld();
	TArray<FHitResult> Hits;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(HOLBannerGround), false);
	if (!World || !World->LineTraceMultiByObjectType(Hits, FVector(Point.X, Point.Y, GroundTraceHalfHeight), FVector(Point.X, Point.Y, -GroundTraceHalfHeight),
		FCollisionObjectQueryParams(ECC_WorldStatic), Params))
	{
		return Fallback;
	}
	double Height = TNumericLimits<double>::Max();
	for (const FHitResult& Hit : Hits)
	{
		Height = FMath::Min(Height, Hit.ImpactPoint.Z);
	}
	return Height;
}

void UHOLBannerSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	// The banners standing: a public sheet's place and the step it falls at, against the server's step.
	Shown.Reset();
	const int32 Step = UHOLHeroLibrary::GetServerStep(this);
	for (int32 Slot = 0; Step >= 0 && Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		if (UHOLHeroLibrary::GetHeroBannerEndStep(this, Slot) > Step)
		{
			const FIntPoint Place = UHOLHeroLibrary::GetHeroBannerLocation(this, Slot);
			TPair<FIntPoint, double>* Known = GroundCache.Find(Slot);
			if (!Known || Known->Key != Place)
			{
				Known = &GroundCache.Add(Slot, TPair<FIntPoint, double>(Place, GroundAt(FVector2D(Place.X, Place.Y), 0.0)));
			}
			FShown& Banner = Shown.AddDefaulted_GetRef();
			Banner.Slot = Slot;
			Banner.Location = FVector(Place.X, Place.Y, Known->Value);
		}
	}
	if (Shown.IsEmpty() && (!Instances || Instances->GetInstanceCount() == 0))
	{
		return;
	}
	if (!Instances)
	{
		UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, BannerMeshPath);
		if (!Mesh)
		{
			UE_LOG(LogHOLPresentation, Warning, TEXT("Banners: no mesh %s, the banners not drawn"), BannerMeshPath);
			return;
		}
		FActorSpawnParameters Parameters;
		Parameters.ObjectFlags |= RF_Transient;
		InstancesOwner = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Parameters);
		if (!InstancesOwner)
		{
			return;
		}
		Instances = NewObject<UInstancedStaticMeshComponent>(InstancesOwner, TEXT("Banners"));
		Instances->SetStaticMesh(Mesh);
		Instances->SetMobility(EComponentMobility::Movable);
		Instances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Instances->SetCastShadow(false);
		Instances->SetCanEverAffectNavigation(false);
		InstancesOwner->SetRootComponent(Instances);
		Instances->RegisterComponent();
	}
	const double Now = World->GetTimeSeconds();
	TArray<FTransform> Transforms;
	for (const FShown& Banner : Shown)
	{
		const double Turn = Now / SwayPeriodSeconds + Banner.Slot * SwaySlotPhase;
		const double Yaw = SwayDegrees * FMath::Sin(UE_DOUBLE_TWO_PI * Turn);
		Transforms.Emplace(FRotator(0.0, Yaw, 0.0), Banner.Location, FVector::OneVector);
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
