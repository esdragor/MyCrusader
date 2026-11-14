#include "Blueprint/WidgetTree.h"
#include "CommonTextBlock.h"
#include "Dev/HOLTestHooks.h"
#include "HUD/HOLCheatPanel.h"
#include "HUD/HOLScenarioPanel.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Text.h"
#include "Internationalization/TextKey.h"
#include "Internationalization/TextLocalizationResource.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "Widgets/HOLQueueItem.h"

#if WITH_DEV_AUTOMATION_TESTS

// The scenario's panel (HOLScenarioPanel) on the real widget: its title, its state and « Relancer » come from the localization tables
// (French source, English and Japanese translated; recette 2026-11-13: « Scénario deploy en préparation… » stayed French in EN/JA), and
// its state does not wrap (it folded into « en / préparation… » under the title and bit into the scene), except an error, which may be long.
namespace HOLScenarioPanelTestsPrivate
{
	/** The game's language for a test, the machine's put back after (the same call as Settings > Language). */
	struct FScopedPanelLanguage
	{
		FString Previous;
		explicit FScopedPanelLanguage(const TCHAR* Culture)
		{
			Previous = FInternationalization::Get().GetCurrentLanguage()->GetName();
			UKismetInternationalizationLibrary::SetCurrentLanguage(Culture, /*SaveToConfig*/ false);
		}
		~FScopedPanelLanguage() { UKismetInternationalizationLibrary::SetCurrentLanguage(Previous, /*SaveToConfig*/ false); }
	};

	FText TextObjectOf(const UHOLScenarioPanel* Panel, const TCHAR* Name)
	{
		const UCommonTextBlock* Text = Panel && Panel->WidgetTree ? Panel->WidgetTree->FindWidget<UCommonTextBlock>(FName(Name)) : nullptr;
		return Text ? Text->GetText() : FText::AsCultureInvariant(TEXT("<no widget>"));
	}

	FString TextOf(const UHOLScenarioPanel* Panel, const TCHAR* Name) { return TextObjectOf(Panel, Name).ToString(); }

	/** The translation of a text the panel shows, looked up by the panel's own namespace and key (FTextInspector) in the culture's
	 *  table: a text left out of the localization (INVTEXT, AsCultureInvariant, the code before the fix) has no key and fails here. */
	bool TranslationOf(FAutomationTestBase& Test, const FTextLocalizationResource& Resource, const FText& Shown, const FString& What, FString& OutTranslation)
	{
		const TOptional<FString> Namespace = FTextInspector::GetNamespace(Shown);
		const TOptional<FString> Key = FTextInspector::GetKey(Shown);
		if (!Test.TestTrue(FString::Printf(TEXT("%s (%s) has a localization key"), *What, *Shown.ToString()), Namespace.IsSet() && Key.IsSet()))
		{
			return false;
		}
		const FTextLocalizationResource::FEntry* Entry = Resource.Entries.Find(FTextId(FTextKey(*Namespace.GetValue()), FTextKey(*Key.GetValue())));
		if (!Test.TestTrue(FString::Printf(TEXT("%s (%s,%s) is in the table"), *What, *Namespace.GetValue(), *Key.GetValue()), Entry && Entry->LocalizedString.IsValid()))
		{
			return false;
		}
		OutTranslation = *Entry->LocalizedString;
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLScenarioPanelTextTest, "HeroesOfLiberty.UI.ScenarioPanel.LocalizedOneLine",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLScenarioPanelTextTest::RunTest(const FString& Parameters)
{
	using namespace HOLScenarioPanelTestsPrivate;
	using FDisplay = HOLTestHooks::FScenarioDisplay;

	const FDisplay Saved = HOLTestHooks::GetScenarioDisplay();
	FDisplay& Display = HOLTestHooks::GetScenarioDisplay();
	Display = FDisplay();
	Display.Name = TEXT("deploy");
	Display.State = FDisplay::EState::Preparing;

	UHOLScenarioPanel* Panel = NewObject<UHOLScenarioPanel>(GetTransientPackage());
	Panel->Initialize();

	{
		const FScopedPanelLanguage French(TEXT("fr"));
		Panel->Refresh();
		TestEqual(TEXT("fr: the title"), TextOf(Panel, TEXT("ScenarioTitle")), FString(TEXT("Scénario deploy")));
		TestEqual(TEXT("fr: the state while preparing"), TextOf(Panel, TEXT("ScenarioState")), FString(TEXT("en préparation…")));
	}
	// English and Japanese cannot be switched live in the test editor (the game's localization is loaded there only by an asynchronous
	// preview: SetCurrentLanguage left the panel French, GALADRIEL-1114c), so the texts the panel shows are translated through the very
	// files the game loads (Content/Localization/Game/<culture>/Game.locres), by the namespace and key the panel's code gives them, as
	// HOLGameNamesTests does; the running game's side (hol.UI.Language) is the scenarios deploy / sites_option.
	struct FExpected { const TCHAR* Culture; const TCHAR* Title; const TCHAR* Preparing; const TCHAR* Ready; };
	const FExpected Cultures[] = {
		{ TEXT("en"), TEXT("Scenario deploy"), TEXT("preparing…"), TEXT("ready") },
		{ TEXT("ja"), TEXT("シナリオ deploy"), TEXT("準備中…"), TEXT("準備完了") } };
	for (const FExpected& Expected : Cultures)
	{
		FTextLocalizationResource Resource;
		const FString File = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Localization/Game"), Expected.Culture, TEXT("Game.locres"));
		if (!TestTrue(FString::Printf(TEXT("%s is readable"), *File), Resource.LoadFromFile(File, 0)))
		{
			continue;
		}
		Display.State = FDisplay::EState::Preparing;
		Panel->Refresh();

		// The title: the panel's own pattern, translated, formatted with the panel's own argument (the scenario's name).
		const FText TitleShown = TextObjectOf(Panel, TEXT("ScenarioTitle"));
		TArray<FHistoricTextFormatData> Formats;
		FTextInspector::GetHistoricFormatData(TitleShown, Formats);
		const FHistoricTextFormatData* TitleFormat = Formats.FindByPredicate([](const FHistoricTextFormatData& Data)
			{ return FTextInspector::GetKey(Data.SourceFmt.GetSourceText()).IsSet(); });
		if (TestNotNull(FString::Printf(TEXT("%s: the title (%s) is a localized pattern"), Expected.Culture, *TitleShown.ToString()), TitleFormat))
		{
			FString Pattern;
			if (TranslationOf(*this, Resource, TitleFormat->SourceFmt.GetSourceText(), FString::Printf(TEXT("%s: the title's pattern"), Expected.Culture), Pattern))
			{
				const FString Title = FText::Format(FTextFormat::FromString(Pattern), TitleFormat->Arguments).ToString();
				TestEqual(FString::Printf(TEXT("%s: the title"), Expected.Culture), Title, FString(Expected.Title));
				TestNotEqual(FString::Printf(TEXT("%s: the title is not the French source"), Expected.Culture), Title, TitleShown.ToString());
			}
		}

		// The states.
		const auto CheckState = [&](const TCHAR* Which, const TCHAR* Want)
		{
			const FText Shown = TextObjectOf(Panel, TEXT("ScenarioState"));
			FString Translated;
			if (TranslationOf(*this, Resource, Shown, FString::Printf(TEXT("%s: the state %s"), Expected.Culture, Which), Translated))
			{
				TestEqual(FString::Printf(TEXT("%s: the state %s"), Expected.Culture, Which), Translated, FString(Want));
				TestNotEqual(FString::Printf(TEXT("%s: the state %s is not the French source"), Expected.Culture, Which), Translated, Shown.ToString());
			}
		};
		CheckState(TEXT("preparing"), Expected.Preparing);
		Display.State = FDisplay::EState::Ready;
		Panel->Refresh();
		CheckState(TEXT("ready"), Expected.Ready);
	}

	// One line: the state never wraps while preparing or ready; only an error (its text may be long) wraps.
	UCommonTextBlock* State = Panel->WidgetTree ? Panel->WidgetTree->FindWidget<UCommonTextBlock>(TEXT("ScenarioState")) : nullptr;
	if (TestNotNull(TEXT("the state's text block"), State))
	{
		Display.State = FDisplay::EState::Preparing;
		Panel->Refresh();
		TestFalse(TEXT("the state does not wrap while preparing"), State->GetAutoWrapText());
		Display.State = FDisplay::EState::Ready;
		Panel->Refresh();
		TestFalse(TEXT("the state does not wrap once ready"), State->GetAutoWrapText());
		Display.State = FDisplay::EState::Failed;
		Display.Error = TEXT("a long error");
		Panel->Refresh();
		TestTrue(TEXT("an error wraps"), State->GetAutoWrapText());
	}

	Display = Saved;
	return true;
}

// The cheats' panel (HOLCheatPanel) on the real widget: its buttons (« +1000 Nourriture »...) and its speeds' labels (« Vitesse des unités :
// x1.0 ») come from the localization tables, English and Japanese translated (recette 2026-11-14: still French in EN/JA, written as
// culture invariant texts). Same way as above: French from the widget, English and Japanese through Game.locres by the panel's own keys.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCheatPanelTextTest, "HeroesOfLiberty.UI.CheatPanel.Localized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLCheatPanelTextTest::RunTest(const FString& Parameters)
{
	using namespace HOLScenarioPanelTestsPrivate;

	UHOLCheatPanel* Panel = NewObject<UHOLCheatPanel>(GetTransientPackage());
	Panel->Initialize();

	const auto ButtonText = [Panel](const TCHAR* Name)
	{
		const UHOLQueueItem* Button = Panel->WidgetTree ? Panel->WidgetTree->FindWidget<UHOLQueueItem>(FName(Name)) : nullptr;
		const UCommonTextBlock* Label = Button && Button->WidgetTree ? Button->WidgetTree->FindWidget<UCommonTextBlock>(TEXT("Label")) : nullptr;
		return Label ? Label->GetText() : FText::AsCultureInvariant(TEXT("<no widget>"));
	};
	const auto LabelText = [Panel](const TCHAR* Name)
	{
		const UCommonTextBlock* Label = Panel->WidgetTree ? Panel->WidgetTree->FindWidget<UCommonTextBlock>(FName(Name)) : nullptr;
		return Label ? Label->GetText() : FText::AsCultureInvariant(TEXT("<no widget>"));
	};

	struct FButton { const TCHAR* Name; const TCHAR* Fr; const TCHAR* En; const TCHAR* Ja; };
	const FButton Buttons[] = {
		{ TEXT("CheatFood"), TEXT("+1000 Nourriture"), TEXT("+1000 Food"), TEXT("+1000 食料") },
		{ TEXT("CheatWood"), TEXT("+1000 Bois"), TEXT("+1000 Wood"), TEXT("+1000 木材") },
		{ TEXT("CheatStone"), TEXT("+1000 Pierre"), TEXT("+1000 Stone"), TEXT("+1000 石材") },
		{ TEXT("CheatGold"), TEXT("+1000 Or"), TEXT("+1000 Gold"), TEXT("+1000 金") },
		{ TEXT("CheatAll"), TEXT("+1000 de tout"), TEXT("+1000 of everything"), TEXT("全資源 +1000") },
		{ TEXT("CheatPopulation"), TEXT("Population +165"), TEXT("Population +165"), TEXT("人口 +165") },
		{ TEXT("CheatFastBuild"), TEXT("Construction rapide : non"), TEXT("Fast build: off"), TEXT("高速建設：オフ") } };
	struct FSpeed { const TCHAR* Name; const TCHAR* Fr; const TCHAR* En; const TCHAR* Ja; };
	const FSpeed Speeds[] = {
		{ TEXT("CheatUnitSpeedLabel"), TEXT("Vitesse des unités : x1.0"), TEXT("Unit speed: x1.0"), TEXT("ユニット速度：x1.0") },
		{ TEXT("CheatGatherSpeedLabel"), TEXT("Vitesse de collecte : x1.0"), TEXT("Gather speed: x1.0"), TEXT("採集速度：x1.0") } };

	{
		const FScopedPanelLanguage French(TEXT("fr"));
		for (const FButton& Button : Buttons)
		{
			TestEqual(FString::Printf(TEXT("fr: %s"), Button.Name), ButtonText(Button.Name).ToString(), FString(Button.Fr));
		}
		for (const FSpeed& Speed : Speeds)
		{
			TestEqual(FString::Printf(TEXT("fr: %s"), Speed.Name), LabelText(Speed.Name).ToString(), FString(Speed.Fr));
		}
	}

	for (const TCHAR* Culture : { TEXT("en"), TEXT("ja") })
	{
		const bool bJapanese = FCString::Strcmp(Culture, TEXT("ja")) == 0;
		FTextLocalizationResource Resource;
		const FString File = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Localization/Game"), Culture, TEXT("Game.locres"));
		if (!TestTrue(FString::Printf(TEXT("%s is readable"), *File), Resource.LoadFromFile(File, 0)))
		{
			continue;
		}
		for (const FButton& Button : Buttons)
		{
			const FText Shown = ButtonText(Button.Name);
			FString Translated;
			if (TranslationOf(*this, Resource, Shown, FString::Printf(TEXT("%s: %s"), Culture, Button.Name), Translated))
			{
				TestEqual(FString::Printf(TEXT("%s: %s"), Culture, Button.Name), Translated, FString(bJapanese ? Button.Ja : Button.En));
			}
		}
		// A speed's label: the panel's pattern and its argument 0 (the speed's name) translated by their keys, the value (argument 1) kept.
		for (const FSpeed& Speed : Speeds)
		{
			const FText Shown = LabelText(Speed.Name);
			TArray<FHistoricTextFormatData> Formats;
			FTextInspector::GetHistoricFormatData(Shown, Formats);
			const FHistoricTextFormatData* Format = Formats.FindByPredicate([](const FHistoricTextFormatData& Data)
				{ return FTextInspector::GetKey(Data.SourceFmt.GetSourceText()).IsSet(); });
			if (!TestNotNull(FString::Printf(TEXT("%s: %s (%s) is a localized pattern"), Culture, Speed.Name, *Shown.ToString()), Format))
			{
				continue;
			}
			FString Pattern;
			FString Name;
			const FFormatArgumentValue* NameArgument = Format->Arguments.Find(TEXT("0"));
			if (TestNotNull(FString::Printf(TEXT("%s: %s has its name as argument 0"), Culture, Speed.Name), NameArgument)
				&& NameArgument->GetType() == EFormatArgumentType::Text
				&& TranslationOf(*this, Resource, Format->SourceFmt.GetSourceText(), FString::Printf(TEXT("%s: %s's pattern"), Culture, Speed.Name), Pattern)
				&& TranslationOf(*this, Resource, NameArgument->GetTextValue(), FString::Printf(TEXT("%s: %s's name"), Culture, Speed.Name), Name))
			{
				FFormatNamedArguments Arguments = Format->Arguments;
				Arguments.Add(TEXT("0"), FText::AsCultureInvariant(Name));
				const FString Label = FText::Format(FTextFormat::FromString(Pattern), Arguments).ToString();
				TestEqual(FString::Printf(TEXT("%s: %s"), Culture, Speed.Name), Label, FString(bJapanese ? Speed.Ja : Speed.En));
			}
		}
	}
	return true;
}

#endif
