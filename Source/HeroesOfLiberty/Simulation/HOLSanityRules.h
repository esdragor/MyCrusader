// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Auto_Journey_File.md, "Processus": the motion audit of a scenario (hol.Test.SanityMotion). Pure rules on a unit's trace (its
 * place at each simulation step): a unit that SPINS (an ordered unit that goes round and round without getting anywhere: the
 * user's Spearman of the neutrals trial) and a unit that SHAKES (goes back and forth by small steps: the units round the hero
 * in the duel_challenge trial). Thresholds are in one place, tuned on the real chain (the scenarios).
 */
namespace HOLSanity
{
	/** One unit over a window: where it stood at each step, and whether it had an order / a target at each step. */
	struct FTrace
	{
		uint32 UnitId = 0;
		float Radius = 60.f;
		TArray<FVector2D> Places;
		TArray<uint8> Ordered;
		/** Debug text per step (hol.Test.SanityVerbose 2). */
		TArray<FString> States;
	};

	struct FSettings
	{
		/** Steps whose move is below this (cm) count as not moving (no heading). */
		float MinMove = 0.3f;
		/** A spin: this much turning (radians, summed over the window), ... */
		float SpinTurn = 3.f * UE_PI;
		/** ... on an ordered unit that ends within this many body radii of where it began. */
		float SpinNetRadii = 4.f;
		/** A shake: reversals (a step against the one before, both moving) per second, ... */
		float ShakePerSecond = 3.f;
		/** ... on a unit whose net move over the window is under this share of the path it walked. */
		float ShakeNetShare = 0.5f;
		/** A trembling walk: a unit that makes headway (net over half its path) yet turns more than this (radians a second) summed. */
		float JitterTurnPerSecond = 2.f;
	};

	struct FResult
	{
		bool bSpin = false;
		bool bShake = false;
		float Path = 0.f;
		float Net = 0.f;
		float Turn = 0.f;
		int32 Reversals = 0;
		float OrderedShare = 0.f;
	};

	inline FResult Analyse(const FTrace& Trace, const float StepSeconds, const FSettings& Settings = FSettings())
	{
		FResult Result;
		const int32 Count = Trace.Places.Num();
		if (Count < 3)
		{
			return Result;
		}
		FVector2D PreviousMove = FVector2D::ZeroVector;
		int32 Ordered = 0;
		for (int32 Index = 1; Index < Count; ++Index)
		{
			Ordered += Trace.Ordered.IsValidIndex(Index) && Trace.Ordered[Index] ? 1 : 0;
			const FVector2D Move = Trace.Places[Index] - Trace.Places[Index - 1];
			const float Length = Move.Size();
			Result.Path += Length;
			if (Length < Settings.MinMove)
			{
				continue;
			}
			if (!PreviousMove.IsZero())
			{
				const float Cos = FVector2D::DotProduct(Move, PreviousMove) / (Length * PreviousMove.Size());
				Result.Turn += FMath::Acos(FMath::Clamp(Cos, -1.f, 1.f));
				Result.Reversals += Cos < -0.5f ? 1 : 0;
			}
			PreviousMove = Move;
		}
		Result.Net = static_cast<float>(FVector2D::Distance(Trace.Places[0], Trace.Places.Last()));
		Result.OrderedShare = static_cast<float>(Ordered) / static_cast<float>(Count - 1);
		const float Seconds = static_cast<float>(Count - 1) * StepSeconds;
		Result.bSpin = Result.OrderedShare >= 0.9f && Result.Turn >= Settings.SpinTurn && Result.Net <= Settings.SpinNetRadii * Trace.Radius;
		Result.bShake = Seconds > 0.f && static_cast<float>(Result.Reversals) / Seconds >= Settings.ShakePerSecond && Result.Net <= Settings.ShakeNetShare * Result.Path;
		// A spin met in a part of the window only (a unit that walked up to a fight, then circled it): any 2 s of it going round
		// (a full turn of heading, under 30% of the path made good) on an order counts.
		const int32 Window = FMath::Max(3, FMath::RoundToInt32(2.f / FMath::Max(StepSeconds, 0.001f)));
		for (int32 Begin = 0; !Result.bSpin && Begin + Window < Count; Begin += Window / 4 + 1)
		{
			float WindowTurn = 0.f, WindowPath = 0.f;
			int32 WindowOrdered = 0;
			FVector2D Before = FVector2D::ZeroVector;
			for (int32 Index = Begin + 1; Index <= Begin + Window; ++Index)
			{
				WindowOrdered += Trace.Ordered.IsValidIndex(Index) && Trace.Ordered[Index] ? 1 : 0;
				const FVector2D Move = Trace.Places[Index] - Trace.Places[Index - 1];
				const float Length = Move.Size();
				WindowPath += Length;
				if (Length < Settings.MinMove)
				{
					continue;
				}
				if (!Before.IsZero())
				{
					WindowTurn += FMath::Acos(FMath::Clamp(FVector2D::DotProduct(Move, Before) / (Length * Before.Size()), -1.f, 1.f));
				}
				Before = Move;
			}
			const float WindowNet = static_cast<float>(FVector2D::Distance(Trace.Places[Begin], Trace.Places[Begin + Window]));
			Result.bSpin = WindowOrdered >= 0.9f * Window && WindowTurn >= 2.f * UE_PI && WindowPath > 6.f * Trace.Radius && WindowNet <= 0.3f * WindowPath;
		}
		// A walk with a heading that wobbles at each step (the units round the hero in the duel_challenge trial: +-12 cm sideways a step
		// while advancing 35): headway made, but a turn of several radians a second, which a straight or gently curved walk never has.
		Result.bShake |= Seconds > 0.f && Result.Path > 10.f * Trace.Radius && Result.Net > Settings.ShakeNetShare * Result.Path && Result.Turn / Seconds >= Settings.JitterTurnPerSecond;
		return Result;
	}
}


