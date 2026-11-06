// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NavRelevantComponent.h"
#include "HOLSourceNavComponent.generated.h"

/**
 * The resource sources as forbidden areas of the navmesh (D297): a single navigation element for all of them, one
 * cylinder per source, given per instance as the instanced meshes do. The navmesh takes only modifiers at run time
 * (DynamicModifiersOnly, D286) and reads the cylinders of a tile when it rebuilds it, so adding a source or using one up
 * dirties that source's box alone: a forest recedes tree by tree, only the tiles under the felled tree are rebuilt.
 * Server and solo game only (a client has no navmesh).
 */
UCLASS(ClassGroup = Navigation)
class HEROESOFLIBERTY_API UHOLSourceNavComponent : public UNavRelevantComponent
{
	GENERATED_BODY()

public:
	UHOLSourceNavComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * Registers with the navigation once the first sources are added (empty bounds do not register) and dirties the boxes
	 * of all of them; the sources added later dirty their own box.
	 */
	void RegisterWithNavigation();

	/** Adds the cylinder of a source standing on Location (its ground point), of body Radius (cm). */
	void AddSource(int32 SourceIndex, const FVector& Location, float Radius);

	/** The source is used up: its cylinder goes, the tiles under it are rebuilt. */
	void RemoveSource(int32 SourceIndex);

	int32 GetNumCylinders() const;

	/** The box of all the cylinders (invalid with none), as the navigation is told. */
	FBox GetCylindersBounds() const;

	/** The cylinders visited by the full scans of CalcAndCacheBounds so far (a test and the load log read it). */
	int64 GetNumBoundsVisits() const { return NumBoundsVisits; }

	/**
	 * The boxes to dirty at registration: the cylinders' boxes merged per cell of the ground, not one per source (a forest
	 * of 143 000 trees would dirty 143 000 boxes, and the navigation and the paths kept would each hold them all).
	 */
	void GetRegistrationAreas(TArray<FBox>& OutAreas) const;

	//~ Begin INavRelevantInterface
	virtual void GetNavigationData(FNavigationRelevantData& Data) const override;
	virtual bool ShouldSkipDirtyAreaOnAddOrRemove() const override { return true; }
	//~ End INavRelevantInterface

	virtual void CalcAndCacheBounds() const override;

private:
	/** The cylinders of the tile being built (called by the navmesh's generator, possibly off the game thread). */
	void GetPerInstanceTransforms(const FBox& AreaBox, TArray<FTransform>& OutTransforms) const;

	/** The box a source's cylinder dirties. */
	static FBox GetCylinderBox(const FTransform& Cylinder);

	/** Tells the navigation the new bounds and the areas to rebuild (registered only). */
	void NotifyChanged(TConstArrayView<FBox> DirtyAreas);

	/** One cylinder per standing source (radius 1, height 1, scaled per source); the key is the source's index. */
	TMap<int32, FTransform> Cylinders;

	/**
	 * Bounds is kept as the cylinders come (a box added to it) and keeps the boxes of those that went (a superset of the
	 * union) until the next CalcAndCacheBounds, which scans once and shrinks it.
	 */
	mutable bool bBoundsStale = false;
	mutable int64 NumBoundsVisits = 0;

	/** The generator reads Cylinders while the game thread may change them. */
	mutable FRWLock CylindersLock;
};
