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
#include "Resources/HOLSourceSubsystem.h"
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
	/** A resource source's ring (J5b): its margin around the source's body and its stroke (cm). */
	constexpr double SourceRingMargin = 60.0;
	constexpr double SourceRingStroke = 16.0;
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
	/** The placement ghost (J6a.3): green where it may go, red where not; its outline's and its tiles' strokes (cm). */
	const FLinearColor GhostValidColor(0.15f, 0.85f, 0.2f);
	const FLinearColor GhostRefusedColor(0.95f, 0.12f, 0.08f);
	constexpr double GhostOutlineStroke = 40.0;
	constexpr double GhostTileStroke = 8.0;
	/** Width (cm) of the posts and beams outlining its volume. */
	constexpr double GhostEdgeWidth = 12.0;
	constexpr int32 NumGhostEdges = 8;

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
	GhostFrames = MakeMarkerComponent(*this, TEXT("GhostFrames"));
	for (int32 Index = 0; Index < NumGhostEdges; ++Index)
	{
		GhostEdges.Add(MakeFlagComponent(*this, *FString::Printf(TEXT("GhostEdge%d"), Index)));
	}
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
	GhostFrames->SetStaticMesh(Plane);
	GhostFrames->SetMaterial(0, Frames->GetMaterial(0));

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
		GhostMaterial = UMaterialInstanceDynamic::Create(Flat, this);
	}
	for (UStaticMeshComponent* Edge : GhostEdges)
	{
		Edge->SetStaticMesh(Cube);
		Edge->SetMaterial(0, GhostMaterial);
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
	UpdateGhost();
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
	// The selected resource source (J5b, D298): a ring around its body.
	const UHOLSourceSubsystem* Sources = World ? World->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
	const int32 SourceIndex = PlayerController ? PlayerController->GetSelectedSource() : INDEX_NONE;
	if (Sources && Sources->GetSources().IsValidIndex(SourceIndex) && Sources->GetSources()[SourceIndex].IsStanding())
	{
		const FHOLSource& Source = Sources->GetSources()[SourceIndex];
		const double Radius = Source.Radius + SourceRingMargin;
		Transforms.Add(MarkerTransform(Source.Location, Radius * 2.0, Radius * 2.0));
		AddCustomData(CustomData, SelectionColor, SourceRingStroke / Radius);
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

void AHOLSelectionMarkers::UpdateGhost()
{
	using namespace UE::HeroesOfLiberty::SelectionMarkers;

	const AHOLPlayerController* PlayerController = Controller.Get();
	FName TypeName;
	FBox2D Footprint;
	double GroundZ = 0.0;
	bool bValid = false;
	const FHOLBuildingTypeRow* Type = PlayerController && PlayerController->GetPlacementGhost(TypeName, Footprint, GroundZ, bValid)
		? UHOLBuildingSettings::FindType(TypeName) : nullptr;
	const EGhost Ghost = !Type ? EGhost::None : bValid ? EGhost::Valid : EGhost::Refused;
	if (Ghost != ShownGhost)
	{
		for (UStaticMeshComponent* Edge : GhostEdges)
		{
			Edge->SetHiddenInGame(Ghost == EGhost::None);
		}
		if (GhostMaterial && Ghost != EGhost::None)
		{
			GhostMaterial->SetVectorParameterValue(TEXT("Color"), Ghost == EGhost::Valid ? GhostValidColor : GhostRefusedColor);
		}
		ShownGhost = Ghost;
	}
	TArray<FTransform> Transforms;
	TArray<float> CustomData;
	if (Type)
	{
		const FLinearColor Color = bValid ? GhostValidColor : GhostRefusedColor;
		const FVector2D Size = Footprint.GetSize();
		const FVector Centre(Footprint.GetCenter(), GroundZ);
		// The outline, then each tile (D311: placement tile by tile), a thin frame.
		Transforms.Add(MarkerTransform(Centre, Size.X, Size.Y));
		AddCustomData(CustomData, Color, GhostOutlineStroke * 2.0 / Size.GetMax());
		const double Tile = UHOLBuildingSettings::GetTileSize();
		const int32 TilesX = FMath::Max(1, FMath::RoundToInt(Size.X / Tile));
		const int32 TilesY = FMath::Max(1, FMath::RoundToInt(Size.Y / Tile));
		const FVector2D TileSize(Size.X / TilesX, Size.Y / TilesY);
		for (int32 X = 0; X < TilesX && TilesX * TilesY > 1; ++X)
		{
			for (int32 Y = 0; Y < TilesY; ++Y)
			{
				const FVector TileCentre(Footprint.Min + TileSize * FVector2D(X + 0.5, Y + 0.5), GroundZ);
				Transforms.Add(MarkerTransform(TileCentre, TileSize.X, TileSize.Y));
				AddCustomData(CustomData, Color, GhostTileStroke * 2.0 / TileSize.GetMax());
			}
		}
		// The edges of its volume: a post at each corner, a beam along each side at its height.
		const double Height = Type->Height;
		const FVector2D Half = Size * 0.5 - FVector2D(GhostEdgeWidth * 0.5);
		const FVector2D Corners[] = { FVector2D(Half.X, Half.Y), FVector2D(Half.X, -Half.Y), FVector2D(-Half.X, -Half.Y), FVector2D(-Half.X, Half.Y) };
		for (int32 Index = 0; Index < 4; ++Index)
		{
			GhostEdges[Index]->SetWorldLocationAndRotation(Centre + FVector(Corners[Index], Height * 0.5), FQuat::Identity);
			GhostEdges[Index]->SetWorldScale3D(FVector(GhostEdgeWidth, GhostEdgeWidth, Height) / ShapeSize);
			const FVector2D Next = Corners[(Index + 1) % 4];
			const FVector2D Middle = (Corners[Index] + Next) * 0.5;
			const FVector2D Span = (Next - Corners[Index]).GetAbs() + FVector2D(GhostEdgeWidth);
			GhostEdges[4 + Index]->SetWorldLocationAndRotation(Centre + FVector(Middle, Height - GhostEdgeWidth * 0.5), FQuat::Identity);
			GhostEdges[4 + Index]->SetWorldScale3D(FVector(Span, GhostEdgeWidth) / ShapeSize);
		}
	}
	SetInstances(*GhostFrames, Transforms, CustomData);
}

FString AHOLSelectionMarkers::Describe() const
{
	const int32 NumPings = PingAge < UE::HeroesOfLiberty::SelectionMarkers::PingSeconds ? 1 : 0;
	return FString::Printf(TEXT("rings %d, frame %d, flag %d, ghost %s"), Rings->GetInstanceCount() - NumPings, Frames->GetInstanceCount(),
		FlagPole->bHiddenInGame ? 0 : 1, ShownGhost == EGhost::None ? TEXT("none") : ShownGhost == EGhost::Valid ? TEXT("valid") : TEXT("refused"));
}
