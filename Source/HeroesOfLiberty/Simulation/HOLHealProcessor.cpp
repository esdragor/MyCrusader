// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLHealProcessor.h"

#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLUnitCommand.h"
#include "Engine/World.h"
#include "HeroesOfLibertyLog.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "Match/HOLVictoryRules.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLHealRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

TArray<UHOLHealProcessor::FLink> UHOLHealProcessor::Links;
double UHOLHealProcessor::TotalHealed = 0.;
int64 UHOLHealProcessor::HealSteps = 0;
TMap<uint32, UHOLHealProcessor::FOrder> UHOLHealProcessor::Orders;
bool UHOLHealProcessor::bIssuing = false;

void UHOLHealProcessor::OrderHeal(const uint32 HealerId, const uint32 TargetId)
{
	Orders.Add(HealerId, FOrder{ TargetId, -1 });
}

void UHOLHealProcessor::ClearOrders()
{
	Orders.Reset();
}

void UHOLHealProcessor::CancelOrders(const TConstArrayView<uint32> UnitIds)
{
	if (Orders.IsEmpty() || bIssuing)
	{
		return;
	}
	for (const uint32 UnitId : UnitIds)
	{
		Orders.Remove(UnitId);
	}
}

UHOLHealProcessor::UHOLHealProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	// After the combat of the step: the hits of the step come off first, the heal then goes on what is left.
	ExecutionOrder.ExecuteAfter.Add(TEXT("HOLCombatProcessor"));
	bRequiresGameThreadExecution = true; // the unit subsystem
}

void UHOLHealProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FHOLUnitStateFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHOLCombatFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FHOLUnitTypeConstants>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
}

void UHOLHealProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	Links.Reset();
	UWorld* World = EntityManager.GetWorld();
	const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	if (!Units)
	{
		return;
	}
	const int32 StepMs = HOLCombat::ToMilliseconds(Context.GetDeltaTimeSeconds());
	const FHOLCampTable CampTable = FHOLCampTable::FromWorld(World);
	const int64 StepIndex = World->GetSubsystem<UHOLSimulationSubsystem>() ? World->GetSubsystem<UHOLSimulationSubsystem>()->GetStepIndex() : 0;

	// The step's units: what the rules read, and where to write the health.
	struct FUnit
	{
		HOLHeal::FCandidate Candidate;
		FHOLUnitStateFragment* State = nullptr;
		const FHOLUnitTypeConstants* Constants = nullptr;
		FVector Location3 = FVector::ZeroVector;
		uint8 Camp = 0;
		bool bHealer = false;
	};
	TArray<FUnit> Step;
	bool bAnyHealer = false;
	EntityQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& Chunk)
	{
		const TArrayView<FHOLUnitStateFragment> States = Chunk.GetMutableFragmentView<FHOLUnitStateFragment>();
		const TConstArrayView<FHOLCombatFragment> Combats = Chunk.GetFragmentView<FHOLCombatFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Chunk.GetFragmentView<FTransformFragment>();
		const FHOLUnitTypeConstants& Constants = Chunk.GetConstSharedFragment<FHOLUnitTypeConstants>();
		const bool bHealer = Constants.HealRate > 0.f && Constants.HealRange > 0.f;
		const bool bUnhealable = !HOLCanBeHealed(Constants.ArmorCategoryMask)
			|| (Constants.ArmorCategoryMask & HOLArmorCategoryBit(EHOLArmorCategory::Siege)) != 0;
		for (FMassExecutionContext::FEntityIterator It = Chunk.CreateEntityIterator(); It; ++It)
		{
			FUnit& Unit = Step.AddDefaulted_GetRef();
			FHOLUnitStateFragment& State = States[It];
			Unit.State = &State;
			Unit.Constants = &Constants;
			Unit.Camp = CampTable.Get(State.OwnerPlayer);
			Unit.Location3 = Transforms[It].GetTransform().GetLocation();
			Unit.bHealer = bHealer;
			bAnyHealer |= bHealer;
			HOLHeal::FCandidate& Candidate = Unit.Candidate;
			Candidate.UnitId = State.UnitId;
			Candidate.Camp = Unit.Camp;
			Candidate.Location = FVector2D(Transforms[It].GetTransform().GetLocation());
			Candidate.Health = State.CurrentHealth;
			Candidate.MaxHealth = static_cast<float>(HOLCombat::GetMaxHealth(Constants, Combats[It].Level, Units->GetTechnologyHealthPercent(Constants, State.OwnerPlayer)));
			Candidate.bUnhealable = bUnhealable || Combats[It].DeathStep >= 0 || State.Activity == EHOLUnitActivity::Dying;
		}
	});
	if (!bAnyHealer)
	{
		Kept.Reset();
		Orders.Reset();
		return;
	}
	Step.Sort([](const FUnit& A, const FUnit& B) { return A.Candidate.UnitId < B.Candidate.UnitId; });

	TMap<uint32, uint32> NewKept;
	// The units as the rules read them, in the same order as Step; a heal updates the target's health here, for the healers after it.
	TArray<HOLHeal::FCandidate> Candidates;
	Candidates.Reserve(Step.Num());
	for (const FUnit& Unit : Step)
	{
		Candidates.Add(Unit.Candidate);
	}
	for (int32 HealerIndex = 0; HealerIndex < Step.Num(); ++HealerIndex)
	{
		const FUnit& Healer = Step[HealerIndex];
		if (!Healer.bHealer || Healer.State->CurrentHealth <= 0.f || Healer.Candidate.bUnhealable)
		{
			continue;
		}
		const uint32* KeptId = Kept.Find(Healer.Candidate.UnitId);
		// The unit its player ordered it to heal (a right click on an ally), for OrderSteps steps at most.
		uint32 OrderedId = 0;
		if (FOrder* Order = Orders.Find(Healer.Candidate.UnitId))
		{
			Order->Step = Order->Step < 0 ? StepIndex : Order->Step;
			if (StepIndex - Order->Step >= OrderSteps)
			{
				Orders.Remove(Healer.Candidate.UnitId);
			}
			else
			{
				OrderedId = Order->Target;
			}
		}
		// The walk of the order (the user's trial E-monk-soigner): the healer goes only until its target is within its heal range, never to
		// contact, follows a target that moves off, and the order ends when the target is gone (dead, not an ally) or healed to full.
		if (OrderedId != 0)
		{
			FOrder& Order = Orders[Healer.Candidate.UnitId];
			const uint32 HealerId = Healer.Candidate.UnitId;
			const auto IssueStop = [&]()
			{
				if (UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>())
				{
					TGuardValue<bool> Issuing(bIssuing, true);
					const uint32 Ids[1] = { HealerId };
					Commands->IssueCombatOrder(Ids, EHOLOrderKind::Stop, 0, Healer.State->OwnerPlayer);
				}
				Order.bWalking = false;
			};
			const int32 TargetIndex = Candidates.IndexOfByPredicate([OrderedId](const HOLHeal::FCandidate& C) { return C.UnitId == OrderedId; });
			const HOLHeal::FCandidate* Target = TargetIndex != INDEX_NONE ? &Candidates[TargetIndex] : nullptr;
			if (!Target || Target->Health <= 0.f || Target->bUnhealable || Target->Camp != Healer.Camp || Target->UnitId == HealerId)
			{
				// Gone: dead, a King or an engine, not an ally. The walk it gave ends where the healer stands.
				if (Order.bWalking)
				{
					IssueStop();
				}
				UE_LOG(LogHOLSimulation, Display, TEXT("Heal: step %lld #%u (slot %d) order on #%u ends, the target is gone"), StepIndex, HealerId, Healer.State->OwnerPlayer, OrderedId);
				Orders.Remove(HealerId);
				OrderedId = 0;
			}
			else
			{
				const float Range = Healer.Constants->HealRange;
				const float Distance = static_cast<float>(FVector2D::Distance(Healer.Candidate.Location, Target->Location));
				const bool bFirst = !Order.bStarted;
				const float Drift = static_cast<float>(FVector2D::Distance(Order.Goal, Target->Location));
				// The first look is taken as a walk on (to end the healer's earlier walk if it stands in reach).
				const HOLHeal::EOrderWalk Walk = HOLHeal::GetOrderWalk(Distance, Range, bFirst || Order.bWalking, bFirst ? TNumericLimits<float>::Max() : Drift);
				Order.bStarted = true;
				if (Walk == HOLHeal::EOrderWalk::Walk)
				{
					if (UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>())
					{
						TGuardValue<bool> Issuing(bIssuing, true);
						const uint32 Ids[1] = { HealerId };
						Commands->IssueMoveOrder(Ids, Step[TargetIndex].Location3, Healer.State->OwnerPlayer);
					}
					Order.bWalking = true;
					Order.Goal = Target->Location;
					if (bFirst || Drift > HOLHeal::OrderRepathDistance)
					{
						UE_LOG(LogHOLSimulation, Display, TEXT("Heal: step %lld #%u (slot %d) walks to #%u, %.0f cm away, to heal it within %.0f cm"), StepIndex, HealerId, Healer.State->OwnerPlayer, OrderedId, Distance, Range);
					}
				}
				else if (Walk == HOLHeal::EOrderWalk::Stop)
				{
					IssueStop();
					UE_LOG(LogHOLSimulation, Display, TEXT("Heal: step %lld #%u (slot %d) stops in reach of #%u at %.0f cm (heal range %.0f cm, %s)"), StepIndex, HealerId,
						Healer.State->OwnerPlayer, OrderedId, Distance, Range, Distance >= Range * 0.5f ? TEXT("at range") : TEXT("too close"));
				}
			}
		}
		const int32 Chosen = HOLHeal::ChooseTarget(Healer.Candidate.UnitId, Healer.Camp, Healer.Candidate.Location, Healer.Constants->HealRange, OrderedId, KeptId ? *KeptId : 0u, Candidates);
		if (Chosen == INDEX_NONE)
		{
			if (KeptId)
			{
				UE_LOG(LogHOLSimulation, Display, TEXT("Heal: step %lld #%u (slot %d) stops, nobody left to heal in reach"), StepIndex, Healer.Candidate.UnitId, Healer.State->OwnerPlayer);
			}
			continue;
		}
		FUnit& Target = Step[Chosen];
		const float Before = Target.State->CurrentHealth;
		const float After = HOLHeal::GetHealedHealth(Before, Target.Candidate.MaxHealth, Healer.Constants->HealRate, StepMs);
		Target.State->CurrentHealth = After;
		Candidates[Chosen].Health = After;
		TotalHealed += static_cast<double>(After - Before);
		NewKept.Add(Healer.Candidate.UnitId, Target.Candidate.UnitId);
		Links.Add(FLink{ Healer.Candidate.UnitId, Target.Candidate.UnitId });
		if (!KeptId || *KeptId != Target.Candidate.UnitId)
		{
			UE_LOG(LogHOLSimulation, Display, TEXT("Heal: step %lld #%u (slot %d) heals #%u (slot %d), health %.2f/%.0f%s"), StepIndex, Healer.Candidate.UnitId,
				Healer.State->OwnerPlayer, Target.Candidate.UnitId, Target.State->OwnerPlayer, Before, Target.Candidate.MaxHealth,
				Target.Candidate.UnitId == OrderedId ? TEXT(" (ordered)") : TEXT(""));
		}
		if (After >= Target.Candidate.MaxHealth)
		{
			if (Target.Candidate.UnitId == OrderedId)
			{
				// Healed to full: the order is over, and so is the walk it gave if the healer was still on it.
				if (const FOrder* Over = Orders.Find(Healer.Candidate.UnitId); Over && Over->bWalking)
				{
					if (UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>())
					{
						TGuardValue<bool> Issuing(bIssuing, true);
						const uint32 Ids[1] = { Healer.Candidate.UnitId };
						Commands->IssueCombatOrder(Ids, EHOLOrderKind::Stop, 0, Healer.State->OwnerPlayer);
					}
				}
				Orders.Remove(Healer.Candidate.UnitId);
			}
			UE_LOG(LogHOLSimulation, Display, TEXT("Heal: step %lld #%u (slot %d) is healed to %.0f by #%u"), StepIndex, Target.Candidate.UnitId, Target.State->OwnerPlayer,
				Target.Candidate.MaxHealth, Healer.Candidate.UnitId);
		}
	}
	Kept = MoveTemp(NewKept);
	HealSteps += Links.Num();
}
