// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
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

// J6a.3 checks: a foundation (D311, D312) blocks nothing, is selected like a building; finished, it is a building. J6a.4: a
// foundation is a ghost its owner alone sees (nothing elsewhere), a site shows its slab and the building rising from it.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingFoundationTest, "HeroesOfLiberty.Buildings.Foundation", UE::HeroesOfLiberty::Tests::Buildings::Flags)
bool FHOLBuildingFoundationTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Buildings;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	// Another player's foundation (no local owner here): nothing shown, nothing to click (J6a.4).
	AHOLBuilding* Other = World->SpawnActor<AHOLBuilding>();
	if (TestNotNull(TEXT("another player's building spawned"), Other))
	{
		Other->Setup(HOLBuildingTypeNames::House, 1, 3, EHOLConstructionState::Foundation);
		TArray<UStaticMeshComponent*> Pieces;
		Other->GetComponents(Pieces);
		TestFalse(TEXT("another player's foundation: not seen here"), Other->IsSeenHere());
		TestEqual(TEXT("another player's foundation: no piece"), Pieces.Num(), 0);
		FHitResult Hit;
		const FVector Above(100.0, 100.0, 5000.0);
		TestFalse(TEXT("another player's foundation: nothing to click"),
			World->LineTraceSingleByChannel(Hit, Above, Above - FVector(0.0, 0.0, 10000.0), HOLCollision::Selection));
		Other->Destroy();
	}

	// The player's own: its local controller owns it, as the server spawns it (J6a.3; local as the game mode makes it).
	APlayerController* Owner = World->SpawnActor<APlayerController>();
	if (Owner)
	{
		Owner->SetAsLocalPlayerController();
	}
	AHOLBuilding* Building = World->SpawnActor<AHOLBuilding>();
	const FHOLBuildingTypeRow* House = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::House);
	if (TestNotNull(TEXT("building spawned"), Building) && TestNotNull(TEXT("controller spawned"), Owner) && TestNotNull(TEXT("house in the table"), House))
	{
		Building->SetOwner(Owner);
		Building->Setup(HOLBuildingTypeNames::House, 0, 2, EHOLConstructionState::Foundation);
		TestTrue(TEXT("a foundation"), Building->IsFoundation() && !Building->IsFinished() && !Building->HasWorkStarted());
		TestTrue(TEXT("its owner's: seen here"), Building->IsSeenHere());

		// A ghost (J6a.4): the frame on the ground, the edges of its volume, an unseen box to click; no slab.
		TArray<UStaticMeshComponent*> Pieces;
		Building->GetComponents(Pieces);
		FBox Box(ForceInit);
		int32 Unseen = 0;
		for (const UStaticMeshComponent* Piece : Pieces)
		{
			Box += Piece->Bounds.GetBox();
			Unseen += Piece->IsVisible() ? 0 : 1;
		}
		TestEqual(TEXT("a ghost: 4 frame pieces, 8 edges, an unseen box"), Pieces.Num(), 13);
		TestEqual(TEXT("one piece unseen (the box a click hits)"), Unseen, 1);
		TestTrue(TEXT("over the footprint (6 x 6 m)"), FVector2D(Box.GetSize()).Equals(House->Footprint, 1.0));
		TestTrue(TEXT("as high as the building"), FMath::IsNearlyEqual(Box.Max.Z, static_cast<double>(House->Height), 1.0));
		TestEqual(TEXT("no piece at the owner's plain colour (lightened)"), CountPiecesOfColor(*Building, 2), 0);

		// Blocks nothing (D312), selected like a building (D262).
		TArray<UBoxComponent*> Boxes;
		Building->GetComponents(Boxes);
		TestTrue(TEXT("its forbidden area does not count for the navmesh"), Boxes.Num() == 1 && !Boxes[0]->CanEverAffectNavigation());
		FHitResult Hit;
		const FVector Above(100.0, 100.0, 5000.0);
		World->LineTraceSingleByChannel(Hit, Above, Above - FVector(0.0, 0.0, 10000.0), HOLCollision::Selection);
		TestTrue(TEXT("the selection ray hits the ghost"), Hit.GetActor() == Building);

		// Its first work (D312): a site, the foundation's slab, border and stakes, the building over it barely risen; its
		// forbidden area counted.
		Building->SetConstructionState(EHOLConstructionState::UnderConstruction);
		Building->GetComponents(Pieces);
		Box = FBox(ForceInit);
		for (const UStaticMeshComponent* Piece : Pieces)
		{
			Box += Piece->Bounds.GetBox();
		}
		TestEqual(TEXT("a site: the foundation (9) and the building (4)"), Pieces.Num(), 13);
		TestEqual(TEXT("its border and the roof at the owner's colour"), CountPiecesOfColor(*Building, 2), 5);
		TestTrue(TEXT("barely risen"), Box.Max.Z <= AHOLBuilding::FoundationStakeHeight + 1.0);
		TestTrue(TEXT("a site: its forbidden area counts"), Boxes.Num() == 1 && Boxes[0]->CanEverAffectNavigation());
		// Half its work: half its height.
		Building->SetConstructionWork(Building->GetTotalWork() / 2);
		Box = FBox(ForceInit);
		for (const UStaticMeshComponent* Piece : Pieces)
		{
			Box += Piece->Bounds.GetBox();
		}
		TestTrue(FString::Printf(TEXT("half its work: half its height (%.0f cm)"), Box.Max.Z), FMath::IsNearlyEqual(Box.Max.Z, House->Height * 0.5, 2.0));

		// Finished: the building's shape, whole, its forbidden area counted.
		Building->SetConstructionState(EHOLConstructionState::Finished);
		Building->GetComponents(Pieces);
		Box = FBox(ForceInit);
		for (const UStaticMeshComponent* Piece : Pieces)
		{
			Box += Piece->Bounds.GetBox();
		}
		TestEqual(TEXT("finished: a block, its top, a tower and its roof"), Pieces.Num(), 4);
		TestTrue(TEXT("finished: whole"), FMath::IsNearlyEqual(Box.Max.Z, static_cast<double>(House->Height), 1.0));
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
