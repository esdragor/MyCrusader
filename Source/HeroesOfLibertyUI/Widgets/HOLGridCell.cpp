#include "Widgets/HOLGridCell.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "CommonTextBlock.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Player/HOLCommandGrid.h"
#include "Style/HOLUIStyles.h"

namespace UE::HeroesOfLiberty::UI::GridCell
{
	constexpr int32 NumResources = 4;
	constexpr float MarkerSize = 8.f;
	/** Opacity of the name of a greyed cell. */
	constexpr float GreyedOpacity = 0.4f;
	const EHOLResource Resources[NumResources] = { EHOLResource::Food, EHOLResource::Wood, EHOLResource::Stone, EHOLResource::Gold };
	const TCHAR* const ResourceLetters[NumResources] = { TEXT("F"), TEXT("W"), TEXT("S"), TEXT("G") };
}

UHOLGridCell::UHOLGridCell()
{
	Style = UHOLHUDButtonStyle::StaticClass();
	SetIsFocusable(false);
}

void UHOLGridCell::BuildCostLine(UHorizontalBox* Row, const FString& Prefix, TArray<TObjectPtr<UImage>>& Markers,
	TArray<TObjectPtr<UCommonTextBlock>>& Amounts)
{
	using namespace UE::HeroesOfLiberty::UI::GridCell;

	const FLinearColor Colors[NumResources] = { HOLUIStyle::Food, HOLUIStyle::Wood, HOLUIStyle::Stone, HOLUIStyle::Gold };
	for (int32 Index = 0; Index < NumResources; ++Index)
	{
		UImage* Marker = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), *FString::Printf(TEXT("%sMarker%d"), *Prefix, Index));
		Marker->SetColorAndOpacity(Colors[Index]);
		Marker->SetDesiredSizeOverride(FVector2D(MarkerSize, MarkerSize));
		UHorizontalBoxSlot* MarkerSlot = Row->AddChildToHorizontalBox(Marker);
		MarkerSlot->SetVerticalAlignment(VAlign_Center);
		MarkerSlot->SetPadding(FMargin(0.f, 0.f, 3.f, 0.f));
		Markers.Add(Marker);

		UCommonTextBlock* Amount = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), *FString::Printf(TEXT("%sAmount%d"), *Prefix, Index));
		Amount->SetStyle(UHOLTextStyleHUDSmall::StaticClass());
		UHorizontalBoxSlot* AmountSlot = Row->AddChildToHorizontalBox(Amount);
		AmountSlot->SetVerticalAlignment(VAlign_Center);
		AmountSlot->SetPadding(FMargin(0.f, 0.f, 5.f, 0.f));
		Amounts.Add(Amount);
	}
}

void UHOLGridCell::NativeOnInitialized()
{
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

		// The name alone (no icon yet, no cost: the tooltip's), in the middle under the key's line. The box fills the
		// cell's width so that the name wraps on it (« Camp de » / « bûcherons »), each line centred.
		UnitBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("UnitBox"));
		UOverlaySlot* UnitSlot = CellContent->AddChildToOverlay(UnitBox);
		UnitSlot->SetHorizontalAlignment(HAlign_Fill);
		UnitSlot->SetVerticalAlignment(VAlign_Center);
		UnitSlot->SetPadding(FMargin(0.f, 12.f, 0.f, 0.f));

		UnitName = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("UnitName"));
		UnitName->SetStyle(UHOLTextStyleHUDTiny::StaticClass());
		UnitName->SetJustification(ETextJustify::Center);
		UnitName->SetAutoWrapText(true);
		UnitBox->AddChildToVerticalBox(UnitName)->SetHorizontalAlignment(HAlign_Fill);
	}

	// The tooltip (D313), outside the cell's hierarchy: shown by Slate over the cell hovered.
	TooltipPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("TooltipPanel"));
	TooltipPanel->SetBrush(FSlateRoundedBoxBrush(HOLUIStyle::HUDPanel.CopyWithNewOpacity(0.96f), 4.f, HOLUIStyle::TextMuted, 1.f));
	TooltipPanel->SetPadding(FMargin(10.f, 6.f));
	UVerticalBox* TooltipColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TooltipColumn"));
	TooltipPanel->SetContent(TooltipColumn);
	TooltipName = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("TooltipName"));
	TooltipName->SetStyle(UHOLTextStyleHUDHeading::StaticClass());
	TooltipColumn->AddChildToVerticalBox(TooltipName)->SetPadding(FMargin(0.f, 0.f, 0.f, 3.f));
	TooltipCostLine = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TooltipCostLine"));
	TooltipColumn->AddChildToVerticalBox(TooltipCostLine)->SetPadding(FMargin(0.f, 0.f, 0.f, 3.f));
	BuildCostLine(TooltipCostLine, TEXT("Tooltip"), TooltipMarkers, TooltipAmounts);
	TooltipEffect = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("TooltipEffect"));
	TooltipEffect->SetStyle(UHOLTextStyleHUD::StaticClass());
	TooltipColumn->AddChildToVerticalBox(TooltipEffect);

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

void UHOLGridCell::ShowCost(const TConstArrayView<TObjectPtr<UImage>> Markers, const TConstArrayView<TObjectPtr<UCommonTextBlock>> Amounts,
	const FHOLResources& Cost, const FHOLResources& Missing, const float Opacity)
{
	using namespace UE::HeroesOfLiberty::UI::GridCell;

	for (int32 Index = 0; Index < NumResources; ++Index)
	{
		// Only the resources it takes.
		const int32 Amount = Cost[Resources[Index]];
		const bool bMissing = Missing[Resources[Index]] > 0;
		const ESlateVisibility Shown = Amount > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
		if (Markers.IsValidIndex(Index) && Markers[Index])
		{
			Markers[Index]->SetVisibility(Shown);
			Markers[Index]->SetRenderOpacity(Opacity);
		}
		if (Amounts.IsValidIndex(Index) && Amounts[Index])
		{
			UCommonTextBlock* Text = Amounts[Index];
			Text->SetVisibility(Shown);
			Text->SetText(FText::AsNumber(Amount, &FNumberFormattingOptions::DefaultNoGrouping()));
			// What the stock lacks in red, whole; the rest as the cell.
			Text->SetColorAndOpacity(bMissing ? HOLUIStyle::Missing : HOLUIStyle::Text);
			Text->SetRenderOpacity(bMissing ? 1.f : Opacity);
		}
	}
}

void UHOLGridCell::SetCommand(const FText& InName, const EHOLGridCellState InState)
{
	using namespace UE::HeroesOfLiberty::UI::GridCell;

	// No icon yet: the name (the user's trial of J6a.5); the cost only in the tooltip.
	const FHOLGridCellFace Face = HOLCommandGrid::GetCellFace(/*bHasIcon*/ false);
	const float Opacity = InState == EHOLGridCellState::Ready ? 1.f : GreyedOpacity;
	if (UnitBox)
	{
		UnitBox->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	if (UnitName)
	{
		UnitName->SetText(Face.bName ? InName : FText::GetEmpty());
		UnitName->SetVisibility(Face.bName ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		UnitName->SetRenderOpacity(Opacity);
	}
	if (State != InState)
	{
		// A greyed cell still takes a click (D313): enabled, its look darker.
		SetStyle(InState == EHOLGridCellState::Greyed ? UHOLHUDButtonStyleGreyed::StaticClass() : UHOLHUDButtonStyle::StaticClass());
	}
	State = InState;
	SetIsEnabled(InState != EHOLGridCellState::Disabled);
}

void UHOLGridCell::SetTooltip(const FText& InName, const FHOLResources& Cost, const FHOLResources& Missing, const FText& InEffect)
{
	using namespace UE::HeroesOfLiberty::UI::GridCell;

	if (InName.IsEmpty() || !TooltipPanel)
	{
		ShownTooltip.Reset();
		SetToolTip(nullptr);
		return;
	}
	TooltipName->SetText(InName);
	ShowCost(TooltipMarkers, TooltipAmounts, Cost, Missing, 1.f);
	TooltipCostLine->SetVisibility(Cost == FHOLResources() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	TooltipEffect->SetText(InEffect);
	TooltipEffect->SetVisibility(InEffect.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);

	TArray<FString> Parts = { InName.ToString() };
	for (int32 Index = 0; Index < NumResources; ++Index)
	{
		if (Cost[Resources[Index]] > 0)
		{
			Parts.Add(FString::Printf(TEXT("%s %d%s"), ResourceLetters[Index], Cost[Resources[Index]], Missing[Resources[Index]] > 0 ? TEXT(" missing") : TEXT("")));
		}
	}
	if (!InEffect.IsEmpty())
	{
		Parts.Add(InEffect.ToString());
	}
	ShownTooltip = FString::Join(Parts, TEXT(" | "));
	if (GetToolTip() != TooltipPanel)
	{
		SetToolTip(TooltipPanel);
	}
}

FString UHOLGridCell::DescribeTooltip() const
{
	return ShownTooltip;
}

FString UHOLGridCell::DescribeContent() const
{
	// The texts the cell shows, its key apart.
	return UnitBox && UnitBox->GetVisibility() != ESlateVisibility::Collapsed && UnitName && UnitName->GetVisibility() != ESlateVisibility::Collapsed
		? UnitName->GetText().ToString() : FString();
}

void UHOLGridCell::ClearUnit()
{
	if (UnitBox)
	{
		UnitBox->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (State == EHOLGridCellState::Greyed)
	{
		SetStyle(UHOLHUDButtonStyle::StaticClass());
	}
	State = EHOLGridCellState::Disabled;
	SetIsEnabled(false);
	SetTooltip(FText::GetEmpty(), FHOLResources(), FHOLResources(), FText::GetEmpty());
}
