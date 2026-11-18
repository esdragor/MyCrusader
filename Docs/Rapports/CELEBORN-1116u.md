# CELEBORN-1116u : S1e partie 2, exposition de CanGarrison, DoorsOpenFor, AlliedVictory, InMatch (branche hero-j8-close, commit 0516664)

BUILDS_DEMANDES: 0 (la session principale compile)
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 3 (Test_DiplomacyGarrisonDoorsReadLibrarySmoke, Test_DiplomacyAlliedVictoryReadLibrarySmoke, Test_DiplomacyInMatchReadLibrarySmoke, dans Script\Tests\HOLS1eLibraries_Test.as ; compilent apres le build seulement)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: 0 (le retrait des copies script suit le build)
TYPES_A_EXPOSER: aucun de plus
TEST_ROUGE: non applicable (enveloppes sans logique ; la preuve est le test de redirection existant, rouge/vert apres build)

## Fait
- C++ (motif : liaison a exposer, D836 point 5) : HOLDiplomacyReadLibrary.h/.cpp : fiche USTRUCT FHOLDiplomacyAlliedVictory (bEnded, Winners), fonctions InMatch(Book, Slot), CanGarrison(UnitOwner, BuildingOwner), DoorsOpenFor(Book, Owner, Other), AlliedVictory(Book, Defeated). Enveloppes de HOLDiplomacy::.
- Redirection script ecrite, NON commitee, dans Docs\Rapports\CELEBORN-1116u_script_apres_build.patch (git apply --check OK sur le worktree) : tous les tests de HOLDiplomacyRules_Test.as passent sur la bibliotheque (FHOLDiplomacyStances, LibHumansBook...), les 3 anciens helpers sur FHOLDiplomacyBook sont retires ; HOLSignalRules.as appelle HOLDiplomacyRead::InMatch et perd sa copie d'une ligne.

## SUITE EXACTE APRES BUILD (session principale)
1. Tools\build.ps1 -LiveCoding=0 -Wait sur le worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, puis quick_check -Parts tests (les 3 tests de fumee de HOLS1eLibraries_Test.as doivent etre verts).
2. git apply C:\Users\baudo\Documents\MyCrusader\Docs\Rapports\CELEBORN-1116u_script_apres_build.patch, quick_check tests (rouge VU : fausser une valeur de Test_DiplomacyAlliedVictory une fois, puis vert), commit.
3. git rm Script\Diplomacy\HOLDiplomacyRules.as SEUL. ATTENTION : NE PAS git rm HOLDiplomacyRules_Test.as, contrairement a l'enonce : apres le patch c'est lui qui porte les tests de garde, portes, victoire alliee ; le supprimer perdrait ces tests (renommer en Script\Tests\HOLDiplomacyRead_Test.as si voulu). FHOLDiplomacyBook, FHOLStanceChange, FHOLVisionChange, FHOLAlliedVictory disparaissent avec Rules.as : verifier par grep qu'aucun .as ne les cite (apres le patch il n'en reste aucun dans les tests).
4. Mettre a jour la ligne MIG-C++-FINAL point 1 (a terminer) et Placeholders.txt non concerne.

## Notes
- Pas de build, pas de quick_check ici. Le worktree sans patch ne casse pas : le script n'appelle rien de neuf hors HOLS1eLibraries_Test.as (qui, comme pour S1e partie 1, ne compile qu'apres le build).
- Les 16 fichiers Tools\Scenarios\c28_*, c31_* non suivis n'ont pas ete touches.
- Anomalie : la file disait « git rm les deux fichiers » ; voir point 3 ci-dessus.