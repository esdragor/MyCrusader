# DIAG-1123pa (2026-11-23, 15:3x)

- CAUSE : attente PERIMEE de quick_check.ps1 (pas un defaut du jeu). Partie `build hud`, check ligne 4273 « tooltips: name, cost missing, effect » : le texte obtenu contient `I C: Porte de palissade | W 10 missing` (apres `I X: Sanctuaire de l'Aube | W 250 missing`) et `II F: Mur de pierre | S 10 | Palier non atteint` (apres `II D: Ecurie`), absents de l'attente. Les checks voisins (4268 grille, 4271 cases, 4513 Paysans) etaient deja a jour (C=PalisadeGate, F=StoneWall).
- VERIFIE : libelles et couts = BuildingTypes.csv lignes 19-20 (PalisadeGate Wood 10, slot 10, age 0 ; StoneWall Stone 10, slot 7, age 1) ; touches C (age I) et F (age II) sans doublon dans leur palier (age I : A Z E R Q S D F W X C ; age II : A Z Q S D F W) ; coherent avec HOLCommandGrid_Test.as:524-551.
- CORRECTIF : b7186d1 sur integ-58 (quick_check.ps1, 1 ligne, BOM garde, ParseFile 0 erreur).
- REJEU : `quick_check.ps1 -Parts build` = PASS in 196 s (aucune ligne FAIL).
- NON REJOUE : `-Parts tests` (le changement ne touche que le regex de la partie hud ; 2154/2154 deja PASS avant). Commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Tests\quick_check.ps1 -Parts tests
- NET : `net` n'etait pas rouge au constat ; non rejoue ici.
- Aucun C++, aucun build, rien fusionne dans main.
- Branches : le CSV et la grille viennent de a2b4208 / d9ec2f0 (branche ability-columns / integ-55), l'attente du hud n'avait pas ete mise a jour par elles.