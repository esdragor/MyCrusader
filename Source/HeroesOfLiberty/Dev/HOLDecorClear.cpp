// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dev/HOLDecorClear.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Materials/MaterialInterface.h"
#include "Resources/HOLSourceField.h"
#include "Resources/HOLSourceSubsystem.h"

namespace UE::HeroesOfLiberty::Decor
{
	namespace
	{
		/** Plain's grove (maps/plain.py): a trunk or a crown, a static mesh actor whose first material is MI_Trunk or MI_Crown. */
		bool IsGroveActor(const AStaticMeshActor& Actor)
		{
			const UStaticMeshComponent* Mesh = Actor.GetStaticMeshComponent();
			const UMaterialInterface* Material = Mesh ? Mesh->GetMaterial(0) : nullptr;
			if (!Material)
			{
				return false;
			}
			const FString Name = Material->GetName();
			return Name.StartsWith(TEXT("MI_Trunk")) || Name.StartsWith(TEXT("MI_Crown"));
		}

		bool IsWithin(const FVector& Location, const FVector2D& At, const double Radius)
		{
			return FVector2D::DistSquared(FVector2D(Location), At) <= Radius * Radius;
		}

		/** The standing WOOD sources (the game's trees, drawn by AHOLSourceField) within Radius of At: their registry indices. */
		TArray<int32> WoodSourcesWithin(const UWorld& World, const FVector2D& At, const double Radius)
		{
			TArray<int32> Found;
			if (const UHOLSourceSubsystem* Registry = World.GetSubsystem<UHOLSourceSubsystem>())
			{
				const TArray<FHOLSource>& All = Registry->GetSources();
				for (int32 Index = 0; Index < All.Num(); ++Index)
				{
					if (All[Index].IsStanding() && All[Index].Resource == EHOLResource::Wood && IsWithin(All[Index].Location, At, Radius))
					{
						Found.Add(Index);
					}
				}
			}
			return Found;
		}
	}

	FDecorCount CountNear(const UWorld& World, const FVector2D& At, const double Radius, const bool bSources)
	{
		FDecorCount Count;
		if (bSources)
		{
			Count.Sources = WoodSourcesWithin(World, At, Radius).Num();
		}
		for (TActorIterator<AActor> It(const_cast<UWorld*>(&World)); It; ++It)
		{
			const AActor* Actor = *It;
			if (const AStaticMeshActor* Shape = Cast<AStaticMeshActor>(Actor))
			{
				Count.Actors += IsGroveActor(*Shape) && IsWithin(Shape->GetActorLocation(), At, Radius);
			}
			if (Actor->IsA<AHOLSourceField>())
			{
				continue;
			}
			TArray<UInstancedStaticMeshComponent*> Meshes;
			Actor->GetComponents(Meshes);
			for (const UInstancedStaticMeshComponent* Mesh : Meshes)
			{
				for (int32 Instance = 0; Instance < Mesh->GetInstanceCount(); ++Instance)
				{
					FTransform Transform;
					if (Mesh->GetInstanceTransform(Instance, Transform, /*bWorldSpace*/ true))
					{
						Count.Instances += IsWithin(Transform.GetLocation(), At, Radius);
					}
				}
			}
		}
		return Count;
	}

	FDecorCount ClearNear(UWorld& World, const FVector2D& At, const double Radius, const bool bSources)
	{
		FDecorCount Taken;
		TArray<AActor*> Grove;
		TArray<TPair<UInstancedStaticMeshComponent*, TArray<int32>>> Scenery;
		for (TActorIterator<AActor> It(&World); It; ++It)
		{
			AActor* Actor = *It;
			if (const AStaticMeshActor* Shape = Cast<AStaticMeshActor>(Actor))
			{
				if (IsGroveActor(*Shape) && IsWithin(Shape->GetActorLocation(), At, Radius))
				{
					Grove.Add(Actor);
				}
			}
			if (Actor->IsA<AHOLSourceField>())
			{
				continue;
			}
			TArray<UInstancedStaticMeshComponent*> Meshes;
			Actor->GetComponents(Meshes);
			for (UInstancedStaticMeshComponent* Mesh : Meshes)
			{
				TArray<int32> Indices;
				for (int32 Instance = 0; Instance < Mesh->GetInstanceCount(); ++Instance)
				{
					FTransform Transform;
					if (Mesh->GetInstanceTransform(Instance, Transform, /*bWorldSpace*/ true) && IsWithin(Transform.GetLocation(), At, Radius))
					{
						Indices.Add(Instance);
					}
				}
				if (!Indices.IsEmpty())
				{
					Scenery.Emplace(Mesh, MoveTemp(Indices));
				}
			}
		}
		// Taken away once the walk is over (nothing changes under the iterators); instances from the last index to the first.
		for (AActor* Actor : Grove)
		{
			Actor->Destroy();
			++Taken.Actors;
		}
		for (TPair<UInstancedStaticMeshComponent*, TArray<int32>>& Entry : Scenery)
		{
			for (int32 Slot = Entry.Value.Num() - 1; Slot >= 0; --Slot)
			{
				Taken.Instances += Entry.Key->RemoveInstance(Entry.Value[Slot]);
			}
		}
		// The trees of the game: the registry takes them off the map (display, navmesh, sight), like a used-up source. Server or solo only.
		if (bSources)
		{
			if (UHOLSourceSubsystem* Registry = World.GetSubsystem<UHOLSourceSubsystem>())
			{
				for (const int32 Index : WoodSourcesWithin(World, At, Radius))
				{
					Taken.Sources += Registry->RemoveSource(Index) ? 1 : 0;
				}
			}
		}
		return Taken;
	}

	namespace
	{
		// hol.Decor.ClearNear X Y [R] [Trees]: the scenery around a ground point goes (the wood sources too with Trees), on every game world of the process.
		// The radius is the one the scenario passes: the C++ cannot call the script rule HOLCampClearing::RadiusCm (2500 cm), so the scenario that clears a camp passes that same number.
		FAutoConsoleCommand CommandClearNear(TEXT("hol.Decor.ClearNear"),
			TEXT("hol.Decor.ClearNear X Y [R] [Trees]: takes away the scenery (Plain's grove trunks and crowns, the scenery instances) within R cm (2000) of a ground point (cm), to see what stands there (scenario neutrals); the optional word Trees (or Sources) takes the standing WOOD sources of the radius too (the game's trees, as the camp clearing rule HOLCampClearing asks), the other resource sources stay"),
			FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
			{
				if (Args.Num() < 2)
				{
					UE_LOG(LogHOLMatch, Warning, TEXT("Decor: usage hol.Decor.ClearNear X Y [R]"));
					return;
				}
				const FVector2D At(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]));
				double Radius = 2000.0;
				bool bSources = false;
				for (int32 Index = 2; Index < Args.Num(); ++Index)
				{
					if (Args[Index].Equals(TEXT("Trees"), ESearchCase::IgnoreCase) || Args[Index].Equals(TEXT("Sources"), ESearchCase::IgnoreCase))
					{
						bSources = true;
					}
					else if (Index == 2)
					{
						Radius = FMath::Max(0.0, FCString::Atod(*Args[Index]));
					}
				}
				for (const FWorldContext& Context : GEngine->GetWorldContexts())
				{
					if (UWorld* World = Context.World(); World && World->IsGameWorld())
					{
						const FDecorCount Taken = ClearNear(*World, At, Radius, bSources);
						const FDecorCount Left = CountNear(*World, At, Radius, bSources);
						UE_LOG(LogHOLMatch, Display, TEXT("Decor.ClearNear %.0f %.0f %.0f: took %d grove actors, %d scenery instances and %d wood sources; %d left within the radius"),
							At.X, At.Y, Radius, Taken.Actors, Taken.Instances, Taken.Sources, Left.Total());
					}
				}
			}));
	}
}
