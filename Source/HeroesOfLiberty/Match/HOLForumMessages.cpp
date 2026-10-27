// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLForumMessages.h"

#include "Match/HOLForumRules.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLVictoryRules.h"

FHOLForumEvent FHOLForumEvent::Make(const EHOLForumEventKind Kind, const int32 Slot, const int32 OtherSlot, const int32 Lives)
{
	FHOLForumEvent Event;
	Event.Kind = Kind;
	Event.Slot = static_cast<int8>(Slot);
	Event.OtherSlot = static_cast<int8>(OtherSlot);
	Event.Lives = static_cast<int8>(FMath::Clamp(Lives, 0, 127));
	return Event;
}

#define LOCTEXT_NAMESPACE "HOLGameScreen"

FText HOLForumMessages::GetSlotName(const int32 Slot, const int32 ViewerSlot)
{
	return Slot == ViewerSlot ? LOCTEXT("You", "Vous") : FText::Format(LOCTEXT("PlayerN", "Joueur {0}"), Slot + 1);
}

FText HOLForumMessages::GetDifficultyName(const EHOLAIDifficulty Difficulty)
{
	switch (Difficulty)
	{
	case EHOLAIDifficulty::Easy: return LOCTEXT("AIEasy", "Facile");
	case EHOLAIDifficulty::Hard: return LOCTEXT("AIHard", "Difficile");
	case EHOLAIDifficulty::Evolving: return LOCTEXT("AIEvolving", "Évolutive");
	default: return LOCTEXT("AINormal", "Normal");
	}
}

FText HOLForumMessages::GetAIName(const int32 Slot, const EHOLAIDifficulty Difficulty)
{
	return FText::Format(LOCTEXT("AIName", "IA {0} {1}"), GetDifficultyName(Difficulty), Slot + 1);
}

FText HOLForumMessages::GetSlotName(const int32 Slot, const int32 ViewerSlot, const FHOLMatchConfig& Config)
{
	// IA0.1-Q1: an AI slot (no lobby name) « IA <level> N »; the viewer, a human, a slot out of Config as before.
	if (Slot != ViewerSlot && Config.Slots.IsValidIndex(Slot) && Config.Slots[Slot].State == EHOLSlotState::AI)
	{
		return GetAIName(Slot, Config.Slots[Slot].Difficulty);
	}
	return GetSlotName(Slot, ViewerSlot);
}

FText HOLForumMessages::GetPlayerName(const FHOLMatchConfig& Config, const int32 Slot, const int32 ViewerSlot)
{
	if (Slot == ViewerSlot)
	{
		return LOCTEXT("You", "Vous");
	}
	const FString Name = Config.Slots.IsValidIndex(Slot) ? Config.Slots[Slot].PlayerName.TrimStartAndEnd() : FString();
	return !Name.IsEmpty() ? FText::AsCultureInvariant(Name) : GetSlotName(Slot, ViewerSlot, Config);
}

FHOLForumMessage HOLForumMessages::GetMessage(const FHOLForumEvent& Event, const FHOLMatchConfig& Config, const int32 ViewerSlot)
{
	FHOLForumMessage Message;
	const int32 Slot = Event.Slot;
	const bool bOwn = ViewerSlot != INDEX_NONE && Slot == ViewerSlot;
	// V4-Q3: its camp alone (itself and its allies) has the major alert; a spectator never.
	const int32 Camp = HOLVictoryRules::GetCamp(Config, Slot);
	Message.bAlert = ViewerSlot != INDEX_NONE && Camp != INDEX_NONE && HOLVictoryRules::GetCamp(Config, ViewerSlot) == Camp;
	// V4.5b: no alert of its own loss (its defeat, its last life gone by a fall or a razing): its Defeat banner comes, the
	// thread's line is enough, whatever the order the multicast and the end's state arrive in. An ally's loss stays an alert.
	const bool bOwnLoss = Event.Kind == EHOLForumEventKind::Defeated || Event.Kind == EHOLForumEventKind::KingDied
		|| ((Event.Kind == EHOLForumEventKind::Fell || Event.Kind == EHOLForumEventKind::Razed) && Event.Lives <= 0);
	if (bOwn && bOwnLoss)
	{
		Message.bAlert = false;
	}
	// The singular for exactly 1, the plural for 0 (as the players' table, V2.3d).
	const FText Lives = FText::Format(Event.Lives == 1 ? LOCTEXT("TableLife", "{0} vie") : LOCTEXT("TableLives", "{0} vies"), Event.Lives);
	const FText Name = GetSlotName(Slot, ViewerSlot, Config);
	switch (Event.Kind)
	{
	case EHOLForumEventKind::Fell:
		if (bOwn)
		{
			Message.Text = FText::Format(LOCTEXT("MessageOwnFell", "Votre forum est tombé — {0}"), Lives);
		}
		else
		{
			Message.Text = FText::Format(LOCTEXT("MessageFell", "Le forum de {0} est tombé — {1}"), Name, Lives);
			Message.NamedSlots.Add(Slot);
		}
		break;
	case EHOLForumEventKind::HoldStarted:
	{
		const int32 Attacker = Event.OtherSlot;
		const bool bAttacker = ViewerSlot != INDEX_NONE && Attacker == ViewerSlot;
		const FText AttackerName = GetSlotName(Attacker, ViewerSlot, Config);
		if (bOwn)
		{
			Message.Text = FText::Format(LOCTEXT("MessageOwnHeld", "{0} tient votre ruine"), AttackerName);
			Message.NamedSlots.Add(Attacker);
		}
		else if (bAttacker)
		{
			Message.Text = FText::Format(LOCTEXT("MessageYouHold", "Vous tenez la ruine de {0}"), Name);
			Message.NamedSlots.Add(Slot);
		}
		else
		{
			Message.Text = FText::Format(LOCTEXT("MessageHeld", "{0} tient la ruine de {1}"), AttackerName, Name);
			Message.NamedSlots.Add(Attacker);
			Message.NamedSlots.Add(Slot);
		}
		break;
	}
	case EHOLForumEventKind::Razed:
		if (bOwn)
		{
			Message.Text = FText::Format(LOCTEXT("MessageOwnRazed", "Vos fondations sont rasées — {0}"), Lives);
		}
		else
		{
			Message.Text = FText::Format(LOCTEXT("MessageRazed", "Fondations de {0} rasées — {1}"), Name, Lives);
			Message.NamedSlots.Add(Slot);
		}
		break;
	case EHOLForumEventKind::KingDied:
		if (bOwn)
		{
			Message.Text = LOCTEXT("MessageOwnKingDied", "Votre Roi est mort");
		}
		else
		{
			Message.Text = FText::Format(LOCTEXT("MessageKingDied", "Le Roi de {0} est mort"), Name);
			Message.NamedSlots.Add(Slot);
		}
		break;
	case EHOLForumEventKind::Defeated:
	default:
		if (bOwn)
		{
			Message.Text = LOCTEXT("MessageOwnDefeated", "Vous êtes vaincu");
		}
		else
		{
			Message.Text = FText::Format(LOCTEXT("MessageDefeated", "{0} est vaincu"), Name);
			Message.NamedSlots.Add(Slot);
		}
		break;
	}
	Message.Shown = FString::Printf(TEXT("Message: %s%s"), *Message.Text.ToString(), Message.bAlert ? TEXT(" (alert)") : TEXT(""));
	return Message;
}

#undef LOCTEXT_NAMESPACE

const TCHAR* HOLForumMessages::GetKindName(const EHOLForumEventKind Kind)
{
	switch (Kind)
	{
	case EHOLForumEventKind::Fell: return TEXT("Fell");
	case EHOLForumEventKind::HoldStarted: return TEXT("HoldStarted");
	case EHOLForumEventKind::Razed: return TEXT("Razed");
	case EHOLForumEventKind::Defeated: return TEXT("Defeated");
	case EHOLForumEventKind::KingDied: return TEXT("KingDied");
	default: return TEXT("?");
	}
}

bool HOLForumMessages::ShouldAnnounceHold(const FHOLForumState& State, uint8& Announced)
{
	// The count back at 0, or no hold: the ruin's announcements forgotten.
	if (!State.IsDown() || State.Hold == EHOLForumHold::None || (State.HoldSteps <= 0 && State.Hold != EHOLForumHold::Holding))
	{
		Announced = 0;
		return false;
	}
	const int32 Holder = State.HolderSlot;
	if (State.Hold != EHOLForumHold::Holding || Holder < 0 || Holder >= FHOLMatchConfig::MaxSlots || (Announced & (1 << Holder)))
	{
		return false;
	}
	Announced |= 1 << Holder;
	return true;
}

TArray<HOLForumMessages::FPiece> HOLForumMessages::SplitNames(const FHOLForumMessage& Message, const int32 ViewerSlot)
{
	return SplitNames(Message, ViewerSlot, FHOLMatchConfig());
}

TArray<HOLForumMessages::FPiece> HOLForumMessages::SplitNames(const FHOLForumMessage& Message, const int32 ViewerSlot, const FHOLMatchConfig& Config)
{
	TArray<FPiece> Pieces;
	const FString Text = Message.Text.ToString();
	int32 From = 0;
	for (const int32 Slot : Message.NamedSlots)
	{
		const FString Name = GetSlotName(Slot, ViewerSlot, Config).ToString();
		const int32 At = Name.IsEmpty() ? INDEX_NONE : Text.Find(Name, ESearchCase::CaseSensitive, ESearchDir::FromStart, From);
		if (At == INDEX_NONE)
		{
			continue;
		}
		if (At > From)
		{
			Pieces.Add({ Text.Mid(From, At - From), INDEX_NONE });
		}
		Pieces.Add({ Name, Slot });
		From = At + Name.Len();
	}
	if (From < Text.Len() || Pieces.IsEmpty())
	{
		Pieces.Add({ Text.Mid(From), INDEX_NONE });
	}
	return Pieces;
}

void FHOLForumThread::Add(const FHOLForumMessage& Message, const double NowSeconds, const int32 Slot)
{
	Lines.Add({ Message, NowSeconds, Slot });
	if (Lines.Num() > MaxLines)
	{
		Lines.RemoveAt(0, Lines.Num() - MaxLines);
	}
}

bool FHOLForumThread::Expire(const double NowSeconds)
{
	return Lines.RemoveAll([NowSeconds](const FHOLForumThreadLine& Line) { return NowSeconds - Line.AddedSeconds >= LineSeconds; }) > 0;
}

float FHOLForumThread::GetOpacity(const FHOLForumThreadLine& Line, const double NowSeconds)
{
	const double Left = LineSeconds - (NowSeconds - Line.AddedSeconds);
	return static_cast<float>(FMath::Clamp(Left / FadeSeconds, 0.0, 1.0));
}

FString FHOLForumThread::Describe() const
{
	return FString::Printf(TEXT("[%s]"), *FString::JoinBy(Lines, TEXT(" | "), [](const FHOLForumThreadLine& Line) { return Line.Message.Text.ToString(); }));
}

bool FHOLForumAlert::Show(const FHOLForumMessage& InMessage, const int32 InSlot, const bool bMatchOverForViewer, const double NowSeconds)
{
	// D202: the end banner holds the screen then; a spectator's messages are never alerts (GetMessage).
	if (!InMessage.bAlert || bMatchOverForViewer)
	{
		return false;
	}
	// V4.5b: one sound for alerts coming less than SoundGapSeconds apart (it goes with the first one's banner).
	const bool bWithSound = NowSeconds - LastArrivalSeconds >= SoundGapSeconds;
	LastArrivalSeconds = NowSeconds;
	Waiting.Add({ InMessage, InSlot, bWithSound });
	if (Waiting.Num() > MaxWaiting)
	{
		// The oldest waiting dropped; its sound, if it had one, goes to the next (a burst keeps its one sound).
		const bool bDroppedSound = Waiting[0].bSound;
		Waiting.RemoveAt(0);
		Waiting[0].bSound = Waiting[0].bSound || bDroppedSound;
	}
	return Advance(NowSeconds, bMatchOverForViewer);
}

bool FHOLForumAlert::Advance(const double NowSeconds, const bool bMatchOverForViewer)
{
	if (bMatchOverForViewer)
	{
		Waiting.Reset();
		return false;
	}
	// The shown banner kept MinSeconds at least before the next waiting one replaces it.
	if (Waiting.IsEmpty() || (bShown && NowSeconds - AddedSeconds < MinSeconds))
	{
		return false;
	}
	const FWaiting Next = Waiting[0];
	Waiting.RemoveAt(0);
	Message = Next.Message;
	Slot = Next.Slot;
	bSound = Next.bSound;
	AddedSeconds = NowSeconds;
	bShown = true;
	return true;
}

bool FHOLForumAlert::Expire(const double NowSeconds)
{
	if (!bShown || NowSeconds - AddedSeconds < ShowSeconds || !Waiting.IsEmpty())
	{
		return false;
	}
	bShown = false;
	return true;
}

float FHOLForumAlert::GetOpacity(const double NowSeconds) const
{
	if (!bShown)
	{
		return 0.f;
	}
	const double Left = ShowSeconds - (NowSeconds - AddedSeconds);
	return static_cast<float>(FMath::Clamp(Left / FadeSeconds, 0.0, 1.0));
}

FString FHOLForumAlert::Describe() const
{
	return bShown ? FString::Printf(TEXT("%s (slot %d)"), *Message.Text.ToString(), Slot) : FString(TEXT("none"));
}
