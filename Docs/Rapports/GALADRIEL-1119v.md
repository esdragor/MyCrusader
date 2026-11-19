# GALADRIEL-1119v : lot K1a (enchainement des portes de Place et Cancel en script, repli C++ garde)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`, branche `galadriel-tab` (base main 41dc1a0). TOUT NON COMPILE, aucun build, aucun jeu lance. Lance a 11:00:55, rapport ecrit vers 11:07.

## Commits (anglais, sans Co-Authored-By, pas de push)
- TEST   c5b88ad : `Script\Tests\HOLConstruction_Test.as` + `Test_Construction_PlaceVerdict`, `Test_Construction_CancelVerdict` (+ helper `ConPlace`)
- SCRIPT b6f4d24 : `Script\Buildings\HOLConstructionBoard.as` (dossier cree ; ni Dev / Developers / Examples / Editor)
- C++    d46074c : `Source\HeroesOfLiberty\Buildings\HOLConstructionBoard.h/.cpp` (neufs, ~110 l.) + `HOLConstructionSubsystem.cpp` (Place et Cancel, +106 / -40)

## Champs du brief commun
- BUILDS_DEMANDES: 1 (lot C++ K1a, a lancer par la session principale)
- TESTS_CPP_RETIRES: 0
- TESTS_SCRIPT_AJOUTES: 2 (Test_Construction_PlaceVerdict, Test_Construction_CancelVerdict ; 14 + 10 assertions)
- LIGNES_CPP_SUPPRIMEES: 0 (repli garde ; le C++ des portes sera retire au lot suivant, apres preuve reseau)
- LIGNES_MIGREES_PAR_LOT: ~84 l. de portes de Place/Cancel ont maintenant une source script (copie C++ gardee en repli)
- TEST_ROUGE: NON VU (rien compile ; rouge attendu a la compilation tant que HOLConstructionBoard.as manque, ou en faussant la valeur ci-dessous)
- Valeur a fausser pour voir le rouge : dans `PlaceVerdict`, permuter les portes faction et age (le cas "faction avant age" donne alors AgeNotReached au lieu de NotOfFaction), ou rendre TryPay strict (cas 50 pour 50 -> NotEnoughResources au lieu de Ok). Pour Cancel : dans `CancelVerdict` retirer `OwnerPlayer != AskerPlayer` (le cas "foundation d'un autre joueur" donne Allowed).
- TESTS_TAUTOLOGIQUES: 0 (les verdicts passent par les vraies regles IsAgeOpen / TryPay / CanCancel / GetCancelRefund exposees)
- EDITS_REFUSES: 0 ; PROCESSUS_DETACHES: non lance (aucun process demarre par moi ; `stray_processes -Stop` aurait pu tuer le build de synchronisation de la session principale)

## Conception (ecart avec la recette, a savoir)
- Pas de parametres `&` sortants : les `FHOLResources&` de la recette sont remplaces par deux evenements qui RENDENT la valeur (`StockAfterPlaceAnswer`, `CancelRefundAnswer`) : evite le doute sur la syntaxe `&out` d'un BlueprintOverride Angelscript. Verdicts en entiers (enum script `EHOLPlaceVerdict` 0 Ok 1 NotBuildable 2 NotOfFaction 3 AgeNotReached 4 PlacementRefused 5 NotEnoughResources ; `EHOLCancelVerdict` 0 Allowed 1 Ignored).
- Les structs `FHOLResources` passent PAR VALEUR dans les `UFUNCTION(BlueprintNativeEvent)` (pas `const&`) : a verifier a la compilation (UHT + liaison script).
- Place : les entrees (faction, tier, refus de `CheckPlacement`, cout, stock) sont calculees AVANT le verdict si un tableau existe et si le type est constructible ; `CheckPlacement` est donc evalue aussi quand faction ou age refuserait (requete pure, un peu plus de travail sur un refus). Le stock est seulement CALCULE (`AskStockAfterPlace`) ; `PlayerState.SetResources(Stock)` reste APRES `SpawnActorDeferred` et son `if (!Foundation) return;` : ordre exact conserve (le joueur ne paie jamais sans batiment).
- Reponse hors plage (`default:` du switch de Place) = repli C++ ; pour Cancel, toute reponse != 0 = ignorer (+ meme ligne de journal). Les lignes de journal et codes de refus sont mot pour mot ceux d'avant. `DamageBuilding`, `RefundProductionQueue`, `DestroyBuilding`, `ApplyOrders` intacts.
- Repli : `Find(World)` nul ou -1 -> la chaine C++ d'origine (maintenant sous `if (Verdict < 0) {...}` dans Place ; `Verdict = ... ? 1 : 0` dans Cancel).
- Fichier de test : ajout en fin de `HOLConstruction_Test.as` (CRLF respecte) car les fonctions globales `ConRes` / `ConResEq` ne sont pas visibles d'un autre fichier.

## TYPES_A_EXPOSER
Aucun de plus : le pont ne passe que des entiers, bool, int64 et `FHOLResources` (deja expose). `AHOLBuilding`, `AHOLPlayerState`, `FHOLConstructionCommand` restent non exposes et inutiles.

## Preuves a rejouer (par la session principale, UN build groupe)
1. Build du worktree (C++ neuf : 2 fichiers + Place/Cancel) ; erreurs probables a surveiller : UHT sur `FHOLResources` par valeur en retour d'un BlueprintNativeEvent ; script : `int64` dans l'override `CancelRefundAnswer`, nom `Ok` / `Allowed` d'enum (portee `EHOLPlaceVerdict::Ok`).
2. Tests script : `HOLConstruction_Test.as` (les 2 nouveaux, puis tout le fichier) via le lanceur de tests script habituel. Rouge a VOIR une fois avec la valeur ci-dessus avant le vert.
3. `quick_check -Parts build` (pose par clic, refus maison / arbre / centre ennemi / sans bois, annulation avec remboursement, D345 1 a 49 des 50 bois), `-Parts net` (chaque client pose et paie seul), et la preuve que le tableau est NE sur le serveur dedie (journal sans erreur de classe, ligne "Construction: ... placed ... foundation").
4. Test d'accord reseau : meme scenario avec et sans tableau (repli) -> memes lignes de journal ; puis lot suivant : retrait de la chaine C++ des portes.
5. `HOLBuildingTests.cpp` (DamageBuilding/DestroyBuilding) doit rester vert (non touche).
6. Merge main : non fait (DLL perimee, la session principale synchronise).