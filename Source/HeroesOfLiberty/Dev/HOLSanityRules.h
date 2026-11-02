// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * The rules of hol.Test.Sanity (Auto_Journey, "Qualité des essais"): what a trial must never show, decided here from a
 * unit's recorded steps (no Mass, no world: the watcher in HOLSanity.cpp feeds it the real simulation's steps).
 * Thresholds live in FSanityRules, one place.
 */
namespace HOLSanity
{
	struct FSanityRules
	{
		/** The window (s) a spinning or a trembling is judged over. */
		double WindowSeconds = 3.0;
		/** A heading is taken once the unit has moved this far (cm) from the last anchor: smaller moves are noise, not a course. */
		double CourseSegmentCm = 40.0;
		/** Spinning: a unit walking all window long turned this much in total (degrees) ... */
		double SpinTurnDegrees = 540.0;
		/** ... and got less than this share of the length it walked (net advance / path length) closer to anywhere. */
		double SpinAdvanceShare = 0.25;
		/** Spinning needs at least this much walked (cm), so a unit that barely moves is not a spinner. */
		double SpinMinPathCm = 300.0;
		/** Trembling: a unit with no order moved more than this (cm) in a step ... */
		double ShakeStepCm = 1.0;
		/** ... in at least this share of the steps of the window ... */
		double ShakeStepShare = 0.8;
		/** ... and went nowhere (net / path under this share): it vibrates, it is not pushed along. */
		double ShakeAdvanceShare = 0.25;
	};

	/**
	 * Check 1 of hol.Test.Sanity (SAN-Q1 = A, D616): the owners in Neutrals must be drawn in no player's colour. The colour of an owner is
	 * the game's one rule, HOLPlayerColors::GetOwnerLinearColor, the same call as the drawing processor's (no copy here); SlotColorIndices[Slot]
	 * is the colour each slot chose. One line per clash ("owner 8 is drawn in the colour of slot 0 (Bleu)"); empty when none.
	 */
	HEROESOFLIBERTY_API TArray<FString> FindColourClashes(TConstArrayView<int32> Neutrals, TConstArrayView<int32> PlayerSlots, TConstArrayView<uint8> SlotColorIndices);

	/** What a unit is doing at a step, for the rules: walking to an order, standing with nothing to do, or busy (fighting, working). */
	enum class EUnitMotion : uint8
	{
		Ordered,
		Idle,
		Busy,
	};

	/** One unit's recent steps. Add one sample per simulation step; Judge says whether the window shows a fault. */
	class HEROESOFLIBERTY_API FUnitTrack
	{
	public:
		/** The unit at Time (s) at Location (XY), doing Motion. */
		void Add(double Time, const FVector2D& Location, EUnitMotion Motion, const FSanityRules& Rules);

		/** Empty while the window holds no fault; otherwise "spins" or "shakes" with the numbers. Judged once the window is full. */
		FString Judge(const FSanityRules& Rules) const;

	private:
		struct FSample
		{
			double Time = 0.;
			FVector2D Location = FVector2D::ZeroVector;
			EUnitMotion Motion = EUnitMotion::Busy;
		};
		TArray<FSample> Samples;
	};

	// ---- The second audit (Duel, DU: the user's Spearman of the neutrals trial and the units round the hero in duel_challenge), merged from hero-j8-close: pure rules on a whole window of a unit's places (hol.Test.Sanity runs them over a rolling window of the watch). ----
	/** One unit over a window: where it stood at each step, and whether it had an order / a target at each step. */
	struct FTrace
	{
		uint32 UnitId = 0;
		float Radius = 60.f;
		TArray<FVector2D> Places;
		TArray<uint8> Ordered;
	/** 1 at a step where the unit was going round the end of its line (a rear flank, Q-flank-demi-tour = A): a deliberate manoeuvre, never judged as a spin or a tremor. */
	TArray<uint8> Flanking;
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
		/**
		 * A trembling walk: a unit that makes headway (net over half its path) yet whose heading zigzags: the turns that undo the turn just
		 * before them (a turn to the left right after a turn to the right), summed, more than this many radians a second. A curve, a pivot
		 * on the spot, a walk that bends round bodies all turn the same way and do not count: only the to-and-fro does (a wobble of +-12 cm
		 * a step at 35 cm ahead is 15 rad/s; a charge through a crowd measured under 1).
		 */
		float JitterZigzagPerSecond = 3.f;
	};

	struct FResult
	{
		bool bSpin = false;
		bool bShake = false;
		float Path = 0.f;
		float Net = 0.f;
		float Turn = 0.f;
		/** The turns added with their signs (left +, right -): what a unit going round something piles up; to-and-fro jostling cancels out of it. */
		float SignedTurn = 0.f;
		/** The part of Turn made of turns against the one before (to and fro), see FSettings::JitterZigzagPerSecond. */
		float Zigzag = 0.f;
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
		float PreviousTurn = 0.f;
		int32 Ordered = 0;
		const auto IsFlanking = [&Trace](const int32 Index) { return Trace.Flanking.IsValidIndex(Index) && Trace.Flanking[Index] != 0; };
		for (int32 Index = 1; Index < Count; ++Index)
		{
			Ordered += Trace.Ordered.IsValidIndex(Index) && Trace.Ordered[Index] ? 1 : 0;
			const FVector2D Move = Trace.Places[Index] - Trace.Places[Index - 1];
			const float Length = Move.Size();
			Result.Path += Length;
			if (IsFlanking(Index))
			{
				// A rear flank in progress: its turns and reversals are the manoeuvre, not a fault (Q-flank-demi-tour = A).
				PreviousMove = FVector2D::ZeroVector;
				PreviousTurn = 0.f;
				continue;
			}
			if (Length < Settings.MinMove)
			{
				continue;
			}
			if (!PreviousMove.IsZero())
			{
				const float Cos = FVector2D::DotProduct(Move, PreviousMove) / (Length * PreviousMove.Size());
				const float Angle = FMath::Acos(FMath::Clamp(Cos, -1.f, 1.f));
				const float Signed = FVector2D::CrossProduct(PreviousMove, Move) < 0.f ? -Angle : Angle;
				Result.Turn += Angle;
				Result.SignedTurn += Signed;
				Result.Reversals += Cos < -0.5f ? 1 : 0;
				if (Signed * PreviousTurn < 0.f)
				{
					Result.Zigzag += FMath::Min(FMath::Abs(Signed), FMath::Abs(PreviousTurn));
				}
				PreviousTurn = Signed;
			}
			PreviousMove = Move;
		}
		Result.Net = static_cast<float>(FVector2D::Distance(Trace.Places[0], Trace.Places.Last()));
		Result.OrderedShare = static_cast<float>(Ordered) / static_cast<float>(Count - 1);
		const float Seconds = static_cast<float>(Count - 1) * StepSeconds;
		Result.bSpin = Result.OrderedShare >= 0.9f && FMath::Abs(Result.SignedTurn) >= Settings.SpinTurn && Result.Net <= Settings.SpinNetRadii * Trace.Radius;
		Result.bShake = Seconds > 0.f && static_cast<float>(Result.Reversals) / Seconds >= Settings.ShakePerSecond && Result.Net <= Settings.ShakeNetShare * Result.Path;
		// A spin met in a part of the window only (a unit that walked up to a fight, then circled it): any 2 s of it going round
		// (a full turn of heading, in one direction, under 30% of the path made good) on an order counts. A unit shoved to and fro by its
		// neighbours turns as much but in both directions: that is a jostle (a tremor is judged below), not a unit going round.
		const int32 Window = FMath::Max(3, FMath::RoundToInt32(2.f / FMath::Max(StepSeconds, 0.001f)));
		for (int32 Begin = 0; !Result.bSpin && Begin + Window < Count; Begin += Window / 4 + 1)
		{
			float WindowTurn = 0.f, WindowPath = 0.f;
			int32 WindowOrdered = 0;
			FVector2D Before = FVector2D::ZeroVector;
			bool bFlankInWindow = false;
			for (int32 Index = Begin + 1; Index <= Begin + Window; ++Index)
			{
				bFlankInWindow |= IsFlanking(Index);
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
					const float Angle = FMath::Acos(FMath::Clamp(FVector2D::DotProduct(Move, Before) / (Length * Before.Size()), -1.f, 1.f));
					WindowTurn += FVector2D::CrossProduct(Before, Move) < 0.f ? -Angle : Angle;
				}
				Before = Move;
			}
			const float WindowNet = static_cast<float>(FVector2D::Distance(Trace.Places[Begin], Trace.Places[Begin + Window]));
			Result.bSpin = !bFlankInWindow && WindowOrdered >= 0.9f * Window && FMath::Abs(WindowTurn) >= 2.f * UE_PI && WindowPath > 6.f * Trace.Radius && WindowNet <= 0.3f * WindowPath;
		}
		// A walk with a heading that wobbles at each step, to and fro (the units round the hero in the duel_challenge trial: +-12 cm sideways a step
		// while advancing 35): headway made, but a turn of several radians a second, which a straight or gently curved walk never has.
		Result.bShake |= Seconds > 0.f && Result.Path > 10.f * Trace.Radius && Result.Net > Settings.ShakeNetShare * Result.Path && Result.Zigzag / Seconds >= Settings.JitterZigzagPerSecond;
		return Result;
	}
}
