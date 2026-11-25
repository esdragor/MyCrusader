// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLGridActionBridge.h"

#include "UObject/UObjectIterator.h"

UHOLGridActionBridge* UHOLGridActionBridge::Get()
{
	static TWeakObjectPtr<UClass> Cached;
	if (!Cached.IsValid() || Cached->HasAnyClassFlags(CLASS_NewerVersionExists))
	{
		Cached = nullptr;
		// Angelscript's hot reload keeps the older classes, flagged CLASS_NewerVersionExists.
		for (TObjectIterator<UClass> It; It; ++It)
		{
			if (It->IsChildOf(StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists | CLASS_Deprecated))
			{
				Cached = *It;
				break;
			}
		}
	}
	return Cached.IsValid() ? Cast<UHOLGridActionBridge>(Cached->GetDefaultObject()) : nullptr;
}

bool UHOLGridActionBridge::NativeDecideGridAction(const int32 Command, const int32 Ability, const bool bGreyed, const int32 CooldownSeconds,
	const FName TypeName, const TArray<int32>& SelectedUnitIds, FHOLGridActionAnswer& OutAnswer)
{
	UHOLGridActionBridge* Bridge = Get();
	if (!Bridge)
	{
		return false;
	}
	OutAnswer = Bridge->BP_DecideGridAction(Command, Ability, bGreyed, CooldownSeconds, TypeName, SelectedUnitIds);
	return true;
}
