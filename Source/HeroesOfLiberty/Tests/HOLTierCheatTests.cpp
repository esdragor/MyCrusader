// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Siege/HOLSiegeRules.h"

#if WITH_DEV_AUTOMATION_TESTS

// G0 (Palier3InProgress.md, D710): hol.Cheat.Tier forces a slot's tier whatever its hero's level. The effect is read through the rule that
// matters, the check of the siege engines (HOLSiege::Check, tier 3), fed by AHOLGameState::GetTier as production, grid and AI do.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLTierCheatTest, "HeroesOfLiberty.Match.TierCheat",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLTierCheatTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	if (TestNotNull(TEXT("game state"), State))
	{
		using HOLSiege::EEngine;
		using HOLSiege::EBlock;
		TestEqual(TEXT("without the command the tier is the hero's level's (no hero: 0)"), State->GetTier(0), 0);
		TestTrue(TEXT("a Ram (tier 1) is refused at tier 0"), HOLSiege::Check(EEngine::Ram, State->GetTier(0), true, true) == EBlock::TierTooLow);

		State->SetTierOverride(0, 3);
		TestEqual(TEXT("the forced tier is read by GetTier though the hero is at level 1"), State->GetTier(0), 3);
		TestTrue(TEXT("a Ram is no longer refused for its tier"), HOLSiege::Check(EEngine::Ram, State->GetTier(0), true, true) == EBlock::None);
		TestEqual(TEXT("the other slots are untouched"), State->GetTier(1), 0);
		TestEqual(TEXT("and report no override"), State->GetTierOverride(1), -1);

		State->SetTierOverride(0, 9);
		TestEqual(TEXT("a tier above 3 is held to 3"), State->GetTier(0), 3);
		State->SetTierOverride(0, 1);
		TestEqual(TEXT("a lower forced tier is read as well (0 to 3 both ways)"), State->GetTier(0), 1);
		State->SetTierOverride(0, 0);
		TestEqual(TEXT("tier 0 forced is an override, not 'none'"), State->GetTierOverride(0), 0);
		State->SetTierOverride(0, -1);
		TestEqual(TEXT("off: back to the hero's level's tier"), State->GetTier(0), 0);
		State->SetTierOverride(99, 3);
		TestEqual(TEXT("a slot out of the table changes nothing"), State->GetTier(99), 0);

		const FProperty* Property = AHOLGameState::StaticClass()->FindPropertyByName(TEXT("TierOverrides"));
		TestTrue(TEXT("the override is a replicated property: a client reads the same tier"), Property && Property->HasAnyPropertyFlags(CPF_Net));
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
