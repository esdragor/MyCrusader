// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLFogRender.generated.h"

class AActor;
class APostProcessVolume;
class UMaterialInstanceDynamic;
class UTexture2D;

/**
 * F7 (FogInProgress.md, F-Q3, D381, D382): the fog drawn on the local player's screen. A texture of the grid (one byte a cell: 0
 * unexplored, about 0.45 explored, 1 in sight) of the viewer's camp, refreshed ten times a second from UHOLVisionSubsystem, laid
 * on the scene by a post process (D562: everything the camera sees, by the world position rebuilt from the depth); hol.Fog.Render
 * 1 draws it, 0 does not. The minimap will read the same texture.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLFogRenderSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** The fog of the viewer's camp, null before the first refresh (or without fog). */
	const UTexture2D* GetTexture() const { return Texture; }

	/** 0 none, 1 post process: what is laid on the scene now. */
	int32 GetAppliedMode() const { return AppliedMode; }

	/** "Fog render: mode 1 (post process), texture 220 x 220, cells in sight 812 explored 1650". */
	FString MakeReport() const;

private:
	/** The camp whose fog this machine shows (a client's own; a solo or host game's local player's); INDEX_NONE if none. */
	int32 GetViewerCamp() const;
	void RefreshTexture(int32 Camp);
	void Apply();
	void Remove();
	void SendCamera();

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> Texture;
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> Material;
	UPROPERTY(Transient)
	TObjectPtr<APostProcessVolume> Volume;

	TArray<uint8> Cells;
	int32 NumVisible = 0;
	int32 NumExplored = 0;
	int32 AppliedMode = 0;
	float Seconds = 0.f;
};
