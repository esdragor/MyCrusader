# FARAMIR-1124d (2026-11-24 11:11 -> 11:17)
APPELS_DECLARES: 11 (arret a 25 vus ou 11:27)
HEURE_DEBUT/HEURE_FIN: 11:11:44 / 11:17
PREMIER_BUILD: sans C++   BUILD: sans objet   LAUNCHCHECK_DEBUT: sans C++ (Get-StaleBinaryProblem : sortie vide, binaire a jour)
TEST_ROUGE: aucun, tache sans objet : la regle (0d81744) et le lecteur (3974596) sont DEJA dans main ; assertit l'effet : sans objet   TEST_ROUGE_SEPARE: non (rien a corriger)
COMMITS: 0 (aucun code), merge_main_fait: oui (Already up to date)
TESTS: preflight ai_sacred_light PASS (60 s), "no warning or error in the log" ; 0 erreur AS au chargement ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: ai_sacred_light : preflight PASS, PNG _after ouvert (zone doree sur le combat), PNG _before non ouvert ; LOGIQUE SEULEMENT (pas rejoue en essai) ; commande : play.ps1 ai_sacred_light -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir
QUESTIONS: aucune
FAIT / RESTE: la ligne 34 de l'Auto_Journey_File etait perimee (liaison C++ = 1123pb, defaut HeroPlace=(0,0,0) = 1123pc, corrige par GLORFINDEL-1123pd : HOLFactionPowerServer::HeroPlaceOf -> HOLHero::GetHeroLocation, lu en HOLAIBrain.as:164-170 ; tous dans main). Ligne passee [x]. RESTE : abe5629 (GLORFINDEL-1124a, lecture au pas de simulation, client garde) ABSENT de main ; la position de la banniere plantee n'est pas relue.
NON VERIFIE: position exacte de la banniere plantee par l'IA (le scenario ne logue ni "banner" ni "Sacred Light cast", hol.AI.LogPowers non active) ; seule la zone doree au PNG _after prouve qu'un cast a eu lieu.
PROCESSUS_DETACHES: voir dernier appel
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: docs seulement (Auto_Journey_File.md, AubeInProgress.md edites par edit_line) : oui