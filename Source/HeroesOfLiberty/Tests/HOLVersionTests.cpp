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

#endif
