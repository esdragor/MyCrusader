// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
#include "HOLArtVisual.generated.h"

class UStaticMesh;
class USkeletalMesh;
class UTexture2D;
class UAnimSequence;
class UAnimSequenceTransformProviderData;
class USoundBase;
struct FHOLUnitTypeConstants;

/** AR3-a: one animation by reference per resource (gathering, walking with a load, standing with a load). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLResourceAnimations
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
	TSoftObjectPtr<UAnimSequence> Food;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
	TSoftObjectPtr<UAnimSequence> Wood;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
	TSoftObjectPtr<UAnimSequence> Stone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
	TSoftObjectPtr<UAnimSequence> Gold;
};

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Body", meta = (InlineEditConditionToggle))
	bool bOverrideMeshYawOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Body", meta = (EditCondition = "bOverrideMeshYawOffset"))
	float MeshYawOffset = -90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Body", meta = (InlineEditConditionToggle))
	bool bOverrideMeshScale = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Body", meta = (EditCondition = "bOverrideMeshScale"))
	float MeshScale = 1.f;

	/** Sequences (v1: numbers in the AnimationProvider's list, as the CSV's); each with its toggle (off = next level). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideIdleSequence = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideIdleSequence"))
	int32 IdleSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideWalkSequence = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideWalkSequence"))
	int32 WalkSequence = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideWalkPlayRate = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideWalkPlayRate"))
	float WalkPlayRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideAttackSequence = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideAttackSequence"))
	int32 AttackSequence = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideFallSequence = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideFallSequence"))
	int32 FallSequence = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideBuildSequence = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideBuildSequence"))
	int32 BuildSequence = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideBuildPlayRate = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideBuildPlayRate"))
	float BuildPlayRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideGatherPlayRate = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideGatherPlayRate"))
	float GatherPlayRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideCarryWalkPlayRate = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideCarryWalkPlayRate"))
	float CarryWalkPlayRate = 1.f;

	/** AR0-d2: gathering, walking with a load and standing with a load, by resource (J5c), each set with one toggle. */
	UPROPERTY(EditAnywhere, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideGatherSequences = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideGatherSequences"))
	FHOLResourceSequences GatherSequences;

	UPROPERTY(EditAnywhere, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideCarryWalkSequences = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideCarryWalkSequences"))
	FHOLResourceSequences CarryWalkSequences;

	UPROPERTY(EditAnywhere, Category = "Art|Sequences", meta = (InlineEditConditionToggle))
	bool bOverrideCarryIdleSequences = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sequences", meta = (EditCondition = "bOverrideCarryIdleSequences"))
	FHOLResourceSequences CarryIdleSequences;

	/** AR0-d2: props (ids of the mesh's vertex bits, 0: none) gathering and carrying each resource, building, striking. */
	UPROPERTY(EditAnywhere, Category = "Art|Props", meta = (InlineEditConditionToggle))
	bool bOverrideGatherProps = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Props", meta = (EditCondition = "bOverrideGatherProps"))
	FHOLResourceProps GatherProps;

	UPROPERTY(EditAnywhere, Category = "Art|Props", meta = (InlineEditConditionToggle))
	bool bOverrideCarryProps = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Props", meta = (EditCondition = "bOverrideCarryProps"))
	FHOLResourceProps CarryProps;

	UPROPERTY(EditAnywhere, Category = "Art|Props", meta = (InlineEditConditionToggle))
	bool bOverrideBuildProp = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Props", meta = (EditCondition = "bOverrideBuildProp", ClampMin = "0", ClampMax = "8"))
	int32 BuildProp = 0;

	UPROPERTY(EditAnywhere, Category = "Art|Props", meta = (InlineEditConditionToggle))
	bool bOverrideAttackProp = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Props", meta = (EditCondition = "bOverrideAttackProp", ClampMin = "0", ClampMax = "8"))
	int32 AttackProp = 0;

	/**
	 * AR3-a: each state's animation by reference. At the Mass type registration its number is looked up in GetSequences()
	 * of the resolved AnimationProvider and replaces the number above; empty = the number; set but absent from the
	 * provider = an error naming (type, state, sequence) and the number is kept (HOLArtVisual::ResolveAnimations).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Animations")
	TSoftObjectPtr<UAnimSequence> IdleAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Animations")
	TSoftObjectPtr<UAnimSequence> WalkAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Animations")
	TSoftObjectPtr<UAnimSequence> AttackAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Animations")
	TSoftObjectPtr<UAnimSequence> FallAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Animations")
	TSoftObjectPtr<UAnimSequence> BuildAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Animations")
	FHOLResourceAnimations GatherAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Animations")
	FHOLResourceAnimations CarryWalkAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Animations")
	FHOLResourceAnimations CarryIdleAnimations;

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

	/** AR6-a: the sounds of this type (empty = the audio table, Script\Audio\HOLAudioTable.as); the faction's asset first. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sounds")
	TSoftObjectPtr<USoundBase> SelectionSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sounds")
	TSoftObjectPtr<USoundBase> OrderSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sounds")
	TSoftObjectPtr<USoundBase> AttackSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sounds")
	TSoftObjectPtr<USoundBase> DeathSound;

	/** A building's construction sound (empty for a unit or a source). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Art|Sounds")
	TSoftObjectPtr<USoundBase> ConstructionSound;

	/** AR6-a: the path of one sound slot (Selection, Order, Attack, Death, Construction), empty when unset; script binding. */
	UFUNCTION(BlueprintPure, Category = "Art|Sounds")
	FString GetSoundPath(FName Slot) const;

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

	/**
	 * Script (AR0-a2): the icon path of a type's art asset (the faction's, then the common one), empty when none sets it.
	 * A null context reads the running game world (the interface rules have none at hand); test entries answer in any world.
	 */
	UFUNCTION(BlueprintCallable, Category = "HOL|Art")
	static FString GetArtIconPath(const UObject* WorldContextObject, FName TypeName, FName Faction);

	/** Script tests: an in-memory asset of (type, faction) whose Icon is IconPath, answering until ClearTestArtVisuals. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Art|Tests")
	static void AddTestArtVisualIcon(FName TypeName, FName Faction, const FString& IconPath);

	/** AR6-a, tests: a test asset of (TypeName, Faction) whose sound Slot is SoundPath (Script\Audio\HOLArtSound_Test.as). */
	UFUNCTION(BlueprintCallable, Category = "HOL|Art|Tests")
	static void AddTestArtVisualSound(FName TypeName, FName Faction, FName Slot, const FString& SoundPath);

	UFUNCTION(BlueprintCallable, Category = "HOL|Art|Tests")
	static void ClearTestArtVisuals();

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

	/** AR3-a: the number of Animation in Provider's GetSequences(), INDEX_NONE when it is not there (or either is empty). */
	HEROESOFLIBERTY_API int32 FindSequenceIndex(const UAnimSequenceTransformProviderData* Provider, const TSoftObjectPtr<UAnimSequence>& Animation);

	/** Whether the faction has an asset of its own for the type (it may then get its own Mass type without an override row). */
	HEROESOFLIBERTY_API bool HasFactionAsset(const UWorld* World, FName TypeName, FName FactionKey);

	/**
	 * D924 (AR0-a2): one field of the art for the readers outside the Mass constants (reception, arrows, buildings, sources,
	 * icon), field by field: the faction's asset, then the type's common asset, then Fallback (the CSV row's value).
	 */
	HEROESOFLIBERTY_API FSoftObjectPath ResolveMesh(const UWorld* World, FName TypeName, FName FactionKey, const FSoftObjectPath& Fallback);
	HEROESOFLIBERTY_API float ResolveMeshScale(const UWorld* World, FName TypeName, FName FactionKey, float Fallback);
	HEROESOFLIBERTY_API FSoftObjectPath ResolveProjectile(const UWorld* World, FName TypeName, FName FactionKey, const FSoftObjectPath& Fallback);
	HEROESOFLIBERTY_API FSoftObjectPath ResolveMuzzleSmoke(const UWorld* World, FName TypeName, FName FactionKey, const FSoftObjectPath& Fallback);
	HEROESOFLIBERTY_API FSoftObjectPath ResolveIcon(const UWorld* World, FName TypeName, FName FactionKey, const FSoftObjectPath& Fallback);

	/** Tests: an asset made in memory (NewObject) answers for its (TypeName, Faction) in every world, until cleared. */
	HEROESOFLIBERTY_API void AddTestEntry(UHOLArtVisual* Asset);
	HEROESOFLIBERTY_API void ClearTestEntries();
}
