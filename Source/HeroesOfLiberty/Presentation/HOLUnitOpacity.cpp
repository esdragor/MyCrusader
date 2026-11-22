// Copyright Epic Games, Inc. All Rights Reserved.

#include "Presentation/HOLUnitOpacity.h"

#include "Misc/ScopeLock.h"

namespace
{
	/** Opacity per unit id (only the units below 1; 1 is the default and is not stored). Local to the machine, guarded: the drawing pass may read it off the game thread. */
	FCriticalSection OpacityLock;
	TMap<int32, float>& GetOpacities()
	{
		static TMap<int32, float> Opacities;
		return Opacities;
	}
}

void UHOLUnitOpacityLibrary::SetStealthOpacity(const int32 UnitId, const float Opacity)
{
	const float Clamped = FMath::Clamp(Opacity, 0.f, 1.f);
	FScopeLock Lock(&OpacityLock);
	if (Clamped >= 1.f)
	{
		GetOpacities().Remove(UnitId);
	}
	else
	{
		GetOpacities().Add(UnitId, Clamped);
	}
}

float UHOLUnitOpacityLibrary::GetStealthOpacity(const int32 UnitId)
{
	FScopeLock Lock(&OpacityLock);
	const float* Found = GetOpacities().Find(UnitId);
	return Found ? *Found : 1.f;
}

void UHOLUnitOpacityLibrary::ClearStealthOpacities()
{
	FScopeLock Lock(&OpacityLock);
	GetOpacities().Empty();
}

HOLUnitDraw::FInstanceCustomData HOLUnitDraw::MakeInstanceCustomData(const FVector3f& Color, const uint8 Prop, const float Opacity)
{
	return { Color, static_cast<float>(Prop), Opacity };
}

HOLUnitDraw::FInstanceCustomData HOLUnitDraw::MakeUnitInstanceCustomData(const FVector3f& Color, const uint8 Prop, const int32 UnitId)
{
	// STUB (red commit): always opaque. The real value is UHOLUnitOpacityLibrary::GetStealthOpacity(UnitId).
	return MakeInstanceCustomData(Color, Prop, 1.f);
}
