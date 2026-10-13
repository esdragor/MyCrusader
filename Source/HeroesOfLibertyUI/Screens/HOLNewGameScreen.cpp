#include "Screens/HOLNewGameScreen.h"

#include "Blueprint/WidgetTree.h"
#include "CommonTextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "HeroesOfLibertyUILog.h"
#include "Input/CommonUIActionRouterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Match/HOLMatchSubsystem.h"
#include "Match/HOLPlayerColors.h"
#include "Style/HOLUIStyles.h"
#include "Widgets/HOLButton.h"
#include "Widgets/HOLDropdown.h"
#include "Widgets/HOLRootLayout.h"

#define LOCTEXT_NAMESPACE "HOLNewGameScreen"

namespace UE::HeroesOfLiberty::UI::NewGame
{
	constexpr float NameWidth = 140.f;
	constexpr float StateWidth = 220.f;
	constexpr float FactionWidth = 280.f;
	constexpr float SwatchSize = 28.f;
	constexpr float ColorWidth = 200.f;
	constexpr float TeamWidth = 160.f;
	constexpr float OptionWidth = 200.f;
	constexpr float ToggleWidth = 120.f;
	constexpr float OptionLabelWidth = 170.f;
	constexpr float ClosedSwatchOpacity = 0.2f;

	/** List values in the order of EHOLSlotState. */
	TArray<FText> StateNames()
	{
		return { LOCTEXT("Closed", "Fermé"), LOCTEXT("Human", "Humain"), LOCTEXT("AI", "IA") };
	}

	/** List values in the order of EHOLFaction (§ 13). */
	TArray<FText> FactionNames()
	{
		return {
			LOCTEXT("DawnOrder", "Ordre de l'Aube"),
			LOCTEXT("BlackLegions", "Légions Noires"),
			LOCTEXT("DragonChildren", "Enfants du Dragon"),
			LOCTEXT("ShadowCircle", "Cercle de l'Ombre"),
			LOCTEXT("FireHeirs", "Héritiers du Feu"),
			LOCTEXT("Templars", "Templiers"),
		};
	}

	TArray<FText> TeamNames()
	{
		TArray<FText> Names = { LOCTEXT("NoTeam", "Aucune") };
		for (int32 Team = 1; Team <= FHOLMatchConfig::MaxTeams; ++Team)
		{
			Names.Add(FText::AsNumber(Team));
		}
		return Names;
	}

	TArray<FText> ColorNames()
	{
		TArray<FText> Names;
		for (int32 Color = 0; Color < HOLPlayerColors::NumColors; ++Color)
		{
			Names.Add(HOLPlayerColors::GetColorName(Color));
		}
		return Names;
	}

	TArray<FLinearColor> ColorSwatches()
	{
		TArray<FLinearColor> Swatches;
		for (int32 Color = 0; Color < HOLPlayerColors::NumColors; ++Color)
		{
			Swatches.Add(FLinearColor(HOLPlayerColors::GetColor(Color)));
		}
		return Swatches;
	}

	TArray<FText> OffOn()
	{
		return { LOCTEXT("Off", "Non"), LOCTEXT("On", "Oui") };
	}

	FName SlotWidgetName(const TCHAR* Prefix, const int32 SlotIndex)
	{
		return FName(FString::Printf(TEXT("%s%d"), Prefix, SlotIndex));
	}
}

FText UHOLNewGameScreen::GetMapName(const FSoftObjectPath& Map)
{
	// One line per map of the list; the names are provisional, the GDD names no map yet.
	const FString Asset = Map.GetAssetName();
	if (Asset == TEXT("Plain"))
	{
		return LOCTEXT("Map_Plain", "Plaine");
	}
	return FText::AsCultureInvariant(Asset);
}

UHOLNewGameScreen* UHOLNewGameScreen::Open(APlayerController* PlayerController)
{
	UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	return Layout ? Cast<UHOLNewGameScreen>(Layout->PushMenu(StaticClass())) : nullptr;
}

UHOLNewGameScreen* UHOLNewGameScreen::Find(APlayerController* PlayerController)
{
	const UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	return Layout ? Cast<UHOLNewGameScreen>(Layout->GetActiveMenu()) : nullptr;
}

void UHOLNewGameScreen::BuildLayout()
{
	using namespace UE::HeroesOfLiberty::UI::NewGame;

	UVerticalBox* Column = MakeCenteredColumn();
	AddText(Column, TEXT("Heading"), LOCTEXT("Heading", "Nouvelle partie"), UHOLTextStyleHeading::StaticClass(), 24.f);

	TArray<FText> MapNames;
	for (const FSoftObjectPath& Map : Maps)
	{
		MapNames.Add(GetMapName(Map));
	}
	// The grid: the map, one line per slot under a line of column headings, then the mode and its options.
	const auto AddCell = [this](UHorizontalBox* Row, UWidget* Content, const float Width)
	{
		USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Box->SetWidthOverride(Width);
		Box->SetContent(Content);
		UHorizontalBoxSlot* CellSlot = Row->AddChildToHorizontalBox(Box);
		CellSlot->SetVerticalAlignment(VAlign_Center);
		CellSlot->SetPadding(FMargin(4.f, 0.f));
	};
	const auto MakeText = [this](const FText& Text, const TSubclassOf<UCommonTextStyle> TextStyle)
	{
		UCommonTextBlock* Block = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass());
		Block->SetStyle(TextStyle);
		Block->SetText(Text);
		return Block;
	};
	// Lines start on the same left edge, the one of the slot lines (the widest).
	const auto AddLine = [this, Column](const float TopPadding)
	{
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UVerticalBoxSlot* RowSlot = Column->AddChildToVerticalBox(Row);
		RowSlot->SetHorizontalAlignment(HAlign_Left);
		RowSlot->SetPadding(FMargin(0.f, TopPadding, 0.f, 4.f));
		return Row;
	};

	UHorizontalBox* MapLine = AddLine(0.f);
	AddCell(MapLine, MakeText(LOCTEXT("Map", "Carte"), UHOLTextStyleBody::StaticClass()), NameWidth);
	MapDropdown = AddDropdown(MapLine, TEXT("MapDropdown"), MapNames, 0, StateWidth + FactionWidth + 8.f);

	UHorizontalBox* Headings = AddLine(16.f);
	AddCell(Headings, MakeText(LOCTEXT("Player", "Joueur"), UHOLTextStyleNote::StaticClass()), NameWidth);
	AddCell(Headings, MakeText(LOCTEXT("Slot", "Emplacement"), UHOLTextStyleNote::StaticClass()), StateWidth);
	AddCell(Headings, MakeText(LOCTEXT("Faction", "Faction"), UHOLTextStyleNote::StaticClass()), FactionWidth);
	AddCell(Headings, MakeText(LOCTEXT("Colour", "Couleur"), UHOLTextStyleNote::StaticClass()), SwatchSize + 8.f + ColorWidth);
	AddCell(Headings, MakeText(LOCTEXT("Team", "Équipe"), UHOLTextStyleNote::StaticClass()), TeamWidth);

	for (int32 SlotIndex = 0; SlotIndex < FHOLMatchConfig::MaxSlots; ++SlotIndex)
	{
		UHorizontalBox* Row = AddLine(0.f);
		AddCell(Row, MakeText(FText::Format(LOCTEXT("PlayerN", "Joueur {0}"), SlotIndex + 1), UHOLTextStyleBody::StaticClass()), NameWidth);
		if (SlotIndex == 0)
		{
			AddCell(Row, MakeText(LOCTEXT("You", "Vous"), UHOLTextStyleBody::StaticClass()), StateWidth);
		}
		else
		{
			AddDropdown(Row, SlotWidgetName(TEXT("SlotState"), SlotIndex), StateNames(), 0, StateWidth);
		}
		AddDropdown(Row, SlotWidgetName(TEXT("SlotFaction"), SlotIndex), FactionNames(), 0, FactionWidth);
		UImage* Swatch = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), SlotWidgetName(TEXT("SlotSwatch"), SlotIndex));
		Swatch->SetDesiredSizeOverride(FVector2D(SwatchSize, SwatchSize));
		AddCell(Row, Swatch, SwatchSize);
		AddDropdown(Row, SlotWidgetName(TEXT("SlotColor"), SlotIndex), ColorNames(), 0, ColorWidth)->SetOptionSwatches(ColorSwatches());
		AddDropdown(Row, SlotWidgetName(TEXT("SlotTeam"), SlotIndex), TeamNames(), 0, TeamWidth);
	}

	UHorizontalBox* Options = AddLine(16.f);
	AddCell(Options, MakeText(LOCTEXT("Mode", "Mode"), UHOLTextStyleBody::StaticClass()), NameWidth);
	ModeDropdown = AddDropdown(Options, TEXT("ModeDropdown"), { LOCTEXT("Standard", "Standard") }, 0, OptionWidth);
	AddCell(Options, MakeText(LOCTEXT("Nomad", "Nomade"), UHOLTextStyleBody::StaticClass()), OptionLabelWidth);
	NomadDropdown = AddDropdown(Options, TEXT("NomadDropdown"), OffOn(), 0, ToggleWidth);
	AddCell(Options, MakeText(LOCTEXT("Regicide", "Régicide"), UHOLTextStyleBody::StaticClass()), OptionLabelWidth);
	RegicideDropdown = AddDropdown(Options, TEXT("RegicideDropdown"), OffOn(), 0, ToggleWidth);
	AddCell(Options, MakeText(LOCTEXT("Catastrophe", "Catastrophe"), UHOLTextStyleBody::StaticClass()), OptionLabelWidth);
	CatastropheDropdown = AddDropdown(Options, TEXT("CatastropheDropdown"), OffOn(), 0, ToggleWidth);

	AddText(Column, TEXT("Note"), LOCTEXT("Greyed", "Grisé : pas encore dans le jeu (IA, factions, équipes, mode et options)."),
		UHOLTextStyleNote::StaticClass(), 4.f);
	AddColumnHint(Column);
	UHorizontalBox* Buttons = AddButtonRow(Column);
	BackButton = AddButton(Buttons, TEXT("BackButton"), LOCTEXT("Back", "Retour"));
	LaunchButton = AddButton(Buttons, TEXT("LaunchButton"), LOCTEXT("Launch", "Lancer"));
}

void UHOLNewGameScreen::NativeOnInitialized()
{
	using namespace UE::HeroesOfLiberty::UI::NewGame;

	Super::NativeOnInitialized();

	for (int32 SlotIndex = 0; SlotIndex < FHOLMatchConfig::MaxSlots; ++SlotIndex)
	{
		SlotStates.Add(Cast<UHOLDropdown>(WidgetTree->FindWidget(SlotWidgetName(TEXT("SlotState"), SlotIndex))));
		SlotFactions.Add(Cast<UHOLDropdown>(WidgetTree->FindWidget(SlotWidgetName(TEXT("SlotFaction"), SlotIndex))));
		SlotColors.Add(Cast<UHOLDropdown>(WidgetTree->FindWidget(SlotWidgetName(TEXT("SlotColor"), SlotIndex))));
		SlotSwatches.Add(Cast<UImage>(WidgetTree->FindWidget(SlotWidgetName(TEXT("SlotSwatch"), SlotIndex))));
		SlotTeams.Add(Cast<UHOLDropdown>(WidgetTree->FindWidget(SlotWidgetName(TEXT("SlotTeam"), SlotIndex))));

		if (SlotStates[SlotIndex])
		{
			// Greyed in the list (D240): AI players are not in the game yet.
			SlotStates[SlotIndex]->SetOptionEnabled(static_cast<int32>(EHOLSlotState::AI), false);
			SlotStates[SlotIndex]->OnSelectionChanged.AddWeakLambda(this, [this, SlotIndex](const int32 Value)
			{
				HandleStateChosen(SlotIndex, Value);
			});
		}
		if (SlotColors[SlotIndex])
		{
			SlotColors[SlotIndex]->OnSelectionChanged.AddWeakLambda(this, [this, SlotIndex](const int32 Value)
			{
				HandleColorChosen(SlotIndex, Value);
			});
		}
		// Greyed (D240): factions and teams are not in the game yet.
		for (UHOLDropdown* Greyed : { SlotFactions[SlotIndex].Get(), SlotTeams[SlotIndex].Get() })
		{
			if (Greyed)
			{
				Greyed->SetIsEnabled(false);
			}
		}
	}
	for (UHOLDropdown* Greyed : { ModeDropdown.Get(), NomadDropdown.Get(), RegicideDropdown.Get(), CatastropheDropdown.Get() })
	{
		if (Greyed)
		{
			Greyed->SetIsEnabled(false);
		}
	}
	if (MapDropdown)
	{
		MapDropdown->OnSelectionChanged.AddUObject(this, &UHOLNewGameScreen::HandleMapChosen);
	}
	if (LaunchButton)
	{
		LaunchButton->OnClicked().AddUObject(this, &UHOLNewGameScreen::Launch);
	}
	if (BackButton)
	{
		BackButton->OnClicked().AddUObject(this, &UHOLNewGameScreen::Close);
	}

	// The configuration of the last launch, or the default one.
	const UGameInstance* GameInstance = GetGameInstance();
	const UHOLMatchSubsystem* Match = GameInstance ? GameInstance->GetSubsystem<UHOLMatchSubsystem>() : nullptr;
	SetConfig(Match ? Match->GetConfigOrDefault() : FHOLMatchConfig::MakeDefault());
}

UWidget* UHOLNewGameScreen::NativeGetDesiredFocusTarget() const
{
	return LaunchButton;
}

void UHOLNewGameScreen::GetColumns(TArray<TArray<UWidget*>>& OutColumns) const
{
	TArray<UWidget*>& States = OutColumns.AddDefaulted_GetRef();
	States.Add(MapDropdown);
	States.Append(SlotStates);
	States.Add(ModeDropdown);
	OutColumns.Emplace(SlotFactions);
	OutColumns.Emplace(SlotColors);
	OutColumns.Emplace(SlotTeams);
}

void UHOLNewGameScreen::SetConfig(const FHOLMatchConfig& InConfig)
{
	Config = InConfig;
	Config.Normalize();
	if (!Maps.IsEmpty() && !Maps.Contains(Config.Map))
	{
		Config.Map = Maps[0];
	}
	Refresh();
}

void UHOLNewGameScreen::Refresh()
{
	using namespace UE::HeroesOfLiberty::UI::NewGame;

	if (MapDropdown)
	{
		MapDropdown->SetSelectedIndex(FMath::Max(0, Maps.IndexOfByKey(Config.Map)));
	}
	for (int32 SlotIndex = 0; SlotIndex < FHOLMatchConfig::MaxSlots; ++SlotIndex)
	{
		const FHOLMatchSlot& SlotConfig = Config.Slots[SlotIndex];
		const bool bOccupied = SlotConfig.State != EHOLSlotState::Closed;
		if (SlotStates[SlotIndex])
		{
			SlotStates[SlotIndex]->SetSelectedIndex(static_cast<int32>(SlotConfig.State));
		}
		if (SlotFactions[SlotIndex])
		{
			SlotFactions[SlotIndex]->SetSelectedIndex(static_cast<int32>(SlotConfig.Faction));
		}
		if (SlotColors[SlotIndex])
		{
			SlotColors[SlotIndex]->SetSelectedIndex(SlotConfig.ColorIndex);
			SlotColors[SlotIndex]->SetIsEnabled(bOccupied);
			for (int32 Color = 0; Color < HOLPlayerColors::NumColors; ++Color)
			{
				SlotColors[SlotIndex]->SetOptionEnabled(Color, !IsColorTaken(SlotIndex, Color));
			}
		}
		if (SlotSwatches[SlotIndex])
		{
			FLinearColor Swatch(HOLPlayerColors::GetColor(SlotConfig.ColorIndex));
			Swatch.A = bOccupied ? 1.f : ClosedSwatchOpacity;
			SlotSwatches[SlotIndex]->SetColorAndOpacity(Swatch);
		}
		if (SlotTeams[SlotIndex])
		{
			SlotTeams[SlotIndex]->SetSelectedIndex(SlotConfig.Team);
		}
	}
	if (NomadDropdown)
	{
		NomadDropdown->SetSelectedIndex(Config.bNomad ? 1 : 0);
	}
	if (RegicideDropdown)
	{
		RegicideDropdown->SetSelectedIndex(Config.bRegicide ? 1 : 0);
	}
	if (CatastropheDropdown)
	{
		CatastropheDropdown->SetSelectedIndex(Config.bCatastrophe ? 1 : 0);
	}
}

void UHOLNewGameScreen::HandleMapChosen(const int32 Value)
{
	if (Maps.IsValidIndex(Value))
	{
		Config.Map = Maps[Value];
	}
	Refresh();
}

void UHOLNewGameScreen::HandleStateChosen(const int32 SlotIndex, const int32 Value)
{
	// AI is greyed in the list (D240): only Closed and Human come here.
	FHOLMatchSlot& SlotConfig = Config.Slots[SlotIndex];
	SlotConfig.State = static_cast<EHOLSlotState>(Value);
	if (SlotConfig.State != EHOLSlotState::Closed && IsColorTaken(SlotIndex, SlotConfig.ColorIndex))
	{
		SlotConfig.ColorIndex = FindFreeColor(SlotIndex, SlotConfig.ColorIndex);
	}
	Refresh();
}

void UHOLNewGameScreen::HandleColorChosen(const int32 SlotIndex, const int32 Value)
{
	// A colour held by another occupied slot is greyed in the list: this one is free.
	Config.Slots[SlotIndex].ColorIndex = Value;
	Refresh();
}

bool UHOLNewGameScreen::IsColorTaken(const int32 SlotIndex, const int32 ColorIndex) const
{
	for (int32 Other = 0; Other < Config.Slots.Num(); ++Other)
	{
		if (Other != SlotIndex && Config.Slots[Other].State != EHOLSlotState::Closed && Config.Slots[Other].ColorIndex == ColorIndex)
		{
			return true;
		}
	}
	return false;
}

int32 UHOLNewGameScreen::FindFreeColor(const int32 SlotIndex, const int32 From) const
{
	constexpr int32 NumColors = HOLPlayerColors::NumColors;
	for (int32 Tried = 0; Tried < NumColors; ++Tried)
	{
		const int32 Color = (From + Tried) % NumColors;
		if (!IsColorTaken(SlotIndex, Color))
		{
			return Color;
		}
	}
	return From;
}

void UHOLNewGameScreen::Launch()
{
	FHOLMatchConfig Launched = Config;
	Launched.Normalize();
	Launched.Seed = static_cast<int32>(FDateTime::UtcNow().GetTicks() & MAX_int32);

	UGameInstance* GameInstance = GetGameInstance();
	UHOLMatchSubsystem* Match = GameInstance ? GameInstance->GetSubsystem<UHOLMatchSubsystem>() : nullptr;
	if (!Match || Launched.Map.IsNull())
	{
		UE_LOG(LogHOLUI, Error, TEXT("New game: no match subsystem or no map, nothing launched"));
		return;
	}
	Match->SetConfig(Launched);
	UE_LOG(LogHOLUI, Display, TEXT("New game: launching %s, %d occupied slots, seed %d"), *Launched.Map.ToString(),
		Launched.GetNumOccupiedSlots(), Launched.Seed);

	// The menu's input config (menu mode) would linger on the viewport once its screens are gone: the match starts in
	// game mode, the mouse free and its cursor shown, as the RTS camera and its edge scrolling want (D254, D255).
	if (UCommonUIActionRouterBase* Router = ULocalPlayer::GetSubsystem<UCommonUIActionRouterBase>(GetOwningLocalPlayer()))
	{
		Router->SetActiveUIInputConfig(FUIInputConfig(ECommonInputMode::Game, EMouseCaptureMode::NoCapture), this);
	}
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, TSoftObjectPtr<UWorld>(Launched.Map));
}

#undef LOCTEXT_NAMESPACE
