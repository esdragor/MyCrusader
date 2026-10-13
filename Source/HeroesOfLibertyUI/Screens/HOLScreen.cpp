#include "Screens/HOLScreen.h"

#include "Blueprint/WidgetTree.h"
#include "CommonTextBlock.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Widgets/HOLButton.h"

TOptional<FUIInputConfig> UHOLScreen::GetDesiredInputConfig() const
{
	return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
}

void UHOLScreen::NativeOnInitialized()
{
	if (!WidgetTree->RootWidget)
	{
		BuildLayout();
	}
	Super::NativeOnInitialized();
}

UVerticalBox* UHOLScreen::MakeCenteredColumn()
{
	// An overlay fills the layer; the column inside it is centred both ways.
	UOverlay* Frame = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Frame"));
	UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
	UOverlaySlot* ColumnSlot = Frame->AddChildToOverlay(Column);
	ColumnSlot->SetHorizontalAlignment(HAlign_Center);
	ColumnSlot->SetVerticalAlignment(VAlign_Center);
	WidgetTree->RootWidget = Frame;
	return Column;
}

UCommonTextBlock* UHOLScreen::AddText(UVerticalBox* Column, const FName Name, const FText& Text,
	const TSubclassOf<UCommonTextStyle> TextStyle, const float BottomPadding)
{
	UCommonTextBlock* Block = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), Name);
	Block->SetStyle(TextStyle);
	Block->SetText(Text);
	Block->SetJustification(ETextJustify::Center);
	UVerticalBoxSlot* BlockSlot = Column->AddChildToVerticalBox(Block);
	BlockSlot->SetHorizontalAlignment(HAlign_Center);
	BlockSlot->SetPadding(FMargin(0.f, 0.f, 0.f, BottomPadding));
	return Block;
}

UHOLButton* UHOLScreen::AddButton(UVerticalBox* Column, const FName Name, const FText& Label)
{
	UHOLButton* Button = WidgetTree->ConstructWidget<UHOLButton>(UHOLButton::StaticClass(), Name);
	Button->SetLabel(Label);
	UVerticalBoxSlot* ButtonSlot = Column->AddChildToVerticalBox(Button);
	ButtonSlot->SetHorizontalAlignment(HAlign_Fill);
	ButtonSlot->SetPadding(FMargin(0.f, 6.f));
	return Button;
}
