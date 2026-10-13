#include "Menu/HOLMenuGameMode.h"

#include "Menu/HOLMenuPlayerController.h"

AHOLMenuGameMode::AHOLMenuGameMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = AHOLMenuPlayerController::StaticClass();
}
