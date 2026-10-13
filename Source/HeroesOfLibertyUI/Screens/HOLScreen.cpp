#include "Screens/HOLScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "CommonTextBlock.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Style/HOLUIStyles.h"
#include "Widgets/HOLButton.h"
#include "Widgets/HOLRotator.h"

namespace UE::HeroesOfLiberty::UI
{
	constexpr float LabelWidth = 280.f;
	constexpr float RotatorWidth = 380.f;
	constexpr float RowButtonWidth = 220.f;
}

UHOLScreen::UHOLScreen()
{
	bIsBackHandler = true;
}

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

UVerticalBox* UHOLScreen::MakeDialogColumn()
{
	UOverlay* Frame = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Frame"));
	UImage* Veil = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Veil"));
	Veil->SetColorAndOpacity(HOLUIStyle::Veil);
	UOverlaySlot* VeilSlot = Frame->AddChildToOverlay(Veil);
	VeilSlot->SetHorizontalAlignment(HAlign_Fill);
	VeilSlot->SetVerticalAlignment(VAlign_Fill);

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Panel"));
	Panel->SetBrush(FSlateRoundedBoxBrush(HOLUIStyle::Panel, 6.f, HOLUIStyle::Accent, 1.f));
	Panel->SetPadding(FMargin(48.f, 32.f));
	UOverlaySlot* PanelSlot = Frame->AddChildToOverlay(Panel);
	PanelSlot->SetHorizontalAlignment(HAlign_Center);
	PanelSlot->SetVerticalAlignment(VAlign_Center);

	UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
	Panel->SetContent(Column);
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

UHorizontalBox* UHOLScreen::AddButtonRow(UVerticalBox* Column)
{
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UVerticalBoxSlot* RowSlot = Column->AddChildToVerticalBox(Row);
	RowSlot->SetHorizontalAlignment(HAlign_Center);
	RowSlot->SetPadding(FMargin(0.f, 24.f, 0.f, 0.f));
	return Row;
}

UHOLButton* UHOLScreen::AddButton(UHorizontalBox* Row, const FName Name, const FText& Label)
{
	USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	Box->SetWidthOverride(UE::HeroesOfLiberty::UI::RowButtonWidth);
	UHOLButton* Button = WidgetTree->ConstructWidget<UHOLButton>(UHOLButton::StaticClass(), Name);
	Button->SetLabel(Label);
	Box->SetContent(Button);
	Row->AddChildToHorizontalBox(Box)->SetPadding(FMargin(8.f, 0.f));
	return Button;
}

UHOLRotator* UHOLScreen::AddRotatorRow(UVerticalBox* Column, const FName Name, const FText& Label,
	const TArray<FText>& Options, const int32 SelectedIndex)
{
	using namespace UE::HeroesOfLiberty::UI;

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UVerticalBoxSlot* RowSlot = Column->AddChildToVerticalBox(Row);
	RowSlot->SetHorizontalAlignment(HAlign_Center);
	RowSlot->SetPadding(FMargin(0.f, 4.f));

	USizeBox* LabelBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	LabelBox->SetWidthOverride(LabelWidth);
	UCommonTextBlock* LabelText = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass());
	LabelText->SetStyle(UHOLTextStyleBody::StaticClass());
	LabelText->SetText(Label);
	LabelBox->SetContent(LabelText);
	Row->AddChildToHorizontalBox(LabelBox)->SetVerticalAlignment(VAlign_Center);

	return AddRotator(Row, Name, Options, SelectedIndex, RotatorWidth);
}

UHOLRotator* UHOLScreen::AddRotator(UHorizontalBox* Row, const FName Name, const TArray<FText>& Options,
	const int32 SelectedIndex, const float Width)
{
	USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	Box->SetWidthOverride(Width);
	UHOLRotator* Rotator = WidgetTree->ConstructWidget<UHOLRotator>(UHOLRotator::StaticClass(), Name);
	Rotator->SetOptions(Options, SelectedIndex);
	Box->SetContent(Rotator);
	UHorizontalBoxSlot* BoxSlot = Row->AddChildToHorizontalBox(Box);
	BoxSlot->SetVerticalAlignment(VAlign_Center);
	BoxSlot->SetPadding(FMargin(4.f, 0.f));
	return Rotator;
}
