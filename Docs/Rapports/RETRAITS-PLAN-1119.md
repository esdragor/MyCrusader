# RETRAITS-PLAN-1119 : recette des 5 retraits C++ (lecture seule)

Worktree lu : `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, HEAD d8fcc64 (confirmé par `git rev-parse`). Aucun fichier de code modifié.
Impact joueur : aucun (code mort ou repli) ; le seul risque est de perdre un test sans équivalent.
Chemins ci-dessous relatifs au worktree. `Source\HeroesOfLiberty` = H, `Source\HeroesOfLibertyUI` = UI.

## Constat transversal (bloquant pour 2 et 3)

Les tests C++ `HeroesOfLiberty.UI.ScenarioPanel.LocalizedOneLine` et `HeroesOfLiberty.UI.CheatPanel.Localized` (même fichier
`UI\Tests\HOLScenarioPanelTests.cpp`, 288 l.) font deux choses : (i) le français lu sur le vrai widget, (ii) l'anglais et le japonais
lus dans `Content\Localization\Game\<culture>\Game.locres` (`FTextLocalizationResource`) par espace de noms + clé. Le script ne sait
PAS lire un `.locres` : `Script\UI\HOLScenarioPanelScript_Test.as:8-9` l'écrit noir sur blanc (« NOT PORTED »), et
`HOLCheatPanelScript_Test.as` n'a aucun cas en/ja (grep `locres` : 0 résultat). Donc le (ii) n'a PAS d'équivalent script ; retirer les
panneaux C++ sans lui ferait baisser le total de tests.
Les clés restent valides : le script utilise les mêmes `NSLOCTEXT("HOLCheatPanel"|"HOLScenarioPanel", clé)`
(`Script\UI\HOLCheatPanelScript.as:12-26`, `HOLScenarioPanelScript.as:41-49`), et `Tools\Localization\fill_po.py:715-732` les renseigne : ne rien y toucher.
Décision à prendre avant d'écrire : soit un petit test C++ neuf « clés du script présentes et traduites dans Game.locres » (lit la table
`fill_po.py` ou une liste en dur, indépendant des panneaux, ~60 l., à fusionner avec le lot C++ de la séance), soit une liaison
`HOLScreenTestLibrary` (UI\Tests\HOLScreenTestLibrary.h) qui rend la traduction d'une clé pour la culture pour la lire depuis le script.

## 1. Test C++ TierCheat

(a) Références restantes (Source/Script/Tools) :
- `H\Tests\HOLTierCheatTests.cpp:14` (`FHOLTierCheatTest`, "HeroesOfLiberty.Match.TierCheat"), 55 l. au total.
- `H\Tests\HOLTestWorldLibrary.h:70` : commentaire seul (« as HOLTierCheatTests.cpp »), à reformuler (« as Script\Tests\HOLTierCheat_Test.as »).
- `Script\Tests\HOLTierCheat_Test.as:1,7,53` : le portage. Aucun `.Build.cs` ni outil (`Tools\`) ne cite le test (grep : 0).
(b) Équivalence cas par cas (C++ `HOLTierCheatTests.cpp` ligne -> `HOLTierCheat_Test.as` ligne), `Test_TierCheat` :
- 23 TestNotNull game state -> 60 AssertTrue State != nullptr (plus 56 : monde créé)
- 27 tier 0 sans commande -> 19 ; 28 Ram refusé à 0 -> 20
- 31 tier 3 forcé -> 24 ; 32 Ram plus refusé -> 25 ; 33 autres slots intacts -> 27 ; 34 pas d'override -> 29
- 37 au-dessus de 3 plafonné -> 33 ; 39 tier forcé plus bas lu -> 36 ; 41 tier 0 forcé = override -> 39 ; 43 retour au niveau du héros -> 42
- 45 slot hors table -> 45 ; 48 `TierOverrides` répliquée -> 47 (+49 : contrôle négatif ajouté, nom inexistant -> faux)
- Les 12 assertions + le TestNotNull ont leur pendant, mêmes valeurs. Le script lit `HOLSiegeCheck::Check` comme le C++.
(c) Supprimer `H\Tests\HOLTierCheatTests.cpp`. Retirer/reformuler le commentaire `HOLTestWorldLibrary.h:70`. Rien d'autre.
(d) Condition : « rouge vu » est annoncé dans l'en-tête du `.as` (ligne 5 dit « stays until seen red then green ») et dans ELROND-1119r/s ; je n'ai PAS relu ces rapports (voir NON VÉRIFIÉ). L'agent d'écriture doit lire la ligne « rouge vu » de ELROND-1119s avant de supprimer.
(e) 55 lignes C++.

## 2. HOLScenarioPanel.cpp / .h

(a) Références :
- `UI\HUD\HOLScenarioPanel.cpp` (314 l.), `UI\HUD\HOLScenarioPanel.h` (87 l.) : définition (`UHOLScenarioPanel`, namespace `UE::HeroesOfLiberty::UI::ScenarioPanel` cpp:24).
- `UI\Tests\HOLScenarioPanelTests.cpp:10` (`#include "HUD/HOLScenarioPanel.h"`), 27-47 (helpers `HOLScenarioPanelTestsPrivate`), 69-162 (test), seul code qui instancie `UHOLScenarioPanel` (:82). Aucun autre `#include` du .h dans `Source\` (grep).
- Commentaires seuls : `UI\HUD\HOLScriptScreen.cpp:516,696-697` (le 697 teste le nom `ScenarioPanel` du Border script, À GARDER), `H\Tests\HOLScenarioNoteTests.cpp:9`, `UI\Tests\HOLNewGameSitesBoxTests.cpp:37`.
- Script (le remplaçant, déjà branché) : `Script\UI\HOLGameScreenScript.as:150-151,286-287,1429-1430,2384`, `HOLScenarioPanelScript.as`, `HOLScenarioPanelScript_Test.as`.
- Tools : `Tools\Localization\fill_po.py:715-719` (clés, inchangées). Aucune ligne de journal du panneau lue par `.ps1` n'a été trouvée par grep sur `ScenarioPanel` (les lignes `HUD overlap:` / `scenario` viennent de `HOLScriptScreen.cpp:696`, qui reste).
(b) Test C++ dépendant : `HeroesOfLiberty.UI.ScenarioPanel.LocalizedOneLine` (`HOLScenarioPanelTests.cpp:69`).
- Pendant script existant : `HOLScenarioPanelScript_Test.as` : `Test_ScenarioPanelFrenchTexts` (fr titre/état/Relancer/×), `Test_ScenarioPanelStateWrapsOnlyAnError` (les 3 cas wrap), + cas ajoutés (`ShownRule`, `DescribeIsTheCppLine`, journal, clics).
- MANQUE : la partie en/ja par `.locres` (lignes 95-142 du C++ : titre, préparation, prêt, 2 cultures, + « n'est pas la source française »). À écrire : voir constat transversal.
- Le fichier script de test porte l'en-tête « NOT COMPILED » (ligne 1) : à re-vérifier qu'il compile et que ses tests sont VUS ROUGES (falsifier une valeur une fois) avant retrait.
(c) Fichiers à supprimer : `HOLScenarioPanel.cpp`, `HOLScenarioPanel.h`. Lignes à retirer ailleurs : dans `HOLScenarioPanelTests.cpp` le `#include` ligne 10, le test lignes 69-162 (et les helpers qu'il est seul à utiliser, à vérifier : `TextOf/TextObjectOf` 41-47) ; commentaires `HOLScriptScreen.cpp:516`, `HOLScenarioNoteTests.cpp:9`, `HOLNewGameSitesBoxTests.cpp:37` (facultatif).
(d) Bloqué par : le (ii) en/ja sans équivalent. Sinon aucun appelant de production.
(e) 401 l. (cpp 314 + h 87) + ~97 l. de test = ~498 l.

## 3. HOLCheatPanel.cpp / .h

(a) Références :
- `UI\HUD\HOLCheatPanel.cpp` (483 l.), `UI\HUD\HOLCheatPanel.h` (149 l.) ; constantes `UE::HeroesOfLiberty::UI::CheatPanel` (cpp:27-104) utilisées seulement dans ce .cpp (grep `CheatPanel::` : rien ailleurs).
- `#include "HUD/HOLCheatPanel.h"` sans AUCUN usage du type derrière (grep ne trouve que l'include) dans : `UI\Dev\HOLUIHUD.cpp:7`, `UI\Dev\HOLUIMouse.cpp:16`, `UI\HUD\HOLGameHUD.cpp:12`, `UI\HUD\HOLScriptScreen.cpp:13`, `UI\Tests\HOLScenarioPanelTests.cpp:9`.
- Commentaires : `UI\HUD\HOLGameScreen.h:247` (« UHOLCheatPanel »), `HOLScriptScreen.cpp:650-651` (le script, à garder).
- Statiques du C++ lues par le script : AUCUNE n'est dans HOLCheatPanel.cpp. `CheatsStartOpen` est `UHOLHUDLayoutReadLibrary::CheatsStartOpen` (`UI\HUD\HOLHUDLayoutReadLibrary.cpp:16`, .h:54) sur `HOLHUDLayout::CheatsStartOpen` (`HOLHUDLayoutRules.h:35`) ; le script l'appelle par `HOLHUDLayoutRead::CheatsStartOpen` (`HOLCheatPanelScript.as:298`, test `Script\Tests\HOLS1dLibraries_Test.as:33`) et `HOLHUDLayoutRead::ShouldFoldCheats` (:461). Le reste (`GetSavedOpen/SetSavedOpen/GetWindowHeight/IsServerLocal/GetServerBuildTime/GetServerSpeed`, `HOLCheatPanelScript.as:297,345,421-431,457`) est dans `UI\HUD\HOLCheatSettingsLibrary.cpp/.h` (section ini `HOL.CheatPanel`, cpp:13), fichier distinct qui reste. Rien ne déménage.
- Tools : `fill_po.py:720-732` seulement. Les lignes de journal du panneau (`HUD: cheat ... clicked`, `HUD: cheats open/folded`, `hol.HUD.Report`) sont produites aujourd'hui par le script (`HOLCheatPanelRules::ClickLine/ToggleLine/Describe`, test lignes 86-98) ; je n'ai pas grepé les `.ps1` de scénarios pour ces motifs (voir NON VÉRIFIÉ).
(b) Test dépendant : `HeroesOfLiberty.UI.CheatPanel.Localized` (`HOLScenarioPanelTests.cpp:167-286`).
- Équivalent script partiel : `HOLCheatPanelScript_Test.as:73` `Test_CheatPanelLabelsAreTheCppOnes` couvre le français de 5 libellés sur 7 boutons + 3 autres (toggle, construction rapide) et `SpeedLabelText` (x1.0, x2.5). Il manque en français : CheatWood (1), CheatStone (2), CheatGold (3) (le C++ boucle sur les 7 boutons + 2 labels de vitesse) ; le `ButtonText` du C++ lit le vrai widget, le script teste la règle pure (pas le widget construit).
- MANQUE : en/ja (7 boutons + 2 labels de vitesse, motif `{0}: x{1}` et nom en argument 0) : voir constat transversal.
- En-tête du `.as` de test : « NOT COMPILED YET » : à re-vérifier compilé + rouge vu.
(c) Supprimer `HOLCheatPanel.cpp`, `HOLCheatPanel.h`. Retirer : 4 `#include` (HOLUIHUD.cpp:7, HOLUIMouse.cpp:16, HOLGameHUD.cpp:12, HOLScriptScreen.cpp:13), l'include `HOLScenarioPanelTests.cpp:9` et le test 164-286, commentaire `HOLGameScreen.h:247`. `HOLHUDLayoutRules::ShouldFoldCheats/CheatsStartOpen` restent (utilisés par la bibliothèque de lecture et leurs tests).
(d) Bloqué par : en/ja sans équivalent + trois boutons non couverts en français. Autre chose : vérifier que `HOLUIHUD.cpp`, `HOLUIMouse.cpp`, `HOLGameHUD.cpp` n'utilisent rien d'autre venant transitivement de ce .h (le .h inclut `CommonUserWidget.h`) : compiler.
(e) 632 l. (cpp 483 + h 149) + ~123 l. de test = ~755 l.

## 4. S1 : FHOLPauseRules et Net.PauseLimits

(a) Références :
- `H\Net\HOLPauseRules.cpp` (76 l.) et `.h` (51 l.), `struct FHOLPauseRules` (.h:14) ; `#include "Net/HOLPauseRules.h"` : `H\Match\HOLGameState.h:17`, `H\Match\HOLPauseLibrary.cpp:8`, `H\Tests\HOLNetTests.cpp:20`.
- Le repli C++ (celui qu'on a gardé), tous dans `H\Match\HOLGameState.cpp` : 1456 `ScriptAnswer >= 0 ? static_cast<EAnswer>(ScriptAnswer) : FHOLPauseRules::CanPause(...)` ; 1486 idem avec `CanResume` ; 1462-1463 `AskPausesLeftAfterPause` puis `FHOLPauseRules::Pause(...)` ; ~1480 `Resume` ; 1411-1416 `TimerLeft` ; 1448/1480 `FHOLPauseRules::FState` ; 1459/1489 `ToString` (journal « Pause: refused to ... ») ; 33 et .h:774 `DefaultPausesPerMatch` ; 1411 `DefaultTimerSeconds`.
- Autre appelant NON listé dans la demande : `H\Match\HOLPauseLibrary.cpp:19-24,73-82` (`MakeState`, `CanPause/CanResume` pour l'interface, rend `== Allowed`) ; commentaires `HOLPauseLibrary.h:12`, `HOLMatchFlowSubsystem.h:13-15`.
- Script : `Script\Match\HOLMatchFlowBoard.as:4,7`, `Script\Match\HOLMatchFlow_Test.as:6-7`.
- `.Build.cs` : aucune mention (grep : 0).
(b) Test C++ : `HeroesOfLiberty.Net.PauseLimits` (`FHOLNetPauseLimitsTest`, `HOLNetTests.cpp:1132-1190`, ~60 l., ~33 assertions : 3 pauses par défaut, CanPause en match / hors match, NotPaused, Pause (état, slot, 2 restantes), TimerLeft à 100/145/200, AlreadyPaused, Resume par l'initiateur à 101, TimerRunning à 159, autre à 160, spectateur, défait, Resume, 3 pauses épuisées, NoPauseLeft, listen server, solo sans limite ni minuterie).
- Équivalent script : `HOLMatchFlow_Test.as` (en-tête lignes 6-7 : « replays, value for value, the C++ test ... : this file IS the agreement test »). Je n'ai PAS comparé assertion par assertion, ni vérifié rouge/vert : à faire par l'agent d'écriture (nom exact des tests du `.as` à lister).
(c) Ordre obligatoire : 1) vérifier l'équivalence + rouge/vert ; 2) réécrire `HOLGameState.cpp` 1405-1495 : supprimer les branches `: FHOLPauseRules::...`, garder seulement la réponse du script ; décider du cas « -1 » (monde sans carte de script : `AskCan*` rend -1, `HOLMatchFlowSubsystem.h:13`) ; 3) déplacer `DefaultPausesPerMatch` / `DefaultTimerSeconds` (HOLGameState.cpp:33,1411, .h:774) dans `HOLGameState.h` ou les lire du script ; remplacer `FState` et `TimerLeft`/`Pause`/`Resume` du GameState par de simples champs ; 4) `HOLPauseLibrary.cpp` : faire passer `CanPause/CanResume` par le script (`UHOLMatchFlowSubsystem::Ask*`) ou le retirer ; 5) supprimer `HOLPauseRules.cpp/.h`, le test (`HOLNetTests.cpp:1130-1190`) et l'include (:20).
(d) Si on retire le repli sans plus : un monde sans script (script non compilé) a `ScriptAnswer = -1` : `static_cast<EAnswer>(-1)` serait invalide ; il faut une réponse définie (refuser avec journal d'erreur, comme `HOLMarketSubsystem.cpp:33` qui rend `Sent` avec `UE_LOG Error`). Les tests C++ du GameState qui construisent un monde sans script (par ex. DisconnectPause, que `HOLMatchFlow_Test.as:7` « replays » ?) verraient la pause refusée : à chercher dans `H\Tests\` (non fait). Touche du code appelé en réseau (serveur) : lot C++ unique, un build.
(e) ~127 l. (cpp+h) + ~60 l. de test + ~15 l. de repli dans GameState + ~10-20 l. de bibliothèque = ~215-225 l.

## 5. R1 : UHOLMarketSubsystem::Received / TributeReceived

(a) Références :
- `H\Match\HOLMarketSubsystem.h:35` (`static Received`), `:63` (`ReceivedNow`), `:94-97` (`TributeReceived` BlueprintNativeEvent + `_Implementation`) ; commentaire `:15`.
- `H\Match\HOLMarketSubsystem.cpp:27-35` (`Received` statique : repli « arrives untaxed », rend `Sent` avec `UE_LOG Error`), `:113-116` (`ReceivedNow`), `:202-206` (`TributeReceived_Implementation`).
- Commentaire `H\Match\HOLMarketRules.h:15` (« reached through UHOLMarketSubsystem::Received »), à corriger.
- Surcharge script : `Script\Economy\HOLMarketBoardScript.as:63-66` (`int TributeReceived(...) { return HOLTribute::Received(...); }`), commentaire :3.
- Appelants de production du statique `Received` : AUCUN (grep sur `Source\` : seuls le .cpp et le test). Le tribut réel passe par `SettleTribute` (`HOLProductionSubsystem.cpp:~255-265` lit `Done.Received`, la règle script `HOLTributeRules.as:72,131` appelle `Received` du script). `HOLGameState::NoteTributeReceived` (`HOLGameState.cpp:1359`, `.h:609`, `HOLPlayerController.cpp:1505`, `HOLProductionSubsystem.cpp:265`) est UNE AUTRE chose : à NE PAS toucher.
(b) Test C++ : `HeroesOfLiberty.Market.ServerBoard` (`H\Tests\HOLMarketBridgeTests.cpp:36`, `FHOLMarketServerBoardTest`), uniquement les lignes 91-101 (`ReceivedNow` x3 : 100->70, 250->175, 1->1 ; `Received` statique x7 : 1->1, 7->5, 10->7, 99->70, 100->70, 250->175, 1000->700). Les lignes suivantes (SettleTribute, `SettleNow`) et la partie récupération du prix restent.
- Équivalent script : `Script\Tests\HOLTributeRules_Test.as:5` `Test_TributeReceived` (70, 175, 1, 7, 100 sans taxe, 50, 0, négatifs) et `Script\Economy\HOLMarketRules_Test.as:369-372`. Les vecteurs 7->5, 99->70, 1000->700 ne sont PAS dans `HOLTributeRules_Test.as` : les ajouter (rouge : falsifier 99->71 une fois, puis vert) avant de retirer. Les cas 100->70, 250->175, 1->1, 10->7 y sont. (7->5 : 7 - 7*30/100 = 7-2 = 5, à vérifier au rouge.)
(c) Retirer : `HOLMarketSubsystem.h` lignes 35, 63, 94-97 ; `HOLMarketSubsystem.cpp` lignes 27-35, 113-116, 202-206 ; `HOLMarketBridgeTests.cpp` lignes 91-101 ; `HOLMarketBoardScript.as` lignes 63-66 (sinon le script surcharge une fonction disparue : ERREUR de compilation script) ; commentaires `HOLMarketRules.h:15`, `HOLMarketBoardScript.as:3`, `HOLMarketSubsystem.h:15`. Aucun fichier à supprimer.
(d) Rien ne bloque à part l'ordre : C++ et script dans le MÊME build (sinon le `.as` surcharge un `BlueprintNativeEvent` absent ou le C++ appelle une fonction sans script). Ajouter d'abord les 3 vecteurs au test script.
(e) ~25 l. C++ (cpp 18, h ~6) + ~11 l. de test ; ~5 l. de script en moins.

## Tableau de synthèse

| retrait | fichiers touchés | lignes C++ en moins | test script à écrire d'abord | conflit de fichier possible |
|---|---|---|---|---|
| 1 TierCheat | `H\Tests\HOLTierCheatTests.cpp` (suppr.), `H\Tests\HOLTestWorldLibrary.h:70` (commentaire) | 55 | non (porté, 12 + 1 assertions ; seulement relire « rouge vu » dans ELROND-1119s) | aucun (seulement `HOLTestWorldLibrary.h` en commentaire, personne d'autre) |
| 2 ScenarioPanel | `UI\HUD\HOLScenarioPanel.cpp/.h` (suppr.), `UI\Tests\HOLScenarioPanelTests.cpp` (include 10, test 69-162), commentaires `HOLScriptScreen.cpp:516`, `HOLScenarioNoteTests.cpp:9`, `HOLNewGameSitesBoxTests.cpp:37` | ~498 | oui : en/ja `.locres` des clés `HOLScenarioPanel,*` (titre, préparation, prêt, erreur, relancer), plus rouge/vu du `.as` « NOT COMPILED » | `HOLScenarioPanelTests.cpp` partagé avec 3 ; `HOLScriptScreen.cpp` partagé avec 3 (commentaire en 2, include en 3) |
| 3 CheatPanel | `UI\HUD\HOLCheatPanel.cpp/.h` (suppr.), `UI\Tests\HOLScenarioPanelTests.cpp` (include 9, test 164-286), includes `HOLUIHUD.cpp:7`, `HOLUIMouse.cpp:16`, `HOLGameHUD.cpp:12`, `HOLScriptScreen.cpp:13`, commentaire `HOLGameScreen.h:247` | ~755 | oui : en/ja `.locres` (7 boutons + 2 vitesses) ; en français CheatWood/Stone/Gold ; rouge vu | `HOLScenarioPanelTests.cpp` et `HOLScriptScreen.cpp` partagés avec 2 : un seul agent pour 2+3 |
| 4 PauseRules (S1) | `H\Net\HOLPauseRules.cpp/.h` (suppr.), `H\Match\HOLGameState.cpp` (1405-1495, 33, 1411), `HOLGameState.h` (17, 774), `HOLPauseLibrary.cpp`, `H\Tests\HOLNetTests.cpp` (20, 1130-1190) | ~215-225 | non si `HOLMatchFlow_Test.as` couvre les ~33 assertions (à comparer, rouge/vert à voir) ; oui sinon, les manquantes | `HOLGameState.cpp/.h` : aucun autre retrait ne les touche ; attention agents extérieurs à ce plan sur le GameState |
| 5 Tribut (R1) | `H\Match\HOLMarketSubsystem.cpp/.h`, `H\Tests\HOLMarketBridgeTests.cpp` (91-101), `Script\Economy\HOLMarketBoardScript.as` (63-66), commentaire `HOLMarketRules.h:15` | ~36 (25 + 11 de test) | oui : ajouter 7->5, 99->70, 1000->700 à `Script\Tests\HOLTributeRules_Test.as`, rouge puis vert | aucun avec 1-4 ; script et C++ dans un même build |

Total ~1560 lignes. Ordre conseillé : 1 et 5 d'abord (petits, sans conflit), 4 ensuite (lot C++ réseau, un build), 2+3 par un seul agent après le test en/ja.
Aucun de ces retraits ne touche `HOLGameScreen.cpp` (la demande l'évoquait en exemple) ; seul `HOLGameScreen.h:247` a un commentaire à reformuler.

## NON VÉRIFIÉ

- « Rouge vu » du port TierCheat : les rapports ELROND-1119r/s n'ont pas été relus (budget d'appels) ; seul l'en-tête du `.as` (« stays until seen red then green ») est lu.
- Équivalence assertion par assertion de `Net.PauseLimits` avec `HOLMatchFlow_Test.as` (non lu au-delà de l'en-tête) ; lesquels des tests C++ du GameState reposent sur le repli (monde sans script).
- Si `HOLCheatPanelScript.as`, `HOLScenarioPanelScript.as` et leurs tests compilent réellement dans d8fcc64 : les en-têtes disent « NOT COMPILED YET » alors que `HOLGameScreenScript.as:286-287` et `HOLScriptScreen.cpp:650` les branchent (en-têtes probablement périmés). Vérifier avant de s'appuyer sur « rouge/vert ».
- GALADRIEL-1119p/q, CELEBORN-1119p/q/r/m, GLORFINDEL-1119m non lus : les « annoncé » du brief n'ont pas été confrontés à eux.
- Aucun grep des scénarios `.ps1` / `.json` sur les lignes de journal du panneau de triche (`HUD: cheat ... clicked`, `cheats open`, `hol.HUD.Report`) ; le grep `Tools\` sur `CheatPanel|ScenarioPanel` ne renvoie que `fill_po.py`, mais ces lignes ne contiennent pas ces mots.
- Les `.Build.cs` : le grep `Cheat|Scenario|Pause` dans `Source\*\*.Build.cs` renvoie 0 ; je n'ai pas vérifié s'il existe une liste de tests / un plancher du total (« le total ne baisse jamais ») dans `Tools\` ou la documentation.
- Lignes de test exactes retirables (`HOLScenarioPanelTests.cpp` 41-47, helpers `TranslationOf`, `FScopedPanelLanguage` : utilisés par les deux tests, seul le dernier retrait peut les supprimer) ; nombre de lignes de `HOLCheatSettingsLibrary` non lu ; `HOLUIHUD.cpp`, `HOLUIMouse.cpp`, `HOLGameHUD.cpp` pas lus (seules les lignes d'`#include` sont connues).
- `HOLPauseLibrary` : qui appelle `CanPause/CanResume` côté interface ou script non cherché.
