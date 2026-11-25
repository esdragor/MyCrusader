// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HOLArtVisual.generated.h"

class UStaticMesh;
class USkeletalMesh;
class UTexture2D;
class UAnimSequenceTransformProviderData;
struct FHOLUnitTypeConstants;

/** Whether the art of a type is still a placeholder or the final one (D924; read by the manifest, AR0-b). */
UENUM(BlueprintType)
enum class EHOLArtStatus : uint8
{
	Placeholder,
	Final
};

/**
 * D924 (AR0): the art of one type (and of one faction, when its visual differs), one asset per type so two artists never
 * lock each other: /Game/Art/<Units|Buildings|Sources>/<Type>/DA_Art_<Type>[_<Faction>]. The truth is TypeName and
 * Faction (none = every faction); the file name is a convention for humans. Every field is optional: an empty one falls
 * back field by field, the faction's asset -> the type's common asset -> the CSV row (HOLArtVisual::Apply).
 * C++ and not script: its readers are the Mass type registration (HOLUnitSubsystem) and other C++ sites (CLAUDE.md).
 */
UCLASS(BlueprintType)
class HEROESOFLIBERTY_API UHOLArtVisual : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** The primary asset type every art visual is cooked under (Config/DefaultGame.ini, /Game/Art). */
	static const FPrimaryAssetType PrimaryAssetType;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** The type this asset dresses (a row name of the unit, building or source table). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art")
	FName TypeName;

	/** The faction (Legions...) whose visual differs; none = every faction. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art")
	FName Faction;

	/** Static mesh: a unit's fallback body, a building's or a source's mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Body")
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Body")
	TSoftObjectPtr<USkeletalMesh> SkinnedMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Body")
	TSoftObjectPtr<UAnimSequenceTransformProviderData> AnimationProvider;

	UPROPERTY(EditAnywhere, Category = "Art|Body", meta = (InlineEditConditionToggle))
	bool bOverrideMeshYawOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Body", meta = (EditCondition = "bOverrideMeshYawOffset"))
	float MeshYawOffset = -90.f;

	UPROPERTY(EditAnywhere, Category = "Art|Body", meta = (InlineEditConditionToggle))
	bool bOverrideMeshScale = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Body", meta = (EditCondition = "bOverrideMeshScale"))
	float MeshScale = 1.f;

	/** Sequences (v1: numbers in the AnimationProvider's list, as the CSV's); each with its toggle (off = next level). */
	UPROPERTY(EditAnywhere, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideIdleSequence = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideIdleSequence"))
	int32 IdleSequence = 0;

	UPROPERTY(EditAnywhere, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideWalkSequence = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideWalkSequence"))
	int32 WalkSequence = 1;

	UPROPERTY(EditAnywhere, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideWalkPlayRate = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideWalkPlayRate"))
	float WalkPlayRate = 1.f;

	UPROPERTY(EditAnywhere, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideAttackSequence = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideAttackSequence"))
	int32 AttackSequence = INDEX_NONE;

	UPROPERTY(EditAnywhere, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideFallSequence = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideFallSequence"))
	int32 FallSequence = INDEX_NONE;

	UPROPERTY(EditAnywhere, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideBuildSequence = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideBuildSequence"))
	int32 BuildSequence = INDEX_NONE;

	UPROPERTY(EditAnywhere, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideBuildPlayRate = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideBuildPlayRate"))
	float BuildPlayRate = 1.f;

	UPROPERTY(EditAnywhere, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideGatherPlayRate = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideGatherPlayRate"))
	float GatherPlayRate = 1.f;

	UPROPERTY(EditAnywhere, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideCarryWalkPlayRate = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideCarryWalkPlayRate"))
	float CarryWalkPlayRate = 1.f;

	/** Arrow and muzzle smoke of a ranged unit (read by HOLArrowSubsystem, not yet wired: AR0-a RESTE). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Shot")
	FSoftObjectPath Projectile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Shot")
	FSoftObjectPath MuzzleSmoke;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Interface")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Interface")
	TSoftObjectPtr<UTexture2D> Portrait;

	/** Register of the art (Docs/assets_art.csv). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Register")
	EHOLArtStatus Status = EHOLArtStatus::Placeholder;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Register")
	FString Author;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Register")
	FString License;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Register")
	FString SourceUrl;

	/** Writes the fields this asset sets over the constants (the ones it leaves empty are untouched). */
	void ApplyTo(FHOLUnitTypeConstants& Constants) const;
};

/**
 * D924: the art visuals of the game, by (type, faction): at start, the Asset Registry lists every UHOLArtVisual under
 * /Game/Art. Two assets for one key = an error naming both files (the first one is kept).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLArtVisualRegistry : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	using FKey = TPair<FName, FName>;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** The asset of exactly this (type, faction); none: the common one. Null when there is none. */
	const UHOLArtVisual* Find(FName TypeName, FName Faction) const;

	/** Script: the asset that dresses a type for a faction (the faction's, else the common one), or null. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Art", meta = (WorldContext = "WorldContextObject"))
	static UHOLArtVisual* FindArtVisual(const UObject* WorldContextObject, FName TypeName, FName Faction);

	/** Builds the (type, faction) map of the assets; a key met twice = one error naming both files, the first one kept. */
	static void BuildMap(const TArray<const UHOLArtVisual*>& Assets, TMap<FKey, const UHOLArtVisual*>& OutMap, TArray<FString>& OutErrors);

	/** Problems met by the last Initialize (duplicates). */
	const TArray<FString>& GetErrors() const { return Errors; }

private:
	TMap<FKey, const UHOLArtVisual*> Map;

	UPROPERTY()
	TArray<TObjectPtr<UHOLArtVisual>> Loaded;

	TArray<FString> Errors;
};

namespace HOLArtVisual
{
	/** The asset of exactly (type, faction) for this world: a test entry first, then the game instance's registry. */
	HEROESOFLIBERTY_API const UHOLArtVisual* Find(const UWorld* World, FName TypeName, FName FactionKey);

	/**
	 * The single resolver (D924), applied right after Row->ToConstants at the Mass type registration and in its faction CRC
	 * (HOLUnitSubsystem): the common asset of the type, then the faction's over it, field by field.
	 */
	HEROESOFLIBERTY_API void Apply(const UWorld* World, FHOLUnitTypeConstants& Constants, FName TypeName, FName FactionKey);

	/** Whether the faction has an asset of its own for the type (it may then get its own Mass type without an override row). */
	HEROESOFLIBERTY_API bool HasFactionAsset(const UWorld* World, FName TypeName, FName FactionKey);

	/** Tests: an asset made in memory (NewObject) answers for its (TypeName, Faction) in every world, until cleared. */
	HEROESOFLIBERTY_API void AddTestEntry(UHOLArtVisual* Asset);
	HEROESOFLIBERTY_API void ClearTestEntries();
}
