# ELROND-1119x : correction du rouge Test_CGKYHardValues (G2a)

Debut 12:35:25, worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, branche `fix-g2a` (cherry-pick e8af0df = 05625c3).

## Reproduction (12:35 - 12:39)
`quick_check -Parts tests` : 1726/1727 (1724 de integ-15 + 3 tests du lot G2a). Seul rouge : `HOLCommandGridKeys_Test.Test_CGKYHardValues`.
Les deux assertions qui tombent (`Saved_Tests\Logs\quick_tests.log`, apres `Sending RunTest Test_CGKYHardValues`) :
1. `II, Q: the barracks` (PressKey(Later, 1, 4) : attendu Build / age II / Barracks)
2. `II, S (empty): nothing` (PressKey(Later, 1, 5) : attendu None / age II)
Ce ne sont PAS les numeros d'enum EBlock (HeroLevelTooLow 5, CannotPay 8) : les tests Describe / Blocks / Feedback passent et le test d'accord les couvre (les 7 `GetBlock*` sont comparees au C++, vertes).

## Verdict par assertion : (a) le TEST se trompe, la regle script est juste
Cause : le contexte des cles en dur `CGKYMakeBuilders()` n'est PAS celui du test de reference. Il appelle `HOLCommandGridRead::WithTableBuildables` (toute la table reelle des batiments : d'autres batiments d'age II occupent des cellules), alors que la source de verite construit un contexte explicite :
- `HOLCommandGrid_Test.as:9-23` (`CGMakeBuilders`) : House cell 0 / LumberCamp 1 / MiningCamp 2, tous age 0, rien d'autre ;
- `HOLCommandGrid_Test.as:806-810` (`Test_CGAges`) : `bOwnUnits = true` puis Barracks cell 4 age 1.
Les valeurs attendues recopiees (842-866) sont celles du C++/du test existant ; elles ne changent pas.
Preuve que la regle n'est pas en cause : `Test_CGKYAgreementKeys` balaie PressKey/ClickAge script contre `HOLCommandGridRead::` (C++) sur ce meme contexte table (tous ReachedAge, tous ChosenAge, toutes cellules) : VERT. Script = C++ sur le contexte table ; seul le contexte du test dur etait faux. Preuve de la correction : avec le contexte exact de `CGMakeBuilders`+`Test_CGAges`, les 2 assertions passent sans toucher a la regle ni aux valeurs attendues.
Aucune regle corrigee, donc pas de cas ajoute a l'accord (le balayage couvre deja le contexte table ; le contexte explicite est couvert par les valeurs dures).

## Correction (commit d0649ab sur fix-g2a)
`Script\Tests\HOLCommandGridKeys_Test.as` : nouvelle `CGKYMakeHardBuilders()` (recopie exacte des deux sources ci-dessus), utilisee par `Test_CGKYHardValues` ; `CGKYMakeBuilders()` (table) reste pour le balayage d'accord.
Vert : 12:48:53, tests 1727/1727 PASS (197 s).

## Rouge vu (HOLCommandGridKeys.as, PressKey ligne 73 : `Result.ChosenAge = ChosenAge;` -> `-1`, branche « un age choisi »)
12:49:15 -> 12:52:10 : 1725/1727, tombent `Test_CGKYAgreementKeys` ET `Test_CGKYHardValues` (3 assertions).
Restauration par `git checkout -- Script/Player/HOLCommandGridKeys.as` (git status propre). Relance de verification : voir fin de ce fichier.

## NON VERIFIE / anomalies
- Je n'ai pas relu la table reelle pour lister quels batiments d'age II occupent les cellules Q et S (le diagnostic repose sur : table != contexte explicite, accord script/C++ vert sur la table, vert apres changement de contexte).
- Un `quick_check` a ete lance par erreur (12:40, sur le fichier non encore corrige : un script python avait echoue sur une erreur de syntaxe) : 1726/1727 a nouveau, 270 s, sans effet.
- Aucun build C++, `EtapeDefini.md` et `Auto_Journey_File.md` non touches. Pas de merge main, pas de push : branche `fix-g2a` prete.

Relance apres restauration : 12:55:31, tests 1727/1727 PASS (175 s), git status propre.
