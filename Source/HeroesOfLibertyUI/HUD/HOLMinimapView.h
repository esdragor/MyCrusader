// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "HOLMinimapView.generated.h"

class UTexture2D;
struct FHOLSource;

namespace HOLMinimapTerrain
{
	/**
	 * What the sources add to the terrain layer, in one number: which are standing, and where. It changes when a source is used up,
	 * added or elsewhere, not when one is only gathered from (an amount going down changes nothing of the picture). With the fog's
	 * version and the frame, it tells UHOLMinimapView::RefreshTerrain whether the 480 x 480 layer is to be made again.
	 */
	HEROESOFLIBERTYUI_API uint32 GetSourcesSignature(TConstArrayView<FHOLSource> Sources);
}

/** One of the things the minimap shows, read from the game (UHOLMinimapView::GatherEntities); the script turns it into a mark. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTYUI_API FHOLMinimapEntity
{
	GENERATED_BODY()

	/** A unit's identifier, or a building's (high bit set): what two readings compare. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "Minimap")
	int32 Id = 0;
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "Minimap")
	bool bBuilding = false;
	/** The player's own, not an ally's or an enemy's. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "Minimap")
	bool bOwn = false;
	/** The owner's slot. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "Minimap")
	int32 Slot = INDEX_NONE;
	/** Ground X and Y (cm). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "Minimap")
	FVector2D Location = FVector2D::ZeroVector;
	/** The owner's colour (HOLPlayerColors), grey for a neutral. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "Minimap")
	FLinearColor Color = FLinearColor::Gray;
	/** Health points (a site's grow with its work). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "Minimap")
	int32 Health = 0;
};

/** A dot of the picture, in pixels, as the script lays it out. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTYUI_API FHOLMinimapMark
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, ScriptReadWrite, Category = "Minimap")
	FVector2D Position = FVector2D::ZeroVector;
	UPROPERTY(BlueprintReadWrite, ScriptReadWrite, Category = "Minimap")
	FLinearColor Color = FLinearColor::White;
	/** Side of the square, px. */
	UPROPERTY(BlueprintReadWrite, ScriptReadWrite, Category = "Minimap")
	float Size = 3.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FHOLMinimapPressed, FVector2D, Position, bool, bRight, bool, bDrag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHOLMinimapTicked);

/**
 * The prototype's minimap (InterfaceInProgress.md, decision of 2026-10-30, in the manner of AoE4): a square picture, bottom right,
 * painted from what the script lays out. The RULES are in script (Script/UI/HOLMinimapRules.as: picture <-> ground, what a click
 * does, the alerts); this widget only (1) reads the game for the script (GatherEntities, GetMapArea, GetCameraGround,
 * HasOrderableSelection), (2) acts for it (FocusCamera, OrderMove), (3) paints what the script hands it (marks, alert points, the
 * camera's frame) over a terrain layer it builds itself (RefreshTerrain: the explored sources and the fog of the viewer's camp), and
 * (4) tells the script where it was pressed (OnPressed, in pixels).
 *
 * THE FOG. The terrain layer reads the picture of UHOLFogRenderSubsystem (FogInProgress.md section 10, GetCellVision: the same picture that
 * darkens the screen) cell by cell a couple of times a second: black where unexplored, dim where explored, clear in sight; sources only
 * where explored. ReadVision (cpp) is the one place that reads it. Units of the others come through UHOLVisionSubsystem::IsUnitShown.
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLMinimapView : public UWidget
{
	GENERATED_BODY()

public:
	/** Side of the picture, px (Slate units). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Minimap")
	void SetSide(float InSide);

	/** The ground the picture covers: the world X and Y of its lower left and upper right (cm); the terrain layer follows it. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Minimap")
	void SetFrame(FVector2D InMin, FVector2D InMax);

	/** What is painted over the terrain: dots (pixels) and the lit alerts' places (pixels). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Minimap")
	void SetLayers(const TArray<FHOLMinimapMark>& InMarks, const TArray<FVector2D>& InAlerts);

	/** The camera's frame (its four corners in pixels), set every frame (OnTicked): it follows the camera at once. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Minimap")
	void SetCameraFrame(const TArray<FVector2D>& InCameraFrame);

	/** The ground point the camera looks at (X, Y cm). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Minimap")
	FVector2D GetCameraFocus() const;

	/** The terrain layer made again from the fog render's picture (the argument is the local slot, not used: all in sight without a fog). 480 x 480 texels (the fog's cells blurred once, then blended between their centres: soft edges), twice a second, and only if the fog's picture, the frame or the standing sources changed since the last time (P2). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Minimap")
	void RefreshTerrain(int32 ViewerSlot);

	/** The units and buildings ViewerSlot's player may see (units of the others only in sight, buildings of the others on explored ground). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Minimap")
	void GatherEntities(int32 ViewerSlot, TArray<FHOLMinimapEntity>& OutEntities) const;

	/** The map's playable area (AHOLMapBounds): world X and Y of its lower left and upper right; false when the map has none. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Minimap")
	bool GetMapArea(FVector2D& OutMin, FVector2D& OutMax) const;

	/** The ground the camera sees: its four screen corners (upper left, upper right, lower right, lower left) on the ground (X, Y cm). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Minimap")
	bool GetCameraGround(TArray<FVector2D>& OutCorners) const;

	/** The camera looks at that ground point (kept inside the map by the camera). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Minimap")
	void FocusCamera(FVector2D World);

	/** The player has own units to order (the active tab's, as a right click): a right click on the minimap sends them. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Minimap")
	bool HasOrderableSelection() const;

	/** The selected own units (the active tab's) walk to that ground point; false when there are none. Logs "Minimap order: N units to (x, y)". */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Minimap")
	bool OrderMove(FVector2D World);

	/** hol.Minimap.AlertSeconds: how long an alert blinks, seconds (not yet made sane: HOLMinimap::SaneLifetime does it in the script). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Minimap")
	float GetAlertSeconds() const;

	/** hol.Minimap.AlertBlink: the half-period of the blink, seconds (lit that long, dark that long). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Minimap")
	float GetAlertBlink() const;
	/** One line of what the picture holds, for hol.HUD.Report: "minimap: 240 px, 1380 marks, 2 alerts, terrain 220 x 220 (sources 640, in sight 3100)". */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Minimap")
	FString Describe() const;

	/** Where it was pressed (pixels from its upper left), with which button, and whether the button was already held (a drag). */
	UPROPERTY(BlueprintAssignable, Category = "HeroesOfLiberty|Minimap")
	FHOLMinimapPressed OnPressed;

	/** Every frame the picture is drawn: the script moves the camera's frame (cheap work only). */
	UPROPERTY(BlueprintAssignable, Category = "HeroesOfLiberty|Minimap")
	FHOLMinimapTicked OnTicked;

	// UWidget
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	/** Called by the Slate widget. */
	void BroadcastPressed(const FVector2D& Position, bool bRight, bool bDrag);
	void BroadcastTicked() { OnTicked.Broadcast(); }

	const TArray<FHOLMinimapMark>& GetMarks() const { return Marks; }
	const TArray<FVector2D>& GetAlerts() const { return Alerts; }
	const TArray<FVector2D>& GetCameraFrame() const { return CameraFrame; }
	UTexture2D* GetTerrain() const { return Terrain; }
	float GetSide() const { return Side; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	TSharedPtr<SWidget> Slate;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> Terrain;

	TArray<FHOLMinimapMark> Marks;
	TArray<FVector2D> Alerts;
	TArray<FVector2D> CameraFrame;
	float Side = 240.f;
	FVector2D FrameMin = FVector2D(-44000.0, -44000.0);
	FVector2D FrameMax = FVector2D(44000.0, 44000.0);
	int32 NumSourcesDrawn = 0;
	int32 NumInSight = 0;

	/** What the terrain layer was last made from (P2): the fog picture's version, the frame, the standing sources. */
	struct FTerrainKey
	{
		uint32 FogVersion = 0;
		bool bFogActive = false;
		FVector2D Min = FVector2D::ZeroVector;
		FVector2D Max = FVector2D::ZeroVector;
		uint32 Sources = 0;

		bool operator==(const FTerrainKey& Other) const
		{
			return FogVersion == Other.FogVersion && bFogActive == Other.bFogActive && Min == Other.Min && Max == Other.Max && Sources == Other.Sources;
		}
	};
	FTerrainKey BuiltKey;
	bool bTerrainBuilt = false;
	/** Times the layer was made (not counting the readings that found it up to date). */
	int32 TerrainBuilds = 0;
};
