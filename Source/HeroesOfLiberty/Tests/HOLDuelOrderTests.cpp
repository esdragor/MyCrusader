// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLCommandSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLDuelSubsystem.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// D4 (DuelInProgress.md; GDD 10.1, D16, D80): the duel commands through the order queue to the script's book and back to the sheets
// every machine reads. The real chain, no number typed in: a hero subsystem's step end takes the orders, the duel subsystem measures the
// heroes (their entities) and the script book decides; the sheets (the true ones and the game state's copies) are read.
namespace UE::HeroesOfLiberty::Tests::DuelOrders
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDuelOrdersInMatchTest, "HeroesOfLiberty.Duel.OrdersInMatch", UE::HeroesOfLiberty::Tests::DuelOrders::Flags)

// Six heroes (slots 0-5, enemies of each other) in pairs: 0 and 1 side by side, 2 and 3 side by side, 4 and 5 side by side, the pairs
// far from each other (and 0 far from nobody else's range):
// - slot 0 challenges slot 4 (too far): refused, nothing shown;
// - slot 0 challenges slot 1: the sheets show a challenge sent (0) and received (1) with the window's end step, in the game state's
//   copies too; slot 0 cannot answer its own challenge; a third slot's sheet shows nothing;
// - slot 1 accepts: both sheets show the duel on with its circle, the book lets neither touch a third hero;
// - slot 2 challenges slot 3, slot 3 refuses: nothing shown any more; slot 2 challenges again: refused (its cooldown is spent);
// - slot 4 challenges slot 5 and nobody answers: the window runs out, nothing shown, and the book told it ended.
bool FHOLDuelOrdersInMatchTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	UHOLDuelSubsystem* Duels = UHOLDuelSubsystem::Find(World);
	if (TestNotNull(TEXT("game state"), GameState) && TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation)
		&& TestNotNull(TEXT("heroes"), Heroes) && TestNotNull(TEXT("commands"), Commands) && TestNotNull(TEXT("the script book of duels"), Duels))
	{
		TestTrue(TEXT("The hero subsystem follows the steps"), Heroes->BindSteps());
		int64 Step = 0;
		const auto EndStep = [Simulation, &Step]
		{
			++Step;
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		const auto Born = [&](const int32 Slot, const FVector& Place)
		{
			FHOLUnitSpawnRequest Request;
			Request.TypeName = FName(TEXT("Hero"));
			Request.Count = 1;
			Request.OwnerPlayer = static_cast<uint8>(Slot);
			Request.Center = Place;
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
		};
		constexpr double Gap = 400.0;
		constexpr double Far = 1000000.0;
		Born(0, FVector(0.0, 0.0, 0.0));
		Born(1, FVector(Gap, 0.0, 0.0));
		Born(2, FVector(Far, 0.0, 0.0));
		Born(3, FVector(Far + Gap, 0.0, 0.0));
		Born(4, FVector(0.0, Far, 0.0));
		Born(5, FVector(Gap, Far, 0.0));
		EndStep();
		bool bAllAlive = true;
		for (int32 Slot = 0; Slot < 6; ++Slot)
		{
			bAllAlive = bAllAlive && Heroes->GetSheets(Slot).Public.State == EHOLHeroState::Alive;
		}
		if (TestTrue(TEXT("the six heroes are alive"), bAllAlive))
		{
			const auto Code = [&](const int32 Slot) { return static_cast<int32>(Heroes->GetSheets(Slot).Public.DuelCode); };
			const auto Order = [&](const int32 Slot, const EHOLDuelOrderKind Kind, const int32 Other)
			{
				Commands->IssueDuelOrder(Slot, Kind, Other);
				EndStep();
			};

			// Too far (slot 0 at the origin, slot 4 a million centimetres away).
			Order(0, EHOLDuelOrderKind::Challenge, 4);
			TestEqual(TEXT("a challenge at a hero too far: nothing shown for the challenger"), Code(0), 0);
			TestEqual(TEXT("... nor for the target"), Code(4), 0);
			// Not an enemy's slot in the range, and oneself.
			Order(0, EHOLDuelOrderKind::Challenge, 0);
			Order(0, EHOLDuelOrderKind::Challenge, FHOLMatchConfig::MaxSlots);
			TestEqual(TEXT("a challenge at oneself or out of range: nothing shown"), Code(0), 0);

			// The challenge.
			Order(0, EHOLDuelOrderKind::Challenge, 1);
			TestEqual(TEXT("the challenger's sheet: a challenge sent"), Code(0), 1);
			TestEqual(TEXT("... against slot 1"), static_cast<int32>(Heroes->GetSheets(0).Public.DuelOpponent), 1);
			TestEqual(TEXT("the target's sheet: a challenge received"), Code(1), 2);
			TestEqual(TEXT("... from slot 0"), static_cast<int32>(Heroes->GetSheets(1).Public.DuelOpponent), 0);
			TestTrue(TEXT("... the window closes at a coming step"), Heroes->GetSheets(1).Public.DuelEndStep > 0);
			TestEqual(TEXT("a third slot's sheet shows nothing"), Code(2), 0);
			TestEqual(TEXT("the game state's copy (what the clients read): the challenger's"), static_cast<int32>(GameState->GetHeroSheet(0).DuelCode), 1);
			TestEqual(TEXT("... the target's"), static_cast<int32>(GameState->GetHeroSheet(1).DuelCode), 2);
			TestTrue(TEXT("... the copy is the true sheet"), GameState->GetHeroSheet(1) == Heroes->GetSheets(1).Public);
			TestTrue(TEXT("a challenge pending is not a duel on: nobody is protected yet"), Duels->CanHitNow(0, 2));

			// The challenger cannot answer its own challenge.
			Order(0, EHOLDuelOrderKind::Accept, INDEX_NONE);
			TestEqual(TEXT("the challenger's own acceptance changes nothing"), Code(0), 1);
			TestEqual(TEXT("... the challenge still waits"), Code(1), 2);

			// The acceptance.
			Order(1, EHOLDuelOrderKind::Accept, INDEX_NONE);
			TestEqual(TEXT("accepted: the challenger's sheet shows the duel on"), Code(0), 3);
			TestEqual(TEXT("... the target's"), Code(1), 3);
			TestTrue(TEXT("... with its circle"), Heroes->GetSheets(0).Public.DuelRadius > 0 && Heroes->GetSheets(0).Public.DuelCenter == Heroes->GetSheets(1).Public.DuelCenter);
			TestEqual(TEXT("... the circle is centred between the two"), Heroes->GetSheets(0).Public.DuelCenter.X, static_cast<int32>(Gap / 2.0));
			TestEqual(TEXT("the book: slot 0's adversary is slot 1"), Duels->GetAdversaryNow(0), 1);
			TestFalse(TEXT("the duellist touches no third hero (the book)"), Duels->CanHitNow(0, 2));
			TestTrue(TEXT("the duellists touch each other"), Duels->CanHitNow(0, 1));
			TestEqual(TEXT("the game state's copy shows the duel on"), static_cast<int32>(GameState->GetHeroSheet(1).DuelCode), 3);

			// The refusal, then the cooldown of the challenge.
			Order(2, EHOLDuelOrderKind::Challenge, 3);
			TestEqual(TEXT("slot 2 challenges slot 3: sent"), Code(2), 1);
			Order(3, EHOLDuelOrderKind::Refuse, INDEX_NONE);
			TestEqual(TEXT("refused: nothing shown for the challenger"), Code(2), 0);
			TestEqual(TEXT("... nor for the target"), Code(3), 0);
			Order(2, EHOLDuelOrderKind::Challenge, 3);
			TestEqual(TEXT("a second challenge at once: refused, the first one spent the cooldown"), Code(2), 0);
			Order(3, EHOLDuelOrderKind::Refuse, INDEX_NONE);
			TestEqual(TEXT("an answer with no challenge waiting changes nothing"), Code(3), 0);

			// The silence.
			Order(4, EHOLDuelOrderKind::Challenge, 5);
			TestEqual(TEXT("slot 4 challenges slot 5: received"), Code(5), 2);
			const int32 WindowSteps = FMath::CeilToInt(10.f / GetDefault<UHOLSimulationSettings>()->GetStepSeconds());
			for (int32 Index = 0; Index < WindowSteps && Code(5) == 2; ++Index)
			{
				EndStep();
			}
			TestEqual(TEXT("nobody answered: the window ran out, nothing shown for the target"), Code(5), 0);
			TestEqual(TEXT("... nor for the challenger"), Code(4), 0);
			TestEqual(TEXT("... the book ended two duels so far: the refusal and the silence"), Duels->GetNumEnded(), 2);

			// A hero apart from the others cannot be challenged back by the one under challenge: orders of no player's slot are dropped.
			Commands->IssueDuelOrder(INDEX_NONE, EHOLDuelOrderKind::Challenge, 1);
			Commands->IssueDuelOrder(FHOLMatchConfig::MaxSlots, EHOLDuelOrderKind::Accept, INDEX_NONE);
			TArray<FHOLDuelOrder> Left;
			Commands->ConsumeDuelOrders(Left);
			TestEqual(TEXT("the orders of no player's slot dropped"), Left.Num(), 0);
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
