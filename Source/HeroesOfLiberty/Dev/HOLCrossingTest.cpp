// Copyright Epic Games, Inc. All Rights Reserved.

#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Map/HOLStartSlot.h"
#include "Match/HOLPlayerState.h"
#include "Math/RandomStream.h"
#include "NavigationSystem.h"
#include "Player/HOLPlayerController.h"
#include "Simulation/HOLUnitView.h"

/**
 * The ridge crossed again and again (the user's trials of ridge_army: an army sent across the ridge to a random point,
 * then back across it, and so on; it stood pressed against a rock, or half of it stayed behind). One army of the local
 * player is made between its main centre and its ridge (create_match_map.py: 30 m in front of the centre, ~20 m wide),
 * then ordered as the player's right click does (RequestMoveUnits) to a random point on the ridge's other side, each
 * time it has arrived. Every frame of each crossing: no unit's centre off the navmesh near the ridge (in a rock), no
 * frame where half the army walks without moving (a stall); then all its units standing near the point in time.
 *   hol.Test.Crossings <Count> [Seed=1] [Units=50] [Type=ManAtArms] [quit]
 * One line per crossing ("Crossing N/M: ... OK" or "... FAIL: why"), then "Crossings: done, M crossings, F failed".
 * Positions in the frame of the player's centre: X towards the map's middle, Y across.
 */
namespace UE::HeroesOfLiberty::Dev::Crossings
{
	/** The ridge's middle (cm, X of the frame) and where a unit off the navmesh is in it (its no-navmesh zone of 1 m). */
	constexpr double RidgeX = 3000.;
	constexpr double RockZoneX = 800.;
	constexpr double RockZoneY = 1900.;
	/** The random targets: in front (between the centre and the ridge) or behind it, across Y. */
	constexpr double FrontMinX = 1200.;
	constexpr double FrontMaxX = 2200.;
	constexpr double BackMinX = 4000.;
	constexpr double BackMaxX = 5500.;
	constexpr double TargetMaxY = 2500.;
	/** Arrived: every unit standing within this (cm) of the point (the block's front is on it, its rows behind). */
	constexpr double ArrivedRadius = 1500.;
	/**
	 * Time allowed (s): the navmesh path's length from the army's middle to the point (round the ridge: the straight line
	 * goes through it) at this pace (cm/s, below the Men-at-arms' 697), times the factor, plus the margin.
	 */
	constexpr double LimitPace = 650.;
	constexpr double LimitFactor = 1.4;
	constexpr double LimitMargin = 5.;
	/** Given up (s) past the limit: the crossing failed, the next one starts from where the army stands. */
	constexpr double GiveUpAfter = 10.;
	/**
	 * A stall: half the walking units moved less than this (cm) in a frame while still on their way (farther than
	 * ArrivedRadius from the point: not the ones standing on their places while the last ones come); at most
	 * StallMaxSeconds in a crossing.
	 */
	constexpr double StallMove = 0.5;
	constexpr double StallMaxSeconds = 0.6;

	struct FRun
	{
		int32 Count = 0;
		int32 NumUnits = 50;
		FString Type = TEXT("ManAtArms");
		bool bQuit = false;
		FRandomStream Random;
		int32 Slot = 0;
		FVector2D Centre = FVector2D::ZeroVector;
		FVector2D Radial = FVector2D(1., 0.);
		FVector2D Across = FVector2D(0., 1.);
		TArray<uint32> Army;
		int32 Frames = 0;
		int32 Trip = 0;
		bool bInFront = true;
		bool bWalking = false;
		double PauseUntil = 0.;
		FVector2D Target = FVector2D::ZeroVector;
		double Start = 0.;
		double Limit = 0.;
		double LastTime = 0.;
		int32 InRock = 0;
		int32 MostInRock = 0;
		double Stall = 0.;
		bool bStallReported = false;
		TMap<uint32, FVector2D> Last;
		TArray<FString> Failures;
		FTSTicker::FDelegateHandle Ticker;

		FVector2D ToWorld(const double X, const double Y) const { return Centre + Radial * X + Across * Y; }
		FVector2D ToFrame(const FVector2D& P) const { return FVector2D(FVector2D::DotProduct(P - Centre, Radial), FVector2D::DotProduct(P - Centre, Across)); }
	};
	TSharedPtr<FRun> Current;

	UWorld* GetGameWorld()
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

	void Finish(FRun& Run, UWorld* World)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Crossings: done, %d crossings, %d failed%s%s"), Run.Trip, Run.Failures.Num(),
			Run.Failures.IsEmpty() ? TEXT("") : TEXT(": "), *FString::Join(Run.Failures, TEXT(", ")));
		FTSTicker::GetCoreTicker().RemoveTicker(Run.Ticker);
		if (Run.bQuit && World)
		{
			GEngine->Exec(World, TEXT("quit"));
		}
	}

	void Order(FRun& Run, UWorld& World, AHOLPlayerController& Controller, const TArray<FHOLUnitView>& Units)
	{
		const bool bToFront = !Run.bInFront;
		const double X = bToFront ? Run.Random.FRandRange(FrontMinX, FrontMaxX) : Run.Random.FRandRange(BackMinX, BackMaxX);
		const double Y = Run.Random.FRandRange(-TargetMaxY, TargetMaxY);
		Run.Target = Run.ToWorld(X, Y);
		FVector2D Middle = FVector2D::ZeroVector;
		int32 Found = 0;
		for (const FHOLUnitView& Unit : Units)
		{
			if (Run.Army.Contains(Unit.UnitId))
			{
				Middle += FVector2D(Unit.Location);
				++Found;
			}
		}
		Middle /= FMath::Max(Found, 1);
		double Distance = FVector2D::Distance(Middle, Run.Target);
		UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(&World);
		const ANavigationData* NavData = NavSystem ? NavSystem->GetDefaultNavDataInstance() : nullptr;
		FNavLocation From;
		FNavLocation To;
		if (NavData && NavSystem->ProjectPointToNavigation(FVector(Middle, 0.), From, FVector(500., 500., 500.), NavData)
			&& NavSystem->ProjectPointToNavigation(FVector(Run.Target, 0.), To, FVector(500., 500., 500.), NavData))
		{
			const FPathFindingResult Result = NavSystem->FindPathSync(FPathFindingQuery(&World, *NavData, From.Location, To.Location));
			if (Result.IsSuccessful() && Result.Path.IsValid())
			{
				Distance = FMath::Max(Distance, static_cast<double>(Result.Path->GetLength()));
			}
		}
		Run.Start = World.GetTimeSeconds();
		Run.LastTime = Run.Start;
		Run.Limit = Distance / LimitPace * LimitFactor + LimitMargin;
		Run.InRock = 0;
		Run.MostInRock = 0;
		Run.Stall = 0.;
		Run.bStallReported = false;
		Run.Last.Reset();
		Run.bWalking = true;
		++Run.Trip;
		const FVector2D Start = Run.ToFrame(Middle);
		UE_LOG(LogHOLMatch, Display, TEXT("Crossing %d/%d: %s, from (%.0f, %.0f) to (%.0f, %.0f) of the centre's frame, path %.1f m, %d units"), Run.Trip, Run.Count,
			bToFront ? TEXT("to the front") : TEXT("to the back"), Start.X, Start.Y, X, Y, Distance / 100., Found);
		Controller.RequestMoveUnits(Run.Army, FVector(Run.Target, 0.));
	}

	bool Tick(const TSharedRef<FRun>& RunRef)
	{
		FRun& Run = *RunRef;
		UWorld* World = GetGameWorld();
		AHOLPlayerController* Controller = World ? Cast<AHOLPlayerController>(World->GetFirstPlayerController()) : nullptr;
		if (!World || !Controller || Current != RunRef)
		{
			return false;
		}
		TArray<FHOLUnitView> Units;
		HOLUnitView::Gather(*World, Units);
		// The army made: its units of the slot and type near the spawn point, once all there.
		if (Run.Army.IsEmpty())
		{
			if (++Run.Frames < 10)
			{
				return true;
			}
			const FVector2D Spawn = Run.ToWorld(1600., 0.);
			TArray<uint32> Found;
			for (const FHOLUnitView& Unit : Units)
			{
				if (Unit.OwnerPlayer == Run.Slot && Unit.TypeName == FName(*Run.Type) && FVector2D::Distance(FVector2D(Unit.Location), Spawn) <= 1500.)
				{
					Found.Add(Unit.UnitId);
				}
			}
			if (Found.Num() >= Run.NumUnits)
			{
				Run.Army = MoveTemp(Found);
				Run.PauseUntil = World->GetTimeSeconds() + 0.5;
			}
			else if (Run.Frames > 600)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Crossings: the army was not made (%d of %d)"), Found.Num(), Run.NumUnits);
				Finish(Run, World);
				return false;
			}
			return true;
		}
		const double Now = World->GetTimeSeconds();
		if (!Run.bWalking)
		{
			if (Now < Run.PauseUntil)
			{
				return true;
			}
			if (Run.Trip >= Run.Count)
			{
				Finish(Run, World);
				return false;
			}
			Order(Run, *World, *Controller, Units);
			return true;
		}

		// Every frame: in a rock, a stall, arrived.
		const UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
		const ANavigationData* NavData = NavSystem ? NavSystem->GetDefaultNavDataInstance() : nullptr;
		int32 InRockNow = 0;
		int32 Walking = 0;
		int32 StillWalking = 0;
		int32 Arrived = 0;
		int32 Alive = 0;
		TArray<FString> Out;
		for (const FHOLUnitView& Unit : Units)
		{
			if (!Run.Army.Contains(Unit.UnitId))
			{
				continue;
			}
			++Alive;
			const FVector2D Position(Unit.Location);
			const FVector2D Frame = Run.ToFrame(Position);
			FNavLocation OnNav;
			if (NavData && FMath::Abs(Frame.X - RidgeX) <= RockZoneX && FMath::Abs(Frame.Y) <= RockZoneY
				&& !NavSystem->ProjectPointToNavigation(Unit.Location, OnNav, FVector(10., 10., 100.), NavData))
			{
				++InRockNow;
				if (Run.InRock + InRockNow <= 5)
				{
					UE_LOG(LogHOLMatch, Display, TEXT("Crossing %d/%d: #%u in a rock at (%.0f, %.0f) of the frame, world (%.0f, %.0f, %.0f)"), Run.Trip, Run.Count, Unit.UnitId,
						Frame.X, Frame.Y, Unit.Location.X, Unit.Location.Y, Unit.Location.Z);
				}
			}
			const bool bMoving = Unit.Activity == EHOLUnitActivity::Moving;
			if (bMoving)
			{
				++Walking;
				const FVector2D* Before = Run.Last.Find(Unit.UnitId);
				StillWalking += Before && FVector2D::Distance(*Before, Position) < StallMove && FVector2D::Distance(Position, Run.Target) > ArrivedRadius ? 1 : 0;
			}
			const bool bThere = !bMoving && FVector2D::Distance(Position, Run.Target) <= ArrivedRadius;
			Arrived += bThere ? 1 : 0;
			if (!bThere && Out.Num() < 8)
			{
				Out.Add(FString::Printf(TEXT("#%u (%.0f, %.0f) %s"), Unit.UnitId, Frame.X, Frame.Y, bMoving ? TEXT("moving") : TEXT("standing")));
			}
			Run.Last.Add(Unit.UnitId, Position);
		}
		const double Delta = Now - Run.LastTime;
		Run.LastTime = Now;
		Run.InRock += InRockNow;
		Run.MostInRock = FMath::Max(Run.MostInRock, InRockNow);
		if (Walking > 0 && StillWalking * 2 >= Walking && Delta > 0.)
		{
			Run.Stall += Delta;
		}
		// A long stall: each unit's order phase logged once (hol.Units.Orders), to see what holds them.
		if ((Run.Stall > 0.5 || (Now - Run.Start > Run.Limit && Arrived < Alive)) && !Run.bStallReported)
		{
			Run.bStallReported = true;
			UE_LOG(LogHOLMatch, Display, TEXT("Crossing %d/%d: stalled %.1f s or late, the orders:"), Run.Trip, Run.Count, Run.Stall);
			GEngine->Exec(World, *FString::Printf(TEXT("hol.Units.Orders %d"), Run.Slot));
		}
		const double Elapsed = Now - Run.Start;
		const bool bAllThere = Alive > 0 && Arrived == Alive;
		if (!bAllThere && Elapsed < Run.Limit + GiveUpAfter)
		{
			return true;
		}
		TArray<FString> Why;
		if (!bAllThere)
		{
			Why.Add(FString::Printf(TEXT("%d of %d not arrived after %.1f s: %s"), Alive - Arrived, Alive, Elapsed, *FString::Join(Out, TEXT("; "))));
		}
		else if (Elapsed > Run.Limit)
		{
			Why.Add(FString::Printf(TEXT("late (%.1f s, limit %.1f s)"), Elapsed, Run.Limit));
		}
		if (Run.InRock > 0)
		{
			Why.Add(FString::Printf(TEXT("%d unit-frames in a rock (%d at once)"), Run.InRock, Run.MostInRock));
		}
		if (Run.Stall > StallMaxSeconds)
		{
			Why.Add(FString::Printf(TEXT("stalled %.1f s"), Run.Stall));
		}
		UE_LOG(LogHOLMatch, Display, TEXT("Crossing %d/%d: arrived %.1f s (limit %.1f s), in a rock %d, stall %.1f s, %s%s"), Run.Trip, Run.Count, Elapsed, Run.Limit,
			Run.InRock, Run.Stall, Why.IsEmpty() ? TEXT("OK") : TEXT("FAIL: "), *FString::Join(Why, TEXT(", ")));
		if (!Why.IsEmpty())
		{
			Run.Failures.Add(FString::FromInt(Run.Trip));
		}
		Run.bInFront = !Run.bInFront;
		Run.bWalking = false;
		Run.PauseUntil = Now + 0.3;
		return true;
	}

	void Start(const TArray<FString>& Args)
	{
		UWorld* World = GetGameWorld();
		AHOLPlayerController* Controller = World ? Cast<AHOLPlayerController>(World->GetFirstPlayerController()) : nullptr;
		const AHOLPlayerState* PlayerState = Controller ? Controller->GetHOLPlayerState() : nullptr;
		const AHOLStartSlot* StartSlot = PlayerState ? AHOLStartSlot::Find(World, PlayerState->GetPlayerIndex()) : nullptr;
		if (Args.IsEmpty() || !StartSlot)
		{
			UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Test.Crossings <Count> [Seed=1] [Units=50] [Type=ManAtArms] [quit], in a match"));
			return;
		}
		TSharedRef<FRun> Run = MakeShared<FRun>();
		Run->Count = FMath::Max(1, FCString::Atoi(*Args[0]));
		Run->Random.Initialize(Args.Num() > 1 ? FCString::Atoi(*Args[1]) : 1);
		Run->NumUnits = Args.Num() > 2 ? FMath::Max(1, FCString::Atoi(*Args[2])) : 50;
		Run->Type = Args.Num() > 3 && Args[3] != TEXT("quit") ? Args[3] : TEXT("ManAtArms");
		Run->bQuit = Args.Contains(TEXT("quit"));
		Run->Slot = PlayerState->GetPlayerIndex();
		Run->Centre = FVector2D(StartSlot->GetMainCentreLocation());
		Run->Radial = (-Run->Centre).GetSafeNormal();
		Run->Across = FVector2D(-Run->Radial.Y, Run->Radial.X);
		const FVector2D Spawn = Run->ToWorld(1600., 0.);
		GEngine->Exec(World, *FString::Printf(TEXT("hol.Units.Spawn %d %.0f %.0f %d %s 600"), Run->NumUnits, Spawn.X, Spawn.Y, Run->Slot, *Run->Type));
		UE_LOG(LogHOLMatch, Display, TEXT("Crossings: %d crossings of slot %d's ridge, %d %s, seed %s"), Run->Count, Run->Slot, Run->NumUnits, *Run->Type,
			Args.Num() > 1 ? *Args[1] : TEXT("1"));
		Current = Run;
		Run->Ticker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Run](float) { return Tick(Run); }));
	}

	FAutoConsoleCommand CrossingsCommand(
		TEXT("hol.Test.Crossings"),
		TEXT("hol.Test.Crossings <Count> [Seed=1] [Units=50] [Type=ManAtArms] [quit]: an army crosses the player's ridge again and again to random points, each crossing checked (no unit in a rock, no stall, arrived in time)"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&Start));
}
