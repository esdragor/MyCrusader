#include "Widgets/HOLRotator.h"

#include "Blueprint/WidgetTree.h"
#include "CommonTextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Style/HOLUIStyles.h"

UHOLRotator::UHOLRotator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Style = UHOLButtonStyle::StaticClass();
}

void UHOLRotator::NativeOnInitialized()
{
	// Runs inside UCommonButtonBase::Initialize, before it wraps the root widget into its internal button.
	if (!WidgetTree->RootWidget)
	{
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Row"));
		LeftArrow = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("LeftArrow"));
		MyText = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("MyText"));
		RightArrow = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("RightArrow"));
		LeftArrow->SetText(INVTEXT("‹"));
		RightArrow->SetText(INVTEXT("›"));
		MyText->SetJustification(ETextJustify::Center);
		Row->AddChildToHorizontalBox(LeftArrow)->SetVerticalAlignment(VAlign_Center);
		UHorizontalBoxSlot* TextSlot = Row->AddChildToHorizontalBox(MyText);
		TextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		TextSlot->SetVerticalAlignment(VAlign_Center);
		TextSlot->SetPadding(FMargin(12.f, 0.f));
		Row->AddChildToHorizontalBox(RightArrow)->SetVerticalAlignment(VAlign_Center);
		WidgetTree->RootWidget = Row;
	}
	Super::NativeOnInitialized();
	HOLUIStyle::ShowFocus(*this);
}

void UHOLRotator::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();
	for (UCommonTextBlock* Block : { MyText.Get(), LeftArrow.Get(), RightArrow.Get() })
	{
		if (Block)
		{
			Block->SetStyle(GetCurrentTextStyleClass());
		}
	}
}

void UHOLRotator::NativeOnClicked()
{
	Super::NativeOnClicked();
	ShiftTextRight();
}

int32 UHOLRotator::GetStep(const int32 Old, const int32 New, const int32 Count)
{
	return Count > 0 && (New - Old + Count) % Count == 1 ? 1 : -1;
}

void UHOLRotator::SetOptions(const TArray<FText>& Options, const int32 InSelectedIndex)
{
	PopulateTextLabels(Options);
	SetSelectedItem(FMath::Clamp(InSelectedIndex, 0, FMath::Max(0, Options.Num() - 1)));
}
