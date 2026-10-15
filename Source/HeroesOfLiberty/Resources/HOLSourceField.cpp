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

	// The colours of the trees of Plain's grove (create_match_map.py), so that the two kinds of tree match.
	const FLinearColor TrunkColor(0.22f, 0.15f, 0.09f);
	const FLinearColor CrownColor(0.10f, 0.22f, 0.08f);
	const FLinearColor BushColor(0.12f, 0.24f, 0.07f);
	const FLinearColor BerryColor(0.55f, 0.04f, 0.05f);
	const FLinearColor StoneColor(0.46f, 0.45f, 0.44f);
	const FLinearColor GoldColor(0.83f, 0.62f, 0.10f);

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

void AHOLSourceField::HideSource(const int32 SourceIndex)
{
	if (Navigation)
	{
		Navigation->RemoveSource(SourceIndex);
	}
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
	// The widest piece of the provisional shapes below (a final mesh is drawn about as wide as its body).
	const float WidthInRadii = Type.Mesh.IsNull() && Type.Resource == EHOLResource::Wood ? 4.8f : 2.f;
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
	switch (Resource)
	{
	case EHOLResource::Wood:
		// D297: a trunk and a green cone; the cone is wider than the body, so that a forest reads as one canopy, and starts
		// above a Peasant's head (2.7 m), so that one gathering at its foot is seen under it.
		Add(CylinderMesh, FVector2D::ZeroVector, FVector(1.f, 1.f, 0.45f), 0.f, 0.f, TrunkColor);
		Add(ConeMesh, FVector2D::ZeroVector, FVector(4.8f, 4.8f, 0.6f), 0.4f, 0.f, CrownColor);
		break;
	case EHOLResource::Food:
		// A bush with red berries on top.
		Add(SphereMesh, FVector2D::ZeroVector, FVector(2.f, 2.f, 1.f), 0.f, 0.f, BushColor);
		Add(SphereMesh, FVector2D(0.1f, -0.1f), FVector(1.3f, 1.3f, 0.45f), 0.62f, 0.f, BerryColor);
		break;
	case EHOLResource::Stone:
	case EHOLResource::Gold:
	{
		// A heap of three blocks, grey or golden.
		const FLinearColor& Color = Resource == EHOLResource::Gold ? GoldColor : StoneColor;
		Add(CubeMesh, FVector2D(-0.3f, -0.2f), FVector(1.1f, 0.9f, 0.6f), 0.f, 15.f, Color);
		Add(CubeMesh, FVector2D(0.3f, 0.15f), FVector(0.9f, 0.8f, 1.f), 0.f, -20.f, Color);
		Add(CubeMesh, FVector2D(-0.05f, 0.5f), FVector(0.7f, 0.6f, 0.45f), 0.f, 40.f, Color);
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
