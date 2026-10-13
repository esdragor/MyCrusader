#include "Screens/HOLScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "CommonInputSubsystem.h"
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
#include "HeroesOfLibertyUILog.h"
#include "Input/CommonUIInputTypes.h"
#include "Input/HOLUIInputData.h"
#include "Style/HOLUIStyles.h"
#include "Widgets/HOLButton.h"
#include "Widgets/HOLDropdown.h"
#include "Widgets/HOLRotator.h"

#define LOCTEXT_NAMESPACE "HOLScreen"

namespace UE::HeroesOfLiberty::UI
{
	constexpr float RowButtonWidth = 220.f;

	bool HasFocus(const UWidget& Widget)
	{
		return Widget.HasAnyUserFocus() || Widget.HasFocusedDescendants();
	}

	bool CanTakeFocus(const UWidget* Widget)
	{
		return Widget && Widget->GetIsEnabled() && Widget->IsVisible() && Widget->GetCachedWidget().IsValid();
	}

	double CentreHeight(const UWidget& Widget)
	{
		const FGeometry& Geometry = Widget.GetCachedGeometry();
		return Geometry.GetAbsolutePosition().Y + Geometry.GetAbsoluteSize().Y / 2.0;
	}
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

	if (UCommonInputSubsystem* Input = UCommonInputSubsystem::Get(GetOwningLocalPlayer()))
	{
		Input->OnInputMethodChangedNative.AddUObject(this, &UHOLScreen::HandleInputMethodChanged);
		HandleInputMethodChanged(Input->GetCurrentInputType());
	}
}

void UHOLScreen::NativeConstruct()
{
	Super::NativeConstruct();

	// D250: bound on every screen, a screen without columns ignores them. Bound at construction and released at
	// destruction, as Common UI does for Back.
	const UInputAction* NextColumn = UHOLUIInputData::GetNextColumnAction();
	const UInputAction* PreviousColumn = UHOLUIInputData::GetPreviousColumnAction();
	if (NextColumn && PreviousColumn)
	{
		ColumnActionHandles.Add(RegisterUIActionBinding(FBindUIActionArgs(NextColumn, /*bShouldDisplayInActionBar*/ false,
			FSimpleDelegate::CreateUObject(this, &UHOLScreen::MoveColumn, 1))));
		ColumnActionHandles.Add(RegisterUIActionBinding(FBindUIActionArgs(PreviousColumn, /*bShouldDisplayInActionBar*/ false,
			FSimpleDelegate::CreateUObject(this, &UHOLScreen::MoveColumn, -1))));
	}
	else
	{
		UE_LOG(LogHOLUI, Error, TEXT("Screen: column actions missing in /Game/UI/Input (Tools/Editor/make_ui_input.py)"));
	}
}

void UHOLScreen::NativeDestruct()
{
	for (FUIActionBindingHandle& Handle : ColumnActionHandles)
	{
		Handle.Unregister();
	}
	ColumnActionHandles.Reset();
	Super::NativeDestruct();
}

UCommonTextBlock* UHOLScreen::AddColumnHint(UVerticalBox* Column)
{
	UCommonTextBlock* Hint = AddText(Column, NAME_None, LOCTEXT("ColumnHint", "LB / RB : changer de colonne"),
		UHOLTextStyleNote::StaticClass(), 0.f);
	ColumnHints.Add(Hint);
	return Hint;
}

void UHOLScreen::HandleInputMethodChanged(const ECommonInputType InputType)
{
	// Hidden rather than collapsed: the layout does not move when the player takes the gamepad.
	for (UCommonTextBlock* Hint : ColumnHints)
	{
		if (Hint)
		{
			Hint->SetVisibility(InputType == ECommonInputType::Gamepad ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		}
	}
}

void UHOLScreen::MoveColumn(const int32 Direction)
{
	using namespace UE::HeroesOfLiberty::UI;

	TArray<TArray<UWidget*>> Columns;
	GetColumns(Columns);
	int32 FromColumn = INDEX_NONE;
	const UWidget* Focused = nullptr;
	for (int32 Index = 0; Index < Columns.Num() && !Focused; ++Index)
	{
		for (const UWidget* Widget : Columns[Index])
		{
			if (Widget && HasFocus(*Widget))
			{
				FromColumn = Index;
				Focused = Widget;
				break;
			}
		}
	}

	// From outside the columns (a tab, a button of the bottom row), RB enters the first one and LB the last one, on
	// their first widget that can take the focus.
	const double Height = Focused ? CentreHeight(*Focused) : 0.0;
	for (int32 ToColumn = Focused ? FromColumn + Direction : (Direction > 0 ? 0 : Columns.Num() - 1);
		Columns.IsValidIndex(ToColumn); ToColumn += Direction)
	{
		UWidget* Target = nullptr;
		for (UWidget* Widget : Columns[ToColumn])
		{
			if (CanTakeFocus(Widget)
				&& (!Target || FMath::Abs(CentreHeight(*Widget) - Height) < FMath::Abs(CentreHeight(*Target) - Height)))
			{
				Target = Widget;
				if (!Focused)
				{
					break;
				}
			}
		}
		if (Target)
		{
			UE_LOG(LogHOLUI, Display, TEXT("Screen: column %d -> %d (%s)"), FromColumn, ToColumn, *Target->GetName());
			Target->SetFocus();
			return;
		}
	}
	UE_LOG(LogHOLUI, Verbose, TEXT("Screen: no column to move to (from column %d of %d)"), FromColumn, Columns.Num());
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
	const TArray<FText>& Options, const int32 SelectedIndex, const float LabelWidth, const float RotatorWidth)
{
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

UHOLDropdown* UHOLScreen::AddDropdown(UHorizontalBox* Row, const FName Name, const TArray<FText>& Options,
	const int32 SelectedIndex, const float Width)
{
	USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	Box->SetWidthOverride(Width);
	UHOLDropdown* Dropdown = WidgetTree->ConstructWidget<UHOLDropdown>(UHOLDropdown::StaticClass(), Name);
	Dropdown->SetOptions(Options, SelectedIndex);
	Box->SetContent(Dropdown);
	UHorizontalBoxSlot* BoxSlot = Row->AddChildToHorizontalBox(Box);
	BoxSlot->SetVerticalAlignment(VAlign_Center);
	BoxSlot->SetPadding(FMargin(4.f, 0.f));
	return Dropdown;
}

#undef LOCTEXT_NAMESPACE
