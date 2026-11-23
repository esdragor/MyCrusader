# ELROND-1123h (2026-11-23, integ-25, sans nouveau C++)

Joueur : une tour de l'IA touchee depuis moins de 5 s appelle desormais ses archers pres d'elle (garnison sur coup) ; reste a le voir en partie.

APPELS_DECLARES: ~30 vus (depasse les 22 de l'enonce : 2 quick_check bloquants + attentes de notification ; heure respectee)
HEURE_DEBUT/HEURE_FIN: 10:47 / 11:00 (Get-Date PowerShell)
PREMIER_BUILD: sans C++   BUILD: sans C++ (binaire de ELROND-1123g, PASS 10:45)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 880373a (Test_AIGarrisonCallReadsSecondsSinceHit), vu a 10:55 : tests 2107/2108 ; assertit l'effet : oui (nombre d'appels et ids des archers qui entrent) ; TEST_ROUGE_SEPARE: oui (880373a test seul, correctif 10db129)
COMMITS: 2 (880373a, 10db129), dernier 10:56, merge_main_fait: non (hors tache ; integ-25 n'a pas fusionne main)
TESTS: (1) -Parts tests a 10:47-10:50 sur 476b896 : 2107/2107 PASS, Player.CommandGrid.SelectionKinds, AI.Summary.GarrisonKindCavalry, AI.Summary.SecondsSinceHit tous Success, 0 erreur Angelscript (-as-exit-on-error). (2) rouge 2107/2108 a 10:55 ; vert 2108/2108 a 10:59 sur 10db129. aiserver : NON LANCE (moins de 5 min restantes). TESTS_TAUTOLOGIQUES: 0 (la valeur assertie sort de PlanFromSummary -> TowerCalls -> Entrants -> AdmittedWith, secondes d'une constante du CVar + 1)
ESSAIS: aucun
QUESTIONS: aucune creee. N = valeur existante `hol.AI.GarrisonAttackedSeconds` (5 s, HOLAIGarrisonRules.as:16). Note de conception : sans lecteur du PREMIER coup, le delai de reaction du niveau (ReactSeconds : Normal 3 s, Hard 1 s) est compte comme ecoule (SinceFirstHit = max(SinceLast, ReactSeconds)) : Hard ne reagit donc pas plus tot que Normal tant qu'un lecteur du premier coup n'existe pas. Une ligne a changer dans HOLAIGarrisonCall.as ; a mettre en question IA si l'utilisateur veut le delai.
FAIT: S4-b lecture en jeu : HOLAIGarrisonCall::PlanFromSummary lit Building.SecondsSinceHit quand SinceFirstHits/SinceLastHits ne donnent rien (10db129) ; HOLAIBrain.ThinkGarrison passe des tableaux vides, donc la regle est branchee de bout en bout (resume -> tour attaquee -> IssueAIGarrisonOrder).
RESTE: voir la garnison en partie (essai IA avec une tour touchee) ; cellule LightSeal dans HOLCommandGridRules.as ; commentaire de HOLAIBrain.as:80-81 encore « aucune duree de coup » (a mettre a jour) ; aiserver.
NON VERIFIE: la regle en partie reelle (test sur donnees du resume, pas sur un vrai coup serveur) ; aiserver ; la boucle du controleur de B2 ; DIAGNOSTICS_AS: 0 erreur (compilation des scripts du run de tests, serveur de langage non consulte).
PROCESSUS_DETACHES: voir la sortie de stray_processes.ps1 lancee en dernier geste (un UnrealEditor 33240 demarre 10:49 n'etait pas le mien : ma quick_check avait fini 10:50 ; verifie avec -Root)
ANOMALIES_PROCESSUS: Edit multi-lignes refuse une fois sur HOLAIGarrisonCall.as (fins de ligne mixtes), refait par ligne ; IN_USE.txt remplace comme demande.
GIT_STATUS_PRINCIPALE: docs seuls (ce rapport, EtapeDefini.md, Auto_Journey_File.md) : oui
BRANCHE PRETE : integ-25, tip 10db129.
