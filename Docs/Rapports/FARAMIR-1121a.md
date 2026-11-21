# FARAMIR-1121a
APPELS_DECLARES: 20 (vus par moi)
HEURE_DEBUT/HEURE_FIN: 10:50:39 / 11:03:44
PREMIER_BUILD: sans C++   BUILD: sans C++   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 10420ec, assertit l'effet : oui (3 cas gate_test : sha ancetre -> PASS, sha non ancetre -> FAIL ABSENTE, faute de frappe -> FAIL inconnue) ; vu a 10:57 : 105 PASS, 2 FAIL
COMMITS: 2 (10420ec test, 6b382c8 correctif), dernier 11:03, merge_main_fait: non (DLL non concernee, sans C++ ; branche zone-effect a bce2d69)
TESTS: gate_test.ps1 complet 107 PASS / 0 FAIL (284 s) ; TESTS_TAUTOLOGIQUES: 0 ; controle reel : E-deploy.md ligne branches: main f46dc2d (...) -> PASS f46dc2d (le FAIL restant sur 'main' = le worktree Faramir est a bce2d69, main est a 3e80dfb : vrai)
ESSAIS: tache 2 non faite (BUG-order_lines_multi_blocs, scenario order_lines_blocks non cree), aucune fiche E ecrite
QUESTIONS: aucune
FAIT / RESTE: FAIT tache 1 (session_gate.ps1 bloc foreach $cb : mot non branche -> git rev-parse --verify --quiet "${cb}^{commit}", sinon FAIL inconnue ; le test d'ancetre existant s'applique ensuite ; le texte entre parentheses etait deja ignore). RESTE tache 2 (order_lines_blocks + .look depuis Tools\Scenarios\order_lines.txt, preflight -Root Faramir, PNG vus).
NON VERIFIE: tache 2 entierement ; E-virtuous_knight.md : session_gate -Essay E-virtuous_knight donne FAIL 'introuvable dans Retours' (le nom du fichier ou le -Essay est a verifier, hors tache) ; DIAGNOSTICS_AS: sans objet (aucun .as)
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Faramir
ANOMALIES_PROCESSUS: gate_test complet dure 284-405 s (premier lancement avance en tache de fond apres 180 s) ; premier Edit refuse (not read), remplace par PowerShell chaine exacte (EDITS_REFUSES: 1)
GIT_STATUS_PRINCIPALE: M AlertsMinorInProgress.md
 M Auto_Journey_File.md
 M Auto_Journey_Jour.md
 M ChatInProgress.md
 M EtapeDefini.md
 M FortificationsInProgress.md
 M GDD.md
 M JourneyInProgress.md
 M MoraleInProgress.md
 M OrdersPatrolInProgress.md
?? Docs/Concept_Faction_Theatre_2026-11-18.md
?? Docs/HSR_Aha_Allegresse_Nihilux.md
?? Docs/Priorite_Socle_Factions_2026-11-16.md
?? Docs/Rapports/CELEBORN-1121a.md
?? Docs/Rapports/ELROND-1121a.md
?? Docs/Rapports/ELROND-1121b.md
?? Docs/Rapports/ELROND-1121c.md
?? Docs/Rapports/GALADRIEL-1121a.md
?? Docs/Rapports/GALADRIEL-1121b.md
?? Docs/Rapports/GALADRIEL-1121c.md
?? Docs/Rapports/GLORFINDEL-1121a.md
?? Docs/Rapports/GLORFINDEL-1121b.md
?? Docs/Rapports/GLORFINDEL-1121c.md
?? Docs/Rapports/HALDIR-1121a.md
?? Docs/Rapports/HALDIR-1121b.md
?? Docs/Rapports/HALDIR-1121c.md
?? Docs/Rapports/SAURON-1121a.md
?? Docs/Rapports/SAURON-1121b.md
?? Retours/Q-secondary-centre.md
?? Retours/Q-towerwood.md ; inchangee hors docs voulus : oui (EtapeDefini, JourneyInProgress, Auto_Journey_File)