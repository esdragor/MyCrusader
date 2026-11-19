// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLCommandGridReadLibrary.h"

#include "Player/HOLCommandGrid.h"
#include "Technologies/HOLTechnologyTypes.h"

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
		Out.Researches = View.Researches;
		Out.bDeployables = View.bDeployables;
		Out.bDeployedAny = View.bDeployedAny;
		Out.bRageUnit = View.bRageUnit;
		Out.bPrimaryForum = View.bPrimaryForum;
		Out.SpecializationTier = View.SpecializationTier;
		Out.SpecializationPendingOption = View.SpecializationPendingOption;
		Out.SpecializationRemainingMs = View.SpecializationRemainingMs;
		for (int32 Ability = 0; Ability < 3 && Ability < View.HeroCooldownSeconds.Num(); ++Ability)
		{
			Out.HeroCooldownSeconds[Ability] = View.HeroCooldownSeconds[Ability];
		}
		const int32 Count = FMath::Min3(View.BuildableTypes.Num(), View.BuildableCells.Num(), View.BuildableAges.Num());
		for (int32 Index = 0; Index < Count; ++Index)
		{
			FHOLBuildable Buildable;
			Buildable.TypeName = View.BuildableTypes[Index];
			Buildable.Cell = View.BuildableCells[Index];
			Buildable.Age = View.BuildableAges[Index];
			Buildable.bAlwaysOpen = View.BuildableAlwaysOpen.IsValidIndex(Index) && View.BuildableAlwaysOpen[Index];
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
		Out.Ability = static_cast<int32>(Command.Ability);
		Out.CooldownSeconds = Command.CooldownSeconds;
		Out.bPack = Command.bPack;
		Out.Tier = Command.Tier;
		Out.Option = Command.Option;
		Out.RemainingMs = Command.RemainingMs;
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
		Out.Ability = static_cast<EHOLHeroAbility>(static_cast<uint8>(View.Ability));
		Out.CooldownSeconds = View.CooldownSeconds;
		Out.bPack = View.bPack;
		Out.Tier = View.Tier;
		Out.Option = View.Option;
		Out.RemainingMs = View.RemainingMs;
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
		Out.Block = static_cast<int32>(Feedback.Block);
		return Out;
	}

	FHOLCommandFeedback ToRule(const FHOLFeedbackView& View)
	{
		FHOLCommandFeedback Out;
		Out.Kind = static_cast<EHOLCommandFeedback>(static_cast<uint8>(View.Kind));
		Out.Resource = View.Resource;
		Out.Refusal = static_cast<EHOLPlacementRefusal>(View.Refusal);
		Out.Category = View.Category;
		Out.Block = static_cast<HOLTechnology::EBlock>(static_cast<uint8>(View.Block));
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

bool UHOLCommandGridReadLibrary::IsBuildGrid(const FHOLGridContextView& Context) { return HOLCommandGrid::IsBuildGrid(ToRule(Context)); }
bool UHOLCommandGridReadLibrary::IsAgeReached(const FHOLGridContextView& Context, const int32 Age) { return HOLCommandGrid::IsAgeReached(ToRule(Context), Age); }

FHOLGridCommandView UHOLCommandGridReadLibrary::GetAgeBuilding(const FHOLGridContextView& Context, const int32 Age, const int32 Cell)
{
	return FromRule(HOLCommandGrid::GetAgeBuilding(ToRule(Context), Age, Cell));
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
	return Type ? (Type->IsBuildable() ? Type->BuildGridSlot + 1 : 0) : INDEX_NONE;
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

bool UHOLCommandGridReadLibrary::IsPopulationBlocked(const int32 PopulationUsed, const int32 PopulationCap, const int32 Places)
{
	return HOLCommandGrid::IsPopulationBlocked(PopulationUsed, PopulationCap, Places);
}

FHOLGridCellFaceView UHOLCommandGridReadLibrary::GetCellFace(const bool bHasIcon)
{
	const FHOLGridCellFace Face = HOLCommandGrid::GetCellFace(bHasIcon);
	FHOLGridCellFaceView Out;
	Out.bIcon = Face.bIcon;
	Out.bName = Face.bName;
	Out.bCost = Face.bCost;
	return Out;
}

// ---- MIG-C-CommandGrid, 2nd lot (GALADRIEL-1117n). NOT COMPILED when written. ----

int32 UHOLCommandGridReadLibrary::GetAttackMoveCell() { return HOLCommandGrid::AttackMoveCell; }
int32 UHOLCommandGridReadLibrary::GetStopCell() { return HOLCommandGrid::StopCell; }
int32 UHOLCommandGridReadLibrary::GetHoldCell() { return HOLCommandGrid::HoldCell; }
int32 UHOLCommandGridReadLibrary::GetDeployCell() { return HOLCommandGrid::DeployCell; }
int32 UHOLCommandGridReadLibrary::GetResearchFirstCell() { return HOLCommandGrid::ResearchFirstCell; }
int32 UHOLCommandGridReadLibrary::GetNumAges() { return HOLCommandGrid::NumAges; }

int32 UHOLCommandGridReadLibrary::GetHeroAbilityAura() { return static_cast<int32>(EHOLHeroAbility::Aura); }
int32 UHOLCommandGridReadLibrary::GetHeroAbilityCloseRanks() { return static_cast<int32>(EHOLHeroAbility::CloseRanks); }
int32 UHOLCommandGridReadLibrary::GetHeroAbilityBanner() { return static_cast<int32>(EHOLHeroAbility::Banner); }

int32 UHOLCommandGridReadLibrary::GetHeroAbilityCell(const int32 Ability)
{
	return HOLCommandGrid::GetHeroAbilityCell(static_cast<EHOLHeroAbility>(static_cast<uint8>(Ability)));
}

int32 UHOLCommandGridReadLibrary::GetRageCell() { return HOLCommandGrid::GetRageCell(); }
int32 UHOLCommandGridReadLibrary::GetSpecializationCell(const int32 Option) { return HOLCommandGrid::GetSpecializationCell(Option); }
FName UHOLCommandGridReadLibrary::GetReportName(const FHOLGridCommandView& Command) { return HOLCommandGrid::GetReportName(ToRule(Command)); }

float UHOLCommandGridReadLibrary::GetAgePanelOpacity(const FHOLGridContextView& Context, const int32 Age) { return HOLCommandGrid::GetAgePanelOpacity(ToRule(Context), Age); }

int32 UHOLCommandGridReadLibrary::GetMouseCursor(const bool bAttackMoveArmed, const bool bPlacing)
{
	return static_cast<int32>(HOLCommandGrid::GetMouseCursor(bAttackMoveArmed, bPlacing));
}

int32 UHOLCommandGridReadLibrary::GetCursorCrosshairs() { return static_cast<int32>(EMouseCursor::Crosshairs); }
int32 UHOLCommandGridReadLibrary::GetCursorDefault() { return static_cast<int32>(EMouseCursor::Default); }

TArray<FName> UHOLCommandGridReadLibrary::GetBuildingResearches(const FName Building)
{
	TArray<FName> Names;
	UHOLTechnologySettings::GetBuildingTechnologies(Building, Names);
	return Names;
}

bool UHOLCommandGridReadLibrary::IsTechnologyInTable(const FName Technology) { return UHOLTechnologySettings::FindTechnology(Technology) != nullptr; }

bool UHOLCommandGridReadLibrary::IsTechnologyImplemented(const FName Technology)
{
	const FHOLTechnologyRow* Row = UHOLTechnologySettings::FindTechnology(Technology);
	return Row && Row->Implemented;
}

int32 UHOLCommandGridReadLibrary::GetTechnologyFoodCost(const FName Technology)
{
	const FHOLTechnologyRow* Row = UHOLTechnologySettings::FindTechnology(Technology);
	return Row ? Row->Cost.Food : -1;
}

FHOLFeedbackView UHOLCommandGridReadLibrary::CheckResearch(const FName Technology, const FHOLTechFactsView& Facts, const FHOLResources& Stock,
	const int32 QueueLength, const int32 MaxQueue)
{
	const FHOLTechnologyRow* Row = UHOLTechnologySettings::FindTechnology(Technology);
	if (!Row)
	{
		return FHOLFeedbackView();
	}
	HOLTechnology::FFacts Rule;
	Rule.HeroTier = Facts.HeroTier;
	Rule.bRequiresDone = Facts.bRequiresDone;
	Rule.bBuildingFinished = Facts.bBuildingFinished;
	Rule.bResearched = Facts.bResearched;
	Rule.bQueued = Facts.bQueued;
	return FromRule(HOLCommandGrid::CheckResearch(*Row, Rule, Stock, QueueLength, MaxQueue));
}

int32 UHOLCommandGridReadLibrary::GetBlockNone() { return static_cast<int32>(HOLTechnology::EBlock::None); }
int32 UHOLCommandGridReadLibrary::GetBlockNotImplemented() { return static_cast<int32>(HOLTechnology::EBlock::NotImplemented); }
int32 UHOLCommandGridReadLibrary::GetBlockAlreadyResearched() { return static_cast<int32>(HOLTechnology::EBlock::AlreadyResearched); }
int32 UHOLCommandGridReadLibrary::GetBlockAlreadyQueued() { return static_cast<int32>(HOLTechnology::EBlock::AlreadyQueued); }
int32 UHOLCommandGridReadLibrary::GetBlockTierTooLow() { return static_cast<int32>(HOLTechnology::EBlock::TierTooLow); }
int32 UHOLCommandGridReadLibrary::GetBlockRequiresMissing() { return static_cast<int32>(HOLTechnology::EBlock::RequiresMissing); }
int32 UHOLCommandGridReadLibrary::GetBlockNoBuilding() { return static_cast<int32>(HOLTechnology::EBlock::NoBuilding); }
