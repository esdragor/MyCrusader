#pragma once

#include "CoreMinimal.h"
#include "Screens/HOLScreen.h"
#include "HOLTitleScreen.generated.h"

/**
 * Title screen (D240): the game's name and the way into the rest of the menu (Play, Settings, Quit). Root of the
 * menu layer: Back does nothing here, Quit asks for a confirmation in the modal layer.
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLTitleScreen : public UHOLScreen
{
	GENERATED_BODY()

public:
	UHOLTitleScreen();

	/** Opens the quit confirmation over the menu of the given player (Quit button, hol.UI.Open Quit). */
	static void AskToQuit(APlayerController* PlayerController);

protected:
	virtual void BuildLayout() override;
	virtual void NativeOnInitialized() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UCommonTextBlock> Title;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> PlayButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> SettingsButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> QuitButton;

private:
	void HandlePlay();
	void HandleSettings();
	void HandleQuit();
};
