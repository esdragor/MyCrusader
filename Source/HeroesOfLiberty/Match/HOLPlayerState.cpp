// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLPlayerState.h"

#include "Net/UnrealNetwork.h"

void AHOLPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHOLPlayerState, PlayerIndex);
	DOREPLIFETIME(AHOLPlayerState, ColorIndex);
}

void AHOLPlayerState::SetMatchSlot(const int32 InPlayerIndex, const int32 InColorIndex)
{
	PlayerIndex = InPlayerIndex;
	ColorIndex = InColorIndex;
}
