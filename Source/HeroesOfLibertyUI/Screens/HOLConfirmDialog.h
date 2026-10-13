#pragma once

#include "CoreMinimal.h"
#include "Screens/HOLScreen.h"
#include "HOLConfirmDialog.generated.h"

/**
 * Yes / no question in the modal layer (D241): blocks the screens below it; Back (Escape, gamepad B) and No close
 * it, Yes runs the given action then closes it. No has the focus, so that a stray Enter never confirms.
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLConfirmDialog : public UHOLScreen
{
	GENERATED_BODY()

public:
	UHOLConfirmDialog();

	void SetUp(const FText& Question, const FText& ConfirmLabel, TFunction<void()> InOnConfirm);

protected:
	virtual void BuildLayout() override;
	virtual void NativeOnInitialized() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UCommonTextBlock> Question;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> ConfirmButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> CancelButton;

private:
	void HandleConfirm();

	TFunction<void()> OnConfirm;
};
