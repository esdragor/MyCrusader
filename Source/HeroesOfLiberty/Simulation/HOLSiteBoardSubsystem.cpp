// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLSiteBoardSubsystem.h"

#include "Engine/World.h"

namespace
{
	/** The sacred sites have no rule without the script board: say it once (an error, the sites stay as they are), never once per step. */
	void ReportNoRule(const TCHAR* Where)
	{
		static bool bReported = false;
		if (!bReported)
		{
			bReported = true;
			UE_LOG(LogTemp, Error, TEXT("Sacred sites: no script board (Script/Sites/HOLSiteBoardScript.as not compiled): %s leaves the state unchanged, the sites never change hands."), Where);
		}
	}
}

UHOLSiteBoardSubsystem* UHOLSiteBoardSubsystem::Find(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	const TArray<UHOLSiteBoardSubsystem*> Boards = World->GetSubsystemArrayCopy<UHOLSiteBoardSubsystem>();
	return Boards.Num() > 0 ? Boards[0] : nullptr;
}

FHOLSiteState UHOLSiteBoardSubsystem::StepSite(UHOLSiteBoardSubsystem* Board, const FHOLSiteState& State, const TArray<int32>& Takers,
	const TArray<int32>& Disturbers, const TArray<int32>& TeamBySlot, const int32 HoldSteps, const int32 DecaySteps)
{
	if (Board)
	{
		return Board->StepSiteNow(State, Takers, Disturbers, TeamBySlot, HoldSteps, DecaySteps);
	}
	// No board script: no rule (the C++ copy is gone), the site is unchanged.
	ReportNoRule(TEXT("StepSite without a board"));
	return State;
}

FHOLSiteVictoryState UHOLSiteBoardSubsystem::VictoryStep(UHOLSiteBoardSubsystem* Board, const FHOLSiteVictoryState& State, const TArray<int32>& OwnerBySite,
	const TArray<int32>& ContestedBySite, const TArray<int32>& TeamBySlot, const int32 VictorySteps)
{
	if (Board)
	{
		return Board->VictoryStepNow(State, OwnerBySite, ContestedBySite, TeamBySlot, VictorySteps);
	}
	ReportNoRule(TEXT("VictoryStep without a board"));
	return State;
}

int32 UHOLSiteBoardSubsystem::GoldStep(UHOLSiteBoardSubsystem* Board, int32& Carry, const int32 Sites, const int32 RatePerMinute)
{
	if (Board)
	{
		return Board->GoldStepNow(Carry, Sites, RatePerMinute);
	}
	ReportNoRule(TEXT("GoldStep without a board"));
	return 0;
}

int32 UHOLSiteBoardSubsystem::GoldStepNow(int32& Carry, const int32 Sites, const int32 RatePerMinute)
{
	return GoldStepRule(Carry, Sites, RatePerMinute);
}

FHOLSiteState UHOLSiteBoardSubsystem::StepSiteNow(const FHOLSiteState& State, const TArray<int32>& Takers, const TArray<int32>& Disturbers,
	const TArray<int32>& TeamBySlot, const int32 HoldSteps, const int32 DecaySteps)
{
	return StepSiteRule(State, Takers, Disturbers, TeamBySlot, HoldSteps, DecaySteps);
}

FHOLSiteVictoryState UHOLSiteBoardSubsystem::VictoryStepNow(const FHOLSiteVictoryState& State, const TArray<int32>& OwnerBySite,
	const TArray<int32>& ContestedBySite, const TArray<int32>& TeamBySlot, const int32 VictorySteps)
{
	return VictoryStepRule(State, OwnerBySite, ContestedBySite, TeamBySlot, VictorySteps);
}

bool UHOLSiteBoardSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Never the abstract C++ class, nor a script class replaced by a reload (as UHOLUnitTierSubsystem).
	if (IsUnreachable() || GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UHOLSiteBoardSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

FHOLSiteState UHOLSiteBoardSubsystem::StepSiteRule_Implementation(const FHOLSiteState& State, const TArray<int32>& Takers, const TArray<int32>& Disturbers,
	const TArray<int32>& TeamBySlot, const int32 HoldSteps, const int32 DecaySteps)
{
	// No rule here: the rule is the script subclass's (the C++ copy is gone). The inert and safe answer: the site is left as it was.
	ReportNoRule(TEXT("StepSiteRule (abstract C++ default)"));
	return State;
}

FHOLSiteVictoryState UHOLSiteBoardSubsystem::VictoryStepRule_Implementation(const FHOLSiteVictoryState& State, const TArray<int32>& OwnerBySite,
	const TArray<int32>& ContestedBySite, const TArray<int32>& TeamBySlot, const int32 VictorySteps)
{
	ReportNoRule(TEXT("VictoryStepRule (abstract C++ default)"));
	return State;
}

int32 UHOLSiteBoardSubsystem::GoldStepRule_Implementation(int32& Carry, const int32 Sites, const int32 RatePerMinute)
{
	ReportNoRule(TEXT("GoldStepRule (abstract C++ default)"));
	return 0;
}
