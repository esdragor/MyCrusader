// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Map/HOLFogMode.h"
#include "Map/HOLVisionSubsystem.h"
#include "HAL/IConsoleManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCampSubsystem.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "UObject/UnrealType.h"

#if WITH_DEV_AUTOMATION_TESTS

// N7: the state of a neutral camp (alive / cleared, its living members) read through the doors that the script binds (UFUNCTIONs
// of UHOLCampSubsystem, found by name and called by reflection as the script's binding does), on the server's real chain: a
// test world with the real units, combat processor and camp subsystem, steps run by hand.
namespace UE::HeroesOfLiberty::Tests::CampState
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FHOLMatchConfig MakeConfig()
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : Index <= 2 ? EHOLSlotState::AI : EHOLSlotState::Closed;
			Config.Slots[Index].Team = 0;
		}
		return Config;
	}

	struct FWorld
	{
		UWorld* World = nullptr;
		AHOLGameState* GameState = nullptr;
		AHOLPlayerState* Player = nullptr;
		AHOLBuilding* Forum = nullptr;
		UHOLUnitSubsystem* Units = nullptr;
		UHOLSimulationSubsystem* Simulation = nullptr;
		UHOLCampSubsystem* Camps = nullptr;
		UHOLCombatProcessor* Combat = nullptr;
		TSharedPtr<FMassEntityManager> EntityManager;
		IConsoleVariable* CombatEnabled = nullptr;
		int32 FormerEnabled = 0;
		float StepSeconds = 0.05f;

		FWorld()
		{
			CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
			if (CombatEnabled)
			{
				FormerEnabled = CombatEnabled->GetInt();
				CombatEnabled->Set(1, ECVF_SetByCode);
			}
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			GameState = World->SpawnActor<AHOLGameState>();
			World->SetGameState(GameState);
			Player = World->SpawnActor<AHOLPlayerState>();
			Forum = World->SpawnActor<AHOLBuilding>(FVector(5000.0, 2000.0, 0.0), FRotator::ZeroRotator);
			Units = World->GetSubsystem<UHOLUnitSubsystem>();
			Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
			Camps = World->GetSubsystem<UHOLCampSubsystem>();
			UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
			if (!GameState || !Player || !Forum || !Units || !Simulation || !Camps || !EntitySubsystem || !CombatEnabled)
			{
				return;
			}
			GameState->InitVictory(MakeConfig());
			Player->SetMatchSlot(0, 0);
			if (!GameState->PlayerArray.Contains(Player))
			{
				GameState->AddPlayerState(Player);
			}
			Forum->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
			Forum->SetPrimaryForum(true);
			// The world does not begin play here: the camps listen to the end of the steps from this call.
			Camps->OnWorldBeginPlay(*World);
			EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
			Combat = NewObject<UHOLCombatProcessor>(Simulation);
			Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
			StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		}

		~FWorld()
		{
			if (CombatEnabled)
			{
				CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
			}
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		bool IsReady() const { return Combat != nullptr; }

		/** One step as UHOLSimulationSubsystem::RunStep makes it. */
		void RunStep()
		{
			Simulation->GetOnStepStarted().Broadcast(Simulation->GetStepIndex());
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			Units->FlushDestructions();
			const int64 Step = Simulation->AdvanceStepIndexForTests();
			Simulation->GetOnStepEnded().Broadcast(Step);
		}

		void RunStepsTo(const int64 Target)
		{
			while (Simulation->GetStepIndex() < Target)
			{
				RunStep();
			}
		}
	};

	/** The door by name, as the script binding reaches it: null while the camp subsystem has no such UFUNCTION. */
	UFunction* Door(const UHOLCampSubsystem& Camps, const TCHAR* Name)
	{
		return Camps.FindFunction(FName(Name));
	}

	/** The count of camps, through the reflected door; -1 without it. */
	int32 CallNumCamps(UHOLCampSubsystem& Camps)
	{
		UFunction* Function = Door(Camps, TEXT("GetNumCamps"));
		if (!Function)
		{
			return -1;
		}
		struct { int32 ReturnValue = -1; } Params;
		Camps.ProcessEvent(Function, &Params);
		return Params.ReturnValue;
	}

	/** Whether the camp is cleared, through the reflected door; -1 without it, else 0 or 1. */
	int32 CallIsCleared(UHOLCampSubsystem& Camps, const int32 CampId)
	{
		UFunction* Function = Door(Camps, TEXT("IsCampCleared"));
		if (!Function)
		{
			return -1;
		}
		struct { int32 CampId = 0; bool ReturnValue = false; } Params;
		Params.CampId = CampId;
		Camps.ProcessEvent(Function, &Params);
		return Params.ReturnValue ? 1 : 0;
	}

	/** The camp's living members at its last thought, through the reflected door; -1 without it. */
	int32 CallLivingMembers(UHOLCampSubsystem& Camps, const int32 CampId)
	{
		UFunction* Function = Door(Camps, TEXT("GetCampLivingMembers"));
		if (!Function)
		{
			return -1;
		}
		struct { int32 CampId = 0; int32 ReturnValue = -1; } Params;
		Params.CampId = CampId;
		Camps.ProcessEvent(Function, &Params);
		return Params.ReturnValue;
	}

	/** The camp's post (cm), through the reflected door; the origin without it. */
	FVector CallPost(UHOLCampSubsystem& Camps, const int32 CampId)
	{
		UFunction* Function = Door(Camps, TEXT("GetCampPost"));
		if (!Function)
		{
			return FVector::ZeroVector;
		}
		struct { int32 CampId = 0; FVector ReturnValue = FVector::ZeroVector; } Params;
		Params.CampId = CampId;
		Camps.ProcessEvent(Function, &Params);
		return Params.ReturnValue;
	}
}

// N7 (Q1 C): what a player's state holds of the camps (AHOLPlayerState::CampViews, replicated to him alone), read by reflection so this
// test builds before the property exists (then it is red: Num -1).
namespace UE::HeroesOfLiberty::Tests::CampState
{
	struct FSeenCamps
	{
		int32 Num = -1;
		bool bCleared = false;
		int32 Living = -1;
	};

	/** How many camps the player's state holds, and the first one's state; Num -1 without the property. */
	FSeenCamps ReadViews(AHOLPlayerState& Player)
	{
		FSeenCamps Out;
		const FArrayProperty* Array = FindFProperty<FArrayProperty>(AHOLPlayerState::StaticClass(), TEXT("CampViews"));
		const FStructProperty* Inner = Array ? CastField<FStructProperty>(Array->Inner) : nullptr;
		if (!Inner)
		{
			return Out;
		}
		FScriptArrayHelper Helper(Array, Array->ContainerPtrToValuePtr<void>(&Player));
		Out.Num = Helper.Num();
		if (Out.Num > 0)
		{
			const uint8* First = Helper.GetRawPtr(0);
			if (const FBoolProperty* Cleared = CastField<FBoolProperty>(Inner->Struct->FindPropertyByName(TEXT("bCleared"))))
			{
				Out.bCleared = Cleared->GetPropertyValue_InContainer(First);
			}
			if (const FByteProperty* Living = CastField<FByteProperty>(Inner->Struct->FindPropertyByName(TEXT("Living"))))
			{
				Out.Living = Living->GetPropertyValue_InContainer(First);
			}
		}
		return Out;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCampStateDoorsTest, "HeroesOfLiberty.Neutrals.CampStateDoors", UE::HeroesOfLiberty::Tests::CampState::Flags)

// A camp of three Spearmen: alive with three living members once adopted; when its members die, cleared and none living.
bool FHOLCampStateDoorsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::CampState;
	FWorld W;
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	UHOLCampSubsystem& Camps = *W.Camps;
	if (!TestNotNull(TEXT("A script door GetNumCamps"), Door(Camps, TEXT("GetNumCamps")))
		|| !TestNotNull(TEXT("A script door IsCampCleared"), Door(Camps, TEXT("IsCampCleared")))
		|| !TestNotNull(TEXT("A script door GetCampLivingMembers"), Door(Camps, TEXT("GetCampLivingMembers")))
		|| !TestNotNull(TEXT("A script door GetCampPost"), Door(Camps, TEXT("GetCampPost"))))
	{
		return false;
	}
	TestEqual(TEXT("No camp yet"), CallNumCamps(Camps), 0);

	const int32 CampId = Camps.AddCamp(FVector(20000.0, 0.0, 0.0), FName(TEXT("Spearman")), 3);
	TestEqual(TEXT("The first camp is camp 0"), CampId, 0);
	TestEqual(TEXT("One camp"), CallNumCamps(Camps), 1);
	TestEqual(TEXT("Its post is where it was made"), CallPost(Camps, CampId), FVector(20000.0, 0.0, 0.0));
	W.RunStepsTo(16);
	TestEqual(TEXT("Born and adopted: not cleared"), CallIsCleared(Camps, CampId), 0);
	TestEqual(TEXT("... three living members"), CallLivingMembers(Camps, CampId), 3);

	W.Units->KillUnitsOfOwner(static_cast<uint8>(FHOLMatchConfig::NeutralOwner));
	W.RunStepsTo(32);
	TestEqual(TEXT("Its members fallen: cleared"), CallIsCleared(Camps, CampId), 1);
	TestEqual(TEXT("... none living"), CallLivingMembers(Camps, CampId), 0);
	W.RunStepsTo(48);
	TestEqual(TEXT("Cleared for good: still cleared"), CallIsCleared(Camps, CampId), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCampViewsTest, "HeroesOfLiberty.Neutrals.CampViews", UE::HeroesOfLiberty::Tests::CampState::Flags)

// N7 (Q1 C), the real chain: the camp a player's camp has in sight reaches his state, alive with its members, then cleared; with the fog,
// a camp never in his sight does not reach it (the one seen before stays, as last seen).
bool FHOLCampViewsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::CampState;
	FWorld W;
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	TestEqual(TEXT("No camp: the player's state holds none"), ReadViews(*W.Player).Num, 0);
	W.Camps->AddCamp(FVector(20000.0, 0.0, 0.0), FName(TEXT("Spearman")), 3);
	W.RunStepsTo(16);
	FSeenCamps Seen = ReadViews(*W.Player);
	TestEqual(TEXT("No fog: the camp reaches the player's state"), Seen.Num, 1);
	TestFalse(TEXT("... alive"), Seen.bCleared);
	TestEqual(TEXT("... with its three members"), Seen.Living, 3);

	W.Units->KillUnitsOfOwner(static_cast<uint8>(FHOLMatchConfig::NeutralOwner));
	W.RunStepsTo(32);
	Seen = ReadViews(*W.Player);
	TestEqual(TEXT("Still one camp in the player's state"), Seen.Num, 1);
	TestTrue(TEXT("... now cleared"), Seen.bCleared);
	TestEqual(TEXT("... none living"), Seen.Living, 0);

	UHOLVisionSubsystem* Vision = W.World->GetSubsystem<UHOLVisionSubsystem>();
	if (Vision)
	{
		Vision->Start(W.GameState->GetLaunchConfig(), EHOLFogMode::Normal);
		Vision->Update();
	}
	if (!Vision || !Vision->IsStarted())
	{
		AddInfo(TEXT("the fog did not start in this world: the fog case is not checked"));
		return true;
	}
	W.Camps->AddCamp(FVector(-30000.0, -30000.0, 0.0), FName(TEXT("Spearman")), 2);
	W.RunStepsTo(48);
	TestEqual(TEXT("With the fog, a camp never in sight is not in the player's state (the first one stays, as last seen)"), ReadViews(*W.Player).Num, 1);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
