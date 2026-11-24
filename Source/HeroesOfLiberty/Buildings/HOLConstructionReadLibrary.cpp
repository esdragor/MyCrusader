// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLConstructionReadLibrary.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Map/HOLMapBounds.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLSimulationFragments.h"
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

FHOLConstructionBox UHOLConstructionReadLibrary::SnapWallPiece(const FVector2D& Point, const FVector2D& Footprint, const double Grid, const int32 SubDivision)
{
	return ConstructionRead_FromBox(HOLConstruction::SnapWallPiece(Point, Footprint, Grid, SubDivision));
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

double UHOLConstructionReadLibrary::GetMinBodySide()
{
	return HOLConstruction::MinBodySide;
}

FVector2D UHOLConstructionReadLibrary::GetBodySize(const FVector2D& Footprint, const double Inset)
{
	return HOLConstruction::GetBodySize(Footprint, Inset);
}

FHOLConstructionBox UHOLConstructionReadLibrary::GetBody(const FHOLConstructionBox& Footprint, const double Inset)
{
	return ConstructionRead_FromBox(HOLConstruction::GetBody(ConstructionRead_ToBox(Footprint), Inset));
}

TArray<FHOLConstructionBox> UHOLConstructionReadLibrary::FindWays(const TArray<FHOLConstructionBox>& Bodies, const double MaxWidth)
{
	const TArray<FBox2D> BodyBoxes = ConstructionRead_ToBoxes(Bodies);
	TArray<FBox2D> Ways;
	HOLConstruction::FindWays(BodyBoxes, MaxWidth, Ways);
	TArray<FHOLConstructionBox> Out;
	for (const FBox2D& Way : Ways)
	{
		Out.Add(ConstructionRead_FromBox(Way));
	}
	return Out;
}

double UHOLConstructionReadLibrary::GetTouchTolerance()
{
	return HOLConstruction::TouchTolerance;
}

double UHOLConstructionReadLibrary::GetSegmentDistance(const FHOLConstructionBox& Footprint, const FVector2D& A, const FVector2D& B)
{
	return HOLConstruction::GetSegmentDistance(ConstructionRead_ToBox(Footprint), A, B);
}

bool UHOLConstructionReadLibrary::DoesWalkCross(const FHOLConstructionBox& Footprint, const double Margin, const TArray<FVector2D>& Points)
{
	return HOLConstruction::DoesWalkCross(ConstructionRead_ToBox(Footprint), Margin, Points);
}

FVector2D UHOLConstructionReadLibrary::PushOutOfFootprint(const FHOLConstructionBox& Footprint, const double Margin, const FVector2D& Point)
{
	return HOLConstruction::PushOutOfFootprint(ConstructionRead_ToBox(Footprint), Margin, Point);
}

double UHOLConstructionReadLibrary::GetKeepOutInsideGap(const double Radius, const double Clearance, const double VisualGap)
{
	return HOLConstruction::GetKeepOutInsideGap(Radius, Clearance, VisualGap);
}

bool UHOLConstructionReadLibrary::KeepOutOfBody(const FVector2D& HalfExtent, const double InsideGap, const double OutsideGap, FVector2D& Local)
{
	return HOLConstruction::KeepOutOfBody(HalfExtent, InsideGap, OutsideGap, Local);
}

int32 UHOLConstructionReadLibrary::FindNextSite(const FHOLConstructionBox& Finished, const TArray<FHOLConstructionBox>& Sites, const double Range)
{
	const TArray<FBox2D> Boxes = ConstructionRead_ToBoxes(Sites);
	return HOLConstruction::FindNextSite(ConstructionRead_ToBox(Finished), Boxes, Range);
}

double UHOLConstructionReadLibrary::GetSourceGap(const FHOLConstructionBox& Footprint, const FVector2D& Centre, const double Radius)
{
	return HOLConstruction::GetSourceGap(ConstructionRead_ToBox(Footprint), Centre, Radius);
}

bool UHOLConstructionReadLibrary::SendsBuildersToGather(const FName TypeName, const TArray<int32>& Accepts)
{
	TArray<EHOLResource> Resources;
	for (const int32 Resource : Accepts)
	{
		Resources.Add(static_cast<EHOLResource>(Resource));
	}
	return HOLConstruction::SendsBuildersToGather(TypeName, Resources);
}

void UHOLConstructionReadLibrary::AddToBuildQueue(TArray<int32>& Queue, const int32 Item, const bool bAppend)
{
	HOLConstruction::AddToBuildQueue<int32>(Queue, Item, bAppend);
}

bool UHOLConstructionReadLibrary::AdvanceBuildQueue(TArray<int32>& Queue, const TArray<int32>& ClosedSites)
{
	return HOLConstruction::AdvanceBuildQueue<int32>(Queue, [&ClosedSites](const int32& Site) { return !ClosedSites.Contains(Site); });
}

int32 UHOLConstructionReadLibrary::MakeWorkBuild(const int32 Site)
{
	return Site * 4;
}

int32 UHOLConstructionReadLibrary::MakeWorkGather(const int32 SourceIndex)
{
	return SourceIndex * 4 + 1;
}

int32 UHOLConstructionReadLibrary::MakeWorkDeposit(const int32 Building)
{
	return Building * 4 + 2;
}

int32 UHOLConstructionReadLibrary::GetWorkItemKind(const int32 Item)
{
	// Decoded to a TWorkItem, whose own predicates say what it is.
	using FItem = HOLConstruction::TWorkItem<int32>;
	const int32 Number = Item >> 2;
	const int32 Code = Item & 3;
	const FItem Decoded = Code == 1 ? FItem::MakeGather(Number) : (Code == 2 ? FItem::MakeDeposit(Number) : FItem::MakeBuild(Number));
	return Decoded.IsGather() ? 1 : (Decoded.IsDeposit() ? 2 : 0);
}

bool UHOLConstructionReadLibrary::AdvanceWorkQueue(TArray<int32>& Queue, const bool bHeadDone, const TArray<int32>& ClosedSites)
{
	using FItem = HOLConstruction::TWorkItem<int32>;
	TArray<FItem> Items;
	for (const int32 Encoded : Queue)
	{
		const int32 Number = Encoded >> 2;
		const int32 Code = Encoded & 3;
		Items.Add(Code == 1 ? FItem::MakeGather(Number) : (Code == 2 ? FItem::MakeDeposit(Number) : FItem::MakeBuild(Number)));
	}
	const bool bLeft = HOLConstruction::AdvanceWorkQueue<int32>(Items, bHeadDone, [&ClosedSites](const int32& Site) { return !ClosedSites.Contains(Site); });
	Queue.Reset();
	for (const FItem& Item : Items)
	{
		Queue.Add(Item.IsGather() ? Item.SourceIndex * 4 + 1 : (Item.IsDeposit() ? Item.Site * 4 + 2 : Item.Site * 4));
	}
	return bLeft;
}

namespace
{
	// A placement query with the arrays it views (unity builds: names unique to this file).
	struct ConstructionRead_FQueryData
	{
		TArray<FBox2D> Buildings;
		TArray<FHOLPlacementCircle> Sources;
		FHOLPlacementQuery Query;
	};

	void ConstructionRead_FillQuery(ConstructionRead_FQueryData& Data, const FHOLConstructionBox& Footprint, const TArray<FHOLConstructionBox>& Buildings,
		const TArray<FVector>& Sources, const FHOLConstructionBox& MapArea, const double AntiRushRadius)
	{
		Data.Buildings = ConstructionRead_ToBoxes(Buildings);
		for (const FVector& Source : Sources)
		{
			Data.Sources.Add(FHOLPlacementCircle{ FVector2D(Source.X, Source.Y), Source.Z });
		}
		Data.Query.Footprint = ConstructionRead_ToBox(Footprint);
		Data.Query.Buildings = Data.Buildings;
		Data.Query.Sources = Data.Sources;
		Data.Query.MapArea = ConstructionRead_ToBox(MapArea);
		Data.Query.AntiRushRadius = AntiRushRadius;
	}

	TArray<EHOLResource> ConstructionRead_ToResources(const TArray<int32>& Values)
	{
		TArray<EHOLResource> Out;
		for (const int32 Value : Values)
		{
			Out.Add(static_cast<EHOLResource>(Value));
		}
		return Out;
	}
}

double UHOLConstructionReadLibrary::GetSettingTileSize()
{
	return UHOLBuildingSettings::GetTileSize();
}

float UHOLConstructionReadLibrary::GetSettingAntiRushRadius()
{
	return GetDefault<UHOLBuildingSettings>()->AntiRushRadius;
}

float UHOLConstructionReadLibrary::GetSettingBuilderExponent()
{
	return GetDefault<UHOLBuildingSettings>()->BuilderExponent;
}

double UHOLConstructionReadLibrary::GetSettingBodyInset()
{
	return UHOLBuildingSettings::GetBodyInset();
}

FVector2D UHOLConstructionReadLibrary::GetSettingBodySize(const FVector2D& Footprint)
{
	return UHOLBuildingSettings::GetBodySize(Footprint);
}

double UHOLConstructionReadLibrary::GetSettingVisualInset()
{
	return UHOLBuildingSettings::GetVisualInset();
}

FVector2D UHOLConstructionReadLibrary::GetSettingVisualSize(const FVector2D& Footprint)
{
	return UHOLBuildingSettings::GetVisualSize(Footprint);
}

bool UHOLConstructionReadLibrary::FindBuildingType(const FName TypeName, FHOLBuildingTypeRow& Out)
{
	if (const FHOLBuildingTypeRow* Row = UHOLBuildingSettings::FindType(TypeName))
	{
		Out = *Row;
		return true;
	}
	return false;
}

bool UHOLConstructionReadLibrary::IsBuildingTypeBuildable(const FHOLBuildingTypeRow& Row)
{
	return Row.IsBuildable();
}

int32 UHOLConstructionReadLibrary::GetBuildingTypeCategory(const FHOLBuildingTypeRow& Row)
{
	return static_cast<int32>(Row.Category);
}

TArray<int32> UHOLConstructionReadLibrary::GetBuildingTypeAccepts(const FHOLBuildingTypeRow& Row)
{
	TArray<int32> Out;
	for (const EHOLResource Resource : Row.Accepts)
	{
		Out.Add(static_cast<int32>(Resource));
	}
	return Out;
}

bool UHOLConstructionReadLibrary::FindSourceTypeInfo(const FName SourceName, bool& bLinked, int32& Resource)
{
	if (const FHOLSourceTypeRow* Row = UHOLSourceSettings::FindType(SourceName))
	{
		bLinked = Row->Linked;
		Resource = static_cast<int32>(Row->Resource);
		return true;
	}
	return false;
}

int32 UHOLConstructionReadLibrary::FindCampSource(const FHOLConstructionBox& Camp, const TArray<int32>& Accepts, const TArray<FHOLConstructionCampSource>& Sources,
	const double Range, double& OutGap)
{
	TArray<FHOLCampSource> Converted;
	for (const FHOLConstructionCampSource& Source : Sources)
	{
		Converted.Add(FHOLCampSource{ Source.Centre, Source.Radius, static_cast<EHOLResource>(Source.Resource), Source.bStanding });
	}
	const TArray<EHOLResource> Resources = ConstructionRead_ToResources(Accepts);
	return HOLConstruction::FindCampSource(ConstructionRead_ToBox(Camp), Resources, Converted, Range, &OutGap);
}

bool UHOLConstructionReadLibrary::StandsClearOfVisibleWall(const int32 Activity, const int32 Phase, const bool bFighting)
{
	return HOLConstruction::StandsClearOfVisibleWall(static_cast<EHOLUnitActivity>(Activity), static_cast<EHOLMovePhase>(Phase), bFighting);
}

bool UHOLConstructionReadLibrary::KeepOutOfVisibleWall(const FVector2D& HalfExtent, const double InsideGap, const double OutsideGap, const double Radius,
	const double VisualGap, const bool bStandsStill, const double MaxStep, FVector2D& Local, bool& bOutEased)
{
	return HOLConstruction::KeepOutOfVisibleWall(HalfExtent, InsideGap, OutsideGap, Radius, VisualGap, bStandsStill, MaxStep, Local, &bOutEased);
}

bool UHOLConstructionReadLibrary::EaseOutOfVisibleWalls(const TArray<FHOLConstructionWallBody>& Bodies, const double Radius, const double VisualGap,
	const double MaxStep, FVector2D& Position, const bool bSquareCorners)
{
	TArray<HOLConstruction::FWallBody> Converted;
	for (const FHOLConstructionWallBody& Body : Bodies)
	{
		HOLConstruction::FWallBody Wall;
		Wall.Centre = Body.Centre;
		Wall.HalfExtent = Body.HalfExtent;
		Wall.Cos = Body.Cos;
		Wall.Sin = Body.Sin;
		Converted.Add(Wall);
	}
	return HOLConstruction::EaseOutOfVisibleWalls(Converted, Radius, VisualGap, MaxStep, Position, bSquareCorners);
}

FHOLConstructionBox UHOLConstructionReadLibrary::SnapOutward(const FHOLConstructionBox& Box, const double Grid)
{
	return ConstructionRead_FromBox(HOLConstruction::SnapOutward(ConstructionRead_ToBox(Box), Grid));
}

bool UHOLConstructionReadLibrary::FindObstacle(const FHOLConstructionBox& Footprint, const TArray<FHOLConstructionBox>& Buildings, const TArray<FVector>& Sources,
	const FHOLConstructionBox& MapArea, const double AntiRushRadius, const FVector2D& Aim, const double Grid, FHOLConstructionBox& OutObstacle)
{
	ConstructionRead_FQueryData Data;
	ConstructionRead_FillQuery(Data, Footprint, Buildings, Sources, MapArea, AntiRushRadius);
	FBox2D Obstacle(ForceInit);
	const bool bFound = HOLConstruction::FindObstacle(Data.Query, Aim, Grid, Obstacle);
	OutObstacle = ConstructionRead_FromBox(Obstacle);
	return bFound;
}

bool UHOLConstructionReadLibrary::FindFreeFootprint(const FHOLConstructionBox& Footprint, const TArray<FHOLConstructionBox>& Buildings, const TArray<FVector>& Sources,
	const FHOLConstructionBox& MapArea, const double AntiRushRadius, const FHOLConstructionBox& Obstacle, const FVector2D& Aim, const double Grid, const int32 MaxRings,
	const TArray<FHOLConstructionBox>& OffNavMesh, FHOLConstructionBox& OutFootprint, int32& OutRing)
{
	ConstructionRead_FQueryData Data;
	ConstructionRead_FillQuery(Data, Footprint, Buildings, Sources, MapArea, AntiRushRadius);
	const TArray<FBox2D> Off = ConstructionRead_ToBoxes(OffNavMesh);
	const auto IsOnNavMesh = [&Off](const FBox2D& Candidate)
	{
		const FVector2D Middle = Candidate.GetCenter();
		for (const FBox2D& Box : Off)
		{
			if (Box.IsInside(Middle))
			{
				return false;
			}
		}
		return true;
	};
	FBox2D Free(ForceInit);
	int32 Ring = -1;
	const bool bFound = HOLConstruction::FindFreeFootprint(Data.Query, ConstructionRead_ToBox(Obstacle), Aim, Grid, MaxRings, IsOnNavMesh, Free, &Ring);
	OutFootprint = ConstructionRead_FromBox(Free);
	OutRing = bFound ? Ring : -1;
	return bFound;
}

// NON COMPILE (F4 wall line): see the header. The last cell of a box is the one holding its far edge minus a hair: touching is fine (D311), so a box ending on a cell border does not take that cell.
double UHOLConstructionReadLibrary::GetWallLineWorld(const UObject* WorldContextObject, FIntPoint& OutMapMin, FIntPoint& OutMapMax, TArray<FIntPoint>& OutOccupied)
{
	const double Tile = UHOLBuildingSettings::GetTileSize();
	OutMapMin = FIntPoint(-100000, -100000);
	OutMapMax = FIntPoint(100000, 100000);
	OutOccupied.Reset();
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World)
	{
		return Tile;
	}
	if (const AHOLMapBounds* Bounds = AHOLMapBounds::Find(World))
	{
		const FBox2D Area = Bounds->GetArea();
		if (Area.bIsValid)
		{
			OutMapMin = FIntPoint(FMath::FloorToInt(Area.Min.X / Tile), FMath::FloorToInt(Area.Min.Y / Tile));
			OutMapMax = FIntPoint(FMath::CeilToInt(Area.Max.X / Tile) - 1, FMath::CeilToInt(Area.Max.Y / Tile) - 1);
		}
	}
	for (FHOLBuildingIterator It(World); It; ++It)
	{
		if (!It->GetType())
		{
			continue;
		}
		const FBox2D Box = It->GetFootprintBounds();
		const int32 MinX = FMath::FloorToInt(Box.Min.X / Tile);
		const int32 MinY = FMath::FloorToInt(Box.Min.Y / Tile);
		const int32 MaxX = FMath::CeilToInt(Box.Max.X / Tile) - 1;
		const int32 MaxY = FMath::CeilToInt(Box.Max.Y / Tile) - 1;
		for (int32 Y = MinY; Y <= MaxY; ++Y)
		{
			for (int32 X = MinX; X <= MaxX; ++X)
			{
				OutOccupied.AddUnique(FIntPoint(X, Y));
			}
		}
	}
	return Tile;
}