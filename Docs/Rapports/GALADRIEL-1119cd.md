# GALADRIEL-1119cd : lot L3a (retrait des natifs de HOLCommandGrid.cpp/.h)

NON COMPILÉ. Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`, branche `grid-l3a`, base ae64d0e, commit fae3c2c (anglais, sans Co-Authored-By, pas de push, pas de merge). Aucun build, aucun quick_check, aucun éditeur. BUILDS_DEMANDES: 0 (le build groupé L4 est celui de la session principale).

## Symboles `*Native` SUPPRIMÉS (19, liste exacte, .cpp et .h)
`HOLCommandGrid::` : IsAgeReachedNative, IsAgeOpenNative, GetAgeBuildingNative, PressKeyNative, ClickAgeNative, GetChosenAgeAfterNative, CheckProductionNative, CheckBuildNative, DescribeNative, BlocksNative, GetCycleRankNative, IsSelectedByCategoryNative, IsRepeatPressNative, IsAloneInGroupNative, GetCommandPriorityNative, NextInCycleNative, SplitSubgroupsNative, MergeCommandsNative, PickEvenlyNative.

## Natifs GARDÉS (2) et pourquoi
`GetCommandNative`, `GetOrderNative` (déclarés `HOLCommandGrid.h:364, 369`, corps dans `HOLCommandGrid.cpp`). Les wrappers `GetCommand` / `GetOrder` gardent `Ask* ? Answer : *Native`. Raison = vérification 1 ci-dessous : deux tests C++ les appellent SANS monde.
ATTENTION pour la fusion avec L3b : la bibliothèque de lecture (`HOLCommandGridReadLibrary.cpp:172, 177`) peut donc continuer à citer ces DEUX-là sans casser le build ; elle NE DOIT PLUS citer aucun des 19 autres. Citations actuelles à retirer côté L3b (HEAD ae64d0e, `HOLCommandGridReadLibrary.cpp`) : 181 (IsAgeReached), 185, 190, 195, 200, 237, 243, 250, 269, 274, 277, 284, 300, 310, 313, 319, 333, 349, 363, 388 (IsAgeOpen). Aucun autre fichier de `Source\` ne cite un `*Native` supprimé (grep `Native\b` sur HOLCommandGrid.h, Board.*, PlayerController.*, UnitView.h, AttackCursor.h, HUD\ : seulement RL.cpp ci-dessus ; le commentaire `HOLCommandGridBoard.h:15-16` est périmé, voir anomalies).

## Comportement sans plateau
Chaque pont : `if (Ask*(...)) return Answer; LogNoBoardOnce(TEXT("<Fonction>")); return <défaut neutre>;`. Défauts : GetAgeBuilding = commande vide ; PressKey / ClickAge = résultat vide avec `ChosenAge` conservé ; GetChosenAgeAfter = INDEX_NONE ; IsAgeReached / IsAgeOpen / Blocks / IsSelectedByCategory / IsRepeatPress / IsAloneInGroup = false ; CheckProduction / CheckBuild = feedback par défaut (None : passe) ; Describe = "none" ; GetCycleRank = 0 ; NextInCycle / PickEvenly = INDEX_NONE ; SplitSubgroups / GetCommandPriority / MergeCommands = tableau vide.
Ligne de journal exacte (niveau Error, catégorie statique `LogHOLCommandGridBridge` nouvelle dans `HOLCommandGrid.cpp`) :
`CommandGrid: <Fonction> refused, no script grid board`
Une fois par fonction et par monde (clé = `FObjectKey` du premier monde de `GEngine->GetWorldContexts()`, `FCriticalSection`), même patron que `LogBoardAbsent` de `HOLPlayerOrders.cpp`.
`GetAgePanelOpacity` (ne devait pas être touchée) cite `IsAgeOpenNative` : elle appelle maintenant `IsAgeOpen` (le pont) : même résultat avec plateau ; sans plateau = page fermée (opacité 0.55) + une ligne `IsAgeOpen refused`.

## Lignes C++
`git diff --numstat` : `.cpp` +136 / -294, `.h` +2 / -26. Net = -182 (le plan estimait -540 ; l'écart = GetCommandNative et GetOrderNative gardés (~85 l.) et le helper de journal (~35 l.)).

## Vérification 1 : appelants dans un monde possiblement SANS plateau
`FindAny` (`HOLCommandGridBoard.cpp:32-53`) ne trouve le plateau que par `GEngine->GetWorldContexts()` + `GetSubsystemArrayCopy` ; `DoesSupportWorldType` (`:282`) = Game ou PIE seulement ; hors thread de jeu : nullptr. Un appel sans `FWorldContext` Game vivant n'a donc AUJOURD'HUI que le repli natif.
- SANS monde (garder le natif) : `Tests\HOLRegicideTests.cpp:292-299` (FHOLRegicideCommandsTest, 286-331, aucun CreateWorld : `GetOrder`), `Tests\HOLVictoryTests.cpp:1201-1204` (FHOLVictoryForumLookTest, 1158-1212, aucun CreateWorld : `GetCommand`). C'est la raison des 2 natifs gardés. Ces extraits sont ceux que L2 doit retirer ; après L2, `GetCommandNative` / `GetOrderNative` + l'`AskGet*` de repli peuvent partir (suite de ~85 l.).
- AVEC monde Game + contexte (`CreateWorld(Game)` + `CreateNewWorldContext`) : `HOLDamnedWarriorTests.cpp:233, 235` (monde 128-299), `HOLStartingHeroTests.cpp:655-661` (monde 625-689) et `:726` (monde 709-782), `HeroesOfLibertyUI\Tests\HOLSpecializationGridTests.cpp:54-115` (monde 41-118, 12 appels `GetCommand`, pas 20 : les 20 incluent `ViewNow` etc.). Ils passent SI le plateau script est présent dans ce monde de test (sous-système créé à `CreateWorld`) : NON VÉRIFIÉ (le script du plateau est-il chargé dans la session d'automatisation ? aujourd'hui un plateau absent y était masqué par le repli natif ; désormais il rendrait une commande vide). Risque nommé : ces 4 tests deviennent rouges si le plateau n'existe pas en automatisation.
- `HeroesOfLibertyUI\HUD\HOLSelectionChipsReadLibrary.cpp:153` appelle `IsAloneInGroup` (étiquettes des puces) : monde de jeu en pratique ; tests qui l'appelleraient sans monde : NON recherchés.
- `HOLPlayerController.cpp` (32 appels) et `HOLGameScreen.cpp` : monde de jeu, signatures inchangées.

## Vérification 2 : ce qui reste par décision du plan
Non touchés (corps identique) : `SetSpecialization`, `GetSpecializationCell`, `GetOpenSpecializationTier`, `CheckResearch`, `GetMouseCursor`, `GetAgePanelOpacity` (une ligne adaptée, voir plus haut), constantes de taille, `IsBuildGrid`, `FindMissingResource`, `IsPopulationBlocked`, `GetCellFace`, `GetCategoryRow`, `GetTrade*`, `GetRageCell`, `GetHeroAbilityCell`, `GetReportName`. `HOLCommandGridBoard.cpp` : inchangé (aucun défaut à toucher).

## Preuves à rejouer (L4) et falsification
1. Build groupé L3a + L3b + L2. Compile = `HOLCommandGridReadLibrary.cpp` ne cite plus aucun des 19 symboles (sinon : erreur d'édition de liens / de nom).
2. `tier_grid_client_run.ps1 -Root <worktree>` : cellule Peasant « enabled », journal client ET serveur SANS « CommandGrid: ... refused, no script grid board ». Rouge à voir : bouchon `return FHOLGridCommandView();` dans `GetCommandAnswer` (`HOLCommandGridBoard.as:13`).
3. Falsification du repli : un test C++ de monde sans plateau (ou le test Regicide Commands sans monde, déjà là pour GetOrder/GetCommand : doit rester VERT grâce aux natifs gardés) ; pour un des 19 : appeler p. ex. `HOLCommandGrid::IsAloneInGroup(MainCentre)` sans monde doit rendre false et écrire `CommandGrid: IsAloneInGroup refused, no script grid board` UNE fois.
4. `quick_check -Parts tests` (partie C++ : Regicide, Victory, DamnedWarrior, StartingHero, Specialization) : même total qu'avant L3a.
5. Échantillon de scénarios `hero`, `market`, `build hud ages`.

## NON VÉRIFIÉ (rien n'est compilé)
- Compilation : `DEFINE_LOG_CATEGORY_STATIC(LogHOLCommandGridBridge...)` (risque : nom déjà déclaré dans un en-tête inclus) ; `FObjectKey(const UWorld*)` accepte nullptr ; includes ajoutés (Engine/Engine.h, Engine/World.h, Misc/ScopeLock.h, UObject/ObjectKey.h) ; `FHOLGridKeyResult` / `FHOLGridCommand` / `FHOLCommandFeedback` ont un défaut « vide » sur construction (supposé : pas lu) ; `FHOLGridKeyResult::ChosenAge` existe (lu dans l'ancien corps).
- Le fichier `.cpp` a été réécrit en LF par l'outil (git le normalise en CRLF : « LF will be replaced by CRLF ») ; le `numstat` est normal, mais lancer `eol_check.ps1 -Fix` si un Edit multi-ligne est refusé.
- Présence du plateau script dans les mondes de tests d'automatisation (voir vérification 1).
- Les commentaires cités par le plan (HOLAttackCursor.h:13, HOLUnitView.h:44, HOLCommandGrid.h:41, 111, HOLPlayerController.h:179-309) NON mis à jour (hors budget d'appels) ; seuls les commentaires des déclarations de GetCommandNative / GetOrderNative l'ont été.

## Anomalies
- `HOLCommandGridBoard.h:15-16` dit encore « keep their C++ body (GetCommandNative / GetOrderNative ...) removed in a later lot » : fichier hors de ma liste, non touché.
- Le plan comptait 21 `*Native` à retirer ; 2 sont gardés pour des tests sans monde (voir ci-dessus), donc 19 retirés.
- L3a retire le gros du repli mais la règle « pas de plateau = grille vide » est maintenant la règle pour 19 fonctions : un client ou un serveur sans plateau script n'a plus de commandes de grille (risque joueur du plan : élevé, couvert par L4).

APPELS_DECLARES: 23 (arrêt à 25) ; HEURE_DEBUT/HEURE_FIN: 17:07 / ~17:11 ; PROCESSUS_DETACHES: aucun (rien lancé).
