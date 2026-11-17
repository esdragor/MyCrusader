// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLForumMessagesReadLibrary.h"

#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLMatchConfig.h"

// Unique names (the module is built unity): every helper is prefixed ForumMsgLib.
namespace
{
	/** As the C++ test's MakeTeams: the default config, then the slots the scenario lists (a missing entry = Closed, team 0, default name). */
	FHOLMatchConfig ForumMsgLibMakeConfig(const FHOLForumScenario& Scenario)
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			FHOLMatchSlot& Slot = Config.Slots[Index];
			const int32 State = Scenario.States.IsValidIndex(Index) ? Scenario.States[Index] : 0;
			Slot.State = State == 1 ? EHOLSlotState::Human : (State == 2 ? EHOLSlotState::AI : EHOLSlotState::Closed);
			Slot.Team = Scenario.Teams.IsValidIndex(Index) ? Scenario.Teams[Index] : 0;
			if (Scenario.Names.IsValidIndex(Index) && !Scenario.Names[Index].IsEmpty())
			{
				Slot.PlayerName = Scenario.Names[Index];
			}
		}
		return Config;
	}

	FHOLForumEvent ForumMsgLibMakeEvent(const FHOLForumEventValue& Event)
	{
		return FHOLForumEvent::Make(static_cast<EHOLForumEventKind>(Event.Kind), Event.Slot, Event.OtherSlot, Event.Lives, static_cast<EHOLForumEventReason>(Event.Reason));
	}

	FHOLForumMessage ForumMsgLibGetMessage(const FHOLForumEventValue& Event, const FHOLForumScenario& Scenario, const int32 ViewerSlot, const bool bSeesForum)
	{
		return HOLForumMessages::GetMessage(ForumMsgLibMakeEvent(Event), ForumMsgLibMakeConfig(Scenario), ViewerSlot, bSeesForum);
	}
}

FHOLForumMessageValue UHOLForumMessagesReadLibrary::GetMessage(const FHOLForumEventValue& Event, const FHOLForumScenario& Scenario, const int32 ViewerSlot, const bool bSeesForum)
{
	const FHOLForumMessage Message = ForumMsgLibGetMessage(Event, Scenario, ViewerSlot, bSeesForum);
	FHOLForumMessageValue Value;
	Value.Source = Message.Text.BuildSourceString();
	Value.Display = Message.Text.ToString();
	Value.bAlert = Message.bAlert;
	Value.NamedSlots = Message.NamedSlots;
	Value.Shown = Message.Shown;
	return Value;
}

bool UHOLForumMessagesReadLibrary::ElidesDe(const FString& Name)
{
	return HOLForumMessages::ElidesDe(Name);
}

FString UHOLForumMessagesReadLibrary::ElidePattern(const FString& Pattern, const int32 NameIndex, const FString& Name)
{
	return HOLForumMessages::ElidePattern(Pattern, NameIndex, Name);
}

bool UHOLForumMessagesReadLibrary::ShouldAnnounceHold(const int32 HoldKind, const int32 HolderSlot, const int32 HoldSteps, int32& Announced)
{
	FHOLForumState State;
	State.Phase = EHOLForumPhase::Ruin;
	State.Hold = static_cast<EHOLForumHold>(HoldKind);
	State.HolderSlot = static_cast<int8>(HolderSlot);
	State.HoldSteps = HoldSteps;
	uint8 Bits = static_cast<uint8>(Announced);
	const bool bAnnounce = HOLForumMessages::ShouldAnnounceHold(State, Bits);
	Announced = Bits;
	return bAnnounce;
}

TArray<FHOLForumPieceValue> UHOLForumMessagesReadLibrary::SplitNames(const FHOLForumEventValue& Event, const FHOLForumScenario& Scenario, const int32 ViewerSlot)
{
	const FHOLMatchConfig Config = ForumMsgLibMakeConfig(Scenario);
	const FHOLForumMessage Message = HOLForumMessages::GetMessage(ForumMsgLibMakeEvent(Event), Config, ViewerSlot);
	TArray<FHOLForumPieceValue> Values;
	for (const HOLForumMessages::FPiece& Piece : HOLForumMessages::SplitNames(Message, ViewerSlot, Config))
	{
		FHOLForumPieceValue& Value = Values.AddDefaulted_GetRef();
		Value.Text = Piece.Text;
		Value.Slot = Piece.Slot;
	}
	return Values;
}

FString UHOLForumMessagesReadLibrary::GetCultureName()
{
	return FInternationalization::Get().GetCurrentCulture()->GetName();
}

FString UHOLForumMessagesReadLibrary::SetCultureName(const FString& Culture)
{
	const FString Previous = FInternationalization::Get().GetCurrentCulture()->GetName();
	FInternationalization::Get().SetCurrentCulture(Culture);
	return Previous;
}

int32 UHOLForumMessagesReadLibrary::GetThreadMaxLines()
{
	return FHOLForumThread::MaxLines;
}

int32 UHOLForumMessagesReadLibrary::GetAlertMaxWaiting()
{
	return FHOLForumAlert::MaxWaiting;
}

UHOLForumThreadHolder* UHOLForumThreadHolder::Create()
{
	return NewObject<UHOLForumThreadHolder>(GetTransientPackage());
}

void UHOLForumThreadHolder::AddEvent(const FHOLForumEventValue& Event, const FHOLForumScenario& Scenario, const int32 ViewerSlot, const double NowSeconds)
{
	Thread.Add(ForumMsgLibGetMessage(Event, Scenario, ViewerSlot, false), NowSeconds);
}

FString UHOLForumThreadHolder::Describe() const
{
	return Thread.Describe();
}

int32 UHOLForumThreadHolder::GetLineCount() const
{
	return Thread.Lines.Num();
}

double UHOLForumThreadHolder::GetLineAddedSeconds(const int32 Index) const
{
	return Thread.Lines.IsValidIndex(Index) ? Thread.Lines[Index].AddedSeconds : -1.0;
}

FString UHOLForumThreadHolder::GetLineText(const int32 Index) const
{
	return Thread.Lines.IsValidIndex(Index) ? Thread.Lines[Index].Message.Text.ToString() : FString();
}

double UHOLForumThreadHolder::GetLineOpacity(const int32 Index, const double NowSeconds) const
{
	return Thread.Lines.IsValidIndex(Index) ? static_cast<double>(FHOLForumThread::GetOpacity(Thread.Lines[Index], NowSeconds)) : 0.0;
}

bool UHOLForumThreadHolder::Expire(const double NowSeconds)
{
	return Thread.Expire(NowSeconds);
}

UHOLForumAlertHolder* UHOLForumAlertHolder::Create()
{
	return NewObject<UHOLForumAlertHolder>(GetTransientPackage());
}

bool UHOLForumAlertHolder::ShowEvent(const FHOLForumEventValue& Event, const FHOLForumScenario& Scenario, const int32 ViewerSlot, const int32 InSlot, const bool bMatchOverForViewer, const double NowSeconds)
{
	return Alert.Show(ForumMsgLibGetMessage(Event, Scenario, ViewerSlot, false), InSlot, bMatchOverForViewer, NowSeconds);
}

bool UHOLForumAlertHolder::Advance(const double NowSeconds, const bool bMatchOverForViewer)
{
	return Alert.Advance(NowSeconds, bMatchOverForViewer);
}

bool UHOLForumAlertHolder::Expire(const double NowSeconds)
{
	return Alert.Expire(NowSeconds);
}

double UHOLForumAlertHolder::GetOpacity(const double NowSeconds) const
{
	return static_cast<double>(Alert.GetOpacity(NowSeconds));
}

FString UHOLForumAlertHolder::Describe() const
{
	return Alert.Describe();
}

int32 UHOLForumAlertHolder::GetSlot() const
{
	return Alert.Slot;
}

bool UHOLForumAlertHolder::HasSound() const
{
	return Alert.bSound;
}

int32 UHOLForumAlertHolder::GetWaitingCount() const
{
	return Alert.Waiting.Num();
}
