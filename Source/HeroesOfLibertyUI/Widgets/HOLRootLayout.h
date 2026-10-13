#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "HOLRootLayout.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetStack;

/**
 * Root of the interface (D241): a background and two layers of activatable screens, the menu and the modal one on
 * top of it (confirmations, later the duel challenge of D117). Back pops the top screen of a layer. Built in code
 * when no Blueprint gives it a layout (D242).
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

	/** The layout of the given player, if it has one on screen. */
	static UHOLRootLayout* Get(const APlayerController* PlayerController);

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetStack> MenuLayer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonActivatableWidgetStack> ModalLayer;
};
