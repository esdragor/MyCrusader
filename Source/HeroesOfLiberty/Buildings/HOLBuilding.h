// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HOLBuilding.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;
struct FHOLBuildingTypeRow;

/**
 * A building (D260): a classic replicated actor (§ 16.4, buildings are few), made by the server. It carries its type
 * (a row of the building type table), its owner (the player's slot, a byte like the units' OwnerPlayer, D233) and its
 * owner's colour; every machine builds its look from the type, so only these three values travel. GAS comes with
 * the first ability or damage (M5). Selection is the local player's own (D262): its pieces block the selection trace
 * channel; nothing on the building shows it (no ring, no hover: the user's test of J2).
 */
UCLASS()
class HEROESOFLIBERTY_API AHOLBuilding : public AActor
{
	GENERATED_BODY()

public:
	AHOLBuilding();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Server only, between a deferred spawn and its FinishSpawning (or right after a spawn). */
	void Setup(FName InTypeName, uint8 InOwnerPlayer, uint8 InColorIndex);

	FName GetTypeName() const { return TypeName; }
	uint8 GetOwnerPlayer() const { return OwnerPlayer; }
	uint8 GetColorIndex() const { return ColorIndex; }

	/** Values of the provisional shape (D261), as fractions of the footprint and the height. */
	static constexpr float BaseHeightRatio = 0.45f;
	static constexpr float TowerSizeRatio = 0.4f;
	static constexpr float RoofSizeRatio = 0.5f;
	static constexpr float CapThicknessRatio = 0.04f;
	/** The top of the block is a darker tone of the owner's colour, so that the tower's roof stands out on it. */
	static constexpr float BaseTopTone = 0.55f;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnRep_Setup();

	/** Builds the look from the type, then paints the owner's colour. Safe to call again. */
	void RefreshAppearance();
	/** Writes the owner's colour on the pieces that wear it. */
	void ApplyColors();
	void BuildProvisionalShape(const FHOLBuildingTypeRow& Type);
	void BuildFinalMesh(const FHOLBuildingTypeRow& Type);
	/** A piece of Size (cm) laid on BottomZ, painted Color, or the owner's colour times OwnerTone when bOwnerColor. */
	UStaticMeshComponent* AddPiece(UStaticMesh* Mesh, const FVector& Size, float BottomZ, const FLinearColor& Color, bool bOwnerColor,
		float OwnerTone = 1.f);

	UPROPERTY(ReplicatedUsing = OnRep_Setup)
	FName TypeName;

	UPROPERTY(ReplicatedUsing = OnRep_Setup)
	uint8 OwnerPlayer = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Setup)
	uint8 ColorIndex = 0;

	/** The type the pieces were built for: the look is rebuilt only when it changes. */
	FName BuiltTypeName;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> Pieces;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	/** Matte, lit (D265). */
	UPROPERTY()
	TObjectPtr<UMaterialInterface> PieceMaterial;

	/** Unlit, for the owner's colour (D261). */
	UPROPERTY()
	TObjectPtr<UMaterialInterface> OwnerPieceMaterial;

	/** Materials painted with the owner's colour, and the tone of each. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> OwnerMaterials;

	TArray<float> OwnerTones;
};
