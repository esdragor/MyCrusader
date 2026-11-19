# SAURON-1119x : lot G2a (clés et retours de la grille de commandes, script seul)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, branche `envelope`, commit `e8af0df` (NON COMPILÉ, aucun build, aucun quick_check, aucun jeu lancé). merge main non fait, DLL périmée.

## Fichiers (neufs)
- `Script\Player\HOLCommandGridKeys.as` (`namespace HOLCommandGridKeys`) : appelle seulement `HOLCommandGridRules::` (G1) et les USTRUCT de `UHOLCommandGridReadLibrary`.
- `Script\Tests\HOLCommandGridKeys_Test.as` : 3 tests, noms globaux préfixés `CGKY`.

## Fonctions portées (HOLCommandGrid.cpp 238-418)
PressKey, ClickAge, GetChosenAgeAfter, FindMissingResource (sans paramètre out : renvoie le n° de ressource ou -1) + HasMissingResource, CheckProduction, CheckBuild, Blocks, IsPopulationBlocked, Describe (+ GetResourceName, GetCategoryName, DescribeBlock), GetCellFace, IsCategorySelectedByCategory (version « catégorie du type », le C++ IsSelectedByCategory prend la ligne de table).

## Fonctions NON portées
- `CheckResearch` : dépend de `FHOLTechnologyRow` et `HOLTechnology::Check` (C++ seul). Reste `HOLCommandGridRead::CheckResearch(FName, FHOLTechFactsView, ...)`.
- `GetCycleRank` : dépend de `FHOLBuildingTypeRow::IsBuildable` et `BuildGridSlot`. Reste `GetCycleRankOfType(FName)` de la bibliothèque.
- Hors lot : cycle de sous-groupes, groupes, `MergeCommands` (G2b), pont C++, retrait du C++.

## Décisions à relire (nombres recopiés, non exposés)
Describe reçoit le refus de placement et le blocage de technologie comme NOMBRES (`FHOLFeedbackView::Refusal`, `::Block`). J'ai recopié l'ordre des enums C++ :
- `EHOLPlacementRefusal` : None 0, OutOfMap 1, Building 2, Source 3, EnemyCentre 4, NavMesh 5 (HOLConstructionRules.h).
- `HOLTechnology::EBlock` : None 0, NotImplemented 1, AlreadyResearched 2, AlreadyQueued 3, TierTooLow 4, HeroLevelTooLow 5, RequiresMissing 6, NoBuilding 7, CannotPay 8. Les positions 5 (HeroLevelTooLow) et 8 (CannotPay) sont déduites de la lecture partielle de l'enum (affichage coupé après HeroLevelTooLow) : le balayage de Describe sur blocs 0..9 les contrôle contre le C++.
- Les textes de `HOLTechnology::DescribeBlock` ("soon", "already researched", ...) sont recopiés : deuxième copie à retirer au pont.
- Piège possible : `UHOLTechnologyCheckLibrary::DescribeBlock(int)` a une AUTRE numérotation (None, AlreadyResearched, TierTooLow, ...) : ne pas l'appeler avec `Feedback.Block`.

## Tests (3, TESTS_SCRIPT_AJOUTES)
- `HeroesOfLiberty.CommandGridKeys.AgreementKeys` (`Test_CGKYAgreementKeys`) : PressKey 4096 sélections x âge choisi -1..3 x cases -1..12 ; ClickAge sur les grilles de bâtisseurs (et 1 masque sur 16 ailleurs) ; grille de la table, tous âges atteints ; GetChosenAgeAfter, 5 événements.
- `HeroesOfLiberty.CommandGridKeys.AgreementFeedback` (`Test_CGKYAgreementFeedback`) : 81 stocks x 81 coûts (HasMissing / GetMissing / CheckBuild / Blocks), CheckProduction 16 x 16 x 3 files x 4 populations (Kind, Resource, Blocks, Describe), IsPopulationBlocked, Describe sur 7 genres x 4 ressources x refus 0..6 x 3 catégories x blocs 0..9, numéros d'enum supposés (`GetPlacementRefusalBuilding`, `GetBlock*`), GetCellFace, IsCategorySelectedByCategory.
- `HeroesOfLiberty.CommandGridKeys.HardValues` (`Test_CGKYHardValues`) : valeurs en dur recopiées de `Script\Tests\HOLCommandGrid_Test.as` lignes 211-236 (retours), 842-880 (clés, âges), 773-776 et 793 (héros, unités), 153 (Marché).

## Rouge à voir (valeur à fausser, dans `HOLCommandGridKeys.as`)
1. `PressKey`, ligne `Result.ChosenAge = ChosenAge;` juste après le commentaire « An age chosen » : mettre `Result.ChosenAge = -1;` (AgreementKeys et HardValues rouges).
2. `Describe` : "queue full" -> "queue is full" (AgreementFeedback et HardValues rouges).
3. `FindMissingResource` : échanger les tests Food et Wood (AgreementFeedback et HardValues rouges).

Commande à lancer par la session principale (après synchro du binaire) : `quick_check` sur la partie script, filtre `CommandGridKeys` ; attendu : 3 tests de plus, vert ; puis un rouge parmi les trois ci-dessus.

## Champs
- BUILDS_DEMANDES: 0
- TESTS_CPP_RETIRES: 0
- TESTS_SCRIPT_AJOUTES: 3 (CommandGridKeys.AgreementKeys, .AgreementFeedback, .HardValues)
- LIGNES_CPP_SUPPRIMEES: 0
- LIGNES_MIGREES_PAR_LOT: ~180 lignes de C++ (238-418) portées en script (fichier .as ~330 lignes avec commentaires)
- TYPES_A_EXPOSER :
  1. `UHOLCommandGridReadLibrary`, nombres d'enums non UENUM, pour supprimer les copies de nombres : `static int32 GetPlacementRefusalOutOfMap()`, `...Source()`, `...EnemyCentre()`, `...NavMesh()` (seul `GetPlacementRefusalBuilding()` existe) ; `GetBlockHeroLevelTooLow()`, `GetBlockCannotPay()` (les autres `GetBlock*` existent). Ou mieux : rendre `EHOLPlacementRefusal` et `HOLTechnology::EBlock` des `UENUM(BlueprintType)` et lire le texte par `StaticEnum`.
  2. `static FString DescribeTechnologyBlock(int32 Block)` sur `UHOLCommandGridReadLibrary` (EBlock -> `HOLTechnology::DescribeBlock`, sans la table de reordonnancement de `UHOLTechnologyCheckLibrary::DescribeBlock`) pour retirer la copie des textes.
  3. Pour porter `GetCycleRank` : `static int32 GetCycleRank(bool bBuildable, int32 BuildGridSlot)` ou exposer `FHOLBuildingTypeRow::IsBuildable()` / `BuildGridSlot` au script.
  4. Pour porter `CheckResearch` : exposer `FHOLTechnologyRow` (Cost : `FHOLResources`, + champs lus par `HOLTechnology::Check`) et `HOLTechnology::Check(Row, Facts)` ; sinon il reste en C++ derrière la bibliothèque.
- PROCESSUS_DETACHES: aucun lancé

## Anomalies / NON VÉRIFIÉ
- Rien n'a été compilé : tout est non vérifié (syntaxe AS, `f"..."` avec appels de fonction dans l'accolade, `EHOLCommandFeedbackKind(Kind)` depuis un int, `FHOLResources.Food` lisible en script, `HOLCommandGridRead::IsCategorySelectedByCategory` et `GetCellFace` exposés comme lus dans le .h).
- Hypothèse du test dur : le contexte « bâtisseurs » de `CGKYMakeBuilders` (table + Barracks âge II case Q) est celui de `CGMakeBuilders` du test C++ (lignes ~806-812, lu en partie). Si `Z, not reached` est rouge en dur mais vert au balayage, l'écart vient de cette recopie, pas du port.
- Le balayage ClickAge saute les masques qui ne sont pas une grille de bâtisseurs, sauf 1 sur 16.
- Poids du test : ~100 000 appels PressKey en script x 2 ; si trop lent, réduire `Mask < 4096`.
- Fins de ligne : git signale LF -> CRLF au commit (normal ici).
