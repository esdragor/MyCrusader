// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Match/HOLForumMessages.h"
#include "Simulation/HOLUnitView.h"
#include "HOLSelectionMarkers.generated.h"

class AHOLPlayerController;
class UInstancedStaticMeshComponent;
class UMaterialInstanceDynamic;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * What the ground shows of the local player's selection (J4, D288), on the player's machine only (never replicated,
 * like the RTS camera): a ring under each selected unit or around the selected resource source (J5b), and a square
 * frame under the selected building (under each of a group, an F key, D30), a neutral
 * white; a brief ring where the player gave an order; the rally flag of the player's own selected building, at its
 * colour (of each building of a group's active subgroup that has one, all of them for « Tous », the user's 3rd trial
 * of J6a.5); in placement mode (J6a.3, D311), the ghost of the building: its footprint on the ground, tile by tile, and the
 * edges of its volume, green where it may go, red where not.
 * Above the units (J7b.5, J7-Q5 and the user's nuance), their health bars: always for a selected unit, otherwise only for
 * a wounded one, the player's own or another's; none for a dying unit.
 * Between the queued orders (Shift, D328) of the selected units, lines on the ground, step by step (J7b.5b-2).
 * Unlit masked planes (Tools/Editor/make_marker_materials.py), instanced, with no collision and no shadow.
 */
UCLASS(NotPlaceable, Transient)
class HEROESOFLIBERTY_API AHOLSelectionMarkers : public AActor
{
	GENERATED_BODY()

public:
	AHOLSelectionMarkers();

	/** Follows that controller's selection and orders. */
	void Init(AHOLPlayerController* InController);

	virtual void Tick(float DeltaSeconds) override;

	/**
	 * "rings 3, frame 1, flag 1 at (-16800, -1500), ghost valid": what the ground shows, for the scripted checks
	 * (hol.Select.Report); each flag shown is followed by its foot.
	 */
	FString Describe() const;

	/**
	 * "bars 3 (shown #12 41/155 selected, ...), unhurt unselected 40": the health bars shown, for the scripted checks
	 * (hol.Units.Bars), and how many units stand unhurt and unselected (without one).
	 */
	FString DescribeHealthBars() const;

	/**
	 * Whether a unit shows its health bar (J7-Q5 and the user's nuance, D.1): always when selected, whatever its side (an
	 * enemy group selected shows its bars, full or not), otherwise only when wounded.
	 */
	static bool ShowsHealthBar(bool bSelected, bool bFullHealth);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleOrderGiven(const FVector& Point);
	void HandleAttackOrderGiven(const FVector& Point);
	void UpdateRings();
	void UpdateFrame();
	void UpdateFlag();
	void UpdateGhost();
	void UpdateHealthBars();
	void UpdateOrderLines();
	/** V3.3 (V3-Q5): the hold's ring around each main forum down (HOLForumRules::GetRing), from the replicated state. */
	void UpdateForumRings();
	/**
	 * J8c.4 (J8c-Q4): the player's own auras on the ground, to him alone: a light golden circle of 12 m around his living
	 * hero and around his standing banner; while the banner is aimed, the 16 m of its reach around the hero and its aura
	 * under the cursor (golden within reach, red beyond). Circles of strips, each at the ground's height (hills).
	 */
	void UpdateAuraRings();
	/** A pole and its cloth more for the flags (one per building with a rally point), made the first time needed. */
	void AddFlag();

	/**
	 * V3.3 (V3-Q5): the rings of the forums down, to everyone (D353, no fog yet): strips on the ground along the zone's
	 * outline (HOLForumRules::GetZoneOutlinePoint), the hold's arc thick in its colour, the rest a thin dark line.
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> ForumRings;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> Rings;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> Frames;

	/** The health bars (J7b.5): two planes each facing the camera, the dark back and the coloured fill. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> HealthBars;

	/**
	 * The lines between queued orders (D328, J7b.5b-2): for each chain with a selected unit, a filled strip on the ground from
	 * the chain's units' centre to its first step, then step to step, coloured by the step it reaches; and a disc on each step.
	 * Also, white, from each selected building's edge to its rally flag (the flags' set, the user's trial of D328).
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> OrderLines;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> OrderPoints;

	/** The rally flags, a pole and a cloth each, as many as ever shown together (hidden beyond the number shown). */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> FlagPoles;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> FlagCloths;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> PoleMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ClothMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> CubeMesh;

	/** The placement ghost (J6a.3): its outline and tiles on the ground, and the edges of its volume (4 posts, 4 beams). */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> GhostFrames;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UStaticMeshComponent>> GhostEdges;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> GhostMaterial;

	/** What the ghost shows: none, valid (green) or refused (red). */
	enum class EGhost : uint8 { None, Valid, Refused };
	EGhost ShownGhost = EGhost::None;
	/** Frames of a group drawn thinner (out of its active subgroup, Tab), for Describe. */
	int32 DimmedFrames = 0;
	/** The feet of the flags shown, for Describe. */
	TArray<FVector> FlagFeet;

	/** The units as drawn this frame (D271), gathered once for the rings and the health bars. */
	TArray<FHOLUnitView> Views;
	/** The bars shown, for DescribeHealthBars: the unit, its health of its maximum (J7-Q5 revised), whether it is selected. */
	struct FShownBar
	{
		uint32 UnitId = 0;
		int32 Health = 0;
		int32 MaxHealth = 0;
		bool bSelected = false;
	};
	TArray<FShownBar> ShownBars;
	int32 UnhurtUnselected = 0;

	TWeakObjectPtr<AHOLPlayerController> Controller;
	FDelegateHandle OrderHandle;
	FDelegateHandle AttackOrderHandle;
	/** The last order's ping is an attack's (J7b.3): red. */
	bool bAttackPing = false;

	/** The last order's point, and the seconds since it was given (the ring shows while under PingSeconds). */
	FVector PingPoint = FVector::ZeroVector;
	float PingAge = TNumericLimits<float>::Max();

	/**
	 * J8b.4 (J8b-Q4): the heroes gone up, as this machine sees their public sheets (every player, spectators too): a slot's
	 * golden flash on its hero, the seconds since it started (HeroFlashSeconds).
	 */
	void UpdateHeroFlashes(float DeltaSeconds);
	FHOLHeroLevelWatch HeroLevelWatch;
	TMap<int32, float> HeroFlashes;
	/** The flash's rings drawn this frame (not counted as rings in Describe). */
	int32 HeroFlashRings = 0;

	UPROPERTY(Transient)
	TObjectPtr<UInstancedStaticMeshComponent> AuraRings;
	/** "hero (x, y) 1200, banner (x, y) 1200, reach 1600, aim (x, y) ok" or "none": what UpdateAuraRings drew last. */
	FString AuraRingsDescription = TEXT("none");

public:
	/** The player's aura circles drawn (hol.Select.Report's "Aura rings:" line). */
	const FString& DescribeAuraRings() const { return AuraRingsDescription; }
};
