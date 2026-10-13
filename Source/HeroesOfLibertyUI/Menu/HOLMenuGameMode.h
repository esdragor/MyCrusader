#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HOLMenuGameMode.generated.h"

/** Game mode of the MainMenu map (D243): no pawn, the menu player controller. Set as the map's override. */
UCLASS()
class HEROESOFLIBERTYUI_API AHOLMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHOLMenuGameMode();
};
