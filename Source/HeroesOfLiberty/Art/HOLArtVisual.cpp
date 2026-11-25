// Copyright Epic Games, Inc. All Rights Reserved.

#include "Art/HOLArtVisual.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
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
	// AR0-a2 red stub: no icon read from the asset yet.
	return FString();
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

	const UHOLArtVisual* Find(const UWorld* World, const FName TypeName, const FName FactionKey)
	{
		if (const TStrongObjectPtr<UHOLArtVisual>* Entry = TestEntries().Find(UHOLArtVisualRegistry::FKey(TypeName, FactionKey)))
		{
			return Entry->Get();
		}
		const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		const UHOLArtVisualRegistry* Registry = GameInstance ? GameInstance->GetSubsystem<UHOLArtVisualRegistry>() : nullptr;
		return Registry ? Registry->Find(TypeName, FactionKey) : nullptr;
	}

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
	}

	bool HasFactionAsset(const UWorld* World, const FName TypeName, const FName FactionKey)
	{
		return FindFaction(World, TypeName, FactionKey) != nullptr;
	}

	// AR0-a2 red stubs: the CSV row's value, every asset ignored (the fix walks faction -> common -> fallback).
	FSoftObjectPath ResolveMesh(const UWorld* World, const FName TypeName, const FName FactionKey, const FSoftObjectPath& Fallback)
	{
		return Fallback;
	}

	float ResolveMeshScale(const UWorld* World, const FName TypeName, const FName FactionKey, const float Fallback)
	{
		return Fallback;
	}

	FSoftObjectPath ResolveProjectile(const UWorld* World, const FName TypeName, const FName FactionKey, const FSoftObjectPath& Fallback)
	{
		return Fallback;
	}

	FSoftObjectPath ResolveMuzzleSmoke(const UWorld* World, const FName TypeName, const FName FactionKey, const FSoftObjectPath& Fallback)
	{
		return Fallback;
	}

	FSoftObjectPath ResolveIcon(const UWorld* World, const FName TypeName, const FName FactionKey, const FSoftObjectPath& Fallback)
	{
		return Fallback;
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
