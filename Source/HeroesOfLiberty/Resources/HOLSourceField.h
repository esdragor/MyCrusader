// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Match/HOLResources.h"
#include "HOLSourceField.generated.h"

class UHOLSourceNavComponent;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class UStaticMesh;
struct FHOLSourceTypeRow;

/**
 * The resource sources of the match as this machine shows them (D297), spawned by UHOLSourceSubsystem on each machine,
 * never replicated: one instanced mesh component per piece of each type (a tree is a trunk and a cone: two components
 * for all the trees, a handful of draw calls in all), and on the server or in a solo game the navigation's forbidden
 * areas (UHOLSourceNavComponent). A used-up source is not removed from the instances (their indices would move): its
 * instances are shrunk to nothing.
 */
UCLASS(NotPlaceable, Transient)
class HEROESOFLIBERTY_API AHOLSourceField : public AActor
{
	GENERATED_BODY()

public:
	AHOLSourceField();

	/** bDisplay: draw the sources (not on a dedicated server); bNavigation: forbid them on the navmesh (server, solo). */
	void Init(bool bDisplay, bool bNavigation);

	/** A source enters the map (or reaches this client). */
	void ShowSource(int32 SourceIndex, uint8 TypeIndex, const FHOLSourceTypeRow& Type, const FVector& Location);

	/** The source is used up: it leaves the map. */
	void HideSource(int32 SourceIndex);

	/** The first sources are placed: the navigation takes them all at once (server, solo). */
	void RegisterNavigation();

	int32 GetNumShown() const { return NumShown; }
	int32 GetNumNavCylinders() const;

	/** Half the width (cm) a source of this type is drawn with (a tree's crown is wider than its trunk): the selection aims there. */
	static float GetDrawnRadius(const FHOLSourceTypeRow& Type);

private:
	/** A piece of a provisional shape (D297), in the source's measures: XY by its radius, Z by its height. */
	struct FPiece
	{
		UStaticMesh* Mesh = nullptr;
		FVector2D Offset = FVector2D::ZeroVector;
		FVector Size = FVector::OneVector;
		float BottomZ = 0.f;
		float Yaw = 0.f;
		FLinearColor Color = FLinearColor::White;
	};

	/** The instance components of one source type, one per piece; a source has the same instance index in each. */
	struct FTypeDisplay
	{
		TArray<TObjectPtr<UInstancedStaticMeshComponent>> Components;
		/** Each piece's transform in the source's frame, for a source of scale 1. */
		TArray<FTransform> PieceTransforms;
	};

	/** The provisional pieces of a resource's sources (trunk and cone, bush and berries, blocks). */
	TArray<FPiece> MakeProvisionalPieces(EHOLResource Resource, const FHOLSourceTypeRow& Type) const;
	FTypeDisplay& GetOrMakeTypeDisplay(uint8 TypeIndex, const FHOLSourceTypeRow& Type);
	UInstancedStaticMeshComponent* MakeComponent(UStaticMesh* Mesh, const FLinearColor* Color);

	bool bDisplay = false;

	TMap<uint8, FTypeDisplay> TypeDisplays;

	/** Per source: its type and instance index, INDEX_NONE until it is shown. */
	TArray<uint8> TypeOfSource;
	TArray<int32> InstanceOfSource;
	int32 NumShown = 0;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> AllComponents;

	UPROPERTY(Transient)
	TObjectPtr<UHOLSourceNavComponent> Navigation;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CylinderMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> ConeMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> SphereMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	/** Matte, lit (D265), painted by its "Color" parameter. */
	UPROPERTY()
	TObjectPtr<UMaterialInterface> MatteMaterial;
};
