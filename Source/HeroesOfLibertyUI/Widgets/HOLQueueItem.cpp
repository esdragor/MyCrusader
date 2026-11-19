#include "Widgets/HOLQueueItem.h"

#include "Blueprint/WidgetTree.h"
#include "CommonTextBlock.h"
#include "Style/HOLUIStyles.h"

UHOLQueueItem::UHOLQueueItem()
{
	Style = UHOLHUDButtonStyle::StaticClass();
	SetIsFocusable(false);
}

void UHOLQueueItem::NativeOnInitialized()
{
	// Runs inside UCommonButtonBase::Initialize, before it wraps the root widget into its internal button.
	if (!WidgetTree->RootWidget)
	{
		Label = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("Label"));
		Label->SetJustification(ETextJustify::Center);
		WidgetTree->RootWidget = Label;
	}
	// A label given before the widget was initialized (a script panel builds the button and labels it at once, H-3).
	if (Label && !LabelText.IsEmpty())
	{
		Label->SetText(LabelText);
	}
	Super::NativeOnInitialized();
}

void UHOLQueueItem::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();
	if (Label)
	{
		Label->SetStyle(GetCurrentTextStyleClass());
	}
}

void UHOLQueueItem::SetLabel(const FText& InLabel)
{
	LabelText = InLabel;
	if (Label)
	{
		Label->SetText(InLabel);
	}
}
