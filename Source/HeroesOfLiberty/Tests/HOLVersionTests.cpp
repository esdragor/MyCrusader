// Copyright Epic Games, Inc. All Rights Reserved.

#include "HOLVersion.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// Version of the game (R1.7): "dev" outside a package, the first line of Version.txt in one.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVersionTest, "HeroesOfLiberty.Version",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLVersionTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("dev outside a package"), HOLVersion::Get(), FString(HOLVersion::Dev));
	TestEqual(TEXT("first line, trimmed"), HOLVersion::Parse(TEXT(" 2026-10-23-f7269a8\r\nmore\n")), FString(TEXT("2026-10-23-f7269a8")));
	TestEqual(TEXT("no newline"), HOLVersion::Parse(TEXT("2026-10-23-f7269a8")), FString(TEXT("2026-10-23-f7269a8")));
	TestEqual(TEXT("empty file"), HOLVersion::Parse(TEXT("\r\n")), FString(HOLVersion::Unknown));
	return true;
}

// Version check at login (R3.3): the server lets in a client of its own version only.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVersionCheckTest, "HeroesOfLiberty.VersionCheck",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLVersionCheckTest::RunTest(const FString& Parameters)
{
	const FString Packaged = TEXT("2026-10-23-b4a8c8c");
	const FString Other = TEXT("2026-10-24-0123abc");
	FString Error;
	TestTrue(TEXT("same packaged version"), HOLVersion::IsAccepted(Packaged, Packaged));
	TestTrue(TEXT("dev with dev"), HOLVersion::IsAccepted(HOLVersion::Dev, HOLVersion::Dev));
	TestTrue(TEXT("dev server, no version (editor tests)"), HOLVersion::IsAccepted(HOLVersion::Dev, FString()));

	TestFalse(TEXT("other packaged version"), HOLVersion::IsAccepted(Packaged, Other, &Error));
	TestEqual(TEXT("mismatch message"), Error, FString::Printf(TEXT("Version mismatch: server %s, client %s"), *Packaged, *Other));
	TestFalse(TEXT("dev client, packaged server"), HOLVersion::IsAccepted(Packaged, HOLVersion::Dev));
	TestFalse(TEXT("packaged client, dev server"), HOLVersion::IsAccepted(HOLVersion::Dev, Packaged));
	TestFalse(TEXT("no version, packaged server"), HOLVersion::IsAccepted(Packaged, FString(), &Error));
	TestEqual(TEXT("missing message"), Error, FString::Printf(TEXT("Version mismatch: server %s, client sent no version"), *Packaged));
	TestFalse(TEXT("unknown server refuses unknown client"), HOLVersion::IsAccepted(HOLVersion::Unknown, HOLVersion::Unknown));
	return true;
}

#endif
