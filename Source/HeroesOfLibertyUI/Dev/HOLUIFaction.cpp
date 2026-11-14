#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyUILog.h"
#include "Match/HOLFactionChoice.h"
#include "Screens/HOLNewGameScreen.h"

/**
 * The faction cheat of the title screen (a triple click on its title) from the console, for the scripted trials
 * (FactionChoiceInProgress.md):
 *   hol.Faction.Cheat [0|1]   (no argument: switches it; the open New game screen shows the Templars or not at once)
 */
namespace UE::HeroesOfLiberty::UI
{
	void FactionCheat(const TArray<FString>& Args, UWorld* World)
	{
		const bool bOn = Args.IsEmpty() ? !HOLFactionChoice::IsCheatOn() : FCString::Atoi(*Args[0]) != 0;
		HOLFactionChoice::SetCheatOn(bOn);
		UE_LOG(LogHOLUI, Display, TEXT("Faction cheat %s"), bOn ? TEXT("on") : TEXT("off"));
		if (UHOLNewGameScreen* Screen = UHOLNewGameScreen::Find(World ? World->GetFirstPlayerController() : nullptr))
		{
			Screen->OnFactionCheatChanged();
		}
	}

	FAutoConsoleCommandWithWorldAndArgs FactionCheatCommand(
		TEXT("hol.Faction.Cheat"),
		TEXT("hol.Faction.Cheat [0|1]: the Templars in the faction list of the New game screen (as a triple click on the title screen's title); no argument switches it"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&FactionCheat));
}
