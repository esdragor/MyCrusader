// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceField.h"
#include "Resources/HOLSourceTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// The provisional looks of the resources chosen by the user in the scenario resources_compare (2026-11-11, PlaceholdersInProgress.md):
// tree = style C (poplar: thin trunk, tall light-green egg crown from 1.1 m to the top), berry bush / stone vein / gold vein = style A
// (green dome with seven red berries; three cubes, one of the rock's colour and two of the ore's). Effect checked on what the game
// draws: AHOLSourceField fed with the rows of the real source type table, the instanced pieces read back (mesh, world box, colour).
namespace UE::HeroesOfLiberty::Tests::SourceLook
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	struct FDrawnPiece
	{
		const UStaticMesh* Mesh = nullptr;
		FBox Box = FBox(ForceInit);
		FLinearColor Color = FLinearColor::Black;
	};

	/** The pieces drawn within 1000 cm of At (one source per spot, the spots 5000 cm apart). */
	TArray<FDrawnPiece> PiecesNear(const AActor& Field, const FVector& At)
	{
		TArray<FDrawnPiece> Pieces;
		TInlineComponentArray<UInstancedStaticMeshComponent*> Components(&Field);
		for (const UInstancedStaticMeshComponent* Component : Components)
		{
			if (!Component->GetStaticMesh())
			{
				continue;
			}
			for (int32 Instance = 0; Instance < Component->GetInstanceCount(); ++Instance)
			{
				FTransform Transform;
				Component->GetInstanceTransform(Instance, Transform, /*bWorldSpace*/ true);
				if (FVector::Dist2D(Transform.GetLocation(), At) > 1000.0)
				{
					continue;
				}
				FDrawnPiece Piece;
				Piece.Mesh = Component->GetStaticMesh();
				Piece.Box = Component->GetStaticMesh()->GetBoundingBox().TransformBy(Transform);
				if (const UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(Component->GetMaterial(0)))
				{
					Material->GetVectorParameterValue(FMaterialParameterInfo(FName(TEXT("Color"))), Piece.Color);
				}
				Pieces.Add(Piece);
			}
		}
		return Pieces;
	}

	bool IsRed(const FLinearColor& Color) { return Color.R > 0.5f && Color.G < 0.1f && Color.B < 0.1f; }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSourceLookTest, "HeroesOfLiberty.Resources.SourceLook", UE::HeroesOfLiberty::Tests::SourceLook::Flags)
bool FHOLSourceLookTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::SourceLook;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	const UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	const UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	AHOLSourceField* Field = World->SpawnActor<AHOLSourceField>();
	const TCHAR* TypeNames[] = { TEXT("Tree"), TEXT("BerryBush"), TEXT("StoneVein"), TEXT("GoldVein") };
	const FHOLSourceTypeRow* Rows[4] = {};
	bool bReady = TestNotNull(TEXT("the source field"), Field) && TestNotNull(TEXT("the sphere"), Sphere) && TestNotNull(TEXT("the cube"), Cube);
	for (int32 Index = 0; Index < 4; ++Index)
	{
		Rows[Index] = UHOLSourceSettings::FindType(FName(TypeNames[Index]));
		bReady &= TestNotNull(FString::Printf(TEXT("the row %s of the source type table"), TypeNames[Index]), Rows[Index]);
	}
	if (bReady)
	{
		Field->Init(/*bDisplay*/ true, /*bNavigation*/ false);
		for (int32 Index = 0; Index < 4; ++Index)
		{
			Field->ShowSource(Index, static_cast<uint8>(Index), *Rows[Index], FVector(5000.0 * Index, 0.0, 0.0));
		}

		// Tree = C: a trunk and an egg-shaped light-green crown that starts low (1.1 m of 7.5 m) and reaches the source's height.
		const TArray<FDrawnPiece> Tree = PiecesNear(*Field, FVector(0.0, 0.0, 0.0));
		TestEqual(TEXT("tree C: two pieces (trunk, crown)"), Tree.Num(), 2);
		const FDrawnPiece* Crown = Tree.FindByPredicate([Sphere](const FDrawnPiece& Piece) { return Piece.Mesh == Sphere; });
		if (TestNotNull(TEXT("tree C: the crown is a sphere (an egg), not a cone"), Crown))
		{
			const double Top = Crown->Box.Max.Z;
			TestTrue(FString::Printf(TEXT("tree C: as tall as the source (%.0f cm, source %.0f, scale 0.9-1.1)"), Top, Rows[0]->Height),
				Top > 0.85 * Rows[0]->Height && Top < 1.15 * Rows[0]->Height);
			TestTrue(FString::Printf(TEXT("tree C: the crown starts low (%.0f cm of %.0f)"), Crown->Box.Min.Z, Top), Crown->Box.Min.Z < 0.25 * Top);
			TestTrue(FString::Printf(TEXT("tree C: the crown is light green (G %.2f)"), Crown->Color.G), Crown->Color.G > 0.35f && Crown->Color.R < Crown->Color.G);
		}

		// Berry bush = A: a green dome and seven red berries.
		const TArray<FDrawnPiece> Bush = PiecesNear(*Field, FVector(5000.0, 0.0, 0.0));
		int32 Berries = 0;
		for (const FDrawnPiece& Piece : Bush)
		{
			Berries += IsRed(Piece.Color) ? 1 : 0;
		}
		TestEqual(TEXT("bush A: eight pieces (dome and berries)"), Bush.Num(), 8);
		TestEqual(TEXT("bush A: seven red berries"), Berries, 7);

		// Veins = A: three cubes, the biggest of the rock's colour (dark), two of the ore's (grey-blue for stone, gold for gold).
		for (int32 Index = 2; Index < 4; ++Index)
		{
			const TArray<FDrawnPiece> Vein = PiecesNear(*Field, FVector(5000.0 * Index, 0.0, 0.0));
			TestEqual(FString::Printf(TEXT("%s A: three pieces"), TypeNames[Index]), Vein.Num(), 3);
			int32 Ore = 0;
			int32 Rock = 0;
			for (const FDrawnPiece& Piece : Vein)
			{
				TestTrue(FString::Printf(TEXT("%s A: cubes only"), TypeNames[Index]), Piece.Mesh == Cube);
				const bool bOre = Index == 2 ? Piece.Color.B > Piece.Color.R + 0.05f : Piece.Color.R > 0.9f && Piece.Color.B < 0.1f;
				const bool bRock = FMath::Max3(Piece.Color.R, Piece.Color.G, Piece.Color.B) < 0.25f;
				Ore += bOre ? 1 : 0;
				Rock += bRock ? 1 : 0;
			}
			TestEqual(FString::Printf(TEXT("%s A: two cubes of ore"), TypeNames[Index]), Ore, 2);
			TestEqual(FString::Printf(TEXT("%s A: one cube of rock"), TypeNames[Index]), Rock, 1);
		}
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
