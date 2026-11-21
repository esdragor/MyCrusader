// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLFollowBridge.h"

#include "UObject/UObjectIterator.h"

UHOLFollowBridge* UHOLFollowBridge::Get()
{
	static TWeakObjectPtr<UClass> Cached;
	if (!Cached.IsValid() || Cached->HasAnyClassFlags(CLASS_NewerVersionExists))
	{
		Cached = nullptr;
		// The hot reload of Angelscript keeps the older classes, flagged CLASS_NewerVersionExists.
		for (TObjectIterator<UClass> It; It; ++It)
		{
			if (It->IsChildOf(StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists | CLASS_Deprecated))
			{
				Cached = *It;
				break;
			}
		}
	}
	return Cached.IsValid() ? Cast<UHOLFollowBridge>(Cached->GetDefaultObject()) : nullptr;
}

bool UHOLFollowBridge::NativeRightClickOnUnit(AHOLPlayerController* Controller, const TArray<int32>& Selected, const int32 TargetId, const bool bTargetIsAlly)
{
	UHOLFollowBridge* Bridge = Get();
	return Bridge && Bridge->BP_RightClickOnUnit(Controller, Selected, TargetId, bTargetIsAlly);
}

void UHOLFollowBridge::NativeStep(AHOLPlayerController* Controller, const int32 PlayerIndex)
{
	if (UHOLFollowBridge* Bridge = Get())
	{
		Bridge->BP_Step(Controller, PlayerIndex);
	}
}
