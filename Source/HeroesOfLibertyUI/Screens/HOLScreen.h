#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "CommonInputTypeEnum.h"
#include "Input/UIActionBindingHandle.h"
#include "HOLScreen.generated.h"

class UCommonTextBlock;
class UHorizontalBox;
class UHOLButton;
class UHOLRotator;
class UVerticalBox;

/**
 * Base of the menu screens: an activatable widget in menu input mode, whose layout is built in code by
 * BuildLayout() when no Blueprint gives one (D242). Back (Escape, gamepad B) closes it, except where a screen says
 * otherwise. On a screen with columns, gamepad LB and RB move the focus to the previous or next column, Left and Right
 * keeping to change the value of a rotator (D250). Helpers add the usual pieces to a centred column.
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
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

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

	/**
	 * The columns of widgets that LB and RB move between (D250), left to right, as they are shown now; none by default.
	 * Greyed or hidden widgets are passed over.
	 */
	virtual void GetColumns(TArray<TArray<UWidget*>>& OutColumns) const {}

	/** The note "LB / RB: change column", shown only while the player uses a gamepad. */
	UCommonTextBlock* AddColumnHint(UVerticalBox* Column);

private:
	/** Moves the focus to the next (+1) or previous (-1) column, onto the widget nearest in height to the focused one. */
	void MoveColumn(int32 Direction);
	void HandleInputMethodChanged(ECommonInputType InputType);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCommonTextBlock>> ColumnHints;

	TArray<FUIActionBindingHandle> ColumnActionHandles;
};
