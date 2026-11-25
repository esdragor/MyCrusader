// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Dev/HOLDecorClear.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceConstant.h"
#include "MassVisualizer.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// hol.Decor.ClearNear (test tool of the `neutrals` scenario): the scenery around a point goes, the rest stays. Effect checked on a
// test world: Plain's grove (static mesh actors with MI_Trunk / MI_Crown), an instanced scenery mesh, and what must NOT go (a ground
// actor with another material, a grove tree far from the point).
namespace UE::HeroesOfLiberty::Tests::DecorClear
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	AStaticMeshActor* SpawnShape(UWorld& World, UStaticMesh* Mesh, const FName MaterialName, const FVector& At)
	{
		AStaticMeshActor* Actor = World.SpawnActor<AStaticMeshActor>(At, FRotator::ZeroRotator);
		if (Actor && Actor->GetStaticMeshComponent())
		{
			Actor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
			Actor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
			UMaterialInstanceConstant* Material = NewObject<UMaterialInstanceConstant>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UMaterialInstanceConstant::StaticClass(), MaterialName), RF_Transient);
			Actor->GetStaticMeshComponent()->SetMaterial(0, Material);
		}
		return Actor;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDecorClearTest, "HeroesOfLiberty.Dev.DecorClear", UE::HeroesOfLiberty::Tests::DecorClear::Flags)
bool FHOLDecorClearTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::DecorClear;
	using namespace UE::HeroesOfLiberty::Decor;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (TestNotNull(TEXT("the engine's cube"), Cube))
	{
		// Plain's grove: 3 trunks and 3 crowns within 20 m of the origin, one trunk and one crown at 100 m.
		for (int32 Number = 0; Number < 3; ++Number)
		{
			const FVector At(300.0 * Number, -200.0 * Number, 250.0);
			TestNotNull(TEXT("a trunk"), SpawnShape(*World, Cube, TEXT("MI_Trunk"), At));
			TestNotNull(TEXT("a crown"), SpawnShape(*World, Cube, TEXT("MI_Crown"), At + FVector(0.0, 0.0, 400.0)));
		}
		AStaticMeshActor* FarTrunk = SpawnShape(*World, Cube, TEXT("MI_Trunk"), FVector(10000.0, 0.0, 250.0));
		AStaticMeshActor* FarCrown = SpawnShape(*World, Cube, TEXT("MI_Crown"), FVector(10000.0, 0.0, 650.0));
		// The ground: a static mesh actor in the radius whose material is neither of the grove's: it stays.
		AStaticMeshActor* Ground = SpawnShape(*World, Cube, TEXT("MI_Ground"), FVector(0.0, 0.0, -50.0));

		// The scenery: one instanced mesh, 4 instances within 20 m, 2 at 100 m.
		AActor* Scenery = World->SpawnActor<AActor>();
		UInstancedStaticMeshComponent* Instances = NewObject<UInstancedStaticMeshComponent>(Scenery);
		Scenery->SetRootComponent(Instances);
		Instances->SetMobility(EComponentMobility::Movable);
		Instances->SetStaticMesh(Cube);
		Instances->RegisterComponent();
		for (const FVector& At : { FVector(100.0, 100.0, 0.0), FVector(-500.0, 400.0, 0.0), FVector(900.0, -900.0, 0.0), FVector(0.0, 1500.0, 0.0),
			FVector(9000.0, 3000.0, 0.0), FVector(-9500.0, -2000.0, 0.0) })
		{
			Instances->AddInstance(FTransform(At), /*bWorldSpace*/ true);
		}

		const FVector2D Origin(0.0, 0.0);
		const double Radius = 2000.0;
		const FDecorCount Before = CountNear(*World, Origin, Radius);
		TestEqual(TEXT("before: the six grove actors within the radius"), Before.Actors, 6);
		TestEqual(TEXT("before: the four scenery instances within the radius"), Before.Instances, 4);

		const FDecorCount Taken = ClearNear(*World, Origin, Radius);
		const FDecorCount After = CountNear(*World, Origin, Radius);
		TestEqual(TEXT("EFFECT: no decor left within the radius (actors)"), After.Actors, 0);
		TestEqual(TEXT("EFFECT: no decor left within the radius (instances)"), After.Instances, 0);
		TestEqual(TEXT("ClearNear reports the six actors it took"), Taken.Actors, 6);
		TestEqual(TEXT("ClearNear reports the four instances it took"), Taken.Instances, 4);
		TestEqual(TEXT("the two far instances stay in the mesh"), Instances->GetInstanceCount(), 2);
		TestTrue(TEXT("the far trunk stays"), IsValid(FarTrunk));
		TestTrue(TEXT("the far crown stays"), IsValid(FarCrown));
		TestTrue(TEXT("the ground (another material) stays"), IsValid(Ground));
		const FDecorCount Far = CountNear(*World, Origin, 20000.0);
		TestEqual(TEXT("the far decor is still counted: two grove actors, two instances"), Far.Total(), 4);
		const FDecorCount Again = ClearNear(*World, Origin, Radius);
		TestEqual(TEXT("a second call takes nothing"), Again.Total(), 0);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// hol.Decor.ClearNear ... Trees (HALDIR-1120pf, scenario neutrals_clairiere): the tree left in front of the camp is a WOOD SOURCE of the
// registry. Effect on the real registry of a test world: a wood source within the radius is still standing after ClearNear without the
// argument, gone after ClearNear with it; a far tree and a berry bush in the radius (not wood) stay.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDecorClearSourcesTest, "HeroesOfLiberty.Dev.DecorClearSources", UE::HeroesOfLiberty::Tests::DecorClear::Flags)
bool FHOLDecorClearSourcesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Decor;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLSourceSubsystem* Sources = World->GetSubsystem<UHOLSourceSubsystem>();
	if (TestNotNull(TEXT("the source registry"), Sources))
	{
		Sources->OnWorldBeginPlay(*World);
		const int32 NearTree = Sources->AddSource(HOLSourceTypeNames::Tree, FVector(500.0, 0.0, 0.0));
		const int32 FarTree = Sources->AddSource(HOLSourceTypeNames::Tree, FVector(9000.0, 0.0, 0.0));
		const int32 Berries = Sources->AddSource(HOLSourceTypeNames::BerryBush, FVector(0.0, 800.0, 0.0));
		if (TestTrue(TEXT("three sources added"), NearTree != INDEX_NONE && FarTree != INDEX_NONE && Berries != INDEX_NONE))
		{
			const FVector2D Origin(0.0, 0.0);
			const double Radius = 2000.0;
			TestEqual(TEXT("before: the near tree is counted when the sources are asked"), CountNear(*World, Origin, Radius, true).Sources, 1);
			TestEqual(TEXT("before: no source is counted without the argument"), CountNear(*World, Origin, Radius).Sources, 0);

			const FDecorCount Without = ClearNear(*World, Origin, Radius);
			TestEqual(TEXT("without the argument ClearNear takes no source"), Without.Sources, 0);
			TestTrue(TEXT("EFFECT: the near tree is still standing without the argument"), Sources->GetSources()[NearTree].IsStanding());

			const FDecorCount With = ClearNear(*World, Origin, Radius, true);
			TestEqual(TEXT("with the argument ClearNear reports the one wood source it took"), With.Sources, 1);
			TestFalse(TEXT("EFFECT: the near tree is gone with the argument"), Sources->GetSources()[NearTree].IsStanding());
			TestTrue(TEXT("the far tree stays"), Sources->GetSources()[FarTree].IsStanding());
			TestTrue(TEXT("the berry bush (not wood) stays"), Sources->GetSources()[Berries].IsStanding());
			TestEqual(TEXT("no wood source left within the radius"), CountNear(*World, Origin, Radius, true).Sources, 0);
			TestEqual(TEXT("a second call takes nothing"), ClearNear(*World, Origin, Radius, true).Total(), 0);
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// AR5-c (GLORFINDEL-1125ar5): hol.Art.Icon All crashed at its 14th type (Assertion Index < ArrayNum, Array.h:1339, in
// UInstancedStaticMeshComponent::SetPreviousTransformById from UMassVisualizationComponent::EndVisualChanges): the booth's
// hol.Decor.ClearNear took the instance of the Ram (a Mass unit drawn by the Mass visualizer's instanced meshes) 2500 cm from the
// next spot, behind Mass's back. Effect: an instance of the Mass visualizer within the radius stays after ClearNear (Mass owns it).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDecorClearMassVisualsTest, "HeroesOfLiberty.Dev.DecorClearMassVisuals", UE::HeroesOfLiberty::Tests::DecorClear::Flags)
bool FHOLDecorClearMassVisualsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Decor;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	AMassVisualizer* Visualizer = World->SpawnActor<AMassVisualizer>();
	if (TestNotNull(TEXT("the engine's cube"), Cube) && TestNotNull(TEXT("a Mass visualizer"), Visualizer))
	{
		// A Mass unit's visual: one instance of an instanced mesh of the Mass visualizer, 300 cm from the point.
		UInstancedStaticMeshComponent* UnitVisuals = NewObject<UInstancedStaticMeshComponent>(Visualizer);
		UnitVisuals->SetMobility(EComponentMobility::Movable);
		UnitVisuals->SetStaticMesh(Cube);
		UnitVisuals->SetupAttachment(Visualizer->GetRootComponent());
		UnitVisuals->RegisterComponent();
		UnitVisuals->AddInstance(FTransform(FVector(300.0, 0.0, 0.0)), /*bWorldSpace*/ true);

		const FDecorCount Taken = ClearNear(*World, FVector2D(0.0, 0.0), 3000.0, true);
		TestEqual(TEXT("EFFECT: the Mass unit's visual instance stays (Mass owns it)"), UnitVisuals->GetInstanceCount(), 1);
		TestEqual(TEXT("ClearNear takes no instance of the Mass visualizer"), Taken.Instances, 0);
		TestEqual(TEXT("CountNear does not count the Mass visualizer's instances as decor"), CountNear(*World, FVector2D(0.0, 0.0), 3000.0).Instances, 0);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
