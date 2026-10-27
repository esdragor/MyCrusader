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
#include "Player/HOLPlayerController.h"
#include "Misc/AutomationTest.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Buildings/HOLRubble.h"
#include "EngineUtils.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
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

// IA1.2 F5 (IA1.2-Q3): a farm (a Walkable type of the real table) is walked over and stood on, finished or not: its footprint
// never blocks the navmesh (no forbidden area, so no keep-out nor body on the paths, which all read BlocksNavigation); a mill
// still does. Another building is still refused on a farm, and a farm on another building (the placement ignores units).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingWalkableTest, "HeroesOfLiberty.Buildings.Walkable", UE::HeroesOfLiberty::Tests::Buildings::Flags)
bool FHOLBuildingWalkableTest::RunTest(const FString& Parameters)
{
	using EState = EHOLConstructionState;

	const FHOLBuildingTypeRow* Farm = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Farm);
	const FHOLBuildingTypeRow* Mill = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Mill);
	if (!TestNotNull(TEXT("farm in the building type table"), Farm) || !TestNotNull(TEXT("mill in the building type table"), Mill))
	{
		return false;
	}
	TestTrue(TEXT("the farm is walked over (table)"), Farm->Walkable);
	TestFalse(TEXT("the mill is not (table)"), Mill->Walkable);

	// The rule: a Walkable type never blocks; any other as D312 (not as a bare foundation).
	TestFalse(TEXT("rule: walkable foundation"), HOLConstruction::BlocksNavigation(EState::Foundation, true));
	TestFalse(TEXT("rule: walkable site"), HOLConstruction::BlocksNavigation(EState::UnderConstruction, true));
	TestFalse(TEXT("rule: walkable, finished"), HOLConstruction::BlocksNavigation(EState::Finished, true));
	TestFalse(TEXT("rule: other foundation"), HOLConstruction::BlocksNavigation(EState::Foundation, false));
	TestTrue(TEXT("rule: other site"), HOLConstruction::BlocksNavigation(EState::UnderConstruction, false));
	TestTrue(TEXT("rule: other, finished"), HOLConstruction::BlocksNavigation(EState::Finished, false));

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	// A farm at the origin (8 x 8 m: -400 to 400), through its whole construction, as the build chain sets its states.
	AHOLBuilding* FarmBuilding = World->SpawnActor<AHOLBuilding>(FVector::ZeroVector, FRotator::ZeroRotator);
	// A finished mill 30 m away (2600 to 3400 along X).
	AHOLBuilding* MillBuilding = World->SpawnActor<AHOLBuilding>(FVector(3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("farm spawned"), FarmBuilding) && TestNotNull(TEXT("mill spawned"), MillBuilding))
	{
		TArray<UBoxComponent*> FarmBoxes;
		FarmBuilding->GetComponents(FarmBoxes);
		const auto FarmAreaCounts = [&FarmBoxes]() { return FarmBoxes.Num() == 1 && FarmBoxes[0]->CanEverAffectNavigation(); };

		FarmBuilding->Setup(HOLBuildingTypeNames::Farm, 0, 1, EState::Foundation);
		TestFalse(TEXT("farm foundation: does not block (D312)"), FarmBuilding->BlocksNavigation());
		TestFalse(TEXT("farm foundation: no forbidden area"), FarmAreaCounts());

		FarmBuilding->SetConstructionState(EState::UnderConstruction);
		FarmBuilding->SetConstructionWork(FarmBuilding->GetTotalWork() / 2);
		TestFalse(TEXT("farm site: does not block"), FarmBuilding->BlocksNavigation());
		TestFalse(TEXT("farm site: no forbidden area"), FarmAreaCounts());

		FarmBuilding->SetConstructionWork(FarmBuilding->GetTotalWork());
		FarmBuilding->SetConstructionState(EState::Finished);
		TestTrue(TEXT("farm finished"), FarmBuilding->IsFinished());
		TestFalse(TEXT("farm finished: does not block"), FarmBuilding->BlocksNavigation());
		TestFalse(TEXT("farm finished: no forbidden area"), FarmAreaCounts());

		MillBuilding->Setup(HOLBuildingTypeNames::Mill, 1, 2);
		TArray<UBoxComponent*> MillBoxes;
		MillBuilding->GetComponents(MillBoxes);
		TestTrue(TEXT("mill finished: blocks"), MillBuilding->BlocksNavigation());
		TestTrue(TEXT("mill finished: its forbidden area counts"), MillBoxes.Num() == 1 && MillBoxes[0]->CanEverAffectNavigation());

		// What the keep-out of the bodies and the paths read (HOLSimulationProcessors, HOLUnitOrderProcessor): the mill alone.
		int32 Blocking = 0;
		for (TActorIterator<AHOLBuilding> It(World); It; ++It)
		{
			Blocking += It->GetType() && It->BlocksNavigation() ? 1 : 0;
		}
		TestEqual(TEXT("one footprint kept out of: the mill's"), Blocking, 1);

		// The placement: buildings still refused over a farm, a farm still refused over a building (D311: touching is fine).
		const auto Check = [World](const FVector2D& Min, const FVector2D& Max)
		{
			return int32(UHOLConstructionSubsystem::CheckPlacement(*World, FBox2D(Min, Max), 0));
		};
		TestEqual(TEXT("a second farm half over the farm: refused"),
			Check(FVector2D(0.0, -400.0), FVector2D(800.0, 400.0)), int32(EHOLPlacementRefusal::Building));
		TestEqual(TEXT("a house on the farm: refused"),
			Check(FVector2D(-400.0, -400.0), FVector2D(400.0, 400.0)), int32(EHOLPlacementRefusal::Building));
		TestEqual(TEXT("a farm half over the mill: refused"),
			Check(FVector2D(2800.0, -400.0), FVector2D(3600.0, 400.0)), int32(EHOLPlacementRefusal::Building));
		TestNotEqual(TEXT("a house touching the farm: no building in the way"),
			Check(FVector2D(400.0, -400.0), FVector2D(1200.0, 400.0)), int32(EHOLPlacementRefusal::Building));
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// IA1.2 F6a (IA1.2-Q2): a farm's field is born and dies with the farm, on the real chain in a test world: the real building
// and source tables, the real source registry started as at the match start, the states set as the build chain sets them
// (HOLBuildProcessor ends a site by SetConstructionState(Finished), its one place), the destruction through DamageBuilding
// (what the combat calls). A farm under construction has no field; finished, it has one at its centre, its owner's; a mill
// none; a farm born finished (a deferred spawn set up finished, as hol.Cheat.SpawnBuilding of the scenarios, or set up
// finished after a spawn) has its own; a destroyed farm's field leaves the map, once, and the others stay.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingFarmFieldTest, "HeroesOfLiberty.Buildings.FarmField", UE::HeroesOfLiberty::Tests::Buildings::Flags)
bool FHOLBuildingFarmFieldTest::RunTest(const FString& Parameters)
{
	using EState = EHOLConstructionState;

	const FHOLBuildingTypeRow* Farm = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Farm);
	const FHOLBuildingTypeRow* Mill = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Mill);
	if (!TestNotNull(TEXT("farm in the building type table"), Farm) || !TestNotNull(TEXT("mill in the building type table"), Mill))
	{
		return false;
	}
	TestEqual(TEXT("the farm's source (table)"), Farm->Source, HOLSourceTypeNames::FarmField);
	TestTrue(TEXT("the mill has none (table)"), Mill->Source.IsNone());

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLSourceSubsystem* Sources = World->GetSubsystem<UHOLSourceSubsystem>();
	if (TestNotNull(TEXT("source registry"), Sources))
	{
		// As the match starts: the registry takes sources from now on (no map here: none of its own).
		Sources->OnWorldBeginPlay(*World);
		const int32 FieldType = Sources->GetTypeNames().IndexOfByKey(HOLSourceTypeNames::FarmField);
		TestTrue(TEXT("FarmField in the source type table"), FieldType != INDEX_NONE);
		// The standing farm fields of a slot (any slot when negative).
		const auto CountFields = [Sources, FieldType](const int32 Slot)
		{
			int32 Count = 0;
			for (const FHOLSource& Source : Sources->GetSources())
			{
				Count += Source.IsStanding() && Source.TypeIndex == FieldType && (Slot < 0 || Source.OwnerSlot == Slot) ? 1 : 0;
			}
			return Count;
		};

		// Slot 1's farm through its whole construction, as the build chain sets its states.
		AHOLBuilding* FarmBuilding = World->SpawnActor<AHOLBuilding>(FVector(1200.0, 800.0, 0.0), FRotator::ZeroRotator);
		AHOLBuilding* MillBuilding = World->SpawnActor<AHOLBuilding>(FVector(5000.0, 0.0, 0.0), FRotator::ZeroRotator);
		if (TestNotNull(TEXT("farm spawned"), FarmBuilding) && TestNotNull(TEXT("mill spawned"), MillBuilding))
		{
			FarmBuilding->Setup(HOLBuildingTypeNames::Farm, 1, 1, EState::Foundation);
			TestEqual(TEXT("a farm foundation: no field"), CountFields(-1), 0);
			TestEqual(TEXT("a farm foundation: no linked source"), FarmBuilding->GetLinkedSource(), int32(INDEX_NONE));
			FarmBuilding->SetConstructionState(EState::UnderConstruction);
			FarmBuilding->SetConstructionWork(FarmBuilding->GetTotalWork() / 2);
			TestEqual(TEXT("a farm site half built: no field"), CountFields(-1), 0);
			TestEqual(TEXT("a farm site: no linked source"), FarmBuilding->GetLinkedSource(), int32(INDEX_NONE));

			// Finished as HOLBuildProcessor ends a site: all its work, then the state.
			FarmBuilding->SetConstructionWork(FarmBuilding->GetTotalWork());
			FarmBuilding->SetConstructionState(EState::Finished);
			TestTrue(TEXT("the farm finished"), FarmBuilding->IsFinished());
			const int32 Field = FarmBuilding->GetLinkedSource();
			TestEqual(TEXT("the finished farm: one field standing"), CountFields(-1), 1);
			TestEqual(TEXT("the field is slot 1's"), CountFields(1), 1);
			if (TestTrue(TEXT("the finished farm holds its field's index"), Sources->GetSources().IsValidIndex(Field)))
			{
				const FHOLSource& Source = Sources->GetSources()[Field];
				TestTrue(TEXT("its field stands"), Source.IsStanding());
				TestEqual(TEXT("its field is a FarmField"), static_cast<int32>(Source.TypeIndex), FieldType);
				TestEqual(TEXT("its field at its centre"), Source.Location, FarmBuilding->GetActorLocation());
				TestEqual(TEXT("its field's owner: the farm's"), static_cast<int32>(Source.OwnerSlot), 1);
				TestTrue(TEXT("its field gives food"), Source.Resource == EHOLResource::Food);
				TestTrue(TEXT("its field is linked (no shape, no hole)"), Sources->IsLinked(Field));
			}
			// Told again (the state set again, set up again finished): still the one field, the same index.
			FarmBuilding->SetConstructionState(EState::Finished);
			FarmBuilding->Setup(HOLBuildingTypeNames::Farm, 1, 1);
			TestEqual(TEXT("told again: still one field"), CountFields(-1), 1);
			TestEqual(TEXT("told again: the same index"), FarmBuilding->GetLinkedSource(), Field);

			// A mill finished the same way: no source of its own.
			const int32 SourcesBeforeMill = Sources->GetSources().Num();
			MillBuilding->Setup(HOLBuildingTypeNames::Mill, 1, 1, EState::Foundation);
			MillBuilding->SetConstructionState(EState::UnderConstruction);
			MillBuilding->SetConstructionWork(MillBuilding->GetTotalWork());
			MillBuilding->SetConstructionState(EState::Finished);
			TestTrue(TEXT("the mill finished"), MillBuilding->IsFinished());
			TestEqual(TEXT("the finished mill: no source added"), Sources->GetSources().Num(), SourcesBeforeMill);
			TestEqual(TEXT("the mill: no linked source"), MillBuilding->GetLinkedSource(), int32(INDEX_NONE));

			// Born finished as hol.Cheat.SpawnBuilding (the scenarios), the main centres and the stress test do it: a deferred
			// spawn set up finished, then finished spawning.
			const FTransform BornTransform(FRotator::ZeroRotator, FVector(-3000.0, 0.0, 0.0));
			AHOLBuilding* BornFarm = World->SpawnActorDeferred<AHOLBuilding>(AHOLBuilding::StaticClass(), BornTransform, nullptr, nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (TestNotNull(TEXT("farm born finished spawned"), BornFarm))
			{
				BornFarm->Setup(HOLBuildingTypeNames::Farm, 2, 2);
				BornFarm->FinishSpawning(BornTransform);
				const int32 BornField = BornFarm->GetLinkedSource();
				if (TestTrue(TEXT("born finished: its field's index"), Sources->GetSources().IsValidIndex(BornField)))
				{
					const FHOLSource& Source = Sources->GetSources()[BornField];
					TestTrue(TEXT("born finished: its field stands"), Source.IsStanding());
					TestEqual(TEXT("born finished: its field at its centre"), Source.Location, BornFarm->GetActorLocation());
					TestEqual(TEXT("born finished: its field slot 2's"), static_cast<int32>(Source.OwnerSlot), 2);
				}
			}
			// Set up finished after a plain spawn (as the tests do).
			AHOLBuilding* SetUpFarm = World->SpawnActor<AHOLBuilding>(FVector(-3000.0, 3000.0, 0.0), FRotator::ZeroRotator);
			if (TestNotNull(TEXT("farm set up finished spawned"), SetUpFarm))
			{
				SetUpFarm->Setup(HOLBuildingTypeNames::Farm, 0, 0);
				TestTrue(TEXT("set up finished: its field"), Sources->GetSources().IsValidIndex(SetUpFarm->GetLinkedSource()));
			}
			TestEqual(TEXT("three fields standing"), CountFields(-1), 3);

			// Slot 1's farm destroyed the combat's way (D345.2a): all its health in one blow.
			const int32 StandingBefore = Sources->GetNumStanding();
			TestTrue(TEXT("the blow dealt the farm's health"), UHOLConstructionSubsystem::DamageBuilding(*FarmBuilding, 100000, EHOLAttackType::Melee) > 0);
			TestFalse(TEXT("the farm destroyed"), IsValid(FarmBuilding));
			TestTrue(TEXT("its field no longer stands"), Sources->GetSources().IsValidIndex(Field) && !Sources->GetSources()[Field].IsStanding());
			TestEqual(TEXT("its linked source forgotten"), FarmBuilding->GetLinkedSource(), int32(INDEX_NONE));
			TestEqual(TEXT("one source standing less"), Sources->GetNumStanding(), StandingBefore - 1);
			TestEqual(TEXT("no field of slot 1 left"), CountFields(1), 0);
			TestEqual(TEXT("the two other fields still stand"), CountFields(-1), 2);
			int32 Rubbles = 0;
			for (TActorIterator<AHOLRubble> It(World); It; ++It)
			{
				++Rubbles;
			}
			TestEqual(TEXT("the farm's rubble left (D345.2a)"), Rubbles, 1);

			// Destroyed again (a second blow, a second destruction, a second release): nothing more taken off the map.
			TestEqual(TEXT("a second blow: nothing dealt"), UHOLConstructionSubsystem::DamageBuilding(*FarmBuilding, 100000, EHOLAttackType::Melee), 0);
			UHOLConstructionSubsystem::DestroyBuilding(*FarmBuilding);
			FarmBuilding->ReleaseLinkedSource();
			TestEqual(TEXT("destroyed twice: still two fields"), CountFields(-1), 2);
			TestEqual(TEXT("destroyed twice: nothing more removed"), Sources->GetNumStanding(), StandingBefore - 1);

			// The mill destroyed: no field taken with it.
			UHOLConstructionSubsystem::DamageBuilding(*MillBuilding, 100000, EHOLAttackType::Melee);
			TestFalse(TEXT("the mill destroyed"), IsValid(MillBuilding));
			TestEqual(TEXT("the mill destroyed: the two fields still stand"), CountFields(-1), 2);
		}
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// IA1.2 F7: a right click on one of the player's farms finished gathers its field (the rally point too), on every machine:
// a client has no LinkedSource (server only), so the controller finds the field by AHOLBuilding::FindLinkedSourceHere (the
// standing linked source at the farm's centre, UHOLSourceSubsystem::FindLinkedSourceAt), the same index as the server's
// GetLinkedSource. Real building and source tables, the real registry in a test world. A farm under construction or
// destroyed has none; a berry bush at a mill's centre is not taken for a field; a field is never picked by a click
// (AHOLPlayerController::IsPickableSource, read by FindSourceAt: the click falls on the farm), a bush and a tree are.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingFarmClickTest, "HeroesOfLiberty.Buildings.FarmClick", UE::HeroesOfLiberty::Tests::Buildings::Flags)
bool FHOLBuildingFarmClickTest::RunTest(const FString& Parameters)
{
	using EState = EHOLConstructionState;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLSourceSubsystem* Sources = World->GetSubsystem<UHOLSourceSubsystem>();
	if (TestNotNull(TEXT("source registry"), Sources))
	{
		// As the match starts: the registry takes sources from now on.
		Sources->OnWorldBeginPlay(*World);
		const auto FindType = [Sources](const FName Name) -> const FHOLSourceTypeRow*
		{
			const int32 Index = Sources->GetTypeNames().IndexOfByKey(Name);
			return Index == INDEX_NONE ? nullptr : Sources->GetType(static_cast<uint8>(Index));
		};

		// The picking rule, from the real source type table.
		const FHOLSourceTypeRow* FieldType = FindType(HOLSourceTypeNames::FarmField);
		const FHOLSourceTypeRow* BushType = FindType(HOLSourceTypeNames::BerryBush);
		const FHOLSourceTypeRow* TreeType = FindType(HOLSourceTypeNames::Tree);
		if (TestNotNull(TEXT("FarmField in the source type table"), FieldType))
		{
			TestFalse(TEXT("a farm field is not picked by a click"), AHOLPlayerController::IsPickableSource(*FieldType));
		}
		if (TestNotNull(TEXT("BerryBush in the source type table"), BushType))
		{
			TestTrue(TEXT("a berry bush is picked by a click"), AHOLPlayerController::IsPickableSource(*BushType));
		}
		if (TestNotNull(TEXT("Tree in the source type table"), TreeType))
		{
			TestTrue(TEXT("a tree is picked by a click"), AHOLPlayerController::IsPickableSource(*TreeType));
		}

		// Slot 1's farm through its construction, as the build chain sets its states.
		AHOLBuilding* FarmBuilding = World->SpawnActor<AHOLBuilding>(FVector(1200.0, 800.0, 0.0), FRotator::ZeroRotator);
		AHOLBuilding* MillBuilding = World->SpawnActor<AHOLBuilding>(FVector(5000.0, 0.0, 0.0), FRotator::ZeroRotator);
		if (TestNotNull(TEXT("farm spawned"), FarmBuilding) && TestNotNull(TEXT("mill spawned"), MillBuilding))
		{
			const FVector Centre = FarmBuilding->GetActorLocation();
			FarmBuilding->Setup(HOLBuildingTypeNames::Farm, 1, 1, EState::Foundation);
			TestEqual(TEXT("a farm foundation: no field found"), FarmBuilding->FindLinkedSourceHere(), int32(INDEX_NONE));
			FarmBuilding->SetConstructionState(EState::UnderConstruction);
			FarmBuilding->SetConstructionWork(FarmBuilding->GetTotalWork() / 2);
			TestEqual(TEXT("a farm site: no field found"), FarmBuilding->FindLinkedSourceHere(), int32(INDEX_NONE));
			TestEqual(TEXT("a farm site: no linked source at its centre"), Sources->FindLinkedSourceAt(Centre), int32(INDEX_NONE));

			// Finished as HOLBuildProcessor ends a site.
			FarmBuilding->SetConstructionWork(FarmBuilding->GetTotalWork());
			FarmBuilding->SetConstructionState(EState::Finished);
			const int32 Field = FarmBuilding->GetLinkedSource();
			if (TestTrue(TEXT("the finished farm has its field (F6a)"), Sources->GetSources().IsValidIndex(Field)))
			{
				TestEqual(TEXT("the finished farm: its field found, the server's index"), FarmBuilding->FindLinkedSourceHere(), Field);
				TestEqual(TEXT("its field found at its centre"), Sources->FindLinkedSourceAt(Centre), Field);
				// A client's place travels as floats (FHOLNetSource): a few cm off is still the field.
				TestEqual(TEXT("its field found a few cm off"), Sources->FindLinkedSourceAt(Centre + FVector(3.0, -2.0, 0.0)), Field);
				TestEqual(TEXT("no field 5 m off"), Sources->FindLinkedSourceAt(Centre + FVector(500.0, 0.0, 0.0)), int32(INDEX_NONE));
			}

			// Slot 2's farm born finished elsewhere: its own field, not slot 1's.
			const FTransform BornTransform(FRotator::ZeroRotator, FVector(-3000.0, 0.0, 0.0));
			AHOLBuilding* BornFarm = World->SpawnActorDeferred<AHOLBuilding>(AHOLBuilding::StaticClass(), BornTransform, nullptr, nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (TestNotNull(TEXT("farm born finished spawned"), BornFarm))
			{
				BornFarm->Setup(HOLBuildingTypeNames::Farm, 2, 2);
				BornFarm->FinishSpawning(BornTransform);
				const int32 BornField = BornFarm->GetLinkedSource();
				TestTrue(TEXT("born finished: its field (F6a)"), Sources->GetSources().IsValidIndex(BornField));
				TestEqual(TEXT("born finished: its field found, the server's index"), BornFarm->FindLinkedSourceHere(), BornField);
				TestNotEqual(TEXT("born finished: not slot 1's field"), BornFarm->FindLinkedSourceHere(), Field);
			}

			// A finished mill with a berry bush at its very centre: a bush is no field.
			MillBuilding->Setup(HOLBuildingTypeNames::Mill, 1, 1);
			const int32 Bush = Sources->AddSource(HOLSourceTypeNames::BerryBush, MillBuilding->GetActorLocation());
			TestTrue(TEXT("the bush added"), Sources->GetSources().IsValidIndex(Bush));
			TestEqual(TEXT("a mill: no field found"), MillBuilding->FindLinkedSourceHere(), int32(INDEX_NONE));
			TestEqual(TEXT("a bush at a building's centre is no linked source"), Sources->FindLinkedSourceAt(MillBuilding->GetActorLocation()),
				int32(INDEX_NONE));

			// Slot 1's farm destroyed the combat's way: its field gone, nothing found where it stood; slot 2's still found.
			UHOLConstructionSubsystem::DamageBuilding(*FarmBuilding, 100000, EHOLAttackType::Melee);
			TestFalse(TEXT("the farm destroyed"), IsValid(FarmBuilding));
			TestEqual(TEXT("the farm destroyed: no field at its centre"), Sources->FindLinkedSourceAt(Centre), int32(INDEX_NONE));
			if (BornFarm)
			{
				TestEqual(TEXT("the other farm's field still found"), BornFarm->FindLinkedSourceHere(), BornFarm->GetLinkedSource());
			}
		}
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// IA1.2 F8: a farm is a flat field a unit walks over, on the real chain in a test world: the real building table, the looks
// built as at the frame (ApplyPendingAppearance) through the states the build chain sets. Its owner's bare foundation (the
// ghost), its site (at its first work, half built) and the finished farm: the visible pieces under FieldMaxHeight and over
// its 8 x 8 m (10 %); finished, its soil, a border at the owner's colour and FieldRowCount rows of crops; the selection ray
// over its centre finds it. A mill keeps its high shape.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingFarmShapeTest, "HeroesOfLiberty.Buildings.FarmShape", UE::HeroesOfLiberty::Tests::Buildings::Flags)
bool FHOLBuildingFarmShapeTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Buildings;
	using EState = EHOLConstructionState;

	const FHOLBuildingTypeRow* Farm = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Farm);
	const FHOLBuildingTypeRow* Mill = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Mill);
	if (!TestNotNull(TEXT("farm in the building type table"), Farm) || !TestNotNull(TEXT("mill in the building type table"), Mill))
	{
		return false;
	}
	TestTrue(TEXT("the farm: walked over, no mesh (table)"), Farm->Walkable && Farm->Mesh.IsNull());
	// What the health bar and the selection marker read (HOLSelectionMarkers): as low as the field.
	TestTrue(FString::Printf(TEXT("the farm's height in the table (%.0f cm) under %.0f cm"), Farm->Height, AHOLBuilding::FieldMaxHeight),
		Farm->Height > 0.f && Farm->Height < AHOLBuilding::FieldMaxHeight);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	// The box of the visible pieces (the ghost's unseen click box left out).
	const auto VisibleBox = [](const AHOLBuilding& Building, int32* OutCount = nullptr)
	{
		TArray<UStaticMeshComponent*> Pieces;
		Building.GetComponents(Pieces);
		FBox Box(ForceInit);
		int32 Count = 0;
		for (const UStaticMeshComponent* Piece : Pieces)
		{
			if (Piece->IsVisible())
			{
				Box += Piece->Bounds.GetBox();
				++Count;
			}
		}
		if (OutCount)
		{
			*OutCount = Count;
		}
		return Box;
	};
	const auto CheckFlat = [this, Farm](const FBox& Box, const TCHAR* State)
	{
		TestTrue(FString::Printf(TEXT("%s: pieces seen"), State), Box.IsValid != 0);
		TestTrue(FString::Printf(TEXT("%s: under %.0f cm (%.1f cm)"), State, AHOLBuilding::FieldMaxHeight, Box.Max.Z),
			Box.IsValid && Box.Max.Z < AHOLBuilding::FieldMaxHeight);
		TestTrue(FString::Printf(TEXT("%s: on the ground (%.1f cm)"), State, Box.Min.Z), Box.IsValid && FMath::IsNearlyZero(Box.Min.Z, 1.0));
		const FVector2D Size(Box.GetSize());
		TestTrue(FString::Printf(TEXT("%s: over its 8 x 8 m at 10 %% (%.0f x %.0f cm)"), State, Size.X, Size.Y),
			Box.IsValid && FMath::Abs(Size.X - Farm->Footprint.X) <= 0.1 * Farm->Footprint.X
			&& FMath::Abs(Size.Y - Farm->Footprint.Y) <= 0.1 * Farm->Footprint.Y);
	};
	const auto HitsCentre = [World](const AHOLBuilding& Building)
	{
		FHitResult Hit;
		const FVector Above = Building.GetActorLocation() + FVector(0.0, 0.0, 5000.0);
		World->LineTraceSingleByChannel(Hit, Above, Above - FVector(0.0, 0.0, 10000.0), HOLCollision::Selection);
		return Hit.GetActor() == &Building;
	};

	// The player's own farm (its local controller owns it, as in Buildings.Foundation), at the origin.
	APlayerController* Owner = World->SpawnActor<APlayerController>();
	if (Owner)
	{
		Owner->SetAsLocalPlayerController();
	}
	AHOLBuilding* FarmBuilding = World->SpawnActor<AHOLBuilding>(FVector::ZeroVector, FRotator::ZeroRotator);
	AHOLBuilding* MillBuilding = World->SpawnActor<AHOLBuilding>(FVector(3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("controller spawned"), Owner) && TestNotNull(TEXT("farm spawned"), FarmBuilding)
		&& TestNotNull(TEXT("mill spawned"), MillBuilding))
	{
		FarmBuilding->SetOwner(Owner);
		FarmBuilding->Setup(HOLBuildingTypeNames::Farm, 0, 2, EState::Foundation);
		// The look is applied at the frame (D325): at once here.
		FarmBuilding->ApplyPendingAppearance();
		TestTrue(TEXT("foundation: its owner's, seen here"), FarmBuilding->IsSeenHere());
		CheckFlat(VisibleBox(*FarmBuilding), TEXT("foundation (ghost)"));
		TestTrue(TEXT("foundation: the selection ray over its centre finds it"), HitsCentre(*FarmBuilding));

		FarmBuilding->SetConstructionState(EState::UnderConstruction);
		FarmBuilding->ApplyPendingAppearance();
		CheckFlat(VisibleBox(*FarmBuilding), TEXT("site, first work"));
		TestTrue(TEXT("site: the selection ray over its centre finds it"), HitsCentre(*FarmBuilding));
		FarmBuilding->SetConstructionWork(FarmBuilding->GetTotalWork() / 2);
		FarmBuilding->ApplyPendingAppearance();
		int32 SitePieces = 0;
		CheckFlat(VisibleBox(*FarmBuilding, &SitePieces), TEXT("site, half built"));

		FarmBuilding->SetConstructionWork(FarmBuilding->GetTotalWork());
		FarmBuilding->SetConstructionState(EState::Finished);
		FarmBuilding->ApplyPendingAppearance();
		int32 FieldPieces = 0;
		CheckFlat(VisibleBox(*FarmBuilding, &FieldPieces), TEXT("finished"));
		TestEqual(TEXT("finished: its soil, 4 border pieces and its rows of crops"), FieldPieces, 1 + 4 + AHOLBuilding::FieldRowCount);
		TestEqual(TEXT("finished: the border at the owner's colour"), CountPiecesOfColor(*FarmBuilding, 2), 4);
		TestTrue(TEXT("site: the foundation under the rising field"), SitePieces > FieldPieces);
		TestTrue(TEXT("finished: the selection ray over its centre finds it"), HitsCentre(*FarmBuilding));

		// A mill, finished: its block and tower, as high as before.
		MillBuilding->Setup(HOLBuildingTypeNames::Mill, 1, 3);
		MillBuilding->ApplyPendingAppearance();
		const FBox MillBox = VisibleBox(*MillBuilding);
		TestTrue(FString::Printf(TEXT("the mill keeps its high shape (%.0f cm)"), MillBox.Max.Z), MillBox.IsValid && MillBox.Max.Z > 100.0);
		TestTrue(TEXT("the mill still found by the selection ray"), HitsCentre(*MillBuilding));
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

#endif
