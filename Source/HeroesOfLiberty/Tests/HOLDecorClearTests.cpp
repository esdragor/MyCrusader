// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Dev/HOLDecorClear.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/AutomationTest.h"

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

#endif
