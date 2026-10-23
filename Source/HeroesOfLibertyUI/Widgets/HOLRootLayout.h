#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "HOLRootLayout.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetStack;

/**
 * Root of the interface (D241): a background and two layers of activatable screens, the menu and the modal one on
 * top of it (confirmations, later the duel challenge of D117). Back pops the top screen of a layer. Built in code
 * when no Blueprint gives it a layout (D242). Also over a match, for the in-game menu (D356): no background then, the
 * game seen and clicked through it while no screen is open.
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLRootLayout : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UCommonActivatableWidget* PushMenu(TSubclassOf<UCommonActivatableWidget> ScreenClass);
	UCommonActivatableWidget* PushModal(TSubclassOf<UCommonActivatableWidget> ScreenClass);

	/** The screen on top of the menu layer. */
	UCommonActivatableWidget* GetActiveMenu() const;
	/** The menu layer holds a screen other than the given one (V1.4a: the end banner under the closing in-game menu). */
	bool HasMenuOtherThan(const UCommonActivatableWidget* Screen) const;
	/** The screen on top of the modal layer (a confirmation), if any. */
	UCommonActivatableWidget* GetActiveModal() const;

	/** The layout of the given player, if it has one on screen: the menu's, or the match's (AHOLGameHUD). */
	static UHOLRootLayout* Get(const APlayerController* PlayerController);

	/** Made over a match (its player not the menu's): no background, no scripted checks' hooks. */
	bool IsOverMatch() const;

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetStack> MenuLayer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetStack> ModalLayer;

private:
	/** A screen of the layer is fading in or out (scripted checks: HOLTestHooks). */
	bool bMenuFading = false;
	bool bModalFading = false;
};
