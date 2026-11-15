#include "Blueprint/WidgetTree.h"
#include "Map/HOLMapRules.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Net/HOLManagerClientRules.h"
#include "Screens/HOLNewGameScreen.h"
#include "UObject/Package.h"
#include "Widgets/HOLDropdown.h"

#if WITH_DEV_AUTOMATION_TESTS

// S7 (SacredSitesInProgress.md, D699, Manager/PROTOCOL.md settings.sacredSites): the box "Sites sacres" of the new game screen is greyed on a
// map without a site layer, in solo and in a lobby (where it is also the leader's alone), through the real screen: its widgets built, the map
// given by SetConfig / EnterLobby as the menu and the manager give it, the box's state read on the widget (the effect, not a predicate).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLNewGameSitesBoxTest, "HeroesOfLiberty.UI.NewGame.SacredSitesBox",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLNewGameSitesBoxTest::RunTest(const FString& Parameters)
{
	const FSoftObjectPath Plain = FHOLMapRules::GetAssetPath(TEXT("Plain"));

	// The count before loading: the table's maps lay sites (the generator's count), a map the table does not know lays none.
	TestTrue(TEXT("Plain lays sites (the generator's count, 2 or 3)"), FHOLMapRules::GetSiteCount(Plain) >= 2);
	TestEqual(TEXT("a test map the table does not know lays no site"),
		FHOLMapRules::GetSiteCount(FSoftObjectPath(TEXT("/Game/Maps/Test/SitesBoxTest.SitesBoxTest"))), 0);
	{
		const FHOLMapRules::FScopedSiteCount NoSites(TEXT("Plain"), 0);
		TestEqual(TEXT("the hook: Plain without a site layer"), FHOLMapRules::GetSiteCount(Plain), 0);
	}
	TestTrue(TEXT("the hook gone: Plain lays its sites again"), FHOLMapRules::GetSiteCount(Plain) >= 2);

	// The real screen.
	UHOLNewGameScreen* Screen = NewObject<UHOLNewGameScreen>(GetTransientPackage());
	Screen->Initialize();
	const UHOLDropdown* Box = Screen->WidgetTree ? Screen->WidgetTree->FindWidget<UHOLDropdown>(TEXT("SacredSitesDropdown")) : nullptr;
	if (!TestNotNull(TEXT("the screen has its box Sites sacres"), Box))
	{
		return false;
	}
	FHOLMatchConfig Config;
	Config.Map = Plain;
	Screen->SetConfig(Config);
	TestTrue(TEXT("solo, Plain with its sites: the box can be ticked"), Box->GetIsEnabled());

	FHOLLobby Lobby;
	Lobby.Leader = TEXT("Alice");
	Lobby.You = TEXT("Alice");
	Lobby.Map = TEXT("Plain");
	{
		const FHOLMapRules::FScopedSiteCount NoSites(TEXT("Plain"), 0);
		Screen->SetConfig(Config);
		TestFalse(TEXT("solo, Plain without a site layer: the box is greyed"), Box->GetIsEnabled());
		Screen->EnterLobby(Lobby);
		TestFalse(TEXT("lobby, the leader, Plain without a site layer: the box is greyed"), Box->GetIsEnabled());
	}
	Screen->EnterLobby(Lobby);
	TestTrue(TEXT("lobby, the leader, Plain with its sites: the box can be ticked"), Box->GetIsEnabled());
	Lobby.You = TEXT("Bob");
	Screen->EnterLobby(Lobby);
	TestFalse(TEXT("lobby, not the leader: the box is greyed (the leader's alone)"), Box->GetIsEnabled());
	return true;
}

#endif
