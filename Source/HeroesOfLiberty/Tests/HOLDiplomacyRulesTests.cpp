// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLDiplomacyRules.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// D4 (Diplomacy, GDD 14.3): the stances where the simulation and the network run, on the same cases as the script test of
// Script/Diplomacy/HOLDiplomacyRules_Test.as: the two rules must agree. D369 (one-sided stances, allied only if both), D375 (the most
// hostile of the two applies), D370 (the break is immediate), D371 (vision a box of its own, cut at the break), D374 (garrison,
// ramparts), D401 (Neutral = adversary that does not fire), D367 (AIs always Enemy), D373 (allied victory).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDiplomacyRulesTest, "HeroesOfLiberty.Diplomacy.Rules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLDiplomacyRulesTest::RunTest(const FString& Parameters)
{
	using namespace HOLDiplomacy;
	const auto Humans = [](const int32 Slots)
	{
		TArray<bool> NoAI;
		NoAI.Init(false, Slots);
		return NewBook(Slots, NoAI);
	};
	const auto MakeAllies = [](FBook& Book, const int32 A, const int32 B)
	{
		SetStance(Book, A, B, Ally);
		SetStance(Book, B, A, Ally);
	};

	// The default stance, the order of the stances, oneself.
	FBook Book = Humans(4);
	TestEqual(TEXT("four slots"), Book.Slots, 4);
	TestEqual(TEXT("default relation"), Relation(Book, 0, 3), DefaultStance);
	TestEqual(TEXT("oneself is an ally"), Relation(Book, 2, 2), Ally);
	TestTrue(TEXT("Enemy < Neutral < Ally"), Enemy < Neutral && Neutral < Ally);

	// Allied only if both say Ally; the most hostile of the two applies (D375), whoever is asked.
	FBook Pair = Humans(2);
	const FStanceChange Offer = SetStance(Pair, 0, 1, Ally);
	TestTrue(TEXT("an offer is allowed"), (Offer.Result) == EResult::Ok);
	TestFalse(TEXT("one side is not an alliance"), IsAlly(Pair, 0, 1) || IsAlly(Pair, 1, 0));
	TestFalse(TEXT("nothing formed yet"), Offer.bFormed);
	TestTrue(TEXT("the second says Ally: formed"), SetStance(Pair, 1, 0, Ally).bFormed);
	TestTrue(TEXT("allied both ways"), IsAlly(Pair, 0, 1) && IsAlly(Pair, 1, 0));
	for (int32 A = Enemy; A <= Ally; ++A)
	{
		for (int32 B = Enemy; B <= Ally; ++B)
		{
			FBook Mixed = Humans(2);
			SetStance(Mixed, 0, 1, A);
			SetStance(Mixed, 1, 0, B);
			TestEqual(TEXT("the most hostile applies (0 to 1)"), Relation(Mixed, 0, 1), FMath::Min(A, B));
			TestEqual(TEXT("and to both (1 to 0)"), Relation(Mixed, 1, 0), FMath::Min(A, B));
		}
	}

	// The break is immediate, straight to Enemy, and says so (D370); it also cuts the vision and does not give it back (D371).
	TestTrue(TEXT("vision opens between allies"), SetVision(Pair, 0, 1, true).bAnnounce);
	TestTrue(TEXT("1 sees what 0 sees"), SharesVision(Pair, 0, 1));
	TestFalse(TEXT("one-sided"), SharesVision(Pair, 1, 0));
	TestFalse(TEXT("a repeat is not announced"), SetVision(Pair, 0, 1, true).bAnnounce);
	const FStanceChange Betrayal = SetStance(Pair, 1, 0, Enemy);
	TestTrue(TEXT("it broke an alliance"), Betrayal.bBroke && !Betrayal.bFormed);
	TestEqual(TEXT("before"), Betrayal.Before, Ally);
	TestEqual(TEXT("straight to Enemy"), Betrayal.After, Enemy);
	TestTrue(TEXT("both fire at once"), FiresAtOnce(Pair, 0, 1) && FiresAtOnce(Pair, 1, 0));
	TestFalse(TEXT("the vision is cut at that second"), SharesVision(Pair, 0, 1));
	TestEqual(TEXT("the betrayed one's stance is not touched"), StanceOf(Pair, 0, 1), Ally);
	MakeAllies(Pair, 0, 1);
	TestTrue(TEXT("allied again"), IsAlly(Pair, 0, 1));
	TestFalse(TEXT("the vision must be opened again"), SharesVision(Pair, 0, 1));
	TestFalse(TEXT("a repeat of a stance is not announced"), SetStance(Pair, 0, 1, Ally).bAnnounce);

	// Neutral: an adversary that does not fire (D401); an ally is no adversary; nobody is their own adversary.
	FBook Calm = Humans(4);
	MakeAllies(Calm, 0, 2);
	SetStance(Calm, 0, 1, Neutral);
	SetStance(Calm, 1, 0, Neutral);
	TestFalse(TEXT("Neutral does not fire"), FiresAtOnce(Calm, 0, 1));
	TestTrue(TEXT("Neutral is an adversary"), IsAdversary(Calm, 0, 1) && IsAdversary(Calm, 1, 0));
	TestFalse(TEXT("an ally is no adversary"), IsAdversary(Calm, 0, 2) || FiresAtOnce(Calm, 0, 2));
	TestTrue(TEXT("Enemy fires"), FiresAtOnce(Calm, 0, 3) && IsAdversary(Calm, 0, 3));
	TestFalse(TEXT("oneself and outside the match: nobody"), IsAdversary(Calm, 0, 0) || IsAdversary(Calm, 0, 9) || FiresAtOnce(Calm, 0, 9));

	// The refusals, in order; a refusal changes nothing.
	FBook Three = Humans(3);
	TestTrue(TEXT("sender beyond the slots"), (SetStance(Three, 3, 1, Ally).Result) == EResult::NoSuchSlot);
	TestTrue(TEXT("negative target"), (SetStance(Three, 0, -1, Ally).Result) == EResult::NoSuchSlot);
	TestTrue(TEXT("toward oneself"), (SetStance(Three, 1, 1, Ally).Result) == EResult::Self);
	TestTrue(TEXT("no such stance"), (SetStance(Three, 0, 1, 3).Result) == EResult::BadStance);
	TestTrue(TEXT("the slots come first"), (SetStance(Three, 7, 7, 9).Result) == EResult::NoSuchSlot);
	TestTrue(TEXT("then oneself"), (SetStance(Three, 1, 1, 9).Result) == EResult::Self);
	TestFalse(TEXT("a refusal is not announced"), SetStance(Three, 0, 1, 3).bAnnounce);
	TestEqual(TEXT("and changes nothing"), StanceOf(Three, 0, 1), DefaultStance);
	MakeAllies(Three, 0, 1);
	SetStance(Three, 0, 2, Ally);
	TestTrue(TEXT("vision: an offer not accepted is not an alliance"), (SetVision(Three, 0, 2, true).Result) == EResult::NotAlly);
	TestTrue(TEXT("vision: toward oneself"), (SetVision(Three, 1, 1, true).Result) == EResult::Self);
	// The AIs are alone against all (D367): Enemy to everybody, they take no stance, no vision.
	TArray<bool> IsAI = { false, false, true, true };
	FBook WithAI = NewBook(4, IsAI);
	TestTrue(TEXT("a human may say Ally to an AI"), (SetStance(WithAI, 0, 2, Ally).Result) == EResult::Ok);
	TestEqual(TEXT("it stays Enemy"), Relation(WithAI, 0, 2), Enemy);
	TestTrue(TEXT("and fires"), FiresAtOnce(WithAI, 2, 0));
	TestTrue(TEXT("an AI takes no stance"), (SetStance(WithAI, 2, 0, Ally).Result) == EResult::SenderIsAI);
	TestEqual(TEXT("two AIs are enemies"), Relation(WithAI, 2, 3), Enemy);
	TestTrue(TEXT("an AI shares nothing"), (SetVision(WithAI, 2, 0, true).Result) == EResult::SenderIsAI);
	// The standard modes: teams fixed, stances locked, teammates see together (an AI teammate included).
	const TArray<int32> Teams = { 1, 1, 2, 0, 0 };
	const TArray<bool> TeamAI = { false, true, false, false, true };
	FBook Locked = NewTeamBook(Teams, TeamAI);
	TestTrue(TEXT("locked"), Locked.bLocked);
	TestTrue(TEXT("same team: allies"), IsAlly(Locked, 0, 1) && IsAlly(Locked, 1, 0));
	TestTrue(TEXT("other teams: enemies"), FiresAtOnce(Locked, 2, 1) && IsAdversary(Locked, 0, 2));
	TestTrue(TEXT("team 0 is alone"), FiresAtOnce(Locked, 3, 4) && !IsAlly(Locked, 3, 4));
	TestTrue(TEXT("no stance can be set"), (SetStance(Locked, 0, 2, Ally).Result) == EResult::Locked);
	TestTrue(TEXT("teammates see together"), SharesVision(Locked, 0, 1) && SharesVision(Locked, 1, 0));
	TestFalse(TEXT("not the other team"), SharesVision(Locked, 0, 2));
	TestTrue(TEXT("no box to tick"), (SetVision(Locked, 0, 1, false).Result) == EResult::Locked);
	// Garrison and ramparts (D374).
	FBook Trio = Humans(3);
	MakeAllies(Trio, 0, 1);
	TestTrue(TEXT("garrison in one's own"), CanGarrison(0, 0));
	TestFalse(TEXT("never in an ally's"), CanGarrison(0, 1));
	TestTrue(TEXT("an ally's doors are open"), DoorsOpenFor(Trio, 1, 0) && DoorsOpenFor(Trio, 0, 1));
	TestFalse(TEXT("a stranger's are not"), DoorsOpenFor(Trio, 1, 2));
	TestTrue(TEXT("allied: down when it wants"), CanLeaveRampart(Trio, 0, 1));
	SetStance(Trio, 1, 0, Enemy);
	TestFalse(TEXT("after the break: it stays there"), CanLeaveRampart(Trio, 0, 1));
	TestTrue(TEXT("the owner's own units come and go"), CanLeaveRampart(Trio, 1, 1));

	// The allied victory (D373).
	FBook Four = Humans(4);
	const TArray<bool> OneDown = { false, false, false, true };
	TestFalse(TEXT("three enemy survivors: no victory"), AlliedVictory(Four, OneDown).bEnded);
	MakeAllies(Four, 0, 1);
	MakeAllies(Four, 1, 2);
	TestFalse(TEXT("no chain: 0-2 not allied"), AlliedVictory(Four, OneDown).bEnded);
	MakeAllies(Four, 0, 2);
	const FAlliedVictory Win = AlliedVictory(Four, OneDown);
	TestTrue(TEXT("all survivors allied, one beaten: victory"), Win.bEnded);
	TestTrue(TEXT("the three win together"), (Win.Winners) == TArray<int32>({ 0, 1, 2 }));
	SetStance(Four, 2, 0, Enemy);
	TestFalse(TEXT("a break: not all allied any more"), AlliedVictory(Four, OneDown).bEnded);
	FBook Allies = Humans(2);
	MakeAllies(Allies, 0, 1);
	TestFalse(TEXT("nobody beaten: it goes on"), AlliedVictory(Allies, { false, false }).bEnded);
	TestTrue(TEXT("the last standing wins"), (AlliedVictory(Allies, { false, true }).Winners) == TArray<int32>({ 0 }));
	TestFalse(TEXT("nobody left: not an allied victory"), AlliedVictory(Allies, { true, true }).bEnded);
	TestTrue(TEXT("a list shorter than the slots: the missing are alive"), (AlliedVictory(Allies, { true }).Winners) == TArray<int32>({ 1 }));
	// An AI alive blocks it (D367); teams give the team victory.
	FBook AIGame = NewBook(4, { false, false, true, false });
	MakeAllies(AIGame, 0, 1);
	TestFalse(TEXT("an AI alive: no victory"), AlliedVictory(AIGame, { false, false, false, true }).bEnded);
	TestTrue(TEXT("the AI eliminated: both win"), (AlliedVictory(AIGame, { false, false, true, true }).Winners) == TArray<int32>({ 0, 1 }));
	FBook TeamGame = NewTeamBook({ 1, 1, 2, 2 }, { false, false, false, false });
	TestFalse(TEXT("one of each team alive: it goes on"), AlliedVictory(TeamGame, { false, true, true, false }).bEnded);
	TestTrue(TEXT("only team 1 left"), AlliedVictory(TeamGame, { false, true, true, true }).bEnded);

	return true;
}

#endif
