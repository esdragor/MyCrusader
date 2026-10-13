// Copyright Epic Games, Inc. All Rights Reserved.

#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"

/**
 * Runs a console command later, for the scripted checks (D246, D257): -ExecCmds runs everything at start-up, this
 * lets a command (a console variable, a report) happen once the match is running.
 *   hol.Delay <DelaySeconds> <Command...>
 */
namespace UE::HeroesOfLiberty::Dev
{
	void Delay(const TArray<FString>& Args)
	{
		if (Args.Num() < 2)
		{
			UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Delay <DelaySeconds> <Command...>"));
			return;
		}
		const float Seconds = FCString::Atof(*Args[0]);
		const FString Command = FString::Join(TArrayView<const FString>(Args).RightChop(1), TEXT(" "));
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Command](float)
		{
			UWorld* GameWorld = nullptr;
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (Context.World() && Context.World()->IsGameWorld())
				{
					GameWorld = Context.World();
				}
			}
			UE_LOG(LogHOLMatch, Display, TEXT("Delayed command: %s"), *Command);
			GEngine->Exec(GameWorld, *Command);
			return false;
		}), Seconds);
	}

	FAutoConsoleCommand DelayCommand(
		TEXT("hol.Delay"),
		TEXT("hol.Delay <DelaySeconds> <Command...>: runs a console command after a delay (scripted checks)"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&Delay));
}
