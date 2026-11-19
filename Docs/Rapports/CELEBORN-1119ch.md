# CELEBORN-1119ch : retrait du C++ couvert par les tests script (UnitTier, Selection)

NON COMPILE, aucun build, aucun quick_check. Worktree Celeborn, branche `test-world-9` (partie de integ-20 a070e50). Commit dbc7d2b (pas de push, pas de merge).

TESTS_CPP_RETIRES: 2 (HeroesOfLiberty.Production.UnitTier = FHOLUnitTierProductionTest ; HeroesOfLiberty.Buildings.Selection = FHOLBuildingSelectionTest)
LIGNES_CPP_SUPPRIMEES: 251 (git diff --stat : 221 fichier HOLUnitTierProductionTests.cpp supprime + 30 dans HOLBuildingTests.cpp ; commit : 4 files, 4 insertions de commentaires script, 255 deletions au total)
TOTAL ATTENDU: 1788 - 2 = 1786 (aucun test script ajoute ; les 2 tests script couvrants existent deja)
BUILDS_DEMANDES: 0 attendu pour ce lot (du C++ de test supprime : un build reste necessaire pour que la DLL n'enregistre plus ces tests ; lot groupe avec la session principale)
TESTS_SCRIPT_AJOUTES: 0
TESTS_TAUTOLOGIQUES: 0

## Comparaison assertion par assertion
- UnitTier : les 9 assertions C++ (the test world stands, tier 0/1 de slot, files 1/1/0/0/0 et 2/2/1/1/1, deux prix du lot 1, files 3/3/1/1/1, prix du lot 2, file Mangonel = 2) ont chacune leur equivalent dans Test_UnitTierProductionWorld (memes valeurs, memes messages) ; le script ajoute PriceLow > 0. Couvert en entier : fichier supprime (git rm). Aides du fichier (namespace Tests::UnitTier, FWorld) : aucune utilisee ailleurs (Grep Source\ : 0).
- Selection : 2 assertions, 2 equivalents dans Test_BuildingSelection. Retire (30 lignes). N'utilisait aucune aide locale ; `HOLCollision` et `Flags` restent utilises par d'autres cas du fichier.

## Ce qui RESTE en C++ et pourquoi (HOLBuildingTests.cpp, 9 tests)
- Foundation : couvert seulement en partie (bloc "another player's foundation" + accesseurs d'etat). Reste : batiment du joueur lui-meme (APlayerController local, 13 pieces de fantome, pieces non visibles, bornes, couleurs, rayon touchant le fantome, 4/13/4 pieces, hauteurs) et CanEverAffectNavigation : composants / controleur non exposes. Cas laisse ENTIER (pas d'amputation : la partie couverte est un prefixe du meme monde, retirer des assertions isolees n'enleverait presque rien).
- Damage : couvert en partie. Reste : CanEverAffectNavigation (2) et les gravats (AHOLRubble : type, duree de vie 10 s, aucun clic, compte 1) : pas d'enveloppe, et aucune enveloppe C++ neuve dans cette tache. Cas laisse ENTIER.
- Appearance, ForumColour, ArcheryRange, FarmField, FarmClick, Walkable, FarmShape : non touches (composants, couleurs, formes).
Pour retirer Damage plus tard : enveloppes CountRubbles / GetRubbleTypeName / GetRubbleLifeSpan (Buildings/HOLRubble.h) + CanEverAffectNavigation du composant de collision.

## Verifications par Grep
- Aucun fichier Source\ (.cpp/.h) ne cite `Tests::UnitTier`, `FHOLUnitTierProductionTest`, `FHOLBuildingSelectionTest`.
- Aucun script de Tools\ (quick_check.ps1, lobby_match.ps1, Tools\Tests, known_reds.txt, scenarios) ne cite `Production.UnitTier` ni `Buildings.Selection` par nom de test. Les occurrences "UnitTier" de quick_check.ps1:6809 / lobby_match.ps1:127 sont des noms de ponts de plateau (BoardBridges), sans lien avec le test C++.
- Commentaires seuls : `HOLUnitTierProduction_Test.as:1` cite encore HOLUnitTierProductionTests.cpp (fichier de SAURON, non modifie, hors tache). Commentaires de tete de HOLUnitTierWorld_Test.as et HOLBuildingWorld_Test.as mis a jour (4 lignes).

## NON VERIFIE
Rien compile ni execute : le build de l'unity C++ sans les deux tests (attendu sans effet : aucun symbole partage), le total 1786 et l'absence des deux noms dans la liste de quick_check. Rouge vu : par la session principale sur integ-19 (17:48-17:51) pour Test_UnitTierProductionWorld, Test_BuildingSelection, et les autres. Commande : `Tools\build.ps1 -LiveCoding=0 -Wait` dans le worktree Celeborn, puis `quick_check -Parts tests` (attendu 1786/1786).
PROCESSUS_DETACHES: aucun processus lance.
IN_USE.txt : occupe a l'entree (CPP: oui), libre a la sortie (non suivi par git).
