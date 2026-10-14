// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLSelectionMarkers.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLPlayerColors.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Player/HOLPlayerController.h"
#include "Simulation/HOLUnitView.h"

namespace UE::HeroesOfLiberty::SelectionMarkers
{
	// The engine's 1 m plane (facing up, UVs 0 to 1) and cube, and the materials of make_marker_materials.py.
	const TCHAR* PlanePath = TEXT("/Engine/BasicShapes/Plane.Plane");
	const TCHAR* CubePath = TEXT("/Engine/BasicShapes/Cube.Cube");
	const TCHAR* RingMaterialPath = TEXT("/Game/Rendering/M_GroundRing.M_GroundRing");
	const TCHAR* FrameMaterialPath = TEXT("/Game/Rendering/M_GroundFrame.M_GroundFrame");
	const TCHAR* FlatMaterialPath = TEXT("/Game/Rendering/M_FlatColor.M_FlatColor");
	/** Size (cm) of the engine's basic shapes. */
	constexpr double ShapeSize = 100.0;

	/** Custom data of a marker instance: colour (3), stroke as a share of the half-size (1). */
	constexpr int32 NumCustomData = 4;
	/** Above the ground (cm), so that the plane does not fight with it. */
	constexpr double GroundOffset = 3.0;

	/** The selection's rings and frame: a neutral white, whoever the owner (the user's trial of J4). */
	const FLinearColor SelectionColor(0.85f, 0.85f, 0.85f);
	/** A unit's ring: its radius over the body's, and its stroke (cm). */
	constexpr double RingRadiusFactor = 1.4;
	constexpr double RingStroke = 12.0;
	/** The building's frame: its margin around the footprint and its stroke (cm). */
	constexpr double FrameMargin = 100.0;
	constexpr double FrameStroke = 30.0;
	/** The order's ring (D288; halved in size and time at the user's trial): it shrinks from the first radius to the second (cm). */
	constexpr float PingSeconds = 0.25f;
	constexpr double PingStartRadius = 90.0;
	constexpr double PingEndRadius = 25.0;
	constexpr double PingStroke = 8.0;
	const FLinearColor PingColor(0.9f, 0.9f, 0.9f);
	/** The rally flag (cm): a pole, and a cloth at its top at the owner's colour. */
	constexpr double PoleHeight = 500.0;
	constexpr double PoleWidth = 10.0;
	const FVector ClothSize(6.0, 160.0, 100.0);
	const FLinearColor PoleColor(0.12f, 0.08f, 0.04f);

	UStaticMesh* LoadMesh(const TCHAR* Path)
	{
		UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Path);
		UE_CLOG(!Mesh, LogHOLMatch, Error, TEXT("Selection markers: mesh %s missing"), Path);
		return Mesh;
	}

	UMaterialInterface* LoadMaterial(const TCHAR* Path)
	{
		UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, Path);
		UE_CLOG(!Material, LogHOLMatch, Error, TEXT("Selection markers: material %s missing (Tools/Editor/make_marker_materials.py)"), Path);
		return Material;
	}

	UInstancedStaticMeshComponent* MakeMarkerComponent(AActor& Owner, const FName Name)
	{
		UInstancedStaticMeshComponent* Component = Owner.CreateDefaultSubobject<UInstancedStaticMeshComponent>(Name);
		Component->SetupAttachment(Owner.GetRootComponent());
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetCastShadow(false);
		Component->SetReceivesDecals(false);
		Component->NumCustomDataFloats = NumCustomData;
		return Component;
	}

	UStaticMeshComponent* MakeFlagComponent(AActor& Owner, const FName Name)
	{
		UStaticMeshComponent* Component = Owner.CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Component->SetupAttachment(Owner.GetRootComponent());
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetCastShadow(false);
		Component->SetHiddenInGame(true);
		return Component;
	}

	/** A marker plane lying on the ground, centred on Centre, of a size (cm) on each axis. */
	FTransform MarkerTransform(const FVector& Centre, const double SizeX, const double SizeY)
	{
		return FTransform(FQuat::Identity, Centre + FVector(0.0, 0.0, GroundOffset), FVector(SizeX / ShapeSize, SizeY / ShapeSize, 1.0));
	}

	void AddCustomData(TArray<float>& CustomData, const FLinearColor& Color, const double StrokeShare)
	{
		CustomData.Append({ Color.R, Color.G, Color.B, static_cast<float>(FMath::Clamp(StrokeShare, 0.0, 1.0)) });
	}

	/** Puts the instances of a marker component, rebuilding them only when their number changes. */
	void SetInstances(UInstancedStaticMeshComponent& Component, const TArray<FTransform>& Transforms, const TArray<float>& CustomData)
	{
		if (Component.GetInstanceCount() != Transforms.Num())
		{
			Component.ClearInstances();
			if (!Transforms.IsEmpty())
			{
				Component.AddInstances(Transforms, /*bShouldReturnIndices*/ false, /*bWorldSpace*/ true);
			}
		}
		else if (!Transforms.IsEmpty())
		{
			Component.BatchUpdateInstancesTransforms(0, Transforms, /*bWorldSpace*/ true, /*bMarkRenderStateDirty*/ false, /*bTeleport*/ true);
		}
		for (int32 Index = 0; Index < Transforms.Num(); ++Index)
		{
			Component.SetCustomData(Index, TArrayView<const float>(CustomData.GetData() + Index * NumCustomData, NumCustomData),
				/*bMarkRenderStateDirty*/ Index == Transforms.Num() - 1);
		}
	}
}

AHOLSelectionMarkers::AHOLSelectionMarkers()
{
	using namespace UE::HeroesOfLiberty::SelectionMarkers;

	PrimaryActorTick.bCanEverTick = true;
	// After the controller has read the input and the units have been drawn this frame.
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	SetReplicates(false);

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	Rings = MakeMarkerComponent(*this, TEXT("Rings"));
	Frames = MakeMarkerComponent(*this, TEXT("Frames"));
	FlagPole = MakeFlagComponent(*this, TEXT("FlagPole"));
	FlagCloth = MakeFlagComponent(*this, TEXT("FlagCloth"));
}

void AHOLSelectionMarkers::Init(AHOLPlayerController* InController)
{
	using namespace UE::HeroesOfLiberty::SelectionMarkers;

	Controller = InController;
	if (InController)
	{
		OrderHandle = InController->OnOrderGiven.AddUObject(this, &AHOLSelectionMarkers::HandleOrderGiven);
	}

	UStaticMesh* Plane = LoadMesh(PlanePath);
	UStaticMesh* Cube = LoadMesh(CubePath);
	Rings->SetStaticMesh(Plane);
	Rings->SetMaterial(0, LoadMaterial(RingMaterialPath));
	Frames->SetStaticMesh(Plane);
	Frames->SetMaterial(0, LoadMaterial(FrameMaterialPath));

	UMaterialInterface* Flat = LoadMaterial(FlatMaterialPath);
	FlagPole->SetStaticMesh(Cube);
	FlagCloth->SetStaticMesh(Cube);
	if (Flat)
	{
		UMaterialInstanceDynamic* PoleMaterial = UMaterialInstanceDynamic::Create(Flat, this);
		PoleMaterial->SetVectorParameterValue(TEXT("Color"), PoleColor);
		FlagPole->SetMaterial(0, PoleMaterial);
		ClothMaterial = UMaterialInstanceDynamic::Create(Flat, this);
		FlagCloth->SetMaterial(0, ClothMaterial);
	}
	FlagPole->SetRelativeScale3D(FVector(PoleWidth, PoleWidth, PoleHeight) / ShapeSize);
	FlagCloth->SetRelativeScale3D(ClothSize / ShapeSize);
}

void AHOLSelectionMarkers::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AHOLPlayerController* PlayerController = Controller.Get())
	{
		PlayerController->OnOrderGiven.Remove(OrderHandle);
	}
	OrderHandle.Reset();
	Super::EndPlay(EndPlayReason);
}

void AHOLSelectionMarkers::HandleOrderGiven(const FVector& Point)
{
	PingPoint = Point;
	PingAge = 0.f;
}

void AHOLSelectionMarkers::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (PingAge < TNumericLimits<float>::Max())
	{
		PingAge += DeltaSeconds;
	}
	UpdateRings();
	UpdateFrame();
	UpdateFlag();
}

void AHOLSelectionMarkers::UpdateRings()
{
	using namespace UE::HeroesOfLiberty::SelectionMarkers;

	TArray<FTransform> Transforms;
	TArray<float> CustomData;
	const AHOLPlayerController* PlayerController = Controller.Get();
	const UWorld* World = GetWorld();
	if (PlayerController && World && !PlayerController->GetSelectedUnits().IsEmpty())
	{
		// Where the units are drawn this frame (D271): the rings follow them.
		TArray<FHOLUnitView> Views;
		HOLUnitView::Gather(*World, Views);
		const TArray<uint32>& Selected = PlayerController->GetSelectedUnits();
		for (const FHOLUnitView& View : Views)
		{
			if (Selected.Contains(View.UnitId))
			{
				const double Radius = View.Radius * RingRadiusFactor;
				Transforms.Add(MarkerTransform(View.Location, Radius * 2.0, Radius * 2.0));
				AddCustomData(CustomData, SelectionColor, RingStroke / Radius);
			}
		}
	}
	if (PingAge < PingSeconds)
	{
		const double Radius = FMath::Lerp(PingStartRadius, PingEndRadius, static_cast<double>(PingAge / PingSeconds));
		Transforms.Add(MarkerTransform(PingPoint, Radius * 2.0, Radius * 2.0));
		AddCustomData(CustomData, PingColor, PingStroke / Radius);
	}
	SetInstances(*Rings, Transforms, CustomData);
}

void AHOLSelectionMarkers::UpdateFrame()
{
	using namespace UE::HeroesOfLiberty::SelectionMarkers;

	TArray<FTransform> Transforms;
	TArray<float> CustomData;
	const AHOLPlayerController* PlayerController = Controller.Get();
	const AHOLBuilding* Building = PlayerController ? PlayerController->GetSelection() : nullptr;
	if (const FHOLBuildingTypeRow* Type = Building ? Building->GetType() : nullptr)
	{
		const FVector2D Size = Type->Footprint + FVector2D(FrameMargin * 2.0);
		Transforms.Add(MarkerTransform(Building->GetActorLocation(), Size.X, Size.Y));
		AddCustomData(CustomData, SelectionColor, FrameStroke * 2.0 / Size.GetMax());
	}
	SetInstances(*Frames, Transforms, CustomData);
}

void AHOLSelectionMarkers::UpdateFlag()
{
	using namespace UE::HeroesOfLiberty::SelectionMarkers;

	// The rally point reaches its owner only (D282): shown while the player's own building is selected.
	const AHOLPlayerController* PlayerController = Controller.Get();
	const AHOLBuilding* Building = PlayerController ? PlayerController->GetOwnSelection() : nullptr;
	const bool bShown = Building && Building->HasRallyPoint();
	if (FlagPole->bHiddenInGame == bShown)
	{
		FlagPole->SetHiddenInGame(!bShown);
		FlagCloth->SetHiddenInGame(!bShown);
	}
	if (!bShown)
	{
		return;
	}
	const FVector Foot = Building->GetRallyPoint();
	FlagPole->SetWorldLocation(Foot + FVector(0.0, 0.0, PoleHeight * 0.5));
	// The cloth hangs from the top of the pole, on its right as the camera sees it (the camera looks along +X, D254).
	FlagCloth->SetWorldLocation(Foot + FVector(0.0, ClothSize.Y * 0.5, PoleHeight - ClothSize.Z * 0.5));
	if (ClothMaterial)
	{
		ClothMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(HOLPlayerColors::GetColor(Building->GetColorIndex())));
	}
}

FString AHOLSelectionMarkers::Describe() const
{
	const int32 NumPings = PingAge < UE::HeroesOfLiberty::SelectionMarkers::PingSeconds ? 1 : 0;
	return FString::Printf(TEXT("rings %d, frame %d, flag %d"), Rings->GetInstanceCount() - NumPings, Frames->GetInstanceCount(),
		FlagPole->bHiddenInGame ? 0 : 1);
}
