// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dev/HOLTestHooks.h"

#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Misc/FileHelper.h"
#include "Misc/OutputDevice.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Misc/Paths.h"
#include "Net/HOLUnitReceptionSubsystem.h"
#include "Player/HOLPlayerController.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "UnrealClient.h"

/**
 * Runs a file of steps one after the other, each as soon as the one before is done, for the scripted checks
 * (Tools/Tests/quick_check.ps1): no fixed delays, a step waits for what it needs and the run stops at the first one
 * that never comes.
 *   hol.Test.Run <File>
 * One step per line (empty lines and lines starting with # are skipped):
 *   frames <N>           lets N frames go by (layout of a screen just opened)
 *   wait <Seconds>       lets time go by (a key held, a camera moving)
 *   waitlog <Text>       waits for a log line holding Text, written since the last command
 *   waitmap <Name>       waits for a game world whose map name holds Name to have begun play
 *   waitplayers <N>      waits for the game world to know N players (a client waits for the others to join)
 *   waitunits <N>        waits for N units at least: simulated (solo, server) or displayed (client, D271)
 *   waitidle <N>         waits for the local player to have N idle workers at least, as the HUD counts them (J5b, D307)
 *   waitui               waits for the interface to be still: a screen just opened or closed has finished fading
 *   capture <Name>       screenshot with the interface to Saved/Screenshots/HOL/<Name>.png, waits for the file;
 *                        logs "Test capture: <Name> (menu=X modal=Y)", the screens shown, for the script to check
 *   mark <Text>          writes "Test mark: Text" to the log (sections for the script that reads it)
 *   anything else        runs as a console command, in the current game world
 * A wait that lasts more than 60 seconds ends the run with an error and quits.
 */
TFunction<bool()>& HOLTestHooks::IsInterfaceBusy()
{
	static TFunction<bool()> Probe;
	return Probe;
}

TFunction<FString()>& HOLTestHooks::DescribeInterface()
{
	static TFunction<FString()> Probe;
	return Probe;
}

namespace UE::HeroesOfLiberty::Dev
{
	constexpr double StepTimeoutSeconds = 60.0;
	/** Frames before waitui looks: a screen opened by a command is pushed on the next frame, its fade starts after. */
	constexpr int32 WaitUIMinFrames = 3;

	/** Keeps the log lines written during a run, from any thread. */
	class FTestLogWatcher : public FOutputDevice
	{
	public:
		virtual void Serialize(const TCHAR* Text, ELogVerbosity::Type Verbosity, const FName& Category) override
		{
			FScopeLock Lock(&Guard);
			Lines.Emplace(Text);
		}

		virtual bool CanBeUsedOnAnyThread() const override { return true; }
		virtual bool CanBeUsedOnMultipleThreads() const override { return true; }

		int32 Num() const
		{
			FScopeLock Lock(&Guard);
			return Lines.Num();
		}

		bool Contains(const FString& Text, const int32 From) const
		{
			FScopeLock Lock(&Guard);
			for (int32 Index = From; Index < Lines.Num(); ++Index)
			{
				if (Lines[Index].Contains(Text))
				{
					return true;
				}
			}
			return false;
		}

	private:
		mutable FCriticalSection Guard;
		TArray<FString> Lines;
	};

	class FTestSequence
	{
	public:
		void Start(TArray<FString>&& InSteps)
		{
			Steps = MoveTemp(InSteps);
			Next = 0;
			Watcher = MakeUnique<FTestLogWatcher>();
			GLog->AddOutputDevice(Watcher.Get());
			RunStart = FPlatformTime::Seconds();
			BeginWait();
			UE_LOG(LogHOLTest, Display, TEXT("Test sequence: %d steps"), Steps.Num());
			Ticker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FTestSequence::Tick));
		}

	private:
		static UWorld* GetGameWorld()
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

		void BeginWait()
		{
			WaitStart = FPlatformTime::Seconds();
			FramesWaited = 0;
		}

		/** True when the current step is done; ends the run on a timeout. */
		bool IsStepDone(const FString& Step)
		{
			FString Verb;
			FString Rest;
			if (!Step.Split(TEXT(" "), &Verb, &Rest))
			{
				Verb = Step;
			}
			const double Waited = FPlatformTime::Seconds() - WaitStart;
			bool bDone = true;
			if (Verb == TEXT("frames"))
			{
				bDone = ++FramesWaited > FCString::Atoi(*Rest);
			}
			else if (Verb == TEXT("wait"))
			{
				bDone = Waited >= FCString::Atod(*Rest);
			}
			else if (Verb == TEXT("waitlog"))
			{
				bDone = Watcher->Contains(Rest, LogMark);
			}
			else if (Verb == TEXT("waitmap"))
			{
				const UWorld* World = GetGameWorld();
				bDone = World && World->HasBegunPlay() && World->GetMapName().Contains(Rest);
			}
			else if (Verb == TEXT("waitplayers"))
			{
				const UWorld* World = GetGameWorld();
				const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
				bDone = GameState && GameState->PlayerArray.Num() >= FCString::Atoi(*Rest);
			}
			else if (Verb == TEXT("waitunits"))
			{
				const UWorld* World = GetGameWorld();
				const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
				const UHOLUnitReceptionSubsystem* Reception = World ? World->GetSubsystem<UHOLUnitReceptionSubsystem>() : nullptr;
				const int32 NumUnits = World && World->GetNetMode() == NM_Client
					? (Reception ? Reception->GetNumDisplayed() : 0) : (Units ? Units->GetNumUnits() : 0);
				bDone = NumUnits >= FCString::Atoi(*Rest);
			}
			else if (Verb == TEXT("waitidle"))
			{
				const UWorld* World = GetGameWorld();
				const AHOLPlayerController* Controller = World ? Cast<AHOLPlayerController>(World->GetFirstPlayerController()) : nullptr;
				bDone = Controller && Controller->GetIdleWorkers().Num() >= FCString::Atoi(*Rest);
			}
			else if (Verb == TEXT("waitui"))
			{
				const TFunction<bool()>& IsBusy = HOLTestHooks::IsInterfaceBusy();
				bDone = ++FramesWaited > WaitUIMinFrames && !(IsBusy && IsBusy());
			}
			else if (Verb == TEXT("capture"))
			{
				const FString Path = FPaths::Combine(FPaths::ScreenShotDir(), TEXT("HOL"), Rest + TEXT(".png"));
				if (FramesWaited++ == 0)
				{
					IFileManager::Get().Delete(*Path, /*RequireExists*/ false, /*EvenReadOnly*/ true, /*Quiet*/ true);
					FScreenshotRequest::RequestScreenshot(Path, /*bShowUI*/ true, /*bAddFilenameSuffix*/ false);
					const TFunction<FString()>& Describe = HOLTestHooks::DescribeInterface();
					UE_LOG(LogHOLTest, Display, TEXT("Test capture: %s (%s)"), *Rest, Describe ? *Describe() : TEXT("no interface"));
					bDone = false;
				}
				else
				{
					bDone = IFileManager::Get().FileSize(*Path) > 0;
				}
			}
			else if (Verb == TEXT("mark"))
			{
				UE_LOG(LogHOLTest, Display, TEXT("Test mark: %s"), *Rest);
			}
			else
			{
				LogMark = Watcher->Num();
				GEngine->Exec(GetGameWorld(), *Step);
			}

			if (!bDone && Waited > StepTimeoutSeconds)
			{
				UE_LOG(LogHOLTest, Error, TEXT("Test sequence: timeout at step %d: %s"), Next + 1, *Step);
				Finish(/*bQuit*/ true);
			}
			return bDone;
		}

		bool Tick(float)
		{
			// One step per frame at most: a command's effect shows on the next frame.
			if (Steps.IsValidIndex(Next) && IsStepDone(Steps[Next]))
			{
				++Next;
				BeginWait();
			}
			if (!Steps.IsValidIndex(Next) && Ticker.IsValid())
			{
				UE_LOG(LogHOLTest, Display, TEXT("Test sequence: done in %.1f s"), FPlatformTime::Seconds() - RunStart);
				Finish(/*bQuit*/ false);
				return false;
			}
			return Ticker.IsValid();
		}

		void Finish(const bool bQuit)
		{
			GLog->RemoveOutputDevice(Watcher.Get());
			FTSTicker::RemoveTicker(Ticker);
			Ticker.Reset();
			Next = Steps.Num();
			if (bQuit)
			{
				GEngine->Exec(GetGameWorld(), TEXT("quit"));
			}
		}

		TArray<FString> Steps;
		int32 Next = 0;
		TUniquePtr<FTestLogWatcher> Watcher;
		/** Log lines from here on are searched by waitlog: those written since the last command. */
		int32 LogMark = 0;
		double RunStart = 0.0;
		double WaitStart = 0.0;
		int32 FramesWaited = 0;
		FTSTicker::FDelegateHandle Ticker;
	};

	FTestSequence Sequence;

	void RunSequence(const TArray<FString>& Args)
	{
		TArray<FString> Lines;
		if (Args.IsEmpty() || !FFileHelper::LoadFileToStringArray(Lines, *Args[0]))
		{
			UE_LOG(LogHOLTest, Error, TEXT("Usage: hol.Test.Run <File> (file %s not read)"), Args.IsEmpty() ? TEXT("missing") : *Args[0]);
			return;
		}
		TArray<FString> Steps;
		for (FString& Line : Lines)
		{
			Line.TrimStartAndEndInline();
			if (!Line.IsEmpty() && !Line.StartsWith(TEXT("#")))
			{
				Steps.Add(MoveTemp(Line));
			}
		}
		Sequence.Start(MoveTemp(Steps));
	}

	FAutoConsoleCommand RunCommand(
		TEXT("hol.Test.Run"),
		TEXT("hol.Test.Run <File>: runs a file of steps one after the other, each as soon as the one before is done (scripted checks)"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&RunSequence));
}
