// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "HOLPlayerState.generated.h"

/**
 * A player of the match: the slot it holds in the match configuration (D244), which is the index its units carry
 * as OwnerPlayer (D233), and the colour the slot chose.
 */
UCLASS()
class HEROESOFLIBERTY_API AHOLPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	int32 GetPlayerIndex() const { return PlayerIndex; }
	int32 GetColorIndex() const { return ColorIndex; }

	/** Server only, by the game mode. */
	void SetMatchSlot(int32 InPlayerIndex, int32 InColorIndex);

private:
	/** Slot index in the match configuration, INDEX_NONE for a player with no slot. */
	UPROPERTY(Replicated)
	int32 PlayerIndex = INDEX_NONE;

	UPROPERTY(Replicated)
	int32 ColorIndex = 0;
};
