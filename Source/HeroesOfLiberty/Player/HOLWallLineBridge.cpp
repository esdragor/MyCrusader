// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLWallLineBridge.h"

#include "UObject/UObjectIterator.h"

UHOLWallLineBridge* UHOLWallLineBridge::Get()
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
	return Cached.IsValid() ? Cast<UHOLWallLineBridge>(Cached->GetDefaultObject()) : nullptr;
}

bool UHOLWallLineBridge::NativeIsWallType(const FName TypeName)
{
	UHOLWallLineBridge* Bridge = Get();
	return Bridge && !TypeName.IsNone() && Bridge->BP_IsWallType(TypeName);
}

TArray<FIntPoint> UHOLWallLineBridge::NativeDragCells(const FVector& Start, const FVector& End, const double Tile)
{
	UHOLWallLineBridge* Bridge = Get();
	return Bridge ? Bridge->BP_DragCells(Start, End, Tile) : TArray<FIntPoint>();
}