// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Simulation/HOLSanityRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLSanitySubsystem.generated.h"

/**
 * The motion audit of a scenario (Auto_Journey_File.md, "Processus"): hol.Test.SanityMotion <Seconds> records the place and the
 * orders of every unit at each simulation step for Seconds of game time, then logs one line per unit that spins or shakes
 * ("Sanity: FAIL ...") and a last line "Sanity: OK motion ..." or "Sanity: FAIL motion ..." (HOLSanityRules.h). Solo or server.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLSanitySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	/** Starts recording; the verdict is logged Seconds later. */
	void StartMotion(float Seconds);

	/** The verdict on what was recorded (logs it); the number of units that spin or shake. */
	int32 Judge();

private:
	void OnStepEnded(int64 StepIndex);

	bool bRecording = false;
	int32 StepsToRecord = 0;
	TMap<uint32, HOLSanity::FTrace> Traces;
	FDelegateHandle StepEndedHandle;
};
