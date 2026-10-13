#pragma once

#include "CoreMinimal.h"
#include "CommonRotator.h"
#include "HOLRotator.generated.h"

/**
 * Choice among a few values (‹ value ›), in the menu button style: Left and Right (keys, gamepad) rotate it, a click
 * takes the next value. Built in code when no Blueprint gives it a layout (D242). Greyed (D240) with SetIsEnabled.
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLRotator : public UCommonRotator
{
	GENERATED_BODY()

public:
	UHOLRotator(const FObjectInitializer& ObjectInitializer);

	/** Values and the one shown first. */
	void SetOptions(const TArray<FText>& Options, int32 InSelectedIndex = 0);

	/** +1 or -1: the way a rotator of Count values went from Old to New (to pass over a value that cannot be chosen). */
	static int32 GetStep(int32 Old, int32 New, int32 Count);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnCurrentTextStyleChanged() override;
	virtual void NativeOnClicked() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UCommonTextBlock> LeftArrow;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UCommonTextBlock> RightArrow;
};
