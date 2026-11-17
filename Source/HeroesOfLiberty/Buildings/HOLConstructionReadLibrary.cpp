// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLConstructionReadLibrary.h"

#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Simulation/HOLUnitTypes.h"

namespace
{
	// Names unique to this file (unity builds merge the anonymous namespaces of the module).
	FBox2D ConstructionRead_ToBox(const FHOLConstructionBox& Box)
	{
		return Box.bValid ? FBox2D(Box.Min, Box.Max) : FBox2D(ForceInit);
	}

	FHOLConstructionBox ConstructionRead_FromBox(const FBox2D& Box)
	{
		FHOLConstructionBox Out;
		Out.Min = Box.Min;
		Out.Max = Box.Max;
		Out.bValid = Box.bIsValid != 0;
		return Out;
	}

	TArray<FBox2D> ConstructionRead_ToBoxes(const TArray<FHOLConstructionBox>& Boxes)
	{
		TArray<FBox2D> Out;
		Out.Reserve(Boxes.Num());
		for (const FHOLConstructionBox& Box : Boxes)
		{
			Out.Add(ConstructionRead_ToBox(Box));
		}
		return Out;
	}
}

FHOLConstructionBox UHOLConstructionReadLibrary::MakeBox(const FVector2D& Min, const FVector2D& Max)
{
	FHOLConstructionBox Out;
	Out.Min = Min;
	Out.Max = Max;
	Out.bValid = true;
	return Out;
}

FHOLConstructionBox UHOLConstructionReadLibrary::SnapToGrid(const FVector2D& Point, const FVector2D& Footprint, const double Grid)
{
	return ConstructionRead_FromBox(HOLConstruction::SnapToGrid(Point, Footprint, Grid));
}

float UHOLConstructionReadLibrary::SnapYaw(const float Yaw)
{
	return HOLConstruction::SnapYaw(Yaw);
}

FHOLConstructionBox UHOLConstructionReadLibrary::MakeFootprint(const FVector2D& Centre, const FVector2D& Footprint)
{
	return ConstructionRead_FromBox(HOLConstruction::MakeFootprint(Centre, Footprint));
}

bool UHOLConstructionReadLibrary::Overlaps(const FHOLConstructionBox& A, const FHOLConstructionBox& B)
{
	return HOLConstruction::Overlaps(ConstructionRead_ToBox(A), ConstructionRead_ToBox(B));
}

double UHOLConstructionReadLibrary::GetDistance(const FHOLConstructionBox& Footprint, const FVector2D& Point)
{
	return HOLConstruction::GetDistance(ConstructionRead_ToBox(Footprint), Point);
}

double UHOLConstructionReadLibrary::GetGap(const FHOLConstructionBox& A, const FHOLConstructionBox& B)
{
	return HOLConstruction::GetGap(ConstructionRead_ToBox(A), ConstructionRead_ToBox(B));
}

int32 UHOLConstructionReadLibrary::CheckPlacement(const FHOLConstructionBox& Footprint, const TArray<FHOLConstructionBox>& Buildings, const TArray<FVector>& Sources,
	const FHOLConstructionBox& MapArea, const TArray<FVector2D>& EnemyMainCentres, const double AntiRushRadius, const bool bOnNavMesh)
{
	const TArray<FBox2D> BuildingBoxes = ConstructionRead_ToBoxes(Buildings);
	TArray<FHOLPlacementCircle> Circles;
	for (const FVector& Source : Sources)
	{
		FHOLPlacementCircle& Circle = Circles.AddDefaulted_GetRef();
		Circle.Centre = FVector2D(Source.X, Source.Y);
		Circle.Radius = Source.Z;
	}
	FHOLPlacementQuery Query;
	Query.Footprint = ConstructionRead_ToBox(Footprint);
	Query.Buildings = BuildingBoxes;
	Query.Sources = Circles;
	Query.MapArea = ConstructionRead_ToBox(MapArea);
	Query.EnemyMainCentres = EnemyMainCentres;
	Query.AntiRushRadius = AntiRushRadius;
	Query.bOnNavMesh = bOnNavMesh;
	return int32(HOLConstruction::CheckPlacement(Query));
}

bool UHOLConstructionReadLibrary::IsAgeOpen(const int32 BuildingAge, const int32 Tier)
{
	return HOLConstruction::IsAgeOpen(BuildingAge, Tier);
}

bool UHOLConstructionReadLibrary::TryPay(FHOLResources& Stock, const FHOLResources& Cost)
{
	return HOLConstruction::TryPay(Stock, Cost);
}

FHOLResources UHOLConstructionReadLibrary::GetRefund(const FHOLResources& Cost, const int64 Work, const int64 TotalWork)
{
	return HOLConstruction::GetRefund(Cost, Work, TotalWork);
}

int32 UHOLConstructionReadLibrary::GetBuildSteps(const float BuildTime, const float StepSeconds)
{
	return HOLConstruction::GetBuildSteps(BuildTime, StepSeconds);
}

int64 UHOLConstructionReadLibrary::GetTotalWork(const int32 BuildSteps)
{
	return HOLConstruction::GetTotalWork(BuildSteps);
}

int32 UHOLConstructionReadLibrary::GetWorkPerBuilder()
{
	return HOLConstruction::WorkPerBuilder;
}

int32 UHOLConstructionReadLibrary::GetWorkPerStep(const int32 Builders, const float Exponent)
{
	return HOLConstruction::GetWorkPerStep(Builders, Exponent);
}

int32 UHOLConstructionReadLibrary::GetStepsToFinish(const int32 BuildSteps, const int32 Builders, const float Exponent)
{
	return HOLConstruction::GetStepsToFinish(BuildSteps, Builders, Exponent);
}

bool UHOLConstructionReadLibrary::AddWork(int64& Work, const int64 TotalWork, const int32 Builders, const float Exponent)
{
	return HOLConstruction::AddWork(Work, TotalWork, Builders, Exponent);
}

float UHOLConstructionReadLibrary::GetProgress(const int64 Work, const int64 TotalWork)
{
	return HOLConstruction::GetProgress(Work, TotalWork);
}

int32 UHOLConstructionReadLibrary::GetHealth(const int32 MaxHealth, const int64 Work, const int64 TotalWork)
{
	return HOLConstruction::GetHealth(MaxHealth, Work, TotalWork);
}

int32 UHOLConstructionReadLibrary::GetHealthWithDamage(const int32 MaxHealth, const int64 Work, const int64 TotalWork, const int32 Damage)
{
	return HOLConstruction::GetHealthWithDamage(MaxHealth, Work, TotalWork, Damage);
}

int32 UHOLConstructionReadLibrary::GetBuildingDamage(const int32 Damage, const int32 AttackType, const int32 RangedArmor)
{
	return HOLConstruction::GetBuildingDamage(Damage, static_cast<EHOLAttackType>(AttackType), RangedArmor);
}

int32 UHOLConstructionReadLibrary::GetPopulationProvided(const int32 TypePopulation, const bool bFinished)
{
	return HOLConstruction::GetPopulationProvided(TypePopulation, bFinished);
}

bool UHOLConstructionReadLibrary::BlocksNavigation(const int32 State, const bool bWalkable)
{
	return HOLConstruction::BlocksNavigation(static_cast<EHOLConstructionState>(State), bWalkable);
}

bool UHOLConstructionReadLibrary::IsServing(const int32 State)
{
	return HOLConstruction::IsServing(static_cast<EHOLConstructionState>(State));
}

bool UHOLConstructionReadLibrary::CanCancel(const int32 State)
{
	return HOLConstruction::CanCancel(static_cast<EHOLConstructionState>(State));
}

bool UHOLConstructionReadLibrary::HasWorkStarted(const int32 State, const int64 Work)
{
	return HOLConstruction::HasWorkStarted(static_cast<EHOLConstructionState>(State), Work);
}

FHOLResources UHOLConstructionReadLibrary::GetCancelRefund(const FHOLResources& Cost, const int32 State, const int64 Work, const int64 TotalWork)
{
	return HOLConstruction::GetCancelRefund(Cost, static_cast<EHOLConstructionState>(State), Work, TotalWork);
}

FHOLConstructionBox UHOLConstructionReadLibrary::GetFootprintBounds(const FVector2D& Centre, const FVector2D& Footprint, const float Yaw)
{
	return ConstructionRead_FromBox(HOLConstruction::GetFootprintBounds(Centre, Footprint, Yaw));
}

TArray<FVector2D> UHOLConstructionReadLibrary::GetNavProbes(const FHOLConstructionBox& Footprint, const double Tile, const double CornerInset)
{
	TArray<FVector2D> Points;
	HOLConstruction::GetNavProbes(ConstructionRead_ToBox(Footprint), Tile, CornerInset, Points);
	return Points;
}

TArray<FVector2D> UHOLConstructionReadLibrary::GetBuildSpots(const FHOLConstructionBox& Footprint, const double Offset, const double Spacing, const FVector2D& From)
{
	TArray<FVector2D> Spots;
	HOLConstruction::GetBuildSpots(ConstructionRead_ToBox(Footprint), Offset, Spacing, From, Spots);
	return Spots;
}

bool UHOLConstructionReadLibrary::IsAtSite(const FHOLConstructionBox& Footprint, const FVector2D& Position, const double BodyRadius, const double Gap)
{
	return HOLConstruction::IsAtSite(ConstructionRead_ToBox(Footprint), Position, BodyRadius, Gap);
}

int32 UHOLConstructionReadLibrary::FindNextSite(const FHOLConstructionBox& Finished, const TArray<FHOLConstructionBox>& Sites, const double Range)
{
	const TArray<FBox2D> Boxes = ConstructionRead_ToBoxes(Sites);
	return HOLConstruction::FindNextSite(ConstructionRead_ToBox(Finished), Boxes, Range);
}
