# SAURON-1119d : HUD H-7, file de production et sa tête (2026-11-19, étape 4)

Impact joueur : aucun changement visible attendu ; la file (5 boutons, barre de tête, note "Population pleine", "Clic : annuler") passe du C++ au script, même place (QueuePanel reste dans CommandColumn, sous le message court H-2), mêmes noms, mêmes textes.
NON COMPILÉ, NON LANCÉ, SANS CRÔCHET : 2 fichiers neufs dans `Documents\HeroesOfLiberty-Sauron\Script\UI\` ; `HOLQueuePanel.as` appelle des liaisons qui n'existent pas encore (en tête du fichier) : **ne pas compiler le module script avec ces fichiers avant le lot C++ ci-dessous**. BUILDS_DEMANDES: 0. Aucun fichier existant modifié.

## Fait
- `HOLQueuePanel.as` : `namespace HOLQueuePanelText` (NSLOCTEXT "HOLGameScreen" PopulationFull, CancelHint : mêmes clés que le LOCTEXT C++), `namespace HOLQueuePanel` (règles pures : `ItemName`, `ItemCount`=5, `ItemLabel`, `PanelShown`, `ItemsShown`, `ItemShown`, `Signature`, `SignatureChanged`, `HeadState`, `HeadPercent`, `ProgressShown`, `NoteShown`, `TwoDecimals`, `ItemState`, `ReportLine`, `ClickLine`, `ClickAllowed`), `class UHOLProductionQueuePanel : UObject` (`Build(Screen, UBorder QueuePanel)`, `Refresh()`, `ClickName(FName)`, `ClickItem(int)`, `Describe()`, 5 gestionnaires `QueueItem0..4Clicked`).
- `HOLQueuePanel_Test.as` : 8 tests purs : `Test_QueuePanelItemNamesAndCountAreTheCppOnes`, `Test_QueuePanelItemVisibilityFollowsTheQueue`, `Test_QueuePanelLabelIsTheResearchNameOrTheUnitName`, `Test_QueuePanelSignatureChangesWithTheQueue`, `Test_QueuePanelHeadStateIsUpdateHeadsRule`, `Test_QueuePanelBarAndNoteFollowTheHead`, `Test_QueuePanelReportLineIsWhatQuickCheckReads`, `Test_QueuePanelClickLineAndRefusals`.
- Règles C++ reprises (HOLGameScreen.cpp : RefreshQueue 2622-2650, UpdateHead 2652-2703, HandleQueueItemClicked 2828-2835, ClickWidget 3017-3029, Describe 3080-3092, mise en page 3662-3708, QueueItemName 333, constantes 249-250, NumQueueItems HOLGameScreen.h:150). Aucune règle de file n'existe en script hors tests (`HOLProductionQueue` n'apparaît que dans Script\Tests\*_Test.as) : rien à appeler ; la progression reste `HOLProduction::GetProgress` (HOLProductionReadLibrary.h:42, C++) lue côté C++ (le script n'a pas GetDisplayStep).
- Ce qui reste C++ : la file du bâtiment (`AHOLBuilding::GetProductionQueue`), l'annulation (`RequestCancelProduction`), le calcul de progression (GetDisplayStep, GetBuildSteps, IsPopulationBlocked), les noms (`HOLGameNames`), `CheckPopulationBlocked` et son journal "HUD: production held at 100 %" (H-2).

## (a) Liaison C++ à ÉCRIRE (un lot, un build) : membres de `UHOLScriptScreen` (HOLScriptScreen.h/.cpp), `UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")`
```cpp
/** The C++ screen's QueuePanel (built by BuildLayout, in CommandColumn, collapsed): the script fills it. */
UBorder* GetQueuePanel() const;
/** RefreshQueue(Own) + UpdateHead in one read. Returns false (all outs empty) when no own building's queue is shown (Own null). Types/Research/TechNames/UnitNames: one entry per queued item, in order;
    Research = UHOLTechnologySettings::FindTechnology(Type) != nullptr; TechNames = HOLGameNames::GetTechnologyName(Type, Research).ToString() (empty if not a research);
    UnitNames = HOLGameNames::GetUnitName(Type, FindUnitType(Type), LocalFactionKey()).ToString() (empty if a research).
    Head (queue[0]): bHeadStarted = Head.IsStarted(); HeadProgress = started ? HOLProduction::GetProgress(Head, UHOLProductionSubsystem::GetDisplayStep(World), BuildSteps) : 0;
    bBlockedAtFull = started && HOLCommandGrid::IsPopulationBlocked(Ps->GetPopulationUsed(), Ps->GetPopulationCap(), UnitType ? UnitType->Population : 1);
    bPopulationShort = !started && Ps && Ps->GetPopulationUsed() + Places > Ps->GetPopulationCap(). Ps = BoundPlayerState, building = the screen's BoundBuilding (the own building RefreshQueue got). */
bool GetQueueNow(TArray<FName>& OutTypes, TArray<bool>& OutResearch, TArray<FString>& OutTechNames, TArray<FString>& OutUnitNames,
                 bool& bOutHeadStarted, float& OutHeadProgress, bool& bOutBlockedAtFull, bool& bOutPopulationShort) const;
/** Stores the Describe() queue part: Describe prints "queue " + this (replaces the Queue string built from ShownQueueTypes/HeadState/HeadProgress). */
void SetQueueReport(const FString& Line);
/** HOLUIStyle::Accent (the progress bar's fill). */
UFUNCTION(BlueprintPure) static FLinearColor GetAccentColor();
```
Notes : `float&` côté C++ veut `float32&` côté script (le .as le fait). `RequestCancelProduction(int32)` d'AHOLPlayerController doit être UFUNCTION exposée au script (le .as l'appelle `Controller.RequestCancelProduction(Index)` comme le duel appelle RequestDuelAnswer) : sinon ajouter `HOLRpc::RequestCancelProduction(Controller, Index)` ou un membre `UHOLScriptScreen::CancelProduction(int)`. Plus : `UHOLScriptScreen::MakeBar(Name, Color)`, `SizedBar(Name, Bar, Height)`, `MakeText(Name, Style)` sont ceux déjà utilisés par HOLGameScreenScript.as (DuelBar) ; `UHOLQueueItem::SetLabel` UFUNCTION : déjà fait (1b30d3e).

## (b) Crochet dans `HOLGameScreenScript.as` (version Arwen)
1. Membre, près de `PlayerTable` : `UHOLProductionQueuePanel QueuePanel;` (NB : nom de membre différent de tout widget : `QueueView` si un `QueuePanel` existe déjà dans la classe).
2. `OnInitialized`, après `PlayerTable.Build(this, GetPlayersPanel());` : 
```
QueueView = Cast<UHOLProductionQueuePanel>(NewObject(this, UHOLProductionQueuePanel));
QueueView.Build(this, GetQueuePanel());
```
3. `Refresh()` du script (là où `TopBar.Refresh()` est appelé) : `QueueView.Refresh();` et, pour la barre fluide, aussi à chaque image (voir (g) cadence).
4. `ClickWidget` du script : avant le `return false` final, `if (QueueView != nullptr && QueueView.ClickName(Name)) return true;` (le C++ : `ClickWidget` fait aujourd'hui la boucle QueueItems, à retirer).
5. Report : `Describe()` C++ garde la ligne ; le script pousse sa partie par `SetQueueReport` à chaque Refresh.

## (c) C++ à retirer de `HOLGameScreen.cpp/.h`
- .cpp : `RefreshQueue` (2622-2650) et son appel 1268 ; `UpdateHead` (2652-2703) et ses appels 654 et 1269 ; `HandleQueueItemClicked` (2828-2835) ; dans BuildLayout 3667-3708 (QueueColumn, QueueRow, 5 ItemBox, QueueStatus, QueueStatusLine, QueueProgress, QueueProgressBox, QueueNote, QueueHint) : GARDER 3663-3666 (QueuePanel, slot, Fill, padding 6 bas) et 3791 (Collapsed) ; dans Bind/OnInitialized 415-423 (QueueItems.Add + OnClicked) ; ClickWidget 3017-3029 ; dans Describe 3080-3092 le calcul de `Queue` remplacé par la chaîne de SetQueueReport. `QueueItemName` (333), `QueueItemWidth` (249) si plus d'usage ; GARDER `QueueItemHeight` (utilisée par IdleWorkers, Menu, Stats, groupes, Wait, Alert, Players).
- .h : `NumQueueItems`, `QueueItems`, `QueueProgress`, `QueueNote`, `HandleQueueItemClicked`, `RefreshQueue`, `UpdateHead`, `EHeadState`, `HeadState`, `HeadProgress`, `ShownQueueTypes`. GARDER `QueuePanel` (UBorder), `bQueueShown` s'il sert à CheckPopulationBlocked / H-2 (grep avant : sinon à lire par GetQueueNow). `CheckPopulationBlocked` lit peut-être HeadState/HeadProgress : à vérifier avant de retirer (c'est le point le plus risqué du lot).
- Reste C++ : `Describe`, `ClickWidget` (autres panneaux), BindBuilding, `QueuePanel` créé vide.

## (d) Outils -> texte lu -> où le script l'écrit (grep de Tools\)
| Outil (fichier:ligne) | Lit | Où le script l'écrit |
|---|---|---|
| quick_check.ps1:1379 (building_queue, bloc de production) | `hol.HUD.Click QueueItem2` puis `waitlog cancelled Peasant` / `(2 born here)` | nom de widget `QueueItem2` : `UHOLProductionQueuePanel.ClickName` (via ClickWidget) ; les deux waitlog sont écrits par le contrôleur/production C++ (inchangé) |
| quick_check.ps1:1711 | `hol.HUD.Click QueueItem0` puis `waitlog cancelled Peasant` | idem |
| quick_check.ps1:3696 | regex `resources F 50 .* queue 3 ` dans la ligne HUD de `hol.HUD.Report` | `HOLQueuePanel::ReportLine` -> `SetQueueReport` -> Describe C++ "queue " + ligne (début "3 [...") |
| quick_check.ps1:3700 | `population 2/10, .* queue 0 ` | idem, "0 []" |
| quick_check.ps1:6376 | `population 1/10, .*queue 0 ` (hol.Prod.Report/HUD) | idem |
| Scenarios\alert_population.txt:40 et .look | `waitlog Population pleine]` (fil de messages H-2, pas la note de la file) ; le .look cite le journal "HUD: production held at 100 %" (CheckPopulationBlocked, H-2, hors de ce panneau) ; la note de file "Population pleine" est le texte du widget `QueueNote` | `QueueNote` : `HOLQueuePanelText::PopulationFull()` |
| (aucun outil ne lit) `HUD: queue item N clicked (cancel)` | grep `queue item` dans Tools\ : aucune occurrence | `UHOLGameMenuScreen::MenuLog(HOLQueuePanel::ClickLine(Index))` (niveau Display, mot pour mot) |
Le grep était `head_limit 60` : 8 occurrences au total, rien coupé. Noms de widgets lus par DescribeRects / hol.Mouse.Hover : conservés (QueuePanel, QueueColumn, QueueRow, QueueItem0..4, QueueStatus, QueueStatusLine, QueueProgress, QueueProgressBox, QueueNote, QueueHint). Les boîtes des items portaient un nom automatique en C++ (`ConstructWidget<USizeBox>` sans nom) : le script les nomme `QueueItemBox0..4` (nouveau nom, sans effet sur les outils, à noter si hud_rects les compte).

## (e) Suspects de compilation
- `UHOLScriptScreen::GetAccentColor()`, `Screen.MakeBar(...)` / `SizedBar(...)` / `MakeText(...)` / `MakePanel` appelés sur `Screen` de l'extérieur (HOLTopBar le fait pour MakeText, MakePanel ; MakeBar/SizedBar sont à confirmer publics).
- `TArray<bool>` en paramètre de sortie d'une UFUNCTION C++ ; `TArray<FName>&` ; `float32` pour `float&`.
- `Controller.RequestCancelProduction(Index)` : exposition non vérifiée.
- `f"{Hundredths / 100}."` : division entière sur int, ok ; `Math::RoundToInt` sur un double ; `bool ? FString : FString` (ternaire de chaînes) ; `Items[Index] != nullptr`, `GetFName()`, `IsVisible()` sur UHOLQueueItem (ClickWidget C++ les utilise ; en script à vérifier).
- Un `UFUNCTION()` à une ligne `void QueueItem0Clicked(UCommonButtonBase Button) { ClickItem(0); }` : forme acceptée normalement ; sinon étaler.
- `SetShown(QueueProgress, bool)` du C++ prend un état visible par défaut que je n'ai pas lu : le script met `Visible` pour la barre ; si le défaut est `HitTestInvisible`, la barre capte les clics (sans effet de jeu) ; mettre `HitTestInvisible` si doute.
- Barre : le C++ pose un style (fond 0.08/0.08/0.1, remplissage Accent, coins 2). `MakeBar(Name, Color)` ne donne que la couleur de remplissage (fond et coins à comparer sur la capture, leçon 3).

## (f) Deux rouges à poser UNE fois (L15-5)
1. `HOLQueuePanel::ItemsShown` : faire renvoyer `QueueCount` (sans le plafond 5) -> `Test_QueuePanelItemNamesAndCountAreTheCppOnes` rouge (9 -> 5 attendu).
2. `HOLQueuePanel::HeadState` : retirer le test `Progress >= 1.0` -> `Test_QueuePanelHeadStateIsUpdateHeadsRule` rouge (0.9 avec place bloquée doit rester InProgress). Remettre ensuite, vert.
Aucun rouge vu par moi (aucun build possible, tâche sans C++).

## (g) Scénario de preuve et coût par image
- `Tools\Scenarios\building_queue.txt` et `shift_queue.txt` : non lus dans cette séance (budget d'appels), donc je ne peux pas dire s'ils suffisent. Ce que quick_check 1379/1711 couvre déjà : clic sur QueueItem2 / QueueItem0 puis annulation et rapport `queue N`. Ce qu'ils ne voient pas (leçon 3) : le dessin (libellés des boutons, barre, note). Proposition : `ecran_hud_file` (.txt, .look, .json avec `"mapSize": "Small"`, `hol.Test.Sanity`, `waitkey` avant chaque étape, ressources par triche, Paysans immobiles) avec 5 captures de la zone QueuePanel, AVANT (C++) et APRÈS (script) : (1) 3 Paysans en file, tête à ~40 % ; (2) file de 5 pleine ; (3) une recherche en file (nom de recherche sur le bouton) ; (4) population pleine, tête tenue à 100 % (barre pleine et note) ; (5) clic sur QueueItem1, la file passe à 2.
- Cadence C++ : `UpdateHead` tourne CHAQUE IMAGE (appel HOLGameScreen.cpp:654, dans le tick), il ne pose le pourcentage que s'il change ; `RefreshQueue` tourne dans le rafraîchissement par événement (1268 : sélection changée, etc.), pas par image. Coût script : `GetQueueNow` remplit 4 tableaux par appel (5 entrées max) ; le script ne remet les libellés que si la signature change, le pourcentage que s'il diffère. Recommandation : appeler `QueueView.Refresh()` à chaque image uniquement quand `bQueueShown` (sinon à 4 Hz comme le héros : la barre avancerait par paliers de 250 ms, visible sur une unité courte), le tableau `Names` étant la seule allocation par appel (à passer en membre si la mesure le demande).

## Champs
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 8 (noms ci-dessus) ; LIGNES_CPP_SUPPRIMEES: 0 (recette) ; LIGNES_MIGREES_PAR_LOT: ~130 lignes C++ (RefreshQueue, UpdateHead, clic, mise en page) ; TYPES_A_EXPOSER: GetQueuePanel, GetQueueNow, SetQueueReport, GetAccentColor, RequestCancelProduction (si non exposée) ; TEST_ROUGE: non vu (pas de build) ; TESTS_TAUTOLOGIQUES: 0 ; PROCESSUS_DETACHES: aucun lancé.
Doute de design noté, aucune question ouverte : le texte "Population pleine" du panneau et le journal restent identiques ; le formatage "%.2f" du C++ est refait à la main (arrondi au centième : un cas limite 0.005 peut différer d'un chiffre).