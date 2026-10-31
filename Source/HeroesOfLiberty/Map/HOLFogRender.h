// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Map/HOLVisionRules.h"
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
 * 1 draws it, 0 does not.
 *
 * THE MINIMAP'S READER (F8 / F9, decision of 2026-10-30: one picture for the screen and the minimap). The picture is kept whether or
 * not the post process is drawn, as long as the viewer has a camp under a fog (mode Explored or Normal). To draw it on a minimap:
 *  - GetTexture() is the picture itself (G8, Grid.Side x Grid.Side, bilinear, clamped); texel (X, Y) is the cell (X, Y), so U runs
 *    along the world's +X and V along +Y, (0, 0) at the grid's lower corner (-X, -Y); ToPictureUV(world point) gives the place.
 *    Texel values: 0 unexplored, 115 explored, 255 in sight (HOLFogPicture); a material can multiply a flat map by it. It is null
 *    until the first refresh and while there is no fog (IsFogActive() false: the minimap draws everything).
 *  - GetCellVision(world point) reads one point (Visible without fog), for a dot or an alert: "is this unit shown?" is
 *    UHOLVisionSubsystem::IsUnitShown, not this.
 *  - GetVersion() changes whenever the picture changes (at most 10 times a second) and OnPictureChanged fires then: repaint only
 *    on it. Both read on the game thread.
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

	/** The fog of the viewer's camp, null before the first refresh (or without fog): see the minimap's reader above. */
	const UTexture2D* GetTexture() const { return Texture; }

	/** A fog is in force for this machine's viewer: a camp under mode Explored or Normal; false: everything is in sight. */
	bool IsFogActive() const { return bHasPicture; }

	/** The grid the picture covers (cells of 4 m, centred on the world's origin); Side 0 until the first refresh. */
	const FHOLVisionGrid& GetGrid() const { return Grid; }

	/** The picture's bytes, one a cell (index Y * Side + X); empty without fog. */
	TConstArrayView<uint8> GetBytes() const { return Cells; }

	/** What the viewer's camp knows of the ground under a point (X, Y in cm); Visible without fog. */
	EHOLCellVision GetCellVision(const FVector2D& Point) const;

	/** A point's place in the picture, (0, 0) at the lower corner, (1, 1) at the upper (out of [0, 1] outside the grid). */
	FVector2D ToPictureUV(const FVector2D& Point) const { return HOLFogPicture::ToUV(Grid, Point); }

	/** Changes whenever the picture does (a refresh that changed a cell, the fog starting or ending). */
	uint32 GetVersion() const { return Version; }

	/** Fired with the new version: repaint a minimap then, not every frame. */
	FSimpleMulticastDelegate OnPictureChanged;

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

	FHOLVisionGrid Grid;
	TArray<uint8> Cells;
	bool bHasPicture = false;
	uint32 Version = 0;
	int32 NumVisible = 0;
	int32 NumExplored = 0;
	int32 AppliedMode = 0;
	float Seconds = 0.f;
};
