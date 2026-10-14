#include "Widgets/HOLGridCell.h"

#include "Blueprint/WidgetTree.h"
#include "CommonTextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Match/HOLResources.h"
#include "Style/HOLUIStyles.h"

namespace UE::HeroesOfLiberty::UI::GridCell
{
	constexpr int32 NumResources = 4;
	constexpr float MarkerSize = 8.f;
	/** Opacity of the name and cost of a greyed cell. */
	constexpr float GreyedOpacity = 0.35f;
}

UHOLGridCell::UHOLGridCell()
{
	Style = UHOLHUDButtonStyle::StaticClass();
	SetIsFocusable(false);
}

void UHOLGridCell::NativeOnInitialized()
{
	using namespace UE::HeroesOfLiberty::UI::GridCell;

	// Runs inside UCommonButtonBase::Initialize, before it wraps the root widget into its internal button.
	if (!WidgetTree->RootWidget)
	{
		UOverlay* CellContent = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Content"));
		WidgetTree->RootWidget = CellContent;

		Key = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("Key"));
		Key->SetStyle(UHOLTextStyleHUDLabel::StaticClass());
		UOverlaySlot* KeySlot = CellContent->AddChildToOverlay(Key);
		KeySlot->SetHorizontalAlignment(HAlign_Left);
		KeySlot->SetVerticalAlignment(VAlign_Top);

		UnitBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("UnitBox"));
		UOverlaySlot* UnitSlot = CellContent->AddChildToOverlay(UnitBox);
		UnitSlot->SetHorizontalAlignment(HAlign_Center);
		UnitSlot->SetVerticalAlignment(VAlign_Center);
		UnitSlot->SetPadding(FMargin(0.f, 10.f, 0.f, 0.f));

		UnitName = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("UnitName"));
		UnitName->SetStyle(UHOLTextStyleHUDSmall::StaticClass());
		UnitName->SetJustification(ETextJustify::Center);
		UnitBox->AddChildToVerticalBox(UnitName)->SetHorizontalAlignment(HAlign_Center);

		UHorizontalBox* CostLine = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("CostLine"));
		UVerticalBoxSlot* CostSlot = UnitBox->AddChildToVerticalBox(CostLine);
		CostSlot->SetHorizontalAlignment(HAlign_Center);
		CostSlot->SetPadding(FMargin(0.f, 2.f, 0.f, 0.f));

		const FLinearColor Colors[NumResources] = { HOLUIStyle::Food, HOLUIStyle::Wood, HOLUIStyle::Stone, HOLUIStyle::Gold };
		for (int32 Index = 0; Index < NumResources; ++Index)
		{
			UImage* Marker = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), *FString::Printf(TEXT("CostMarker%d"), Index));
			Marker->SetColorAndOpacity(Colors[Index]);
			Marker->SetDesiredSizeOverride(FVector2D(MarkerSize, MarkerSize));
			UHorizontalBoxSlot* MarkerSlot = CostLine->AddChildToHorizontalBox(Marker);
			MarkerSlot->SetVerticalAlignment(VAlign_Center);
			MarkerSlot->SetPadding(FMargin(0.f, 0.f, 3.f, 0.f));
			CostMarkers.Add(Marker);

			UCommonTextBlock* Amount = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), *FString::Printf(TEXT("CostAmount%d"), Index));
			Amount->SetStyle(UHOLTextStyleHUDSmall::StaticClass());
			UHorizontalBoxSlot* AmountSlot = CostLine->AddChildToHorizontalBox(Amount);
			AmountSlot->SetVerticalAlignment(VAlign_Center);
			AmountSlot->SetPadding(FMargin(0.f, 0.f, 5.f, 0.f));
			CostAmounts.Add(Amount);
		}
	}
	Super::NativeOnInitialized();
	ClearUnit();
}

void UHOLGridCell::SetKey(const FText& InKey)
{
	if (Key)
	{
		Key->SetText(InKey);
	}
}

void UHOLGridCell::SetUnit(const FText& InName, const FHOLResources& Cost, const bool bInEnabled)
{
	using namespace UE::HeroesOfLiberty::UI::GridCell;

	if (UnitBox)
	{
		UnitBox->SetVisibility(ESlateVisibility::HitTestInvisible);
		UnitBox->SetRenderOpacity(bInEnabled ? 1.f : GreyedOpacity);
	}
	if (UnitName)
	{
		UnitName->SetText(InName);
	}
	const int32 Amounts[NumResources] = { Cost.Food, Cost.Wood, Cost.Stone, Cost.Gold };
	for (int32 Index = 0; Index < NumResources; ++Index)
	{
		// Only the resources the unit takes.
		const ESlateVisibility Shown = Amounts[Index] > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
		if (CostMarkers.IsValidIndex(Index) && CostMarkers[Index])
		{
			CostMarkers[Index]->SetVisibility(Shown);
		}
		if (CostAmounts.IsValidIndex(Index) && CostAmounts[Index])
		{
			CostAmounts[Index]->SetVisibility(Shown);
			CostAmounts[Index]->SetText(FText::AsNumber(Amounts[Index], &FNumberFormattingOptions::DefaultNoGrouping()));
		}
	}
	SetIsEnabled(bInEnabled);
}

void UHOLGridCell::ClearUnit()
{
	if (UnitBox)
	{
		UnitBox->SetVisibility(ESlateVisibility::Collapsed);
	}
	SetIsEnabled(false);
}
