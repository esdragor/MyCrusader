# SAURON-1119z : pont de la grille de commandes, suite (familles 1 a 3), NON COMPILE

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, branche `grid-bridge-2` (creee depuis `integ-16`). BUILDS_DEMANDES: 0. Aucun build, aucun quick_check, aucun jeu lance. `EtapeDefini.md` et `Auto_Journey_File.md` non touches. merge main non fait. Branche prete a compiler.

## Commits (anglais, sans Co-Authored-By, pas de push)
- `b182406` famille 1 : ages et touches
- `dada344` famille 2 : Check* et Describe
- `3232982` famille 3 : groupes

Chaque commit = pont (`HOLCommandGridBoard.h/.cpp`) + planche (`Script\Player\HOLCommandGridBoard.as`) + branchement (`HOLCommandGrid.h/.cpp`) + `HOLCommandGridReadLibrary.cpp`. Les regles script (`Rules.as`, `Keys.as`, `Groups.as`) ne sont PAS modifiees : toutes les signatures se raccordent (namespaces `HOLCommandGridRules`, `HOLCommandGridKeys`, `HOLCommandGridGroups`).

## Patron applique a chaque fonction F
1. `UHOLCommandGridBoard` : `BlueprintNativeEvent` `FAnswer` + `FAnswer_Implementation` (= pas de reponse : vide / false / INDEX_NONE) + statique `AskF(..., Out)` (false = pas de planche, ou entree non representable).
2. `HOLCommandGridBoard.as` : `BlueprintOverride FAnswer` qui appelle la regle du namespace.
3. `HOLCommandGrid.cpp` : le corps C++ devient `FNative` (REPLI GARDE, declare dans `HOLCommandGrid.h`, `HEROESOFLIBERTY_API`) ; `F` public = `AskF ? reponse : FNative`. TOUS les appels internes de `HOLCommandGrid.cpp` a ces fonctions passent aux `Native` (regex sur le fichier : un `PressKeyNative` ne rappelle donc jamais le script via `IsAgeOpen`, sinon le test d'accord comparerait le script a lui-meme). Regex gardee contre `HOLUnitPicking::NextInCycle` (autre fonction, autre fichier).
4. `HOLCommandGridReadLibrary.cpp` : chaque `HOLCommandGrid::F(` devient `HOLCommandGrid::FNative(`. Verifie par relecture du diff pour CHAQUE fonction : IsAgeReached, IsAgeOpen, GetAgeBuilding, PressKey, ClickAge, GetChosenAgeAfter, CheckProduction, CheckBuild, Describe, NextInCycle, SplitSubgroups, MergeCommands, PickEvenly (13 / 13).

## Fonctions branchees (13)
- Famille 1 : `GetAgeBuilding`, `IsAgeReached`, `IsAgeOpen`, `PressKey`, `ClickAge`, `GetChosenAgeAfter`.
- Famille 2 : `CheckProduction`, `CheckBuild`, `Describe`.
- Famille 3 : `NextInCycle`, `PickEvenly`, `SplitSubgroups`, `MergeCommands`.
- (deja a l'etape precedente : `GetCommand`, `GetOrder`.)

Conversions ajoutees dans le pont (aucun type nouveau expose) : `FHOLGridKeyResultView -> FHOLGridKeyResult` (la bibliotheque n'avait que l'autre sens ; ranges dans l'espace de noms `HOLCommandGridBoardPrivate`, nom unique pour les unity builds), `FHOLCommandFeedback <-> FHOLFeedbackView` et `FHOLGridCommand -> View` (reutilisent les `FromRule` / `ToRule` a lien externe de la bibliotheque, declares en avant dans le .cpp du pont), groupes / cycle / file <-> vues. `MergeCommands` : si un batiment n'a pas exactement `NumCells` commandes, `AskMergeCommands` rend false et le corps C++ garde la main (la forme a plat du script ne porte pas une autre largeur).

## NON branchees et pourquoi
- `CheckResearch`, `GetCycleRank` : pas de regle script (lignes de table `FHOLTechnologyRow` / `FHOLBuildingTypeRow` non exposees). Restent C++.
- `GetMouseCursor` (famille 4, curseur) : NON branche. Appele a CHAQUE IMAGE (`HOLPlayerController.cpp:1865`, comparaison avec `CurrentMouseCursor`) ; la regle script `IsAttackCursor` existe mais un appel de pont par image n'est pas justifie. Reste C++ (consigne : appele par image = C++). Decision a prendre si on veut quand meme le brancher (evenement `IsAttackCursorAnswer(bool, bool)` + conversion `bool -> Crosshairs / Default`).
- `GetAgePanelOpacity` : reste C++ (fondu, base sur `IsAgeOpenNative`, ne passe jamais par le pont). Aucune autre fonction d'opacite.
- `IsBuildGrid` : non demande dans la liste ordonnee, et appele en interne par beaucoup de fonctions : laisse C++ (a brancher plus tard si voulu, meme patron).
- Hors liste, regles script existantes, non branchees (budget / non demandees) : `Blocks`, `IsPopulationBlocked`, `FindMissingResource`, `IsRepeatPress`, `IsAloneInGroup`, `GetCommandPriority`, `GetReportName`, `GetTrade*` (Cell / Gain / GainText), `GetHeroAbilityCell`, `GetRageCell`, `GetCategoryRow`, `GetCellFace`, `IsSelectedByCategory`, `SetSpecialization`, `GetSpecializationCell`.

Frequence des sites d'appel des fonctions branchees (grep) : `PressKey` / `ClickAge` / `GetChosenAgeAfter` = evenements clavier / clic / selection / annulation (`HOLPlayerController.cpp` 1700, 3379, 3393, 4138, 5791, 5897, 5902) ; `IsAgeReached` = un `UE_CLOG` (3392) ; `CheckProduction` / `CheckBuild` = clic ou ordre (3462, 3574, 4051, 4088) ; `SplitSubgroups` = selection (2570, 2680) ; `MergeCommands` = rafraichissement de la grille d'un groupe (3240) ; `NextInCycle` / `PickEvenly` = touche (2925, 3682). Rien par image. Le HUD script lit la bibliotheque de lecture (donc le `Native`), pas le pont. Les ensembles `CheckBuild(...).IsNone()` a 4088 : non verifie s'il est par image.

## Valeur a fausser par famille pour voir le rouge de CHAINE
Fausser la regle script dans `Script\Player\...` fait tomber le test d'accord ET (si la planche est nee) les tests C++ qui passent par le public :
- Famille 1 : dans `HOLCommandGridRules.as` `IsAgeReached` (`Age <= Context.ReachedAge` -> `Age < Context.ReachedAge`) : tombent les tests d'accord des ages et ceux de `GetAgeBuilding` (grise) ; ou `HOLCommandGridKeys.as` `GetChosenAgeAfter` (`PlacedKept` garde l'age -> rend toujours INDEX_NONE) : tombent l'accord et les tests C++ de la touche Shift.
- Famille 2 : dans `HOLCommandGridKeys.as` `CheckBuild` (rendre `None` au lieu de `NotEnoughResources`) ou `Describe` (changer un texte) : tombent l'accord ; un test C++ qui appelle `HOLCommandGrid::CheckBuild` / `Describe` en monde PIE tombe aussi.
- Famille 3 : dans `HOLCommandGridGroups.as` `PickEvenly` (choisir la file la plus longue au lieu de la plus courte) ou `SplitSubgroups` (omettre le sous-groupe `Tous`) : tombent l'accord et les tests C++ de groupe en monde PIE.
Rouge de PONT (le jeu lit bien le script, la bibliotheque NON) : fausser `...Answer` dans `HOLCommandGridBoard.as` (par ex. `return false;` dans `IsAgeOpenAnswer`) : le test d'accord ne bouge PAS (il compare le script au `Native`), mais un test C++ sous monde de jeu / PIE qui appelle le public (`HOLCommandGrid::IsAgeOpen`) change. Sans monde de jeu, un test C++ garde le repli (le pont n'est exerce qu'en jeu / PIE).

## A lancer par la session principale
1. Build C++ (UHT : 13 `UFUNCTION(BlueprintNativeEvent)` de plus dans `UHOLCommandGridBoard`) : `<worktree>\Tools\build.ps1 -LiveCoding=0 -Wait`.
2. `quick_check` parties grille de commandes : tests d'accord (ages, touches, checks, groupes) + tests C++ de la grille (Roster, WarriorGrid, Zombie, UnitTier...). Puis un rouge de chaine par famille (ci-dessus).
3. `Tools\Scenarios\tier_grid_client_run.ps1 -Root <worktree> -Expect enabled`.
4. Mesure : le pont de `MergeCommands` convertit `12 x N` commandes en vues a chaque rafraichissement de groupe ; a regarder si le groupe est gros.

## Champs
- BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; LIGNES_CPP_SUPPRIMEES: 0 (repli garde, pont : environ +340 l. C++ / script) ; LIGNES_MIGREES_PAR_LOT: 0.
- TYPES_A_EXPOSER: aucun.
- TESTS_TAUTOLOGIQUES: sans objet (pas de test neuf).
- PROCESSUS_DETACHES: non lance (aucun processus demarre).

## NON VERIFIE / risques
- Tout est non compile. Risques de compilation : (a) `BlueprintNativeEvent` qui retournent `TArray<...>` et `FString`, qui prennent `const FHOLResources&` (USTRUCT deja utilise par la bibliotheque) et `EHOLAgeEventKind` ; (b) overrides script `BlueprintOverride` a retour `TArray` / `FString` ; (c) `using namespace HOLCommandGridBoardPrivate;` place avant `FindAny` dans `HOLCommandGridBoard.cpp` ; (d) `TConstArrayView<TArray<FHOLGridCommand>>` -> boucle sur `const TArray<FHOLGridCommand>&` ; (e) `Context.ReachedAge` etc. deja convertis par `MakeContextView`.
- `FindAny` prend la premiere planche de tous les mondes ; une planche existante repond toujours (une surcharge manquante rendrait la reponse vide, jamais le repli) : toutes les surcharges des 13 fonctions sont ecrites.
- Le commentaire d'en-tete « To do » de `HOLCommandGridBoard.as` est perime (reste a retirer).

## Anomalies
- Un appel `Bash` refuse par le classifieur (hook `rtk`), remplace par l'outil Grep. Premier fichier ecrit a mon 11e appel (consigne : 8e) : la lecture du pont, des signatures et de la bibliotheque de lecture a pris plus que prevu.
- Piege PowerShell : `$W` (table) ecrasait `$w` (chemin du worktree) : variables a noms distincts a partir de la famille 2. Aucune edition perdue (verifie par diff).
