# GALADRIEL-1117j : Sites (MIG-J-Site, reste du C++)
Worktree HeroesOfLiberty-Galadriel, branche galadriel-tab ; commits ac4ca24 (test script), 8a9ac55 (pont d'or), suivi (correctif d'un caractere). NON compile.

## Decisions par fonction
- GoldStep : migre. Appele une fois par joueur et par pas, mais le compte de slots est minime, et le pont appelle deja le script par site et par pas. Nouvel evenement UHOLSiteBoardSubsystem::GoldStepRule (report en entree/sortie, defaut C++ inerte comme StepSiteRule), script HOLSiteVictory::GoldStepWith. HOLSite::GoldStep C++ supprime (-9 lignes .cpp, -6 .h). Pas de test C++ de GoldStep a retirer ; le test C++ FHOLSiteSubsystemGoldTest (chaine reelle) reste.
- ResetCounts, AddUnit : GARDES en C++. AddUnit est appele par unite et par site dans le parcours Mass (ForEachLivingUnitWithType) : par pas, par entite. Un aller-retour script par unite serait le cout que D836 reserve au C++.
- MakeView : garde (vue repliquee C++, hors enonce).
- HOLSiteRulesLibrary : N'EST PAS un jumeau pur. GetVictoryCount lit l'etat de partie pour l'interface (HOLGameScreenScript.as:945) : liaison, gardee. GetHoldSeconds/GetDecaySteps ne servent qu'au test d'accord Test_SiteConstantsAgreeWithTheSimulation : a retirer avec la lecture des constantes par le pont (voir plus bas).
- Constantes C++ (DefaultHoldSeconds 60, DefaultDecaySteps 2, DefaultGoldPerMinute 30, DefaultVictorySeconds 600) : gardees, valeurs de reglage de UPROPERTY config de HOLSiteSubsystem ; deux copies donc test d'accord conserve. Les retirer demande que le C++ lise le script au chargement (hors du pont par pas) : a trancher, non fait.

## Tests
- Script ajoute : Test_SiteGoldStepWithTheCallersCarry (HOLSiteVictory_Test.as). Rouge = non compile tant que GoldStepWith n'existe pas (commit ac4ca24 seul) ; vert attendu apres le build (a VOIR).
- C++ : HOLSiteBridgeTests.cpp ServerBoard etendu (gold avec board, sans board). Aucun test C++ retire.
