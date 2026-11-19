# ELROND-1119b : HUD H-3, liaison + bascule + retrait (branche tiers, NON COMPILE, NON LANCE)
Impact joueur : aucun changement visible attendu (barre du haut, Inactifs, Menu / Statistiques passent au script).

## Commits (tiers, worktree HeroesOfLiberty-Elrond)
1. 0433bc5 liaison : UHOLScriptScreen::GetEconomyNow, GetIdleWorkerCount, GetStatsButtonShown, SelectNextIdleWorker, OpenGameMenu, OpenStats, GetResourceColor (static ; muted = TextMuted pour hors 0..3) ; UHOLQueueItem::SetLabel UFUNCTION ; include HOLResources.h dans HOLScriptScreen.h.
2. 69e5012 HOLTopBar.as : GetResourceColor appele en statique `UHOLScriptScreen::GetResourceColor` (comme GetTextColor).
3. d5c333f bascule + retrait (un seul commit : les retraits ont ete faits avec la bascule) : crochet HOLGameScreenScript.as (membre TopBar, Build apres BuildVolcanoBanner, Refresh apres RefreshVolcano, ClickWidget avant return false, MenuPanel du bloc heros via GetWidgetBox(TopBar.MenuPanel)) ; HOLGameScreen.cpp/.h ne construit plus TopBar / Resources / IdleWorkersBox / MenuPanel / MenuRow, plus de liaison de clics, plus de branches ClickWidget Menu / Stats / Idle, plus de HandleMenu/Stats/IdleWorkersClicked ni RefreshStatsButton, plus de membres widgets ni ShownIncome, constantes IdleWorkersName/MenuButtonName/StatsButtonName/IdleWorkersWidth/MarkerSize retirees (diff : 4 fichiers, +28 -271).
Non fait : scenario ecran_hud_hautdroite (section g) faute d'appels ; le retrait n'a pas fait l'objet d'un grep Script\ des symboles retires (ils sont des membres prives de UHOLGameScreen, aucune UFUNCTION retiree).

## Choix
- (c).1 Describe : RefreshTopBar reduit (bEconomyShown + Shown* copies, aucun widget) et RefreshIdleWorkers reduit (ShownIdleWorkers = compteur du controleur, appele aux memes endroits) : lignes hol.HUD.Report mot pour mot.
- (c).2 GetTopBar() : definie dans le .cpp, rend `ScriptScreen->WidgetTree->FindWidget("TopBar")` (par nom, a chaque appel, pas d'UFUNCTION SetTopBarWidget). SetAnchorWidget NON LU (grep sans resultat dans HOLCheatPanel.cpp) : si le panneau garde un pointeur pris avant la construction du script, a verifier ; HOLGameHUD.cpp:89/97 inchanges.
- (c).3 MenuPanel : sorti de GameNames (HOLScriptScreen.cpp, il reste PlayersPanel) ; le TopBar et le MenuPanel du script sont des Border externes de l'arbre du script, donc recoltes par la boucle de GatherPanelBoxes (DescribeRects / DescribeOverlaps les comptent, MenuBox de DescribeOverlaps trouve "MenuPanel" par nom) ; bloc du heros : GetWidgetBox(TopBar.MenuPanel, ...).
- HoverIncome (C++, scenario income_tooltip) lisait FoodAmount.. : il cherche maintenant les 4 *Amount par nom dans l'arbre du script.

## Outils -> arbre du script
- hol.Mouse.Hover WoodAmount / GoldAmount : OUI, HOLUIMouse.cpp:105-122 (FindIn recurse dans les UUserWidget tenus, profondeur 8, dont ScriptScreen).
- hol.HUD.Click : OUI, HOLScriptScreen::ClickWidget (script) appele avant le C++ (HOLGameScreen.cpp ClickWidget) ; TopBar.ClickName gere IdleWorkers / MenuButton / StatsButton.
- hol.UI.Focus StatsButton (quick_check.ps1:768) : NON pour l'arbre du HUD : HOLUIOpen.cpp:163-164 ne cherche que le modal et le menu actif ; il ne cherchait deja pas le HUD C++. Le StatsButton vise est donc probablement celui d'un ecran menu (HOLStatsScreen.* contient "StatsButton") : NON VERIFIE. Aucune extension faite.
| quick_check.ps1:1560, 6377, 6425-6429 | Describe C++ garde (c).1 |
| lobby_match.ps1:164-165,170,586 | script ClickWidget -> OpenStats / OpenGameMenu (journaux ecrits par UHOLStatsScreen::Open / UHOLGameMenuScreen::Open) |
| income_tooltip.txt | Mouse.Hover OUI ; hol.HUD.HoverIncome via AmountNamed |
| HOLGameHUD.cpp:89,97 | GetTopBar par nom |

## Suspects de compilation
- Script : `Screen.ConstructWidget` depuis un autre objet, Cast<UHOLQueueItem>, f-string `{Stock.Food}`, DescribeEconomy() const, GetWidgetBox(TopBar.MenuPanel,...) (UBorder -> UWidget*), liaison statique GetResourceColor.
- C++ : HOLScriptScreen.cpp inclut Screens/HOLGameMenuScreen.h, HOLStatsScreen.h, Player/HOLGameMenuRulesBridge.h, HOLPlayerController.h, Match/HOLGameState.h ; GetEconomyNow est BlueprintPure avec 4 sorties struct (si le generateur refuse, passer en BlueprintCallable) ; GetIdleWorkers() const sur le controleur ; lambda AmountNamed dans HoverIncome ; membres inutilises restants dans HOLGameScreen.cpp (variables locales du BuildLayout) a verifier, avertissement = erreur possible.
- HOLGameScreen.h : `UBorder` encore declare en avance ? (GetTopBar renvoie UBorder*, inchange). Les `#include` Components/Image, SizeBox... restent utilises ailleurs.

## Validation (session principale)
- `Tools\build.ps1 -LiveCoding=0 -Wait` sur le worktree Elrond, puis `quick_check.ps1 -Parts` (HUD, hud_rects, scripts : HOLTopBar_Test, lignes 1560, 6377, 6425-6429), `hud_rects_game_test.ps1`, `Tools\Scenarios\income_tooltip.txt`, lobby_match.ps1 (Menu / Stats), preflight de peasant_fight.
- Rouges de la recette (f) : FormatAmount `{Value + 1}` ; WorkersLine sans Math::Max.

BUILDS_DEMANDES: 1. LIGNES_CPP_SUPPRIMEES: ~271 (net, 4 fichiers). TESTS_CPP_RETIRES: 0. TESTS_SCRIPT_AJOUTES: 0 (5 de 1119a). TYPES_A_EXPOSER: fait (0433bc5).
