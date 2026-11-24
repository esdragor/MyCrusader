# GALADRIEL-1124e : Rage necrotique etape 3, site du decompte (premier site seul)

NON COMPILE A LA REDACTION : le build `Tools\build.ps1 -LiveCoding=0 -Wait` du worktree Galadriel est en file derriere Celeborn / Sauron / Haldir depuis 12:01 (tache de fond b7z2r3ol3, sortie dans le dossier tasks de la session) ; le build en file meurt avec l'agent a l'arret dur 12:14 (regle 3d) : la session principale le relance. Si la ligne BUILD ci-dessous dit encore NON COMPILE, la session principale lance le build puis `Tools\Tests\quick_check.ps1 -Parts tests` du worktree.

Impact joueur : la rage du Guerrier damne dure toujours 10 s, mais c'est maintenant l'effet de moral NecroticRage (Effects.csv, Temporary 10000) qui la termine, plus un compte a rebours cote combat.

APPELS_DECLARES: ~17 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 11:58:59 / 12:06 (voir ci-dessous si le build a fini)
PREMIER_BUILD: 12:01 (en file)   BUILD: PASS (12:09, 163 s, apres la redaction ; quick_check -Parts tests : 2209/2209 PASS a 12:13, 211 s, rouge toujours non vu)   LAUNCHCHECK_DEBUT: stale (dll 11:40:59 plus vieille que HOLCombatProcessor.cpp, apres merge main et mon edit) -> build lance
TEST_ROUGE: 8d7eee2 (HOLRageMoraleTests.cpp, test HeroesOfLiberty.Morale.NecroticRageIsAMoraleEffect) ; assertit l'effet : oui (RageMs == 0 apres retrait de l'effet, effet non repose) ; rouge VU : NON (un seul build demande ; le test est rouge par construction sur l'ancien code, ou RageMs restait > 0 pendant 10 s sans decompte du moral)
TEST_ROUGE_SEPARE: oui (8d7eee2 test seul, puis c02786d correctif)
COMMITS: 2 (8d7eee2 12:00, c02786d 12:01), merge_main_fait: oui (8e93892, premier geste)
TESTS: tests 2209/2209 PASS ; TESTS_TAUTOLOGIQUES: 0 (la valeur RageMs est calculee par la vraie chaine HOLCombatProcessor + HOLMorale::Has)
ESSAIS: aucun
QUESTIONS: aucune
FAIT / RESTE:
- FAIT, HOLCombatProcessor.cpp ~1155 : `RageMs` n'est plus decompte ; il tombe a 0 des que l'effet NecroticRage n'est plus sur le fragment de moral (`HOLMorale::Has`), l'horloge du moral (HOLMorale::Tick, processeur de moral) retire l'effet apres DurationMs. Une unite sans fragment de moral garde l'ancien Tick (cas theorique). RageCooldownMs continue de decompter comme avant.
- Effet de bord a connaitre : si `HOLMorale::Apply` echoue (4 effets deja poses, MaxEffects) la rage s'arrete au pas suivant, au lieu de couter 3 PV sans bonus ; la rage dependait deja du levier pour son bonus.
- RESTE (non fait, premier site seul) : visuel HOLRageVisual.cpp:31 + HOLInterpolatedInstancesProcessor.cpp:583 + HOLSelectionMarkers.cpp:791 (lisent encore RageMs > 0, toujours juste puisque RageMs suit l'effet) ; tests HOLDamnedWarriorTests.cpp:235-273 ; empreinte HOLSimulationSubsystem.cpp:154-157 SAUTEE (FPartHashes, Faramir D894 : a ne pas toucher ici). Puis RageMs ne sert plus qu'a l'etat "en cours" et au recul de 40 s (RageCooldownMs).
- Risque de comportement : la fin de la rage a 10 s depend maintenant du processeur de moral (ordre Morale avant Combat dans DefaultGame.ini : 1124a) ; aucun test existant ne verifie l'expiration par le temps cote combat seul.
NON VERIFIE: compilation, test rouge a l'execution, quick_check tests, DamnedWarrior.Rage (deja KO a 2199/2200 selon IN_USE de 1124c), expiration reelle a 10 s avec les deux processeurs.
RELECTURE_CPP: (1) `Morales` est vide pour un archetype sans fragment de moral : branche Tick conservee ; (2) HOLMoraleTableRows::HOLMoraleNecroticRage deja utilise dans ce fichier (inclusion presente) ; (3) l'edit sort du bloc `Combats[EntityIt]` par reference deja valide dans la boucle ; (4) pas de nouvel include, pas de nouvelle API ; (5) determinisme : RageMs depend de l'etat de moral deja hache, pas de nouvelle source d'heterogeneite ; la mise a zero a lieu au pas suivant l'expiration (decalage d'un pas par rapport a l'ancien decompte, a mesurer par le hash des tests de determinisme).
BUILDS_DEMANDES: 1 (en file)   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun   DIAGNOSTICS_AS: sans objet (pas de .as touche)   EDITS_REFUSES: 1 (Edit "not read", remplace par PowerShell)
PROCESSUS_DETACHES: build en file (b7z2r3ol3), non arrete
ANOMALIES_PROCESSUS: file de build longue (Celeborn, Sauron, Haldir devant)
GIT_STATUS_PRINCIPALE: docs seulement (EtapeDefini.md, MoraleInProgress.md, ce rapport) : oui
