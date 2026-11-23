// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLCommandGrid.h"

#include "Player/HOLCommandGridBoard.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Misc/ScopeLock.h"
#include "UObject/ObjectKey.h"

DEFINE_LOG_CATEGORY_STATIC(LogHOLCommandGridBridge, Log, All);

namespace
{
	// SP3 (Retours/Q-specialization-ui.md, default A): the Forum's grid cell of the first specialization option, the second after it.
	TAutoConsoleVariable<int32> CVarSpecializationCell(TEXT("hol.UI.SpecializationCell"), 6,
		TEXT("SP3: the main Forum's grid cell (0-10) of the first specialization option, the second one after it (default 6: D then F)."));

	/**
	 * A rule asked while no world has a script grid board (UHOLCommandGridBoard::FindAny null): the rule is the script's alone, the
	 * answer is the function's neutral default, said once per game (world) and per function (the HUD asks every frame).
	 */
	void LogNoBoardOnce(const TCHAR* Function)
	{
		const UWorld* World = nullptr;
		if (GEngine)
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (Context.World())
				{
					World = Context.World();
					break;
				}
			}
		}
		static FCriticalSection Lock;
		static TMap<FName, FObjectKey> Logged;
		const FObjectKey Key(World);
		{
			FScopeLock ScopeLock(&Lock);
			const FObjectKey* Last = Logged.Find(FName(Function));
			if (Last && *Last == Key)
			{
				return;
			}
			Logged.Add(FName(Function), Key);
		}
		if (!IsInGameThread())
		{
			// FindAny is null off the game thread whatever the boards: not the same cause as "no script grid board".
			UE_LOG(LogHOLCommandGridBridge, Error, TEXT("CommandGrid: %s refused, not on the game thread"), Function);
			return;
		}
		UE_LOG(LogHOLCommandGridBridge, Error, TEXT("CommandGrid: %s refused, no script grid board"), Function);
	}
}

namespace HOLCommandGrid
{
	int32 GetSpecializationCell(const int32 Option)
	{
		return FMath::Clamp(CVarSpecializationCell.GetValueOnAnyThread(), 0, NumCells - SpecializationOptions) + Option;
	}

	int32 GetOpenSpecializationTier(const int32 PlayerTier, const FHOLSpecializationView& View)
	{
		if (View.PendingTier > 0)
		{
			return View.PendingTier;
		}
		for (int32 Tier = 1; Tier <= 3; ++Tier)
		{
			if (View.GetChosen(Tier) < 0)
			{
				return Tier <= PlayerTier ? Tier : 0;
			}
		}
		return 0;
	}

	void SetSpecialization(FHOLGridContext& Context, const int32 PlayerTier, const FHOLSpecializationView& View)
	{
		Context.SpecializationTier = GetOpenSpecializationTier(PlayerTier, View);
		Context.SpecializationPendingOption = View.PendingTier > 0 ? View.PendingOption : INDEX_NONE;
		Context.SpecializationRemainingMs = View.PendingTier > 0 ? View.RemainingMs : 0;
	}

	FName GetReportName(const FHOLGridCommand& Command)
	{
		if (Command.Command != EHOLGridCommand::Specialize && Command.Command != EHOLGridCommand::CancelSpecialization)
		{
			return NAME_None;
		}
		return FName(*FString::Printf(TEXT("%sT%d%s"), Command.Command == EHOLGridCommand::Specialize ? TEXT("Specialize") : TEXT("CancelSpecialization"),
			Command.Tier, Command.Option == 0 ? TEXT("A") : TEXT("B")));
	}

	FHOLGridCommand GetCommand(const FHOLGridContext& Context, const int32 Cell)
	{
		FHOLGridCommand Answer;
		if (UHOLCommandGridBoard::AskGetCommand(Context, Cell, Answer))
		{
			return Answer;
		}
		LogNoBoardOnce(TEXT("GetCommand"));
		return FHOLGridCommand();
	}

	int32 GetRageCell()
	{
		return 2;
	}

	void NoteSelectedUnitType(FHOLGridContext& Context, const FName TypeName)
	{
		Context.bRageUnit |= TypeName == FName(TEXT("DamnedWarrior"));
	}
	int32 GetHeroAbilityCell(const EHOLHeroAbility Ability)
	{
		return Ability == EHOLHeroAbility::CloseRanks ? 0 : Ability == EHOLHeroAbility::Banner ? 1 : INDEX_NONE;
	}

	int32 GetTradeCell(const EHOLResource Resource, const bool bBuy)
	{
		const int32 Good = static_cast<int32>(Resource);
		if (Good < 0 || Good >= TradableGoods)
		{
			return INDEX_NONE;
		}
		return (bBuy ? 0 : GridColumns) + Good;
	}

	FHOLTradeGain GetTradeGain(const EHOLResource Resource, const bool bBuy, const int32 Received)
	{
		FHOLTradeGain Gain;
		Gain.Amount = FMath::Max(Received, 0);
		Gain.Logo = bBuy ? Resource : EHOLResource::Gold;
		return Gain;
	}

	FString GetTradeGainText(const FHOLTradeGain& Gain)
	{
		return FString::Printf(TEXT("+%d"), Gain.Amount);
	}

	FHOLGridCommand GetOrder(const FHOLGridContext& Context, const int32 Cell)
	{
		FHOLGridCommand Answer;
		if (UHOLCommandGridBoard::AskGetOrder(Context, Cell, Answer))
		{
			return Answer;
		}
		LogNoBoardOnce(TEXT("GetOrder"));
		return FHOLGridCommand();
	}

	EMouseCursor::Type GetMouseCursor(const bool bAttackMoveArmed, const bool bPlacing)
	{
		return bAttackMoveArmed && !bPlacing ? EMouseCursor::Crosshairs : EMouseCursor::Default;
	}

	bool IsBuildGrid(const FHOLGridContext& Context)
	{
		return !Context.bOwnBuilding && Context.bBuilders && !Context.bSoldiers;
	}

	bool IsAgeReached(const FHOLGridContext& Context, const int32 Age)
	{
		bool bAnswer = false;
		if (UHOLCommandGridBoard::AskIsAgeReached(Context, Age, bAnswer))
		{
			return bAnswer;
		}
		LogNoBoardOnce(TEXT("IsAgeReached"));
		return false;
	}

	bool IsAgeOpen(const FHOLGridContext& Context, const int32 Age)
	{
		bool bAnswer = false;
		if (UHOLCommandGridBoard::AskIsAgeOpen(Context, Age, bAnswer))
		{
			return bAnswer;
		}
		LogNoBoardOnce(TEXT("IsAgeOpen"));
		return false;
	}

	float GetAgePanelOpacity(const FHOLGridContext& Context, const int32 Age)
	{
		// A page faded only while nothing on it can be built: a page opened by a type built whatever the tier (a Nomad's first
		// forum, D797) shown whole, its locked cells greyed one by one (bGreyed); its medallion keeps its locked look.
		return IsAgeOpen(Context, Age) ? 1.f : LockedAgePanelOpacity;
	}

	int32 GetCategoryRow(const EHOLBuildingCategory Category)
	{
		return Category == EHOLBuildingCategory::Economic ? 0 : Category == EHOLBuildingCategory::Military ? 1 : 2;
	}

	FHOLGridCommand GetAgeBuilding(const FHOLGridContext& Context, const int32 Age, const int32 Cell)
	{
		FHOLGridCommand Answer;
		if (UHOLCommandGridBoard::AskGetAgeBuilding(Context, Age, Cell, Answer))
		{
			return Answer;
		}
		LogNoBoardOnce(TEXT("GetAgeBuilding"));
		return FHOLGridCommand();
	}

	FHOLGridKeyResult PressKey(const FHOLGridContext& Context, const int32 ChosenAge, const int32 Cell)
	{
		FHOLGridKeyResult Answer;
		if (UHOLCommandGridBoard::AskPressKey(Context, ChosenAge, Cell, Answer))
		{
			return Answer;
		}
		LogNoBoardOnce(TEXT("PressKey"));
		FHOLGridKeyResult None;
		None.ChosenAge = ChosenAge;
		return None;
	}

	FHOLGridKeyResult ClickAge(const FHOLGridContext& Context, const int32 ChosenAge, const int32 Age, const int32 Cell)
	{
		FHOLGridKeyResult Answer;
		if (UHOLCommandGridBoard::AskClickAge(Context, ChosenAge, Age, Cell, Answer))
		{
			return Answer;
		}
		LogNoBoardOnce(TEXT("ClickAge"));
		FHOLGridKeyResult None;
		None.ChosenAge = ChosenAge;
		return None;
	}

	int32 GetChosenAgeAfter(const int32 ChosenAge, const EHOLAgeEvent Event)
	{
		int32 Answer = INDEX_NONE;
		if (UHOLCommandGridBoard::AskGetChosenAgeAfter(ChosenAge, Event, Answer))
		{
			return Answer;
		}
		LogNoBoardOnce(TEXT("GetChosenAgeAfter"));
		return INDEX_NONE;
	}

	bool FindMissingResource(const FHOLResources& Stock, const FHOLResources& Cost, EHOLResource& OutResource)
	{
		for (const EHOLResource Resource : { EHOLResource::Food, EHOLResource::Wood, EHOLResource::Stone, EHOLResource::Gold })
		{
			if (Stock[Resource] < Cost[Resource])
			{
				OutResource = Resource;
				return true;
			}
		}
		return false;
	}

	FHOLCommandFeedback CheckProduction(const FHOLResources& Stock, const FHOLResources& Cost, const int32 QueueLength, const int32 MaxQueue,
		const int32 PopulationUsed, const int32 PopulationCap, const int32 Places)
	{
		FHOLCommandFeedback Answer;
		if (UHOLCommandGridBoard::AskCheckProduction(Stock, Cost, QueueLength, MaxQueue, PopulationUsed, PopulationCap, Places, Answer))
		{
			return Answer;
		}
		LogNoBoardOnce(TEXT("CheckProduction"));
		return FHOLCommandFeedback();
	}

	FHOLCommandFeedback CheckResearch(const FHOLTechnologyRow& Row, HOLTechnology::FFacts Facts, const FHOLResources& Stock,
		const int32 QueueLength, const int32 MaxQueue)
	{
		FHOLCommandFeedback Feedback;
		Facts.bCanPay = Stock.Covers(Row.Cost);
		const HOLTechnology::EBlock Block = HOLTechnology::Check(Row, Facts);
		if (Block == HOLTechnology::EBlock::CannotPay)
		{
			FindMissingResource(Stock, Row.Cost, Feedback.Resource);
			Feedback.Kind = EHOLCommandFeedback::NotEnoughResources;
		}
		else if (Block != HOLTechnology::EBlock::None)
		{
			Feedback.Kind = EHOLCommandFeedback::ResearchRefused;
			Feedback.Block = Block;
		}
		else if (QueueLength >= MaxQueue)
		{
			Feedback.Kind = EHOLCommandFeedback::QueueFull;
		}
		return Feedback;
	}

	FHOLCommandFeedback CheckBuild(const FHOLResources& Stock, const FHOLResources& Cost)
	{
		FHOLCommandFeedback Answer;
		if (UHOLCommandGridBoard::AskCheckBuild(Stock, Cost, Answer))
		{
			return Answer;
		}
		LogNoBoardOnce(TEXT("CheckBuild"));
		return FHOLCommandFeedback();
	}

	bool Blocks(const FHOLCommandFeedback& Feedback)
	{
		bool bAnswer = false;
		if (UHOLCommandGridBoard::AskBlocks(Feedback, bAnswer))
		{
			return bAnswer;
		}
		LogNoBoardOnce(TEXT("Blocks"));
		return false;
	}

	bool IsPopulationBlocked(const int32 PopulationUsed, const int32 PopulationCap, const int32 Places)
	{
		return PopulationUsed + FMath::Max(Places, 0) > PopulationCap;
	}

	FString Describe(const FHOLCommandFeedback& Feedback)
	{
		FString Answer;
		if (UHOLCommandGridBoard::AskDescribe(Feedback, Answer))
		{
			return Answer;
		}
		LogNoBoardOnce(TEXT("Describe"));
		return TEXT("none");
	}

	FHOLGridCellFace GetCellFace(const bool bHasIcon)
	{
		FHOLGridCellFace Face;
		Face.bIcon = bHasIcon;
		Face.bName = !bHasIcon;
		return Face;
	}

	int32 GetCycleRank(const FHOLBuildingTypeRow& Type)
	{
		int32 Answer = 0;
		if (UHOLCommandGridBoard::AskGetCycleRank(Type.IsBuildable(), Type.BuildGridSlot, Answer))
		{
			return Answer;
		}
		LogNoBoardOnce(TEXT("GetCycleRank"));
		return 0;
	}

	bool IsSelectedByCategory(const FHOLBuildingTypeRow& Type, const EHOLBuildingCategory Category)
	{
		bool bAnswer = false;
		if (UHOLCommandGridBoard::AskIsSelectedByCategory(Type.Category, Category, bAnswer))
		{
			return bAnswer;
		}
		LogNoBoardOnce(TEXT("IsSelectedByCategory"));
		return false;
	}

	bool IsRepeatPress(const TConstArrayView<uint32> Selected, const TConstArrayView<uint32> LastSelected, const bool bSameKey)
	{
		bool bAnswer = false;
		if (UHOLCommandGridBoard::AskIsRepeatPress(Selected, LastSelected, bSameKey, bAnswer))
		{
			return bAnswer;
		}
		LogNoBoardOnce(TEXT("IsRepeatPress"));
		return false;
	}

	int32 NextInCycle(const TConstArrayView<FHOLCycleBuilding> Buildings, const uint32 LastKey)
	{
		int32 Answer = INDEX_NONE;
		if (UHOLCommandGridBoard::AskNextInCycle(Buildings, LastKey, Answer))
		{
			return Answer;
		}
		LogNoBoardOnce(TEXT("NextInCycle"));
		return INDEX_NONE;
	}

	bool IsAloneInGroup(const FName TypeName)
	{
		bool bAnswer = false;
		if (UHOLCommandGridBoard::AskIsAloneInGroup(TypeName, bAnswer))
		{
			return bAnswer;
		}
		LogNoBoardOnce(TEXT("IsAloneInGroup"));
		return false;
	}

	TArray<FHOLSubgroup> SplitSubgroups(const TConstArrayView<FHOLGroupMember> Members)
	{
		TArray<FHOLSubgroup> Answer;
		if (UHOLCommandGridBoard::AskSplitSubgroups(Members, Answer))
		{
			return Answer;
		}
		LogNoBoardOnce(TEXT("SplitSubgroups"));
		return TArray<FHOLSubgroup>();
	}

	TArray<int32> GetCommandPriority(const TConstArrayView<FHOLGroupMember> Members)
	{
		TArray<int32> Answer;
		if (UHOLCommandGridBoard::AskGetCommandPriority(Members, Answer))
		{
			return Answer;
		}
		LogNoBoardOnce(TEXT("GetCommandPriority"));
		return TArray<int32>();
	}

	TArray<FHOLGridCommand> MergeCommands(const TConstArrayView<TArray<FHOLGridCommand>> PerBuilding, const TConstArrayView<int32> Priority)
	{
		TArray<FHOLGridCommand> Answer;
		if (UHOLCommandGridBoard::AskMergeCommands(PerBuilding, Priority, Answer))
		{
			return Answer;
		}
		LogNoBoardOnce(TEXT("MergeCommands"));
		return TArray<FHOLGridCommand>();
	}

	int32 PickEvenly(const TConstArrayView<FHOLQueueChoice> Choices, const uint32 LastKey)
	{
		int32 Answer = INDEX_NONE;
		if (UHOLCommandGridBoard::AskPickEvenly(Choices, LastKey, Answer))
		{
			return Answer;
		}
		LogNoBoardOnce(TEXT("PickEvenly"));
		return INDEX_NONE;
	}
}
