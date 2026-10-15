// Copyright Epic Games, Inc. All Rights Reserved.

#include "Resources/HOLSourceNavComponent.h"

#include "AI/Navigation/NavigationRelevantData.h"
#include "AI/NavigationModifier.h"
#include "AI/NavigationSystemBase.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "NavAreas/NavArea_Null.h"

namespace UE::HeroesOfLiberty::Resources
{
	/** The cylinder starts under the ground and stands above any unit: the navmesh's surface is always inside it. */
	constexpr double CylinderBottom = -100.;
	constexpr double CylinderHeight = 400.;

	/** Margin (cm) of the dirtied box around a cylinder: the navmesh widens a forbidden area by the agent's radius. */
	constexpr double DirtyMargin = 100.;
}

UHOLSourceNavComponent::UHOLSourceNavComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Its own navigation element, not merged into the owner's root component: the dirty areas name this component.
	bAttachToOwnersRoot = false;
}

void UHOLSourceNavComponent::RegisterWithNavigation()
{
	if (!IsRegistered())
	{
		RegisterComponent();
	}
	TArray<FBox> Areas;
	{
		FRWScopeLock Lock(CylindersLock, SLT_ReadOnly);
		for (const TPair<int32, FTransform>& Cylinder : Cylinders)
		{
			Areas.Add(GetCylinderBox(Cylinder.Value));
		}
	}
	NotifyChanged(Areas);
}

void UHOLSourceNavComponent::AddSource(const int32 SourceIndex, const FVector& Location, const float Radius)
{
	using namespace UE::HeroesOfLiberty::Resources;
	const FTransform Cylinder(FQuat::Identity, Location + FVector(0., 0., CylinderBottom), FVector(Radius, Radius, CylinderHeight));
	{
		FRWScopeLock Lock(CylindersLock, SLT_Write);
		Cylinders.Add(SourceIndex, Cylinder);
	}
	const FBox Areas[] = { GetCylinderBox(Cylinder) };
	NotifyChanged(Areas);
}

void UHOLSourceNavComponent::RemoveSource(const int32 SourceIndex)
{
	FTransform Cylinder;
	{
		FRWScopeLock Lock(CylindersLock, SLT_Write);
		if (!Cylinders.RemoveAndCopyValue(SourceIndex, Cylinder))
		{
			return;
		}
	}
	const FBox Areas[] = { GetCylinderBox(Cylinder) };
	NotifyChanged(Areas);
}

int32 UHOLSourceNavComponent::GetNumCylinders() const
{
	FRWScopeLock Lock(CylindersLock, SLT_ReadOnly);
	return Cylinders.Num();
}

void UHOLSourceNavComponent::GetNavigationData(FNavigationRelevantData& Data) const
{
	// One unit cylinder, forbidden, placed and scaled per source by the transforms the generator asks for each tile.
	Data.Modifiers.MarkAsPerInstanceModifier();
	Data.Modifiers.Add(FAreaNavModifier(1.f, 1.f, FTransform::Identity, UNavArea_Null::StaticClass()));
	Data.NavDataPerInstanceTransformDelegate = FNavDataPerInstanceTransformDelegate::CreateUObject(this, &UHOLSourceNavComponent::GetPerInstanceTransforms);
}

void UHOLSourceNavComponent::GetPerInstanceTransforms(const FBox& AreaBox, TArray<FTransform>& OutTransforms) const
{
	FRWScopeLock Lock(CylindersLock, SLT_ReadOnly);
	for (const TPair<int32, FTransform>& Cylinder : Cylinders)
	{
		if (AreaBox.Intersect(GetCylinderBox(Cylinder.Value)))
		{
			OutTransforms.Add(Cylinder.Value);
		}
	}
}

void UHOLSourceNavComponent::CalcAndCacheBounds() const
{
	FBox NewBounds(ForceInit);
	{
		FRWScopeLock Lock(CylindersLock, SLT_ReadOnly);
		for (const TPair<int32, FTransform>& Cylinder : Cylinders)
		{
			NewBounds += GetCylinderBox(Cylinder.Value);
		}
	}
	Bounds = NewBounds;
	bBoundsInitialized = true;
}

FBox UHOLSourceNavComponent::GetCylinderBox(const FTransform& Cylinder)
{
	using namespace UE::HeroesOfLiberty::Resources;
	const FVector Scale = Cylinder.GetScale3D();
	const FVector Base = Cylinder.GetLocation();
	return FBox(Base - FVector(Scale.X, Scale.Y, 0.), Base + FVector(Scale.X, Scale.Y, Scale.Z)).ExpandBy(DirtyMargin);
}

void UHOLSourceNavComponent::NotifyChanged(const TConstArrayView<FBox> DirtyAreas)
{
	CalcAndCacheBounds();
	if (IsRegistered() && Bounds.IsValid && DirtyAreas.Num() > 0)
	{
		FNavigationSystem::OnObjectBoundsChanged(*this, Bounds, DirtyAreas);
		// The paths kept for the gathering's walks across these areas are forgotten (D305).
		if (UHOLCommandSubsystem* Commands = GetWorld() ? GetWorld()->GetSubsystem<UHOLCommandSubsystem>() : nullptr)
		{
			for (const FBox& Area : DirtyAreas)
			{
				Commands->NotifyNavigationChanged(Area);
			}
		}
	}
}
