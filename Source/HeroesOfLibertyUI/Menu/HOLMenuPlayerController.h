#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HOLMenuPlayerController.generated.h"

class UHOLRootLayout;
class UHOLScreen;

/**
 * Player controller of the menu map (D243): gives the local player the interface keys (IMC_UI), puts the root
 * layout on screen and opens the title screen.
 */
UCLASS()
class HEROESOFLIBERTYUI_API AHOLMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AHOLMenuPlayerController();

	UHOLRootLayout* GetRootLayout() const { return RootLayout; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UHOLRootLayout> RootLayout;
};
