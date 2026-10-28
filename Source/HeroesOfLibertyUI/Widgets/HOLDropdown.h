#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CommonButtonBase.h"
#include "Components/MenuAnchor.h"
#include "Widgets/HOLButton.h"
#include "HOLDropdown.generated.h"

class UCommonTextBlock;
class UHOLDropdownList;
class UImage;

/** The anchor of a drop-down list: no engine menu background, the list draws its own panel. */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLMenuAnchor : public UMenuAnchor
{
	GENERATED_BODY()

public:
	UHOLMenuAnchor(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
		InitShowMenuBackground(false);
	}
};

/**
 * Choice in a drop-down list (value and a down chevron), in the menu button style: a click (or Accept) opens the list under it, a click
 * on a value (or Accept) takes it, Back or a click outside closes the list without changing anything. Up and Down move
 * between the rows of the screen while the list is closed. A value can be greyed (D240: not in the game yet, or held by
 * another slot); the whole list is greyed with SetIsEnabled. Built in code when no Blueprint gives it a layout (D242).
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLDropdown : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UHOLDropdown(const FObjectInitializer& ObjectInitializer);

	/** Values and the one shown first. */
	void SetOptions(const TArray<FText>& InOptions, int32 InSelectedIndex = 0);
	/** A colour square before each value in the list (player colours); none if empty. */
	void SetOptionSwatches(const TArray<FLinearColor>& InSwatches);
	/** Greys one value in the list: it is shown but cannot be taken. */
	void SetOptionEnabled(int32 Index, bool bEnabled);

	/** Shows a value, without broadcasting OnSelectionChanged. */
	void SetSelectedIndex(int32 Index);
	int32 GetSelectedIndex() const { return SelectedIndex; }
	/** The text the dropdown shows (its value's, unless the screen shows another one); hol.UI.Text for the scripted trials. */
	FText GetShownText() const;
	/** Shows another text than the value's (IA0.6 S2: the slot's name), until the next SetSelectedIndex; the list keeps its values. */
	void SetShownText(const FText& Text);

	bool IsListOpen() const;
	/** Opens the list (as a click does) or closes it without a choice; for the scripted trials. */
	void OpenList();
	void CloseList();
	/** A value taken in the list (as a click on it does; also hol.UI.Choose for the scripted trials). */
	void Choose(int32 Index);

	/** The player took a value in the list (a different one from the value shown). */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnSelectionChanged, int32 /*Index*/);
	FOnSelectionChanged OnSelectionChanged;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnCurrentTextStyleChanged() override;
	virtual void NativeOnClicked() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLMenuAnchor> Anchor;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> ValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Arrow;

private:
	friend class UHOLDropdownList;

	UFUNCTION()
	UUserWidget* MakeList();
	UFUNCTION()
	void HandleListOpenChanged(bool bIsOpen);

	TArray<FText> Options;
	TArray<FLinearColor> Swatches;
	TArray<bool> OptionEnabled;
	int32 SelectedIndex = 0;
	/** When the list last closed: the click that closes it by landing on the dropdown must not open it again. */
	double ListClosedTime = -1.0;

	UPROPERTY(Transient)
	TObjectPtr<UHOLDropdownList> List;
};

/** A value of an open drop-down list: a menu button with an optional colour square before its label. */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLDropdownOption : public UHOLButton
{
	GENERATED_BODY()

public:
	void SetSwatch(TOptional<FLinearColor> Color);

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Swatch;
};

/** The open list of a UHOLDropdown, shown in a pop-up under it. Back closes it; navigation stays inside it. */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLDropdownList : public UUserWidget
{
	GENERATED_BODY()

public:
	void Build(UHOLDropdown& InOwner, float MinWidth);
	/** Gives the focus to the value shown by the dropdown (or the first one that can be taken). */
	void FocusSelected();
	UHOLDropdownOption* GetOption(int32 Index) const { return Buttons.IsValidIndex(Index) ? Buttons[Index].Get() : nullptr; }

protected:
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	TWeakObjectPtr<UHOLDropdown> Owner;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLDropdownOption>> Buttons;
};
