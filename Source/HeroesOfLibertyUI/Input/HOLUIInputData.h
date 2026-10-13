#pragma once

#include "CoreMinimal.h"
#include "CommonInputBaseTypes.h"
#include "HOLUIInputData.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * Common UI input (D241) through Enhanced Input, the system the Commands tab will remap at M4 (D245): confirm and
 * back actions, and the mapping context that gives them their keys. Assets made by Tools/Editor/make_ui_input.py.
 * Declared in DefaultGame.ini ([/Script/CommonInput.CommonInputSettings] InputData).
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLUIInputData : public UCommonUIInputData
{
	GENERATED_BODY()

public:
	UHOLUIInputData();

	/** Added to each local player of a menu, so Common UI finds the keys of the confirm and back actions. */
	static UInputMappingContext* GetMappingContext();

	/** Move the focus to the next or previous column of a screen (gamepad RB and LB, D250). */
	static const UInputAction* GetNextColumnAction();
	static const UInputAction* GetPreviousColumnAction();
};
