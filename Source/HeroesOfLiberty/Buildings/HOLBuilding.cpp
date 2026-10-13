// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"

#include "Buildings/HOLBuildingTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLPlayerColors.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

namespace UE::HeroesOfLiberty::Buildings
{
	// The engine's basic shapes (1 m, centred) and their material, whose "Color" parameter paints a piece.
	const TCHAR* CubePath = TEXT("/Engine/BasicShapes/Cube.Cube");
	const TCHAR* ConePath = TEXT("/Engine/BasicShapes/Cone.Cone");
	const TCHAR* ShapeMaterialPath = TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial");
	const FName ColorParameter(TEXT("Color"));

	const FLinearColor StoneColor(0.42f, 0.40f, 0.37f);
	const FLinearColor DarkStoneColor(0.30f, 0.29f, 0.27f);
}

AHOLBuilding::AHOLBuilding()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Made by the server, seen by everyone: no movement to send, and nothing changes after the setup (until M5).
	bReplicates = true;
	SetReplicatingMovement(false);
	bAlwaysRelevant = true;	// until the fog of war (M7)
	SetNetUpdateFrequency(1.f);

	// Hard references, so that the shapes are cooked with the game.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(UE::HeroesOfLiberty::Buildings::CubePath);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(UE::HeroesOfLiberty::Buildings::ConePath);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(UE::HeroesOfLiberty::Buildings::ShapeMaterialPath);
	CubeMesh = Cube.Object;
	ConeMesh = Cone.Object;
	PieceMaterial = ShapeMaterial.Object;
}

void AHOLBuilding::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHOLBuilding, TypeName);
	DOREPLIFETIME(AHOLBuilding, OwnerPlayer);
	DOREPLIFETIME(AHOLBuilding, ColorIndex);
}

void AHOLBuilding::Setup(const FName InTypeName, const uint8 InOwnerPlayer, const uint8 InColorIndex)
{
	TypeName = InTypeName;
	OwnerPlayer = InOwnerPlayer;
	ColorIndex = InColorIndex;
	// A deferred spawn builds its look in BeginPlay; an actor already spawned builds it now.
	if (IsActorInitialized())
	{
		RefreshAppearance();
	}
}

void AHOLBuilding::BeginPlay()
{
	Super::BeginPlay();
	RefreshAppearance();
}

void AHOLBuilding::OnRep_Setup()
{
	RefreshAppearance();
}

void AHOLBuilding::RefreshAppearance()
{
	if (TypeName.IsNone())
	{
		return;
	}
	if (BuiltTypeName != TypeName)
	{
		for (UStaticMeshComponent* Piece : Pieces)
		{
			Piece->DestroyComponent();
		}
		Pieces.Reset();
		OwnerMaterials.Reset();
		BuiltTypeName = TypeName;

		const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(TypeName);
		if (!Type)
		{
			return;
		}
		if (Type->Mesh.IsNull())
		{
			BuildProvisionalShape(*Type);
		}
		else
		{
			BuildFinalMesh(*Type);
		}
	}

	const FLinearColor OwnerColor(HOLPlayerColors::GetLinearColor(ColorIndex));
	for (UMaterialInstanceDynamic* Material : OwnerMaterials)
	{
		Material->SetVectorParameterValue(UE::HeroesOfLiberty::Buildings::ColorParameter, OwnerColor);
	}
}

void AHOLBuilding::BuildProvisionalShape(const FHOLBuildingTypeRow& Type)
{
	using namespace UE::HeroesOfLiberty::Buildings;

	// D261: a block and a central tower; the top of the block and the tower's roof wear the owner's colour, as they
	// are what the RTS camera sees from above.
	const FVector2D Footprint = Type.Footprint;
	const float Height = Type.Height;
	const float BaseHeight = Height * BaseHeightRatio;
	const float TowerHeight = Height * TowerHeightRatio;
	const float CapThickness = Height * CapThicknessRatio;

	AddPiece(CubeMesh, FVector(Footprint, BaseHeight), 0.f, StoneColor, false);
	AddPiece(CubeMesh, FVector(Footprint, CapThickness), BaseHeight, FLinearColor::White, true);
	AddPiece(CubeMesh, FVector(Footprint * TowerSizeRatio, TowerHeight), 0.f, DarkStoneColor, false);
	AddPiece(ConeMesh, FVector(Footprint * RoofSizeRatio, Height - TowerHeight), TowerHeight, FLinearColor::White, true);
}

void AHOLBuilding::BuildFinalMesh(const FHOLBuildingTypeRow& Type)
{
	UStaticMesh* Mesh = Type.Mesh.LoadSynchronous();
	if (!Mesh)
	{
		UE_LOG(LogHOLMatch, Error, TEXT("Building type %s: mesh %s not found"), *TypeName.ToString(), *Type.Mesh.ToString());
		return;
	}
	UStaticMeshComponent* Piece = NewObject<UStaticMeshComponent>(this);
	Piece->SetStaticMesh(Mesh);
	Piece->SetupAttachment(RootComponent);
	Piece->RegisterComponent();
	Pieces.Add(Piece);
}

UStaticMeshComponent* AHOLBuilding::AddPiece(UStaticMesh* Mesh, const FVector& Size, const float BottomZ, const FLinearColor& Color,
	const bool bOwnerColor)
{
	if (!Mesh || !PieceMaterial)
	{
		return nullptr;
	}
	// Scaled to Size and laid with its bounding box's bottom at BottomZ, centred on the actor.
	const FBoxSphereBounds Bounds = Mesh->GetBounds();
	const FVector Scale = Size / (2.0 * Bounds.BoxExtent);
	const FVector Origin = Bounds.Origin * Scale;
	const double MeshBottom = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * Scale.Z;

	UStaticMeshComponent* Piece = NewObject<UStaticMeshComponent>(this);
	Piece->SetStaticMesh(Mesh);
	Piece->SetRelativeScale3D(Scale);
	Piece->SetRelativeLocation(FVector(-Origin.X, -Origin.Y, BottomZ - MeshBottom));
	Piece->SetCastShadow(false);	// D258
	UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(PieceMaterial, Piece);
	Material->SetVectorParameterValue(UE::HeroesOfLiberty::Buildings::ColorParameter, Color);
	Piece->SetMaterial(0, Material);
	Piece->SetupAttachment(RootComponent);
	Piece->RegisterComponent();

	Pieces.Add(Piece);
	if (bOwnerColor)
	{
		OwnerMaterials.Add(Material);
	}
	return Piece;
}
