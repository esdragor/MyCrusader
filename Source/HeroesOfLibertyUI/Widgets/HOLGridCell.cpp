#include "Widgets/HOLGridCell.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "CommonTextBlock.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/SizeBox.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
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
	/** The gain's logo in the scripted checks (DescribeContent, D781), in the order of EHOLResource. */
	const TCHAR* const ResourceWords[NumResources] = { TEXT("Food"), TEXT("Wood"), TEXT("Stone"), TEXT("Gold") };
	/** The gain's logo on the cell (D781): the resource's marker, larger than the tooltip's, until the resource icons exist. */
	constexpr float GainMarkerSize = 10.f;
	/** The room under the status line's text, at the bottom of the cell. */
	constexpr float StatusBottomPad = 2.f;
	/** A text line of the Tiny style when it cannot be measured (no renderer, as in a commandlet): 11 pt = 14.7 px, about 1.2 lines. */
	constexpr float FallbackLineHeight = 18.f;
	/** Width of a character of the Tiny style when it cannot be measured (no renderer): 11 pt, half a line for a Latin letter, a line for a CJK one. */
	constexpr float FallbackCharWidth = 5.5f;
	constexpr float FallbackWideCharWidth = 11.f;

	FLinearColor GetResourceColor(const EHOLResource Resource)
	{
		switch (Resource)
		{
		case EHOLResource::Food: return HOLUIStyle::Food;
		case EHOLResource::Wood: return HOLUIStyle::Wood;
		case EHOLResource::Stone: return HOLUIStyle::Stone;
		default: return HOLUIStyle::Gold;
		}
	}
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
		UnitSlot = CellContent->AddChildToOverlay(UnitBox);
		UnitSlot->SetHorizontalAlignment(HAlign_Fill);
		UnitSlot->SetVerticalAlignment(VAlign_Center);
		UnitSlot->SetPadding(FMargin(0.f, 12.f, 0.f, 0.f));

		UnitName = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("UnitName"));
		UnitName->SetStyle(UHOLTextStyleHUDTiny::StaticClass());
		UnitName->SetJustification(ETextJustify::Center);
		UnitName->SetAutoWrapText(true);
		// The name sits in a box that SetStatus bounds to whole lines (clipped to its bounds: a name too long is cut at a line, not in the
		// middle of one); not bounded on a cell without a status.
		NameBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("NameBox"));
		NameBox->SetClipping(EWidgetClipping::ClipToBounds);
		NameBox->SetContent(UnitName);
		UnitBox->AddChildToVerticalBox(NameBox)->SetHorizontalAlignment(HAlign_Fill);

		// The status (Bientot): a line of its own at the bottom of the cell, outside the name's box, hidden until SetStatus gives one.
		StatusText = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("StatusText"));
		StatusText->SetStyle(UHOLTextStyleHUDTiny::StaticClass());
		StatusText->SetJustification(ETextJustify::Center);
		StatusText->SetVisibility(ESlateVisibility::Collapsed);
		// The status sits in a clipping box: reduced to the cell's width (NativeTick), a text still too wide is cut, never drawn on the next cell.
		StatusClip = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("StatusClip"));
		StatusClip->SetClipping(EWidgetClipping::ClipToBounds);
		StatusClip->SetVisibility(ESlateVisibility::HitTestInvisible);
		StatusClip->SetContent(StatusText);
		UOverlaySlot* StatusSlot = CellContent->AddChildToOverlay(StatusClip);
		StatusSlot->SetHorizontalAlignment(HAlign_Fill);
		StatusSlot->SetVerticalAlignment(VAlign_Bottom);
		StatusSlot->SetPadding(FMargin(0.f, 0.f, 0.f, UE::HeroesOfLiberty::UI::GridCell::StatusBottomPad));
	}

	// A key given before the widget was initialized (a script panel builds the cell and keys it at once, H-8).
	if (Key && !KeyText.IsEmpty())
	{
		Key->SetText(KeyText);
	}

	// D781: a trade cell's gain under the name, « +96 » then the logo of what it gives; hidden on every other cell.
	if (UnitBox && !GainLine)
	{
		using namespace UE::HeroesOfLiberty::UI::GridCell;
		GainLine = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("GainLine"));
		GainAmount = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("GainAmount"));
		GainAmount->SetStyle(UHOLTextStyleHUDTiny::StaticClass());
		UHorizontalBoxSlot* AmountSlot = GainLine->AddChildToHorizontalBox(GainAmount);
		AmountSlot->SetVerticalAlignment(VAlign_Center);
		AmountSlot->SetPadding(FMargin(0.f, 0.f, 3.f, 0.f));
		GainMarker = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("GainMarker"));
		GainMarker->SetDesiredSizeOverride(FVector2D(GainMarkerSize, GainMarkerSize));
		GainLine->AddChildToHorizontalBox(GainMarker)->SetVerticalAlignment(VAlign_Center);
		UnitBox->AddChildToVerticalBox(GainLine)->SetHorizontalAlignment(HAlign_Center);
		GainLine->SetVisibility(ESlateVisibility::Collapsed);
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
	OnClicked().AddUObject(this, &UHOLGridCell::BroadcastCellClicked);
	ClearUnit();
}

void UHOLGridCell::BroadcastCellClicked()
{
	OnCellClicked.Broadcast(this);
}

void UHOLGridCell::SetKey(const FText& InKey)
{
	KeyText = InKey;
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

	// A command's status (SetStatus) is given after it: the cell reused for a new command shows none.
	SetStatus(FText::GetEmpty());

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
	if (GainLine)
	{
		// Only a trade cell has a gain (SetGain, after this call).
		GainLine->SetVisibility(ESlateVisibility::Collapsed);
		GainLine->SetRenderOpacity(Opacity);
	}
	if (State != InState)
	{
		// A greyed cell still takes a click (D313): enabled, its look darker.
		SetStyle(InState == EHOLGridCellState::Greyed ? UHOLHUDButtonStyleGreyed::StaticClass() : UHOLHUDButtonStyle::StaticClass());
	}
	State = InState;
	SetIsEnabled(InState != EHOLGridCellState::Disabled);
}

void UHOLGridCell::SetGain(const FHOLTradeGain& Gain)
{
	using namespace UE::HeroesOfLiberty::UI::GridCell;

	if (!GainLine || !GainAmount || !GainMarker)
	{
		return;
	}
	GainAmount->SetText(FText::FromString(HOLCommandGrid::GetTradeGainText(Gain)));
	GainMarker->SetColorAndOpacity(GetResourceColor(Gain.Logo));
	ShownGainLogo = Gain.Logo;
	GainLine->SetVisibility(ESlateVisibility::HitTestInvisible);
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
	using namespace UE::HeroesOfLiberty::UI::GridCell;

	// The texts the cell shows, its key apart; a trade's gain after them, its logo by the resource's word (D781).
	if (!UnitBox || UnitBox->GetVisibility() == ESlateVisibility::Collapsed)
	{
		return FString();
	}
	FString Content = UnitName && UnitName->GetVisibility() != ESlateVisibility::Collapsed ? UnitName->GetText().ToString() : FString();
	if (StatusText && StatusText->GetVisibility() != ESlateVisibility::Collapsed)
	{
		// The status is a line of its own under the name (the text the scripted checks read is the one the glued text gave).
		Content += TEXT("\n") + ShownStatus;
	}
	if (GainLine && GainLine->GetVisibility() != ESlateVisibility::Collapsed && GainAmount && GainMarker
		&& GainMarker->GetVisibility() != ESlateVisibility::Collapsed)
	{
		const int32 Logo = static_cast<int32>(ShownGainLogo);
		Content += FString::Printf(TEXT(" %s [%s]"), *GainAmount->GetText().ToString(), Logo >= 0 && Logo < NumResources ? ResourceWords[Logo] : TEXT("?"));
	}
	return Content;
}

void UHOLGridCell::ClearUnit()
{
	SetStatus(FText::GetEmpty());
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

FString UHOLGridCell::BreakStatusLines(const FString& Status)
{
	// A status of several words takes two lines at most: broken at the space nearest its middle ("Coming soon" -> "Coming" / "soon").
	// A single word, or a text already broken, is returned as it is.
	if (Status.Contains(TEXT("\n")))
	{
		return Status;
	}
	const int32 Middle = Status.Len() / 2;
	int32 Best = INDEX_NONE;
	for (int32 Index = 0; Index < Status.Len(); ++Index)
	{
		if (Status[Index] == TEXT(' ') && (Best == INDEX_NONE || FMath::Abs(Index - Middle) < FMath::Abs(Best - Middle)))
		{
			Best = Index;
		}
	}
	return Best == INDEX_NONE ? Status : Status.Left(Best) + TEXT("\n") + Status.Mid(Best + 1);
}

int32 UHOLGridCell::GetStatusLineCount() const
{
	return StatusLineCount;
}

FHOLGridCellNameZone UHOLGridCell::ComputeNameZone(const float CellSize, const float TopBand, const float LineHeight, const bool bHasStatus, const int32 StatusLines)
{
	using namespace UE::HeroesOfLiberty::UI::GridCell;

	// Without a status the cell is as before: nothing bounded, no line kept.
	FHOLGridCellNameZone Zone;
	if (!bHasStatus || LineHeight <= 0.f)
	{
		return Zone;
	}
	// The status takes its lines (one, or two for a text broken in two: D889) at the bottom; the name keeps the whole lines left under the top band, at least one.
	Zone.StatusHeight = FMath::Max(1, StatusLines) * LineHeight + StatusBottomPad;
	const float Room = FMath::Max(0.f, CellSize - TopBand - Zone.StatusHeight);
	Zone.NameLines = FMath::Max(1, FMath::FloorToInt(Room / LineHeight));
	Zone.NameHeight = Zone.NameLines * LineHeight;
	return Zone;
}

void UHOLGridCell::SetStatus(const FText& InStatus)
{
	using namespace UE::HeroesOfLiberty::UI::GridCell;

	const bool bHasStatus = !InStatus.IsEmpty();
	ShownStatus = InStatus.ToString();
	StatusLineCount = bHasStatus ? 1 : 0;
	if (StatusText)
	{
		StatusText->SetText(InStatus);
		StatusText->SetVisibility(bHasStatus ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		StatusText->SetRenderOpacity(State == EHOLGridCellState::Ready ? 1.f : GreyedOpacity);
	}
	// D889: the break into two lines is decided on the width of the cell (FitStatusToWidth), here on the last known one; NativeTick keeps it
	// on the real width once the cell is laid out.
	FitStatusToWidth(FittedCellWidth);
	RefreshNameZone();
}

void UHOLGridCell::FitStatusToWidth(const float CellWidth)
{
	FittedCellWidth = CellWidth;
	const bool bHasStatus = !ShownStatus.IsEmpty();
	int32 Lines = bHasStatus ? 1 : 0;
	FString Shown = ShownStatus;
	// D889: a status wider than the cell on one line ("Coming soon") is broken in two lines instead of being reduced past the readable bound;
	// measured on the font the text has NOW, against the real width of the cell (not a constant).
	if (bHasStatus && MeasureTextWidth(ShownStatus) > CellWidth - 2.f * StatusSidePad)
	{
		const FString Broken = BreakStatusLines(ShownStatus);
		if (Broken != ShownStatus)
		{
			Shown = Broken;
			Lines = 2;
		}
	}
	if (StatusText && bHasStatus && StatusText->GetText().ToString() != Shown)
	{
		StatusText->SetText(FText::FromString(Shown));
	}
	if (Lines != StatusLineCount)
	{
		StatusLineCount = Lines;
		RefreshNameZone();
	}
}

void UHOLGridCell::RefreshNameZone()
{
	using namespace UE::HeroesOfLiberty::UI::GridCell;

	const bool bHasStatus = !ShownStatus.IsEmpty();
	// The height of a name line: measured on the name's font when a renderer exists, a constant otherwise.
	float LineHeight = FallbackLineHeight;
	if (bHasStatus && UnitName && FSlateApplication::IsInitialized() && FSlateApplication::Get().GetRenderer())
	{
		const TSharedRef<FSlateFontMeasure> Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		const float Measured = Measure->GetMaxCharacterHeight(UnitName->GetFont());
		LineHeight = Measured > 0.f ? Measured : FallbackLineHeight;
	}
	const FHOLGridCellNameZone Zone = ComputeNameZone(DefaultCellSize, NameTopBand, LineHeight, bHasStatus, FMath::Max(1, StatusLineCount));
	NameZoneHeight = Zone.NameHeight;
	if (NameBox)
	{
		if (bHasStatus)
		{
			NameBox->SetMaxDesiredHeight(Zone.NameHeight);
		}
		else
		{
			NameBox->ClearMaxDesiredHeight();
		}
	}
	if (UnitSlot)
	{
		// The status line's room is taken from the bottom of the name's slot: the name and the gain line never reach it.
		UnitSlot->SetPadding(FMargin(0.f, NameTopBand, 0.f, Zone.StatusHeight));
	}
}

float UHOLGridCell::GetNameZoneHeight() const
{
	return NameZoneHeight;
}
float UHOLGridCell::MeasureStatusWidth() const
{
	using namespace UE::HeroesOfLiberty::UI::GridCell;

	if (!StatusText || StatusText->GetVisibility() == ESlateVisibility::Collapsed)
	{
		return 0.f;
	}
	return MeasureTextWidth(StatusText->GetText().ToString());
}

float UHOLGridCell::MeasureTextWidth(const FString& Text) const
{
	using namespace UE::HeroesOfLiberty::UI::GridCell;

	if (!StatusText || Text.IsEmpty())
	{
		return 0.f;
	}
	// The widest line (a status broken in two lines, D889, is as wide as its wider half).
	TArray<FString> Lines;
	Text.ParseIntoArrayLines(Lines, false);
	const bool bCanMeasure = FSlateApplication::IsInitialized() && FSlateApplication::Get().GetRenderer();
	float Widest = 0.f;
	for (const FString& Line : Lines)
	{
		float LineWidth = 0.f;
		if (bCanMeasure)
		{
			const TSharedRef<FSlateFontMeasure> Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
			LineWidth = static_cast<float>(Measure->Measure(Line, StatusText->GetFont()).X);
		}
		if (LineWidth <= 0.f)
		{
			for (const TCHAR Character : Line)
			{
				LineWidth += Character >= 0x2E80 ? FallbackWideCharWidth : FallbackCharWidth;
			}
		}
		Widest = FMath::Max(Widest, LineWidth);
	}
	return Widest;
}

FHOLGridCellStatusFit UHOLGridCell::ComputeStatusFit(const float TextWidth, const float AvailableWidth)
{
	// A text that fits (or an empty one) is not reduced; a wider one shrinks to the room, never below the readable bound.
	FHOLGridCellStatusFit Fit;
	Fit.Width = TextWidth;
	if (TextWidth <= 0.f || AvailableWidth >= TextWidth)
	{
		return Fit;
	}
	Fit.Scale = FMath::Clamp(AvailableWidth / TextWidth, MinStatusScale, 1.f);
	Fit.Width = TextWidth * Fit.Scale;
	return Fit;
}

void UHOLGridCell::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Only a cell showing a status is measured; the others are not touched.
	if (!StatusText || StatusText->GetVisibility() == ESlateVisibility::Collapsed)
	{
		return;
	}
	const FVector2D LocalSize = MyGeometry.GetLocalSize();
	// D889: the break into two lines follows the real width of the cell (the cell is laid out only now), before the reduction.
	if (LocalSize.X > 1.0)
	{
		FitStatusToWidth(static_cast<float>(LocalSize.X));
	}
	const float Available = static_cast<float>(LocalSize.X) - 2.f * StatusSidePad;
	const float Scale = ComputeStatusFit(MeasureStatusWidth(), Available).Scale;
	if (!FMath::IsNearlyEqual(Scale, AppliedStatusScale))
	{
		AppliedStatusScale = Scale;
		StatusText->SetRenderScale(FVector2D(Scale, Scale));
	}
}