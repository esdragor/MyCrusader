#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "HOLScreen.generated.h"

class UCommonTextBlock;
class UHorizontalBox;
class UHOLButton;
class UHOLRotator;
class UVerticalBox;

/**
 * Base of the menu screens: an activatable widget in menu input mode, whose layout is built in code by
 * BuildLayout() when no Blueprint gives one (D242). Back (Escape, gamepad B) closes it, except where a screen says
 * otherwise. Helpers add the usual pieces to a centred column.
 */
UCLASS(Abstract)
class HEROESOFLIBERTYUI_API UHOLScreen : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UHOLScreen();

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

protected:
	virtual void NativeOnInitialized() override;

	/** Builds the layout in code; only called when no Blueprint gave one. */
	virtual void BuildLayout() {}

	/** A centred column, made the root of the tree. */
	UVerticalBox* MakeCenteredColumn();
	/** A centred column on a panel, over a veil that hides the screen below (dialogs of the modal layer). */
	UVerticalBox* MakeDialogColumn();

	UCommonTextBlock* AddText(UVerticalBox* Column, FName Name, const FText& Text, TSubclassOf<class UCommonTextStyle> TextStyle, float BottomPadding);
	UHOLButton* AddButton(UVerticalBox* Column, FName Name, const FText& Label);
	/** A row of buttons, centred, for the bottom of a screen. */
	UHorizontalBox* AddButtonRow(UVerticalBox* Column);
	UHOLButton* AddButton(UHorizontalBox* Row, FName Name, const FText& Label);
	/** A label and its rotator on one line. */
	UHOLRotator* AddRotatorRow(UVerticalBox* Column, FName Name, const FText& Label, const TArray<FText>& Options, int32 SelectedIndex,
		float LabelWidth = 360.f, float RotatorWidth = 380.f);
	/** A rotator of the given width, added to a row. */
	UHOLRotator* AddRotator(UHorizontalBox* Row, FName Name, const TArray<FText>& Options, int32 SelectedIndex, float Width);

	/** Closes this screen, as Back does. */
	void Close() { DeactivateWidget(); }
};
