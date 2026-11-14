// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dev/HOLTestHooks.h"
#include "Dev/HOLTestSequence.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// The panel of a scenario (HOLScenarioPanel) shows HOLTestHooks::GetScenarioDisplay(): its notes and its state. These tests run a short
// scenario through the real runner (HOLTestSequence::Run, as hol.Scenario does) and read what the panel would show.
namespace HOLScenarioNoteTestsPrivate
{
	/** A run waiting for its key that never comes within this many frames is a failure, not a hang. */
	constexpr int32 NoteTestMaxFrames = 600;

	void StartScenarioSteps(const TArray<FString>& InSteps)
	{
		// The panel starts again at each scenario (HOLScenario.cpp).
		HOLTestHooks::GetScenarioDisplay() = HOLTestHooks::FScenarioDisplay();
		HOLTestSequence::FRunOptions Options;
		Options.bQuitOnTimeout = false;
		TArray<FString> Steps = InSteps;
		HOLTestSequence::Run(TEXT("ScenarioNoteTest"), MoveTemp(Steps), MoveTemp(Options));
	}
}

// notereset <Text>: the compare scenarios show one option at a time, its note replacing the notes before it (note only adds).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLScenarioNoteResetTest, "HeroesOfLiberty.Dev.Scenario.NoteReset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLScenarioNoteResetTest::RunTest(const FString& Parameters)
{
	HOLScenarioNoteTestsPrivate::StartScenarioSteps({ TEXT("note A"), TEXT("note C"), TEXT("notereset B"), TEXT("note D"), TEXT("waitkey") });
	const TSharedRef<int32> Frames = MakeShared<int32>(0);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Frames]()
	{
		HOLTestHooks::FScenarioDisplay& Display = HOLTestHooks::GetScenarioDisplay();
		if (Display.WaitingForKey.IsEmpty() && ++*Frames < HOLScenarioNoteTestsPrivate::NoteTestMaxFrames)
		{
			return false;
		}
		TestFalse(TEXT("the run reached its waitkey"), Display.WaitingForKey.IsEmpty());
		TestEqual(TEXT("the notes after note A, note C, notereset B, note D"), Display.Notes, TArray<FString>{ TEXT("B"), TEXT("D") });
		HOLTestHooks::PressContinueKey();
		return true;
	}));
	return true;
}

// The scenario is ready to play as soon as its first waitkey waits (the user plays while it waits), not only once the last step is done.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLScenarioReadyAtWaitKeyTest, "HeroesOfLiberty.Dev.Scenario.ReadyAtFirstWaitKey",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLScenarioReadyAtWaitKeyTest::RunTest(const FString& Parameters)
{
	HOLScenarioNoteTestsPrivate::StartScenarioSteps({ TEXT("note A"), TEXT("waitkey"), TEXT("note B"), TEXT("waitkey") });
	const TSharedRef<int32> Frames = MakeShared<int32>(0);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Frames]()
	{
		HOLTestHooks::FScenarioDisplay& Display = HOLTestHooks::GetScenarioDisplay();
		if (Display.WaitingForKey.IsEmpty() && ++*Frames < HOLScenarioNoteTestsPrivate::NoteTestMaxFrames)
		{
			return false;
		}
		TestFalse(TEXT("the run reached its first waitkey"), Display.WaitingForKey.IsEmpty());
		TestTrue(TEXT("the scenario is ready at its first waitkey"), Display.State == HOLTestHooks::FScenarioDisplay::EState::Ready);
		HOLTestHooks::PressContinueKey();
		return true;
	}));
	return true;
}

// quit: an unattended run (play.ps1 -Hidden, here the automation run) closes the game at the scenario's real end, the steps after it never run;
// play.ps1 -Hidden then ends with the game instead of waiting for its -TimeoutSec.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLScenarioQuitTest, "HeroesOfLiberty.Dev.Scenario.QuitUnattended",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLScenarioQuitTest::RunTest(const FString& Parameters)
{
	const TSharedRef<int32> Quits = MakeShared<int32>(0);
	HOLTestHooks::OnScenarioQuit() = [Quits]() { ++*Quits; };
	HOLScenarioNoteTestsPrivate::StartScenarioSteps({ TEXT("note A"), TEXT("quit"), TEXT("note After"), TEXT("waitkey") });
	const TSharedRef<int32> Frames = MakeShared<int32>(0);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Frames, Quits]()
	{
		HOLTestHooks::FScenarioDisplay& Display = HOLTestHooks::GetScenarioDisplay();
		// Without the verb, quit is a plain console command (the engine's own quit) and the run goes on to its waitkey.
		if (*Quits == 0 && Display.WaitingForKey.IsEmpty() && ++*Frames < HOLScenarioNoteTestsPrivate::NoteTestMaxFrames)
		{
			return false;
		}
		HOLTestHooks::OnScenarioQuit() = nullptr;
		TestEqual(TEXT("the unattended run closed the game once at its quit step"), *Quits, 1);
		TestEqual(TEXT("no step after quit ran (the notes)"), Display.Notes, TArray<FString>{ TEXT("A") });
		TestTrue(TEXT("no step after quit ran (no waitkey waiting)"), Display.WaitingForKey.IsEmpty());
		HOLTestHooks::PressContinueKey();
		return true;
	}));
	return true;
}

#endif
