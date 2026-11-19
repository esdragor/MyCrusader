#include "Blueprint/WidgetTree.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Map/HOLMapRules.h"
#include "Misc/ScopeExit.h"
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

	// The real screen. It builds its widgets in NativeOnInitialized, which the engine runs only with a player (UUserWidget::Initialize):
	// a local player and its controller in a game world, as HOLScenarioPanelTests' cheat panel (GLORFINDEL-1115b: without them the
	// screen had no widget, « the screen has its box Sites sacres » failed after the green code).
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	ULocalPlayer* Player = NewObject<ULocalPlayer>(GEngine);
	APlayerController* Controller = World->SpawnActor<APlayerController>();
	Controller->Player = Player;
	Player->PlayerController = Controller;
	if (!World->GetPlayerControllerIterator())
	{
		World->AddController(Controller);
	}
	ON_SCOPE_EXIT
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	};
	// NG-6b: the base class has no layout any more (BP_BuildNewGame is the script's): the real screen is the script's class (Script/UI/HOLNewGameScreen.as).
	const TSubclassOf<UHOLNewGameScreen> ScriptClass = UHOLNewGameScreen::FindScriptClass();
	if (!TestNotNull(TEXT("the script's New game screen class exists"), ScriptClass.Get()))
	{
		return false;
	}
	UHOLNewGameScreen* Screen = NewObject<UHOLNewGameScreen>(GetTransientPackage(), ScriptClass);
	Screen->SetPlayerContext(FLocalPlayerContext(Player, World));
	if (!TestTrue(TEXT("the screen has a player"), Screen->GetPlayerContext().IsValid()))
	{
		return false;
	}
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
