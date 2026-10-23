// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeroesOfLiberty.h"
#include "HeroesOfLibertyLog.h"
#include "HOLVersion.h"
#include "Modules/ModuleManager.h"

class FHeroesOfLibertyModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		// One line to find in the log of the game and of the server (R1.7).
		UE_LOG(LogHOLNet, Display, TEXT("Version: %s"), *HOLVersion::Get());
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE( FHeroesOfLibertyModule, HeroesOfLiberty, "HeroesOfLiberty" );
