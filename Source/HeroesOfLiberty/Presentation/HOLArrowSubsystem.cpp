// Copyright Epic Games, Inc. All Rights Reserved.

#include "Presentation/HOLArrowSubsystem.h"

#include "Art/HOLArtVisual.h"
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
	const FVector ArrowBaseScale(1.5, 4.0, 4.0);
	/** S3-a: for the trials only: the arrow drawn this many times larger (default 1 = the base above), and its drawing this many times slower (default 1). */
	float ArrowScaleFactor = 1.f;
	FAutoConsoleVariableRef CVarArrowScale(TEXT("hol.Arrow.Scale"), ArrowScaleFactor, TEXT("Multiplier of the drawn arrow's scale (default 1: unchanged; a trial raises it, e.g. 3, to see the arrow of a tower)"));
	float ArrowSlowMotion = 1.f;
	FAutoConsoleVariableRef CVarArrowSlowMotion(TEXT("hol.Arrow.SlowMotion"), ArrowSlowMotion, TEXT("Multiplier of the drawn arrow's flight time (default 1: unchanged; a trial raises it, e.g. 4, so the arrow stays in view; the damage still lands on time)"));

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

FVector UHOLArrowSubsystem::GetArrowScale()
{
	return ArrowBaseScale * static_cast<double>(ArrowScaleFactor);
}

double UHOLArrowSubsystem::GetFlightDurationSeconds(const int32 FlightMs)
{
	return FMath::Max(FlightMs, 1) / 1000.0 * static_cast<double>(FMath::Max(ArrowSlowMotion, 0.01f));
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

UStaticMesh* UHOLArrowSubsystem::GetProjectileMesh(const FName ShooterType, const FName ShooterFaction)
{
	// The shooter type's Projectile column (AR9); the arrow when it is empty or the type unknown.
	const UHOLSimulationSettings* Settings = GetDefault<UHOLSimulationSettings>();
	const UDataTable* UnitTypes = Settings ? Settings->UnitTypesTable.LoadSynchronous() : nullptr;
	const FHOLUnitTypeRow* Row = UnitTypes && !ShooterType.IsNone() ? UnitTypes->FindRow<FHOLUnitTypeRow>(ShooterType, TEXT("Arrows"), false) : nullptr;
	// D924 (AR0-a2): the art asset of the shooter type first (no faction known here: the common one), the CSV row last.
	const TSoftObjectPtr<UStaticMesh> Projectile(HOLArtVisual::ResolveProjectile(GetWorld(), ShooterType, NAME_None, Row ? Row->Projectile.ToSoftObjectPath() : FSoftObjectPath()));
	if (!Projectile.IsNull())
	{
		if (UStaticMesh* Mesh = Projectile.LoadSynchronous())
		{
			return Mesh;
		}
	}
	return LoadObject<UStaticMesh>(nullptr, ArrowMeshPath);
}

UStaticMesh* UHOLArrowSubsystem::GetMuzzleSmokeMesh(const FName ShooterType, const FName ShooterFaction)
{
	const UHOLSimulationSettings* Settings = GetDefault<UHOLSimulationSettings>();
	const UDataTable* UnitTypes = Settings ? Settings->UnitTypesTable.LoadSynchronous() : nullptr;
	const FHOLUnitTypeRow* Row = UnitTypes && !ShooterType.IsNone() ? UnitTypes->FindRow<FHOLUnitTypeRow>(ShooterType, TEXT("Arrows"), false) : nullptr;
	const TSoftObjectPtr<UStaticMesh> Smoke(HOLArtVisual::ResolveMuzzleSmoke(GetWorld(), ShooterType, NAME_None, Row ? Row->MuzzleSmoke.ToSoftObjectPath() : FSoftObjectPath()));
	return Smoke.IsNull() ? nullptr : Smoke.LoadSynchronous();
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
	GroupByType.Add(ShooterType, INDEX_NONE);
	UStaticMesh* Mesh = GetProjectileMesh(ShooterType);
	if (!GetWorld() || !Mesh)
	{
		UE_LOG(LogHOLPresentation, Warning, TEXT("Arrows: no projectile mesh for %s (nor %s), its shots not drawn"), *ShooterType.ToString(), ArrowMeshPath);
		return INDEX_NONE;
	}
	const int32 Group = FindOrAddMeshGroup(Mesh);
	GroupByType.Add(ShooterType, Group);
	return Group;
}

int32 UHOLArrowSubsystem::FindOrAddSmokeGroup(const FName ShooterType)
{
	if (const int32* Known = SmokeGroupByType.Find(ShooterType))
	{
		return *Known;
	}
	UStaticMesh* Mesh = GetMuzzleSmokeMesh(ShooterType);
	const int32 Group = Mesh ? FindOrAddMeshGroup(Mesh, /*NumCustomData: the opacity*/ 1) : INDEX_NONE;
	SmokeGroupByType.Add(ShooterType, Group);
	return Group;
}

int32 UHOLArrowSubsystem::FindOrAddMeshGroup(UStaticMesh* Mesh, const int32 NumCustomData)
{
	UWorld* World = GetWorld();
	if (!World || !Mesh)
	{
		return INDEX_NONE;
	}
	int32 Group = Meshes.IndexOfByKey(Mesh);
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
	Component->NumCustomDataFloats = NumCustomData;
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
	Arrow.Duration = GetFlightDurationSeconds(Launch.FlightMs);
	Arrow.Group = FindOrAddGroup(Launch.ShooterType);
	++Launched;
	// A gun's shot (the Arquebusier): a puff of powder smoke at the muzzle, ahead of the shooter at his shoulder's height.
	const int32 SmokeGroup = FindOrAddSmokeGroup(Launch.ShooterType);
	if (SmokeGroup != INDEX_NONE)
	{
		FPuff& Puff = Puffs.AddDefaulted_GetRef();
		Puff.Origin = Launch.From + Ahead * MuzzleForward + FVector(0.0, 0.0, MuzzleHeight);
		Puff.Start = World->GetTimeSeconds();
		Puff.Group = SmokeGroup;
		LastSmokeOrigin = Puff.Origin;
	}
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
	if (!World || (Arrows.IsEmpty() && Puffs.IsEmpty() && !Instances.ContainsByPredicate([](const UInstancedStaticMeshComponent* Component)
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
	// The puffs' opacities, in the order of their group's instances (only the smoke's component has the custom value).
	TArray<TArray<float>> GroupOpacities;
	GroupOpacities.SetNum(Instances.Num());
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
		GroupTransforms[Arrow.Group].Emplace(FRotationMatrix::MakeFromX(Velocity.GetSafeNormal()).ToQuat(), Position, GetArrowScale());
	}

	// The puffs of powder smoke: swelling, rising and fading out over SmokeSeconds (their own scale, not the arrows'): a translucent
	// cloud whose opacity (custom data 0 of M_PowderSmoke) goes from SmokeOpacity to 0.
	for (int32 Index = Puffs.Num() - 1; Index >= 0; --Index)
	{
		const FPuff& Puff = Puffs[Index];
		const double Age = (Now - Puff.Start) / SmokeSeconds;
		if (Age >= 1.0)
		{
			Puffs.RemoveAtSwap(Index, EAllowShrinking::No);
			continue;
		}
		if (!GroupTransforms.IsValidIndex(Puff.Group))
		{
			continue;
		}
		const double T = FMath::Max(Age, 0.0);
		const double Ease = 1.0 - (1.0 - T) * (1.0 - T);
		const double Scale = FMath::Lerp(SmokeStartScale, SmokeEndScale, Ease);
		GroupTransforms[Puff.Group].Emplace(FQuat::Identity, Puff.Origin + FVector(0.0, 0.0, SmokeRise * Ease), FVector(Scale));
		GroupOpacities[Puff.Group].Add(static_cast<float>(GetSmokeOpacity(T)));
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
		const TArray<float>& Opacities = GroupOpacities[Group];
		for (int32 Index = 0; Component->NumCustomDataFloats > 0 && Index < Opacities.Num(); ++Index)
		{
			Component->SetCustomDataValue(Index, 0, Opacities[Index], Index == Opacities.Num() - 1);
		}
	}
}
