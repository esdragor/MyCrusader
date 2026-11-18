#pragma once

#include "CoreMinimal.h"
#include "Screens/HOLScreen.h"
#include "HOLConfirmDialogScreen.generated.h"

class UCommonTextBlock;
class UHorizontalBox;
class UHOLButton;
class UVerticalBox;

/**
 * Yes / no question in the modal layer (D241, MIG-CPP bascule ConfirmDialog, D830/D835): blocks the screens below it; Back
 * (Escape, gamepad B) and No close it, Yes runs the given action then closes it. No has the focus, so that a stray Enter
 * never confirms. Its layout, its two buttons and their clicks are written in script (a class deriving from this one,
 * FindScriptClass: Script/UI/HOLConfirmDialog.as); this base keeps what the script cannot reach: the opening (the root
 * layout), the action (a C++ callback, not a dynamic delegate: the callers are C++ lambdas) and the widget helpers.
 */
UCLASS(Abstract, Blueprintable)
class HEROESOFLIBERTYUI_API UHOLConfirmDialogScreen : public UHOLScreen
{
	GENERATED_BODY()

public:
	UHOLConfirmDialogScreen();

	/**
	 * Pushes the question over the screens of this player (the script's class) and sets it up; null when it could not open.
	 * An empty ConfirmLabel keeps "Oui" (D356: "Abandonner la partie ?" Oui / Non).
	 */
	static UHOLConfirmDialogScreen* Ask(APlayerController* PlayerController, const FText& InQuestion, const FText& ConfirmLabel, TFunction<void()> InOnConfirm);

	/** The script's dialog (the newest class deriving from this one, as UHOLScriptScreen::FindScriptClass); null: none. */
	UFUNCTION(BlueprintPure, Category = "HOL|ConfirmDialog")
	static TSubclassOf<UHOLConfirmDialogScreen> FindScriptClass();

	/** The default labels ("Oui" / "Non", localised in this C++ namespace HOLConfirmDialog). */
	UFUNCTION(BlueprintPure, Category = "HOL|ConfirmDialog")
	static FText YesLabel();
	UFUNCTION(BlueprintPure, Category = "HOL|ConfirmDialog")
	static FText NoLabel();

	/** UHOLScreen::MakeDialogColumn for the script. */
	UFUNCTION(BlueprintCallable, Category = "HOL|ConfirmDialog")
	UVerticalBox* MakeConfirmColumn();

	/** The question (heading style, 8 below), UHOLScreen::AddText for the script. */
	UFUNCTION(BlueprintCallable, Category = "HOL|ConfirmDialog")
	UCommonTextBlock* AddConfirmText(UVerticalBox* Column, FName Name, const FText& Text);

	UFUNCTION(BlueprintCallable, Category = "HOL|ConfirmDialog")
	UHorizontalBox* AddConfirmRow(UVerticalBox* Column);

	/** UHOLScreen::AddButton for the script; its clicks: OnButtonBaseClicked. */
	UFUNCTION(BlueprintCallable, Category = "HOL|ConfirmDialog")
	UHOLButton* AddConfirmButton(UHorizontalBox* Row, FName Name, const FText& Label);

	/** "Oui": closes the dialog then runs the action. */
	UFUNCTION(BlueprintCallable, Category = "HOL|ConfirmDialog")
	void Confirm();

	/** "Non": closes the dialog. */
	UFUNCTION(BlueprintCallable, Category = "HOL|ConfirmDialog")
	void Cancel();

	/** Set by the script (BuildDialog): the question's text and the two buttons. */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|ConfirmDialog")
	TObjectPtr<UCommonTextBlock> Question;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|ConfirmDialog")
	TObjectPtr<UHOLButton> ConfirmButton;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|ConfirmDialog")
	TObjectPtr<UHOLButton> CancelButton;

protected:
	virtual void BuildLayout() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	/** The script builds the dialog's column (in place of BuildLayout). */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|ConfirmDialog")
	void BP_BuildDialog();

private:
	void SetUp(const FText& InQuestion, const FText& ConfirmLabel, TFunction<void()> InOnConfirm);

	TFunction<void()> OnConfirm;
};
