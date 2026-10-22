// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HOLRubble.generated.h"

class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * What a destroyed building leaves (D345.2a): rubble over its footprint for LifeSeconds, then nothing. Made by the server
 * (UHOLConstructionSubsystem::DestroyBuilding), replicated to everyone with the building's type and owner's colour; every
 * machine but a dedicated server builds its look from them: low, flattened pieces in dark and grey tones, a few of the
 * owner's colour. It blocks nothing (no navmesh area, no collision) and answers no click: not selectable, not a target.
 */
UCLASS()
class HEROESOFLIBERTY_API AHOLRubble : public AActor
{
	GENERATED_BODY()

public:
	AHOLRubble();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Server only, between a deferred spawn and its FinishSpawning: the destroyed building's type and owner's colour. */
	void Setup(FName InTypeName, uint8 InColorIndex);

	FName GetTypeName() const { return TypeName; }

	/** Seconds the rubble stays (D345.2a). */
	static constexpr float LifeSeconds = 10.f;
	/** Height of the flattened heap (cm) and of its pieces at most. */
	static constexpr float HeapHeight = 25.f;
	static constexpr float PieceMaxHeight = 70.f;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnRep_Setup();

	/** Builds the pieces once the type is known; never on a dedicated server. */
	void BuildLook();
	void AddPiece(const FVector& Size, const FVector2D& Offset, float Yaw, const FLinearColor& Color);

	UPROPERTY(ReplicatedUsing = OnRep_Setup)
	FName TypeName;

	UPROPERTY(ReplicatedUsing = OnRep_Setup)
	uint8 ColorIndex = 0;

	bool bBuilt = false;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> PieceMaterial;
};
