// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIOrders.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Commands/HOLPlayerOrders.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
#include "HeroesOfLibertyLog.h"

namespace
{
	/** The world of WorldContextObject when it holds the authority (server, solo); null on a client. */
	UWorld* GetAuthorityWorld(const UObject* WorldContextObject)
	{
		UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
		if (World && World->GetNetMode() == NM_Client)
		{
			UE_LOG(LogHOLMatch, Warning, TEXT("AI order refused on a client: the AI runs on the server"));
			return nullptr;
		}
		return World;
	}

	/** PlayerIndex's building nearest Location, within 100 cm (a summary's FHOLAIBuilding::Location); null if none. */
	AHOLBuilding* FindOwnBuilding(const UWorld& World, const int32 PlayerIndex, const FVector& Location)
	{
		AHOLBuilding* Nearest = nullptr;
		double NearestDistance = 100.0;
		for (TActorIterator<AHOLBuilding> It(&World); It; ++It)
		{
			const double Distance = FVector::Dist2D(It->GetActorLocation(), Location);
			if (It->GetOwnerPlayer() == PlayerIndex && Distance <= NearestDistance)
			{
				Nearest = *It;
				NearestDistance = Distance;
			}
		}
		return Nearest;
	}

	/** A point of the footprint's middle on the ground. */
	FVector ToPoint(const UWorld& World, const FBox2D& Footprint, const double DefaultZ)
	{
		const FVector2D Middle = Footprint.GetCenter();
		return FVector(Middle, UHOLConstructionSubsystem::FindGroundZ(World, Middle, DefaultZ));
	}

	/** Script's int32 identifiers back to the simulation's uint32 (a negative one names no unit: 0). */
	TArray<uint32> ToUnitIds(const TArray<int32>& UnitIds)
	{
		TArray<uint32> Ids;
		Ids.Reserve(UnitIds.Num());
		for (const int32 Id : UnitIds)
		{
			Ids.Add(Id > 0 ? static_cast<uint32>(Id) : 0u);
		}
		return Ids;
	}
}

bool UHOLAIOrderLibrary::IssueAIGatherOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const TArray<int32>& UnitIds, const int32 SourceIndex,
	const bool bQueue)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	return World && HOLPlayerOrders::Gather(*World, PlayerIndex, ToUnitIds(UnitIds), SourceIndex, bQueue);
}

bool UHOLAIOrderLibrary::IssueAIMoveOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const TArray<int32>& UnitIds, const FVector Target,
	const bool bQueue)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	return World && HOLPlayerOrders::Move(*World, PlayerIndex, ToUnitIds(UnitIds), Target, bQueue);
}

int32 UHOLAIOrderLibrary::CheckAIPlacement(const UObject* WorldContextObject, const int32 PlayerIndex, const FName TypeName, const FVector Point)
{
	const UWorld* World = GetAuthorityWorld(WorldContextObject);
	const FHOLBuildingTypeRow* Type = TypeName.IsNone() ? nullptr : UHOLBuildingSettings::FindType(TypeName);
	if (!World || !Type || !Type->IsBuildable())
	{
		return -1;
	}
	return static_cast<int32>(UHOLConstructionSubsystem::CheckPlacement(*World, UHOLConstructionSubsystem::SnapFootprint(*Type, Point), PlayerIndex));
}

bool UHOLAIOrderLibrary::FindAIPlacement(const UObject* WorldContextObject, const int32 PlayerIndex, const FName TypeName, const FVector Centre,
	const int32 MaxRings, FVector& OutPoint)
{
	const UWorld* World = GetAuthorityWorld(WorldContextObject);
	const FHOLBuildingTypeRow* Type = TypeName.IsNone() ? nullptr : UHOLBuildingSettings::FindType(TypeName);
	FBox2D Footprint;
	if (!World || !Type || !Type->IsBuildable()
		|| !UHOLConstructionSubsystem::FindRingFootprint(*World, *Type, FVector2D(Centre), PlayerIndex, MaxRings, {}, Footprint))
	{
		return false;
	}
	OutPoint = ToPoint(*World, Footprint, Centre.Z);
	return true;
}

bool UHOLAIOrderLibrary::FindAIFarmBlock(const UObject* WorldContextObject, const int32 PlayerIndex, const FVector Centre, const int32 MaxRings,
	FVector& OutMill, TArray<FVector>& OutFarms)
{
	const UWorld* World = GetAuthorityWorld(WorldContextObject);
	const FHOLBuildingTypeRow* MillType = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Mill);
	const FHOLBuildingTypeRow* FarmType = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Farm);
	FBox2D Mill;
	TArray<FBox2D> Farms;
	if (!World || !MillType || !FarmType
		|| !UHOLConstructionSubsystem::FindFarmBlock(*World, *MillType, *FarmType, FVector2D(Centre), PlayerIndex, MaxRings, Mill, Farms))
	{
		return false;
	}
	OutMill = ToPoint(*World, Mill, Centre.Z);
	OutFarms.Reset();
	for (const FBox2D& Farm : Farms)
	{
		OutFarms.Add(ToPoint(*World, Farm, Centre.Z));
	}
	return true;
}

bool UHOLAIOrderLibrary::IssueAIPlaceOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const FName TypeName, const FVector Point,
	const TArray<int32>& UnitIds, const bool bQueue)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	if (!World || !HOLPlayerOrders::IsAccepted(PlayerIndex, 0))
	{
		return false;
	}
	// What the human's ghost shows before it sends the order: the placement checked here, the server (the construction at its
	// step) checks it again and takes the cost.
	const int32 Refusal = CheckAIPlacement(WorldContextObject, PlayerIndex, TypeName, Point);
	if (Refusal != 0)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("AI order of slot %d: %s not placed at (%.0f, %.0f): %s"), PlayerIndex, *TypeName.ToString(), Point.X, Point.Y,
			Refusal < 0 ? TEXT("not a building to place") : UHOLConstructionSubsystem::DescribeRefusal(static_cast<EHOLPlacementRefusal>(Refusal)));
		return false;
	}
	return HOLPlayerOrders::Place(*World, PlayerIndex, TypeName, Point, ToUnitIds(UnitIds), bQueue);
}

bool UHOLAIOrderLibrary::IssueAIBuildOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const FVector BuildingLocation,
	const TArray<int32>& UnitIds, const bool bQueue)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	AHOLBuilding* Building = World ? FindOwnBuilding(*World, PlayerIndex, BuildingLocation) : nullptr;
	return Building && HOLPlayerOrders::Build(*World, PlayerIndex, ToUnitIds(UnitIds), Building, bQueue);
}

bool UHOLAIOrderLibrary::IssueAIProduceOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const FVector BuildingLocation, const FName TypeName)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	AHOLBuilding* Building = World ? FindOwnBuilding(*World, PlayerIndex, BuildingLocation) : nullptr;
	return Building && HOLPlayerOrders::QueueProduction(*World, PlayerIndex, Building, TypeName);
}

bool UHOLAIOrderLibrary::IssueAISetRallyOrder(const UObject* WorldContextObject, const int32 PlayerIndex, const FVector BuildingLocation, const FVector Point,
	const int32 SourceIndex)
{
	UWorld* World = GetAuthorityWorld(WorldContextObject);
	AHOLBuilding* Building = World ? FindOwnBuilding(*World, PlayerIndex, BuildingLocation) : nullptr;
	return Building && HOLPlayerOrders::SetRally(*World, PlayerIndex, Building, Point, SourceIndex);
}

bool UHOLAIOrderLibrary::ReadAIUnitFigures(const UObject* WorldContextObject, const FName TypeName, FHOLAITypeFigures& OutFigures)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	const FHOLUnitTypeRow* Row = (Units && !TypeName.IsNone()) ? Units->FindTypeRow(TypeName) : nullptr;
	if (!Row)
	{
		return false;
	}
	OutFigures = FHOLAITypeFigures();
	OutFigures.CostFood = Row->Cost.Food;
	OutFigures.CostWood = Row->Cost.Wood;
	OutFigures.CostStone = Row->Cost.Stone;
	OutFigures.CostGold = Row->Cost.Gold;
	OutFigures.BuildTime = Row->BuildTime;
	OutFigures.Population = Row->Population;
	OutFigures.Speed = Row->Speed;
	OutFigures.CarryCapacity = Row->CarryCapacity;
	OutFigures.GatherFood = Row->GatherRates.Food;
	OutFigures.GatherWood = Row->GatherRates.Wood;
	OutFigures.GatherStone = Row->GatherRates.Stone;
	OutFigures.GatherGold = Row->GatherRates.Gold;
	return true;
}

bool UHOLAIOrderLibrary::ReadAIBuildingFigures(const FName TypeName, FHOLAITypeFigures& OutFigures)
{
	const FHOLBuildingTypeRow* Row = TypeName.IsNone() ? nullptr : UHOLBuildingSettings::FindType(TypeName);
	if (!Row)
	{
		return false;
	}
	OutFigures = FHOLAITypeFigures();
	OutFigures.CostFood = Row->Cost.Food;
	OutFigures.CostWood = Row->Cost.Wood;
	OutFigures.CostStone = Row->Cost.Stone;
	OutFigures.CostGold = Row->Cost.Gold;
	OutFigures.BuildTime = Row->BuildTime;
	OutFigures.Population = Row->PopulationProvided;
	return true;
}
