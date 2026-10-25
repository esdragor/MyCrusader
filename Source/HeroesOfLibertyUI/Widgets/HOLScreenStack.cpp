#include "Widgets/HOLScreenStack.h"

#include "CommonActivatableWidget.h"
#include "HeroesOfLibertyUILog.h"
#include "Slate/SCommonAnimatedSwitcher.h"

void UHOLScreenStack::OnWidgetAddedToList(UCommonActivatableWidget& AddedWidget)
{
	// The displayed screen deactivated while the switcher still heads up the stack: covered by the last push, not
	// closed. Forgotten as the displayed one, so that it stays in the stack under the new screen.
	if (MySwitcher && DisplayedWidget && !DisplayedWidget->IsActivated() && MySwitcher->IsTransitionPlaying()
		&& MySwitcher->GetPendingActiveWidgetIndex() > MySwitcher->GetActiveWidgetIndex())
	{
		UE_LOG(LogHOLUI, Display, TEXT("Screen stack: %s kept under %s (pushed while fading)"), *DisplayedWidget->GetName(), *AddedWidget.GetName());
		DisplayedWidget = nullptr;
	}
	Super::OnWidgetAddedToList(AddedWidget);
}
