# GALADRIEL-1119p : HUD H-11a, panneau de triche, lot C++ + bascule (reprise 14, 2026-11-19)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`, branche galadriel-tab. Impact joueur : aucun (outil de developpement). RIEN compile, RIEN lance (interdit).

## Commits
| Etape | sha | Contenu |
|---|---|---|
| 1 | 35ab81e | `HUD\HOLCheatSettingsLibrary.h/.cpp` (6 fonctions de la recette : GetSavedOpen, SetSavedOpen, GetWindowHeight, IsServerLocal(WorldContext), GetServerBuildTime, GetServerSpeed(Index)), `UHOLScriptScreen::SetCheatsReport` + `GetCheatsReport` (C++) + `ApplyCheatsOpen` (BlueprintNativeEvent, corps vide), `UHOLGameScreen::DescribeCheats` + `ApplyCheatsOpen` (relais vers l'ecran script) |
| 2 | 4367729 | Crochet dans `HOLGameScreenScript.as` + bascule C++ (voir ci-dessous) |
| 3 | aucun | NON FAIT, voir plus bas |
| 4 | 003a0d9 | `Tools\Scenarios\ecran_hud_triche.txt/.look/.json` (mapSize Small, hol.Test.Sanity, waitkey avant chaque etape, ScenarioClose avant chaque capture) ; non joue |

## Etape 2 : ce qui a change
- Script : membre `UHOLCheatPanelScript CheatPanel`, `Build(this, Frame, TopBar.TopBar)` APRES `BuildPauseScreen(Frame)` (dernier enfant du cadre = au-dessus de tout, comme l'ancien z 1) ; `Refresh(HeroView.bVisible)` + `SetCheatsReport(CheatPanel.Describe())` en fin de `Refresh()` ; `ClickName` dans `ClickWidget` ; `CheatBottom()` remplace les 4 `GetCheatPanelBottom()` ; `ApplyCheatsOpen` (BlueprintOverride) appelle `SetOpen(bOpen, false)` ; `SetCheatsHeroBlockShown` retire (le bloc du heros passe en parametre de `Refresh`).
- C++ : `HOLGameHUD.cpp/.h` ne cree / detruit / expose plus le panneau (`CheatPanel`, `GetCheatPanel`, `FindCheatPanel`, declaration anticipee retires) ; `HOLScriptScreen.h/.cpp` : `GetCheatPanelBottom` et `SetCheatsHeroBlockShown` retires ; `HOLUIHUD.cpp` : ligne `HUD cheats:` lue par `Screen.DescribeCheats()` (n'est ecrite que si non vide), `hol.HUD.Click` ne passe plus par le panneau C++ (le `ClickWidget` de l'ecran script le couvre), `hol.HUD.Cheats` appelle `Screen.ApplyCheatsOpen` puis garde sa ligne `HUD cheats set:` ; `HOLUIMouse.cpp` : `FindCheatPanel` retire de la liste des racines (le panneau est dans l'arbre de l'ecran, `hol.Mouse.Hover` le trouve).
- Condition de la decision Q1 (place, ordre de dessin, nom dans le releve) : nom GARANTI par le code : `GatherPanelBoxes` lit le Border `CheatPanel` de l'arbre script, l'ajoute sous le nom `cheats`, a la meme place dans la liste (en premier), avec la meme regle (replie = Collapsed ou taille nulle : exclu) et `HOLHUDSpace::BoxOf` ; la boucle des bordures externes l'ignore (sinon doublon `CheatPanel`). Ordre de dessin : dernier enfant du cadre. Place : depend de `Screen.MakePanel` (marge 16 puis `TopOf(hauteur de la barre)`) : NON VERIFIE a l'ecran, voir ci-dessous.

## Etape 3 : retrait de HOLCheatPanel.cpp/.h : NON FAIT (voulu)
Plus aucun appelant du panneau dans le jeu (grep de `UHOLCheatPanel` / `FindCheatPanel` / `GetCheatPanel` apres l'etape 2 : il ne reste que HOLCheatPanel.cpp/.h eux-memes, les `#include "HUD/HOLCheatPanel.h"` de HOLScriptScreen.cpp, HOLGameHUD.cpp, HOLUIHUD.cpp, HOLUIMouse.cpp (inutiles, laisses pour ne pas risquer une inclusion transitive manquante sans build) et `Tests\HOLScenarioPanelTests.cpp`). Le test `HeroesOfLiberty.UI.CheatPanel.Localized` construit un `UHOLCheatPanel` reel : il garde donc TOUTE la classe. Lignes C++ retirees a l'etape 2 : 75 supprimees / 40 ajoutees (hors bibliotheque neuve de 76 lignes). A retirer plus tard : HOLCheatPanel.cpp (483 l.) + .h, apres rouge puis vert d'un test script equivalent (lacune : les tests de libelle du script couvrent le FR, EN / JA restent a couvrir) ; les 4 `#include` ci-dessus a retirer en meme temps.

## Cvar hol.HUD.CheatsLowFold (Q2)
Grep de `Tools\` et `Config\` : AUCUN lecteur. Notee « sans lecteur ». Elle reste definie dans HOLCheatPanel.cpp:85-87 (non retiree), mais le panneau script ne la lit plus (le pli de fenetre basse se fait toujours) : elle n'a donc plus d'effet. A decider quand la classe C++ sera retiree (la cvar disparaitra avec elle). `hol.HUD.Cheats` garde son nom et son effet.

## Ecarts a la recette
- `SetCheatsReport` : ajout de `GetCheatsReport()` (C++) et de `DescribeCheats()` sur `UHOLGameScreen` (l'ecran script est prive dans l'ecran de jeu), comme `SetQueueReportMirror` ; `hol.HUD.Report` n'ecrit `HUD cheats:` que si le script a deja ecrit sa ligne (avant le 1er `Refresh`, pas de ligne ; avant : ligne si le panneau existait).
- Ajout de `ApplyCheatsOpen` (evenement natif) : la commande `hol.HUD.Cheats` reste en C++ (HOLUIHUD.cpp) et pilote le panneau script par ce crochet, au lieu d'etre re-enregistree en script.
- Macros : `BlueprintPure` / `BlueprintCallable` (celles de `HOLScriptScreen.h`), pas `ScriptCallable`.
- La cvar LowFold n'est pas reprise dans la bibliotheque (doublon de nom avec HOLCheatPanel.cpp qui reste).
- Scenario : l'etat « Paysans immobiles » de la lecon 14 n'est pas pose (aucune commande verifiee pour cela) ; les captures se comparent sur la zone du panneau et la barre du haut.

## NON VERIFIE (rien compile, rien joue)
- Compilation C++ (4 fichiers de HUD + bibliotheque + Dev) : `GetWorldFromContextObject`, `GGameUserSettingsIni`, `BlueprintNativeEvent` avec corps `_Implementation` vide, include de `Border`/`HOLHUDSpace` deja presents dans HOLScriptScreen.cpp.
- Compilation Angelscript : tout ce que la liste du rapport 1119m cite, plus : `HOLCheatSettings::IsServerLocal()` appele sans argument (le plugin doit fournir le contexte de monde depuis un UObject ; sinon passer `Screen`), `TopBar.TopBar` (champ `UBorder TopBar` de `UHOLTopBarPanel`), `HeroView.bVisible`, ordre de declaration `CheatBottom()` / `ApplyCheatsOpen`.
- Place a l'ecran du panneau, `MakePanel` (style / padding 8 / coins 4), clic sur le fond du panneau consomme (le C++ posait `SetConsumePointerInput(true)`, le panneau script ne le fait pas : un clic sur le fond peut passer au monde), `HUD overlap: none` et `HUD rects:` portant `cheats=`.
- Les 8 tests de `HOLCheatPanelScript_Test.as` : jamais vus rouges ni verts.

## Commandes de test (session principale)
1. Build C++ (lot de 35ab81e + 4367729), puis compilation script.
2. `Tools\Scenarios\preflight.ps1 ecran_hud_triche` ; verifier `HUD overlap: none` et `cheats=` dans `HUD rects:`.
3. `play.ps1 ecran_hud_triche -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel` ; AVANT = meme scenario sur main (copie C++).
4. Regressions a lire : `Scenarios\heros_triches_600.txt` (`waitlog HUD cheats: cheats folded`), `ai_level_line.txt`, `diplomacy_eight.txt`, `diplomacy_fit.txt` (`hol.HUD.Click CheatToggle`), lobby_match.ps1 (lignes `LogHOLUI: Display:`), quick_check.
5. Test C++ `HeroesOfLiberty.UI.CheatPanel.Localized` : doit rester vert (classe inchangee).
