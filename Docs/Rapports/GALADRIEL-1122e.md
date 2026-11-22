# GALADRIEL-1122e : risque réel des ponts qui rendent le CDO (lecture seule)

Code lu : `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel` (Source\ et Script\). Aucun fichier de code touché.

## Verdict

Les deux ponts sont SANS RISQUE : ce sont des règles pures sans état, sans monde. Aucune correction proposée (aucun risque prouvé).

## Recensement (grep `GetDefaultObject` dans tout Source\ *.cpp/*.h)

Seulement 2 occurrences : `Player/HOLWallLineBridge.cpp:23` et `Player/HOLGameMenuRulesBridge.cpp:23`. Les autres fichiers *Bridge* de Source\ (`Dev/HOLScriptRpcProbeBridge.cpp`, `HOLMarketBridgeTypes.h`, `*BridgeTests.cpp`) ne rendent pas de CDO via GetDefaultObject (ProbeBridge prend `Controller->GetWorld()`, ligne 30, c'est le contraire du CDO).

## Tableau

| Pont | Appelants C++ (lignes) | Fonctions script appelées | Besoin d'un monde / état ? | Risque |
|---|---|---|---|---|
| UHOLWallLineBridge (`HOLWallLineBridge.cpp:23`, script `Script/Buildings/HOLWallLineBridge.as`) | `Player/HOLPlayerController.cpp:3224`, `:3326` (NativeIsWallType), `:3335` (NativeDragCells) | `HOLWallLine::IsWallType`, `HOLWallLine::DragCellsFromPoints(..., MaxLineCells())` (`HOLWallLineRules.as:199, 223, 147`) | Non. Arithmétique sur entiers et FVector, tableau local. `WallTypes()` rend un tableau local vide (l.193). Seule lecture externe : `FConsoleVariable CVar_WallMaxLineCells` (l.12, `.GetInt()`), une cvar globale, sans monde. Aucun WorldContext / GetWorld / acteur / sous-système / timer, aucune variable globale modifiée. | NON |
| UHOLGameMenuRulesBridge (`HOLGameMenuRulesBridge.cpp:23`, script `Script/UI/HOLGameMenuRulesBridge.as`) | `HOLPlayerController.cpp:5190` (Escape) ; `HeroesOfLibertyUI/HUD/HOLGameHUD.cpp:156, 161` ; `HUD/HOLGameScreen.cpp:697` ; `HUD/HOLScriptScreen.cpp:548` ; `Screens/HOLGameMenuScreen.cpp:102, 185, 256` | `HOLGameMenuRules::GetEscapeAction, CanSurrender, CanQuitToMenu, OffersReturnToLobby, QuitsToList, KeepsMatchMenuInput, ShowsStatsButton` (`HOLGameMenuRules.as:36-83`) ; `HOLEndBannerScript::GetEndBanner, ShouldOpenEndBanner` (`HOLEndBannerScript.as:30-46`) | Non. Fonctions de booléens, masques (`1 << MySlot`) et énumérations, entrées par paramètres. Aucun accès monde, aucun état conservé dans l'objet partagé : donc rien à partager ni à croiser entre deux contrôleurs (le masque et le slot viennent de l'appelant à chaque appel). (`HOLEndBannerScript::Make` et ses champs `bVisible/bReturn` sont une structure locale, pas appelée par le pont.) | NON |

Note : `HOLGameMenuRulesBridge.as` n'appelle pas `NativeOffersReturnToLobby` etc. depuis les lignes listées sauf ce qui est cité ; tout reste du même type pur.

## Journaux « requires WorldContext »

- `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Docs\Integrate_integ-49.log` : 0 occurrence (ni « WorldContext » seul).
- `Galadriel\Saved\Logs\*.log` (HeroesOfLiberty.log 16:49 du 22/11, 3 backups 22/11, build.log, backups 21/11 et 18/11) et `Haldir\Saved\Logs\*.log`, `Haldir\Docs\*.log` : 0 occurrence de « WorldContext ». Les 19 727 erreurs de L22-5 ne se retrouvent donc dans aucun journal présent.

## NON VÉRIFIÉ

- Aucune exécution : pas de test en jeu des appels via CDO (lecture seule). L'absence de risque repose sur la lecture du code des règles, pas sur une trace d'exécution.
- Je n'ai pas relu les fonctions annexes d'Angelscript appelées indirectement hors des fichiers cités (`HOLWallLine::Cells`, `Validate` ne sont pas appelées par le pont ; `Cells` l'est via `DragCells`, non lue en détail : non lue dans cette séance, mais elle ne figure pas dans la recherche de motifs WorldContext/GetWorld/Subsystem/Timer sur HOLWallLineRules.as, qui est vide).
