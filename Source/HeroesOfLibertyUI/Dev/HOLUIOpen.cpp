#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Containers/Ticker.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyUILog.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "Misc/FileHelper.h"
#include "Screens/HOLGameMenu.h"
#include "Screens/HOLMultiplayerScreen.h"
#include "Screens/HOLNewGameScreen.h"
#include "Screens/HOLSettingsScreen.h"
#include "Screens/HOLStatsScreen.h"
#include "Screens/HOLTitleScreen.h"
#include "Widgets/HOLDropdown.h"
#include "Widgets/HOLRootLayout.h"

/**
 * Opens a menu screen or launches a match by the same path as their buttons, for the scripted end-to-end checks
 * (D246).
 *   hol.UI.Open <NewGame|Multiplayer|Settings[:Tab]|Quit|GameMenu|Surrender|Stats> [DelaySeconds=0]   (Tab: 0 Video, 1 Sound, 2 Controls, 3 Language;
 *   GameMenu and Surrender in a match, D356: the in-game menu, then its surrender confirmation over it)
 *   hol.UI.NewGame.Launch [ConfigFile|-] [DelaySeconds=0]   (the file, JSON as for -MatchConfig=, is shown on the screen
 *   first; - launches what the screen shows)
 *   hol.UI.Focus <WidgetName> [DelaySeconds=0]   (gives the focus to a widget of the open confirmation or menu screen, found by its name,
 *   as the keyboard would; then hol.UI.Key drives it)
 *   hol.UI.Language <fr|en|ja>   (switches the language live and saves it, as Settings > Language does)
 *   hol.UI.Shown <WidgetName>...   (logs whether each named widget of the open menu screen is shown: "Shown SlotTeam3: no")
 *   hol.UI.Text <WidgetName>...   (logs the text a label or a drop-down list of the open menu screen shows: "Text SlotState1: IA Normal 2")
 */
namespace UE::HeroesOfLiberty::UI
{
	void AfterDelay(const float Delay, TFunction<void()> Action)
	{
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Action = MoveTemp(Action)](float)
		{
			Action();
			return false;
		}), Delay);
	}

	APlayerController* GetPlayer(const TWeakObjectPtr<UWorld>& WeakWorld)
	{
		return WeakWorld.IsValid() ? WeakWorld->GetFirstPlayerController() : nullptr;
	}

	void Open(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.IsEmpty())
		{
			UE_LOG(LogHOLUI, Warning, TEXT("Usage: hol.UI.Open <NewGame|Multiplayer|Settings[:Tab]|Quit|GameMenu|Surrender|Stats> [DelaySeconds=0]"));
			return;
		}
		const FString Screen = Args[0];
		const float Delay = Args.Num() > 1 ? FCString::Atof(*Args[1]) : 0.f;
		AfterDelay(Delay, [Screen, WeakWorld = TWeakObjectPtr<UWorld>(World)]()
		{
			APlayerController* PlayerController = GetPlayer(WeakWorld);
			if (Screen == TEXT("NewGame"))
			{
				UHOLNewGameScreen::Open(PlayerController);
			}
			else if (Screen == TEXT("Multiplayer"))
			{
				UHOLMultiplayerScreen::Open(PlayerController);
			}
			else if (Screen.StartsWith(TEXT("Settings")))
			{
				// The settings screen already open is reused, so that a script can go through the tabs.
				UHOLSettingsScreen* Settings = UHOLSettingsScreen::Find(PlayerController);
				Settings = Settings ? Settings : UHOLSettingsScreen::Open(PlayerController);
				FString Tab;
				if (Settings && Screen.Split(TEXT(":"), nullptr, &Tab))
				{
					Settings->SelectTab(FCString::Atoi(*Tab));
				}
			}
			else if (Screen == TEXT("Quit"))
			{
				UHOLTitleScreen::AskToQuit(PlayerController);
			}
			else if (Screen == TEXT("GameMenu"))
			{
				if (!UHOLGameMenu::Open(PlayerController))
				{
					UE_LOG(LogHOLUI, Warning, TEXT("hol.UI.Open: no match to open the in-game menu over"));
					return;
				}
			}
			else if (Screen == TEXT("Stats"))
			{
				if (!UHOLStatsScreen::Open(PlayerController))
				{
					UE_LOG(LogHOLUI, Warning, TEXT("hol.UI.Open: no match to open the statistics over"));
				}
			}
			else if (Screen == TEXT("Surrender"))
			{
				if (!UHOLGameMenu::AskToSurrender(PlayerController))
				{
					UE_LOG(LogHOLUI, Warning, TEXT("hol.UI.Open: no in-game menu open, or surrender not offered"));
					return;
				}
			}
			else
			{
				UE_LOG(LogHOLUI, Warning, TEXT("hol.UI.Open: unknown screen '%s'"), *Screen);
				return;
			}
			UE_LOG(LogHOLUI, Display, TEXT("Screen opened: %s"), *Screen);
		});
	}

	void LaunchNewGame(const TArray<FString>& Args, UWorld* World)
	{
		const FString File = Args.Num() > 0 && Args[0] != TEXT("-") ? Args[0] : FString();
		const float Delay = Args.Num() > 1 ? FCString::Atof(*Args[1]) : 0.f;
		AfterDelay(Delay, [File, WeakWorld = TWeakObjectPtr<UWorld>(World)]()
		{
			APlayerController* PlayerController = GetPlayer(WeakWorld);
			UHOLNewGameScreen* Screen = UHOLNewGameScreen::Find(PlayerController);
			if (!Screen)
			{
				Screen = UHOLNewGameScreen::Open(PlayerController);
			}
			if (!Screen)
			{
				UE_LOG(LogHOLUI, Warning, TEXT("hol.UI.NewGame.Launch: no menu to open the New game screen in"));
				return;
			}
			if (!File.IsEmpty())
			{
				FString Json;
				FString Error;
				FHOLMatchConfig Config;
				if (!FFileHelper::LoadFileToString(Json, *File) || !FHOLMatchConfig::FromJson(Json, Config, &Error))
				{
					UE_LOG(LogHOLUI, Warning, TEXT("hol.UI.NewGame.Launch: cannot read '%s' %s"), *File, *Error);
					return;
				}
				Screen->SetConfig(Config);
			}
			Screen->Launch();
		});
	}

	void Focus(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.IsEmpty())
		{
			UE_LOG(LogHOLUI, Warning, TEXT("Usage: hol.UI.Focus <WidgetName> [DelaySeconds=0]"));
			return;
		}
		const FName Name(*Args[0]);
		const float Delay = Args.Num() > 1 ? FCString::Atof(*Args[1]) : 0.f;
		AfterDelay(Delay, [Name, WeakWorld = TWeakObjectPtr<UWorld>(World)]()
		{
			const UHOLRootLayout* Layout = UHOLRootLayout::Get(GetPlayer(WeakWorld));
			// A confirmation open (modal layer) is searched first: it is on top.
			const UCommonActivatableWidget* Modal = Layout ? Layout->GetActiveModal() : nullptr;
			const UCommonActivatableWidget* Screen = Layout ? Layout->GetActiveMenu() : nullptr;
			UWidget* Widget = Modal ? Modal->WidgetTree->FindWidget(Name) : nullptr;
			Widget = Widget ? Widget : Screen ? Screen->WidgetTree->FindWidget(Name) : nullptr;
			if (!Widget)
			{
				UE_LOG(LogHOLUI, Warning, TEXT("hol.UI.Focus: no widget '%s' on the open menu screen or confirmation"), *Name.ToString());
				return;
			}
			Widget->SetFocus();
			UE_LOG(LogHOLUI, Display, TEXT("Focus given to %s"), *Name.ToString());
		});
	}

	void ChooseValue(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.Num() < 2)
		{
			UE_LOG(LogHOLUI, Warning, TEXT("Usage: hol.UI.Choose <DropdownName> <Index>"));
			return;
		}
		const UHOLRootLayout* Layout = UHOLRootLayout::Get(GetPlayer(World));
		const UCommonActivatableWidget* Screen = Layout ? Layout->GetActiveMenu() : nullptr;
		UHOLDropdown* Dropdown = Screen ? Cast<UHOLDropdown>(Screen->WidgetTree->FindWidget(FName(*Args[0]))) : nullptr;
		if (!Dropdown)
		{
			UE_LOG(LogHOLUI, Warning, TEXT("hol.UI.Choose: no drop-down list '%s' on the open menu screen"), *Args[0]);
			return;
		}
		Dropdown->Choose(FCString::Atoi(*Args[1]));
	}

	void LogShown(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.IsEmpty())
		{
			UE_LOG(LogHOLUI, Warning, TEXT("Usage: hol.UI.Shown <WidgetName>..."));
			return;
		}
		const UHOLRootLayout* Layout = UHOLRootLayout::Get(GetPlayer(World));
		const UCommonActivatableWidget* Screen = Layout ? Layout->GetActiveMenu() : nullptr;
		for (const FString& Name : Args)
		{
			const UWidget* Widget = Screen ? Screen->WidgetTree->FindWidget(FName(*Name)) : nullptr;
			// ", greyed" after it for a widget shown but not enabled (TC-Q12: the teams of the New game screen).
			UE_LOG(LogHOLUI, Display, TEXT("Shown %s: %s%s"), *Name, !Widget ? TEXT("missing") : Widget->IsVisible() ? TEXT("yes") : TEXT("no"),
				Widget && !Widget->GetIsEnabled() ? TEXT(", greyed") : TEXT(""));
		}
	}

	void LogText(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.IsEmpty())
		{
			UE_LOG(LogHOLUI, Warning, TEXT("Usage: hol.UI.Text <WidgetName>..."));
			return;
		}
		const UHOLRootLayout* Layout = UHOLRootLayout::Get(GetPlayer(World));
		const UCommonActivatableWidget* Screen = Layout ? Layout->GetActiveMenu() : nullptr;
		for (const FString& Name : Args)
		{
			const UWidget* Widget = Screen ? Screen->WidgetTree->FindWidget(FName(*Name)) : nullptr;
			// IA0.6 S2: the merged Player list shows the name the slot will have ("IA Normal 2"), not its value.
			if (const UHOLDropdown* Dropdown = Cast<UHOLDropdown>(Widget))
			{
				UE_LOG(LogHOLUI, Display, TEXT("Text %s: %s"), *Name, *Dropdown->GetShownText().ToString());
			}
			else if (const UTextBlock* Label = Cast<UTextBlock>(Widget))
			{
				UE_LOG(LogHOLUI, Display, TEXT("Text %s: %s"), *Name, *Label->GetText().ToString());
			}
			else
			{
				UE_LOG(LogHOLUI, Display, TEXT("Text %s: %s"), *Name, Widget ? TEXT("(no text)") : TEXT("(missing)"));
			}
		}
	}

	void SetLanguage(const TArray<FString>& Args)
	{
		if (Args.IsEmpty())
		{
			UE_LOG(LogHOLUI, Warning, TEXT("Usage: hol.UI.Language <fr|en|ja>"));
			return;
		}
		const bool bSet = UKismetInternationalizationLibrary::SetCurrentLanguage(Args[0], /*SaveToConfig*/ true);
		UE_LOG(LogHOLUI, Display, TEXT("Language %s%s"), *Args[0], bSet ? TEXT("") : TEXT(" refused"));
	}

	FAutoConsoleCommandWithWorldAndArgs OpenCommand(
		TEXT("hol.UI.Open"),
		TEXT("hol.UI.Open <NewGame|Multiplayer|Settings[:Tab]|Quit|GameMenu|Surrender|Stats> [DelaySeconds=0]: opens a menu screen by the same path as its button"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Open));

	FAutoConsoleCommandWithWorldAndArgs LaunchCommand(
		TEXT("hol.UI.NewGame.Launch"),
		TEXT("hol.UI.NewGame.Launch [ConfigFile|-] [DelaySeconds=0]: launches a match from the New game screen, as its Launch button"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&LaunchNewGame));

	FAutoConsoleCommandWithWorldAndArgs FocusCommand(
		TEXT("hol.UI.Focus"),
		TEXT("hol.UI.Focus <WidgetName> [DelaySeconds=0]: gives the focus to a widget of the open menu screen, found by its name"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Focus));

	FAutoConsoleCommandWithWorldAndArgs ChooseCommand(
		TEXT("hol.UI.Choose"),
		TEXT("hol.UI.Choose <DropdownName> <Index>: takes a value in a drop-down list of the open menu screen, as a click in its list"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&ChooseValue));

	FAutoConsoleCommandWithWorldAndArgs ShownCommand(
		TEXT("hol.UI.Shown"),
		TEXT("hol.UI.Shown <WidgetName>...: logs whether each named widget of the open menu screen is shown"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&LogShown));

	FAutoConsoleCommandWithWorldAndArgs TextCommand(
		TEXT("hol.UI.Text"),
		TEXT("hol.UI.Text <WidgetName>...: logs the text a label or a drop-down list of the open menu screen shows"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&LogText));

	FAutoConsoleCommand LanguageCommand(
		TEXT("hol.UI.Language"),
		TEXT("hol.UI.Language <fr|en|ja>: switches the interface language live and saves it, as Settings > Language does"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&SetLanguage));
}
