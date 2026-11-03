// Copyright Epic Games, Inc. All Rights Reserved.

#include "HUD/HOLHUDSpace.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Layout/Geometry.h"

FVector2D HOLHUDSpace::AbsoluteToViewport(const UObject* WorldContext, const FVector2D& Absolute)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (World && World->IsGameWorld() && World->GetGameViewport())
	{
		FVector2D Pixel;
		FVector2D Viewport;
		USlateBlueprintLibrary::AbsoluteToViewport(WorldContext, Absolute, Pixel, Viewport);
		return Viewport;
	}
	const float Dpi = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(WorldContext), UE_KINDA_SMALL_NUMBER);
	return Absolute / Dpi;
}

FBox2D HOLHUDSpace::BoxOf(const UObject* WorldContext, const FGeometry& Geometry)
{
	const FVector2D Low = FVector2D(Geometry.GetAbsolutePosition());
	return FBox2D(AbsoluteToViewport(WorldContext, Low), AbsoluteToViewport(WorldContext, Low + FVector2D(Geometry.GetAbsoluteSize())));
}
