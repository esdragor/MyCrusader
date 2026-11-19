# ELROND-1119f : HUD H-5a, banniere d'alerte majeure (liaison + bascule + scenario), 2026-11-19

Impact joueur : aucun changement visible voulu ; la banniere d'alerte majeure (forum tombe...) est dessinee par le script, memes noms (AlertPanel, AlertRow), memes lignes de journal, meme clic qui centre la camera.
Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, branche tiers. NON COMPILE, NON LANCE (aucun build, aucun quick_check). BUILDS_DEMANDES: 1 (lot C++ de la session principale).

## Commits (tiers, non pousses)
1. `10de003` LIAISON : `UHOLScriptScreen::GetAlertBanner` (etat C++ -> shown/serial/slot/opacite/son/texte/pieces/couleurs), `PlayAlertSound`, `FocusCameraOnForum`, evenement `AlertFrameTick(float)` ; `UHOLGameScreen::GetAlert()`, `GetAlertPresentCount()` (membre `AlertPresentCount`, ++ dans `PresentAlert`), `FocusCameraOnForum(Slot)` (camera seule). Aucun changement visible. Signatures du script `HOLAlertBanner.as` identiques a la recette : aucune correction.
2. `6cbf731` BASCULE : crochet dans `HOLGameScreenScript.as` (membre `AlertBanner`, `Build` apres `WaitBanner.Build`, surcharge `AlertFrameTick` -> `AlertBanner.Refresh()`, `ClickName` en tete de `ClickWidget`) ET `HOLGameScreen` ne construit ni ne dessine plus `AlertPanel`/`AlertRow` (jamais dessine deux fois). Plus `UHOLScriptScreen::BindAlertClick` + `HandleAlertMouseDown` (voir choix 3).
3. `60e0d06` scenario `Tools\Scenarios\ecran_hud_alerte.txt/.look/.json` (a jouer AVANT sur une branche sans le lot et APRES).
Commit 3 de la tache (RETRAIT) : RIEN a retirer de plus. Apres grep de `Source\` et `Script\` : plus aucune reference a `AlertPanel`/`AlertRow`/`HandleAlertMouseDown` (C++ screen) ; `FillMessageRow`, `FocusAlertSlot` et `UGameplayStatics` servent encore au fil. Le retrait est donc inclus dans la bascule (decision : un seul commit, sinon un commit intermediaire ne compile pas).

## Choix
1. Appel par image : comme le message court H-2 (`bMessageLive` -> `FrameTick`) : `NativeTick` de `UHOLScriptScreen` appelle `AlertFrameTick` seulement tant que `UHOLGameScreen::GetAlert().bShown`, et UNE fois de plus a la fin (`bAlertLive`) pour que le script cache la banniere. Pas d'appel par image sans alerte. Le fondu est maintenant par image (le C++ lisait `GetOpacity` dans `TickAlert`, deja par image aussi).
2. Machine a etats : reste C++ (`FHOLForumAlert Alert`, `ShowAlert`, `IsMatchOverForMe`, `TickAlert` reduit a `Advance`/`Expire`, `PresentAlert` reduit a `++AlertPresentCount`). **ECART NOTE, NON CORRIGE** : `UHOLForumAlertHolder::ShowEvent` passe `bSeesForum=false` a `GetMessage` (HOLForumMessagesReadLibrary.cpp:160) alors que le C++ ecran passe `GameState->DoesViewerSeeForum(...)` ; le script n'utilise PAS le holder pour piloter (seuls les tests le font, avec des evenements ou l'ecart ne joue pas).
3. Clic souris : la liaison dynamique `OnMouseButtonDownEvent` d'un `UBorder` n'a jamais ete faite depuis le script : la RECEPTION reste C++ : `UHOLScriptScreen::BindAlertClick(UBorder*)` lie `HandleAlertMouseDown` (bouton gauche seulement) qui appelle `ClickWidget(AlertPanel)` du script (`ClickName`, faux tant qu'aucune banniere). Le comportement visible est conserve ; supprime les suspects 1 et 2 de la recette (FEventReply et GetEffectingButton en script).
4. Ligne de journal du clic : ecrite UNE fois par clic de banniere, par le script (`MenuLog(ClickLogLine)`, `FocusShown`), puis `FocusCameraOnForum` (camera seule, sans ligne). **ECART VOLONTAIRE avec la consigne** : le C++ `FocusAlertSlot` GARDE sa ligne `HUD: alert of slot %d clicked ...`, car les clics sur les lignes du fil (`HandleMessageLineMouseDown`, fil encore C++) l'utilisent ; chaque clic donne donc une seule ligne. A retirer du C++ au lot du fil (alors `FocusAlertSlot` disparait).
5. Ligne `Alert: ...` : ecrite par `PresentLogLine` + `MenuLog` (niveau Display) au premier `Refresh` apres chaque `PresentAlert` (au plus un frame apres, mot pour mot comme le C++).
6. Tests : retires `Test_AlertBannerOnlyAShownBannerTakesALeftClick` (fonctions d'identite `MouseClickAccepted`/`NamedClickAccepted`, retirees du .as ; la regle bouton gauche est maintenant en C++, la regle « refuse sans banniere » est `ClickName` avec `bVisible`) et `Test_AlertBannerPlaceAndStyleAreTheCppOnes` (recopie de constantes de mise en page = tautologique ; la mise en page est jugee par la capture du scenario). Ajoute `Test_AlertBannerLineFollowsTheCppSoundRule` (vraie machine C++ via `UHOLForumAlertHolder` : `HasSound` du 1er bandeau, d'un bandeau 25 s plus tard, et la ligne qui en decoule). Reste 4 tests (TESTS_SCRIPT_AJOUTES: 1 ; retires: 2 ; TESTS_CPP_RETIRES: 0). TESTS_TAUTOLOGIQUES: 0 attendu ; `LogLinesAreTheCppOnes` teste des chaines litterales de la regle `PresentLogLine` (mot pour mot de la ligne lue par quick_check), `NamesArePiecesWithASlot` traverse la vraie regle C++ `SplitNames`.

## Table outils -> ligne lue -> ou ecrite
| Outil | Lu | Ecrit par |
|---|---|---|
| Tests\quick_check.ps1:3240 ; Scenarios\forum_messages.txt:35,42,48 (`waitlog Alert:`) | `Alert: <texte> (sound <path>[, missing])` / `(no sound, its burst's played)` | script `Refresh` -> `PresentLogLine` -> `MenuLog` (`LogHOLUI: Display: ...`) |
| quick_check.ps1:3286, 3293, 3344 | absence de `Alert: ` | idem : rien sans banniere |
| quick_check.ps1:3242, 3285, 3292, 3343, 6942 ; tribute_i18n.txt | `HUD alert:` / `HUD messages:` | C++ `DescribeAlert()` / `DescribeMessages()` INCHANGES (l'etat reste C++) |
| quick_check.ps1:793 | `hol.HUD.Click AlertPanel` puis `Camera above the forum of slot 0` | `ClickWidget` C++ -> `ScriptScreen->ClickWidget` -> `ClickName(n"AlertPanel")` -> `FocusCameraOnForum` ; la ligne `Camera above...` = camera C++, inchangee |
| (aucun) | `HUD: alert of slot` | script (clic banniere) ; C++ `FocusAlertSlot` (clic du fil) |

## Suspects de compilation
- `HOLScriptScreen.h` : `FEventReply`/`FGeometry`/`FPointerEvent` dans un UFUNCTION : include peut manquer (ajouter `Input/Reply.h`/`Components/Widget.h` si besoin) ; `#include "Match/HOLForumMessagesReadLibrary.h"` ajoute pour `TArray<FHOLForumPieceValue>` en param UFUNCTION.
- `HOLScriptScreen.cpp` : `GetGameState<AHOLGameState>()` sur `const UWorld*` ; `FHOLForumAlert::GetOpacity` renvoie `float` ; `HOLForumMessages::FPiece::Text` en `FString` (verifie).
- Script : `Screen.GetAlertBanner(..., float32 Opacity, ...)` ; `Screen.ConstructWidget(UCommonTextBlock, NAME_None)` nom vide ; `AlertPanel.SetBrushColor(FLinearColor(1,1,1,0.85))` MULTIPLIE peut-etre l'alpha deja pose par `MakePanel` (le C++ posait `HUDPanel.CopyWithNewOpacity(0.85)`) : a juger a la capture ; `Piece.SetColorAndOpacity(FLinearColor)`.
- `AlertFrameTick` appele avant que le script ait fini `OnInitialized` : garde `AlertBanner != nullptr`.
- Piege : un seul `AlertPresentCount` ; une banniere remplacee a 1,5 s change le serial (redessinee sans image masquee).

## Commandes de validation (session principale)
1. `Tools\build.ps1 -LiveCoding=0 -Wait` (worktree Elrond) ; compiler le module script.
2. `quick_check -Parts script` puis la partie HUD (`mark victory forum alert`, `forum_messages`).
3. Rouges a poser une fois (recette ELROND-1119e (f)) : `PresentLogLine(Text,false,false)` attendu faussé en `"(no sound)"` ; `Decide` `Serial != ShownSerial` -> `false` : `FollowsTheCppStateMachine` doit passer rouge.
4. `play.ps1 ecran_hud_alerte -Hidden -Root <worktree>` AVANT (sur main/branche sans le lot) et APRES ; comparer la zone haut-centre a l'oeil (le libelle visible, lecon 3).

## Champs
BUILDS_DEMANDES: 1 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 1 (`Test_AlertBannerLineFollowsTheCppSoundRule`), 2 retires ; LIGNES_CPP_SUPPRIMEES: ~72 net dans `HOLGameScreen.cpp` (PresentAlert/TickAlert widgets, `HandleAlertMouseDown`, bloc `BuildLayout`, branche `ClickWidget`, constante `AlertPanelName`) + `HOLGameScreen.h` (2 membres, 1 UFUNCTION) ; ~120 lignes C++ ajoutees (liaison) ; LIGNES_MIGREES_PAR_LOT: ~110 (banniere) ; TYPES_A_EXPOSER: aucun ; TEST_ROUGE: non vu (rien compile) ; PROCESSUS_DETACHES: aucun ; NON VERIFIE : tout (compilation, tests, rendu).
Reste : fil de messages (`HOLMessageFeed.as`, lot suivant), qui retirera la ligne `HUD: alert of slot` de `FocusAlertSlot`.
Non touche : EtapeDefini.md, Auto_Journey_File.md, Auto_Journey_Retours.md, Placeholders.txt (le son d'alerte reste provisoire `FHOLForumAlert::SoundPath`, deja liste). Une ligne ajoutee a AngelscriptInProgress.md.
