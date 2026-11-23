// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLUnitScriptLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Match/HOLGameState.h"
#include "Match/HOLDiplomacyRules.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"
#include "Simulation/HOLUnitView.h"

bool UHOLUnitScriptLibrary::GetUnitLocation(const UObject* WorldContextObject, const int32 UnitId, FVector& OutLocation)
{
	OutLocation = FVector::ZeroVector;
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	FHOLUnitView View;
	// HOLUnitView::Find: the simulation in solo and on the server, the displayed units on a client; a Dying unit is left out (bIncludeDying false).
	if (!World || UnitId <= 0 || !HOLUnitView::Find(*World, static_cast<uint32>(UnitId), View, false))
	{
		return false;
	}
	OutLocation = View.Location;
	return true;
}

bool UHOLUnitScriptLibrary::IsUnitAlive(const UObject* WorldContextObject, const int32 UnitId)
{
	FVector Unused;
	return GetUnitLocation(WorldContextObject, UnitId, Unused);
}

bool UHOLUnitScriptLibrary::GetEnemiesNear(const UObject* WorldContextObject, const int32 UnitId, const float Radius, TArray<int32>& OutIds, TArray<FVector>& OutLocations)
{
	OutIds.Reset();
	OutLocations.Reset();
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	FHOLUnitView Self;
	if (!World || UnitId <= 0 || !HOLUnitView::Find(*World, static_cast<uint32>(UnitId), Self, false))
	{
		return false;
	}
	// The diplomacy book of this machine (the game state): an enemy is a unit of a player the owner is at war with. No game state: any other owner is an enemy.
	const AHOLGameState* State = World->GetGameState<AHOLGameState>();
	TArray<FHOLUnitView> Units;
	HOLUnitView::Gather(*World, Units, false);
	const double RadiusSquared = FMath::Square(static_cast<double>(FMath::Max(Radius, 0.f)));
	for (const FHOLUnitView& Other : Units)
	{
		if (Other.UnitId == Self.UnitId || Other.OwnerPlayer == Self.OwnerPlayer)
		{
			continue;
		}
		const bool bEnemy = State ? State->GetDiplomacyRelation(Self.OwnerPlayer, Other.OwnerPlayer) == HOLDiplomacy::Enemy : true;
		if (bEnemy && FVector::DistSquared(Self.Location, Other.Location) <= RadiusSquared)
		{
			OutIds.Add(static_cast<int32>(Other.UnitId));
			OutLocations.Add(Other.Location);
		}
	}
	return true;
}

float UHOLUnitScriptLibrary::GetAttackRange(const UObject* WorldContextObject, const int32 UnitId)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	FHOLUnitView View;
	if (!World || UnitId <= 0 || !HOLUnitView::Find(*World, static_cast<uint32>(UnitId), View, false))
	{
		return 0.f;
	}
	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	const FHOLUnitTypeRow* Row = UnitTypes && !View.TypeName.IsNone() ? UnitTypes->FindRow<FHOLUnitTypeRow>(View.TypeName, TEXT("HOLUnitScriptLibrary"), false) : nullptr;
	return Row ? Row->AttackRange : 0.f;
}

bool UHOLUnitScriptLibrary::GetUnitOwner(const UObject* WorldContextObject, const int32 UnitId, int32& OutOwnerSlot)
{
	OutOwnerSlot = INDEX_NONE;
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	FHOLUnitView View;
	if (!World || UnitId <= 0 || !HOLUnitView::Find(*World, static_cast<uint32>(UnitId), View, false))
	{
		return false;
	}
	OutOwnerSlot = static_cast<int32>(View.OwnerPlayer);
	return true;
}

FName UHOLUnitScriptLibrary::GetUnitType(const UObject* WorldContextObject, const int32 UnitId)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	FHOLUnitView View;
	if (!World || UnitId <= 0 || !HOLUnitView::Find(*World, static_cast<uint32>(UnitId), View, false))
	{
		return NAME_None;
	}
	return View.TypeName;
}
