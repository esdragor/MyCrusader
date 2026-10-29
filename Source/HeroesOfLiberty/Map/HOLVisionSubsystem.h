// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Map/HOLVisionRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLVisionSubsystem.generated.h"

struct FHOLMatchConfig;

/** D522: the fog of a match (the menu offers the three). */
UENUM()
enum class EHOLFogMode : uint8
{
	/** Everything in sight (« Sans brouillard »): the trials, the measures, the tests' default. */
	Disabled,
	/** D389: the map explored from the start (terrain and resources), the enemy's units and buildings hidden out of sight. */
	Explored,
	/** D381: the map black at the start. The game's default (F8). */
	Normal,
};

/**
 * F3 (FogInProgress.md, D381-D383, D521): the fog of every camp of the match, on the server and in a solo game (a client
 * computes its own from what it receives, F6e). Its observers are the living units (UHOLUnitSubsystem) and, later (F3.2),
 * the buildings; a camp is a team, or a slot alone (HOLVictoryRules::GetCamp). Each fixed step, only the observers that
 * changed cell are counted again. The forest is the source registry's trees (FHOLVisionMap::AddBlocker).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLVisionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	/**
	 * The match starts (OnWorldBeginPlay, or a test): its camps from Config, the largest grid (D344) centred on the origin,
	 * the trees already standing, then every source told (UHOLSourceSubsystem::OnSourceStanding). Again: starts over.
	 */
	void Start(const FHOLMatchConfig& Config, EHOLFogMode InMode);

	/** One step (after the simulation's, UHOLSimulationSubsystem::GetOnStepEnded): the observers that moved, came or went. */
	void Update();

	EHOLFogMode GetMode() const { return Mode; }
	bool IsStarted() const { return bStarted; }

	/** The fog's camp of a slot (an index of GetMap), INDEX_NONE for a closed slot or none. */
	int32 GetCampIndex(int32 Slot) const;

	/** What Slot's camp knows of Location (Disabled: everything in sight; a slot with no camp: nothing). */
	EHOLCellVision GetVision(int32 Slot, const FVector& Location) const;
	bool IsVisible(int32 Slot, const FVector& Location) const { return GetVision(Slot, Location) == EHOLCellVision::Visible; }

	const FHOLVisionMap& GetMap() const { return Map; }

	/** "Fog: mode Normal, camps 2, observers 12, camp 0 (slots 0) visible 812 explored 1650, ...; step 0.08 ms". */
	FString MakeReport() const;

	static const TCHAR* GetModeName(EHOLFogMode InMode);

private:
	void OnSourceStanding(int32 SourceIndex, bool bStanding);
	void OnStepEnded(int64 StepIndex);
	double GetUnitSight(FName TypeName);

	struct FTracked
	{
		int32 Handle = INDEX_NONE;
		uint32 Stamp = 0;
		double Radius = 0.;
	};

	FHOLVisionMap Map;
	EHOLFogMode Mode = EHOLFogMode::Disabled;
	bool bStarted = false;
	/** Per slot: its camp's index in Map (INDEX_NONE: closed). */
	TArray<int32> CampOfSlot;
	/** The living units seen as observers, by identifier; Stamp: the last Update that walked them. */
	TMap<uint32, FTracked> UnitObservers;
	/** F3.2: the buildings, by actor (UObject's unique identifier); Radius: their sight at the last Update. */
	TMap<uint32, FTracked> BuildingObservers;
	TMap<FName, double> UnitSights;
	uint32 Stamp = 0;
	double LastUpdateMs = 0.;
	FDelegateHandle SourceHandle;
	FDelegateHandle StepHandle;
};
