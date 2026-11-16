// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Simulation/HOLSiteBoardTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLSiteBoardSubsystem.generated.h"

/**
 * The sacred-site rules where the match runs (MIG-CPP-Site, D836 point 3: the script is the source of the rule). The thinnest bridge, as
 * UHOLUnitTierSubsystem: the server (UHOLSiteSubsystem::OnStepEnded) asks its script subclass (Script/Sites/HOLSiteBoardScript.as, the rules
 * Script/Sites/HOLSiteRules.as HOLSite::Step and Script/Sites/HOLSiteVictory.as HOLSiteVictory::Step) for the site after one step and for the
 * victory count after one step. Abstract: only a script subclass is born. Without it (no script compiled) StepSite and VictoryStep fall back to
 * the C++ rules HOLSite::Step and HOLSite::VictoryStep, so the behaviour is the same as before the bridge; the C++ rules are kept in this lot.
 *
 * PERFORMANCE: the script is called ONCE per site and per simulation step (StepSite) and ONCE per step (VictoryStep), with the counts the C++
 * has already aggregated (HOLSite::ResetCounts / AddUnit stay C++, in the Mass walk of the units); never once per unit.
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLSiteBoardSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The world's site board (its script subclass), or null (no script compiled, or no world). Find it once a step, not once a site. */
	static UHOLSiteBoardSubsystem* Find(const UWorld* World);

	/** The site after one step: the board's answer, or HOLSite::Step's when Board is null (the same rule). The server's call, once per site and step. */
	static FHOLSiteState StepSite(UHOLSiteBoardSubsystem* Board, const FHOLSiteState& State, const TArray<int32>& Takers, const TArray<int32>& Disturbers,
		const TArray<int32>& TeamBySlot, int32 HoldSteps, int32 DecaySteps);

	/** The victory count after one step: the board's answer, or HOLSite::VictoryStep's when Board is null. The server's call, once per step. */
	static FHOLSiteVictoryState VictoryStep(UHOLSiteBoardSubsystem* Board, const FHOLSiteVictoryState& State, const TArray<int32>& OwnerBySite,
		const TArray<int32>& ContestedBySite, const TArray<int32>& TeamBySlot, int32 VictorySteps);

	/** The board's answers (the native events below, i.e. the script rules in the script subclass). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Site")
	FHOLSiteState StepSiteNow(const FHOLSiteState& State, const TArray<int32>& Takers, const TArray<int32>& Disturbers, const TArray<int32>& TeamBySlot,
		int32 HoldSteps, int32 DecaySteps);
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Site")
	FHOLSiteVictoryState VictoryStepNow(const FHOLSiteVictoryState& State, const TArray<int32>& OwnerBySite, const TArray<int32>& ContestedBySite,
		const TArray<int32>& TeamBySlot, int32 VictorySteps);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** The site after one step. The default implementation is the C++ rule HOLSite::Step. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Site")
	FHOLSiteState StepSiteRule(const FHOLSiteState& State, const TArray<int32>& Takers, const TArray<int32>& Disturbers, const TArray<int32>& TeamBySlot,
		int32 HoldSteps, int32 DecaySteps);
	virtual FHOLSiteState StepSiteRule_Implementation(const FHOLSiteState& State, const TArray<int32>& Takers, const TArray<int32>& Disturbers,
		const TArray<int32>& TeamBySlot, int32 HoldSteps, int32 DecaySteps);

	/** The victory count after one step. The default implementation is the C++ rule HOLSite::VictoryStep. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Site")
	FHOLSiteVictoryState VictoryStepRule(const FHOLSiteVictoryState& State, const TArray<int32>& OwnerBySite, const TArray<int32>& ContestedBySite,
		const TArray<int32>& TeamBySlot, int32 VictorySteps);
	virtual FHOLSiteVictoryState VictoryStepRule_Implementation(const FHOLSiteVictoryState& State, const TArray<int32>& OwnerBySite,
		const TArray<int32>& ContestedBySite, const TArray<int32>& TeamBySlot, int32 VictorySteps);
};
