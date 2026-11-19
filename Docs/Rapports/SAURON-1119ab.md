# SAURON-1119ab : lots C9-1 et C9-2 (commandes console « pilote de test » du contrôleur -> script), NON COMPILÉ

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, branche `pilots` (depuis `net-playtest`). Aucun build, aucun quick_check, aucun jeu lancé. Branche prête, merge main non fait.

## Commits (anglais, sans Co-Authored-By, non poussés)
| sha | contenu |
|---|---|
| f92d5d1 | liaison : `HOLControllerCommandLibrary.h/.cpp` section C9 (28 enveloppes statiques UFUNCTION, +304 lignes) |
| 5e2b8df | `Script\Player\HOLControllerCommandsC9a.as` + `_Test.as` (12 commandes) |
| 978c021 | `Script\Player\HOLControllerCommandsC9b.as` + `_Test.as` (6 commandes) |
| d1e88ac | retrait C++ C9-1 de `HOLPlayerController.cpp` (-289 lignes) |
| 82c20de | retrait C++ C9-2 + suppression pure Select.Order / OrderLine (-138 lignes) |

## Commandes migrées (18, script ; même nom, mêmes arguments, mêmes lignes de journal LogHOLMatch, même niveau)
C9a : `hol.Match.Pause`, `hol.Match.Resume`, `hol.Victory.Surrender`, `hol.Units.AttackMove`, `OrderLine`, `AttackBuilding`, `GatherOrder`, `DropOff`, `Build`, `Garrison`, `UngarrisonAll`, `hol.Prod.RallySite`.
C9b : `hol.Select.Stop`, `Hold`, `NextIdle`, `Own`, `Type`, `hol.Build.CancelSelected`.
Les lignes « Order: N units ... » (Display), « Usage: ... » et « no ... known here » (Warning) sont reprises mot pour mot ; `%.0f` devient `Math::RoundToInt`. Les gardes (spectateur compris) restent celles des `Request<X>` C++ (appelés par la liaison) ; le script ne les double pas. Le `#if !UE_BUILD_SHIPPING` autour de `hol.Units.*` / `RallySite` n'a pas de jumeau (scripts non chargés en Shipping).

## Supprimées (pur, 0 usage)
`hol.Select.Order`, `hol.Select.OrderLine` : Grep `Select\.Order\b|Select\.OrderLine\b` sur `Tools\` et `Script\` du worktree = 0 occurrence (revérifié) ; aucun appel par la console dans `Source\`. `HasLineFlag` et `ProjectGroundPoint` restent (utilisés par OrderLineGround, BoxGround...).

## Restées en C++ (aucune)
Aucune des 18 n'a dépassé ~15 lignes de liaison neuve (la plus lourde : `FindGatherSource`, 17 lignes, partagée avec rien d'autre : acceptée car découpée en 3 codes de retour). Non touchés comme demandé : LeftClickGround, TestCursorGround, Banner, AttackMove(Near), rapports, caméra, groupes / grille / catégories / sous-groupes, `Do<X>` / `Request<X>`.

## Champs
- BUILDS_DEMANDES: 1 (lot C++ : liaison + retrait ; à lancer par la session principale, voir ci-dessous)
- TESTS_CPP_RETIRES: 0
- TESTS_SCRIPT_AJOUTES: 6 (C9a : `Test_ControllerCommandsC9aEachNameOnceAndKnown`, `...C9aArguments`, `...C9aOrderTexts` ; C9b : `...C9bEachNameOnceAndKnown`, `...C9bTypeLast`, `...C9bTexts`)
- TESTS_TAUTOLOGIQUES: 0 (les plans et textes sont ceux que la console enregistrée appelle)
- TEST_ROUGE: NON VU (aucun build autorisé). Valeurs à fausser (une fois, puis remettre) : C9a : dans `HOLControllerCommandsC9a.as` `Queued()` renvoyer `"; queued"` -> `Test_ControllerCommandsC9aOrderTexts` rouge sur « attack-move queued suffix » ; C9b : dans `KeepLast` garder `Ids[0]` au lieu du dernier -> `Test_ControllerCommandsC9bTypeLast` rouge sur « last keeps the newest id ».
- LIGNES_CPP_SUPPRIMEES: 427 (C9-1 : 289, C9-2 : 138) dans `HOLPlayerController.cpp` ; LIGNES_CPP_AJOUTEES (liaison) : 304 (le plan en prévoyait ~140 : l'enveloppe est verbeuse en `{ }` à une ligne et les en-têtes commentés) ; net C++ : -123.
- LIGNES_MIGREES_PAR_LOT: C9-1 289 -> ~330 lignes script (C9a + test) ; C9-2 138 (dont 33 par suppression pure) -> ~115 script (C9b + test)
- TYPES_A_EXPOSER: `AHOLBuilding` (UCLASS : poignée et `GetActorLocation` utilisés par le script, à confirmer à la compilation du script)
- EDITS_REFUSES: 0 ; PROCESSUS_DETACHES: aucun lancé (pas de stray_processes)

## À lancer par la session principale (je n'ai rien exécuté)
1. `Tools\build.ps1 -LiveCoding=0 -Wait` dans `HeroesOfLiberty-Sauron` (C++ : liaison + retrait ; une erreur possible : un include manquant dans `HOLControllerCommandLibrary.cpp` ou un symbole de `HOLPlayerController.cpp` devenu inutilisé -> avertissement seulement).
2. Tests script (compilation des `.as` incluse) puis `quick_check -Parts` des parties touchant `Select.Own`, `Select.Type`, `Select.Stop`, `Select.Hold`, `Build.CancelSelected`, `Match.Pause/Resume`.
3. Rouge unique par lot (valeurs ci-dessus), puis vert.

## Pour le rejeu : 2 passages par commande (Sc = scénario `Tools\Scenarios\*.txt`, QC = `quick_check.ps1`, R = runner réseau)
| Commande | Passages |
|---|---|
| Match.Pause / Resume | `disconnect_pause.txt` + QC (2 sites chacune) ; `lobby_match.ps1`, `rpc_routes_run.ps1` (Pause) |
| Victory.Surrender | `lobby_match.ps1`, `manager_client.ps1` (aucun Sc, aucun QC) |
| Units.AttackMove / OrderLine | `rpc_orders_p1p2_run.ps1` (les deux) ; `rpc_orders_p1p2` lit « Order: N units attack-move / in line » |
| Units.GatherOrder / Build / Garrison, Prod.RallySite | `rpc_orders_p3p4_run.ps1` (les quatre) |
| Units.AttackBuilding / DropOff / UngarrisonAll | AUCUN passage trouvé (plan C9-PLAN : seulement un commentaire de `HOLRpcPlayer.as`) : NON VÉRIFIÉ, à jouer par la main du runner `rpc_orders_*` si elle les construit par nom |
| Select.Stop | `building_destroy.txt`, QC (1) |
| Select.Hold | QC (2 sites), pas de Sc |
| Select.NextIdle | `building_attack.txt` (pas de QC) |
| Build.CancelSelected | QC (3 sites), pas de Sc |
| Select.Own | QC (6 sites), 32 scénarios dont `monk.txt` (à confirmer par `grep -l`) |
| Select.Type | QC (1), 9 scénarios (`grep -l "Select.Type"`) |
Valeur à fausser pour le rouge d'une commande : le texte (« Order: N units ... », « Select own: N units », « Select type T: N units ») car les scénarios l'attendent par `waitlog`.

## NON VÉRIFIÉ
- Compilation C++ et script (rien lancé) ; les noms exacts `HOLScriptLog::LogDisplay/LogWarning(FName, FString)` sont repris de C2b.
- `Math::RoundToInt`, `FName(FString)` avec chaîne vide = `None`, `GetAllActorsOfClass`/`IsLocalController` (repris de C2b), `AHOLBuilding` utilisable comme type script, `TArray<int>` passé à un paramètre `TArray<int32>&`.
- La liaison des `.as` : `HOLControllerCommand::OwnUnitIds` vs les anciennes fonctions homonymes (`OwnSelectedUnitIds` existe, nom distinct).
- `FindMainCentreOf` suppose que `UHOLControllerReadLibrary::FindMainCentre` accepte `const AHOLPlayerController&` (signature lue : oui).
- Comptes de scénarios : issus de C9-PLAN-1119 (non revérifiés, sauf le 0 de Select.Order / OrderLine).
- Pas de `git merge main` (DLL du worktree périmée par ces changements).

## Anomalies
- Un script de retrait a d'abord copié la liste par référence : le 2e commit a été faux ; `git reset --hard d1e88ac` (mon propre commit) puis re-découpe avec ancres vérifiées -> `82c20de` correct (427 suppressions au total, accolades équilibrées).
- Un appel Bash avec `python3` en entrée a bloqué 120 s (aucun effet sur les fichiers).
- Aucune écriture dans `EtapeDefini.md` / `Auto_Journey_File.md` (consigne).
