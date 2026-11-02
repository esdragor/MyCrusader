// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dev/HOLTestSequence.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// The capture sequence of a scenario (Tools/Scenarios, rule 12a of Auto_Journey.md): `captureseq <Name> <Count> <Step> [Command]` is written
// out as steps on the game's clock, so that a moving thing (a shrinking circle) is shown by several pictures, not by one.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCaptureSeqExpandTest, "HeroesOfLiberty.Dev.CaptureSeq.Expand",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLCaptureSeqExpandTest::RunTest(const FString& Parameters)
{
	TArray<FString> Steps{ TEXT("frames 3"), TEXT("captureseq ring 4 2"), TEXT("note done") };
	HOLTestSequence::ExpandSequences(Steps);
	const TArray<FString> Wanted{ TEXT("frames 3"), TEXT("seqclock"),
		TEXT("waitclock 0"), TEXT("capture ring_0"), TEXT("waitclock 2"), TEXT("capture ring_1"),
		TEXT("waitclock 4"), TEXT("capture ring_2"), TEXT("waitclock 6"), TEXT("capture ring_3"), TEXT("note done") };
	TestEqual(TEXT("steps after the expansion"), Steps, Wanted);

	// A command given is run before each picture (a report of what is shown, read at the same game time).
	TArray<FString> WithCommand{ TEXT("captureseq a 2 0.25 hol.HUD.Report 1") };
	HOLTestSequence::ExpandSequences(WithCommand);
	const TArray<FString> WantedWithCommand{ TEXT("seqclock"), TEXT("waitclock 0"), TEXT("hol.HUD.Report 1"), TEXT("capture a_0"),
		TEXT("waitclock 0.25"), TEXT("hol.HUD.Report 1"), TEXT("capture a_1") };
	TestEqual(TEXT("steps with a command"), WithCommand, WantedWithCommand);

	// Bad arguments: a mark that says so, never a silent run.
	for (const TCHAR* Bad : { TEXT("captureseq"), TEXT("captureseq x"), TEXT("captureseq x 0 1"), TEXT("captureseq x 3 -1"), TEXT("captureseq x many 1") })
	{
		TArray<FString> BadSteps{ Bad };
		HOLTestSequence::ExpandSequences(BadSteps);
		if (TestEqual(FString::Printf(TEXT("'%s' gives one step"), Bad), BadSteps.Num(), 1))
		{
			TestTrue(FString::Printf(TEXT("'%s' gives a mark"), Bad), BadSteps[0].StartsWith(TEXT("mark captureseq: bad arguments")));
		}
	}
	return true;
}

#endif
