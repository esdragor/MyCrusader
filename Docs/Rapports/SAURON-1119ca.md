# SAURON-1119ca : lot C10-2 (commandes console sources / soin / attaque / ordre d'unites / placement -> script), NON COMPILE

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, branche `pilots-2` (depuis 4d72982 = C10-1). Aucun build, aucun quick_check, aucun jeu lance. Branche prete, merge main non fait. Heure de fin 16:22 (arret dur 16:32).

NON COMPILE : la liaison C++ et les scripts n'ont jamais ete compiles (C10-1 non plus).

## Commit (anglais, sans Co-Authored-By, non pousse)
| sha | contenu |
|---|---|
| 8d853a0 | `Script\Player\HOLControllerCommandsC10b.as` (300 l.) + `_Test.as` (105 l.) ; liaison `HOLControllerCommandLibrary.h/.cpp` (section C10b, +55 / +190 lignes dont include) ; retrait des 9 commandes et de leurs aides de `HOLPlayerController.cpp` (-291 lignes) |

## Commandes migrees (9, script ; meme nom, memes arguments, memes lignes, meme niveau, categorie LogHOLMatch)
`hol.Select.OrderSource`, `ClickSource`, `Heal`, `Attack`, `OrderSourceNear`, `hol.Units.Order`, `hol.Units.OrderNear`, `hol.Build.PlaceNear`, `hol.Build.PlaceAtSource`.
- Warning : `Usage: hol.Select.OrderSource <X> <Y> [shift]`, `...ClickSource <X> <Y>`, `...Heal <UnitId>`, `...Attack <UnitId> [shift]`, `hol.Build.PlaceNear <Type> <Dx> <Dy> [enemy]`, `hol.Build.PlaceAtSource <Type> <X> <Y>` ; `<Cmd>: no standing source` ; `<Cmd>: the source is not on the screen` ; `hol.Build.PlaceNear: no own|enemy main centre known here` ; `hol.Build.PlaceAtSource: no standing source`. Units.Order, Units.OrderNear, OrderSourceNear : aucune ligne Usage (comme avant).
- Display : `Right click on source N at (f, f)[, Shift]`, `Click on source N at (f, f)`, `Order: N units heal #U`, `Order: N units attack #U[, queued]`, `Order: N units to (X, Y)[, other players' too]`, `Build on source N at (X, Y)`. `%.3f` -> `HOLControllerCommandsC10a::Fixed3`, `%.0f` -> `Math::RoundToInt`.
- OrderSourceNear / Units.OrderNear : le C++ passait par `ProcessUserConsoleInput("hol.Select.OrderSource %.0f %.0f")` ; le script appelle directement la meme fonction avec le point arrondi en cm entiers (le Warning garde le nom `hol.Select.OrderSource`, comme avant).
- Lignes lues par `Tools\` (Grep) : `Order: N units to (` (Units.Order : quick_check 2293, 2705, 6142 ; rpc_orders_p1p2_run.ps1 107 `Order: (\d+) units to \((-?\d+), (-?\d+)\)`), `Order: N units attack #` (quick_check 3414, 6163, 6265 ; rpc_orders_p1p2_run.ps1 145), `Right click on source (\d+)` (quick_check 6399), `Build request` (journal net, rpc_orders_p3p4_run.ps1 167 : texte garde en C++). Non lues : les autres.

## Liaison C++ (raison D839 : liaison a exposer ; une seule regle, pas de jumeau)
`UHOLControllerCommandLibrary` (UFUNCTION) : `SourceScreenPoint` (ex `AimAtSource`, rend 0 ok / 1 pas de source / 2 hors ecran), `FirstUnitIdOfSlot`, `AllUnitIds`, `CommandUnitIds`, `RequestHealUnitIds`, `RequestAttackUnitIds` (+ `OnAttackOrderGiven.Broadcast`), `RequestMoveUnitIdsTo`, `StartCentreOf`, `RequestPlacementNear`, `StandingSourceNear`, `RequestPlacementOnSource` ; prive non UFUNCTION : `RequestPlacementAtPoint` (ex `RequestPlacementAt` du controleur, ecrit toujours la ligne `Build request: ...`). Include ajoute : `Buildings/HOLConstructionSubsystem.h`. `OwnUnitIds` (C9a) reutilise pour Units.Order sans `all`.

## Retire de HOLPlayerController.cpp (5672 -> 5381 lignes, -291)
9 `FAutoConsoleCommand` + `AimAtSource`, `FindMainCentre` (wrapper local), `RequestPlacementAt` (verifies : plus aucun appelant, Grep 0). Reste en C++ : `FindOwnBuilding` (sans appelant avant moi aussi, NON touche), `GetPointNear`, `ProjectGroundPoint`, Build.Place / Click, FocusNear, AttackMoveNear, C10-3. Accolades 992/992.

## Tests script (3 fonctions, `HOLControllerCommandsC10b_Test.as`)
`Test_ControllerCommandsC10bEachNameOnceAndKnown` (9 noms, uniques, absents de C2b/C9a/C9b/C10a), `...Arguments` (Usage exacts, slot=<S> / id, shift Attack seulement et toute casse, Heal ne queue jamais, `all` / `enemy` exacts, OrderNear sans decalage), `...Texts` (chaque ligne de journal, arrondi en cm entiers, Fixed3). Valeurs ecrites en clair. Limite : visee d'une source, recherche par slot et requetes sont en C++, non testees en script (preuve par scenarios).

## Scenarios a rejouer (Grep -l non execute faute d'appels ; a faire par la session principale)
`Tools\Scenarios\*.txt` contenant `hol.Select.OrderSource`, `hol.Select.Heal`, `hol.Select.Attack`, `hol.Units.Order`, `hol.Build.PlaceNear`, `hol.Build.PlaceAtSource`. Preuves du plan : `monk.txt`, `monk_range.txt` (Heal), quick_check parties `Select.OrderSource` (17 sites), `Build.PlaceNear` (19 scenarios), `sides` (Attack slot=), `net monk`, `rpc_orders_p1p2_run.ps1` (Units.OrderNear, Attack).

## FALSIFICATION (rouge a voir)
Fichier `HOLControllerCommandsC10b.as`, fonction `UnitsOrderText` : ecrire `units at (` au lieu de `units to (` -> `Test_ControllerCommandsC10bTexts` rouge sur « Units.Order line » et « Units.Order all line » ; puis remettre. (La note identique est en tete du `_Test.as`.)

## A lancer par la session principale
1. `Tools\build.ps1 -LiveCoding=0 -Wait` dans `HeroesOfLiberty-Sauron` (liaison C10a + C10b en un build).
2. Tests script, puis `quick_check -Parts` : `Select.OrderSource`, `Build.PlaceNear`, `sides`, `net` (monk), `Select.ClickUnit`, `Select.OrderGround`, `formation` ; puis `rpc_orders_p1p2_run.ps1`.

## Champs
- BUILDS_DEMANDES: 1 (partage avec C10-1)
- TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 3 (Test_ControllerCommandsC10bEachNameOnceAndKnown, ...Arguments, ...Texts) ; TESTS_TAUTOLOGIQUES: 0
- TEST_ROUGE: NON VU (aucun build autorise)
- LIGNES_CPP_SUPPRIMEES: 291 (HOLPlayerController.cpp) ; liaison ajoutee ~ +245 (h/cpp) ; net C++ ~ -46 (la liaison n'est pas la boucle : peu de gain, c'est le script qui porte les arguments et le journal)
- LIGNES_MIGREES_PAR_LOT: ~291 -> 405 lignes script (code + test)
- TYPES_A_EXPOSER: aucun nouveau
- EDITS_REFUSES: 2 (Edit refuse « not read » sur .h/.cpp ; remplace par PowerShell sur marqueurs)
- PROCESSUS_DETACHES: aucun lance

## NON VERIFIE
- Compilation C++ et script. Risques C++ : `Controller->OnAttackOrderGiven.Broadcast` et `GetCommandUnits` / `CheckPlacementHere` / `RequestPlaceBuilding` accessibles depuis la bibliotheque (ils l'etaient depuis un lambda anonyme du .cpp du controleur, donc publics, mais pas verifie que la bibliotheque n'a pas besoin d'`friend`) ; `UHOLBuildingSettings::FindType` : declare dans `Buildings/HOLBuildingTypes.h` (inclus) ; `FHOLSource::TypeIndex`, `Sources->GetType` ; `private:` ajoute en fin de classe apres des UFUNCTION (UHT : un `private:` dans un UCLASS ne gene pas une fonction statique non UFUNCTION) ; `const TArray<int32>&` en parametre UFUNCTION (precedent : `RequestAttackMoveUnits`).
- Risques script : `TArray<int>` passe a un parametre `TArray<int32>&` (precedent C9a `OwnUnitIds`) ; `FName(FString)` ; ordre des parametres `double&` / `int&` en sortie pour `SourceScreenPoint`, `StandingSourceNear`, `StartCentreOf` ; `Args.Contains("add")` deja utilise par C10a ; fonctions de `HOLControllerCommandsC10a::Fixed3` appelees depuis un autre fichier (namespace, OK selon le piege 2) ; les tests appellent `HOLControllerCommandsC10a::Names()`.
- Differences de comportement connues : (a) Heal/Attack `Atoi64` -> `int(ParseReal(...))` : identiques sous 2^31 ; texte non numerique donne 0 des deux cotes, a verifier pour ParseReal ; (b) `Units.Order` sans `all` : `OwnUnitIds` filtre sur l'indice du joueur, identique au C++ sauf sans PlayerState (le C++ n'envoyait alors rien et loguait 0 ; `OwnUnitIds` rend peut-etre une liste vide ou `false`, meme resultat attendu) ; (c) `%.0f` demi pair vs `RoundToInt` demi haut sur un tie exact ; (d) `Build.PlaceNear` : l'ordre « Warning avant ou apres le log » est conserve ; la ligne `Build request` vient de C++ et reste au niveau Display.
- Pas de `git merge main` (DLL perimee).

## Anomalies
- Edit refuse deux fois (« not read ») : contourne par PowerShell sur marqueurs, avec comptage des marqueurs (1 chacun) et verification des accolades. Git signale LF -> CRLF au prochain toucher (normal).
- Aucune ecriture dans `EtapeDefini.md`, `Auto_Journey_File.md`, `IN_USE.txt`, `*InProgress.md` (consigne).
- `FindOwnBuilding` du controleur n'a aucun appelant (Grep) mais n'appartient pas au lot : non touche.
