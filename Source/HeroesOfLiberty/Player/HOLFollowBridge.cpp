// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLFollowBridge.h"

#include "Player/HOLPlayerController.h"
#include "UObject/UObjectIterator.h"

namespace
{
	/** The newest concrete script class deriving from the bridge, nullptr without one. */
	UClass* FindScriptClass()
	{
		static TWeakObjectPtr<UClass> Cached;
		if (!Cached.IsValid() || Cached->HasAnyClassFlags(CLASS_NewerVersionExists))
		{
			Cached = nullptr;
			// The hot reload of Angelscript keeps the older classes, flagged CLASS_NewerVersionExists.
			for (TObjectIterator<UClass> It; It; ++It)
			{
				if (It->IsChildOf(UHOLFollowBridge::StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists | CLASS_Deprecated))
				{
					Cached = *It;
					break;
				}
			}
		}
		return Cached.Get();
	}
}

UHOLFollowBridge* UHOLFollowBridge::GetFor(AHOLPlayerController* Controller)
{
	if (!Controller)
	{
		return nullptr;
	}
	UClass* ScriptClass = FindScriptClass();
	if (!ScriptClass)
	{
		return nullptr;
	}
	// One instance per controller, created on the first call; a bridge of an older class (hot reload) is replaced.
	if (!Controller->FollowBridge || Controller->FollowBridge->GetClass() != ScriptClass)
	{
		Controller->FollowBridge = NewObject<UHOLFollowBridge>(Controller, ScriptClass);
	}
	return Controller->FollowBridge;
}

bool UHOLFollowBridge::NativeRightClickOnUnit(AHOLPlayerController* Controller, const TArray<int32>& Selected, const int32 TargetId, const bool bTargetIsAlly)
{
	UHOLFollowBridge* Bridge = GetFor(Controller);
	return Bridge && Bridge->BP_RightClickOnUnit(Controller, Selected, TargetId, bTargetIsAlly);
}

void UHOLFollowBridge::NativeStep(AHOLPlayerController* Controller, const int32 PlayerIndex)
{
	if (UHOLFollowBridge* Bridge = GetFor(Controller))
	{
		Bridge->BP_Step(Controller, PlayerIndex);
	}
}

void UHOLFollowBridge::NativeManualOrder(AHOLPlayerController* Controller, const TConstArrayView<uint32> UnitIds)
{
	if (UnitIds.IsEmpty())
	{
		return;
	}
	if (UHOLFollowBridge* Bridge = GetFor(Controller))
	{
		TArray<int32> Ids;
		for (const uint32 UnitId : UnitIds)
		{
			Ids.Add(static_cast<int32>(UnitId));
		}
		Bridge->BP_ManualOrder(Ids);
	}
}
