# GALADRIEL-1117h : MIG-Market-NewBook (NewBook porte par le pont, copie C++ supprimee, D836)

Worktree HeroesOfLiberty-Galadriel, branche galadriel-tab. Commits : 4558af0 (tests script, rouge), 3e22e03 (code). C++ : OUI, NON COMPILE (aucun build lance). Aucun push, aucune fusion. `git merge main` fait, BOM de Tools\Tests\quick_check.ps1 = efbbbf (ok).

## Frequence et sort
| Element | Frequence | Sort |
|---|---|---|
| NewBook (HOLGameState::ResetMarketBook) | 1 par partie | PORTE : UHOLMarketSubsystem::NewBook -> NewBookNow -> FreshBook (override HOLMarketBoardScript.as) -> HOLMarket::NewBook (HOLMarketRules.as) ; copie C++ HOLMarket::NewBook supprimee (.h -1, .cpp -10) ; sans carte script : livre vide (zeros) + erreur loggee, pas de repli C++ |
| Devis Slate (HOLGameScreen.cpp:2381, Sell/BuyWithGold) | par rafraichissement | NON TOUCHE (C++, mesure) |
| LoadStandardSettings / SettingsFromRow | 1 fois | restent C++ (UDataTable) |

## Tests
| Test | Sort |
|---|---|
| Script Test_MarketNewBookShortSettings (HOLMarketNewBook_Test.as) | NEUF, rouge au commit 4558af0 : NewBook indexait BasePrices[Resource] sans garde (tableau court du pont = valeur absente comptee 0, comme FromBridge) ; vert apres le garde (3e22e03). Rouge a voir par valeur faussee apres le build |
| Script Test_MarketNewBookAtRest | NEUF : 4 prix, bois 80, pierre 120 (= les 2 assertions C++ retirees) |
| C++ Market.Rules, assertions "fresh book: wood at 80 / stone at 120" (2 lignes) | RETIREES (equivalent : Test_MarketNewBookAtRest, aussi Test_MarketRulesVectors l.304-305) ; le livre du test est ecrit a la main depuis les bases |
| C++ Market.ServerBoard | +1 boucle : UHOLMarketSubsystem::NewBook(World, Settings) = bases (4 assertions) |
Tests C++ retires : 0 entier, 2 assertions. Tests script ajoutes : 2. Total quick_check attendu : 1394 + 2 = 1396 (tests script comptes), apres build.

## Commande
`C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\build.ps1 -LiveCoding=0 -Wait`, puis `quick_check -Parts Cpp,Script` (attendus : Market.ServerBoard, Market.Rules, Market.Chain verts ; Test_MarketNewBook* verts).

## Non verifie
Compilation ; UFUNCTION BlueprintNativeEvent FreshBook avec un seul parametre struct (meme forme que RecoverPrices) ; que plus aucun appelant C++ de HOLMarket::NewBook (grep Source : seulement HOLGameState et le test, tous deux modifies) ; HOLGameState::ResetMarketBook appelee quand le monde a deja la carte script (sinon livre a zero, erreur loggee).
