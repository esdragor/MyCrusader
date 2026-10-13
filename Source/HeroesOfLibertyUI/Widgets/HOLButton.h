#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "HOLButton.generated.h"

class UCommonTextBlock;

/**
 * Menu button: a label in the menu button style. Built in code when no Blueprint gives it a layout (D242).
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UHOLButton();

	void SetLabel(const FText& InLabel);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnCurrentTextStyleChanged() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Label;
};
