// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLCommandGridReadLibrary.h"

#include "Player/HOLCommandGrid.h"

// Unique namespace: unity builds merge the .cpp files, an anonymous namespace would collide with another file's helpers.
namespace HOLCommandGridReadLibraryPrivate
{
	FHOLGridContext ToRule(const FHOLGridContextView& View)
	{
		FHOLGridContext Out;
		Out.bOwnBuilding = View.bOwnBuilding;
		Out.bBuildingGroup = View.bBuildingGroup;
		Out.bBuildingFinished = View.bBuildingFinished;
		Out.bForumDown = View.bForumDown;
		Out.bMarket = View.bMarket;
		Out.Produces = View.Produces;
		Out.bBuilders = View.bBuilders;
		Out.bOwnUnits = View.bOwnUnits;
		Out.bSoldiers = View.bSoldiers;
		Out.bOnlyKings = View.bOnlyKings;
		Out.bHero = View.bHero;
		Out.HeroLevel = View.HeroLevel;
		Out.ReachedAge = View.ReachedAge;
		const int32 Count = FMath::Min3(View.BuildableTypes.Num(), View.BuildableCells.Num(), View.BuildableAges.Num());
		for (int32 Index = 0; Index < Count; ++Index)
		{
			FHOLBuildable Buildable;
			Buildable.TypeName = View.BuildableTypes[Index];
			Buildable.Cell = View.BuildableCells[Index];
			Buildable.Age = View.BuildableAges[Index];
			Out.Buildable.Add(Buildable);
		}
		return Out;
	}

	FHOLGridCommandView FromRule(const FHOLGridCommand& Command)
	{
		FHOLGridCommandView Out;
		Out.Command = static_cast<EHOLGridCommandKind>(static_cast<uint8>(Command.Command));
		Out.TypeName = Command.TypeName;
		Out.Age = Command.Age;
		Out.bGreyed = Command.bGreyed;
		Out.Resource = Command.Resource;
		Out.bBuy = Command.bBuy;
		return Out;
	}

	FHOLGridCommand ToRule(const FHOLGridCommandView& View)
	{
		FHOLGridCommand Out;
		Out.Command = static_cast<EHOLGridCommand>(static_cast<uint8>(View.Command));
		Out.TypeName = View.TypeName;
		Out.Age = View.Age;
		Out.bGreyed = View.bGreyed;
		Out.Resource = View.Resource;
		Out.bBuy = View.bBuy;
		return Out;
	}

	FHOLGridKeyResultView FromRule(const FHOLGridKeyResult& Result)
	{
		FHOLGridKeyResultView Out;
		Out.Target = static_cast<EHOLGridKeyTargetKind>(static_cast<uint8>(Result.Target));
		Out.ChosenAge = Result.ChosenAge;
		Out.TypeName = Result.TypeName;
		return Out;
	}

	FHOLFeedbackView FromRule(const FHOLCommandFeedback& Feedback)
	{
		FHOLFeedbackView Out;
		Out.Kind = static_cast<EHOLCommandFeedbackKind>(static_cast<uint8>(Feedback.Kind));
		Out.Resource = Feedback.Resource;
		Out.Refusal = static_cast<int32>(Feedback.Refusal);
		Out.Category = Feedback.Category;
		return Out;
	}

	FHOLCommandFeedback ToRule(const FHOLFeedbackView& View)
	{
		FHOLCommandFeedback Out;
		Out.Kind = static_cast<EHOLCommandFeedback>(static_cast<uint8>(View.Kind));
		Out.Resource = View.Resource;
		Out.Refusal = static_cast<EHOLPlacementRefusal>(View.Refusal);
		Out.Category = View.Category;
		return Out;
	}

	TArray<FHOLGroupMember> ToRule(const TArray<FHOLGroupMemberView>& Views)
	{
		TArray<FHOLGroupMember> Out;
		for (const FHOLGroupMemberView& View : Views)
		{
			FHOLGroupMember Member;
			Member.TypeName = View.TypeName;
			Member.bAlone = View.bAlone;
			Out.Add(Member);
		}
		return Out;
	}

	TArray<uint32> ToKeys(const TArray<int32>& Values)
	{
		TArray<uint32> Out;
		for (const int32 Value : Values)
		{
			Out.Add(static_cast<uint32>(Value));
		}
		return Out;
	}
}

using namespace HOLCommandGridReadLibraryPrivate;

int32 UHOLCommandGridReadLibrary::GetNumCells() { return HOLCommandGrid::NumCells; }
int32 UHOLCommandGridReadLibrary::GetCancelCell() { return HOLCommandGrid::CancelCell; }
int32 UHOLCommandGridReadLibrary::GetGridColumns() { return HOLCommandGrid::GridColumns; }
int32 UHOLCommandGridReadLibrary::GetPlacementRefusalBuilding() { return static_cast<int32>(EHOLPlacementRefusal::Building); }

int32 UHOLCommandGridReadLibrary::GetCategoryRow(const EHOLBuildingCategory Category) { return HOLCommandGrid::GetCategoryRow(Category); }
int32 UHOLCommandGridReadLibrary::GetTradeCell(const EHOLResource Resource, const bool bBuy) { return HOLCommandGrid::GetTradeCell(Resource, bBuy); }

FHOLTradeGainView UHOLCommandGridReadLibrary::GetTradeGain(const EHOLResource Resource, const bool bBuy, const int32 Received)
{
	const FHOLTradeGain Gain = HOLCommandGrid::GetTradeGain(Resource, bBuy, Received);
	FHOLTradeGainView Out;
	Out.Amount = Gain.Amount;
	Out.Logo = Gain.Logo;
	return Out;
}

FString UHOLCommandGridReadLibrary::GetTradeGainText(const FHOLTradeGainView& Gain)
{
	FHOLTradeGain Rule;
	Rule.Amount = Gain.Amount;
	Rule.Logo = Gain.Logo;
	return HOLCommandGrid::GetTradeGainText(Rule);
}

FHOLGridCommandView UHOLCommandGridReadLibrary::GetCommand(const FHOLGridContextView& Context, const int32 Cell)
{
	return FromRule(HOLCommandGrid::GetCommand(ToRule(Context), Cell));
}

FHOLGridCommandView UHOLCommandGridReadLibrary::GetOrder(const FHOLGridContextView& Context, const int32 Cell)
{
	return FromRule(HOLCommandGrid::GetOrder(ToRule(Context), Cell));
}

bool UHOLCommandGridReadLibrary::IsBuildGrid(const FHOLGridContextView& Context) { return HOLCommandGrid::IsBuildGrid(ToRule(Context)); }
bool UHOLCommandGridReadLibrary::IsAgeReached(const FHOLGridContextView& Context, const int32 Age) { return HOLCommandGrid::IsAgeReached(ToRule(Context), Age); }

FHOLGridCommandView UHOLCommandGridReadLibrary::GetAgeBuilding(const FHOLGridContextView& Context, const int32 Age, const int32 Cell)
{
	return FromRule(HOLCommandGrid::GetAgeBuilding(ToRule(Context), Age, Cell));
}

FHOLGridKeyResultView UHOLCommandGridReadLibrary::PressKey(const FHOLGridContextView& Context, const int32 ChosenAge, const int32 Cell)
{
	return FromRule(HOLCommandGrid::PressKey(ToRule(Context), ChosenAge, Cell));
}

FHOLGridKeyResultView UHOLCommandGridReadLibrary::ClickAge(const FHOLGridContextView& Context, const int32 ChosenAge, const int32 Age, const int32 Cell)
{
	return FromRule(HOLCommandGrid::ClickAge(ToRule(Context), ChosenAge, Age, Cell));
}

int32 UHOLCommandGridReadLibrary::GetChosenAgeAfter(const int32 ChosenAge, const EHOLAgeEventKind Event)
{
	return HOLCommandGrid::GetChosenAgeAfter(ChosenAge, static_cast<EHOLAgeEvent>(static_cast<uint8>(Event)));
}

FHOLGridContextView UHOLCommandGridReadLibrary::WithTableBuildables(const FHOLGridContextView& Context)
{
	FHOLGridContextView Out = Context;
	TArray<FHOLBuildable> Buildable;
	UHOLBuildingSettings::GetBuildableTypes(Buildable);
	Out.BuildableTypes.Reset();
	Out.BuildableCells.Reset();
	Out.BuildableAges.Reset();
	for (const FHOLBuildable& Type : Buildable)
	{
		Out.BuildableTypes.Add(Type.TypeName);
		Out.BuildableCells.Add(Type.Cell);
		Out.BuildableAges.Add(Type.Age);
	}
	return Out;
}

TArray<FName> UHOLCommandGridReadLibrary::GetProducesOf(const FName TypeName)
{
	const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(TypeName);
	return Type ? Type->Produces : TArray<FName>();
}

bool UHOLCommandGridReadLibrary::IsTypeInTable(const FName TypeName) { return UHOLBuildingSettings::FindType(TypeName) != nullptr; }

EHOLBuildingCategory UHOLCommandGridReadLibrary::GetTypeCategory(const FName TypeName)
{
	const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(TypeName);
	return Type ? Type->Category : EHOLBuildingCategory::Economic;
}

int32 UHOLCommandGridReadLibrary::GetCycleRankOfType(const FName TypeName)
{
	const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(TypeName);
	return Type ? HOLCommandGrid::GetCycleRank(*Type) : INDEX_NONE;
}

bool UHOLCommandGridReadLibrary::IsTypeSelectedByCategory(const FName TypeName, const EHOLBuildingCategory Category)
{
	const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(TypeName);
	return Type && HOLCommandGrid::IsSelectedByCategory(*Type, Category);
}

bool UHOLCommandGridReadLibrary::IsCategorySelectedByCategory(const EHOLBuildingCategory TypeCategory, const EHOLBuildingCategory Category)
{
	FHOLBuildingTypeRow Row;
	Row.Category = TypeCategory;
	return HOLCommandGrid::IsSelectedByCategory(Row, Category);
}

bool UHOLCommandGridReadLibrary::HasMissingResource(const FHOLResources& Stock, const FHOLResources& Cost)
{
	EHOLResource Missing = EHOLResource::Gold;
	return HOLCommandGrid::FindMissingResource(Stock, Cost, Missing);
}

EHOLResource UHOLCommandGridReadLibrary::GetMissingResource(const FHOLResources& Stock, const FHOLResources& Cost)
{
	EHOLResource Missing = EHOLResource::Gold;
	HOLCommandGrid::FindMissingResource(Stock, Cost, Missing);
	return Missing;
}

FHOLFeedbackView UHOLCommandGridReadLibrary::CheckProduction(const FHOLResources& Stock, const FHOLResources& Cost, const int32 QueueLength, const int32 MaxQueue,
	const int32 PopulationUsed, const int32 PopulationCap, const int32 Places)
{
	return FromRule(HOLCommandGrid::CheckProduction(Stock, Cost, QueueLength, MaxQueue, PopulationUsed, PopulationCap, Places));
}

FHOLFeedbackView UHOLCommandGridReadLibrary::CheckBuild(const FHOLResources& Stock, const FHOLResources& Cost)
{
	return FromRule(HOLCommandGrid::CheckBuild(Stock, Cost));
}

bool UHOLCommandGridReadLibrary::Blocks(const FHOLFeedbackView& Feedback) { return HOLCommandGrid::Blocks(ToRule(Feedback)); }

bool UHOLCommandGridReadLibrary::IsPopulationBlocked(const int32 PopulationUsed, const int32 PopulationCap, const int32 Places)
{
	return HOLCommandGrid::IsPopulationBlocked(PopulationUsed, PopulationCap, Places);
}

FString UHOLCommandGridReadLibrary::Describe(const FHOLFeedbackView& Feedback) { return HOLCommandGrid::Describe(ToRule(Feedback)); }

FHOLGridCellFaceView UHOLCommandGridReadLibrary::GetCellFace(const bool bHasIcon)
{
	const FHOLGridCellFace Face = HOLCommandGrid::GetCellFace(bHasIcon);
	FHOLGridCellFaceView Out;
	Out.bIcon = Face.bIcon;
	Out.bName = Face.bName;
	Out.bCost = Face.bCost;
	return Out;
}

bool UHOLCommandGridReadLibrary::IsRepeatPress(const TArray<int32>& Selected, const TArray<int32>& LastSelected, const bool bSameKey)
{
	const TArray<uint32> SelectedKeys = ToKeys(Selected);
	const TArray<uint32> LastKeys = ToKeys(LastSelected);
	return HOLCommandGrid::IsRepeatPress(SelectedKeys, LastKeys, bSameKey);
}

int32 UHOLCommandGridReadLibrary::NextInCycle(const TArray<FHOLCycleBuildingView>& Buildings, const int32 LastKey)
{
	TArray<FHOLCycleBuilding> Rule;
	for (const FHOLCycleBuildingView& View : Buildings)
	{
		Rule.Add({ View.Rank, static_cast<uint32>(View.Key) });
	}
	return HOLCommandGrid::NextInCycle(Rule, static_cast<uint32>(LastKey));
}

bool UHOLCommandGridReadLibrary::IsAloneInGroup(const FName TypeName) { return HOLCommandGrid::IsAloneInGroup(TypeName); }

TArray<FHOLSubgroupView> UHOLCommandGridReadLibrary::SplitSubgroups(const TArray<FHOLGroupMemberView>& Members)
{
	const TArray<FHOLGroupMember> Rule = ToRule(Members);
	TArray<FHOLSubgroupView> Out;
	for (const FHOLSubgroup& Subgroup : HOLCommandGrid::SplitSubgroups(Rule))
	{
		FHOLSubgroupView View;
		View.bAll = Subgroup.bAll;
		View.TypeName = Subgroup.TypeName;
		View.Members = Subgroup.Members;
		Out.Add(View);
	}
	return Out;
}

TArray<int32> UHOLCommandGridReadLibrary::GetCommandPriority(const TArray<FHOLGroupMemberView>& Members)
{
	const TArray<FHOLGroupMember> Rule = ToRule(Members);
	return HOLCommandGrid::GetCommandPriority(Rule);
}

TArray<FHOLGridCommandView> UHOLCommandGridReadLibrary::MergeCommands(const TArray<FHOLGridCommandView>& PerBuilding, const TArray<int32>& Priority)
{
	TArray<TArray<FHOLGridCommand>> Rule;
	const int32 Cells = HOLCommandGrid::NumCells;
	for (int32 Start = 0; Start + Cells <= PerBuilding.Num(); Start += Cells)
	{
		TArray<FHOLGridCommand>& Building = Rule.AddDefaulted_GetRef();
		for (int32 Cell = 0; Cell < Cells; ++Cell)
		{
			Building.Add(ToRule(PerBuilding[Start + Cell]));
		}
	}
	TArray<FHOLGridCommandView> Out;
	for (const FHOLGridCommand& Command : HOLCommandGrid::MergeCommands(Rule, Priority))
	{
		Out.Add(FromRule(Command));
	}
	return Out;
}

int32 UHOLCommandGridReadLibrary::PickEvenly(const TArray<FHOLQueueChoiceView>& Choices, const int32 LastKey)
{
	TArray<FHOLQueueChoice> Rule;
	for (const FHOLQueueChoiceView& View : Choices)
	{
		Rule.Add({ View.QueueLength, View.MaxQueue, static_cast<uint32>(View.Key) });
	}
	return HOLCommandGrid::PickEvenly(Rule, static_cast<uint32>(LastKey));
}
