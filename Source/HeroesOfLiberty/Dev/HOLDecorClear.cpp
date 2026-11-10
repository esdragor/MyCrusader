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
	}

	FDecorCount CountNear(const UWorld& World, const FVector2D& At, const double Radius)
	{
		FDecorCount Count;
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

	FDecorCount ClearNear(UWorld& World, const FVector2D& At, const double Radius)
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
		return Taken;
	}

	namespace
	{
		// hol.Decor.ClearNear X Y [R]: the scenery (not the sources) around a ground point goes, on every game world of the process.
		FAutoConsoleCommand CommandClearNear(TEXT("hol.Decor.ClearNear"),
			TEXT("hol.Decor.ClearNear X Y [R]: takes away the scenery (Plain's grove trunks and crowns, the scenery instances; not the resource sources) within R cm (2000) of a ground point (cm), to see what stands there (scenario neutrals)"),
			FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
			{
				if (Args.Num() < 2)
				{
					UE_LOG(LogHOLMatch, Warning, TEXT("Decor: usage hol.Decor.ClearNear X Y [R]"));
					return;
				}
				const FVector2D At(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]));
				const double Radius = Args.Num() > 2 ? FMath::Max(0.0, FCString::Atod(*Args[2])) : 2000.0;
				for (const FWorldContext& Context : GEngine->GetWorldContexts())
				{
					if (UWorld* World = Context.World(); World && World->IsGameWorld())
					{
						const FDecorCount Taken = ClearNear(*World, At, Radius);
						const FDecorCount Left = CountNear(*World, At, Radius);
						UE_LOG(LogHOLMatch, Display, TEXT("Decor.ClearNear %.0f %.0f %.0f: took %d grove actors and %d scenery instances; %d left within the radius"),
							At.X, At.Y, Radius, Taken.Actors, Taken.Instances, Left.Total());
					}
				}
			}));
	}
}
