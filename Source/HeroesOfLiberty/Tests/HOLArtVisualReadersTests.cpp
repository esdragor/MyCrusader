// Copyright Epic Games, Inc. All Rights Reserved.

// D924 (AR0-a2): the art visual table read by the readers outside the Mass constants: the client reception (mesh scale),
// the arrows (projectile, muzzle smoke), the buildings and the sources (mesh). Mass simulation tests (D838), the assets made
// in memory (HOLArtVisual::AddTestEntry) as in HOLArtVisualTests.cpp; each level falls back field by field to the CSV row.

#include "Art/HOLArtVisual.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Net/HOLUnitReceptionSubsystem.h"
#include "Presentation/HOLArrowSubsystem.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace UE::HeroesOfLiberty::Tests::ArtVisualReaders
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FName Spearman(TEXT("Spearman"));
	const FName Archer(TEXT("Archer"));
	const FName Legions(TEXT("Legions"));
	const FSoftObjectPath ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
	const FSoftObjectPath CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	const FSoftObjectPath SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	/** A game world with its subsystems; the test entries cleared on both ends. */
	struct FReadersWorld
	{
		UWorld* World = nullptr;

		FReadersWorld()
		{
			HOLArtVisual::ClearTestEntries();
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
		}

		~FReadersWorld()
		{
			HOLArtVisual::ClearTestEntries();
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}
	};

	UHOLArtVisual* MakeAsset(const FName TypeName, const FName Faction, const TCHAR* Name)
	{
		UHOLArtVisual* Asset = NewObject<UHOLArtVisual>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UHOLArtVisual::StaticClass(), FName(Name)));
		Asset->TypeName = TypeName;
		Asset->Faction = Faction;
		HOLArtVisual::AddTestEntry(Asset);
		return Asset;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLArtVisualReceptionScaleTest, "HeroesOfLiberty.Art.Visual.ReceptionMeshScaleFromAsset", UE::HeroesOfLiberty::Tests::ArtVisualReaders::Flags)

bool FHOLArtVisualReceptionScaleTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ArtVisualReaders;
	FReadersWorld Game;
	UHOLUnitReceptionSubsystem* Reception = Game.World->GetSubsystem<UHOLUnitReceptionSubsystem>();
	if (!TestNotNull(TEXT("client reception in a game world"), Reception))
	{
		return false;
	}
	UHOLArtVisual* Common = MakeAsset(Spearman, NAME_None, TEXT("DA_Art_Spearman"));
	Common->bOverrideMeshScale = true;
	Common->MeshScale = 2.5f;
	UHOLArtVisual* Own = MakeAsset(Spearman, Legions, TEXT("DA_Art_Spearman_Legions"));
	Own->bOverrideMeshScale = true;
	Own->MeshScale = 3.f;

	// The reception's own reader (MakeView's MeshScale), the CSV row's value given as the last level.
	TestEqual(TEXT("a Spearman of no faction: the common asset's scale"), Reception->GetArtMeshScale(Spearman, NAME_None, 1.f), 2.5f);
	TestEqual(TEXT("a Legions Spearman: his faction's asset's scale"), Reception->GetArtMeshScale(Spearman, Legions, 1.f), 3.f);
	TestEqual(TEXT("an Archer, no asset: the CSV row's scale"), Reception->GetArtMeshScale(Archer, NAME_None, 1.25f), 1.25f);
	TestEqual(TEXT("asked again: the cached scale"), Reception->GetArtMeshScale(Spearman, Legions, 1.f), 3.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLArtVisualArrowsTest, "HeroesOfLiberty.Art.Visual.ArrowsReadProjectileAndSmoke", UE::HeroesOfLiberty::Tests::ArtVisualReaders::Flags)

bool FHOLArtVisualArrowsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ArtVisualReaders;
	FReadersWorld Game;
	UHOLArrowSubsystem* Display = Game.World->GetSubsystem<UHOLArrowSubsystem>();
	const UStaticMesh* Cone = Cast<UStaticMesh>(ConeMesh.TryLoad());
	const UStaticMesh* Cube = Cast<UStaticMesh>(CubeMesh.TryLoad());
	if (!TestNotNull(TEXT("arrow display in a game world"), Display) || !TestNotNull(TEXT("engine cone"), Cone) || !TestNotNull(TEXT("engine cube"), Cube))
	{
		return false;
	}
	TestFalse(TEXT("before: the Archer's CSV projectile is not the cone"), Display->GetProjectileMesh(Archer) == Cone);
	UHOLArtVisual* Asset = MakeAsset(Archer, NAME_None, TEXT("DA_Art_Archer"));
	Asset->Projectile = ConeMesh;
	Asset->MuzzleSmoke = CubeMesh;

	TestTrue(TEXT("an Archer shoots his art asset's projectile"), Display->GetProjectileMesh(Archer) == Cone);
	TestTrue(TEXT("an Archer's muzzle smoke is his art asset's"), Display->GetMuzzleSmokeMesh(Archer) == Cube);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLArtVisualBuildingMeshTest, "HeroesOfLiberty.Art.Visual.BuildingMeshFactionThenCommonThenCsv", UE::HeroesOfLiberty::Tests::ArtVisualReaders::Flags)

bool FHOLArtVisualBuildingMeshTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ArtVisualReaders;
	FReadersWorld Game;
	const FName Tower(TEXT("ArtTestTower"));
	const FName Hut(TEXT("ArtTestHut"));
	MakeAsset(Tower, NAME_None, TEXT("DA_Art_ArtTestTower"))->Mesh = TSoftObjectPtr<UStaticMesh>(ConeMesh);
	MakeAsset(Tower, Legions, TEXT("DA_Art_ArtTestTower_Legions"))->Mesh = TSoftObjectPtr<UStaticMesh>(CubeMesh);

	// The resolver AHOLBuilding::GetVisualMesh reads (the owner's faction key, the row's Mesh as the fallback).
	TestEqual(TEXT("a Legions tower: its faction's mesh"), HOLArtVisual::ResolveMesh(Game.World, Tower, Legions, SphereMesh).ToString(), CubeMesh.ToString());
	TestEqual(TEXT("a tower of another faction: the common mesh"), HOLArtVisual::ResolveMesh(Game.World, Tower, NAME_None, SphereMesh).ToString(), ConeMesh.ToString());
	TestEqual(TEXT("a type without asset: the CSV row's mesh"), HOLArtVisual::ResolveMesh(Game.World, Hut, Legions, SphereMesh).ToString(), SphereMesh.ToString());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLArtVisualSourceMeshTest, "HeroesOfLiberty.Art.Visual.SourceMeshFromCommonAsset", UE::HeroesOfLiberty::Tests::ArtVisualReaders::Flags)

bool FHOLArtVisualSourceMeshTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ArtVisualReaders;
	FReadersWorld Game;
	const FName Berries(TEXT("ArtTestBerries"));
	UHOLArtVisual* Asset = MakeAsset(Berries, NAME_None, TEXT("DA_Art_ArtTestBerries"));
	Asset->Mesh = TSoftObjectPtr<UStaticMesh>(ConeMesh);
	Asset->bOverrideMeshScale = false;

	// A source has no faction (AHOLSourceField::GetOrMakeTypeDisplay asks with none): the common asset over an empty CSV mesh.
	TestEqual(TEXT("a source type: its art asset's mesh over the empty CSV one"), HOLArtVisual::ResolveMesh(Game.World, Berries, NAME_None, FSoftObjectPath()).ToString(), ConeMesh.ToString());
	TestEqual(TEXT("an asset without scale keeps the CSV scale"), HOLArtVisual::ResolveMeshScale(Game.World, Berries, NAME_None, 0.75f), 0.75f);
	return true;
}

#endif
