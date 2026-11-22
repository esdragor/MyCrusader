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

UHOLFollowBridge* UHOLFollowBridge::GetFor(AHOLPlayerController* Controller)
{
	return Get(); // STUB (red commit, L22-2): still the CDO; the fix creates the instance.
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

void UHOLFollowBridge::NativeManualOrder(const TConstArrayView<uint32> UnitIds)
{
	if (UnitIds.IsEmpty())
	{
		return;
	}
	if (UHOLFollowBridge* Bridge = Get())
	{
		TArray<int32> Ids;
		for (const uint32 UnitId : UnitIds)
		{
			Ids.Add(static_cast<int32>(UnitId));
		}
		Bridge->BP_ManualOrder(Ids);
	}
}
