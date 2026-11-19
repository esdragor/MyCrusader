# HALDIR-1119cd : lot L3b (bibliotheque de lecture de la grille, tests d'accord retires)

NON COMPILE, NON TESTE (aucun build, aucun quick_check, par consigne de la session principale).

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`, branche `grid-l3b` (depart ae64d0e). Commit : `80f6340` (amende une fois, voir anomalies). `Content\Maps\Crown.umap` non touche. Pas de merge main, pas de push.

APPELS_DECLARES: ~28 vus par moi (la limite de 25 est depassee de 3 : un decoupage mal compte, plus une correction d'un bug de mon propre script). HEURE_FIN: 17:12 (arret dur 17:22 respecte).
BUILDS_DEMANDES: 0 (la session principale compile : un seul build avec L3a).

## Fonctions `HOLCommandGridRead::` SUPPRIMEES (19), `.cpp` et `.h`
Transferts purs vers un corps `HOLCommandGrid::*Native` :
GetCommand, GetOrder, PressKey, ClickAge, GetChosenAgeAfter, CheckProduction, CheckBuild, Blocks, Describe, IsRepeatPress, NextInCycle, IsAloneInGroup, SplitSubgroups, GetCommandPriority, MergeCommands, PickEvenly, IsAgeOpen.
Supprimees en plus (leur seul corps etait un `*Native` et leur seul usage un test d'accord ou une assertion remplacee) : IsTypeSelectedByCategory, IsCategorySelectedByCategory.

## Symboles `*Native` que `HOLCommandGridReadLibrary.cpp` ne cite plus (Grep `Native` : 0 resultat)
GetCommandNative, GetOrderNative, IsAgeReachedNative, GetAgeBuildingNative, PressKeyNative, ClickAgeNative, GetChosenAgeAfterNative, GetCycleRankNative, IsSelectedByCategoryNative, CheckProductionNative, CheckBuildNative, BlocksNative, DescribeNative, IsRepeatPressNative, NextInCycleNative, IsAloneInGroupNative, SplitSubgroupsNative, GetCommandPriorityNative, MergeCommandsNative, PickEvenlyNative, IsAgeOpenNative.
Aucun `*Native` n'est cite ailleurs par moi : L3a peut tous les supprimer sans casser la bibliotheque (a verifier au build).

## Fonctions GARDEES (et pourquoi)
- Lectures de table sans jumeau script : CheckResearch, GetCycleRankOfType, IsTypeInTable, GetTypeCategory, WithTableBuildables, GetProducesOf, GetBuildingResearches, IsTechnologyInTable, IsTechnologyImplemented, GetTechnologyFoodCost, GetBlock*, GetHeroAbility*, GetPlacementRefusalBuilding.
- Constantes et lectures non-Native (appel de fonctions publiques de HOLCommandGrid qui survivent) : GetNumCells, GetCancelCell, GetGridColumns, GetCategoryRow, GetTradeCell/Gain/GainText, IsBuildGrid, HasMissingResource, GetMissingResource, IsPopulationBlocked, GetCellFace, GetAttackMoveCell, GetStopCell, GetHoldCell, GetDeployCell, GetResearchFirstCell, GetNumAges, GetHeroAbilityCell, GetRageCell, GetSpecializationCell, GetReportName, GetAgePanelOpacity, GetMouseCursor, GetCursorCrosshairs, GetCursorDefault. Ces comparaisons script vs C++ restent dans `Test_CGRAgreementTables` et `Test_CGKYAgreementFeedback` (constantes, pas de jumeau a retirer).
- Appelees par `Script\UI\HOLAgePanels.as` (hors tests) : IsBuildGrid, IsAgeReached, GetAgeBuilding, GetAgePanelOpacity ; par `Script\Player\HOLCommandGridRules.as` : GetSpecializationCell. Aucun autre appel hors tests dans `Script\`.
- `Source\` : `HOLCommandGridBoard.cpp` et `HOLScriptScreen.cpp/.h` n'utilisent que le namespace prive `HOLCommandGridReadLibraryPrivate` (ToRule/FromRule) et les USTRUCT des vues : tous gardes, intacts.

## Deux fonctions gardees mais REORIENTEES (a cause du retrait de L3a)
- `IsAgeReached` et `GetAgeBuilding` (appelees par `HOLAgePanels.as`, donc a garder) ne transferent plus a `IsAgeReachedNative` / `GetAgeBuildingNative` : elles appellent maintenant les enveloppes publiques `HOLCommandGrid::IsAgeReached` / `HOLCommandGrid::GetAgeBuilding` (qui interrogent le plateau script). Effet joueur : les panneaux d'age lisent la regle par le plateau script, comme la grille (meme source). Risque : si un monde n'a pas de plateau, les panneaux sont vides comme la grille (couvert par la preuve L4 de L3a).
- `GetCycleRankOfType` (gardee, demandee par la session principale) ne peut pas appeler `GetCycleRankNative` (retire par L3a) ni l'enveloppe (plateau absent dans un test script) : corps inline `Type->IsBuildable() ? Type->BuildGridSlot + 1 : 0` (la formule exacte du natif, HOLCommandGrid.cpp:488). Cela recopie une regle d'une ligne en C++ (jumeau de `HOLCommandGridGroups::GetCycleRank`) : a trancher par la session principale (garder, ou exposer IsBuildable/BuildGridSlot et faire calculer le rang au script dans `Test_CGCycle`).

## Tests script RETIRES (5, noms)
- `Test_CGRAgreementSelections` (HOLCommandGridRules_Test.as) : balayage 4096 selections x cellules, GetCommand/GetOrder/GetReportName/IsBuildGrid/IsAgeReached/IsAgeOpen/GetAgePanelOpacity/GetAgeBuilding script vs natif.
- `Test_CGGSubgroupsSweep`, `Test_CGGMergeSweep`, `Test_CGGDistributionSweep` (HOLCommandGridGroups_Test.as) : SplitSubgroups, GetCommandPriority, IsAloneInGroup, MergeCommands, PickEvenly script vs natif.
- `Test_CGKYAgreementKeys` (HOLCommandGridKeys_Test.as) : PressKey, ClickAge, GetChosenAgeAfter script vs natif.
Aides de test devenues sans appel et retirees : CGRMakeContext, CGRSame, CGRDescribe, CGGSameCommand, CGGSameInts, CGGMakeMembers, CGKYSameKey, CGKYDescribeKey, CGKYMakeContext, CGKYMakeBuilders.

## Tests TRANSFORMES ou REDUITS
- `Test_CGGCycleSweep` -> `Test_CGGFaceAndCategory` : le balayage NextInCycle/IsRepeatPress est retire ; reste la comparaison GetCellFace script vs C++ (fonction gardee) ; le croisement F1/F2/F3 devient une valeur en clair (une touche de categorie ne prend que sa propre categorie : `bSelected == (A == B)`, 9 cas).
- `Test_CGKYAgreementFeedback` : reduit aux parties non-Native (ressource manquante 81x81, IsPopulationBlocked, nombres que le script suppose, GetCellFace). Retire de ce test : CheckBuild, CheckProduction, Blocks, Describe (balayages natifs), IsCategorySelectedByCategory.
- `Test_CGRAgreementTables` : les deux balayages natifs (GetAgeBuilding par age atteint ; GetCommand par type de la table) retires ; restent les echanges, rangees de categorie, cellules heros, rage, curseur, constantes.
- `Test_CGBIsAloneInGroup`, `Test_CGBIsRepeatPress`, `Test_CGBGetCommandPriority`, `Test_CGBIsSelectedByCategory`, `Test_CGBBlocks` (Board_Test) : seules les lignes « the board and the C++ agree » (et le bloc `Native` de GetCommandPriority) retirees ; toutes les valeurs en clair gardees.
- `Test_CGCategories` (HOLCommandGrid_Test.as) : `IsTypeSelectedByCategory(TypeName, X)` remplace par `HOLCommandGridGroups::IsCategorySelectedByCategory(HOLCommandGridRead::GetTypeCategory(TypeName), X)` (meme verite, par la regle script, categorie lue dans la table).
- Commentaires d'en-tete des 5 fichiers de test mis a jour.

## Total de tests attendu : 37 -> 32 (Board 6, Groups 5->2, Keys 5->4, Rules 6->5, Grid 15). `quick_check` baisse donc de 5 (decision de la session principale : une seule source).
Verifie par Grep apres edition : 6 + 2 + 4 + 5 + 15 = 32 `void Test_*`. Il faut peut-etre corriger un compte attendu dans `Tools\Tests` ou un journal (Grep des noms retires dans `Tools\` : 0 ; seuls des anciens rapports `Docs\Rapports\{ELROND-1119x,HALDIR-1119y,SAURON-1119t,SAURON-1119x}.md` les citent).

## Lignes
C++ : `HOLCommandGridReadLibrary.cpp` 444 -> 320 (-124), `.h` 469 -> 427 (-42) : -166 (le plan en estimait -430 : la bibliotheque garde surtout des constantes et des lectures de table).
Script : -768 net sur 5 fichiers de test (29 inserees, 797 supprimees).

## Couverture perdue (franchise)
Aucune valeur en clair n'a ete retiree. Mais trois balayages n'avaient AUCUN equivalent en clair et ne sont plus verifies : (1) `Describe` sur toutes les combinaisons kind/ressource/refus/categorie/bloc (seul « not enough Wood » reste en clair dans `Test_CGKYHardValues`) ; (2) la regle script `GetCommand` sur chaque batiment reel de la table (Produces/Researches) ; (3) `PressKey`/`ClickAge` sur 4096 selections. Les valeurs que rend la regle script pour ces cas n'etant pas lues ici, je n'ai pas invente de valeurs en clair : a decider (lot de tests de valeurs, ou accepter).

## NON VERIFIE (rien n'est compile)
- Compilation C++ du .cpp/.h : suppression par motif de lignes ; il faut verifier qu'aucune declaration `UFUNCTION` orpheline ne reste (le Grep de verification n'a trouve aucun `HOLCommandGridRead::` manquant dans `Script\`, et toutes les fonctions citees par les scripts ont declaration et definition).
- Helpers du namespace prive (`ToKeys`, `ToRule(Members)`, conversions de listes) sans doute inutilises maintenant : peut produire un avertissement, pas une erreur (a retirer si le build les signale).
- `IsAgeReached` / `GetAgeBuilding` via l'enveloppe : suppose que ces enveloppes gardent leurs signatures `(const FHOLGridContext&, int32...)` apres L3a (le plan dit « signatures publiques inchangees »).
- Scripts : aucun test script lance. Les nouveaux tests (`Test_CGGFaceAndCategory`, `Test_CGKYAgreementFeedback` reduit, `Test_CGCategories` modifie) n'ont pas de rouge vu. Rouge a voir : fausser `GetCellFace` dans `HOLCommandGridGroups.as`, ou la ligne « F2 takes it » (attendre Military).
- Hypothese a verifier : `HOLCommandGridGroups::IsCategorySelectedByCategory(A, B)` vaut `A == B` pour les trois categories (vue dans les tests de Board : Economic/Economic vrai, Military/Economic faux, Economic/Military faux ; les 9 cas ne sont lus nulle part).
- Si L3a retire aussi `HOLCommandGrid::FindMissingResource`, `IsPopulationBlocked`, `GetCellFace` (ils ne sont pas dans la liste `*Native` du plan), la bibliotheque ne compilerait plus.

## Commande a lancer (session principale, un seul build avec L3a)
`C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\build.ps1 -LiveCoding=0 -Wait` puis `quick_check -Parts tests` (total attendu 32 tests de grille dans ces 5 fichiers).

## Anomalies
- Mon script d'edition a eu un bug (la variable `$e` ecrasait `$E`, fin de ligne) : `HOLCommandGridRules_Test.as` a ete ecrit sur une seule ligne dans le premier commit ; reparee et commit amende (non publie) avant de rendre. L'etat final est coherent (253 lignes, 5 tests, fins de ligne CRLF). Le seul fichier concerne.
- Git a signale « LF will be replaced by CRLF » pour `HOLCommandGridGroups_Test.as` (fins de ligne mixtes d'origine, non touchees par moi) : lancer `eol_check.ps1 -Fix` si besoin.
- PROCESSUS_DETACHES: aucun lance. ANOMALIES_PROCESSUS: aucune. GIT_STATUS_PRINCIPALE: non touchee par moi (hors le present rapport).
