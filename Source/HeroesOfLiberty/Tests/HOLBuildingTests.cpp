// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HOLCollision.h"
#include "Match/HOLPlayerColors.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// J2 checks (D264): the building type table (D260) and the provisional look of a building (D261).
namespace UE::HeroesOfLiberty::Tests::Buildings
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** Pieces of a building painted with the given player colour, at the given tone. */
	int32 CountPiecesOfColor(const AHOLBuilding& Building, const int32 ColorIndex, const float Tone = 1.f)
	{
		FLinearColor Expected = FLinearColor(HOLPlayerColors::GetLinearColor(ColorIndex)) * Tone;
		Expected.A = 1.f;
		TArray<UStaticMeshComponent*> Pieces;
		Building.GetComponents(Pieces);
		int32 Count = 0;
		for (const UStaticMeshComponent* Piece : Pieces)
		{
			FLinearColor Color;
			const UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(Piece->GetMaterial(0));
			if (Material && Material->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("Color")), Color)
				&& Color.Equals(Expected, 1.e-4f))
			{
				++Count;
			}
		}
		return Count;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingTypesTest, "HeroesOfLiberty.Buildings.Types", UE::HeroesOfLiberty::Tests::Buildings::Flags)
bool FHOLBuildingTypesTest::RunTest(const FString& Parameters)
{
	const FHOLBuildingTypeRow* MainCentre = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::MainCentre);
	if (TestNotNull(TEXT("main centre in the building type table"), MainCentre))
	{
		TestFalse(TEXT("named"), MainCentre->DisplayName.IsEmpty());
		TestEqual(TEXT("12 x 12 m: 4 x 4 tiles (D261, D311)"), MainCentre->Footprint, FVector2D(1200.0, 1200.0));
		TestEqual(TEXT("~10 m high (D261)"), MainCentre->Height, 1000.f);
		TestTrue(TEXT("health points"), MainCentre->MaxHealth > 0);
		TestTrue(TEXT("provisional shape: no final mesh yet"), MainCentre->Mesh.IsNull());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingAppearanceTest, "HeroesOfLiberty.Buildings.Appearance", UE::HeroesOfLiberty::Tests::Buildings::Flags)
bool FHOLBuildingAppearanceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Buildings;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	// As in a match: the actors spawned from now on are initialised (a building builds its look once initialised).
	World->InitializeActorsForPlay(FURL());

	AHOLBuilding* Building = World->SpawnActor<AHOLBuilding>();
	if (TestNotNull(TEXT("building spawned"), Building))
	{
		TestTrue(TEXT("replicated"), Building->GetIsReplicated());
		Building->Setup(HOLBuildingTypeNames::MainCentre, 2, 3);
		TestEqual(TEXT("owner"), Building->GetOwnerPlayer(), static_cast<uint8>(2));

		TArray<UStaticMeshComponent*> Pieces;
		Building->GetComponents(Pieces);
		TestEqual(TEXT("a block, its top, a tower and its roof"), Pieces.Num(), 4);

		// The shape fills the type's footprint and height, standing on the ground, centred on the actor.
		FBox Box(ForceInit);
		for (const UStaticMeshComponent* Piece : Pieces)
		{
			Box += Piece->Bounds.GetBox();
		}
		TestTrue(TEXT("footprint 12 x 12 m"), Box.GetSize().Equals(FVector(1200.0, 1200.0, 1000.0), 1.0));
		TestTrue(TEXT("on the ground, centred"), Box.Min.Equals(FVector(-600.0, -600.0, 0.0), 1.0));

		// The navmesh's forbidden area: the footprint widened by the bodies' clearance (D310), as tall as the shape.
		TArray<UBoxComponent*> Boxes;
		Building->GetComponents(Boxes);
		const double Clearance = UHOLBuildingSettings::GetBodyClearance();
		TestTrue(TEXT("forbidden area: the footprint and the clearance"), Boxes.Num() == 1 && Boxes[0]->CanEverAffectNavigation()
			&& Boxes[0]->GetScaledBoxExtent().Equals(FVector(600.0 + Clearance, 600.0 + Clearance, 500.0), 1.0));

		TestEqual(TEXT("the roof at the owner's colour"), CountPiecesOfColor(*Building, 3), 1);
		TestEqual(TEXT("the top of the block at a darker tone of it"), CountPiecesOfColor(*Building, 3, AHOLBuilding::BaseTopTone), 1);
		Building->Setup(HOLBuildingTypeNames::MainCentre, 2, 5);
		TestEqual(TEXT("a new colour repaints them"),
			CountPiecesOfColor(*Building, 5) + CountPiecesOfColor(*Building, 5, AHOLBuilding::BaseTopTone), 2);
		Building->GetComponents(Pieces);
		TestEqual(TEXT("without rebuilding the pieces"), Pieces.Num(), 4);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// J6a.3 checks: a foundation (D311, D312) looks flat, blocks nothing, is selected like a building; finished, it is a building.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingFoundationTest, "HeroesOfLiberty.Buildings.Foundation", UE::HeroesOfLiberty::Tests::Buildings::Flags)
bool FHOLBuildingFoundationTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Buildings;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLBuilding* Building = World->SpawnActor<AHOLBuilding>();
	const FHOLBuildingTypeRow* House = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::House);
	if (TestNotNull(TEXT("building spawned"), Building) && TestNotNull(TEXT("house in the table"), House))
	{
		Building->Setup(HOLBuildingTypeNames::House, 0, 2, EHOLConstructionState::Foundation);
		TestTrue(TEXT("a foundation"), Building->IsFoundation() && !Building->IsFinished() && !Building->HasWorkStarted());

		// Flat: the slab, its border, its stakes, no higher than a stake, over the whole footprint.
		TArray<UStaticMeshComponent*> Pieces;
		Building->GetComponents(Pieces);
		FBox Box(ForceInit);
		for (const UStaticMeshComponent* Piece : Pieces)
		{
			Box += Piece->Bounds.GetBox();
		}
		TestEqual(TEXT("a slab, four borders, four stakes"), Pieces.Num(), 9);
		TestTrue(TEXT("over the footprint (6 x 6 m)"), FVector2D(Box.GetSize()).Equals(House->Footprint, 1.0));
		TestTrue(TEXT("flat: no higher than its stakes"), Box.Max.Z <= AHOLBuilding::FoundationStakeHeight + 1.0);
		TestEqual(TEXT("its border at the owner's colour"), CountPiecesOfColor(*Building, 2), 4);

		// Blocks nothing (D312), selected like a building (D262).
		TArray<UBoxComponent*> Boxes;
		Building->GetComponents(Boxes);
		TestTrue(TEXT("its forbidden area does not count for the navmesh"), Boxes.Num() == 1 && !Boxes[0]->CanEverAffectNavigation());
		FHitResult Hit;
		const FVector Above(100.0, 100.0, 5000.0);
		World->LineTraceSingleByChannel(Hit, Above, Above - FVector(0.0, 0.0, 10000.0), HOLCollision::Selection);
		TestTrue(TEXT("the selection ray hits the foundation"), Hit.GetActor() == Building);

		// Finished: the building's shape, its forbidden area counted.
		Building->SetConstructionState(EHOLConstructionState::Finished);
		Building->GetComponents(Pieces);
		TestEqual(TEXT("finished: a block, its top, a tower and its roof"), Pieces.Num(), 4);
		TestTrue(TEXT("finished: its forbidden area counts"), Boxes.Num() == 1 && Boxes[0]->CanEverAffectNavigation());
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingSelectionTest, "HeroesOfLiberty.Buildings.Selection", UE::HeroesOfLiberty::Tests::Buildings::Flags)
bool FHOLBuildingSelectionTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLBuilding* Building = World->SpawnActor<AHOLBuilding>();
	if (TestNotNull(TEXT("building spawned"), Building))
	{
		Building->Setup(HOLBuildingTypeNames::MainCentre, 1, 4);

		// D262: the selection ray, from the height of the camera, hits the building and nothing beside it.
		FHitResult Hit;
		const FVector Above(0.0, 0.0, 5000.0);
		World->LineTraceSingleByChannel(Hit, Above, Above - FVector(0.0, 0.0, 10000.0), HOLCollision::Selection);
		TestTrue(TEXT("the selection ray hits the building"), Hit.GetActor() == Building);
		const FVector Beside(2000.0, 0.0, 5000.0);
		TestFalse(TEXT("nothing beside it"),
			World->LineTraceSingleByChannel(Hit, Beside, Beside - FVector(0.0, 0.0, 10000.0), HOLCollision::Selection));
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

#endif
