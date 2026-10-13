#include "Widgets/HOLButton.h"

#include "Blueprint/WidgetTree.h"
#include "CommonTextBlock.h"
#include "Style/HOLUIStyles.h"

UHOLButton::UHOLButton()
{
	Style = UHOLButtonStyle::StaticClass();
}

void UHOLButton::NativeOnInitialized()
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

void UHOLButton::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();
	if (Label)
	{
		Label->SetStyle(GetCurrentTextStyleClass());
	}
}

void UHOLButton::SetLabel(const FText& InLabel)
{
	if (Label)
	{
		Label->SetText(InLabel);
	}
}
