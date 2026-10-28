// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLForumMessages.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// V4.1 checks (V4-Q1, V4-Q3, V4-Q4): the message of each event of the ruin, as each viewer sees it.
namespace UE::HeroesOfLiberty::Tests::ForumMessages
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** Slots 0 and 1 in team 1, slot 2 in team 2, every other slot closed. */
	FHOLMatchConfig MakeTeams()
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (FHOLMatchSlot& Slot : Config.Slots)
		{
			Slot.State = EHOLSlotState::Closed;
			Slot.Team = 0;
		}
		Config.Slots[0].State = EHOLSlotState::Human;
		Config.Slots[0].Team = 1;
		Config.Slots[1].State = EHOLSlotState::AI;
		Config.Slots[1].Team = 1;
		Config.Slots[2].State = EHOLSlotState::AI;
		Config.Slots[2].Team = 2;
		return Config;
	}

	FString TextOf(const FHOLForumMessage& Message)
	{
		return Message.Text.BuildSourceString();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryMessagesFellTest, "HeroesOfLiberty.Victory.Messages.Fell", UE::HeroesOfLiberty::Tests::ForumMessages::Flags)
bool FHOLVictoryMessagesFellTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ForumMessages;
	const FHOLMatchConfig Teams = MakeTeams();
	const FHOLForumEvent Fell = FHOLForumEvent::Make(EHOLForumEventKind::Fell, 0, INDEX_NONE, 1);

	FHOLForumMessage Message = HOLForumMessages::GetMessage(Fell, Teams, 0);
	TestEqual(TEXT("own: text"), TextOf(Message), FString(TEXT("Votre forum est tombé — 1 vie")));
	TestTrue(TEXT("own: alert"), Message.bAlert);
	TestTrue(TEXT("own: logged with (alert)"), Message.Shown.StartsWith(TEXT("Message: ")) && Message.Shown.EndsWith(TEXT(" (alert)")));

	Message = HOLForumMessages::GetMessage(Fell, Teams, 1);
	TestEqual(TEXT("ally: text"), TextOf(Message), FString(TEXT("Le forum de Joueur 1 est tombé — 1 vie")));
	TestTrue(TEXT("ally: alert"), Message.bAlert);
	TestEqual(TEXT("ally: the slot named"), Message.NamedSlots, TArray<int32>({ 0 }));

	Message = HOLForumMessages::GetMessage(Fell, Teams, 2);
	TestEqual(TEXT("adversary: text"), TextOf(Message), FString(TEXT("Le forum de Joueur 1 est tombé — 1 vie")));
	TestFalse(TEXT("adversary: no alert"), Message.bAlert);
	TestTrue(TEXT("adversary: logged without (alert)"), Message.Shown.StartsWith(TEXT("Message: ")) && !Message.Shown.EndsWith(TEXT("(alert)")));

	Message = HOLForumMessages::GetMessage(Fell, Teams, INDEX_NONE);
	TestEqual(TEXT("spectator: text"), TextOf(Message), FString(TEXT("Le forum de Joueur 1 est tombé — 1 vie")));
	TestFalse(TEXT("spectator: no alert"), Message.bAlert);

	// V4.5b: its last life gone, its own loss: no alert, the thread's line alone; for its ally, the alert stays.
	const FHOLForumEvent Last = FHOLForumEvent::Make(EHOLForumEventKind::Fell, 0, INDEX_NONE, 0);
	Message = HOLForumMessages::GetMessage(Last, Teams, 0);
	TestFalse(TEXT("own, 0 vies: no alert (its loss)"), Message.bAlert);
	TestEqual(TEXT("own, 0 vies: logged without (alert)"), Message.Shown, FString(TEXT("Message: Votre forum est tombé — 0 vies")));
	TestTrue(TEXT("ally, 0 vies: alert"), HOLForumMessages::GetMessage(Last, Teams, 1).bAlert);

	// The singular for exactly 1, the plural for 0 and 2 (as the players' table).
	TestEqual(TEXT("0 vies"), TextOf(HOLForumMessages::GetMessage(FHOLForumEvent::Make(EHOLForumEventKind::Fell, 0, INDEX_NONE, 0), Teams, 2)),
		FString(TEXT("Le forum de Joueur 1 est tombé — 0 vies")));
	TestEqual(TEXT("2 vies, own"), TextOf(HOLForumMessages::GetMessage(FHOLForumEvent::Make(EHOLForumEventKind::Fell, 0, INDEX_NONE, 2), Teams, 0)),
		FString(TEXT("Votre forum est tombé — 2 vies")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryMessagesHoldTest, "HeroesOfLiberty.Victory.Messages.HoldStarted", UE::HeroesOfLiberty::Tests::ForumMessages::Flags)
bool FHOLVictoryMessagesHoldTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ForumMessages;
	const FHOLMatchConfig Teams = MakeTeams();
	// Slot 2 holds the ruin of slot 0.
	const FHOLForumEvent Hold = FHOLForumEvent::Make(EHOLForumEventKind::HoldStarted, 0, 2);

	FHOLForumMessage Message = HOLForumMessages::GetMessage(Hold, Teams, 0);
	TestEqual(TEXT("own: text"), TextOf(Message), FString(TEXT("IA Normal 3 tient votre ruine")));
	TestTrue(TEXT("own: alert"), Message.bAlert);
	TestEqual(TEXT("own: the attacker named"), Message.NamedSlots, TArray<int32>({ 2 }));

	Message = HOLForumMessages::GetMessage(Hold, Teams, 1);
	TestEqual(TEXT("ally: text"), TextOf(Message), FString(TEXT("IA Normal 3 tient la ruine de Joueur 1")));
	TestTrue(TEXT("ally: alert"), Message.bAlert);
	TestEqual(TEXT("ally: both named, in order"), Message.NamedSlots, TArray<int32>({ 2, 0 }));

	Message = HOLForumMessages::GetMessage(Hold, Teams, 2);
	TestEqual(TEXT("the attacker: text"), TextOf(Message), FString(TEXT("Vous tenez la ruine de Joueur 1")));
	TestFalse(TEXT("the attacker: no alert"), Message.bAlert);

	Message = HOLForumMessages::GetMessage(Hold, Teams, INDEX_NONE);
	TestEqual(TEXT("spectator: text"), TextOf(Message), FString(TEXT("IA Normal 3 tient la ruine de Joueur 1")));
	TestFalse(TEXT("spectator: no alert"), Message.bAlert);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryMessagesRazedTest, "HeroesOfLiberty.Victory.Messages.Razed", UE::HeroesOfLiberty::Tests::ForumMessages::Flags)
bool FHOLVictoryMessagesRazedTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ForumMessages;
	const FHOLMatchConfig Teams = MakeTeams();
	const FHOLForumEvent Razed = FHOLForumEvent::Make(EHOLForumEventKind::Razed, 2, INDEX_NONE, 0);

	FHOLForumMessage Message = HOLForumMessages::GetMessage(Razed, Teams, 2);
	TestEqual(TEXT("own: text"), TextOf(Message), FString(TEXT("Vos fondations sont rasées — 0 vies")));
	// V4.5b: its last life razed, its own loss: the thread's line alone.
	TestFalse(TEXT("own, at 0 lives: no alert (its loss)"), Message.bAlert);
	TestEqual(TEXT("own, at 0 lives: logged without (alert)"), Message.Shown, FString(TEXT("Message: ")) + Message.Text.ToString());
	TestTrue(TEXT("own, at 1 life: alert"), HOLForumMessages::GetMessage(FHOLForumEvent::Make(EHOLForumEventKind::Razed, 2, INDEX_NONE, 1), Teams, 2).bAlert);

	Message = HOLForumMessages::GetMessage(Razed, Teams, 0);
	TestEqual(TEXT("adversary: text"), TextOf(Message), FString(TEXT("Fondations de IA Normal 3 rasées — 0 vies")));
	TestFalse(TEXT("adversary: no alert"), Message.bAlert);

	Message = HOLForumMessages::GetMessage(FHOLForumEvent::Make(EHOLForumEventKind::Razed, 2, INDEX_NONE, 1), Teams, INDEX_NONE);
	TestEqual(TEXT("spectator: 1 vie"), TextOf(Message), FString(TEXT("Fondations de IA Normal 3 rasées — 1 vie")));
	TestFalse(TEXT("spectator: no alert"), Message.bAlert);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryMessagesDefeatedTest, "HeroesOfLiberty.Victory.Messages.Defeated", UE::HeroesOfLiberty::Tests::ForumMessages::Flags)
bool FHOLVictoryMessagesDefeatedTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ForumMessages;
	const FHOLMatchConfig Teams = MakeTeams();
	const FHOLForumEvent Defeated = FHOLForumEvent::Make(EHOLForumEventKind::Defeated, 1);

	FHOLForumMessage Message = HOLForumMessages::GetMessage(Defeated, Teams, 1);
	TestEqual(TEXT("own: text"), TextOf(Message), FString(TEXT("Vous êtes vaincu")));
	// V4.5b: its own defeat, the thread's line alone (its Defeat banner comes), whatever the replication's order.
	TestFalse(TEXT("own: no alert (its loss)"), Message.bAlert);
	TestEqual(TEXT("own: logged without (alert)"), Message.Shown, FString(TEXT("Message: Vous êtes vaincu")));
	TestTrue(TEXT("own: nobody else named"), Message.NamedSlots.IsEmpty() || Message.NamedSlots == TArray<int32>({ 1 }));

	Message = HOLForumMessages::GetMessage(Defeated, Teams, 0);
	TestEqual(TEXT("ally: text"), TextOf(Message), FString(TEXT("IA Normal 2 est vaincu")));
	TestTrue(TEXT("ally: alert"), Message.bAlert);

	Message = HOLForumMessages::GetMessage(Defeated, Teams, 2);
	TestEqual(TEXT("adversary: text"), TextOf(Message), FString(TEXT("IA Normal 2 est vaincu")));
	TestFalse(TEXT("adversary: no alert"), Message.bAlert);

	Message = HOLForumMessages::GetMessage(Defeated, Teams, INDEX_NONE);
	TestFalse(TEXT("spectator: no alert"), Message.bAlert);
	TestEqual(TEXT("spectator: logged"), Message.Shown, FString(TEXT("Message: ")) + Message.Text.ToString());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryMessagesHoldAnnouncedTest, "HeroesOfLiberty.Victory.Messages.HoldAnnounced", UE::HeroesOfLiberty::Tests::ForumMessages::Flags)
bool FHOLVictoryMessagesHoldAnnouncedTest::RunTest(const FString& Parameters)
{
	// V4.2: HoldStarted once per attacker reaching Holding since the count was at 0.
	const auto Hold = [](const EHOLForumHold Kind, const int32 Holder, const int32 Steps)
	{
		FHOLForumState State;
		State.Phase = EHOLForumPhase::Ruin;
		State.Hold = Kind;
		State.HolderSlot = static_cast<int8>(Holder);
		State.HoldSteps = Steps;
		return State;
	};
	uint8 Announced = 0;
	// None -> Contested -> Holding: the attacker killed the defender, its hold announced.
	TestFalse(TEXT("contested at 0: no message"), HOLForumMessages::ShouldAnnounceHold(Hold(EHOLForumHold::Contested, 2, 0), Announced));
	TestTrue(TEXT("then holding: a message"), HOLForumMessages::ShouldAnnounceHold(Hold(EHOLForumHold::Holding, 2, 1), Announced));
	TestFalse(TEXT("holding on: no second message"), HOLForumMessages::ShouldAnnounceHold(Hold(EHOLForumHold::Holding, 2, 2), Announced));
	// Contested, receding, then the same attacker again: no new message.
	TestFalse(TEXT("contested"), HOLForumMessages::ShouldAnnounceHold(Hold(EHOLForumHold::Contested, 2, 2), Announced));
	TestFalse(TEXT("the same again after a contest"), HOLForumMessages::ShouldAnnounceHold(Hold(EHOLForumHold::Holding, 2, 3), Announced));
	TestFalse(TEXT("receding"), HOLForumMessages::ShouldAnnounceHold(Hold(EHOLForumHold::Receding, 2, 2), Announced));
	TestFalse(TEXT("the same again after a recession"), HOLForumMessages::ShouldAnnounceHold(Hold(EHOLForumHold::Holding, 2, 3), Announced));
	// Another attacker takes the lead: a message; the first back: none (already announced).
	TestTrue(TEXT("another attacker"), HOLForumMessages::ShouldAnnounceHold(Hold(EHOLForumHold::Holding, 1, 4), Announced));
	TestFalse(TEXT("the first back"), HOLForumMessages::ShouldAnnounceHold(Hold(EHOLForumHold::Holding, 2, 5), Announced));
	// Back to 0 (None): forgotten, the same attacker announced anew.
	TestFalse(TEXT("none"), HOLForumMessages::ShouldAnnounceHold(Hold(EHOLForumHold::None, INDEX_NONE, 0), Announced));
	TestEqual(TEXT("forgotten at 0"), static_cast<int32>(Announced), 0);
	TestTrue(TEXT("anew from 0"), HOLForumMessages::ShouldAnnounceHold(Hold(EHOLForumHold::Holding, 2, 1), Announced));
	// No attacker known: no message.
	Announced = 0;
	TestFalse(TEXT("no holder"), HOLForumMessages::ShouldAnnounceHold(Hold(EHOLForumHold::Holding, INDEX_NONE, 1), Announced));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryMessagesThreadTest, "HeroesOfLiberty.Victory.Messages.Thread", UE::HeroesOfLiberty::Tests::ForumMessages::Flags)
bool FHOLVictoryMessagesThreadTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ForumMessages;
	const FHOLMatchConfig Teams = MakeTeams();
	const FHOLForumMessage Fell = HOLForumMessages::GetMessage(FHOLForumEvent::Make(EHOLForumEventKind::Fell, 0, INDEX_NONE, 1), Teams, 0);
	const FHOLForumMessage Defeated = HOLForumMessages::GetMessage(FHOLForumEvent::Make(EHOLForumEventKind::Defeated, 1), Teams, 0);

	FHOLForumThread Thread;
	TestEqual(TEXT("empty"), Thread.Describe(), FString(TEXT("[]")));
	Thread.Add(Fell, 10.0);
	Thread.Add(Defeated, 12.0);
	if (!TestEqual(TEXT("two lines"), Thread.Lines.Num(), 2))
	{
		return false;
	}
	TestEqual(TEXT("the newest at the bottom"), Thread.Describe(), FString(TEXT("[Votre forum est tombé — 1 vie | IA Normal 2 est vaincu]")));
	TestEqual(TEXT("opaque at first"), FHOLForumThread::GetOpacity(Thread.Lines[0], 10.0), 1.f);
	TestTrue(TEXT("fading near its end"), FHOLForumThread::GetOpacity(Thread.Lines[0], 17.75) < 1.f && FHOLForumThread::GetOpacity(Thread.Lines[0], 17.75) > 0.f);
	TestFalse(TEXT("nothing gone before 8 s"), Thread.Expire(17.9));
	TestTrue(TEXT("the first gone at 8 s"), Thread.Expire(18.0));
	TestEqual(TEXT("the second left"), Thread.Describe(), FString(TEXT("[IA Normal 2 est vaincu]")));
	TestTrue(TEXT("the second gone at 20 s"), Thread.Expire(20.0));
	TestEqual(TEXT("empty again"), Thread.Lines.Num(), 0);

	// At most 5 lines: the oldest dropped.
	for (int32 Index = 0; Index < 7; ++Index)
	{
		Thread.Add(Index == 6 ? Defeated : Fell, 30.0 + Index);
	}
	TestEqual(TEXT("5 lines at most"), Thread.Lines.Num(), FHOLForumThread::MaxLines);
	if (Thread.Lines.Num() != FHOLForumThread::MaxLines)
	{
		return false;
	}
	TestEqual(TEXT("the oldest dropped"), Thread.Lines[0].AddedSeconds, 32.0);
	TestEqual(TEXT("the newest last"), Thread.Lines.Last().Message.Text.ToString(), Defeated.Text.ToString());

	// The names cut out for their colours.
	const FHOLForumMessage Held = HOLForumMessages::GetMessage(FHOLForumEvent::Make(EHOLForumEventKind::HoldStarted, 0, 2), Teams, 1);
	const TArray<HOLForumMessages::FPiece> Pieces = HOLForumMessages::SplitNames(Held, 1, Teams);
	TestEqual(TEXT("pieces"), Pieces.Num(), 3);
	if (Pieces.Num() == 3)
	{
		TestEqual(TEXT("attacker"), Pieces[0].Text, FString(TEXT("IA Normal 3")));
		TestEqual(TEXT("attacker slot"), Pieces[0].Slot, 2);
		TestEqual(TEXT("words"), Pieces[1].Text, FString(TEXT(" tient la ruine de ")));
		TestEqual(TEXT("words slot"), Pieces[1].Slot, static_cast<int32>(INDEX_NONE));
		TestEqual(TEXT("owner"), Pieces[2].Text, FString(TEXT("Joueur 1")));
		TestEqual(TEXT("owner slot"), Pieces[2].Slot, 0);
	}
	const TArray<HOLForumMessages::FPiece> Own = HOLForumMessages::SplitNames(Fell, 0);
	TestTrue(TEXT("no name: one piece"), Own.Num() == 1 && Own[0].Slot == INDEX_NONE);
	return true;
}

// V4.3: the major alert's banner: only for an alert, 4 s then gone, a new one replacing it, none once the match is over for me.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVictoryMessagesAlertTest, "HeroesOfLiberty.Victory.Messages.Alert", UE::HeroesOfLiberty::Tests::ForumMessages::Flags)
bool FHOLVictoryMessagesAlertTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ForumMessages;

	const FHOLMatchConfig Teams = MakeTeams();
	// Slot 0 sees its own forum fall (an alert), slot 2's (an adversary's: no alert).
	const FHOLForumMessage Own = HOLForumMessages::GetMessage(FHOLForumEvent::Make(EHOLForumEventKind::Fell, 0, INDEX_NONE, 1), Teams, 0);
	const FHOLForumMessage Ally = HOLForumMessages::GetMessage(FHOLForumEvent::Make(EHOLForumEventKind::Fell, 1, INDEX_NONE, 1), Teams, 0);
	const FHOLForumMessage Adversary = HOLForumMessages::GetMessage(FHOLForumEvent::Make(EHOLForumEventKind::Fell, 2, INDEX_NONE, 1), Teams, 0);
	FHOLForumAlert Alert;
	TestEqual(TEXT("none at first"), Alert.Describe(), FString(TEXT("none")));
	TestFalse(TEXT("an adversary's forum: no banner"), Alert.Show(Adversary, 2, false, 10.0));
	TestEqual(TEXT("still none"), Alert.Describe(), FString(TEXT("none")));
	TestTrue(TEXT("my forum: the banner"), Alert.Show(Own, 0, false, 10.0));
	TestEqual(TEXT("its text and slot"), Alert.Describe(), FString(TEXT("Votre forum est tombé — 1 vie (slot 0)")));
	TestEqual(TEXT("opaque at first"), Alert.GetOpacity(10.0), 1.f);
	TestTrue(TEXT("fading near its end"), Alert.GetOpacity(13.75) > 0.f && Alert.GetOpacity(13.75) < 1.f);
	TestFalse(TEXT("still there before 4 s"), Alert.Expire(13.9));
	TestTrue(TEXT("gone at 4 s"), Alert.Expire(14.0));
	TestEqual(TEXT("none again"), Alert.Describe(), FString(TEXT("none")));
	TestEqual(TEXT("hidden: transparent"), Alert.GetOpacity(14.0), 0.f);
	TestFalse(TEXT("gone once only"), Alert.Expire(15.0));

	// A new alert replaces the shown one, its 4 s from its own start.
	TestTrue(TEXT("mine"), Alert.Show(Own, 0, false, 20.0));
	TestTrue(TEXT("my ally's replaces it"), Alert.Show(Ally, 1, false, 22.0));
	TestEqual(TEXT("the ally's slot"), Alert.Slot, 1);
	TestFalse(TEXT("not gone at the first one's 4 s"), Alert.Expire(24.5));
	TestTrue(TEXT("gone at its own 4 s"), Alert.Expire(26.0));

	// The match over for me (my Victory / Defeat banner): the thread's line is enough.
	TestFalse(TEXT("match over for me: no banner"), Alert.Show(Own, 0, true, 30.0));
	TestTrue(TEXT("shown, then the match over for me: kept, its 4 s"), Alert.Show(Own, 0, false, 40.0) && !Alert.Show(Ally, 1, true, 41.0) && Alert.Slot == 0);

	// V4.5b: a banner kept 1.5 s at least; one come sooner waits, then is shown its 4 s; one sound for a burst.
	FHOLForumAlert Burst;
	TestTrue(TEXT("burst: the fall shown"), Burst.Show(Own, 0, false, 50.0) && Burst.bSound);
	TestFalse(TEXT("burst: the hold 40 ms later waits"), Burst.Show(Ally, 1, false, 50.04));
	TestEqual(TEXT("burst: the fall still shown"), Burst.Slot, 0);
	TestFalse(TEXT("burst: still waiting at 1.4 s"), Burst.Advance(51.4, false));
	TestFalse(TEXT("burst: not hidden while one waits"), Burst.Expire(51.4));
	TestTrue(TEXT("burst: the hold shown at 1.5 s"), Burst.Advance(51.5, false));
	TestEqual(TEXT("burst: the hold's slot"), Burst.Slot, 1);
	TestFalse(TEXT("burst: the hold silent (came within 1 s)"), Burst.bSound);
	TestFalse(TEXT("burst: the hold its 4 s"), Burst.Expire(55.4));
	TestTrue(TEXT("burst: gone at its 4 s"), Burst.Expire(55.5));
	// Three more within 1 s: at most 2 waiting, the oldest dropped, one sound.
	TestTrue(TEXT("burst 2: shown"), Burst.Show(Own, 0, false, 60.0) && Burst.bSound);
	Burst.Show(Ally, 1, false, 60.1);
	Burst.Show(Own, 3, false, 60.2);
	Burst.Show(Ally, 4, false, 60.3);
	TestEqual(TEXT("burst 2: 2 waiting"), Burst.Waiting.Num(), FHOLForumAlert::MaxWaiting);
	TestTrue(TEXT("burst 2: the next at 1.5 s, the oldest waiting dropped"), Burst.Advance(61.5, false) && Burst.Slot == 3 && !Burst.bSound);
	TestFalse(TEXT("burst 2: the last waits 1.5 s more"), Burst.Advance(62.9, false));
	TestTrue(TEXT("burst 2: the last at 3 s"), Burst.Advance(63.0, false) && Burst.Slot == 4 && !Burst.bSound);
	// More than 1 s after: its own sound.
	TestFalse(TEXT("later: waits"), Burst.Show(Own, 0, false, 64.2));
	TestTrue(TEXT("later: shown with its sound"), Burst.Advance(64.5, false) && Burst.bSound);
	// The match over for me: the waiting ones dropped.
	Burst.Show(Ally, 1, false, 64.6);
	TestFalse(TEXT("match over: nothing more shown"), Burst.Advance(66.5, true));
	TestEqual(TEXT("match over: none waiting"), Burst.Waiting.Num(), 0);

	// A spectator is never alerted.
	const FHOLForumMessage Spectator = HOLForumMessages::GetMessage(FHOLForumEvent::Make(EHOLForumEventKind::Fell, 0, INDEX_NONE, 1), Teams, INDEX_NONE);
	FHOLForumAlert Watching;
	TestFalse(TEXT("spectator: no banner"), Watching.Show(Spectator, 0, false, 10.0));
	return true;
}

#endif
