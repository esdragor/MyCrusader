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

namespace
{
	/** Game worlds alive in this process holding the reset subsystem (game thread only: world init and cleanup). */
	int32 LiveGameWorlds = 0;
}

void UHOLStealthOpacityResetSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// The first game world of the process (or the first after all others ended): nothing of an older match may survive. A world joining a live one keeps the table.
	if (++LiveGameWorlds == 1)
	{
		UHOLUnitOpacityLibrary::ClearStealthOpacities();
	}
}

void UHOLStealthOpacityResetSubsystem::Deinitialize()
{
	// The last game world ends: the match is over, its opacities go with it (a world ending while another lives leaves the table alone).
	if (--LiveGameWorlds == 0)
	{
		UHOLUnitOpacityLibrary::ClearStealthOpacities();
	}
	Super::Deinitialize();
}

int32 UHOLStealthOpacityResetSubsystem::GetLiveGameWorlds()
{
	return LiveGameWorlds;
}

HOLUnitDraw::FInstanceCustomData HOLUnitDraw::MakeInstanceCustomData(const FVector3f& Color, const uint8 Prop, const float Opacity)
{
	return { Color, static_cast<float>(Prop), Opacity };
}

HOLUnitDraw::FInstanceCustomData HOLUnitDraw::MakeUnitInstanceCustomData(const FVector3f& Color, const uint8 Prop, const int32 UnitId)
{
	// D824 Q3: the opacity the stealth rule set for this unit (0.5 in a zone, 1 when none was set).
	return MakeInstanceCustomData(Color, Prop, UHOLUnitOpacityLibrary::GetStealthOpacity(UnitId));
}

HOLUnitDraw::FOpacitySnapshot HOLUnitDraw::TakeOpacitySnapshot()
{
	// One lock for the whole pass. Nothing is copied when the table is empty (the normal case: no stealth zone); otherwise the (small) table is copied.
	FScopeLock Lock(&OpacityLock);
	const TMap<int32, float>& Table = GetOpacities();
	if (Table.IsEmpty())
	{
		return FOpacitySnapshot();
	}
	return FOpacitySnapshot(TMap<int32, float>(Table));
}

HOLUnitDraw::FInstanceCustomData HOLUnitDraw::MakeUnitInstanceCustomData(const FVector3f& Color, const uint8 Prop, const int32 UnitId, const FOpacitySnapshot& Snapshot)
{
	return MakeInstanceCustomData(Color, Prop, Snapshot.Get(UnitId));
}
