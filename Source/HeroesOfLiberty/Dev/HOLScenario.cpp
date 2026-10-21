// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dev/HOLTestHooks.h"
#include "Dev/HOLTestSequence.h"

#if !UE_BUILD_SHIPPING

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLMatchSubsystem.h"
#include "Misc/Paths.h"

/**
 * Opens the game on a ready-made situation for a trial (soldiers, enemies, buildings, resources), instead of setting
 * it up by hand each time (Tools/Scenarios/README.md).
 *   hol.Scenario             logs the scenarios of Tools/Scenarios (<Name>.txt)
 *   hol.Scenario <Name>      starts a match with Tools/Scenarios/<Name>.json as its configuration (the default one if
 *                            there is none), from the main menu as its New game screen does or by reloading the map from
 *                            inside a match; once the local player's camera is above its start, runs the steps of
 *                            <Name>.txt (the verbs of hol.Test.Run, plus note); then logs "Scenario <Name>: ready".
 * A step that never comes shows an error on the screen and stops the scenario; the game stays open.
 */
namespace UE::HeroesOfLiberty::Dev
{
	FString GetScenarioDir()
	{
		return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("Tools"), TEXT("Scenarios")));
	}

	UWorld* GetScenarioWorld()
	{
		UWorld* GameWorld = nullptr;
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->IsGameWorld())
			{
				GameWorld = Context.World();
			}
		}
		return GameWorld;
	}

	void ListScenarios()
	{
		TArray<FString> Files;
		IFileManager::Get().FindFiles(Files, *FPaths::Combine(GetScenarioDir(), TEXT("*.txt")), /*Files*/ true, /*Directories*/ false);
		Files.Sort();
		UE_LOG(LogHOLTest, Display, TEXT("Scenarios in %s: %d"), *GetScenarioDir(), Files.Num());
		for (const FString& File : Files)
		{
			UE_LOG(LogHOLTest, Display, TEXT("  hol.Scenario %s"), *FPaths::GetBaseFilename(File));
		}
	}

	void RunScenario(const TArray<FString>& Args)
	{
		if (Args.IsEmpty())
		{
			ListScenarios();
			return;
		}
		const FString Name = Args[0];
		const FString StepsFile = FPaths::Combine(GetScenarioDir(), Name + TEXT(".txt"));
		TArray<FString> ScenarioSteps;
		if (!HOLTestSequence::LoadSteps(StepsFile, ScenarioSteps))
		{
			UE_LOG(LogHOLTest, Error, TEXT("hol.Scenario: no scenario %s (%s not read)"), *Name, *StepsFile);
			ListScenarios();
			return;
		}

		UWorld* World = GetScenarioWorld();
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		UHOLMatchSubsystem* Match = GameInstance ? GameInstance->GetSubsystem<UHOLMatchSubsystem>() : nullptr;
		if (!Match)
		{
			UE_LOG(LogHOLTest, Error, TEXT("hol.Scenario %s: no game world or no match subsystem"), *Name);
			return;
		}
		const FString ConfigFile = FPaths::Combine(GetScenarioDir(), Name + TEXT(".json"));
		const bool bHasConfigFile = IFileManager::Get().FileExists(*ConfigFile);
		if (bHasConfigFile)
		{
			if (!Match->LoadFromFile(ConfigFile))
			{
				return;
			}
		}
		else
		{
			Match->SetConfig(FHOLMatchConfig::MakeDefault());
		}
		FHOLMatchConfig Config = Match->GetConfigOrDefault();
		Config.Normalize();
		// Without a .json the default configuration names BattleMap, which the New game screen does not offer: Plain,
		// as for a .json without a map.
		if (!bHasConfigFile || Config.Map.IsNull())
		{
			Config.Map = FSoftObjectPath(TEXT("/Game/Maps/Plain.Plain"));
		}
		Config.Seed = static_cast<int32>(FDateTime::UtcNow().GetTicks() & MAX_int32);
		Match->SetConfig(Config);

		// From the main menu, the New game screen launches it (the match then starts as from its button: game screen,
		// input in game mode); from inside a match, the map is opened again with the configuration set.
		TArray<FString> Steps;
		if (World->GetMapName().Contains(TEXT("MainMenu")))
		{
			Steps = { TEXT("waitmap MainMenu"), TEXT("waitui"), TEXT("hol.UI.NewGame.Launch - 0") };
		}
		else
		{
			Steps = { FString::Printf(TEXT("open %s"), *Config.Map.GetLongPackageName()) };
		}
		Steps.Add(FString::Printf(TEXT("waitmap %s"), *Config.Map.GetAssetName()));
		Steps.Add(TEXT("waitlog Camera above the start of slot"));
		Steps.Add(TEXT("frames 5"));
		Steps.Append(MoveTemp(ScenarioSteps));
		UE_LOG(LogHOLTest, Display, TEXT("Scenario %s: %s, %d occupied slots"), *Name, *Config.Map.ToString(), Config.GetNumOccupiedSlots());
		// The panel starts again: a reload's notes replace those of the run before.
		HOLTestHooks::FScenarioDisplay& Display = HOLTestHooks::GetScenarioDisplay();
		Display = HOLTestHooks::FScenarioDisplay();
		Display.Name = Name;
		HOLTestHooks::NotifyScenarioChanged(FString(), FColor::White);

		HOLTestSequence::FRunOptions Options;
		Options.bQuitOnTimeout = false;
		Options.OnDone = [Name]()
		{
			UE_LOG(LogHOLTest, Display, TEXT("Scenario %s: ready"), *Name);
			HOLTestHooks::GetScenarioDisplay().State = HOLTestHooks::FScenarioDisplay::EState::Ready;
			HOLTestHooks::NotifyScenarioChanged(FString::Printf(TEXT("Scénario %s prêt"), *Name), FColor::Green);
		};
		HOLTestSequence::Run(TEXT("Scenario"), MoveTemp(Steps), MoveTemp(Options));
	}

	FAutoConsoleCommand ScenarioCommand(
		TEXT("hol.Scenario"),
		TEXT("hol.Scenario [Name]: starts a match on the test scenario Tools/Scenarios/<Name>.txt (+ .json, its match configuration); no name: lists them (development only)"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&RunScenario));
}

#endif
