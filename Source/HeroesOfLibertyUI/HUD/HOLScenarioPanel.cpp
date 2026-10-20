#include "HUD/HOLScenarioPanel.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "CommonTextBlock.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Dev/HOLTestHooks.h"
#include "HeroesOfLibertyUILog.h"
#include "Style/HOLUIStyles.h"
#include "Widgets/HOLQueueItem.h"

namespace UE::HeroesOfLiberty::UI::ScenarioPanel
{
	/** As the game screen's panels: the screen's margin. */
	constexpr float ScreenMargin = 16.f;
	constexpr float AnchorGap = 6.f;
	/** Its width at most: a long note wraps (the top bar on the left, the screen's middle third or so). */
	constexpr float MaxWidth = 860.f;
	constexpr float CloseSize = 29.f;
	constexpr float NoteGap = 4.f;
	const FName CloseName(TEXT("ScenarioClose"));

	using FDisplay = HOLTestHooks::FScenarioDisplay;
}

UHOLScenarioPanel::UHOLScenarioPanel()
{
	// A click on the panel stops here; the rest of the screen is hit-test invisible and goes to the game.
	SetIsFocusable(false);
	SetConsumePointerInput(true);
}

void UHOLScenarioPanel::NativeOnInitialized()
{
	if (!WidgetTree->RootWidget)
	{
		BuildLayout();
	}
	Super::NativeOnInitialized();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (CloseButton)
	{
		CloseButton->OnClicked().AddUObject(this, &UHOLScenarioPanel::HandleCloseClicked);
	}
	Refresh();
}

void UHOLScenarioPanel::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	using namespace UE::HeroesOfLiberty::UI::ScenarioPanel;

	Super::NativeTick(MyGeometry, InDeltaTime);
	// Under the top bar, which only shows once the player's economy has arrived and may change height.
	const UWidget* AnchorWidget = Anchor.Get();
	const float AnchorHeight = AnchorWidget && AnchorWidget->GetVisibility() != ESlateVisibility::Collapsed ? AnchorWidget->GetDesiredSize().Y : 0.f;
	if (AnchorHeight != PlacedAnchorHeight && WidthSlot)
	{
		PlacedAnchorHeight = AnchorHeight;
		const float Top = AnchorHeight > 0.f ? ScreenMargin + AnchorHeight + AnchorGap : ScreenMargin;
		WidthSlot->SetPadding(FMargin(ScreenMargin, Top, ScreenMargin, 0.f));
	}
}

void UHOLScenarioPanel::Refresh()
{
	using namespace UE::HeroesOfLiberty::UI::ScenarioPanel;

	const FDisplay& Display = HOLTestHooks::GetScenarioDisplay();
	const bool bShown = !Display.Name.IsEmpty() && !Display.bClosed;
	if (Panel)
	{
		Panel->SetVisibility(bShown ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	if (!bShown)
	{
		return;
	}
	if (Title)
	{
		Title->SetText(FText::AsCultureInvariant(FString::Printf(TEXT("Scénario %s"), *Display.Name)));
	}
	if (State)
	{
		switch (Display.State)
		{
		case FDisplay::EState::Preparing:
			State->SetText(INVTEXT("en préparation…"));
			State->SetColorAndOpacity(HOLUIStyle::TextMuted);
			break;
		case FDisplay::EState::Ready:
			State->SetText(INVTEXT("prêt"));
			State->SetColorAndOpacity(HOLUIStyle::Accent);
			break;
		case FDisplay::EState::Failed:
			State->SetText(FText::AsCultureInvariant(FString::Printf(TEXT("erreur : %s"), *Display.Error)));
			State->SetColorAndOpacity(HOLUIStyle::Missing);
			break;
		}
	}
	if (NoteColumn && ShownNotes != Display.Notes)
	{
		ShownNotes = Display.Notes;
		NoteColumn->ClearChildren();
		for (int32 Index = 0; Index < ShownNotes.Num(); ++Index)
		{
			UCommonTextBlock* Note = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass());
			Note->SetStyle(UHOLTextStyleBody::StaticClass());
			Note->SetAutoWrapText(true);
			Note->SetText(FText::AsCultureInvariant(ShownNotes[Index]));
			NoteColumn->AddChildToVerticalBox(Note)->SetPadding(FMargin(0.f, NoteGap, 0.f, 0.f));
		}
		NoteColumn->SetVisibility(ShownNotes.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	}
	UE_LOG(LogHOLUI, Display, TEXT("Scenario panel: %s"), *Describe());
}

FString UHOLScenarioPanel::Describe() const
{
	using namespace UE::HeroesOfLiberty::UI::ScenarioPanel;

	const FDisplay& Display = HOLTestHooks::GetScenarioDisplay();
	if (Display.Name.IsEmpty())
	{
		return TEXT("no scenario");
	}
	if (Display.bClosed)
	{
		return FString::Printf(TEXT("scenario %s closed"), *Display.Name);
	}
	const TCHAR* StateName = Display.State == FDisplay::EState::Ready ? TEXT("ready")
		: Display.State == FDisplay::EState::Failed ? TEXT("failed") : TEXT("preparing");
	return FString::Printf(TEXT("scenario %s %s, %d notes"), *Display.Name, StateName, ShownNotes.Num());
}

void UHOLScenarioPanel::HandleCloseClicked()
{
	HOLTestHooks::GetScenarioDisplay().bClosed = true;
	Refresh();
	UE_LOG(LogHOLUI, Display, TEXT("HUD: scenario panel closed"));
}

void UHOLScenarioPanel::BuildLayout()
{
	using namespace UE::HeroesOfLiberty::UI::ScenarioPanel;

	// Only the panel is hit-test visible: the frame lets the clicks through to the game.
	UOverlay* Frame = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("ScenarioFrame"));
	Frame->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = Frame;

	USizeBox* WidthBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("ScenarioWidth"));
	WidthBox->SetMaxDesiredWidth(MaxWidth);
	WidthBox->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidthSlot = Frame->AddChildToOverlay(WidthBox);
	WidthSlot->SetHorizontalAlignment(HAlign_Center);
	WidthSlot->SetVerticalAlignment(VAlign_Top);
	WidthSlot->SetPadding(FMargin(ScreenMargin));

	Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ScenarioPanel"));
	Panel->SetBrush(FSlateRoundedBoxBrush(HOLUIStyle::HUDPanel, 4.f));
	Panel->SetPadding(FMargin(14.f, 8.f, 8.f, 12.f));
	Panel->SetVisibility(ESlateVisibility::Collapsed);
	WidthBox->SetContent(Panel);

	UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ScenarioColumn"));
	Column->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	Panel->SetContent(Column);

	// The title and its state on one line, the « × » on the right.
	UHorizontalBox* Header = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ScenarioHeader"));
	Header->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	Column->AddChildToVerticalBox(Header);

	Title = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("ScenarioTitle"));
	Title->SetStyle(UHOLTextStyleHUDHeading::StaticClass());
	UHorizontalBoxSlot* TitleSlot = Header->AddChildToHorizontalBox(Title);
	TitleSlot->SetVerticalAlignment(VAlign_Center);

	State = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("ScenarioState"));
	State->SetStyle(UHOLTextStyleHUDHeading::StaticClass());
	State->SetAutoWrapText(true);
	UHorizontalBoxSlot* StateSlot = Header->AddChildToHorizontalBox(State);
	StateSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	StateSlot->SetVerticalAlignment(VAlign_Center);
	StateSlot->SetPadding(FMargin(10.f, 0.f, 16.f, 0.f));

	USizeBox* CloseBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("ScenarioCloseBox"));
	CloseBox->SetWidthOverride(CloseSize);
	CloseBox->SetHeightOverride(CloseSize);
	CloseButton = WidgetTree->ConstructWidget<UHOLQueueItem>(UHOLQueueItem::StaticClass(), CloseName);
	CloseButton->SetLabel(INVTEXT("×"));
	CloseBox->SetContent(CloseButton);
	UHorizontalBoxSlot* CloseSlot = Header->AddChildToHorizontalBox(CloseBox);
	CloseSlot->SetVerticalAlignment(VAlign_Top);

	NoteColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ScenarioNotes"));
	NoteColumn->SetVisibility(ESlateVisibility::Collapsed);
	Column->AddChildToVerticalBox(NoteColumn)->SetPadding(FMargin(0.f, 4.f, 0.f, 0.f));
}
