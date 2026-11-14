#include "Blueprint/WidgetTree.h"
#include "CommonTextBlock.h"
#include "Dev/HOLTestHooks.h"
#include "HUD/HOLScenarioPanel.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "Misc/AutomationTest.h"
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

	FString TextOf(const UHOLScenarioPanel* Panel, const TCHAR* Name)
	{
		const UCommonTextBlock* Text = Panel && Panel->WidgetTree ? Panel->WidgetTree->FindWidget<UCommonTextBlock>(FName(Name)) : nullptr;
		return Text ? Text->GetText().ToString() : FString(TEXT("<no widget>"));
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
	for (const TCHAR* Culture : { TEXT("en"), TEXT("ja") })
	{
		const FScopedPanelLanguage Scoped(Culture);
		Display.State = FDisplay::EState::Preparing;
		Panel->Refresh();
		const FString TitleText = TextOf(Panel, TEXT("ScenarioTitle"));
		TestNotEqual(FString::Printf(TEXT("%s: the title is translated (%s)"), Culture, *TitleText), TitleText, FString(TEXT("Scénario deploy")));
		TestTrue(FString::Printf(TEXT("%s: the title names the scenario (%s)"), Culture, *TitleText), TitleText.Contains(TEXT("deploy")));
		const FString Preparing = TextOf(Panel, TEXT("ScenarioState"));
		TestNotEqual(FString::Printf(TEXT("%s: « en préparation… » is translated (%s)"), Culture, *Preparing), Preparing, FString(TEXT("en préparation…")));
		Display.State = FDisplay::EState::Ready;
		Panel->Refresh();
		const FString Ready = TextOf(Panel, TEXT("ScenarioState"));
		TestNotEqual(FString::Printf(TEXT("%s: « prêt » is translated (%s)"), Culture, *Ready), Ready, FString(TEXT("prêt")));
	}
	{
		const FScopedPanelLanguage English(TEXT("en"));
		Display.State = FDisplay::EState::Preparing;
		Panel->Refresh();
		TestEqual(TEXT("en: the title"), TextOf(Panel, TEXT("ScenarioTitle")), FString(TEXT("Scenario deploy")));
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
