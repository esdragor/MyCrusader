// Copyright Epic Games, Inc. All Rights Reserved.

#include "Resources/HOLSourceField.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "HeroesOfLibertyLog.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Resources/HOLSourceNavComponent.h"
#include "Resources/HOLSourceTypes.h"
#include "UObject/ConstructorHelpers.h"

namespace UE::HeroesOfLiberty::Resources
{
	// The engine's basic shapes (1 m, centred) and the matte material of the buildings (D265).
	const TCHAR* CylinderPath = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
	const TCHAR* ConePath = TEXT("/Engine/BasicShapes/Cone.Cone");
	const TCHAR* SpherePath = TEXT("/Engine/BasicShapes/Sphere.Sphere");
	const TCHAR* CubePath = TEXT("/Engine/BasicShapes/Cube.Cube");
	const TCHAR* MattePath = TEXT("/Game/Rendering/M_MatteColor.M_MatteColor");
	const FName ColorParameter(TEXT("Color"));

	// The styles chosen by the user in the scenario resources_compare (2026-11-11): tree C, berry bush A, stone and gold veins A.
	// Colours and sizes are those of Script/Visual/HOLResourceReview.as (the review tool, kept for memory); Seen = the radius and
	// height (cm) of the source the style was shown at there.
	const FLinearColor TreeBark(0.07f, 0.04f, 0.02f);
	const FLinearColor TreeCrown(0.16f, 0.42f, 0.07f);
	const FLinearColor BushLeaf(0.05f, 0.28f, 0.05f);
	const FLinearColor BerryRed(0.60f, 0.03f, 0.03f);
	const FLinearColor StoneRock(0.20f, 0.20f, 0.22f);
	const FLinearColor StoneOre(0.50f, 0.54f, 0.62f);
	const FLinearColor GoldRock(0.16f, 0.13f, 0.09f);
	const FLinearColor GoldOre(0.95f, 0.62f, 0.04f);
	// The farm field keeps the look the berry bush had before (green flat dome, red patch).
	const FLinearColor FieldGreen(0.12f, 0.24f, 0.07f);
	const FLinearColor FieldRed(0.55f, 0.04f, 0.05f);
	const FVector2D TreeSeen(60.f, 750.f);
	const FVector2D BushSeen(110.f, 160.f);
	const FVector2D VeinSeen(300.f, 300.f);

	/** A source's turn and size, drawn from its ground point: the same on every machine, and a forest is not a grid. */
	void GetVariation(const FVector& Location, float& OutYaw, float& OutScale)
	{
		const uint32 Hash = HashCombine(GetTypeHash(FMath::RoundToInt(Location.X)), GetTypeHash(FMath::RoundToInt(Location.Y)));
		OutYaw = static_cast<float>(Hash % 360u);
		OutScale = 0.9f + 0.2f * static_cast<float>((Hash >> 9) % 1000u) / 1000.f;
	}
}

AHOLSourceField::AHOLSourceField()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Hard references, so that the shapes are cooked with the game.
	using namespace UE::HeroesOfLiberty::Resources;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(CylinderPath);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(ConePath);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(SpherePath);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(CubePath);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Matte(MattePath);
	CylinderMesh = Cylinder.Object;
	ConeMesh = Cone.Object;
	SphereMesh = Sphere.Object;
	CubeMesh = Cube.Object;
	MatteMaterial = Matte.Object;
}

void AHOLSourceField::Init(const bool bInDisplay, const bool bNavigation)
{
	bDisplay = bInDisplay;
	if (bNavigation && !Navigation)
	{
		// Registered with the navigation once the first sources are in (RegisterNavigation).
		Navigation = NewObject<UHOLSourceNavComponent>(this, TEXT("SourceNavigation"));
	}
}

void AHOLSourceField::ShowSource(const int32 SourceIndex, const uint8 TypeIndex, const FHOLSourceTypeRow& Type, const FVector& Location)
{
	if (InstanceOfSource.Num() <= SourceIndex)
	{
		const int32 OldNum = InstanceOfSource.Num();
		InstanceOfSource.SetNum(SourceIndex + 1);
		for (int32 Index = OldNum; Index <= SourceIndex; ++Index)
		{
			InstanceOfSource[Index] = INDEX_NONE;
		}
		TypeOfSource.SetNumZeroed(SourceIndex + 1);
	}
	if (Navigation)
	{
		Navigation->AddSource(SourceIndex, Location, Type.Radius);
	}
	if (!bDisplay || InstanceOfSource[SourceIndex] != INDEX_NONE)
	{
		return;
	}

	float Yaw = 0.f;
	float Scale = 1.f;
	UE::HeroesOfLiberty::Resources::GetVariation(Location, Yaw, Scale);
	const FTransform SourceTransform(FRotator(0.f, Yaw, 0.f), Location, FVector(Scale));

	FTypeDisplay& Display = GetOrMakeTypeDisplay(TypeIndex, Type);
	int32 Instance = INDEX_NONE;
	for (int32 Piece = 0; Piece < Display.Components.Num(); ++Piece)
	{
		Instance = Display.Components[Piece]->AddInstance(Display.PieceTransforms[Piece] * SourceTransform, /*bWorldSpace*/ true);
	}
	TypeOfSource[SourceIndex] = TypeIndex;
	InstanceOfSource[SourceIndex] = Instance;
	++NumShown;
}

void AHOLSourceField::HideSource(const int32 SourceIndex, const bool bKeepDrawn)
{
	if (Navigation)
	{
		Navigation->RemoveSource(SourceIndex);
	}
	if (!bKeepDrawn)
	{
		EraseDrawing(SourceIndex);
	}
}

void AHOLSourceField::EraseDrawing(const int32 SourceIndex)
{
	if (!InstanceOfSource.IsValidIndex(SourceIndex) || InstanceOfSource[SourceIndex] == INDEX_NONE)
	{
		return;
	}
	// Shrunk to nothing rather than removed: the other sources keep their instance indices.
	if (const FTypeDisplay* Display = TypeDisplays.Find(TypeOfSource[SourceIndex]))
	{
		const FTransform Gone(FQuat::Identity, FVector::ZeroVector, FVector::ZeroVector);
		for (UInstancedStaticMeshComponent* Component : Display->Components)
		{
			Component->UpdateInstanceTransform(InstanceOfSource[SourceIndex], Gone, /*bWorldSpace*/ true, /*bMarkRenderStateDirty*/ true);
		}
	}
	InstanceOfSource[SourceIndex] = INDEX_NONE;
	--NumShown;
}

void AHOLSourceField::RegisterNavigation()
{
	if (Navigation && Navigation->GetNumCylinders() > 0)
	{
		Navigation->RegisterWithNavigation();
	}
}

int32 AHOLSourceField::GetNumNavCylinders() const
{
	return Navigation ? Navigation->GetNumCylinders() : 0;
}

float AHOLSourceField::GetDrawnRadius(const FHOLSourceTypeRow& Type)
{
	// The widest piece of the provisional shapes below (a final mesh is drawn about as wide as its body). Kept at 2 radii for the
	// poplar of tree style C too (its crown is 4 radii wide): the selection aims at the trunk's side, as before (2026-11-12).
	const float WidthInRadii = 2.f;
	return Type.Radius * WidthInRadii * 0.5f;
}

TArray<AHOLSourceField::FPiece> AHOLSourceField::MakeProvisionalPieces(const EHOLResource Resource, const FHOLSourceTypeRow& Type) const
{
	using namespace UE::HeroesOfLiberty::Resources;
	TArray<FPiece> Pieces;
	auto Add = [&Pieces](UStaticMesh* Mesh, const FVector2D& Offset, const FVector& Size, const float BottomZ, const float Yaw,
		const FLinearColor& Color)
	{
		Pieces.Add({ Mesh, Offset, Size, BottomZ, Yaw, Color });
	};
	// A piece in the centimetres of Script/Visual/HOLResourceReview.as, where the style was shown on a source of radius / height
	// Seen (X, Y): turned into the source's measures, so that the small, medium and big veins keep the shape.
	auto AddSeen = [&Add](UStaticMesh* Mesh, const FVector2D& OffsetCm, const FVector& SizeCm, const float BottomCm, const float Yaw,
		const FLinearColor& Color, const FVector2D& Seen)
	{
		Add(Mesh, OffsetCm / Seen.X, FVector(SizeCm.X / Seen.X, SizeCm.Y / Seen.X, SizeCm.Z / Seen.Y), static_cast<float>(BottomCm / Seen.Y), Yaw, Color);
	};
	switch (Resource)
	{
	case EHOLResource::Wood:
		// Style C of resources_compare (the user, 2026-11-11): a poplar, thin trunk and tall light-green egg crown. Replaces the trunk
		// and cone of D297 / M-Q10. Shown at 7.5 m, the crown from 1.1 m and 2.4 m wide (Q-resources-tree-size: size kept as seen).
		AddSeen(CylinderMesh, FVector2D::ZeroVector, FVector(50.f, 50.f, 200.f), 0.f, 0.f, TreeBark, TreeSeen);
		AddSeen(SphereMesh, FVector2D::ZeroVector, FVector(240.f, 240.f, 640.f), 110.f, 0.f, TreeCrown, TreeSeen);
		break;
	case EHOLResource::Food:
		if (Type.Endless)
		{
			// A farm field (endless): its flat parcel as before (resources_compare did not show it).
			Add(SphereMesh, FVector2D::ZeroVector, FVector(2.f, 2.f, 1.f), 0.f, 0.f, FieldGreen);
			Add(SphereMesh, FVector2D(0.1f, -0.1f), FVector(1.3f, 1.3f, 0.45f), 0.62f, 0.f, FieldRed);
			break;
		}
		// Style A of resources_compare: a green dome and seven red berries around it (the berries 1.7 times their first size).
		AddSeen(SphereMesh, FVector2D::ZeroVector, FVector(220.f, 220.f, 160.f), -10.f, 0.f, BushLeaf, BushSeen);
		for (int32 Berry = 0; Berry < 7; ++Berry)
		{
			const float Angle = FMath::DegreesToRadians(Berry * 360.f / 7.f);
			const float Around = 92.f * (1.f - 0.18f * (Berry % 3));
			const float Size = 30.f * 1.7f;
			AddSeen(SphereMesh, FVector2D(FMath::Cos(Angle) * Around, FMath::Sin(Angle) * Around), FVector(Size), 70.f + 14.f * (Berry % 3) - 0.5f * Size,
				0.f, BerryRed, BushSeen);
		}
		break;
	case EHOLResource::Stone:
	case EHOLResource::Gold:
	{
		// Style A of resources_compare: three cubes, the biggest of the rock's colour, two of the ore's (grey-blue or gold).
		const bool bGold = Resource == EHOLResource::Gold;
		const FLinearColor& Rock = bGold ? GoldRock : StoneRock;
		const FLinearColor& Ore = bGold ? GoldOre : StoneOre;
		AddSeen(CubeMesh, FVector2D::ZeroVector, FVector(300.f, 300.f, 260.f), 0.f, 20.f, Rock, VeinSeen);
		AddSeen(CubeMesh, FVector2D(150.f, -120.f), FVector(220.f, 220.f, 200.f), 0.f, 50.f, Ore, VeinSeen);
		AddSeen(CubeMesh, FVector2D(-140.f, 100.f), FVector(180.f, 180.f, 180.f), 0.f, -15.f, Ore, VeinSeen);
		break;
	}
	}
	return Pieces;
}

AHOLSourceField::FTypeDisplay& AHOLSourceField::GetOrMakeTypeDisplay(const uint8 TypeIndex, const FHOLSourceTypeRow& Type)
{
	if (FTypeDisplay* Existing = TypeDisplays.Find(TypeIndex))
	{
		return *Existing;
	}
	FTypeDisplay& Display = TypeDisplays.Add(TypeIndex);

	UStaticMesh* FinalMesh = Type.Mesh.IsNull() ? nullptr : Type.Mesh.LoadSynchronous();
	if (!Type.Mesh.IsNull() && !FinalMesh)
	{
		UE_LOG(LogHOLMatch, Error, TEXT("Source type %s: mesh %s not found, provisional shape drawn"), *Type.DisplayName.ToString(),
			*Type.Mesh.ToString());
	}
	if (FinalMesh)
	{
		// The final model is drawn as it is made, its pivot on the ground.
		Display.Components.Add(MakeComponent(FinalMesh, nullptr));
		Display.PieceTransforms.Add(FTransform::Identity);
		return Display;
	}

	// A provisional shape (D297): each piece is the engine's 1 m shape, scaled to its size and laid on its bottom.
	for (const FPiece& Piece : MakeProvisionalPieces(Type.Resource, Type))
	{
		if (!Piece.Mesh)
		{
			continue;
		}
		const FVector Size(Piece.Size.X * Type.Radius, Piece.Size.Y * Type.Radius, Piece.Size.Z * Type.Height);
		const FBoxSphereBounds Bounds = Piece.Mesh->GetBounds();
		const FVector Scale = Size / (2.0 * Bounds.BoxExtent);
		const double MeshBottom = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * Scale.Z;
		const FRotator Rotation(0.f, Piece.Yaw, 0.f);
		const FVector Centre = Rotation.RotateVector(FVector(Bounds.Origin.X * Scale.X, Bounds.Origin.Y * Scale.Y, 0.));
		const FVector Location(Piece.Offset.X * Type.Radius - Centre.X, Piece.Offset.Y * Type.Radius - Centre.Y,
			Piece.BottomZ * Type.Height - MeshBottom);
		Display.Components.Add(MakeComponent(Piece.Mesh, &Piece.Color));
		Display.PieceTransforms.Add(FTransform(Rotation, Location, Scale));
	}
	return Display;
}

UInstancedStaticMeshComponent* AHOLSourceField::MakeComponent(UStaticMesh* Mesh, const FLinearColor* Color)
{
	UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(this);
	Component->SetStaticMesh(Mesh);
	Component->SetCastShadow(false);	// D258
	Component->SetCanEverAffectNavigation(false);	// the forbidden areas are UHOLSourceNavComponent's (D297)
	Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Color && MatteMaterial)
	{
		UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(MatteMaterial, Component);
		Material->SetVectorParameterValue(UE::HeroesOfLiberty::Resources::ColorParameter, *Color);
		Component->SetMaterial(0, Material);
	}
	Component->SetupAttachment(RootComponent);
	Component->RegisterComponent();
	AllComponents.Add(Component);
	return Component;
}
