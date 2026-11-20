# SAURON-1120g (neutrals_clairiere, sans C++)
APPELS_DECLARES: 14 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 11:06:11 / 11:10:10
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scenario, pas de regle) ; scenario_mapsize_check PASS
COMMITS: 1, a3c3ce5 vers 11:09, merge integ-27 fait en premier geste (branche tool-known-reds)
TESTS: scenario_mapsize_check PASS ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: neutrals_clairiere : preflight NON LANCE (consigne : aucun preflight), aucun PNG : LOGIQUE SEULEMENT ; commande a lancer apres preflight : play.ps1 neutrals_clairiere -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron (le worktree n'a pas de binaire : jouer depuis un worktree compile contenant a3c3ce5)
QUESTIONS: aucune. Retours\Q-neutrals-camp-cache.md est INTROUVABLE (ni Retours de main ni du worktree) ; seule trace : Docs\Archive\Retours_2026-11-10\Q-neutrals-trees-decor.md (option A = ClearNear, faite) ; defaut applique = neutrals inchange
FAIT: Tools\Scenarios\neutrals_clairiere.txt/.look/.json (Plain, mapSize Small, 1 joueur sans paysans ni heros, camp passif Aggro 1 au centre, 3 Hommes d'armes a -3500, hol.Decor.ClearNear 0 0 2500, captures avant / large / gros plan, note, Sanity, quit). Commande verifiee dans Source\HeroesOfLiberty\Dev\HOLDecorClear.cpp:127 (hol.Decor.ClearNear X Y [R], defaut 2000 cm, retire troncs/cimes de Plain + instances du decor, jamais les sources) ; log attendu 'Decor.ClearNear X Y R: took'.
RESTE: preflight PASS, PNG ouverts, fiche E-neutrals-clairiere (GESTE interne, SON non entendu).
NON VERIFIE: tout l'execution : le cadrage (Focus -1700 0 / Zoom -3 puis Focus 0 0 / Zoom 6, Zoom suppose relatif comme dans neutrals), la presence d'un bosquet au centre d'une Plain mapSize Small (si elle n'en a pas, avant == apres), position du camp (0,0) vs base du joueur en Small, texte exact 'Camp 0 at (0, 0): guarding, members 3 of 3, hits taken 0'. Manque : l'objet 'tente, feu' n'existe pas (Camp.Add = 3 Lanciers seulement), le .look ne le promet donc pas. Observation : neutrals.txt appelle deja ClearNear 0 0 2500 pour son 2e camp (0,0) ; seul le camp 1 (0,-9000) reste dans le bosquet.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Sauron

ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: docs seulement (voir ci-dessous), inchange hors docs voulus : oui
 M AubeInProgress.md
 M Auto_Journey_File.md
 M EtapeDefini.md
 M FortificationsInProgress.md
 M MoraleInProgress.md
 M NeutralsInProgress.md
 M SpecializationInProgress.md
?? Docs/Concept_Faction_Theatre_2026-11-18.md
?? Docs/HSR_Aha_Allegresse_Nihilux.md
?? Docs/Priorite_Socle_Factions_2026-11-16.md
?? Docs/Rapports/ELROND-1120e.md
?? Docs/Rapports/ELROND-1120f.md
?? Docs/Rapports/FARAMIR-1120e.md
?? Docs/Rapports/GALADRIEL-1120f.md
?? Docs/Rapports/GLORFINDEL-1120f.md
?? Docs/Rapports/HALDIR-1120h.md
?? Docs/Rapports/PLAN-MORAL-1120.md
?? Retours/Q-moral-valeurs.md
?? Retours/Q-mur-ligne-longueur.md
?? Retours/Q-specialization-troisieme-case.md
