#include "Blueprint/WidgetTree.h"
#include "CommonTextBlock.h"
#include "Dev/HOLTestHooks.h"
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

#endif
