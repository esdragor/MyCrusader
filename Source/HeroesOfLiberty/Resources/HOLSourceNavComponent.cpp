// Copyright Epic Games, Inc. All Rights Reserved.

#include "Resources/HOLSourceNavComponent.h"

#include "AI/Navigation/NavigationRelevantData.h"
#include "AI/NavigationModifier.h"
#include "AI/NavigationSystemBase.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "HeroesOfLibertyLog.h"
#include "NavAreas/NavArea_Null.h"

namespace UE::HeroesOfLiberty::Resources
{
	/** The cylinder starts under the ground and stands above any unit: the navmesh's surface is always inside it. */
	constexpr double CylinderBottom = -100.;
	constexpr double CylinderHeight = 400.;

	/** Margin (cm) of the dirtied box around a cylinder: the navmesh widens a forbidden area by the agent's radius. */
	constexpr double DirtyMargin = 100.;

	/** Side (cm) of the cells of the ground whose cylinders are dirtied as one box at registration. */
	constexpr double RegistrationCell = 2000.;
}

UHOLSourceNavComponent::UHOLSourceNavComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Its own navigation element, not merged into the owner's root component: the dirty areas name this component.
	bAttachToOwnersRoot = false;
	Bounds = FBox(ForceInit);
}

void UHOLSourceNavComponent::RegisterWithNavigation()
{
	if (!IsRegistered())
	{
		RegisterComponent();
	}
	TArray<FBox> Areas;
	GetRegistrationAreas(Areas);
	UE_LOG(LogHOLMatch, Log, TEXT("Source navigation: %d cylinders forbidden, %d areas dirtied, %lld cylinders visited by bounds scans"), GetNumCylinders(), Areas.Num(),
		NumBoundsVisits);
	NotifyChanged(Areas);
}

void UHOLSourceNavComponent::GetRegistrationAreas(TArray<FBox>& OutAreas) const
{
	using namespace UE::HeroesOfLiberty::Resources;
	OutAreas.Reset();
	TMap<FIntPoint, FBox> Cells;
	{
		FRWScopeLock Lock(CylindersLock, SLT_ReadOnly);
		for (const TPair<int32, FTransform>& Cylinder : Cylinders)
		{
			const FVector Foot = Cylinder.Value.GetLocation();
			FBox& Cell = Cells.FindOrAdd(FIntPoint(FMath::FloorToInt32(Foot.X / RegistrationCell), FMath::FloorToInt32(Foot.Y / RegistrationCell)), FBox(ForceInit));
			Cell += GetCylinderBox(Cylinder.Value);
		}
	}
	OutAreas.Reserve(Cells.Num());
	for (const TPair<FIntPoint, FBox>& Cell : Cells)
	{
		OutAreas.Add(Cell.Value);
	}
}

FBox UHOLSourceNavComponent::GetCylindersBounds() const
{
	CalcAndCacheBounds();
	return Bounds;
}

void UHOLSourceNavComponent::AddSource(const int32 SourceIndex, const FVector& Location, const float Radius)
{
	using namespace UE::HeroesOfLiberty::Resources;
	const FTransform Cylinder(FQuat::Identity, Location + FVector(0., 0., CylinderBottom), FVector(Radius, Radius, CylinderHeight));
	const FBox Box = GetCylinderBox(Cylinder);
	{
		FRWScopeLock Lock(CylindersLock, SLT_Write);
		// A source put again elsewhere leaves its old box in the bounds, as a removal does.
		bBoundsStale |= Cylinders.Contains(SourceIndex);
		Cylinders.Add(SourceIndex, Cylinder);
		Bounds += Box;
	}
	const FBox Areas[] = { Box };
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
		// The bounds keep the box (they still hold every cylinder): they are rescanned if somebody asks (CalcAndCacheBounds).
		bBoundsStale = true;
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
	// Bounds follows the adds by itself (AddSource) and keeps the boxes of the cylinders removed since (a superset of the
	// union, harmless to the navigation: the generator filters per cylinder). The full scan to shrink it is made here, when
	// the navigation or a caller asks for exact bounds, never per add or per removal (a forest holds 143 000 cylinders).
	FRWScopeLock Lock(CylindersLock, SLT_Write);
	if (bBoundsStale)
	{
		FBox NewBounds(ForceInit);
		for (const TPair<int32, FTransform>& Cylinder : Cylinders)
		{
			NewBounds += GetCylinderBox(Cylinder.Value);
		}
		NumBoundsVisits += Cylinders.Num();
		Bounds = NewBounds;
		bBoundsStale = false;
	}
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
	// Nothing to tell before the registration: the navigation reads the bounds when the component registers. Bounds is
	// kept by AddSource: no scan of the cylinders here.
	if (!IsRegistered())
	{
		return;
	}
	if (Bounds.IsValid && DirtyAreas.Num() > 0)
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
