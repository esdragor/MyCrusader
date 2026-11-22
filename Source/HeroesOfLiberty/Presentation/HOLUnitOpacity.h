// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLUnitOpacity.generated.h"

/**
 * The render opacity of a unit, for Angelscript (a binding to expose, D839): the stealth rule (HOLStealth::OwnUnitOpacity, Script/Fog/HOLStealthRules.as,
 * D824 Q3 B) decides the value, this only carries it to where the units are drawn. One value per unit id (1 = opaque, the default), local to this
 * machine: it is a presentation datum, never replicated. The drawing pass (UHOLInterpolatedInstancesProcessor, where a unit's instance custom data is
 * written) reads it through GetStealthOpacity. Unit ids are int32 here (script). Thread-safe (the drawing pass may run off the game thread).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLUnitOpacityLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Sets the opacity a unit is drawn with, clamped to 0..1. Setting 1 forgets the unit (back to the default). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Units")
	static void SetStealthOpacity(int32 UnitId, float Opacity);

	/** The opacity a unit is drawn with: 1 when none was set. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Units")
	static float GetStealthOpacity(int32 UnitId);

	/** Forgets every unit's opacity (end of match, tests). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Units")
	static void ClearStealthOpacities();
};

/**
 * Forgets the opacity table at the start of a game world (ARWEN-1122g, REVUE-1122 defect 1: the table is a process static, unit ids restart at 1 in
 * every world, and the script only writes CHANGES, so a unit left at 0.5 by the previous match stayed translucent in the next one). Safe for two
 * PIE worlds in one process: the table is cleared when a game world starts or ends ONLY if no other game world is alive, so a second world never
 * wipes the first one's values. A match without stealth zone pays nothing (clearing an empty table). Values and cadence are untouched.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLStealthOpacityResetSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** The number of game worlds (Game / PIE) currently holding this subsystem (game thread). */
	static int32 GetLiveGameWorlds();
};

/**
 * The per-instance custom data of a drawn unit (what UHOLInterpolatedInstancesProcessor hands to Mass, one float per component and the same count
 * for every unit and corpse of a component): colour in 0-2 (M_UnitPlayerColor), the prop shown in 3 (M_UnitPlayerColorProps), the stealth opacity
 * in 4 (D824 Q3: 0.5 for a unit in a zone, 1 otherwise). Indexes 0-3 never move: the material reads them. Plain floats (Mass copies the struct at any
 * float offset).
 */
namespace HOLUnitDraw
{
	constexpr int32 NumInstanceCustomFloats = 5;

	struct FInstanceCustomData
	{
		FVector3f Color;
		float Prop;
		float Opacity;
	};
	static_assert(sizeof(FInstanceCustomData) == NumInstanceCustomFloats * sizeof(float));

	/** The custom data with an explicit opacity (the bodies: 1). */
	HEROESOFLIBERTY_API FInstanceCustomData MakeInstanceCustomData(const FVector3f& Color, uint8 Prop, float Opacity);

	/** The custom data of a living unit: its opacity is UHOLUnitOpacityLibrary::GetStealthOpacity(UnitId) (one lock per call: for one-off callers and tests; the drawing pass uses the snapshot overload). */
	HEROESOFLIBERTY_API FInstanceCustomData MakeUnitInstanceCustomData(const FVector3f& Color, uint8 Prop, int32 UnitId);

	/**
	 * An immutable copy of the opacity table, taken ONCE per run of the drawing pass (TakeOpacitySnapshot, one lock): the per-unit lookups then take no
	 * lock, and a write made during the pass (script, game thread) never alters a snapshot already taken. An empty table (the normal case: no stealth zone)
	 * answers 1 without any search.
	 */
	class FOpacitySnapshot
	{
	public:
		FOpacitySnapshot() = default;
		explicit FOpacitySnapshot(TMap<int32, float>&& InTable) : Table(MoveTemp(InTable)) {}

		bool IsEmpty() const { return Table.IsEmpty(); }

		/** The opacity of a unit in this snapshot: 1 when none was set (and no search when the table is empty). */
		float Get(const int32 UnitId) const
		{
			if (Table.IsEmpty())
			{
				return 1.f;
			}
			const float* Found = Table.Find(UnitId);
			return Found ? *Found : 1.f;
		}

	private:
		TMap<int32, float> Table;
	};

	/** Copies the opacity table under the lock, once (nothing is copied when the table is empty). */
	HEROESOFLIBERTY_API FOpacitySnapshot TakeOpacitySnapshot();

	/** The custom data of a living unit, its opacity read from the snapshot (no lock). Called once per unit and frame by the drawing pass. */
	HEROESOFLIBERTY_API FInstanceCustomData MakeUnitInstanceCustomData(const FVector3f& Color, uint8 Prop, int32 UnitId, const FOpacitySnapshot& Snapshot);
}
