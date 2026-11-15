# Foret lente et gue (F4 / F5, GDD 8.1, reste de D33) : journal (GALADRIEL-1115e)

Releve `Docs\ReleveGDD_2026-11-15.md` ligne B4 : couvert en foret SUPPRIME (D796) ; restent F4 « foret lente pour la cavalerie et le
siege » et F5 « defense reduite au gue » : aucun code avant ce chantier. Suite de `ForestFordInProgress.md` (Haldir, F1 retire par D796).
Regles pures Angelscript seulement (`Script\Terrain\HOLTerrainRules.as` + `_Test.as`), SANS C++, aucun build. Rien n'est branche au jeu.

## Question
- `C:\Users\baudo\Documents\MyCrusader\Retours\Q-forest-ford.md` [A REPONDRE] : definition de « etre en foret » + valeurs moderees.
  Defaut code (A) : cavalerie 80 %, siege 70 % en foret ; 120 % de degats recus sur un gue ; « etre en foret » laisse a l'appelant (bool).

## Plan (etapes de 15 min)
1. Journal + question (fait).
2. Test rouge : stubs neutres (100 % / degats inchanges) + `Test_TerrainForestSlowsCavalryAndSiege`, `Test_TerrainFordWeakensDefence` ;
   commit ; `quick_check -Parts tests` VU ROUGE.
3. Vert : `ForestSpeedPercent`, `FordDamageTaken` ; valeurs dans `FHOLTerrainSettings` (une ligne chacune) ; commit vert.
4. Docs : EtapeDefini, Auto_Journey_File, ce journal ; merge main ; IN_USE libre.

## Branchement C++ qui restera (pas fait ici)
- Mouvement : le simulateur de mouvement (pas des marcheurs, serveur) multiplie la vitesse par `ForestSpeedPercent(categorie, en foret)`
  comme il le fait du marais (`FHOLTerrainRules::GetScaledSpeed`) ; copie C++ dans `Source\HeroesOfLiberty\Map\HOLTerrainRules.h`
  (+ test d'accord script/serveur comme `ServerSpeedPercent`). La categorie vient du type d'unite (cavalerie / siege / a pied).
- Combat : le calcul des degats recus multiplie par `FordDamageTaken(degats, sur un gue)` pour le defenseur.
- « Etre en foret » / « sur un gue » : selon la reponse a Q-forest-ford (zone peinte de la carte, ou arbres proches) ; D496 : sur la carte
  Les Gues les gues ne ralentissent pas (la defense reduite n'y est pas tranchee).

## Etat
- (voir le journal ci-dessous)

## Journal
- 2026-11-15 12:25 : merge main (avance rapide 2b9251c -> cdbfe27), IN_USE OCCUPE, chantier ouvert.

## Appels (un trait par appel)
||||||||||||||
