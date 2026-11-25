// Copyright Epic Games, Inc. All Rights Reserved.

#include "Art/HOLArtVisual.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Simulation/HOLUnitTypes.h"
#include "UObject/StrongObjectPtr.h"

// AR0-a red stub (D924): the classes and the test entries exist, the resolver and the registry do nothing yet.

const FPrimaryAssetType UHOLArtVisual::PrimaryAssetType(TEXT("HOLArtVisual"));

FPrimaryAssetId UHOLArtVisual::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(PrimaryAssetType, GetFName());
}

void UHOLArtVisual::ApplyTo(FHOLUnitTypeConstants& Constants) const
{
}

void UHOLArtVisualRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

const UHOLArtVisual* UHOLArtVisualRegistry::Find(const FName TypeName, const FName Faction) const
{
	const UHOLArtVisual* const* Found = Map.Find(FKey(TypeName, Faction));
	return Found ? *Found : nullptr;
}

UHOLArtVisual* UHOLArtVisualRegistry::FindArtVisual(const UObject* WorldContextObject, const FName TypeName, const FName Faction)
{
	return nullptr;
}

void UHOLArtVisualRegistry::BuildMap(const TArray<const UHOLArtVisual*>& Assets, TMap<FKey, const UHOLArtVisual*>& OutMap, TArray<FString>& OutErrors)
{
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
	}

	bool HasFactionAsset(const UWorld* World, const FName TypeName, const FName FactionKey)
	{
		return false;
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
