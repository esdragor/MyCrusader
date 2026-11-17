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
- 12:26 : test rouge commite bd48de5 (stubs neutres : `ForestSpeedPercent` = 100, `FordDamageTaken` = degats) ; quick_check -Parts tests
  lance 12:26 (attendu rouge : Test_TerrainForestSlowsCavalryAndSiege, Test_TerrainFordWeakensDefence).
- 12:30 : VU ROUGE : tests 1303/1305, seuls les 2 attendus (assertions « Expected true, got false »).
- 12:34 : vert : `ForestSpeedPercent` (cavalerie / siege en foret = leur ligne de reglage, a pied et hors foret 100) et
  `FordDamageTaken` (degats x FordDamageTakenPercent / 100 sur un gue, 0 reste 0) ; tests 1305/1305 PASS (186 s).
  Valeurs assertees calculees par la regle (ScaledSpeed(ForestSpeedPercent), FordDamageTaken), jamais recopiees : tautologiques 0.

## Etat (12:34)
- F4 / F5 regles pures FAITES ET VERTES (galadriel-tab, pas dans main). Rien de branche au jeu.
- Prochaine etape (C++, un autre agent, apres la reponse a Q-forest-ford) : copie C++ dans HOLTerrainRules.h + test d'accord script /
  serveur ; le simulateur de mouvement lit ForestSpeedPercent (categorie de l'unite, en foret), le combat lit FordDamageTaken ;
  definir « en foret » / « sur un gue » (zone peinte `Forest` / `Ford`, ou arbres) selon Q1.

## Appels (un trait par appel)
|||||||||||||||||||||||||||||
- 2026-11-15 (réponse de l'utilisateur, session principale, docs seulement) : D810 (Q-forest-ford) = Q1 A (zone peinte) + Q2 A (cavalerie 80 %, siège 70 %, gué 120 % ; défaut codé) ; reste le branchement C++ du mouvement et du combat (ligne [ ] Galadriel).

- 2026-11-16 21:36 (HALDIR-1116n, D836) : la vitesse marais/route est la regle C++ seule (FHOLTerrainRules) ; HOLTerrainRules.as ne garde que les reglages et ForestSpeedPercent / FordDamageTaken / StopsCharge (+ Contains pour StopsCharge) jusqu'au branchement de D810 ; le test de la foret passe par HOLTerrain::ServerScaledSpeed (C++). Commit c377240.
- 2026-11-16 22:4x (HALDIR-1116p, sans C++) : appelants verifies (Script, Source : AUCUN pour ForestSpeedPercent / FordDamageTaken / StopsCharge) ; rouge vu 22:38 (3 reglages faussés : Test_TerrainFordWeakensDefence, Test_TerrainForestSlowsCavalryAndSiege, Test_TerrainMarshStopsCharge + Test_TerrainUnknownZoneKindNeutral rouges, 1342/1348), puis vert ; question Retours\Q-terrain-regles-sans-appelant.md (brancher en D810 ou retirer).
- 2026-11-17 09:33 (HALDIR-1117d, branche maps, e6fa3a4, NON COMPILE) : D841 lot 1 : sens = regle C++ (FHOLTerrainRules) lue par Mass, script par UHOLTerrainLibrary, copie script a supprimer ; C++ pur ajoute (Forest=3, Ford=4, vitesse foret, degats gue, StopsCharge) + Terrain.Rules etendu ; reste : appelants Mass, bibliotheque, zones peintes dans common.py ; plan dans Docs\Rapports\HALDIR-1117d.md.
- 2026-11-17 10:4x (HALDIR-1117e, maps, NON COMPILE) : D841 lot 2. Commits 1ddc2ff (tests rouges Terrain.ForestSpeed : LightCavalry 80 / Ram 70 / ManAtArms 100 en foret peinte, table hors peinture et sans zone ; Terrain.FordHit : coup sur gue = 120 pour cent du coup hors gue, sans zone = hors gue) puis 046d758 (branchement : UHOLTerrainSubsystem::GetWalkerSpeedPercentAt / IsOnFord / GetFordDamageTakenPercentAt ; HOLUnitOrderProcessor GetTerrainShare(.., Constants) aux 3 appels de vitesse d'ordre avec categorie Siege / Cavalry / pied ; HOLCombatProcessor::StepTerrain pose dans Execute, GetDamageTakenPercent x pourcentage du gue). Hors zone peinte : inchange (tests hors peinture et sans zone). NON FAIT : blocs de formation (GetTerrainShare du bloc, l.~2597, part marais seul : un bloc melange pied / cavalerie n'a pas de categorie unique), peinture Forest / Ford dans common.py (regeneration des cartes seule), UHOLTerrainLibrary, suppression de HOLTerrainRules.as, StopsCharge sans appelant. Rouge a voir : build du commit 1ddc2ff seul puis quick_check -Parts tests (ForestSpeed et FordHit rouges), puis 046d758 vert.

- 2026-11-17 (HALDIR-1117f, maps, NON COMPILE) : jumeau script D836. 405638f : UHOLTerrainReadLibrary (Map/HOLTerrainReadLibrary.h/.cpp, namespace script HOLTerrainRead : enveloppes seules de FHOLTerrainRules, kinds Marsh 0 / Road 1 / Forest 3 / Ford 4, categories, defauts 60/130/80/70/120, GetForestSpeedPercent, GetWalkerSpeedPercentAt, IsInZoneOfKind, GetFordDamageTaken(Percent), StopsCharge ; zones = TArray<int32> de quadruplets Kind,X,Y,Rayon). 03279eb (ne compile qu apres build) : Script/Terrain/HOLTerrainRules.as ne garde plus aucune regle (reglages et constantes lus de la bibliotheque, fonctions = appels), tests Test_TerrainReadsTheCppRule / Test_TerrainWalkerSpeedAtPoint / Test_TerrainFordZone ajoutes (80 / 70 / 120 et kinds 3, 4 donnes en dur). RESTE : build + quick_check (rouge vu : fausser un attendu, ex. 80 -> 81, puis vert) ; verifier que les valeurs par defaut de struct par appel de fonction (HOLTerrainRead:: dans FHOLTerrainSettings) et les const globaux par appel compilent en Angelscript, sinon les passer en fonctions ; bloc de formation mixte NON touche (Retours\Q-terrain-formation.md : A plus lent membre, recommandee, ou B marais seul) ; peinture Forest/Ford dans common.py ; appelant de StopsCharge (charge lourde, absente) ; UHOLTerrainLibrary (ServerSpeedPercent...) encore utile aux tests C++ de HOLTerrainTests seulement a verifier.
- Rouge vu du lot terrain (HALDIR-1117g, 12:56, branche maps, 0 build) : 80->81 (WalkerSpeedAtPoint), 120->121 (ReadsTheCppRule), 30->31 (FordZone) = 1398/1403 ; valeurs remises = 1401/1403 vert (known_reds Nomad x2). Le 120 de FordZone n'existe pas, il est dans ReadsTheCppRule.
