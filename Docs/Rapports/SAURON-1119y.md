# SAURON-1119y : pont de la grille de commandes (G1), GetCommand + GetOrder de bout en bout, NON COMPILE

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, branche `envelope`. BUILDS_DEMANDES: 0. Aucun test lance, aucun quick_check.

## Commits (3, anglais, sans Co-Authored-By, pas de push)
- `b946667` pont C++ : `Source\HeroesOfLiberty\Player\HOLCommandGridBoard.h/.cpp` (fichiers neufs, 162 l.)
- `4a18ec4` planche script : `Script\Player\HOLCommandGridBoard.as` (23 l.)
- `0b73639` branchement : `HOLCommandGrid.cpp/.h` + `HOLCommandGridReadLibrary.cpp` (+21 / -3)

## Fait
- `UHOLCommandGridBoard` : `UCLASS(Abstract)` `UWorldSubsystem` (calque de `UHOLPlayerOrdersBoard` : `ShouldCreateSubsystem` refuse Abstract / NewerVersionExists, `DoesSupportWorldType` Game + PIE). Deux `BlueprintNativeEvent` : `GetCommandAnswer`, `GetOrderAnswer` (prennent `FHOLGridContextView` + `Cell`, rendent `FHOLGridCommandView` : types DEJA exposes de la bibliotheque de lecture, aucun type cree). `_Implementation` rend une vue vide ; l'existence de la planche EST la reponse (pas de drapeau « repondu »).
- Statiques `AskGetCommand` / `AskGetOrder(Context, Cell, FHOLGridCommand& Out)` rendent false quand aucune planche (hors thread jeu, ou aucune classe script compilee) : l'appelant garde alors son corps C++. `FindAny()` parcourt `GEngine->GetWorldContexts()` (les fonctions de la grille n'ont pas d'argument monde). `MakeContextView(FHOLGridContext)` = nouveau sens regle -> vue (l'inverse du `ToRule` de la bibliotheque, qui n'existait pas). Le sens vue de commande -> regle reutilise `HOLCommandGridReadLibraryPrivate::ToRule(const FHOLGridCommandView&)` (declare en externe dans le .cpp du pont : ce `ToRule` a un lien externe, pas d'espace anonyme), donc pas de doublon.
- Script : `UHOLCommandGridBoardScript : UHOLCommandGridBoard`, deux `BlueprintOverride` qui appellent `HOLCommandGridRules::GetCommand` / `GetOrder` (dans `Script\Player\`, pas dans un dossier interdit : tourne sur client). Aucune fonction de bibliotheque a `WorldContext` appelee.
- Branchement : dans `HOLCommandGrid.cpp`, `GetCommand` / `GetOrder` publics demandent d'abord au pont ; les corps d'aujourd'hui sont intacts, renommes `GetCommandNative` / `GetOrderNative` (declares dans le .h, `HEROESOFLIBERTY_API`). REPLI C++ GARDE.
- Boucle (point 3) : `UHOLCommandGridReadLibrary::GetCommand` / `GetOrder` (`HOLCommandGridReadLibrary.cpp` ~172 / 177) appellent maintenant `HOLCommandGrid::GetCommandNative` / `GetOrderNative` : les tests d'accord comparent bien le script au C++ direct. Meme raison, 2e boucle fermee : `HOLCommandGrid::PressKey` (C++, `HOLCommandGrid.cpp` ~267) appelait `GetCommand` public ; il appelle `GetCommandNative`, sinon le test d'accord de PressKey (G2a) comparait le script a un C++ qui demande au script. Les tests C++ directs (`HOLDamnedWarriorTests.cpp:233-235`, `HOLRegicideTests.cpp:292-294`) appellent `HOLCommandGrid::GetCommand/GetOrder` : sans monde de jeu ils tombent sur le C++ (pas de planche) ; avec un monde PIE/jeu ils passeraient par le script (meme resultat attendu, c'est justement un test de plus).

## Fonctions branchees / reste
Branchees : `GetCommand`, `GetOrder`.
Reste (a faire dans un lot ulterieur, meme patron, `BlueprintNativeEvent` + `Ask*` + planche script) : `GetAgeBuilding`, `IsBuildGrid`, `IsAgeReached`, `IsAgeOpen`, `GetAgePanelOpacity` (sites : HUD `HOLGameScreen.cpp`, `HOLGridCell.cpp`), `PressKey`, `ClickAge` (3 / 2 sites), `GetChosenAgeAfter` (6), `CheckBuild` (4), `CheckProduction`, `CheckResearch`, `Describe`, `Blocks`, `IsPopulationBlocked`, `FindMissingResource`, `GetCycleRank`, `IsSelectedByCategory`, `IsRepeatPress`, `NextInCycle`, `IsAloneInGroup`, `SplitSubgroups` (3), `GetCommandPriority`, `MergeCommands` (2), `PickEvenly` (2), `GetReportName`, `GetMouseCursor`, `GetTrade*`, `GetHeroAbilityCell`, `GetRageCell`, `GetCategoryRow`, `GetCellFace`, `SetSpecialization`, `GetSpecializationCell`. Les groupes/cles (G2a/G2b) ne sont pas dans ma branche (G2b : `selection`) : le pont de ces fonctions attend la fusion. Pour chacune : meme regle, la bibliotheque de lecture doit passer au corps `...Native`.

## Performance (point 4)
- Lu dans le code : `HOLPlayerController.cpp` ~3225-3252 : `GetCommand` appele `NumGridCells` = 12 fois par lecture de la grille d'un contexte (boucle `OutCommands[Cell]`) ; pour un GROUPE de batiments (branche ~3237) 12 appels PAR batiment du groupe, puis `MergeCommands`. `GetOrder` : 12 fois (~3350). Le HUD (`HOLGameScreen.cpp:1055`) boucle 12 cases (G1-PLAN-1119.md:48). Donc : 12 appels de pont par rafraichissement pour la grille de droite, +12 pour le panneau des ordres, soit 24 en selection d'unites (grille + ordres), 12 x N batiments pour un groupe.
- Chaque appel construit un `FHOLGridContextView` (tableaux `Produces`, `Researches`, 4 tableaux `Buildable*`, copies de `TArray<FName>`) : cout par appel a mesurer ; 12 conversions IDENTIQUES du meme contexte par rafraichissement (un `MakeContextView` par cellule). Piste de reduction (non faite, a decider apres mesure) : un evenement `GetCommandsAnswer(Context)` qui rend les 12 cellules d'un coup (1 traversee + 1 vue par rafraichissement au lieu de 12).
- Frequence : rafraichissement a chaque selection / stock / palier + ~0,25 s tant qu'une case heros/rage/deploiement est visible (G1-PLAN-1119.md:48, SAURON-1119m:71) : ~48 traversees/s au pire. NON MESURE.
- Cvar de trace / mesure existante pour la grille : AUCUNE trouvee (grep des `TAutoConsoleVariable` de `HOLGameScreen.cpp` : seulement `hol.HUD.UnitStats` / `UnitStatsBuilding` ; `hol.UI.SpecializationCell` dans `HOLCommandGrid.cpp` regle une case, pas une mesure). NON VERIFIE hors ces fichiers. Pas de cvar ajoutee (hors budget).

## A lancer par la session principale
1. Build C++ (un lot : nouveaux `HOLCommandGridBoard.h/.cpp` + `UHT` pour le `UCLASS`) : `<worktree>\Tools\build.ps1 -LiveCoding=0 -Wait`.
2. `quick_check` des parties grille de commandes : tests d'accord G1 (`Test_CG*` : GetCommand / GetOrder script vs bibliotheque, qui doit maintenant etre la version Native) + tests C++ de la grille ; rouge a voir : faire renvoyer une commande vide a `GetCommandAnswer` dans la planche script, le test d'accord ne doit PAS changer (la bibliotheque est sur le C++), mais `hol.HUD.Report` d'un client doit changer.
3. Preuve reseau : `Tools\Scenarios\tier_grid_client_run.ps1 -Root <worktree> -Expect enabled` (case Peasant enabled) ; cette preuve couvre la grille cote client seulement si la planche est nee sur le client : a verifier que `UHOLCommandGridBoardScript` existe dans le monde client (aucun log « no script board » n'a ete ajoute : sans planche le repli C++ est silencieux, donc une planche absente ne se voit PAS : a ajouter si on veut le temoin).

## Champs
- TYPES_A_EXPOSER: aucun nouveau (vues existantes de `UHOLCommandGridReadLibrary`) ; `FHOLGridContext` -> `FHOLGridContextView` fait en C++ (`MakeContextView`).
- TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 (le pont n'a pas de test propre ; le test d'accord existant couvre la regle, pas le pont) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 (pont : +185 l. C++/script, repli garde).

## NON VERIFIE / risques
- Tout : non compile, aucun test. Risques de compilation : (a) signature de l'override script `FHOLGridCommandView GetCommandAnswer(const FHOLGridContextView& Context, int Cell)` (calque des planches existantes, mais ici un retour struct) ; (b) `FHOLGridContextView` / `FHOLGridCommandView` utilises comme parametre/retour d'un `UFUNCTION(BlueprintNativeEvent)` : USTRUCT BlueprintType, devrait aller ; (c) `Out.HeroCooldownSeconds.Append(Context.HeroCooldownSeconds, 3)` sur tableau C.
- `FindAny` prend la premiere planche de tous les mondes (serveur + client PIE dans le meme processus : la regle est pure, meme reponse). La planche existe -> reponse : un sous-classe script qui ne surchargerait pas `GetCommandAnswer` rendrait une commande vide au lieu du repli (non le cas du seul sous-classe ecrit).
- Un test C++ sans monde de jeu garde le repli : le pont n'est donc exerce que dans un monde jeu/PIE.
- Le pont n'est pas atteint avant que le script soit compile : si `HOLCommandGridRules.as` ne compile pas, aucune planche, repli C++ silencieux.

## Anomalies
- `rtk` : deux appels `Bash` refuses par le classifieur (hook qui reecrit la commande), refaits ; ils comptent dans mes 25 appels. Premier fichier ecrit a mon ~15e appel (consigne : 8e) : la lecture des patrons (planches des ordres / construction, absentes de `envelope`, lues par `git show`) et de la bibliotheque de lecture a pris plus que prevu.
- Les patrons `HOLConstructionBoard.*` et `HOLPlayerOrdersBoard.*` ne sont pas dans la branche `envelope` (lus sur `selection` et `integ-15`) ; ce pont suppose la meme classe de base `UWorldSubsystem` que ces patrons (rien de `envelope` ne les referencie).
- Sauvegarde : ni `EtapeDefini.md` ni `Auto_Journey_File.md` touches (consigne).

merge main non fait (aucun build, DLL perimee). Branche `envelope` : prete a compiler.
