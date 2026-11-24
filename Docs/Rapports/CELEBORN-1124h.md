# CELEBORN-1124h : compilation de G4 corrigee, test grille fine vert (integ-41)

Impact joueur : aucun a SubDivision 1 (defaut) ; avec hol.Wall.SubDivision 4 les pieces de mur se posent sur la grille de 1 m (code G4 compile et teste).

APPELS_DECLARES: 16 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 12:00 / ~12:12
PREMIER_BUILD: 12:00   BUILD: PASS (39 s, file derriere Sauron)   LAUNCHCHECK_DEBUT: binaire perime (dll 2026-11-23T17:52 < HOLConstructionSubsystem.cpp), build lance d'abord
TEST_ROUGE: sans objet (correction de compilation ; rouge de G4 = aa1bbfe, voir CELEBORN-1124g)   TEST_ROUGE_SEPARE: sans objet
COMMITS: 1 dans cette seance (d8d2939, 12:07, test seul) ; la correction C++ de compilation etait deja le commit 20d4888 de 1124g (tip reel de integ-41 = d8d2939, pas 405e578) ; merge_main_fait: non (non demande)
TESTS: quick_check -Parts tests du worktree : 1er passage 2205/2206 (Test_Construction_WallPieceSubGrid rouge) ; apres d8d2939 : 2206/2206 PASS, 220 s ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune

FAIT :
- Erreur de build lue : C2039 'TypeName' absent de FHOLBuildingTypeRow (HOLConstructionSubsystem.cpp:77). Deja corrigee par 20d4888 (SnapFootprint compare `&Type` au pointeur `UHOLBuildingSettings::FindType("Palisade"|"StoneWall")`, FindType renvoie `const FHOLBuildingTypeRow*`, HOLBuildingTypes.h:302). Aucun changement C++ de ma part.
- `Tools\build.ps1 -LiveCoding=0 -Wait` : PASS.
- 1er quick_check : le seul rouge etait Test_Construction_WallPieceSubGrid, assertion "lower corner on the 1 m grid, (100, 0)". Cause = le TEST, pas la regle : `ConTestTile` vaut 300 (HOLConstruction_Test.as) alors que le test suppose la maille reelle de 4 m (400). Avec 300 : grille 75, coin (75, 0). SnapWallPiece est correct.
- d8d2939 : le test passe 400.0 comme maille (pieces a SubDivision 4 et 1) et attend (800, -400) pour la maison 600x600 a SubDivision 1 ((1234,-60) : coin le plus proche sur la grille de 4 m). 2e quick_check : 2206/2206.

RESTE : voir CELEBORN-1124g (GetWallLineWorldAtPitch sans test, fantomes client G8, IsWallPieceType duplique, PlanFromRequestFine / ExpandOccupied sans appelant en jeu) ; scenario wall_line_long a SubDivision 4 non joue.

RELECTURE_CPP : aucun C++ neuf ici. 20d4888 : comparaison de pointeur `&Type == FindType(...)` valide car l'appelant passe la ligne de la table (non une copie) ; FindType peut renvoyer nullptr (alors faux, pas de crash) ; FName construit a chaque appel (par pose / fantome, pas par entite et par pas). A confirmer : qu'aucun appelant de SnapFootprint ne passe une copie de la ligne (non verifie : sinon SubDivision > 1 retomberait sur la grille de 4 m sans erreur).
DIAGNOSTICS_AS: non consulte (le test a charge et passe au quick_check)

NON VERIFIE: effet a SubDivision 4 en jeu (wall_line_long, bois = 5 x pieces) ; cvar `hol.Wall.SubDivision` lue par GetWallSubDivision sous ce nom exact ; appelants de SnapFootprint passant la ligne de table par adresse ; merge main.
PROCESSUS_DETACHES: voir derniere ligne ci-dessous
ANOMALIES_PROCESSUS: quick_check et build passes en tache de fond apres 180 s (comportement de l'outil), laisses finir ; build file derriere Sauron (aucun arret).
GIT_STATUS_PRINCIPALE: docs seuls modifies (ce rapport) ; autres entrees = autres agents : oui
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
