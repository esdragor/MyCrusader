#include "HUD/HOLCheatPanel.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyUILog.h"
#include "Match/HOLResources.h"
#include "Misc/ConfigCacheIni.h"
#include "Player/HOLPlayerController.h"
#include "Style/HOLUIStyles.h"
#include "Widgets/HOLQueueItem.h"

namespace UE::HeroesOfLiberty::UI::CheatPanel
{
	/** As the game screen's panels: the screen's margin, and a gap under the top bar. */
	constexpr float ScreenMargin = 16.f;
	constexpr float AnchorGap = 6.f;
	constexpr float ButtonWidth = 190.f;
	constexpr float ButtonHeight = 29.f;
	constexpr float ButtonGap = 3.f;
	/** What a resource button adds, and the places the population button adds (hol.Cheat.* defaults). */
	constexpr int32 ResourceAmount = 1000;
	constexpr int32 PopulationPlaces = 165;
	const TCHAR* const ConfigSection = TEXT("HOL.CheatPanel");
	const TCHAR* const ConfigOpenKey = TEXT("bOpen");

	struct FCheatButton
	{
		const TCHAR* Name;
		const TCHAR* Label;
	};

	/** In the order of ECheat; the fast build's label is written by RefreshLabels. */
	const FCheatButton CheatSpecs[] = {
		{ TEXT("CheatFood"), TEXT("+1000 Nourriture") },
		{ TEXT("CheatWood"), TEXT("+1000 Bois") },
		{ TEXT("CheatStone"), TEXT("+1000 Pierre") },
		{ TEXT("CheatGold"), TEXT("+1000 Or") },
		{ TEXT("CheatAll"), TEXT("+1000 de tout") },
		{ TEXT("CheatPopulation"), TEXT("Population +165") },
		{ TEXT("CheatFastBuild"), TEXT("") },
	};
	const FName ToggleName(TEXT("CheatToggle"));
}

UHOLCheatPanel::UHOLCheatPanel()
{
	// A click on the panel stops here; the rest of the screen is hit-test invisible and goes to the game.
	SetIsFocusable(false);
	SetConsumePointerInput(true);
}

void UHOLCheatPanel::NativeOnInitialized()
{
	using namespace UE::HeroesOfLiberty::UI::CheatPanel;

	if (!WidgetTree->RootWidget)
	{
		BuildLayout();
	}
	Super::NativeOnInitialized();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	if (Toggle)
	{
		Toggle->OnClicked().AddUObject(this, &UHOLCheatPanel::HandleToggleClicked);
	}
	for (int32 Index = 0; Index < CheatButtons.Num(); ++Index)
	{
		if (CheatButtons[Index])
		{
			CheatButtons[Index]->OnClicked().AddUObject(this, &UHOLCheatPanel::HandleCheatClicked, static_cast<ECheat>(Index));
		}
	}
	GConfig->GetBool(ConfigSection, ConfigOpenKey, bOpen, GGameUserSettingsIni);
	SetOpen(bOpen);
}

void UHOLCheatPanel::NativeConstruct()
{
	Super::NativeConstruct();

	Controller = GetOwningPlayer<AHOLPlayerController>();
	// Solo or listen server: the server's own setting (hol.Prod.BuildTime, set by hol.Cheat.FastBuild).
	const UWorld* World = GetWorld();
	if (World && World->GetNetMode() != NM_Client)
	{
		const IConsoleVariable* BuildTime = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Prod.BuildTime"));
		bFastBuild = BuildTime && BuildTime->GetFloat() > 0.f;
	}
	RefreshLabels();
}

void UHOLCheatPanel::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	using namespace UE::HeroesOfLiberty::UI::CheatPanel;

	Super::NativeTick(MyGeometry, InDeltaTime);

	// Under the top bar, which only shows once the player's economy has arrived and may change height.
	const UWidget* AnchorWidget = Anchor.Get();
	const float AnchorHeight = AnchorWidget && AnchorWidget->GetVisibility() != ESlateVisibility::Collapsed ? AnchorWidget->GetDesiredSize().Y : 0.f;
	if (AnchorHeight != PlacedAnchorHeight && Panel)
	{
		PlacedAnchorHeight = AnchorHeight;
		if (UOverlaySlot* PanelSlot = Cast<UOverlaySlot>(Panel->Slot))
		{
			const float Top = AnchorHeight > 0.f ? ScreenMargin + AnchorHeight + AnchorGap : ScreenMargin;
			PanelSlot->SetPadding(FMargin(ScreenMargin, Top, 0.f, 0.f));
		}
	}
}

void UHOLCheatPanel::SetOpen(const bool bInOpen)
{
	using namespace UE::HeroesOfLiberty::UI::CheatPanel;

	const bool bChanged = bOpen != bInOpen;
	bOpen = bInOpen;
	if (CheatColumn)
	{
		CheatColumn->SetVisibility(bOpen ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
	RefreshLabels();
	if (bChanged)
	{
		GConfig->SetBool(ConfigSection, ConfigOpenKey, bOpen, GGameUserSettingsIni);
		GConfig->Flush(/*bRemoveFromCache*/ false, GGameUserSettingsIni);
	}
}

void UHOLCheatPanel::HandleToggleClicked()
{
	SetOpen(!bOpen);
	UE_LOG(LogHOLUI, Display, TEXT("HUD: cheats %s"), bOpen ? TEXT("open") : TEXT("folded"));
}

void UHOLCheatPanel::HandleCheatClicked(const ECheat Cheat)
{
	using namespace UE::HeroesOfLiberty::UI::CheatPanel;

	AHOLPlayerController* PlayerController = Controller.Get();
	if (!PlayerController)
	{
		return;
	}
	UE_LOG(LogHOLUI, Display, TEXT("HUD: cheat %s clicked"), CheatSpecs[static_cast<int32>(Cheat)].Name);
	switch (Cheat)
	{
	case ECheat::Food: PlayerController->RequestCheatResources(FHOLResources(ResourceAmount, 0, 0, 0)); break;
	case ECheat::Wood: PlayerController->RequestCheatResources(FHOLResources(0, ResourceAmount, 0, 0)); break;
	case ECheat::Stone: PlayerController->RequestCheatResources(FHOLResources(0, 0, ResourceAmount, 0)); break;
	case ECheat::Gold: PlayerController->RequestCheatResources(FHOLResources(0, 0, 0, ResourceAmount)); break;
	case ECheat::All: PlayerController->RequestCheatResources(FHOLResources(ResourceAmount, ResourceAmount, ResourceAmount, ResourceAmount)); break;
	case ECheat::Population: PlayerController->RequestCheatPopulation(PopulationPlaces); break;
	case ECheat::FastBuild:
		bFastBuild = !bFastBuild;
		PlayerController->RequestCheatFastBuild(bFastBuild);
		RefreshLabels();
		break;
	}
}

void UHOLCheatPanel::RefreshLabels()
{
	if (Toggle)
	{
		Toggle->SetLabel(bOpen ? INVTEXT("Triches  -") : INVTEXT("Triches  +"));
	}
	const int32 FastBuildIndex = static_cast<int32>(ECheat::FastBuild);
	if (CheatButtons.IsValidIndex(FastBuildIndex) && CheatButtons[FastBuildIndex])
	{
		CheatButtons[FastBuildIndex]->SetLabel(bFastBuild ? INVTEXT("Construction rapide : oui") : INVTEXT("Construction rapide : non"));
	}
}

bool UHOLCheatPanel::ClickWidget(const FName Name)
{
	if (Toggle && Name == UE::HeroesOfLiberty::UI::CheatPanel::ToggleName)
	{
		HandleToggleClicked();
		return true;
	}
	for (int32 Index = 0; Index < CheatButtons.Num(); ++Index)
	{
		if (CheatButtons[Index] && CheatButtons[Index]->GetFName() == Name)
		{
			if (!bOpen)
			{
				return false;
			}
			HandleCheatClicked(static_cast<ECheat>(Index));
			return true;
		}
	}
	return false;
}

FString UHOLCheatPanel::Describe() const
{
	return bOpen ? FString::Printf(TEXT("cheats open, fast build %s"), bFastBuild ? TEXT("on") : TEXT("off")) : FString(TEXT("cheats folded"));
}

void UHOLCheatPanel::BuildLayout()
{
	using namespace UE::HeroesOfLiberty::UI::CheatPanel;

	// Only the panel is hit-test visible: the frame lets the clicks through to the game.
	UOverlay* Frame = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("CheatFrame"));
	Frame->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = Frame;

	Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("CheatPanel"));
	Panel->SetBrush(FSlateRoundedBoxBrush(HOLUIStyle::HUDPanel, 4.f));
	Panel->SetPadding(FMargin(8.f));
	Panel->SetVisibility(ESlateVisibility::Visible);
	UOverlaySlot* PanelSlot = Frame->AddChildToOverlay(Panel);
	PanelSlot->SetHorizontalAlignment(HAlign_Left);
	PanelSlot->SetVerticalAlignment(VAlign_Top);
	PanelSlot->SetPadding(FMargin(ScreenMargin));

	UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CheatPanelColumn"));
	Column->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	Panel->SetContent(Column);

	const auto MakeButton = [this](const FName Name, UVerticalBox* Box, const float TopPadding)
	{
		USizeBox* ButtonBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		ButtonBox->SetWidthOverride(ButtonWidth);
		ButtonBox->SetHeightOverride(ButtonHeight);
		UHOLQueueItem* Button = WidgetTree->ConstructWidget<UHOLQueueItem>(UHOLQueueItem::StaticClass(), Name);
		ButtonBox->SetContent(Button);
		Box->AddChildToVerticalBox(ButtonBox)->SetPadding(FMargin(0.f, TopPadding, 0.f, 0.f));
		return Button;
	};

	// The toggle, then the cheats under it (collapsed while folded).
	Toggle = MakeButton(ToggleName, Column, 0.f);
	CheatColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CheatColumn"));
	CheatColumn->SetVisibility(ESlateVisibility::Collapsed);
	Column->AddChildToVerticalBox(CheatColumn)->SetPadding(FMargin(0.f, ButtonGap * 2.f, 0.f, 0.f));
	for (int32 Index = 0; Index < static_cast<int32>(UE_ARRAY_COUNT(CheatSpecs)); ++Index)
	{
		UHOLQueueItem* Button = MakeButton(CheatSpecs[Index].Name, CheatColumn, Index == 0 ? 0.f : ButtonGap);
		Button->SetLabel(FText::AsCultureInvariant(CheatSpecs[Index].Label));
		CheatButtons.Add(Button);
	}
}
