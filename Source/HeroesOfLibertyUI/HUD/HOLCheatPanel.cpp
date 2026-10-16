#include "HUD/HOLCheatPanel.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "CommonTextBlock.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
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
	/** The speed sliders: their range and their step. */
	constexpr float MinSpeed = 0.5f;
	constexpr float MaxSpeed = 5.f;
	constexpr float SpeedStep = 0.5f;
	constexpr float SliderHeight = 20.f;
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

	struct FSpeedSlider
	{
		const TCHAR* Name;
		const TCHAR* Label;
		/** The server's setting it changes (hol.Cheat.UnitSpeed, hol.Cheat.GatherSpeed). */
		const TCHAR* Variable;
		/** In Describe. */
		const TCHAR* Described;
	};

	/** In the order of ESpeed. */
	const FSpeedSlider SpeedSpecs[] = {
		{ TEXT("CheatUnitSpeed"), TEXT("Vitesse des unités"), TEXT("hol.Units.SpeedScale"), TEXT("unit speed") },
		{ TEXT("CheatGatherSpeed"), TEXT("Vitesse de collecte"), TEXT("hol.Gather.SpeedScale"), TEXT("gather speed") },
	};
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
	for (int32 Index = 0; Index < SpeedSliders.Num(); ++Index)
	{
		USlider* Slider = SpeedSliders[Index];
		if (Slider && static_cast<ESpeed>(Index) == ESpeed::Units)
		{
			Slider->OnValueChanged.AddDynamic(this, &UHOLCheatPanel::HandleUnitSpeedChanged);
		}
		else if (Slider)
		{
			Slider->OnValueChanged.AddDynamic(this, &UHOLCheatPanel::HandleGatherSpeedChanged);
		}
	}
	GConfig->GetBool(ConfigSection, ConfigOpenKey, bOpen, GGameUserSettingsIni);
	SetOpen(bOpen);
}

void UHOLCheatPanel::NativeConstruct()
{
	using namespace UE::HeroesOfLiberty::UI::CheatPanel;

	Super::NativeConstruct();

	Controller = GetOwningPlayer<AHOLPlayerController>();
	// Solo or listen server: the server's own settings (hol.Prod.BuildTime, set by hol.Cheat.FastBuild; the sliders'
	// variables, set by hol.Cheat.UnitSpeed and hol.Cheat.GatherSpeed).
	const UWorld* World = GetWorld();
	if (World && World->GetNetMode() != NM_Client)
	{
		ServerBuildTime = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Prod.BuildTime"));
		for (int32 Index = 0; Index < NumSpeeds; ++Index)
		{
			ServerSpeeds[Index] = IConsoleManager::Get().FindConsoleVariable(SpeedSpecs[Index].Variable);
		}
	}
	FollowServerSettings();
	RefreshLabels();
}

void UHOLCheatPanel::FollowServerSettings()
{
	bool bChanged = false;
	if (ServerBuildTime && (ServerBuildTime->GetFloat() > 0.f) != bFastBuild)
	{
		bFastBuild = !bFastBuild;
		bChanged = true;
	}
	for (int32 Index = 0; Index < NumSpeeds; ++Index)
	{
		const float ServerSpeed = ServerSpeeds[Index] ? ServerSpeeds[Index]->GetFloat() : Speeds[Index];
		if (ServerSpeed == Speeds[Index])
		{
			continue;
		}
		Speeds[Index] = ServerSpeed;
		bChanged = true;
		USlider* Slider = SpeedSliders.IsValidIndex(Index) ? SpeedSliders[Index].Get() : nullptr;
		if (Slider && Slider->GetValue() != ServerSpeed)
		{
			Slider->SetValue(ServerSpeed);
		}
	}
	if (bChanged)
	{
		RefreshLabels();
	}
}

void UHOLCheatPanel::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	using namespace UE::HeroesOfLiberty::UI::CheatPanel;

	Super::NativeTick(MyGeometry, InDeltaTime);
	FollowServerSettings();

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

void UHOLCheatPanel::HandleUnitSpeedChanged(const float Value)
{
	HandleSpeedChanged(ESpeed::Units, Value);
}

void UHOLCheatPanel::HandleGatherSpeedChanged(const float Value)
{
	HandleSpeedChanged(ESpeed::Gather, Value);
}

void UHOLCheatPanel::HandleSpeedChanged(const ESpeed Speed, const float Value)
{
	using namespace UE::HeroesOfLiberty::UI::CheatPanel;

	// The slider moves by steps: one request a step, not one a frame while it is dragged.
	const int32 Index = static_cast<int32>(Speed);
	const float Scale = FMath::Clamp(FMath::GridSnap(Value, SpeedStep), MinSpeed, MaxSpeed);
	if (Scale == Speeds[Index])
	{
		return;
	}
	Speeds[Index] = Scale;
	RefreshLabels();
	if (AHOLPlayerController* PlayerController = Controller.Get())
	{
		UE_LOG(LogHOLUI, Display, TEXT("HUD: cheat %s x%.1f"), SpeedSpecs[Index].Described, Scale);
		if (Speed == ESpeed::Units)
		{
			PlayerController->RequestCheatUnitSpeed(Scale);
		}
		else
		{
			PlayerController->RequestCheatGatherSpeed(Scale);
		}
	}
}

void UHOLCheatPanel::RefreshLabels()
{
	using namespace UE::HeroesOfLiberty::UI::CheatPanel;

	if (Toggle)
	{
		Toggle->SetLabel(bOpen ? INVTEXT("Triches  -") : INVTEXT("Triches  +"));
	}
	const int32 FastBuildIndex = static_cast<int32>(ECheat::FastBuild);
	if (CheatButtons.IsValidIndex(FastBuildIndex) && CheatButtons[FastBuildIndex])
	{
		CheatButtons[FastBuildIndex]->SetLabel(bFastBuild ? INVTEXT("Construction rapide : oui") : INVTEXT("Construction rapide : non"));
	}
	for (int32 Index = 0; Index < SpeedLabels.Num(); ++Index)
	{
		if (SpeedLabels[Index])
		{
			SpeedLabels[Index]->SetText(FText::AsCultureInvariant(FString::Printf(TEXT("%s : x%.1f"), SpeedSpecs[Index].Label, Speeds[Index])));
		}
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
	using namespace UE::HeroesOfLiberty::UI::CheatPanel;

	if (!bOpen)
	{
		return TEXT("cheats folded");
	}
	FString Description = FString::Printf(TEXT("cheats open, fast build %s"), bFastBuild ? TEXT("on") : TEXT("off"));
	for (int32 Index = 0; Index < NumSpeeds; ++Index)
	{
		Description += FString::Printf(TEXT(", %s x%.1f"), SpeedSpecs[Index].Described, Speeds[Index]);
	}
	return Description;
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

	// The speeds: each one's label, then its slider (by steps of x0.5).
	for (int32 Index = 0; Index < NumSpeeds; ++Index)
	{
		const FString Name = SpeedSpecs[Index].Name;
		UCommonTextBlock* Label = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), *(Name + TEXT("Label")));
		Label->SetStyle(UHOLTextStyleHUDLabel::StaticClass());
		CheatColumn->AddChildToVerticalBox(Label)->SetPadding(FMargin(2.f, ButtonGap * 3.f, 0.f, 0.f));
		SpeedLabels.Add(Label);
		USlider* Slider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), *Name);
		Slider->SetMinValue(MinSpeed);
		Slider->SetMaxValue(MaxSpeed);
		Slider->SetStepSize(SpeedStep);
		Slider->MouseUsesStep = true;
		Slider->IsFocusable = false;
		Slider->SetValue(Speeds[Index]);
		Slider->SetSliderBarColor(HOLUIStyle::TextMuted);
		Slider->SetSliderHandleColor(HOLUIStyle::Accent);
		USizeBox* SliderBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *(Name + TEXT("Box")));
		SliderBox->SetWidthOverride(ButtonWidth);
		SliderBox->SetHeightOverride(SliderHeight);
		SliderBox->SetContent(Slider);
		CheatColumn->AddChildToVerticalBox(SliderBox);
		SpeedSliders.Add(Slider);
	}
}
