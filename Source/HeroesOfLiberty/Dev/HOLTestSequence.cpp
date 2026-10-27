// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dev/HOLTestSequence.h"

#include "Dev/HOLTestHooks.h"

#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Misc/App.h"
#include "Misc/CoreDelegates.h"
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
 *   note <Text>          adds Text to the scenario's panel (HOLTestHooks::GetScenarioDisplay) and logs
 *                        "Scenario note: Text" (what a scenario asks to try)
 *   waitkey [Text]       waits for the user to press the continue key (Enter, in game), Text (else « Entrée : étape
 *                        suivante ») shown on the panel in its own colour meanwhile; no timeout. Logs "Scenario waits
 *                        for the key: Text", then "Scenario key: go on (key|auto)". Goes on by itself after
 *                        AutoContinueSeconds in a scripted check (hol.Test.Run), with -unattended, under automation or
 *                        with hol.Scenario.AutoContinue 1 (dry runs of a scenario)
 *   signal <File>        writes the file (absolute path): another game on the machine waiting for it goes on
 *   waitfile <File> [S]  waits for the file to exist: two clients wait for each other (each one's last report done
 *                        before either quits, quick_check's net part); S: its own timeout in seconds, instead of 60
 *                        (the dedicated server waits for a client's step of the end, J7b.4)
 *   anything else        runs as a console command, in the current game world
 * A wait that lasts more than 60 seconds ends the run with an error and quits (a scenario's run: the error is shown on
 * the screen too and the game stays open, HOLTestSequence.h).
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

HOLTestHooks::FScenarioDisplay& HOLTestHooks::GetScenarioDisplay()
{
	static FScenarioDisplay Display;
	return Display;
}

TFunction<void()>& HOLTestHooks::OnScenarioChanged()
{
	static TFunction<void()> Hook;
	return Hook;
}

void HOLTestHooks::NotifyScenarioChanged(const FString& Fallback, const FColor& FallbackColor)
{
	// No interface (dedicated server): the old debug line, for whoever looks at that screen.
	constexpr float FallbackSeconds = 30.f;
	if (const TFunction<void()>& Hook = OnScenarioChanged())
	{
		Hook();
	}
	else if (GEngine && !Fallback.IsEmpty())
	{
		GEngine->AddOnScreenDebugMessage(-1, FallbackSeconds, FallbackColor, Fallback);
	}
}

namespace UE::HeroesOfLiberty::Dev
{
	constexpr double StepTimeoutSeconds = 60.0;
	/** Frames before waitui looks: a screen opened by a command is pushed on the next frame, its fade starts after. */
	constexpr int32 WaitUIMinFrames = 3;
	/** A waitkey that nobody will answer (scripted check, unattended, auto-continue) goes on after this. */
	constexpr double AutoContinueSeconds = 1.0;
	const TCHAR* const DefaultWaitKeyText = TEXT("Entrée : étape suivante");
	/**
	 * On a fixed time step (quick_check's -UseFixedTimeStep -FPS=n, Docs/Archive/TestSpeedInProgress.md), a frame's game time while the
	 * sequence gives commands, takes captures or counts frames: as short as a frame in real time off screen (~200 a
	 * second), so that the commands follow each other as they do in real time; the -FPS step only while it waits for the game.
	 */
	constexpr double CommandFrameSeconds = 0.005;

	/** The steps that wait for the game to move on (a walk, a fight, a death): the frames go by at the -FPS step. */
	bool IsGameWait(const FString& Step)
	{
		FString Verb = Step;
		Step.Split(TEXT(" "), &Verb, nullptr);
		return Verb == TEXT("wait") || Verb == TEXT("waitlog") || Verb == TEXT("waitunits") || Verb == TEXT("waitidle")
			|| Verb == TEXT("waitmap") || Verb == TEXT("waitplayers");
	}

	TAutoConsoleVariable<int32> CVarTestSkipWaitRendering(
		TEXT("hol.Test.SkipWaitRendering"),
		0,
		TEXT("1: on a fixed time step, the world is not drawn while the sequence waits for the game (quick_check off screen, "
			 "Docs/Archive/TestSpeedInProgress.md); it is for the commands, the frames and the captures"));
	/** Frames drawn before a capture once the world is drawn again: the temporal anti-aliasing's history filled. */
	constexpr int32 CaptureDrawnFrames = 4;

	TAutoConsoleVariable<int32> CVarScenarioAutoContinue(
		TEXT("hol.Scenario.AutoContinue"),
		0,
		TEXT("1: a scenario's waitkey steps go on by themselves after 1 s, without the continue key (dry runs off-screen)"));

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
		~FTestSequence()
		{
			// Destroyed with Sequences, at the module's static destruction: a run still going was stopped before the
			// engine exits (StopSequences); past that point GLog and the core ticker may be gone, they are left alone.
			if (bRunning && !IsEngineExitRequested())
			{
				Finish(/*bQuit*/ false);
			}
		}

		/**
		 * Stops a run still going, its last step not done (the game closed while a scenario was being set up). Tested on
		 * bRunning, not Ticker: the engine empties the core ticker before OnEnginePreExit, the handle has expired by then.
		 */
		void Stop()
		{
			if (bRunning)
			{
				UE_LOG(LogHOLTest, Display, TEXT("Test sequence: the game exits, the run is stopped at step %d"), Next + 1);
				Finish(/*bQuit*/ false);
			}
		}

		void Start(TArray<FString>&& InSteps, HOLTestSequence::FRunOptions&& InOptions = {})
		{
			if (Ticker.IsValid())
			{
				UE_LOG(LogHOLTest, Warning, TEXT("Test sequence: the run still going is stopped at step %d"), Next + 1);
				Finish(/*bQuit*/ false);
			}
			Options = MoveTemp(InOptions);
			Steps = MoveTemp(InSteps);
			Next = 0;
			Watcher = MakeUnique<FTestLogWatcher>();
			GLog->AddOutputDevice(Watcher.Get());
			bRunning = true;
			RunStart = FPlatformTime::Seconds();
			RunStartGame = FApp::GetCurrentTime();
			RunStartFrame = GFrameCounter;
			WaitFrameSeconds = FApp::UseFixedTimeStep() ? FApp::GetFixedDeltaTime() : 0.0;
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
			WaitStartGame = FApp::GetCurrentTime();
			FramesWaited = 0;
			StepTimeout = StepTimeoutSeconds;
			// The next frame's game time (UEngine::UpdateTimeAndHandleMaxTickRate reads it at the head of the frame): short,
			// a wait already met (a log line written by the command before it) costs no game time.
			if (WaitFrameSeconds > 0.0)
			{
				FApp::SetFixedDeltaTime(CommandFrameSeconds);
			}
			SetWorldDrawn(true);
		}

		/** The world drawn or not (hol.Test.SkipWaitRendering); FramesDrawn counts the frames since it is drawn again. */
		void SetWorldDrawn(const bool bDrawn)
		{
			if (bWorldDrawn == bDrawn)
			{
				return;
			}
			bWorldDrawn = bDrawn;
			FramesDrawn = 0;
			if (UGameViewportClient* Viewport = GEngine ? GEngine->GameViewport : nullptr)
			{
				Viewport->bDisableWorldRendering = !bDrawn;
			}
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
				// The game's time: with a fixed time step (quick_check's -UseFixedTimeStep -FPS=n) it runs faster than the
				// clock, and what a wait lets happen (a walk, a fight) happens in the game's seconds.
				bDone = FApp::GetCurrentTime() - WaitStartGame >= FCString::Atod(*Rest);
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
			else if (Verb == TEXT("capture") && FramesDrawn < CaptureDrawnFrames)
			{
				// The world drawn again a few frames first (it was not while the sequence waited).
				bDone = false;
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
			else if (Verb == TEXT("note"))
			{
				UE_LOG(LogHOLTest, Display, TEXT("Scenario note: %s"), *Rest);
				HOLTestHooks::GetScenarioDisplay().Notes.Add(Rest);
				HOLTestHooks::NotifyScenarioChanged(Rest, FColor::Yellow);
			}
			else if (Verb == TEXT("waitkey"))
			{
				// As long as the user wants: no timeout.
				StepTimeout = TNumericLimits<double>::Max();
				HOLTestHooks::FScenarioDisplay& Display = HOLTestHooks::GetScenarioDisplay();
				if (FramesWaited++ == 0)
				{
					bKeyPressed = false;
					bWaitingForKey = true;
					Display.WaitingForKey = Rest.IsEmpty() ? FString(DefaultWaitKeyText) : Rest;
					UE_LOG(LogHOLTest, Display, TEXT("Scenario waits for the key: %s"), *Display.WaitingForKey);
					HOLTestHooks::NotifyScenarioChanged(Display.WaitingForKey, FColor::Cyan);
				}
				const bool bAuto = ShouldAutoContinue() && Waited >= AutoContinueSeconds;
				bDone = bKeyPressed || bAuto;
				if (bDone)
				{
					EndWaitForKey();
					UE_LOG(LogHOLTest, Display, TEXT("Scenario key: go on (%s)"), bKeyPressed ? TEXT("key") : TEXT("auto"));
				}
			}
			else if (Verb == TEXT("signal"))
			{
				FFileHelper::SaveStringToFile(TEXT("signal"), *Rest);
				UE_LOG(LogHOLTest, Display, TEXT("Test signal: %s"), *Rest);
			}
			else if (Verb == TEXT("waitfile"))
			{
				FString File = Rest;
				FString Timeout;
				if (Rest.Split(TEXT(" "), &File, &Timeout, ESearchCase::CaseSensitive, ESearchDir::FromEnd) && Timeout.IsNumeric())
				{
					StepTimeout = FCString::Atod(*Timeout);
				}
				else
				{
					File = Rest;
				}
				bDone = IFileManager::Get().FileExists(*File);
			}
			else
			{
				LogMark = Watcher->Num();
				GEngine->Exec(GetGameWorld(), *Step);
			}

			if (!bDone && Waited > StepTimeout)
			{
				UE_LOG(LogHOLTest, Error, TEXT("Test sequence: timeout at step %d: %s"), Next + 1, *Step);
				if (!Options.bQuitOnTimeout)
				{
					HOLTestHooks::FScenarioDisplay& Display = HOLTestHooks::GetScenarioDisplay();
					Display.State = HOLTestHooks::FScenarioDisplay::EState::Failed;
					Display.Error = FString::Printf(TEXT("étape %d jamais venue (60 s) : %s"), Next + 1, *Step);
					HOLTestHooks::NotifyScenarioChanged(FString::Printf(TEXT("Test sequence: timeout at step %d: %s"), Next + 1, *Step), FColor::Red);
				}
				Finish(/*bQuit*/ Options.bQuitOnTimeout);
			}
			return bDone;
		}

		bool Tick(float)
		{
			if (bWorldDrawn)
			{
				++FramesDrawn;
			}
			// One step per frame at most: a command's effect shows on the next frame.
			if (Steps.IsValidIndex(Next) && IsStepDone(Steps[Next]))
			{
				++Next;
				BeginWait();
			}
			else if (WaitFrameSeconds > 0.0 && Steps.IsValidIndex(Next) && IsGameWait(Steps[Next]))
			{
				// Waiting for the game: the next frames at the -FPS step, the world not drawn if asked.
				FApp::SetFixedDeltaTime(WaitFrameSeconds);
				if (CVarTestSkipWaitRendering.GetValueOnGameThread() > 0)
				{
					SetWorldDrawn(false);
				}
			}
			if (!Steps.IsValidIndex(Next) && Ticker.IsValid())
			{
				const double RealSeconds = FPlatformTime::Seconds() - RunStart;
				const double GameSeconds = FApp::GetCurrentTime() - RunStartGame;
				UE_LOG(LogHOLTest, Display, TEXT("Test sequence: done in %.1f s (game time %.1f s, x%.2f, fixed step %s, %llu frames)"), RealSeconds,
					GameSeconds, RealSeconds > 0.0 ? GameSeconds / RealSeconds : 0.0,
					WaitFrameSeconds > 0.0 ? *FString::Printf(TEXT("%.4f s"), WaitFrameSeconds) : TEXT("off"), GFrameCounter - RunStartFrame);
				Finish(/*bQuit*/ false);
				if (const TFunction<void()> OnDone = MoveTemp(Options.OnDone))
				{
					OnDone();
				}
				return false;
			}
			return Ticker.IsValid();
		}

		/** True when nobody will press the key: a scripted check, an unattended or automated run, or auto-continue. */
		bool ShouldAutoContinue() const
		{
			return Options.bQuitOnTimeout || FApp::IsUnattended() || GIsAutomationTesting
				|| CVarScenarioAutoContinue.GetValueOnGameThread() > 0;
		}

		/** bNotify false at the end of a run: the game may be exiting, the panel is left alone. */
		void EndWaitForKey(const bool bNotify = true)
		{
			if (bWaitingForKey)
			{
				bWaitingForKey = false;
				HOLTestHooks::GetScenarioDisplay().WaitingForKey.Empty();
				if (bNotify)
				{
					HOLTestHooks::NotifyScenarioChanged(FString(), FColor::White);
				}
			}
		}

		void Finish(const bool bQuit)
		{
			SetWorldDrawn(true);
			if (WaitFrameSeconds > 0.0)
			{
				FApp::SetFixedDeltaTime(WaitFrameSeconds);
			}
			EndWaitForKey(/*bNotify*/ false);
			GLog->RemoveOutputDevice(Watcher.Get());
			bRunning = false;
			FTSTicker::RemoveTicker(Ticker);
			Ticker.Reset();
			Next = Steps.Num();
			if (bQuit)
			{
				GEngine->Exec(GetGameWorld(), TEXT("quit"));
			}
		}

		TArray<FString> Steps;
		HOLTestSequence::FRunOptions Options;
		int32 Next = 0;
		/** The current step's timeout: StepTimeoutSeconds, or a waitfile's own. */
		double StepTimeout = StepTimeoutSeconds;
		TUniquePtr<FTestLogWatcher> Watcher;
		/** Log lines from here on are searched by waitlog: those written since the last command. */
		int32 LogMark = 0;
		/** Real time (the timeouts) and the game's time (FApp::GetCurrentTime: the wait steps). */
		double RunStart = 0.0;
		double RunStartGame = 0.0;
		uint64 RunStartFrame = 0;
		double WaitStart = 0.0;
		double WaitStartGame = 0.0;
		/** The -FPS step of a fixed time step (0: real time); the frames of the other steps take CommandFrameSeconds. */
		double WaitFrameSeconds = 0.0;
		/** hol.Test.SkipWaitRendering: the world drawn, and the frames drawn since it is again (a capture waits for some). */
		bool bWorldDrawn = true;
		int32 FramesDrawn = CaptureDrawnFrames;
		int32 FramesWaited = 0;
		FTSTicker::FDelegateHandle Ticker;
		/** From Start to Finish: the watcher is in GLog. */
		bool bRunning = false;
		/** The current step is a waitkey: the panel says so, the key is awaited. */
		bool bWaitingForKey = false;
		/** The continue key was pressed during the current waitkey. */
		bool bKeyPressed = false;

	public:
		/** The continue key: true when this run was waiting for it. */
		bool PressKey()
		{
			if (!bRunning || !bWaitingForKey)
			{
				return false;
			}
			bKeyPressed = true;
			return true;
		}
	};

	/** One run per owner: hol.Test.Run's, a scenario's. */
	TMap<FName, TUniquePtr<FTestSequence>> Sequences;

	/** Before the engine exits (OnEnginePreExit), while GLog still stands: no run left to Sequences' destruction. */
	void StopSequences()
	{
		for (const TPair<FName, TUniquePtr<FTestSequence>>& Pair : Sequences)
		{
			Pair.Value->Stop();
		}
	}

	bool PressContinueKey()
	{
		bool bWaited = false;
		for (const TPair<FName, TUniquePtr<FTestSequence>>& Pair : Sequences)
		{
			bWaited |= Pair.Value->PressKey();
		}
		return bWaited;
	}

	FTestSequence& GetSequence(const FName Owner)
	{
		// Once, with the first run; a static function, no object left pointed to.
		[[maybe_unused]] static const FDelegateHandle PreExitHandle = FCoreDelegates::OnEnginePreExit.AddStatic(&StopSequences);
		TUniquePtr<FTestSequence>& Sequence = Sequences.FindOrAdd(Owner);
		if (!Sequence)
		{
			Sequence = MakeUnique<FTestSequence>();
		}
		return *Sequence;
	}

	void RunSequence(const TArray<FString>& Args)
	{
		TArray<FString> Steps;
		if (Args.IsEmpty() || !HOLTestSequence::LoadSteps(Args[0], Steps))
		{
			UE_LOG(LogHOLTest, Error, TEXT("Usage: hol.Test.Run <File> (file %s not read)"), Args.IsEmpty() ? TEXT("missing") : *Args[0]);
			return;
		}
		GetSequence(TEXT("Test")).Start(MoveTemp(Steps));
	}

	FAutoConsoleCommand RunCommand(
		TEXT("hol.Test.Run"),
		TEXT("hol.Test.Run <File>: runs a file of steps one after the other, each as soon as the one before is done (scripted checks)"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&RunSequence));
}

bool HOLTestSequence::LoadSteps(const FString& File, TArray<FString>& OutSteps)
{
	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *File))
	{
		return false;
	}
	for (FString& Line : Lines)
	{
		Line.TrimStartAndEndInline();
		if (!Line.IsEmpty() && !Line.StartsWith(TEXT("#")))
		{
			OutSteps.Add(MoveTemp(Line));
		}
	}
	return true;
}

bool HOLTestHooks::PressContinueKey()
{
	const bool bWaited = UE::HeroesOfLiberty::Dev::PressContinueKey();
	UE_LOG(LogHOLTest, Display, TEXT("Scenario key: pressed (%s)"), bWaited ? TEXT("a step was waiting") : TEXT("nothing waiting"));
	return bWaited;
}

void HOLTestSequence::Run(const FName Owner, TArray<FString>&& Steps, FRunOptions&& Options)
{
	UE::HeroesOfLiberty::Dev::GetSequence(Owner).Start(MoveTemp(Steps), MoveTemp(Options));
}
