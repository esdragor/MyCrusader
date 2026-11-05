// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLVolcanoLayout.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

AHOLVolcanoLayout::AHOLVolcanoLayout()
{
	PrimaryActorTick.bCanEverTick = false;
	// The sites are the simulation's data, but the clients draw the placeholder altar from the map's own copy (2026-11-04).
	bNetLoadOnClient = true;
	SetCanBeDamaged(false);
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AHOLVolcanoLayout::BeginPlay()
{
	Super::BeginPlay();
	if (GetNetMode() != NM_DedicatedServer)
	{
		BuildAltarMeshes();
	}
}

void AHOLVolcanoLayout::BuildAltarMeshes()
{
	// A placeholder, assumed (2026-11-04, the user: a mesh for the altar so that it shows on the maps): two basalt steps, a pillar and a red top,
	// made of the engine's basic shapes, no collision, no navigation, no shadow (D258, D265).
	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UMaterialInterface* Parent = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (!Cylinder || !Cube)
	{
		return;
	}
	UMaterialInstanceDynamic* Basalt = Parent ? UMaterialInstanceDynamic::Create(Parent, this) : nullptr;
	UMaterialInstanceDynamic* Ember = Parent ? UMaterialInstanceDynamic::Create(Parent, this) : nullptr;
	if (Basalt)
	{
		Basalt->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.04f, 0.04f, 0.05f));
	}
	if (Ember)
	{
		Ember->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.9f, 0.22f, 0.03f));
	}
	auto AddPart = [this](UStaticMesh* Mesh, UMaterialInterface* Material, const FVector& Where, const FVector& Scale)
	{
		UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this);
		Part->SetStaticMesh(Mesh);
		if (Material)
		{
			Part->SetMaterial(0, Material);
		}
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Part->SetCanEverAffectNavigation(false);
		Part->SetCastShadow(false);
		Part->SetMobility(EComponentMobility::Movable);
		Part->SetupAttachment(GetRootComponent());
		Part->RegisterComponent();
		Part->SetWorldLocationAndRotation(Where, FRotator::ZeroRotator);
		Part->SetWorldScale3D(Scale);
		AltarMeshes.Add(Part);
	};
	for (const FHOLPlacedVolcano& Site : Sites)
	{
		if (!Site.bAltar)
		{
			continue;
		}
		const FVector At = Site.AltarLocation;
		// The shapes are 100 cm: a 7 m step 30 cm high, a 4.5 m step 30 cm high, a 1.2 m pillar 3.4 m high, a 2 m red top 30 cm high.
		AddPart(Cylinder, Basalt, At + FVector(0, 0, 15), FVector(7.0, 7.0, 0.3));
		AddPart(Cylinder, Basalt, At + FVector(0, 0, 45), FVector(4.5, 4.5, 0.3));
		AddPart(Cube, Basalt, At + FVector(0, 0, 60 + 170), FVector(1.2, 1.2, 3.4));
		AddPart(Cylinder, Ember, At + FVector(0, 0, 60 + 340 + 15), FVector(2.0, 2.0, 0.3));
	}
}

int32 FHOLVolcanoLayoutRules::CountUnbalancedFlows(const TConstArrayView<FHOLPlacedVolcano> Sites, const double StepDegrees)
{
	const double Radians = FMath::DegreesToRadians(StepDegrees);
	const double Cos = FMath::Cos(Radians);
	const double Sin = FMath::Sin(Radians);
	int32 Unmatched = 0;
	for (const FHOLPlacedVolcano& Site : Sites)
	{
		for (const FHOLPlacedLavaFlow& Flow : Site.Flows)
		{
			bool bMatched = false;
			for (const FHOLPlacedLavaFlow& Other : Site.Flows)
			{
				bool bSame = Other.Points.Num() == Flow.Points.Num();
				for (int32 Index = 0; bSame && Index < Flow.Points.Num(); ++Index)
				{
					const double TurnedX = Flow.Points[Index].X * Cos - Flow.Points[Index].Y * Sin;
					const double TurnedY = Flow.Points[Index].X * Sin + Flow.Points[Index].Y * Cos;
					bSame = FMath::Abs(Other.Points[Index].X - TurnedX) <= SymmetryTolerance && FMath::Abs(Other.Points[Index].Y - TurnedY) <= SymmetryTolerance;
				}
				if (bSame)
				{
					bMatched = true;
					break;
				}
			}
			Unmatched += bMatched ? 0 : 1;
		}
	}
	return Unmatched;
}

double FHOLVolcanoLayoutRules::AltarSpread(const FHOLPlacedVolcano& Site, const TConstArrayView<FVector2D> SlotCentres)
{
	if (!Site.bAltar || SlotCentres.Num() < 2)
	{
		return 0.;
	}
	double Nearest = TNumericLimits<double>::Max();
	double Farthest = 0.;
	for (const FVector2D& Slot : SlotCentres)
	{
		const double Distance = FVector2D::Distance(FVector2D(Site.AltarLocation.X, Site.AltarLocation.Y), Slot);
		Nearest = FMath::Min(Nearest, Distance);
		Farthest = FMath::Max(Farthest, Distance);
	}
	return Farthest - Nearest;
}

int32 FHOLVolcanoLayoutRules::CountUnfairAltars(const TConstArrayView<FHOLPlacedVolcano> Sites, const TConstArrayView<FVector2D> SlotCentres)
{
	int32 Count = 0;
	for (const FHOLPlacedVolcano& Site : Sites)
	{
		Count += AltarSpread(Site, SlotCentres) > SymmetryTolerance ? 1 : 0;
	}
	return Count;
}

int32 FHOLVolcanoLayoutRules::CountEmptyFlows(const TConstArrayView<FHOLPlacedVolcano> Sites)
{
	int32 Count = 0;
	for (const FHOLPlacedVolcano& Site : Sites)
	{
		Count += Site.Flows.IsEmpty() ? 1 : 0;
		for (const FHOLPlacedLavaFlow& Flow : Site.Flows)
		{
			HOLVolcano::FFlow Converted;
			Converted.Points = Flow.Points;
			Count += Converted.Length() > 0.f ? 0 : 1;
		}
	}
	return Count;
}

int32 FHOLVolcanoLayoutRules::CountOutside(const TConstArrayView<FHOLPlacedVolcano> Sites, const double HalfSize)
{
	int32 Count = 0;
	for (const FHOLPlacedVolcano& Site : Sites)
	{
		for (const FHOLPlacedLavaFlow& Flow : Site.Flows)
		{
			for (const FVector& Point : Flow.Points)
			{
				Count += FMath::Abs(Point.X) > HalfSize || FMath::Abs(Point.Y) > HalfSize ? 1 : 0;
			}
		}
		if (Site.bAltar)
		{
			Count += FMath::Abs(Site.AltarLocation.X) + Site.AltarRadius > HalfSize || FMath::Abs(Site.AltarLocation.Y) + Site.AltarRadius > HalfSize ? 1 : 0;
		}
	}
	return Count;
}

TArray<HOLVolcano::FSite> FHOLVolcanoLayoutRules::ToSites(const TConstArrayView<FHOLPlacedVolcano> Placed)
{
	TArray<HOLVolcano::FSite> Sites;
	for (const FHOLPlacedVolcano& Site : Placed)
	{
		HOLVolcano::FSite& Converted = Sites.AddDefaulted_GetRef();
		for (const FHOLPlacedLavaFlow& Flow : Site.Flows)
		{
			Converted.Flows.AddDefaulted_GetRef().Points = Flow.Points;
		}
		Converted.Altar.bPresent = Site.bAltar;
		Converted.Altar.Location = Site.AltarLocation;
		Converted.Altar.Radius = Site.AltarRadius;
	}
	return Sites;
}
