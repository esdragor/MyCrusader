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
	if (Label)
	{
		Label->SetText(InLabel);
	}
}
