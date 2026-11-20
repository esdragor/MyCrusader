// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dev/HOLTestHooks.h"
#include "Dev/HOLTestSequence.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS

// The panel of a scenario (HOLScenarioPanelScript.as) shows HOLTestHooks::GetScenarioDisplay(): its notes and its state. These tests run a short
// scenario through the real runner (HOLTestSequence::Run, as hol.Scenario does) and read what the panel would show.
namespace HOLScenarioNoteTestsPrivate
{
	/** A run waiting for its key that never comes within this many frames is a failure, not a hang. */
	constexpr int32 NoteTestMaxFrames = 600;

	void StartScenarioSteps(const TArray<FString>& InSteps, const FString& Name = FString())
	{
		// The panel starts again at each scenario (HOLScenario.cpp), with the scenario's name.
		HOLTestHooks::GetScenarioDisplay() = HOLTestHooks::FScenarioDisplay();
		HOLTestHooks::GetScenarioDisplay().Name = Name;
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

// Rule 15 of « Qualité des essais » (the user, 2026-11-14): in a <name>_compare scenario, Enter = next option and Backspace = previous
// option. An option ends at its waitkey and starts right after the waitkey before it (D828; the first option at its first notereset):
// Backspace at option B's waitkey runs option A again, from its notereset to its own waitkey; at the first option there is none before (the key does nothing).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLScenarioBackKeyTest, "HeroesOfLiberty.Dev.Scenario.BackKeyPreviousOption",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLScenarioBackKeyTest::RunTest(const FString& Parameters)
{
	HOLScenarioNoteTestsPrivate::StartScenarioSteps({ TEXT("note setup"), TEXT("notereset A"), TEXT("note A2"), TEXT("waitkey key A"),
		TEXT("notereset B"), TEXT("waitkey key B"), TEXT("notereset C"), TEXT("waitkey key C") }, TEXT("back_compare"));
	const TSharedRef<int32> Frames = MakeShared<int32>(0);
	const TSharedRef<int32> Phase = MakeShared<int32>(0);
	const TSharedRef<bool> bBackAtB = MakeShared<bool>(false);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Frames, Phase, bBackAtB]()
	{
		HOLTestHooks::FScenarioDisplay& Display = HOLTestHooks::GetScenarioDisplay();
		if (++*Frames >= HOLScenarioNoteTestsPrivate::NoteTestMaxFrames)
		{
			AddError(FString::Printf(TEXT("phase %d never came: waiting for \"%s\", notes %s"), *Phase, *Display.WaitingForKey,
				*FString::Join(Display.Notes, TEXT(", "))));
			HOLTestHooks::PressContinueKey();
			return true;
		}
		if (*Phase == 0)
		{
			// Option A waits: Enter, the next option.
			if (Display.WaitingForKey == TEXT("key A"))
			{
				HOLTestHooks::PressContinueKey();
			}
			else if (Display.WaitingForKey.StartsWith(TEXT("key B")))
			{
				TestEqual(TEXT("option B shows its note"), Display.Notes, TArray<FString>{ TEXT("B") });
				*bBackAtB = HOLTestHooks::PressBackKey();
				*Phase = 1;
			}
			return false;
		}
		if (Display.WaitingForKey != TEXT("key A"))
		{
			return false;
		}
		TestTrue(TEXT("Backspace at option B's waitkey was taken"), *bBackAtB);
		TestEqual(TEXT("option A ran again (its notereset and its note)"), Display.Notes, TArray<FString>{ TEXT("A"), TEXT("A2") });
		TestFalse(TEXT("Backspace at the first option does nothing (no option before it)"), HOLTestHooks::PressBackKey());
		HOLTestHooks::PressContinueKey();
		return true;
	}));
	return true;
}

// D828 (the user, 2026-11-16, Q-waitkey-texte Q2): an option starts right after the waitkey before it, so Backspace runs again the command
// that sets the option even when it comes before the option's notereset (rocks_compare: hol.Rocks.Review C, then notereset STYLE C). Here
// that command is a signal (a file written): Backspace at option C's waitkey writes option B's file again. D827 (Q1): the panel's waitkey
// text says « · Backspace : option précédente » when an option before exists, not at the first option.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLScenarioBackKeyReplaysSettingTest, "HeroesOfLiberty.Dev.Scenario.BackKeyReplaysOptionSetting",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLScenarioBackKeyReplaysSettingTest::RunTest(const FString& Parameters)
{
	const FString SignalB = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("ScenarioBackKeyOptionB.signal"));
	IFileManager::Get().Delete(*SignalB);
	HOLScenarioNoteTestsPrivate::StartScenarioSteps({ TEXT("note setup"), TEXT("notereset A"), TEXT("waitkey key A"),
		FString(TEXT("signal ")) + SignalB, TEXT("notereset B"), TEXT("waitkey key B"), TEXT("notereset C"), TEXT("waitkey key C") },
		TEXT("setting_compare"));
	const FString BackText = TEXT("· Backspace : option précédente");
	const TSharedRef<int32> Frames = MakeShared<int32>(0);
	const TSharedRef<int32> Phase = MakeShared<int32>(0);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Frames, Phase, SignalB, BackText]()
	{
		HOLTestHooks::FScenarioDisplay& Display = HOLTestHooks::GetScenarioDisplay();
		if (++*Frames >= HOLScenarioNoteTestsPrivate::NoteTestMaxFrames)
		{
			AddError(FString::Printf(TEXT("phase %d never came: waiting for \"%s\""), *Phase, *Display.WaitingForKey));
			HOLTestHooks::PressContinueKey();
			return true;
		}
		if (*Phase == 0 && Display.WaitingForKey.StartsWith(TEXT("key A")))
		{
			TestEqual(TEXT("D827: the first option's waitkey text does not offer Backspace"), Display.WaitingForKey, FString(TEXT("key A")));
			HOLTestHooks::PressContinueKey();
			*Phase = 1;
		}
		else if (*Phase == 1 && Display.WaitingForKey.StartsWith(TEXT("key B")))
		{
			TestTrue(TEXT("option B ran its setting command (the signal file)"), IFileManager::Get().FileExists(*SignalB));
			IFileManager::Get().Delete(*SignalB);
			HOLTestHooks::PressContinueKey();
			*Phase = 2;
		}
		else if (*Phase == 2 && Display.WaitingForKey.StartsWith(TEXT("key C")))
		{
			TestEqual(TEXT("D827: option C's waitkey text offers Backspace"), Display.WaitingForKey, FString(TEXT("key C ")) + BackText);
			TestTrue(TEXT("Backspace at option C's waitkey was taken"), HOLTestHooks::PressBackKey());
			*Phase = 3;
		}
		else if (*Phase == 3 && Display.WaitingForKey.StartsWith(TEXT("key B")))
		{
			TestTrue(TEXT("D828: Backspace ran option B's setting command again (before its notereset)"), IFileManager::Get().FileExists(*SignalB));
			TestEqual(TEXT("D827: option B's waitkey text offers Backspace"), Display.WaitingForKey, FString(TEXT("key B ")) + BackText);
			IFileManager::Get().Delete(*SignalB);
			HOLTestHooks::PressContinueKey();
			*Phase = 4;
		}
		else if (*Phase == 4 && Display.WaitingForKey.StartsWith(TEXT("key C")))
		{
			HOLTestHooks::PressContinueKey();
			return true;
		}
		return false;
	}));
	return true;
}

// Backspace only in a compare scenario: elsewhere its steps are not options, nothing runs again.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLScenarioBackKeyNotCompareTest, "HeroesOfLiberty.Dev.Scenario.BackKeyOnlyInCompare",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLScenarioBackKeyNotCompareTest::RunTest(const FString& Parameters)
{
	HOLScenarioNoteTestsPrivate::StartScenarioSteps({ TEXT("notereset A"), TEXT("waitkey key A"), TEXT("notereset B"), TEXT("waitkey key B") },
		TEXT("plain"));
	const TSharedRef<int32> Frames = MakeShared<int32>(0);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Frames]()
	{
		HOLTestHooks::FScenarioDisplay& Display = HOLTestHooks::GetScenarioDisplay();
		if (Display.WaitingForKey == TEXT("key A"))
		{
			HOLTestHooks::PressContinueKey();
		}
		if (Display.WaitingForKey != TEXT("key B") && ++*Frames < HOLScenarioNoteTestsPrivate::NoteTestMaxFrames)
		{
			return false;
		}
		TestEqual(TEXT("the run reached its second waitkey"), Display.WaitingForKey, FString(TEXT("key B")));
		TestFalse(TEXT("Backspace outside a compare scenario does nothing"), HOLTestHooks::PressBackKey());
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
