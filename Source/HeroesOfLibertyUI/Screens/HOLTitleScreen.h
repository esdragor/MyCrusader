#pragma once

#include "CoreMinimal.h"
#include "Screens/HOLScreen.h"
#include "HOLTitleScreen.generated.h"

/**
 * Title screen (D240): the game's name and the way into the rest of the menu. Provisional at front-end step 3
 * (title and Quit only, to check the Common UI chain); Play and Settings come at step 4.
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLTitleScreen : public UHOLScreen
{
	GENERATED_BODY()

protected:
	virtual void BuildLayout() override;
	virtual void NativeOnInitialized() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UCommonTextBlock> Title;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> QuitButton;

private:
	void HandleQuit();
};
