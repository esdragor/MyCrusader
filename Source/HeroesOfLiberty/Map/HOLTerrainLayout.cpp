// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLTerrainLayout.h"

AHOLTerrainLayout::AHOLTerrainLayout()
{
	PrimaryActorTick.bCanEverTick = false;
	// Data for the server: the speed of a walker is the server's (a client receives the positions).
	bNetLoadOnClient = false;
	SetCanBeDamaged(false);
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}
