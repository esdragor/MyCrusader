// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLGridFaceBridge.h"

#include "UObject/UObjectIterator.h"

UHOLGridFaceBridge* UHOLGridFaceBridge::Get()
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
	return Cached.IsValid() ? Cast<UHOLGridFaceBridge>(Cached->GetDefaultObject()) : nullptr;
}

bool UHOLGridFaceBridge::NativeGetCellFace(const int32 Command, const int32 Ability, const bool bGreyed, const int32 CommandCooldownSeconds,
	const int32 FactCooldownSeconds, const int32 UnlockLevel, const TArray<int32>& Numbers, FHOLGridFaceAnswer& OutAnswer)
{
	UHOLGridFaceBridge* Bridge = Get();
	if (!Bridge)
	{
		return false;
	}
	OutAnswer = Bridge->BP_GetCellFace(Command, Ability, bGreyed, CommandCooldownSeconds, FactCooldownSeconds, UnlockLevel, Numbers);
	return true;
}
