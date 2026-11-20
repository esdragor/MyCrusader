#pragma once

#include "CoreMinimal.h"

class UWidgetTree;

namespace UE::HeroesOfLiberty::UI
{
	/**
	 * hol.UI.Type's reach (D872, GALADRIEL-1120pf): puts Text in the UEditableTextBox named Name found in Tree or in the trees of the
	 * user widgets it holds (the game screen's chat field "ChatInputBox", built by the script inside the HUD). False: no such text box.
	 */
	bool TypeIntoTree(const UWidgetTree* Tree, FName Name, const FString& Text);
}