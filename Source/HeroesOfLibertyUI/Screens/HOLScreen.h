#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "HOLScreen.generated.h"

class UCommonTextBlock;
class UHOLButton;
class UVerticalBox;

/**
 * Base of the menu screens: an activatable widget in menu input mode, whose layout is built in code by
 * BuildLayout() when no Blueprint gives one (D242). Helpers add the usual pieces to a centred column.
 */
UCLASS(Abstract)
class HEROESOFLIBERTYUI_API UHOLScreen : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

protected:
	virtual void NativeOnInitialized() override;

	/** Builds the layout in code; only called when no Blueprint gave one. */
	virtual void BuildLayout() {}

	/** A centred column, made the root of the tree. */
	UVerticalBox* MakeCenteredColumn();
	UCommonTextBlock* AddText(UVerticalBox* Column, FName Name, const FText& Text, TSubclassOf<class UCommonTextStyle> TextStyle, float BottomPadding);
	UHOLButton* AddButton(UVerticalBox* Column, FName Name, const FText& Label);
};
