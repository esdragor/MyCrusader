// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLVolcanoRulesLibrary.h"

#include "Engine/DataTable.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"

namespace UE::HeroesOfLiberty::VolcanoEnvelope
{
	HOLVolcano::FFlow Flow(const TArray<FVector>& Points)
	{
		HOLVolcano::FFlow Out;
		Out.Points = Points;
		return Out;
	}

	FHOLVolcanoCycleView View(const HOLVolcano::FCycle& C)
	{
		FHOLVolcanoCycleView Out;
		Out.WarningAt = C.WarningAt;
		Out.EruptAt = C.EruptAt;
		Out.EruptEndAt = C.EruptEndAt;
		Out.CooledAt = C.CooledAt;
		Out.ReArmAt = C.ReArmAt;
		return Out;
	}
}

using namespace UE::HeroesOfLiberty::VolcanoEnvelope;

FHOLVolcanoTuning::FHOLVolcanoTuning()
{
	const FHOLVolcanoSettings D;
	WindowOpensAfter = D.WindowOpensAfter;
	WindowLength = D.WindowLength;
	WarningMin = D.WarningMin;
	WarningMax = D.WarningMax;
	LavaSpeedFactor = D.LavaSpeedFactor;
	LavaHalfWidth = D.LavaHalfWidth;
	AltarAdvance = D.AltarAdvance;
	AltarHoldSeconds = D.AltarHoldSeconds;
	AltarDecayRate = D.AltarDecayRate;
	FreshDepositAlongMin = D.FreshDepositAlongMin;
	FreshDepositAlongMax = D.FreshDepositAlongMax;
	FreshDepositGoldShare = D.FreshDepositGoldShare;
	FreshDepositBigShare = D.FreshDepositBigShare;
	FreshDepositGoldSmallType = D.FreshDepositGoldSmallType;
	FreshDepositGoldBigType = D.FreshDepositGoldBigType;
	FreshDepositStoneSmallType = D.FreshDepositStoneSmallType;
	FreshDepositStoneBigType = D.FreshDepositStoneBigType;
}

FHOLVolcanoSettings FHOLVolcanoTuning::ToSettings() const
{
	FHOLVolcanoSettings S;
	S.WindowOpensAfter = WindowOpensAfter;
	S.WindowLength = WindowLength;
	S.WarningMin = WarningMin;
	S.WarningMax = WarningMax;
	S.LavaSpeedFactor = LavaSpeedFactor;
	S.LavaHalfWidth = LavaHalfWidth;
	S.AltarAdvance = AltarAdvance;
	S.AltarHoldSeconds = AltarHoldSeconds;
	S.AltarDecayRate = AltarDecayRate;
	S.FreshDepositAlongMin = FreshDepositAlongMin;
	S.FreshDepositAlongMax = FreshDepositAlongMax;
	S.FreshDepositGoldShare = FreshDepositGoldShare;
	S.FreshDepositBigShare = FreshDepositBigShare;
	S.FreshDepositGoldSmallType = FreshDepositGoldSmallType;
	S.FreshDepositGoldBigType = FreshDepositGoldBigType;
	S.FreshDepositStoneSmallType = FreshDepositStoneSmallType;
	S.FreshDepositStoneBigType = FreshDepositStoneBigType;
	return S;
}

float UHOLVolcanoRulesLibrary::ManAtArmsSpeed()
{
	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	const FHOLUnitTypeRow* Row = UnitTypes ? UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("ManAtArms"), TEXT("Volcano script"), false) : nullptr;
	return Row ? Row->Speed : 0.f;
}

FHOLVolcanoCycleView UHOLVolcanoRulesLibrary::MakeCycle(const int32 Seed, const int32 Index, const float PreviousReArmAt, const FHOLVolcanoTuning& Tuning, const float EruptionLength)
{
	return View(HOLVolcano::MakeCycle(Seed, Index, PreviousReArmAt, Tuning.ToSettings(), EruptionLength));
}

int32 UHOLVolcanoRulesLibrary::GetPhase(const bool bCatastrophe, const int32 Seed, const float Now, const FHOLVolcanoTuning& Tuning, const float EruptionLength)
{
	return static_cast<int32>(HOLVolcano::GetPhase(bCatastrophe, Seed, Now, Tuning.ToSettings(), EruptionLength));
}

float UHOLVolcanoRulesLibrary::FlowLength(const TArray<FVector>& Points) { return Flow(Points).Length(); }
float UHOLVolcanoRulesLibrary::EruptionDuration(const TArray<FVector>& Points, const float Speed) { return HOLVolcano::EruptionDuration(Flow(Points), Speed); }
float UHOLVolcanoRulesLibrary::FrontDistance(const TArray<FVector>& Points, const float Now, const float EruptAt, const float Speed) { return HOLVolcano::FrontDistance(Flow(Points), Now, EruptAt, Speed); }
bool UHOLVolcanoRulesLibrary::IsInLava(const TArray<FVector>& Points, const float FrontDistance, const float HalfWidth, const FVector& Point, const float Radius) { return HOLVolcano::IsInLava(Flow(Points), FrontDistance, HalfWidth, Point, Radius); }
bool UHOLVolcanoRulesLibrary::IsInCorridor(const TArray<FVector>& Points, const float HalfWidth, const FVector& Point, const float Radius) { return HOLVolcano::IsInCorridor(Flow(Points), HalfWidth, Point, Radius); }
FVector UHOLVolcanoRulesLibrary::EvacuationPoint(const TArray<FVector>& Points, const float HalfWidth, const FVector& Point, const float Radius, const float Beyond) { return HOLVolcano::EvacuationPoint(Flow(Points), HalfWidth, Point, Radius, Beyond); }
float UHOLVolcanoRulesLibrary::EvacuationReach(const float HalfWidth, const float Radius) { return HOLVolcano::EvacuationReach(HalfWidth, Radius); }
bool UHOLVolcanoRulesLibrary::LavaDestroys(const bool bIsHero, const bool bHeroInDuel) { return HOLVolcano::LavaDestroys(bIsHero, bHeroInDuel); }
FVector UHOLVolcanoRulesLibrary::PointAt(const TArray<FVector>& Points, const float Distance) { return HOLVolcano::PointAt(Flow(Points), Distance); }
float UHOLVolcanoRulesLibrary::DistanceToFlow(const TArray<FVector>& Points, const FVector& Point) { return HOLVolcano::DistanceToFlow(Flow(Points), Point); }
int32 UHOLVolcanoRulesLibrary::DrawFlow(const int32 Seed, const int32 Index, const int32 FlowCount) { return HOLVolcano::DrawFlow(Seed, Index, FlowCount); }
int32 UHOLVolcanoRulesLibrary::SiteSeed(const int32 MatchSeed, const int32 SiteIndex) { return HOLVolcano::SiteSeed(MatchSeed, SiteIndex); }
bool UHOLVolcanoRulesLibrary::FreshDepositDue(const int32 Phase, const int32 FlowIndex)
{
	HOLVolcano::FSiteState State;
	State.Phase = static_cast<EHOLVolcanoPhase>(Phase);
	State.FlowIndex = FlowIndex;
	return HOLVolcano::FreshDepositDue(State);
}

TArray<FHOLVolcanoBoxView> UHOLVolcanoRulesLibrary::GetObstacleBoxes(const TArray<FVector>& Points, const float HalfWidth)
{
	TArray<FHOLVolcanoBoxView> Out;
	for (const HOLVolcano::FObstacleBox& Box : HOLVolcano::GetObstacleBoxes(Flow(Points), HalfWidth))
	{
		FHOLVolcanoBoxView& View = Out.AddDefaulted_GetRef();
		View.Centre = Box.Centre;
		View.HalfLength = Box.HalfLength;
		View.HalfWidth = Box.HalfWidth;
		View.YawDegrees = Box.YawDegrees;
	}
	return Out;
}

int32 UHOLVolcanoRulesLibrary::StepAltar(float& HeldSeconds, const int32 CampsInZone, const float DeltaSeconds, const FHOLVolcanoTuning& Tuning)
{
	return static_cast<int32>(HOLVolcano::StepAltar(HeldSeconds, CampsInZone, DeltaSeconds, Tuning.ToSettings()));
}

int32 UHOLVolcanoRulesLibrary::StepAltarOwned(float& HeldSeconds, int32& Holder, const int32 CampsInZone, const int32 SoleCamp, const float DeltaSeconds, const FHOLVolcanoTuning& Tuning)
{
	return static_cast<int32>(HOLVolcano::StepAltarOwned(HeldSeconds, Holder, CampsInZone, SoleCamp, DeltaSeconds, Tuning.ToSettings()));
}

FHOLVolcanoCycleView UHOLVolcanoRulesLibrary::AdvanceForAltar(const FHOLVolcanoCycleView& Cycle, const float HeldAt, const FHOLVolcanoTuning& Tuning)
{
	HOLVolcano::FCycle In;
	In.WarningAt = Cycle.WarningAt;
	In.EruptAt = Cycle.EruptAt;
	In.EruptEndAt = Cycle.EruptEndAt;
	In.CooledAt = Cycle.CooledAt;
	In.ReArmAt = Cycle.ReArmAt;
	return View(HOLVolcano::AdvanceForAltar(In, HeldAt, Tuning.ToSettings()));
}

FHOLVolcanoFreshView UHOLVolcanoRulesLibrary::FreshDeposit(const int32 SiteSeedValue, const int32 CycleIndex, const TArray<FVector>& Points, const FHOLVolcanoTuning& Tuning)
{
	const HOLVolcano::FFreshDeposit D = HOLVolcano::FreshDeposit(SiteSeedValue, CycleIndex, Flow(Points), Tuning.ToSettings());
	FHOLVolcanoFreshView Out;
	Out.bPresent = D.bPresent;
	Out.Kind = static_cast<int32>(D.Kind);
	Out.bBig = D.bBig;
	Out.Along = D.Along;
	Out.Location = D.Location;
	Out.TypeName = D.TypeName;
	return Out;
}

FHOLVolcanoStateView UHOLVolcanoRulesLibrary::SiteStateAt(const bool bCatastrophe, const int32 MatchSeed, const int32 SiteIndex, const FHOLVolcanoSiteLayout& Site, const float Now, const FHOLVolcanoTuning& Tuning, const float ManAtArmsSpeedValue, const int32 WakeCycle, const float WakeAt)
{
	HOLVolcano::FSite Layout;
	for (const FHOLVolcanoFlowLine& Line : Site.Flows)
	{
		Layout.Flows.Add(Flow(Line.Points));
	}
	Layout.Altar.bPresent = Site.bAltarPresent;
	Layout.Altar.Location = Site.AltarLocation;
	HOLVolcano::FWakes Wakes;
	if (WakeCycle != INDEX_NONE)
	{
		Wakes.Add(WakeCycle, WakeAt);
	}
	const HOLVolcano::FSiteState S = HOLVolcano::SiteStateAt(bCatastrophe, MatchSeed, SiteIndex, Layout, Now, Tuning.ToSettings(), ManAtArmsSpeedValue, &Wakes);
	FHOLVolcanoStateView Out;
	Out.Phase = static_cast<int32>(S.Phase);
	Out.CycleIndex = S.CycleIndex;
	Out.FlowIndex = S.FlowIndex;
	Out.WarningAt = S.WarningAt;
	Out.EruptAt = S.EruptAt;
	Out.SecondsToErupt = S.SecondsToErupt;
	Out.FrontDistance = S.FrontDistance;
	Out.FrontPoint = S.FrontPoint;
	Out.bAltarLit = S.bAltarLit;
	Out.bWoken = S.bWoken;
	return Out;
}
