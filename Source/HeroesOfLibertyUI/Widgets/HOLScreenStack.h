#pragma once

#include "CoreMinimal.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "HOLScreenStack.generated.h"

/**
 * The stack of a layer of the root layout. Common UI takes a displayed screen already deactivated, when a screen is
 * pushed, for one closed by something else, and drops it from the stack once the switcher lands. That holds for a
 * screen closing (Back), not for one covered by a push still fading in: the menu's BeginPlay back from a lobby's match
 * (title, Multiplayer, then the lobby) lost the title that way, Back on the list then showed an empty layer (E2-3).
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLScreenStack : public UCommonActivatableWidgetStack
{
	GENERATED_BODY()

protected:
	virtual void OnWidgetAddedToList(UCommonActivatableWidget& AddedWidget) override;
};
