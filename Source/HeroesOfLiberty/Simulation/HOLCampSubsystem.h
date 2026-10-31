// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Simulation/HOLCampRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLCampSubsystem.generated.h"

namespace HOLCamps
{
	/** The unit is a member of a neutral camp that stands at its post or walks back to it: it takes on nobody by itself (the combat processor reads it). */
	HEROESOFLIBERTY_API bool IsPassive(uint32 UnitId);

	/** A hit on a neutral unit, noted by the combat processor (melee and arrows) for the camp it belongs to; Damage in health points. */
	HEROESOFLIBERTY_API void NoteHit(uint32 VictimId, int32 AttackerOwner, float Damage);
}

/**
 * The neutral camps of a match (NeutralsInProgress.md N3 to N5, GDD 8.4, D32), on the server (and in solo). A camp is a post and a
 * group of units of the Neutral owner (FHOLMatchConfig::NeutralOwner, hostile to every slot, D401). Its thoughts come from
 * HOLCampRules (the twin of Script/Neutrals/HOLCampRules.as): at the tail of every fourth step the camp reads its living members
 * and the adversary units near its post, takes the step, and acts on it: guarding or returning members are passive (the combat
 * processor leaves them alone), fighting ones are not and are sent at the intruder (attack-move, a server order), a cleared camp
 * is cleared for good and its XP is shared out among the heroes by the credit rule (Exploration source). The cvars hol.Camp.Aggro
 * (0 on sight, 1 when attacked: NQ1) and hol.Camp.Credit (0 last blow, 1 most damage, 2 shared: NQ2) pick the two open design
 * choices. Camps come from hol.Camp.Add for now (the maps place none yet: N2).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLCampSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	/** A camp of Count units of TypeName around Post (cm); its id. The units are born at the next step and adopted then. */
	int32 AddCamp(const FVector& Post, FName TypeName, int32 Count);

	int32 GetNumCamps() const { return Camps.Num(); }
	HOLCampRules::EState GetCampState(int32 CampId) const { return Camps.IsValidIndex(CampId) ? Camps[CampId].State : HOLCampRules::EState::Cleared; }

	/** One line per camp (hol.Camp.Report). */
	FString MakeReport() const;

	/** The rules' settings from the cvars. */
	static HOLCampRules::FSettings MakeSettings();

private:
	struct FCamp
	{
		FVector Post = FVector::ZeroVector;
		TArray<uint32> MemberIds;
		HOLCampRules::EState State = HOLCampRules::EState::Guarding;
		TArray<HOLCampRules::FHit> Hits;
		bool bAdopted = false;
		bool bAttacked = false;
		int32 Members = 0;
		int32 Size = 0;
	};

	void OnStepEnded(int64 StepIndex);

	TArray<FCamp> Camps;
	FDelegateHandle StepEndedHandle;
};
