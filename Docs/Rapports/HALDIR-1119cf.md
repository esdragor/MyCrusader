# HALDIR-1119cf : GetCycleRankOfType -> deux lectures de table (tache 2)

NON COMPILE, NON TESTE (aucun build, aucun quick_check, par consigne). Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`, branche `grid-l3b`, commit `6e03e63` (sur 943022f). Lance 17:34, fin ~17:37 (arret dur 17:49 respecte), ~9 appels. Pas de merge, pas de push. `Content\Maps\Crown.umap` non touche. EtapeDefini.md / Auto_Journey_*.md non touches. IN_USE.txt : OCCUPE a l'entree (CPP: oui), libre a la sortie.
BUILDS_DEMANDES: 0 (a lancer par la session principale : lot C++ a grouper). TESTS_CPP_RETIRES: 0. TESTS_SCRIPT_AJOUTES: 0. LIGNES_CPP_SUPPRIMEES: 5 (GetCycleRankOfType : 1 UFUNCTION + 1 declaration + corps 5 lignes dont `Type->BuildGridSlot + 1`) ; LIGNES_CPP_AJOUTEES: 14 (.h : 1 commentaire + 2 UFUNCTION/declarations ; .cpp : 2 corps de 5 lignes + ligne vide). LIGNES_MIGREES_PAR_LOT: 0. TYPES_A_EXPOSER: aucun de plus (IsBuildable et BuildGridSlot sont maintenant lus par FName). TESTS_TAUTOLOGIQUES: 0.

## Fait
- Grep avant ecriture (Source + Script du worktree) : aucune lecture existante de `IsBuildable` / `BuildGridSlot` pour un FName (HOLCommandGridReadLibrary, HOLProductionTables, UHOLTestWorldLibrary : rien) ; seuls usages = lignes de table lues en script par `FHOLBuildingTypeRow` dans d'autres tests (HOLConstruction_Test) et le `Type.IsBuildable()` de HOLCommandGrid.cpp:483-488. Donc ajout justifie.
- `HOLCommandGridReadLibrary.h/.cpp` : `GetCycleRankOfType` retire ; ajoutes `static bool IsTypeBuildable(FName)` (= `Type && Type->IsBuildable()`) et `static int32 GetBuildGridSlotOfType(FName)` (= `Type->BuildGridSlot`, INDEX_NONE si le type n'est pas dans la table), UFUNCTION(BlueprintPure), aucune regle.
- `Script\Tests\HOLCommandGrid_Test.as` Test_CGCycle : le rang = `HOLCommandGridGroups::GetCycleRank(IsTypeBuildable(T), GetBuildGridSlotOfType(T))` pour MainCentre / House / MiningCamp ; assertion en clair `Centre < House && House < Camp` inchangee. La garde `IsTypeInTable` (bOk1) reste.
- `HOLCommandGridBoard.h:15-16` NON modifie : dans CE worktree, `GetCommandNative` / `GetOrderNative` existent encore (HOLCommandGrid.cpp:63,182 ; .h:364,369), le commentaire « removed in a later lot » est donc exact ici ; il ne devient perime qu'apres la retrait natif (L1a-L4) : a corriger par la session qui fusionne cette suppression.
- Fins de ligne : fichiers CRLF conserves (git `eol=crlf`).

## ROUGE VU (non executable ici) : falsification d'UNE ligne qui rend Test_CGCycle rouge
`Script\Player\HOLCommandGridGroups.as:34` `return bBuildable ? BuildGridSlot + 1 : 0;` -> `return bBuildable ? 1 : 0;` : les trois rangs valent 1, `Centre < House` faux, Test_CGCycle rouge (rouge deja attendu aussi sur Test_CGBGetCycleRank / Test_CGGHardValues). Variante cote lecture C++ : `GetBuildGridSlotOfType` -> `return Type ? 0 : INDEX_NONE;` donne le meme rouge. A remettre apres.
Limite : l'assertion d'ordre ne distingue pas `slot + 1` de `slot` (voir le dernier point de NON VERIFIE).

## Relecture de HOLCommandGridSweeps_Test.as contre les pieges du brief (par Grep, precedents dans Script\)
- `int&` / `FString&` en parametre : precedents qui compilent deja : HOLGather_Test.as:75 (`int& Left, int& Asked`), :12 ; HOLAvoidRules_Test.as:197 (`int& Clear`) ; HOLVolcanoBanner.as:56 (`FString& OutDescribe`).
- `FString::Contains` : HOLAIDuel_Test.as:146, HOLAIForumLives_Test.as:117 etc. `TArray<FName>::Contains(FName)` : HOLAITech.as:145, HOLAIOpening.as:1088, HOLControllerCommandsC10a_Test.as:25.
- Cast d'enum depuis un entier : HOLSiegeRules_Test.as:36 (`EHOLSiegeEngine(Index)`), HOLSocleLibraries_Test.as:35.
- Ternaire entre deux f-strings : HOLAIRelay.as:264,276 ; HOLChatRules_Test.as:144.
- Aucune correction necessaire : rien sans precedent. Aucun conteneur imbrique (Seen, BuiltHere = TArray<FName> simples), fonctions CGSW* prefixees (pas de fonction globale partagee entre fichiers), pas de `float` en cause. Pas relu ligne a ligne : le piege « methode de classe appelee comme argument de AssertEquals » n'a pas ete rechecke (les aides sont des fonctions globales, pas des methodes).

## NON VERIFIE
- Compilation C++ (2 UFUNCTION neuves) et Angelscript (binding `HOLCommandGridRead::IsTypeBuildable` / `GetBuildGridSlotOfType` ne existe qu'apres build ; Test_CGCycle ne compile pas avant).
- `Script\Player\HOLCommandGridKeys.as:9` cite encore `GetCycleRankOfType(FName) of the library stays the way in` : commentaire perime, fichier hors de mes 3 fichiers, non touche.
- Anomalie : `Source\HeroesOfLiberty\Player\HOLCommandGrid.cpp:488` garde une autre copie C++ de `slot + 1` (`GetCycleRankNative`, replie du corps natif de la grille) ; elle disparait avec le retrait natif L1a-L4, pas par cette tache.
- Test faible : `Centre < House < Camp` ne verrouille pas le `+ 1` ; si on veut une valeur en clair (ex. MainCentre rang = slot + 1), c'est un ajout de test, pas fait (consigne : valeurs inchangees).
- Duree et verts des 9 tests Sweeps (voir HALDIR-1119ce.md).
PROCESSUS_DETACHES: aucun lance.