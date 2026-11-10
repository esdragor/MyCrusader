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
		for (TActorIterator<AActor> It(&World); It; ++It)
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
		return FDecorCount();
	}
}
