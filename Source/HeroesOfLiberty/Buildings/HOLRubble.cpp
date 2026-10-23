// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLRubble.h"

#include "Buildings/HOLBuildingTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Match/HOLPlayerColors.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

namespace UE::HeroesOfLiberty::Rubble
{
	const TCHAR* CubePath = TEXT("/Engine/BasicShapes/Cube.Cube");
	const TCHAR* MaterialPath = TEXT("/Game/Rendering/M_MatteColor.M_MatteColor");
	const FName ColorParameter(TEXT("Color"));

	const FLinearColor AshColor(0.09f, 0.085f, 0.08f);
	const FLinearColor DarkStoneColor(0.17f, 0.165f, 0.155f);
	const FLinearColor StoneColor(0.28f, 0.27f, 0.25f);
	/** The owner's colour on a few broken pieces, darkened (burnt). */
	constexpr float OwnerTone = 0.45f;

	/** A piece of the heap: its middle (share of the half footprint), size (share of the footprint, share of the height), turn. */
	struct FPiece
	{
		FVector2D Offset;
		FVector2D Size;
		float Height;
		float Yaw;
		int32 Tone;	// 0 dark stone, 1 stone, 2 owner
	};
	const FPiece Pieces[] = {
		{ FVector2D(-0.45, -0.35), FVector2D(0.30, 0.22), 0.9f, 12.f, 1 },
		{ FVector2D(0.40, -0.45), FVector2D(0.26, 0.30), 0.6f, -20.f, 0 },
		{ FVector2D(0.10, 0.05), FVector2D(0.34, 0.28), 1.0f, 35.f, 0 },
		{ FVector2D(-0.20, 0.50), FVector2D(0.22, 0.18), 0.7f, -8.f, 2 },
		{ FVector2D(0.55, 0.35), FVector2D(0.18, 0.24), 0.8f, 50.f, 1 },
		{ FVector2D(-0.60, 0.10), FVector2D(0.16, 0.20), 0.5f, 25.f, 0 },
		{ FVector2D(0.25, -0.10), FVector2D(0.14, 0.12), 0.6f, -40.f, 2 },
		{ FVector2D(-0.05, -0.60), FVector2D(0.20, 0.14), 0.4f, 5.f, 1 },
	};
}

AHOLRubble::AHOLRubble()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	// Made by the server, seen by everyone (as the buildings, until the fog of war): its place and type only, once.
	bReplicates = true;
	SetReplicatingMovement(false);
	bAlwaysRelevant = true;
	SetNetUpdateFrequency(1.f);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(UE::HeroesOfLiberty::Rubble::CubePath);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(UE::HeroesOfLiberty::Rubble::MaterialPath);
	CubeMesh = Cube.Object;
	PieceMaterial = Material.Object;
}

void AHOLRubble::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHOLRubble, TypeName);
	DOREPLIFETIME(AHOLRubble, ColorIndex);
}

void AHOLRubble::Setup(const FName InTypeName, const uint8 InColorIndex)
{
	TypeName = InTypeName;
	ColorIndex = InColorIndex;
	// The server alone ends it (its InitialLifeSpan, set again by BeginPlay): a client's copy leaves with the server's.
	SetLifeSpan(LifeSeconds);
}

void AHOLRubble::BeginPlay()
{
	Super::BeginPlay();
	BuildLook();
}

void AHOLRubble::OnRep_Setup()
{
	BuildLook();
}

void AHOLRubble::BuildLook()
{
	using namespace UE::HeroesOfLiberty::Rubble;

	if (bBuilt || TypeName.IsNone() || GetNetMode() == NM_DedicatedServer || !HasActorBegunPlay())
	{
		return;
	}
	const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(TypeName);
	if (!Type)
	{
		return;
	}
	bBuilt = true;
	TArray<FLookPiece> Heap;
	GetLookPieces(UHOLBuildingSettings::GetVisualSize(Type->Footprint), ColorIndex, Heap);
	for (const FLookPiece& Piece : Heap)
	{
		AddPiece(Piece.Size, Piece.Offset, Piece.Yaw, Piece.Color);
	}
}

void AHOLRubble::GetLookPieces(const FVector2D& VisualSize, const uint8 InColorIndex, TArray<FLookPiece>& OutPieces)
{
	using namespace UE::HeroesOfLiberty::Rubble;

	OutPieces.Reset();
	// A flat heap over the building's visible shape, then broken pieces lying on it, lower than a man.
	OutPieces.Add({ FVector(VisualSize * 0.92, HeapHeight), FVector2D::ZeroVector, 0.f, AshColor });
	const FLinearColor OwnerShade = FLinearColor(HOLPlayerColors::GetLinearColor(InColorIndex)) * OwnerTone;
	for (const FPiece& Piece : Pieces)
	{
		const FLinearColor Color = Piece.Tone == 2 ? FLinearColor(OwnerShade.R, OwnerShade.G, OwnerShade.B, 1.f) : Piece.Tone == 1 ? StoneColor : DarkStoneColor;
		OutPieces.Add({ FVector(VisualSize * Piece.Size, HeapHeight + PieceMaxHeight * Piece.Height), VisualSize * 0.5 * Piece.Offset, Piece.Yaw, Color });
	}
}

void AHOLRubble::AddPiece(const FVector& Size, const FVector2D& Offset, const float Yaw, const FLinearColor& Color)
{
	if (!CubeMesh || !PieceMaterial)
	{
		return;
	}
	// The engine's cube (1 m, centred) scaled to Size, laid on the ground.
	const FBoxSphereBounds Bounds = CubeMesh->GetBounds();
	const FVector Scale = Size / (2.0 * Bounds.BoxExtent);
	UStaticMeshComponent* Piece = NewObject<UStaticMeshComponent>(this);
	Piece->SetStaticMesh(CubeMesh);
	Piece->SetRelativeScale3D(Scale);
	Piece->SetRelativeLocation(FVector(Offset, Size.Z * 0.5 - Bounds.Origin.Z * Scale.Z));
	Piece->SetRelativeRotation(FRotator(0.f, Yaw, 0.f));
	Piece->SetCastShadow(false);
	// Blocks nothing, answers no click (D345.2a).
	Piece->SetCanEverAffectNavigation(false);
	Piece->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Piece->SetGenerateOverlapEvents(false);
	UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(PieceMaterial, Piece);
	Material->SetVectorParameterValue(UE::HeroesOfLiberty::Rubble::ColorParameter, Color);
	Piece->SetMaterial(0, Material);
	Piece->SetupAttachment(RootComponent);
	Piece->RegisterComponent();
}
