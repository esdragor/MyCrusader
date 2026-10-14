// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"

#include "Buildings/HOLBuildingTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "HOLCollision.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLPlayerColors.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

namespace UE::HeroesOfLiberty::Buildings
{
	// The engine's basic cube (1 m, centred), painted by the matte material's "Color" parameter (D265: no
	// specular highlight; Tools/Editor/make_matte_materials.py).
	const TCHAR* CubePath = TEXT("/Engine/BasicShapes/Cube.Cube");
	const TCHAR* ShapeMaterialPath = TEXT("/Game/Rendering/M_MatteColor.M_MatteColor");
	// The owner's colour is unlit: the same colour on every face, with no darker side (D261).
	const TCHAR* OwnerMaterialPath = TEXT("/Game/Rendering/M_FlatColor.M_FlatColor");
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
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(UE::HeroesOfLiberty::Buildings::ShapeMaterialPath);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> OwnerMaterial(UE::HeroesOfLiberty::Buildings::OwnerMaterialPath);
	CubeMesh = Cube.Object;
	PieceMaterial = ShapeMaterial.Object;
	OwnerPieceMaterial = OwnerMaterial.Object;
}

void AHOLBuilding::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHOLBuilding, TypeName);
	DOREPLIFETIME(AHOLBuilding, OwnerPlayer);
	DOREPLIFETIME(AHOLBuilding, ColorIndex);
	DOREPLIFETIME_CONDITION(AHOLBuilding, ProductionQueue, COND_OwnerOnly);
}

const FHOLBuildingTypeRow* AHOLBuilding::GetType() const
{
	return TypeName.IsNone() ? nullptr : UHOLBuildingSettings::FindType(TypeName);
}

void AHOLBuilding::NotifyProductionChanged()
{
	// The queue changes a few times a minute: sent at the next update rather than within the second of NetUpdateFrequency.
	ForceNetUpdate();
	OnProductionChanged.Broadcast();
}

void AHOLBuilding::OnRep_ProductionQueue()
{
	OnProductionChanged.Broadcast();
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
		OwnerTones.Reset();
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

	ApplyColors();
}

void AHOLBuilding::ApplyColors()
{
	const FLinearColor OwnerColor(HOLPlayerColors::GetLinearColor(ColorIndex));
	for (int32 Index = 0; Index < OwnerMaterials.Num(); ++Index)
	{
		FLinearColor Shown = OwnerColor * OwnerTones[Index];
		Shown.A = 1.f;
		OwnerMaterials[Index]->SetVectorParameterValue(UE::HeroesOfLiberty::Buildings::ColorParameter, Shown);
	}
}

void AHOLBuilding::BuildProvisionalShape(const FHOLBuildingTypeRow& Type)
{
	using namespace UE::HeroesOfLiberty::Buildings;

	// D261: a block and a central tower; the top of the block (a darker tone) and the tower's flat roof, a little wider
	// than the tower, wear the owner's colour, as they are what the RTS camera sees from above; unlit, so that the colour
	// is the same everywhere (a lit cone showed a dark side, an unlit one read as a flat disc).
	const FVector2D Footprint = Type.Footprint;
	const float Height = Type.Height;
	const float BaseHeight = Height * BaseHeightRatio;
	const float CapThickness = Height * CapThicknessRatio;
	const float TowerHeight = Height - CapThickness;

	AddPiece(CubeMesh, FVector(Footprint, BaseHeight), 0.f, StoneColor, false);
	AddPiece(CubeMesh, FVector(Footprint, CapThickness), BaseHeight, FLinearColor::White, true, BaseTopTone);
	AddPiece(CubeMesh, FVector(Footprint * TowerSizeRatio, TowerHeight), 0.f, DarkStoneColor, false);
	AddPiece(CubeMesh, FVector(Footprint * RoofSizeRatio, CapThickness), TowerHeight, FLinearColor::White, true);
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
	Piece->SetCollisionResponseToChannel(HOLCollision::Selection, ECR_Block);	// D262
	Piece->SetupAttachment(RootComponent);
	Piece->RegisterComponent();
	Pieces.Add(Piece);
}

UStaticMeshComponent* AHOLBuilding::AddPiece(UStaticMesh* Mesh, const FVector& Size, const float BottomZ, const FLinearColor& Color,
	const bool bOwnerColor, const float OwnerTone)
{
	UMaterialInterface* BaseMaterial = bOwnerColor ? OwnerPieceMaterial : PieceMaterial;
	if (!Mesh || !BaseMaterial)
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
	Piece->SetCollisionResponseToChannel(HOLCollision::Selection, ECR_Block);	// D262
	UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Piece);
	Material->SetVectorParameterValue(UE::HeroesOfLiberty::Buildings::ColorParameter, Color);
	Piece->SetMaterial(0, Material);
	Piece->SetupAttachment(RootComponent);
	Piece->RegisterComponent();

	Pieces.Add(Piece);
	if (bOwnerColor)
	{
		OwnerMaterials.Add(Material);
		OwnerTones.Add(OwnerTone);
	}
	return Piece;
}
