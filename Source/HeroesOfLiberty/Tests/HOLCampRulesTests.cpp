// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLMatchConfig.h"
#include "Match/HOLVictoryRules.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCampRules.h"
#include "Simulation/HOLCampSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

using namespace HOLCampRules;

namespace
{
	TArray<FMember> MakeMembers(const int32 Count, const FVector2D& Where)
	{
		TArray<FMember> Members;
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Members.Add(FMember{ static_cast<uint32>(500 + Index), Where });
		}
		return Members;
	}

	TArray<FIntruder> MakeIntruder(const FVector2D& Post, const float Distance)
	{
		return { FIntruder{ 1, 0, Post + FVector2D(Distance, 0.) } };
	}
}

// N3: the camp rules (the C++ twin of Script/Neutrals/HOLCampRules.as: the same cases as HOLCampRules_Test.as).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCampRulesTest, "HeroesOfLiberty.Neutrals.CampRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLCampRulesTest::RunTest(const FString& Parameters)
{
	const FVector2D Post(10000., 0.);
	FSettings S;
	S.Aggro = EAggro::OnSight;
	const TArray<FMember> Members = MakeMembers(3, Post);

	// On sight: an intruder at the aggro radius (inclusive) draws the camp, one beyond does not.
	EState State = EState::Guarding;
	FStep Step = HOLCampRules::Step(State, S, Post, Members, TArray<FIntruder>(), false);
	TestTrue(TEXT("nobody near: guarding"), State == EState::Guarding && !Step.bChanged);
	HOLCampRules::Step(State, S, Post, Members, MakeIntruder(Post, S.AggroRadius + 1.f), false);
	TestTrue(TEXT("beyond the aggro radius: still guarding"), State == EState::Guarding);
	Step = HOLCampRules::Step(State, S, Post, Members, MakeIntruder(Post, S.AggroRadius), false);
	TestTrue(TEXT("at the aggro radius: fighting"), State == EState::Fighting && Step.bChanged);

	// The leash: nobody within it, everyone goes back; then at the post, guarding again.
	Step = HOLCampRules::Step(State, S, Post, Members, MakeIntruder(Post, S.LeashRadius + 1.f), false);
	TestTrue(TEXT("nobody within the leash: returning, all sent back"), State == EState::Returning && Step.ReturnIds.Num() == 3);
	HOLCampRules::Step(State, S, Post, MakeMembers(3, Post + FVector2D(S.ArriveRadius + 1.f, 0.)), TArray<FIntruder>(), false);
	TestTrue(TEXT("not yet at the post: still returning"), State == EState::Returning);
	HOLCampRules::Step(State, S, Post, MakeMembers(3, Post + FVector2D(S.ArriveRadius, 0.)), TArray<FIntruder>(), false);
	TestTrue(TEXT("at the post: guarding"), State == EState::Guarding);

	// A member chasing past the leash is sent back alone.
	State = EState::Fighting;
	TArray<FMember> Chasers = MakeMembers(2, Post);
	Chasers[1].Location = Post + FVector2D(S.LeashRadius + 1.f, 0.);
	Step = HOLCampRules::Step(State, S, Post, Chasers, MakeIntruder(Post, 100.f), false);
	TestTrue(TEXT("one member past the leash is sent back"), State == EState::Fighting && Step.ReturnIds.Num() == 1 && Step.ReturnIds[0] == Chasers[1].Id);

	// Only when attacked: the sight does not draw it, a blow from one it can reach does, a blow from beyond the leash does not.
	S.Aggro = EAggro::OnAttack;
	State = EState::Guarding;
	HOLCampRules::Step(State, S, Post, Members, MakeIntruder(Post, 100.f), false);
	TestTrue(TEXT("on attack: an intruder on the post, no blow: guarding"), State == EState::Guarding);
	HOLCampRules::Step(State, S, Post, Members, MakeIntruder(Post, S.LeashRadius + 1.f), true);
	TestTrue(TEXT("a blow from beyond the leash is not answered"), State == EState::Guarding);
	HOLCampRules::Step(State, S, Post, Members, MakeIntruder(Post, 100.f), true);
	TestTrue(TEXT("a blow draws it"), State == EState::Fighting);

	// Cleared for good: no member left = cleared once; members listed later do not bring it back.
	State = EState::Fighting;
	Step = HOLCampRules::Step(State, S, Post, TArray<FMember>(), TArray<FIntruder>(), false);
	TestTrue(TEXT("no member left: cleared, once"), State == EState::Cleared && Step.bJustCleared);
	Step = HOLCampRules::Step(State, S, Post, Members, MakeIntruder(Post, 10.f), true);
	TestTrue(TEXT("cleared for good (D32)"), State == EState::Cleared && !Step.bChanged && !Step.bJustCleared);

	// The reward by credit rule: the shares always add up to the reward.
	const TArray<FHit> Hits = { FHit{ 1, 60 }, FHit{ 0, 30 }, FHit{ 1, 10 }, FHit{ 0, 10 } };
	S.RewardXP = 100;
	S.Credit = ECredit::LastBlow;
	TArray<FShare> Shares = Rewards(S, Hits);
	TestTrue(TEXT("last blow: all to the owner of the last hit"), Shares.Num() == 1 && Shares[0].Owner == 0 && Shares[0].XP == 100);
	S.Credit = ECredit::MostDamage;
	Shares = Rewards(S, Hits);
	TestTrue(TEXT("most damage: all to owner 1 (70 against 40)"), Shares.Num() == 1 && Shares[0].Owner == 1 && Shares[0].XP == 100);
	S.Credit = ECredit::Shared;
	Shares = Rewards(S, Hits);
	int32 Sum = 0;
	for (const FShare& Share : Shares)
	{
		Sum += Share.XP;
	}
	TestTrue(TEXT("shared: by the damage, owners ascending, adding up"), Shares.Num() == 2 && Shares[0].Owner == 0 && Shares[0].XP == 36 && Shares[1].XP == 64 && Sum == 100);
	TestEqual(TEXT("no hit, no share"), Rewards(S, TArray<FHit>()).Num(), 0);
	return true;
}

// N3: the neutral owner is out of the slots and hostile to every slot of any launch (D401, FHOLCampTable); no slot index reaches it.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLNeutralOwnerTest, "HeroesOfLiberty.Neutrals.Owner",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLNeutralOwnerTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("the first number out of the slots"), FHOLMatchConfig::NeutralOwner, FHOLMatchConfig::MaxSlots);
	TestTrue(TEXT("recognised"), FHOLMatchConfig::IsNeutralOwner(FHOLMatchConfig::NeutralOwner));
	TestFalse(TEXT("a slot is not neutral"), FHOLMatchConfig::IsNeutralOwner(FHOLMatchConfig::MaxSlots - 1));

	FHOLMatchConfig Config;
	Config.Normalize();
	for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		TestTrue(*FString::Printf(TEXT("slot %d is hostile to the neutral owner"), Slot), HOLVictoryRules::AreHostile(Config, Slot, FHOLMatchConfig::NeutralOwner));
	}
	// Teams: allies are one camp, the neutral owner is alone in its own.
	Config.Slots[0].State = EHOLSlotState::AI;
	Config.Slots[1].State = EHOLSlotState::AI;
	Config.Slots[0].Team = 1;
	Config.Slots[1].Team = 1;
	TestFalse(TEXT("allies are not hostile"), HOLVictoryRules::AreHostile(Config, 0, 1));
	TestTrue(TEXT("a team is hostile to the neutral owner"), HOLVictoryRules::AreHostile(Config, 1, FHOLMatchConfig::NeutralOwner));

	// The combat processor's two questions of the camps' statics: nobody is passive until a camp says so, and a hit with no camp is harmless.
	TestFalse(TEXT("no passive unit by default"), HOLCamps::IsPassive(123456));
	HOLCamps::NoteHit(123456, 0, 10.f);
	HOLCamps::NoteHit(123456, FHOLMatchConfig::NeutralOwner, 10.f);
	return true;
}

#endif
