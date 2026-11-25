// Copyright Epic Games, Inc. All Rights Reserved.

#include "Art/HOLArtVisual.h"

#include "Animation/AnimSequence.h"
#include "Animation/AnimSequenceTransformProviderData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Misc/PackageName.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "UObject/Package.h"
#include "Engine/World.h"
#include "Match/HOLFactionRules.h"
#include "Modules/ModuleManager.h"
#include "Simulation/HOLUnitTypes.h"
#include "UObject/StrongObjectPtr.h"

DEFINE_LOG_CATEGORY_STATIC(LogHOLArt, Log, All);

namespace
{
	/** Where the art visuals live and are cooked from (Config/DefaultGame.ini, PrimaryAssetTypesToScan). */
	const TCHAR* const ArtRoot = TEXT("/Game/Art");
}

const FPrimaryAssetType UHOLArtVisual::PrimaryAssetType(TEXT("HOLArtVisual"));

FPrimaryAssetId UHOLArtVisual::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(PrimaryAssetType, GetFName());
}

void UHOLArtVisual::ApplyTo(FHOLUnitTypeConstants& Constants) const
{
	// An empty field leaves the level below (the common asset, then the CSV row) in place.
	if (!Mesh.IsNull())
	{
		Constants.Mesh = Mesh.ToSoftObjectPath();
	}
	if (!SkinnedMesh.IsNull())
	{
		Constants.SkinnedMesh = SkinnedMesh.ToSoftObjectPath();
	}
	if (!AnimationProvider.IsNull())
	{
		Constants.AnimationProvider = AnimationProvider.ToSoftObjectPath();
	}
	const auto Set = [](const bool bOverride, auto& Target, const auto Value)
	{
		if (bOverride)
		{
			Target = Value;
		}
	};
	Set(bOverrideMeshYawOffset, Constants.MeshYawOffset, MeshYawOffset);
	Set(bOverrideMeshScale, Constants.MeshScale, MeshScale);
	Set(bOverrideIdleSequence, Constants.IdleSequence, IdleSequence);
	Set(bOverrideWalkSequence, Constants.WalkSequence, WalkSequence);
	Set(bOverrideWalkPlayRate, Constants.WalkPlayRate, WalkPlayRate);
	Set(bOverrideAttackSequence, Constants.AttackSequence, AttackSequence);
	Set(bOverrideFallSequence, Constants.FallSequence, FallSequence);
	Set(bOverrideBuildSequence, Constants.BuildSequence, BuildSequence);
	Set(bOverrideBuildPlayRate, Constants.BuildPlayRate, BuildPlayRate);
	Set(bOverrideGatherPlayRate, Constants.GatherPlayRate, GatherPlayRate);
	Set(bOverrideCarryWalkPlayRate, Constants.CarryWalkPlayRate, CarryWalkPlayRate);
	Set(bOverrideGatherSequences, Constants.GatherSequences, GatherSequences);
	Set(bOverrideCarryWalkSequences, Constants.CarryWalkSequences, CarryWalkSequences);
	Set(bOverrideCarryIdleSequences, Constants.CarryIdleSequences, CarryIdleSequences);
	Set(bOverrideGatherProps, Constants.GatherProps, GatherProps);
	Set(bOverrideCarryProps, Constants.CarryProps, CarryProps);
	Set(bOverrideBuildProp, Constants.BuildProp, BuildProp);
	Set(bOverrideAttackProp, Constants.AttackProp, AttackProp);
}

void UHOLArtVisualRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	if (!FPlatformProperties::RequiresCookedData())
	{
		// An editor build may still be scanning at start: the art folder is small, scan it now.
		AssetRegistry.ScanPathsSynchronous({ FString(ArtRoot) });
	}
	FARFilter Filter;
	Filter.PackagePaths.Add(FName(ArtRoot));
	Filter.bRecursivePaths = true;
	Filter.ClassPaths.Add(UHOLArtVisual::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;
	TArray<FAssetData> Found;
	AssetRegistry.GetAssets(Filter, Found);

	TArray<const UHOLArtVisual*> Assets;
	for (const FAssetData& Data : Found)
	{
		if (UHOLArtVisual* Asset = Cast<UHOLArtVisual>(Data.GetAsset()))
		{
			Loaded.Add(Asset);
			Assets.Add(Asset);
		}
	}
	Map.Reset();
	Errors.Reset();
	BuildMap(Assets, Map, Errors);
	for (const FString& Error : Errors)
	{
		UE_LOG(LogHOLArt, Error, TEXT("%s"), *Error);
	}
	UE_LOG(LogHOLArt, Log, TEXT("Art visuals: %d under %s, %d keys, %d errors (D924)"), Assets.Num(), ArtRoot, Map.Num(), Errors.Num());
}

const UHOLArtVisual* UHOLArtVisualRegistry::Find(const FName TypeName, const FName Faction) const
{
	const UHOLArtVisual* const* Found = Map.Find(FKey(TypeName, Faction));
	return Found ? *Found : nullptr;
}

UHOLArtVisual* UHOLArtVisualRegistry::FindArtVisual(const UObject* WorldContextObject, const FName TypeName, const FName Faction)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UHOLArtVisual* Own = Faction.IsNone() ? nullptr : HOLArtVisual::Find(World, TypeName, Faction);
	return const_cast<UHOLArtVisual*>(Own ? Own : HOLArtVisual::Find(World, TypeName, NAME_None));
}

FString UHOLArtVisualRegistry::GetArtIconPath(const UObject* WorldContextObject, const FName TypeName, const FName Faction)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World && GEngine)
	{
		// The interface rules have no context at hand: the running game world (a client has one).
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if ((Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE) && Context.World())
			{
				World = Context.World();
				break;
			}
		}
	}
	return HOLArtVisual::ResolveIcon(World, TypeName, Faction, FSoftObjectPath()).ToString();
}

void UHOLArtVisualRegistry::AddTestArtVisualIcon(const FName TypeName, const FName Faction, const FString& IconPath)
{
	UHOLArtVisual* Asset = NewObject<UHOLArtVisual>(GetTransientPackage());
	Asset->TypeName = TypeName;
	Asset->Faction = Faction;
	Asset->Icon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(IconPath));
	HOLArtVisual::AddTestEntry(Asset);
}

void UHOLArtVisualRegistry::ClearTestArtVisuals()
{
	HOLArtVisual::ClearTestEntries();
}

void UHOLArtVisualRegistry::BuildMap(const TArray<const UHOLArtVisual*>& Assets, TMap<FKey, const UHOLArtVisual*>& OutMap, TArray<FString>& OutErrors)
{
	for (const UHOLArtVisual* Asset : Assets)
	{
		if (!Asset)
		{
			continue;
		}
		if (Asset->TypeName.IsNone())
		{
			OutErrors.Add(FString::Printf(TEXT("Art visual %s has no TypeName: it dresses nothing (D924)"), *Asset->GetPathName()));
			continue;
		}
		const FKey Key(Asset->TypeName, Asset->Faction);
		if (const UHOLArtVisual* const* Existing = OutMap.Find(Key))
		{
			OutErrors.Add(FString::Printf(TEXT("Art visual (%s, %s) defined twice: %s and %s; the first one is kept (D924)"),
				*Key.Key.ToString(), Key.Value.IsNone() ? TEXT("every faction") : *Key.Value.ToString(),
				*(*Existing)->GetPathName(), *Asset->GetPathName()));
			continue;
		}
		OutMap.Add(Key, Asset);
	}
}

namespace HOLArtVisual
{
	namespace
	{
		TMap<UHOLArtVisualRegistry::FKey, TStrongObjectPtr<UHOLArtVisual>>& TestEntries()
		{
			static TMap<UHOLArtVisualRegistry::FKey, TStrongObjectPtr<UHOLArtVisual>> Entries;
			return Entries;
		}

		/** The faction's own asset: by the key as given, then by the faction table's resolved key (an alias of the faction). */
		const UHOLArtVisual* FindFaction(const UWorld* World, const FName TypeName, const FName FactionKey)
		{
			if (FactionKey.IsNone())
			{
				return nullptr;
			}
			if (const UHOLArtVisual* Own = Find(World, TypeName, FactionKey))
			{
				return Own;
			}
			const FName Resolved = HOLFactionRules::GetTable().ResolveKey(FactionKey);
			return Resolved != FactionKey && !Resolved.IsNone() ? Find(World, TypeName, Resolved) : nullptr;
		}
	}

	/** The type's common asset /Game/Art/<Units|Buildings|Sources>/<Type>/DA_Art_<Type>, loaded on demand; null if absent. */
	static const UHOLArtVisual* LoadCommon(const FName TypeName)
	{
		static TSet<FName> Missing;
		if (TypeName.IsNone() || Missing.Contains(TypeName))
		{
			return nullptr;
		}
		const FString Type = TypeName.ToString();
		const FString Name = TEXT("DA_Art_") + Type;
		for (const TCHAR* Folder : { TEXT("Units"), TEXT("Buildings"), TEXT("Sources") })
		{
			const FString Package = FString::Printf(TEXT("/Game/Art/%s/%s/%s"), Folder, *Type, *Name);
			const FString Object = FString::Printf(TEXT("%s.%s"), *Package, *Name);
			if (UHOLArtVisual* Found = FindObject<UHOLArtVisual>(nullptr, *Object))
			{
				return Found;
			}
			if (FPackageName::DoesPackageExist(Package))
			{
				if (UHOLArtVisual* Loaded = LoadObject<UHOLArtVisual>(nullptr, *Object, nullptr, LOAD_NoWarn | LOAD_Quiet))
				{
					return Loaded;
				}
			}
		}
		Missing.Add(TypeName);
		return nullptr;
	}

	const UHOLArtVisual* Find(const UWorld* World, const FName TypeName, const FName FactionKey)
	{
		if (const TStrongObjectPtr<UHOLArtVisual>* Entry = TestEntries().Find(UHOLArtVisualRegistry::FKey(TypeName, FactionKey)))
		{
			return Entry->Get();
		}
		const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		const UHOLArtVisualRegistry* Registry = GameInstance ? GameInstance->GetSubsystem<UHOLArtVisualRegistry>() : nullptr;
		if (Registry)
		{
			return Registry->Find(TypeName, FactionKey);
		}
		// D925 (AR0-d2): a world without a game instance (tests, tools) reads the common asset from /Game/Art itself, the
		// row no longer carrying the visual fields.
		return FactionKey.IsNone() ? LoadCommon(TypeName) : nullptr;
	}

	void ResolveAnimations(const UHOLArtVisual* Common, const UHOLArtVisual* Own, FHOLUnitTypeConstants& Constants, FName TypeName);

	void Apply(const UWorld* World, FHOLUnitTypeConstants& Constants, const FName TypeName, const FName FactionKey)
	{
		// Field by field: the type's common asset over the CSV row, then the faction's over both.
		const UHOLArtVisual* Common = Find(World, TypeName, NAME_None);
		if (Common)
		{
			Common->ApplyTo(Constants);
		}
		const UHOLArtVisual* Own = FindFaction(World, TypeName, FactionKey);
		if (Own && Own != Common)
		{
			Own->ApplyTo(Constants);
		}
		// AR3-a: the animations by reference, field by field the same way, looked up in the provider resolved above.
		ResolveAnimations(Common, Own != Common ? Own : nullptr, Constants, TypeName);
	}

	int32 FindSequenceIndex(const UAnimSequenceTransformProviderData* Provider, const TSoftObjectPtr<UAnimSequence>& Animation)
	{
		if (!Provider || Animation.IsNull())
		{
			return INDEX_NONE;
		}
		const FSoftObjectPath Path = Animation.ToSoftObjectPath();
		const TArray<FAnimSequenceTransformProviderSequence>& Sequences = Provider->GetSequences();
		for (int32 Index = 0; Index < Sequences.Num(); ++Index)
		{
			const UAnimSequence* Sequence = Sequences[Index].Sequence;
			if (Sequence && FSoftObjectPath(Sequence) == Path)
			{
				return Index;
			}
		}
		return INDEX_NONE;
	}

	/** AR3-a: the references a level sets over the ones below (an empty one leaves the level below in place). */
	struct FAnimationRefs
	{
		TSoftObjectPtr<UAnimSequence> Idle, Walk, Attack, Fall, Build;
		FHOLResourceAnimations Gather, CarryWalk, CarryIdle;

		void Overlay(const UHOLArtVisual& Asset)
		{
			const auto Set = [](TSoftObjectPtr<UAnimSequence>& Target, const TSoftObjectPtr<UAnimSequence>& Value)
			{
				if (!Value.IsNull())
				{
					Target = Value;
				}
			};
			const auto SetResources = [&Set](FHOLResourceAnimations& Target, const FHOLResourceAnimations& Value)
			{
				Set(Target.Food, Value.Food);
				Set(Target.Wood, Value.Wood);
				Set(Target.Stone, Value.Stone);
				Set(Target.Gold, Value.Gold);
			};
			Set(Idle, Asset.IdleAnimation);
			Set(Walk, Asset.WalkAnimation);
			Set(Attack, Asset.AttackAnimation);
			Set(Fall, Asset.FallAnimation);
			Set(Build, Asset.BuildAnimation);
			SetResources(Gather, Asset.GatherAnimations);
			SetResources(CarryWalk, Asset.CarryWalkAnimations);
			SetResources(CarryIdle, Asset.CarryIdleAnimations);
		}

		/** Each reference set gives its number in the provider; one absent = a named error, the number kept. */
		void Resolve(FHOLUnitTypeConstants& Constants, const FName TypeName) const
		{
			const UAnimSequenceTransformProviderData* Provider = nullptr;
			const auto One = [&Provider, &Constants, TypeName](const TCHAR* State, const TSoftObjectPtr<UAnimSequence>& Animation, int32& Number)
			{
				if (Animation.IsNull())
				{
					return;
				}
				if (!Provider)
				{
					Provider = Cast<UAnimSequenceTransformProviderData>(Constants.AnimationProvider.TryLoad());
				}
				const int32 Index = FindSequenceIndex(Provider, Animation);
				if (Index == INDEX_NONE)
				{
					UE_LOG(LogHOLArt, Error, TEXT("Art of '%s': the %s animation %s is not in the sequences of %s; its number %d is kept"),
						*TypeName.ToString(), State, *Animation.ToString(), *Constants.AnimationProvider.ToString(), Number);
					return;
				}
				Number = Index;
			};
			const auto Resources = [&One](const TCHAR* State, const FHOLResourceAnimations& Animations, FHOLResourceSequences& Numbers)
			{
				One(*FString::Printf(TEXT("%s Food"), State), Animations.Food, Numbers.Food);
				One(*FString::Printf(TEXT("%s Wood"), State), Animations.Wood, Numbers.Wood);
				One(*FString::Printf(TEXT("%s Stone"), State), Animations.Stone, Numbers.Stone);
				One(*FString::Printf(TEXT("%s Gold"), State), Animations.Gold, Numbers.Gold);
			};
			One(TEXT("Idle"), Idle, Constants.IdleSequence);
			One(TEXT("Walk"), Walk, Constants.WalkSequence);
			One(TEXT("Attack"), Attack, Constants.AttackSequence);
			One(TEXT("Fall"), Fall, Constants.FallSequence);
			One(TEXT("Build"), Build, Constants.BuildSequence);
			Resources(TEXT("Gather"), Gather, Constants.GatherSequences);
			Resources(TEXT("CarryWalk"), CarryWalk, Constants.CarryWalkSequences);
			Resources(TEXT("CarryIdle"), CarryIdle, Constants.CarryIdleSequences);
		}
	};

	void ResolveAnimations(const UHOLArtVisual* Common, const UHOLArtVisual* Own, FHOLUnitTypeConstants& Constants, const FName TypeName)
	{
		FAnimationRefs Animations;
		if (Common)
		{
			Animations.Overlay(*Common);
		}
		if (Own)
		{
			Animations.Overlay(*Own);
		}
		Animations.Resolve(Constants, TypeName);
	}

	bool HasFactionAsset(const UWorld* World, const FName TypeName, const FName FactionKey)
	{
		return FindFaction(World, TypeName, FactionKey) != nullptr;
	}

	/** Field by field (AR0-a2): the faction's asset when it sets the field, then the type's common asset, then Fallback. */
	template <typename TValue, typename TRead>
	TValue ResolveField(const UWorld* World, const FName TypeName, const FName FactionKey, const TValue& Fallback, TRead Read)
	{
		TValue Value = Fallback;
		if (const UHOLArtVisual* Own = FindFaction(World, TypeName, FactionKey))
		{
			if (Read(*Own, Value))
			{
				return Value;
			}
		}
		if (const UHOLArtVisual* Common = Find(World, TypeName, NAME_None))
		{
			if (Read(*Common, Value))
			{
				return Value;
			}
		}
		return Fallback;
	}

	FSoftObjectPath ResolveMesh(const UWorld* World, const FName TypeName, const FName FactionKey, const FSoftObjectPath& Fallback)
	{
		return ResolveField(World, TypeName, FactionKey, Fallback, [](const UHOLArtVisual& Asset, FSoftObjectPath& Out)
		{
			Out = Asset.Mesh.ToSoftObjectPath();
			return !Asset.Mesh.IsNull();
		});
	}

	float ResolveMeshScale(const UWorld* World, const FName TypeName, const FName FactionKey, const float Fallback)
	{
		return ResolveField(World, TypeName, FactionKey, Fallback, [](const UHOLArtVisual& Asset, float& Out)
		{
			Out = Asset.MeshScale;
			return Asset.bOverrideMeshScale;
		});
	}

	FSoftObjectPath ResolveProjectile(const UWorld* World, const FName TypeName, const FName FactionKey, const FSoftObjectPath& Fallback)
	{
		return ResolveField(World, TypeName, FactionKey, Fallback, [](const UHOLArtVisual& Asset, FSoftObjectPath& Out)
		{
			Out = Asset.Projectile;
			return !Asset.Projectile.IsNull();
		});
	}

	FSoftObjectPath ResolveMuzzleSmoke(const UWorld* World, const FName TypeName, const FName FactionKey, const FSoftObjectPath& Fallback)
	{
		return ResolveField(World, TypeName, FactionKey, Fallback, [](const UHOLArtVisual& Asset, FSoftObjectPath& Out)
		{
			Out = Asset.MuzzleSmoke;
			return !Asset.MuzzleSmoke.IsNull();
		});
	}

	FSoftObjectPath ResolveIcon(const UWorld* World, const FName TypeName, const FName FactionKey, const FSoftObjectPath& Fallback)
	{
		return ResolveField(World, TypeName, FactionKey, Fallback, [](const UHOLArtVisual& Asset, FSoftObjectPath& Out)
		{
			Out = Asset.Icon.ToSoftObjectPath();
			return !Asset.Icon.IsNull();
		});
	}


	void AddTestEntry(UHOLArtVisual* Asset)
	{
		if (Asset)
		{
			TestEntries().Add(UHOLArtVisualRegistry::FKey(Asset->TypeName, Asset->Faction), TStrongObjectPtr<UHOLArtVisual>(Asset));
		}
	}

	void ClearTestEntries()
	{
		TestEntries().Reset();
	}
}
