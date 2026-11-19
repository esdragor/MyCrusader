# ELROND-1119e : HUD H-5 (1re moitie), banniere d'alerte majeure en script (2026-11-19, etape 4)

Impact joueur : aucun changement visible attendu ; la banniere d'alerte majeure (forum tombe, tenue, allie) passe du C++ au script, memes places, memes noms (AlertPanel, AlertRow), memes lignes de journal, meme clic qui centre la camera.
NON COMPILE, NON LANCE, SANS CROCHET : 2 fichiers neufs dans `Documents\HeroesOfLiberty-Elrond\Script\UI\` ; `HOLAlertBanner.as` appelle 3 liaisons qui n'existent pas encore (en-tete du fichier) : **ne pas compiler le module script avec lui avant le lot C++ ci-dessous**. `HOLGameScreenScript.as`, `HOLGameScreen.cpp/.h` intacts.
**LE FIL DE MESSAGES (HOLMessageFeed.as) N'EST PAS FAIT** : lot suivant. Seule la banniere est livree.

## Choix de conception (a confirmer par la session principale)
- La MACHINE A ETATS de l'alerte (Show / Advance / Expire / GetOpacity : 4 s, fondu 0,5 s, 1,5 s minimum, 2 en attente, un son par rafale) RESTE C++ : `FHOLForumAlert` est une regle a une source (D836), deja exposee en script par `UHOLForumAlertHolder` pour ses tests, et `DescribeAlert()` (ligne `HUD alert:` lue par quick_check:3242) la lit. Le script est la VUE : il lit l'etat chaque frame et dessine. Raison : `UHOLForumAlertHolder::ShowEvent` passe `bSeesForum=false` a `GetMessage` (HOLForumMessagesReadLibrary.cpp:160) alors que le C++ passe `GameState->DoesViewerSeeForum(...)` (HandleForumEvent) : le holder n'est PAS equivalent, une 2e copie de l'etat divergerait.
- Les evenements du forum (`OnForumEvent`, `HandleForumEvent`) restent recus par le C++ (delegues du `BoundTableState`) ; il appelle toujours `MessageThread.Add` (le fil reste C++ jusqu'au lot suivant) et `ShowAlert` -> `Alert.Show(...)`. Le script ne les recoit pas : il lit l'etat resultant (`GetAlertBanner`).
- Conteneur : `AlertPanel` est construit par le script dans la `Frame` du HUD (comme WaitPanel), pas dans la RightColumn (le panneau d'alerte n'est pas dans la colonne droite ; seul le fil l'est).

## Fait
- `Script\UI\HOLAlertBanner.as` : `namespace HOLAlertBanner` (regles pures : `Decide` (None/Present/Hide/Fade selon l'etat C++, le numero de banniere dessinee et la visibilite), `PresentLogLine`, `ClickLogLine`, `MouseClickAccepted`, `NamedClickAccepted`, `PieceIsName`, `SoundPath`, mise en page `Top/Margin/PanelOpacity/PaddingX/PaddingY` en fonctions) et `class UHOLAlertBannerPanel : UObject` (`Build(Screen, Frame)`, `Refresh()` a chaque frame, `FillRow`, `HandleAlertMouseDown`, `ClickName(FName)`, `FocusShown`).
- `Script\UI\HOLAlertBanner_Test.as` : 5 tests. `Test_AlertBannerLogLinesAreTheCppOnes`, `Test_AlertBannerOnlyAShownBannerTakesALeftClick`, `Test_AlertBannerNamesAreThePiecesWithASlot` (traverse la vraie regle C++ `SplitNames`), `Test_AlertBannerFollowsTheCppStateMachine` (pilote la vraie machine C++ via `UHOLForumAlertHolder` : banniere montree, attente 40 ms, remplacement a 1,5 s sans image masquee, masquage a 4 s), `Test_AlertBannerPlaceAndStyleAreTheCppOnes` (valeurs de mise en page recopiees du C++ ; tautologique par nature : garde-fou de recopie, pas de logique).
- Regles de `HOLAlertRules.as` : NON dupliquees, NON appelees ici (ce sont les alertes MINEURES population / attaque qui ajoutent des lignes au fil, donc le lot du fil).

## (a) Liaison C++ a ECRIRE (un lot, un build) : membres de `UHOLScriptScreen` (`Source\HeroesOfLibertyUI\HUD\HOLScriptScreen.h/.cpp`, `UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")`)
```cpp
/** The shown banner, read from UHOLGameScreen::Alert (FHOLForumAlert): bOutShown = Alert.bShown; OutSerial = AlertPresentCount (new int32 member of UHOLGameScreen, ++ in PresentAlert, never reset); OutSlot = Alert.Slot;
 *  OutOpacity = Alert.GetOpacity(World->GetRealTimeSeconds()); bOutSound = Alert.bSound; OutText = Alert.Message.Text.ToString();
 *  OutPieces = HOLForumMessages::SplitNames(Alert.Message, ViewerSlot, GameState->GetLaunchConfig()) as FHOLForumPieceValue (Text, Slot);
 *  OutColors = parallel to OutPieces: FLinearColor(HOLPlayerColors::GetColor(Launch.Slots[Part.Slot].ColorIndex)) for a name, white otherwise.
 *  ViewerSlot = BoundPlayerState ? GetPlayerIndex() : INDEX_NONE. False (all neutral) without world or screen. */
bool GetAlertBanner(bool& bOutShown, int32& OutSerial, int32& OutSlot, float32& OutOpacity, bool& bOutSound, FString& OutText,
                    TArray<FHOLForumPieceValue>& OutPieces, TArray<FLinearColor>& OutColors) const;
/** LoadObject<USoundBase>(FHOLForumAlert::SoundPath) + UGameplayStatics::PlaySound2D; true if the asset loaded. */
bool PlayAlertSound();
/** AHOLPlayerController::FocusCameraOnForum(Slot) of the owning controller (FocusAlertSlot minus its journal line, which the script writes). */
void FocusCameraOnForum(int32 Slot);
```
Le script appelle `Screen.GetAlertBanner(...)` avec des `float32` pour `OutOpacity` (lecon 9 : un `float&` C++ veut un `float32`). `FHOLForumPieceValue` est deja visible (HOLForumMessagesReadLibrary.h:76). Un `Refresh()` par frame demande un point d'appel par frame (voir (b)).

## (b) Crochet exact dans `HOLGameScreenScript.as` (version Arwen a jour)
- Membre apres `UHOLWaitBannerPanel WaitBanner;` (ligne 127) : `UHOLAlertBannerPanel AlertBanner;`
- `OnInitialized`, apres `WaitBanner.Build(this, Frame);` (ligne 259) :
  `AlertBanner = Cast<UHOLAlertBannerPanel>(NewObject(this, UHOLAlertBannerPanel)); AlertBanner.Build(this, Frame);`
  (ordre : apres `WaitBanner`, avant `PlayerTable`, pour que la banniere recouvre comme le C++ ; l'AlertPanel C++ etait ajoute avant PlayersPanel et apres AbsencePanel)
- Le fondu dure 0,5 s : `Refresh()` du HUD (4 fois par seconde, ligne 1369) est trop lent. Ajouter une evenement par image : `UFUNCTION(BlueprintEvent) void ScriptTick(float DeltaTime)` dans `UHOLScriptScreen.h`, appele depuis `UHOLScriptScreen::NativeTick` (C++ existant), surcharge script `UFUNCTION(BlueprintOverride) void ScriptTick(float DeltaTime) { if (AlertBanner != nullptr) AlertBanner.Refresh(); }`. (Aucun `Tick` n'existe aujourd'hui dans HOLGameScreenScript.as : verifie par grep.) Si le lot refuse l'evenement : mettre `AlertBanner.Refresh();` dans `Refresh()` apres `WaitBanner.Refresh()` ; le fondu sera alors a 4 pas par seconde (degrade visible, non conforme).
- `ClickWidget(FName Name)` du script : ajouter `if (AlertBanner != nullptr && AlertBanner.ClickName(Name)) return true;` en tete. Le C++ `UHOLGameScreen::ClickWidget` appelle deja `ScriptScreen->ClickWidget(Name)` en premier (HOLGameScreen.cpp:2832).

## (c) C++ a retirer / ce qui RESTE C++
Retirer de `HOLGameScreen.cpp/.h` : membres `AlertPanel`, `AlertRow` ; la construction `BuildLayout` (bloc « V4.3: the major alert's banner ... AlertRow », ~3165-3178) ; `HandleAlertMouseDown` (UFUNCTION liee) ; dans `PresentAlert` tout ce qui touche un widget (`FillMessageRow(... *AlertRow ...)`, `SetRenderOpacity`, `SetShown(AlertPanel...)`, `LoadObject`/`PlaySound2D` et les deux `UE_LOG "Alert:"`) ; dans `TickAlert` `SetShown(AlertPanel,false)` et `SetRenderOpacity` ; la branche `AlertPanel` de `ClickWidget` (2836-2844) ; les gardes `!AlertPanel || !AlertRow` de `ShowAlert` et `TickAlert` (sinon la machine a etats ne tourne plus).
RESTE C++ : `Alert` (FHOLForumAlert) ; `ShowAlert`, `IsMatchOverForMe`, `PresentAlert` reduit a `++AlertPresentCount`, `TickAlert` reduit a `Advance` / `Expire` ; `HandleForumEvent` et les autres `Handle*Message` ; `DescribeAlert()` et `DescribeMessages()` (lignes `HUD alert:` / `HUD messages:` de HOLUIHUD.cpp:68-70, lues par quick_check:3242-3245/3285/3292/3343) ; `FocusAlertSlot` reduit a l'appel (la ligne `HUD: alert of slot %d clicked` est ecrite par le script ; les clics du fil (HandleMessageLineMouseDown) gardent le C++ `FocusAlertSlot` ET sa ligne : NE PAS retirer la ligne du C++ tant que le fil est C++, sinon deux lignes sur le clic de la banniere : garder la ligne dans `FocusAlertSlot` et faire appeler au script `FocusCameraOnForum` SANS ecrire la ligne, ou inversement. DOUTE a trancher : voir (e)).
`FillMessageRow` (HOLGameScreen.cpp:~830) reste pour le fil.

## (d) Outils -> texte lu / nom de widget -> ou le script l'ecrit (grep de `Tools\`)
| Outil (fichier:ligne) | Lu | Ecrit par |
|---|---|---|
| Tests\quick_check.ps1:3240 | `Alert: <texte> (sound /Engine/...)` | `UHOLAlertBannerPanel.Refresh` -> `PresentLogLine` -> `MenuLog` (niveau Display compris) |
| quick_check.ps1:3286, 3293, 3344 | absence de `Alert: ` (`-SimpleMatch`) | idem : pas de ligne si pas de banniere |
| quick_check.ps1:3242, 3285, 3292, 3343, 6942 | `HUD alert: ...` | C++ `DescribeAlert()` inchange (l'etat reste C++) |
| quick_check.ps1:793 | `hol.HUD.Click AlertPanel` puis `Camera above the forum of slot 0` | `ClickName(n"AlertPanel")` -> `FocusCameraOnForum` ; la ligne `Camera above...` est du C++ de la camera, inchangee |
| Scenarios\forum_messages.txt:35, 42, 48 | `waitlog Alert:` | idem 3240 |
| Scenarios\tribute_i18n.txt:9,16,23 | `HUD messages:` | C++ `DescribeMessages()` inchange |
| hud_rects* / DescribeRects | aucun `AlertPanel` trouve par grep | le widget garde le nom `AlertPanel` (cree par `MakePanel(n"AlertPanel")`) |
Aucun outil ne lit `HUD: alert of slot` (grep vide) ; aucun ne lit `AlertRow`.

## (e) Suspects de compilation / de comportement
1. `AlertPanel.OnMouseButtonDownEvent.BindUFunction(this, n"HandleAlertMouseDown")` : delegue dynamique `FOnPointerEvent` sur un `UBorder`, fonction `UFUNCTION() FEventReply (FGeometry, const FPointerEvent&)` d'un UObject script : jamais fait dans le projet. Repli : un `FEventReply` retourne par `WidgetBlueprint::Handled()/Unhandled()` (a verifier), ou faire passer le clic par un `UHOLQueueItem`/bouton transparent, ou garder `HandleAlertMouseDown` en C++ appelant un `ScriptScreen->ClickWidget(n"AlertPanel")`.
2. `MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton` : liaison `FPointerEvent` a verifier (sinon `MouseEvent.GetEffectingButton().KeyName`).
3. `Screen.ConstructWidget(UCommonTextBlock, NAME_None)` : nom vide accepte ? (le C++ construit sans nom).
4. `Piece.SetColorAndOpacity(FLinearColor)` : HOLPlayerTable.as:373 le fait deja (implicite FSlateColor).
5. `UHOLTextStyleHUDHeading` passe a `SetStyle` : `MakeText(name, style)` le fait ; ici `SetStyle(UHOLTextStyleHUDHeading)` direct (la classe est un `TSubclassOf<UCommonTextStyle>` : OK en script d'apres MakeText).
6. DOUTE (journal du clic) : deux voies appellent `FocusAlertSlot` (clic banniere, clic ligne du fil) ; la ligne `HUD: alert of slot ...` est dans le C++ aujourd'hui. Recommande : la laisser dans le C++ `FocusAlertSlot` tant que le fil est C++, et faire que `FocusCameraOnForum` de la liaison APPELLE `FocusAlertSlot` (donc ecrit la ligne) ; alors le script N'ECRIT PAS `ClickLogLine` (retirer l'appel `MenuLog` dans `FocusShown`) et `ClickLogLine` ne sert qu'aux tests/au lot du fil. A decider au lot C++ ; non tranche ici.
7. Le `Serial` : `PresentAlert` n'est appele que sur `Show`/`Advance` vrai ; c'est le seul chemin qui change la banniere montree. Une banniere identique mais de numero different (deux chutes de suite) est bien redessinee.
8. Trou du test : un test de regle ne voit pas la vue (lecon 3). Le libelle des pieces et le titre « Heading » ne sont verifies que par la capture.

## (f) Deux rouges a poser une fois (apres compilation, dans le worktree, a remettre)
1. Dans `HOLAlertBanner_Test.as` : `PresentLogLine(Text, false, false)` attendu `"... (no sound, its burst's played)"` -> le fausser en `"... (no sound)"` : le test `LogLinesAreTheCppOnes` doit passer rouge (preuve que la ligne lue par quick_check:3286 est testee mot pour mot) ; le remettre.
2. Dans `HOLAlertBanner.as` `Decide` : remplacer `Serial != ShownSerial` par `false` (la banniere remplacee sans image masquee n'est plus redessinee) : `Test_AlertBannerFollowsTheCppStateMachine` doit passer rouge a l'etape « replaced in place » ; le remettre.
Ensuite vert : `quick_check -Parts script` (ou la commande de tests script du worktree, apres le lot C++).

## (g) Scenario de preuve
`siege_alert.txt` ne suffit PAS : il verifie `alerts N` de la mini-carte (alertes d'attaque), pas la banniere. `forum_messages.txt` (waitlog `Alert:`, clic sur le bandeau, 4 s) prouve le journal, le clic et la camera, mais ne montre pas le bandeau en image avant/apres : le defaut vu seulement a la capture (lecon 3) n'y serait pas vu. A ajouter : `ecran_hud_alerte` (.txt, .look, .json), modele `ecran_hud_joueurs.json`/`ecran_hud_hautdroite` :
- `.json` : `"mapSize": "Small"` ; `.txt` : `hol.Test.Sanity` en tete ; ressources fixees par triche, Paysans immobiles (lecon 14) ;
- `hol.Victory.FallForum 0` ; `waitlog Alert:` ; `waitkey` ; capture zone haut-centre (banniere pleine) ; `wait 4.6` ; capture (banniere disparue) ; `hol.Victory.FallForum 2` (forum allie) ; `waitkey` ; `hol.HUD.Click AlertPanel` ; `waitlog Camera above the forum of slot 2` ; capture ;
- `.look` : un objet par PNG (« la banniere sombre sous la barre du haut avec "Votre forum est tombe — 1 vie", le nom en couleur »), avant (C++) et apres (script) comparees a l'oeil sur la zone du panneau.

## Champs d'agent
BUILDS_DEMANDES: 0 (le lot C++ de (a) est a faire, un build) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 5 (voir plus haut) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: ~110 (banniere, sans le fil) ; TYPES_A_EXPOSER: aucun type, 3 fonctions de `UHOLScriptScreen` + `ScriptTick` ; TEST_ROUGE: non vu (rien compile) ; TESTS_TAUTOLOGIQUES: 1 (PlaceAndStyle, recopie de constantes) ; PROCESSUS_DETACHES: aucun lance.
