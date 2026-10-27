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
#include "Buildings/HOLConstructionSubsystem.h"
#include "Buildings/HOLRubble.h"
#include "EngineUtils.h"
#include "Simulation/HOLUnitTypes.h"

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
		TestEqual(TEXT("16 x 16 m: 4 x 4 tiles of 4 m (D261, D311, D344)"), MainCentre->Footprint, FVector2D(1600.0, 1600.0));
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
		// The look is applied at the frame (D325): at once here.
		Building->ApplyPendingAppearance();
		TestEqual(TEXT("owner"), Building->GetOwnerPlayer(), static_cast<uint8>(2));

		TArray<UStaticMeshComponent*> Pieces;
		Building->GetComponents(Pieces);
		TestEqual(TEXT("a block, its top, a tower and its roof"), Pieces.Num(), 4);

		// The shape fills the type's visible size (D324 refined: its 16 x 16 m of tiles inset by VisualInset on every side,
		// at least 90 % of them) and height, standing on the ground, centred on the actor; its body, narrower, blocks.
		FBox Box(ForceInit);
		for (const UStaticMeshComponent* Piece : Pieces)
		{
			Box += Piece->Bounds.GetBox();
		}
		const double Seen = 1600.0 - 2.0 * UHOLBuildingSettings::GetVisualInset();
		const double Body = 1600.0 - 2.0 * UHOLBuildingSettings::GetBodyInset();
		TestTrue(FString::Printf(TEXT("the visible size of its 16 x 16 m (%.0f cm)"), Seen), Box.GetSize().Equals(FVector(Seen, Seen, 1000.0), 1.0));
		TestTrue(TEXT("at least 90 % of its tiles"), Seen >= 0.9 * 1600.0 - 0.01);
		TestTrue(TEXT("on the ground, centred"), Box.Min.Equals(FVector(-Seen * 0.5, -Seen * 0.5, 0.0), 1.0));
		TestTrue(TEXT("its body box"), Building->GetBodyBounds().GetSize().Equals(FVector2D(Body), 0.01));
		TestTrue(TEXT("its tiles box"), Building->GetFootprintBounds().GetSize().Equals(FVector2D(1600.0), 0.01));

		// The navmesh's forbidden area: the body widened by the bodies' clearance (D310), as tall as the shape.
		TArray<UBoxComponent*> Boxes;
		Building->GetComponents(Boxes);
		const double Clearance = UHOLBuildingSettings::GetBodyClearance();
		TestTrue(TEXT("forbidden area: the body and the clearance"), Boxes.Num() == 1 && Boxes[0]->CanEverAffectNavigation()
			&& Boxes[0]->GetScaledBoxExtent().Equals(FVector(Body * 0.5 + Clearance, Body * 0.5 + Clearance, 500.0), 1.0));

		TestEqual(TEXT("the roof at the owner's colour"), CountPiecesOfColor(*Building, 3), 1);
		TestEqual(TEXT("the top of the block at a darker tone of it"), CountPiecesOfColor(*Building, 3, AHOLBuilding::BaseTopTone), 1);
		Building->Setup(HOLBuildingTypeNames::MainCentre, 2, 5);
		// The look is applied at the frame (D325): at once here.
		Building->ApplyPendingAppearance();
		TestEqual(TEXT("a new colour repaints them"),
			CountPiecesOfColor(*Building, 5) + CountPiecesOfColor(*Building, 5, AHOLBuilding::BaseTopTone), 2);
		Building->GetComponents(Pieces);
		TestEqual(TEXT("without rebuilding the pieces"), Pieces.Num(), 4);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// AR7a checks (AR-Q2a, AR-Q2b): the Archery range's row of the real table, the Barracks' double, and its provisional
// shape built from it: a low shelter along one side, no tower, three targets at the owner's colour in the yard. The
// Barracks keeps the block and tower of D261.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingArcheryRangeTest, "HeroesOfLiberty.Buildings.ArcheryRange", UE::HeroesOfLiberty::Tests::Buildings::Flags)
bool FHOLBuildingArcheryRangeTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Buildings;

	const FHOLBuildingTypeRow* Range = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::ArcheryRange);
	const FHOLBuildingTypeRow* Barracks = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Barracks);
	if (!TestNotNull(TEXT("Archery range in the building type table"), Range) || !TestNotNull(TEXT("Barracks in the table"), Barracks))
	{
		return false;
	}
	// AR-Q2a: the Barracks' double.
	TestFalse(TEXT("named"), Range->DisplayName.IsEmpty());
	TestEqual(TEXT("3 x 3 tiles (12 x 12 m)"), Range->Footprint, FVector2D(1200.0, 1200.0));
	TestEqual(TEXT("height 600"), Range->Height, 600.f);
	TestEqual(TEXT("1500 health points"), Range->MaxHealth, 1500);
	TestEqual(TEXT("ranged armour 40"), Range->RangedArmor, 40);
	TestEqual(TEXT("150 wood, nothing else"), Range->Cost.ToString(), FHOLResources(0, 150, 0, 0).ToString());
	TestEqual(TEXT("built in 40 s"), Range->BuildTime, 40.f);
	TestEqual(TEXT("queue of 5"), Range->MaxQueue, 5);
	TestTrue(TEXT("military"), Range->Category == EHOLBuildingCategory::Military);
	TestEqual(TEXT("age 0"), Range->Age, 0);
	TestEqual(TEXT("no population"), Range->PopulationProvided, 0);
	TestTrue(TEXT("no deposit"), Range->Accepts.IsEmpty());
	TestTrue(TEXT("no final mesh: its provisional shape"), Range->Mesh.IsNull());
	TestTrue(TEXT("its shape: a range (AR-Q2b)"), Range->Shape == EHOLBuildingShape::Range);
	TestTrue(TEXT("the Barracks keeps the block and tower (D261)"), Barracks->Shape == EHOLBuildingShape::BlockAndTower);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	const auto GetBox = [](const TArray<UStaticMeshComponent*>& Pieces)
	{
		FBox Box(ForceInit);
		for (const UStaticMeshComponent* Piece : Pieces)
		{
			Box += Piece->Bounds.GetBox();
		}
		return Box;
	};

	// The finished range, at colour 4.
	AHOLBuilding* Building = World->SpawnActor<AHOLBuilding>();
	if (TestNotNull(TEXT("range spawned"), Building))
	{
		Building->Setup(HOLBuildingTypeNames::ArcheryRange, 1, 4);
		Building->ApplyPendingAppearance();
		TArray<UStaticMeshComponent*> Pieces;
		Building->GetComponents(Pieces);
		const FBox Box = GetBox(Pieces);
		const double Seen = UHOLBuildingSettings::GetVisualSize(Range->Footprint).X;
		TestTrue(FString::Printf(TEXT("the visible size of its 12 x 12 m (%.0f cm), on the ground, centred"), Seen),
			FVector2D(Box.GetSize()).Equals(FVector2D(Seen), 1.0) && Box.Min.Equals(FVector(-Seen * 0.5, -Seen * 0.5, 0.0), 1.0));

		// Low: nothing higher than the shelter, the block of D261 without its tower.
		const double BlockTop = Range->Height * AHOLBuilding::BaseHeightRatio;
		const double ShelterTop = Range->Height * (AHOLBuilding::BaseHeightRatio + AHOLBuilding::CapThicknessRatio);
		TestTrue(FString::Printf(TEXT("low, no tower: top at %.0f cm, the shelter's %.0f"), Box.Max.Z, ShelterTop),
			FMath::IsNearlyEqual(Box.Max.Z, ShelterTop, 1.0));
		// The pieces reaching the block's height: the shelter and its roof, along one whole side, at most half the depth.
		int32 Tall = 0;
		bool bShelterAlongASide = true;
		FBox Shelter(ForceInit);
		for (const UStaticMeshComponent* Piece : Pieces)
		{
			const FBox PieceBox = Piece->Bounds.GetBox();
			if (PieceBox.Max.Z >= BlockTop - 1.0)
			{
				++Tall;
				Shelter += PieceBox;
				bShelterAlongASide &= FMath::IsNearlyEqual(PieceBox.GetSize().Y, Seen, 1.0) && PieceBox.GetSize().X <= Seen * 0.5;
			}
		}
		TestEqual(TEXT("the shelter and its roof alone reach the block's height"), Tall, 2);
		TestTrue(TEXT("the shelter: one whole side, at most half the depth"), bShelterAlongASide
			&& (FMath::IsNearlyEqual(Shelter.Min.X, -Seen * 0.5, 1.0) || FMath::IsNearlyEqual(Shelter.Max.X, Seen * 0.5, 1.0)));
		TestEqual(TEXT("its roof at the darker tone of the owner's colour"), CountPiecesOfColor(*Building, 4, AHOLBuilding::BaseTopTone), 1);

		// Three targets' marks at the owner's colour, in the yard (out of the shelter), seen from above (not edge-on).
		int32 Targets = 0;
		int32 InYard = 0;
		FLinearColor Expected = FLinearColor(HOLPlayerColors::GetLinearColor(4));
		Expected.A = 1.f;
		for (const UStaticMeshComponent* Piece : Pieces)
		{
			FLinearColor Color;
			const UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(Piece->GetMaterial(0));
			if (Material && Material->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("Color")), Color) && Color.Equals(Expected, 1.e-4f))
			{
				++Targets;
				const FBox PieceBox = Piece->Bounds.GetBox();
				const bool bOutOfShelter = PieceBox.Min.X > Shelter.Max.X || PieceBox.Max.X < Shelter.Min.X;
				const bool bSeenFromAbove = PieceBox.GetSize().X >= 0.5 * PieceBox.GetSize().Y;
				InYard += bOutOfShelter && bSeenFromAbove ? 1 : 0;
			}
		}
		TestEqual(TEXT("three targets' marks at the owner's colour"), Targets, 3);
		TestEqual(TEXT("in the yard, facing up"), InYard, 3);
		Building->Destroy();
	}

	// The Barracks: still a block and a tower, as high as its type.
	AHOLBuilding* Other = World->SpawnActor<AHOLBuilding>();
	if (TestNotNull(TEXT("barracks spawned"), Other))
	{
		Other->Setup(HOLBuildingTypeNames::Barracks, 1, 4);
		Other->ApplyPendingAppearance();
		TArray<UStaticMeshComponent*> Pieces;
		Other->GetComponents(Pieces);
		TestEqual(TEXT("barracks: a block, its top, a tower and its roof"), Pieces.Num(), 4);
		TestTrue(TEXT("barracks: its tower as high as its type"), FMath::IsNearlyEqual(GetBox(Pieces).Max.Z, static_cast<double>(Barracks->Height), 1.0));
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
		// The look is applied at the frame (D325): at once here.
		Other->ApplyPendingAppearance();
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
		// The look is applied at the frame (D325): at once here.
		Building->ApplyPendingAppearance();
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
		TestTrue(TEXT("over the visible shape (D324 refined: inset in its 6 x 6 m)"), FVector2D(Box.GetSize()).Equals(UHOLBuildingSettings::GetVisualSize(House->Footprint), 1.0));
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
		// The look is applied at the frame (D325): at once here.
		Building->ApplyPendingAppearance();
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
		// The look is applied at the frame (D325): at once here.
		Building->ApplyPendingAppearance();
		Box = FBox(ForceInit);
		for (const UStaticMeshComponent* Piece : Pieces)
		{
			Box += Piece->Bounds.GetBox();
		}
		TestTrue(FString::Printf(TEXT("half its work: half its height (%.0f cm)"), Box.Max.Z), FMath::IsNearlyEqual(Box.Max.Z, House->Height * 0.5, 2.0));

		// Finished: the building's shape, whole, its forbidden area counted.
		Building->SetConstructionState(EHOLConstructionState::Finished);
		// The look is applied at the frame (D325): at once here.
		Building->ApplyPendingAppearance();
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
		// The look is applied at the frame (D325): at once here.
		Building->ApplyPendingAppearance();

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

// D345.2a: a building's health (its work's share, the damage kept), its armour against arrows, its destruction (its
// footprint reopened, rubble that blocks nothing nor answers a click).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingDamageTest, "HeroesOfLiberty.Buildings.Damage", UE::HeroesOfLiberty::Tests::Buildings::Flags)
bool FHOLBuildingDamageTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	const FHOLBuildingTypeRow* House = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::House);
	AHOLBuilding* Site = World->SpawnActor<AHOLBuilding>();
	if (TestNotNull(TEXT("house in the table"), House) && TestNotNull(TEXT("site spawned"), Site))
	{
		TestEqual(TEXT("every building: 40 ranged armour (D345)"), House->RangedArmor, 40);
		Site->Setup(HOLBuildingTypeNames::House, 0, 1, EHOLConstructionState::Foundation);
		TestEqual(TEXT("a bare foundation: 1 health"), Site->GetHealth(), 1);
		Site->SetConstructionState(EHOLConstructionState::UnderConstruction);
		Site->SetConstructionWork(Site->GetTotalWork() / 2);
		TestEqual(TEXT("half built: half the health"), Site->GetHealth(), House->MaxHealth / 2);
		TestEqual(TEXT("50 ranged: 10 dealt"), UHOLConstructionSubsystem::DamageBuilding(*Site, 50, EHOLAttackType::Ranged), 10);
		TestTrue(TEXT("hurt"), Site->IsHurt());
		Site->SetConstructionWork(Site->GetTotalWork());
		Site->SetConstructionState(EHOLConstructionState::Finished);
		TestEqual(TEXT("finished: the damage kept"), Site->GetHealth(), House->MaxHealth - 10);

		TArray<UBoxComponent*> Boxes;
		Site->GetComponents(Boxes);
		TestTrue(TEXT("its footprint blocks the navmesh"), Boxes.Num() == 1 && Boxes[0]->CanEverAffectNavigation());
		TestEqual(TEXT("a blow of all its health: all of it dealt"), UHOLConstructionSubsystem::DamageBuilding(*Site, 100000, EHOLAttackType::Melee),
			House->MaxHealth - 10);
		TestTrue(TEXT("destroyed"), !IsValid(Site));
		TestTrue(TEXT("its footprint no longer blocks"), Boxes.Num() == 1 && !Boxes[0]->CanEverAffectNavigation());
		int32 Rubbles = 0;
		for (TActorIterator<AHOLRubble> It(World); It; ++It)
		{
			++Rubbles;
			TestEqual(TEXT("the rubble of a house"), It->GetTypeName(), HOLBuildingTypeNames::House);
			TestTrue(TEXT("it goes away after 10 s"), FMath::IsNearlyEqual(It->GetLifeSpan(), AHOLRubble::LifeSeconds, 0.5f));
			FHitResult Hit;
			const FVector Above = It->GetActorLocation() + FVector(0.0, 0.0, 5000.0);
			TestFalse(TEXT("the rubble answers no click"), World->LineTraceSingleByChannel(Hit, Above, Above - FVector(0.0, 0.0, 10000.0), HOLCollision::Selection));
		}
		TestEqual(TEXT("rubble left"), Rubbles, 1);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

#endif
