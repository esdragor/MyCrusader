// Copyright Epic Games, Inc. All Rights Reserved.

#include "Resources/HOLSourceLayout.h"

AHOLSourceLayout::AHOLSourceLayout()
{
	PrimaryActorTick.bCanEverTick = false;
	// Data for the server: a client does not load it (it receives the sources, D297).
	bNetLoadOnClient = false;
	SetCanBeDamaged(false);
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}
