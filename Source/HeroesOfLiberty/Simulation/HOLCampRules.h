// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * The neutral camps' rules (NeutralsInProgress.md N3, GDD 8.4, D32, D401), the C++ twin of Script/Neutrals/HOLCampRules.as (N1, the
 * same states, the same order of tests, the same numbers): pure functions of the settings and of what the camp sees, so that the
 * simulation (UHOLCampSubsystem) calls them and the tests run them alone. The two design choices left to the user (NQ1: aggro on
 * sight or only when attacked; NQ2: who takes the XP of a cleared camp) are settings, both coded; the console variables
 * hol.Camp.Aggro and hol.Camp.Credit pick the branch.
 */
namespace HOLCampRules
{
	/** NQ1: how a camp is drawn into a fight. */
	enum class EAggro : uint8
	{
		/** Anyone of an adversary camp within AggroRadius of the post draws it. */
		OnSight,
		/** Only an attack on one of its members draws it. */
		OnAttack,
	};

	/** NQ2: who gets the XP of a cleared camp. */
	enum class ECredit : uint8
	{
		LastBlow,
		MostDamage,
		Shared,
	};

	enum class EState : uint8
	{
		Guarding,
		Fighting,
		Returning,
		/** Every member is dead: for the rest of the game (D32: no respawn). */
		Cleared,
	};

	struct FSettings
	{
		EAggro Aggro = EAggro::OnSight;
		ECredit Credit = ECredit::Shared;
		/** cm, from the post (OnSight). */
		float AggroRadius = 1500.f;
		/** The leash: nobody within this of the post = the camp gives up; a member farther than this goes back (cm). */
		float LeashRadius = 3000.f;
		/** Back at the post when all members stand within this of it (cm). */
		float ArriveRadius = 400.f;
		/** Moderate (D32), shared out by the credit rule. */
		int32 RewardXP = 100;
	};

	struct FMember
	{
		uint32 Id = 0;
		FVector2D Location = FVector2D::ZeroVector;
	};

	/** A unit of an adversary camp near the post (the camp's owner is Neutral: everyone is an adversary, D401). */
	struct FIntruder
	{
		uint32 Id = 0;
		int32 Owner = INDEX_NONE;
		FVector2D Location = FVector2D::ZeroVector;
	};

	struct FStep
	{
		bool bChanged = false;
		/** The last member died in this step: the reward is due, once. */
		bool bJustCleared = false;
		/** The members to send back to the post. */
		TArray<uint32> ReturnIds;
	};

	/** One hit on a member, in the order it happened (the last is the last blow). */
	struct FHit
	{
		int32 Owner = INDEX_NONE;
		int32 Damage = 0;
	};

	struct FShare
	{
		int32 Owner = INDEX_NONE;
		int32 XP = 0;
	};

	/** One thought of a camp. Members are the living ones; bAttacked: one of them was hit by an adversary since the last thought. */
	HEROESOFLIBERTY_API FStep Step(EState& InOutState, const FSettings& Settings, const FVector2D& Post, const TArray<FMember>& Members,
		const TArray<FIntruder>& Intruders, bool bAttacked);

	/** The XP of a cleared camp by owner (ascending), from the hits it took; the shares add up to RewardXP. Nothing without a hit. */
	HEROESOFLIBERTY_API TArray<FShare> Rewards(const FSettings& Settings, const TArray<FHit>& Hits);

	HEROESOFLIBERTY_API const TCHAR* GetStateName(EState State);
}
