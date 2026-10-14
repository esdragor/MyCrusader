#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "HOLQueueItem.generated.h"

class UCommonTextBlock;

/**
 * An item of the production queue in the HUD (J3, D282): the name of the unit in production or waiting; a click
 * cancels it. Never takes the keyboard focus, as the cells of the command grid. Built in code when no Blueprint gives
 * it a layout (D242).
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLQueueItem : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UHOLQueueItem();

	void SetLabel(const FText& InLabel);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnCurrentTextStyleChanged() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Label;
};
