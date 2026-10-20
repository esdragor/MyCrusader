// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Pure rules of the chains of queued orders (D328, J7b.5b-1a): each unit's steps (its current action, then its queued
 * actions, Shift) are grouped into chains, one per common queue: the units whose steps are the same share one chain.
 * No engine state: tested on their own (HeroesOfLiberty.Orders.Chains).
 */
namespace HOLOrderChain
{
	/** Steps kept per chain (D328): enough for any queue a player gives by hand; the rest is not drawn. */
	inline constexpr int32 MaxSteps = 16;

	enum class EStepKind : uint8
	{
		Move,
		Attack,
		AttackMove,
		Build,
		Gather,
	};

	/**
	 * One step: its kind and its target. Move, AttackMove: the point of the group order (not the unit's own place in the
	 * formation); Attack: the unit to take on (D223 identifier); Build: the site (an identifier the caller gives, its
	 * actor's unique id); Gather: the source (an index of UHOLSourceSubsystem).
	 */
	struct FStep
	{
		EStepKind Kind = EStepKind::Move;
		FVector Point = FVector::ZeroVector;
		uint32 TargetId = 0;
		int32 SourceIndex = INDEX_NONE;

		static FStep MakeMove(const FVector& InPoint, const bool bAttackMove = false)
		{
			FStep Step;
			Step.Kind = bAttackMove ? EStepKind::AttackMove : EStepKind::Move;
			Step.Point = InPoint;
			return Step;
		}
		static FStep MakeAttack(const uint32 UnitId)
		{
			FStep Step;
			Step.Kind = EStepKind::Attack;
			Step.TargetId = UnitId;
			return Step;
		}
		static FStep MakeBuild(const uint32 SiteId)
		{
			FStep Step;
			Step.Kind = EStepKind::Build;
			Step.TargetId = SiteId;
			return Step;
		}
		static FStep MakeGather(const int32 InSourceIndex)
		{
			FStep Step;
			Step.Kind = EStepKind::Gather;
			Step.SourceIndex = InSourceIndex;
			return Step;
		}
		bool operator==(const FStep& Other) const
		{
			return Kind == Other.Kind && Point.Equals(Other.Point, 1.) && TargetId == Other.TargetId && SourceIndex == Other.SourceIndex;
		}
		bool operator!=(const FStep& Other) const { return !(*this == Other); }
	};

	/** One unit's steps, current action first. */
	struct FUnitSteps
	{
		uint32 UnitId = 0;
		TArray<FStep> Steps;
	};

	/** A chain: its units (in the order given), its steps (at most MaxSteps). */
	struct FChain
	{
		TArray<uint32> UnitIds;
		TArray<FStep> Steps;
	};

	/**
	 * The chains of a player: units with the same steps (the first MaxSteps) share one chain, chains in the order of their
	 * first unit, units in the order given. A unit with no step makes no chain.
	 */
	HEROESOFLIBERTY_API void MakeChains(TConstArrayView<FUnitSteps> Units, TArray<FChain>& OutChains);

	HEROESOFLIBERTY_API const TCHAR* KindName(EStepKind Kind);
}
