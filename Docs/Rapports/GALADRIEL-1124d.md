# GALADRIEL-1124d (regression Combat.DamnedWarrior.Rage depuis l'etape 1 de la Rage necrotique) : rapport

RESULTAT FINAL (11:46) : BUILD PASS 11:41:03 (102 s) ; quick_check -Parts tests (worktree) 2200/2200 PASS en 246 s, 0 rouge ; Combat.DamnedWarrior.Rage vert. Commit 494c933. Appels vus ~31 (au-dela des 25 pour attendre build et tests, fin 11:46 < arret dur 11:50). Les lignes NON COMPILE / NON TESTE ci-dessous sont l'etat de 11:41, perimees.

NON COMPILE / NON TESTE (perime) : build lance 11:36:56 (`Tools\build.ps1 -LiveCoding=0 -Wait`, passe en arriere-plan apres 180 s, cl.exe actifs depuis 11:39:29, toujours en cours a 11:41) ; quick_check NON LANCE (limite de 25 appels atteinte avant la fin du build). La session principale doit lancer : `Tools\build.ps1 -LiveCoding=0 -Wait` puis `Tools\Tests\quick_check.ps1 -Parts tests` DU WORKTREE `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel` (0 rouge attendu).

Impact joueur : aucun changement en jeu (le jeu etait juste) ; seul le test de la rage du Guerrier damne etait faux.

VERDICT EN JEU : l'effet de moral NE SURVIT PAS a la fin de la rage. Preuves de code (lues, pas jouees) :
- `UHOLMoraleProcessor::Execute` (HOLMoraleProcessor.cpp:195) appelle `HOLMorale::Tick(Fragments[Index].State, StepMs)` pour CHAQUE unite a CHAQUE pas ; il est `ExecuteBefore HOLCombatProcessor` (l.151) et enregistre dans DefaultGame.ini l.41 avant Combat l.42.
- L'effet NecroticRage est `Temporary 10000 ms` (HOLMoraleTableRows.h:39) ; RageMs vaut `RageDurationMs` = 10000 (HOLUnitSubsystem.cpp:1108), decompte au meme pas (HOLCombatProcessor.cpp:1155) : les deux tombent a 0 au meme pas. Fin par PV : le combat retire l'effet (HOLCombatProcessor.cpp:3325, RemoveById).
- `HOLDamnedWarriorTests.cpp` ne lance QUE `UHOLCombatProcessor` (RunStep, l.142-151) : personne ne fait `HOLMorale::Tick`, l'effet Dealt 40 % reste pose apres 10 s, d'ou 16 x 1,4 = 22,4.
Donc correctif (a) (test), pas (b) : retirer l'effet dans le combat a RageMs == 0 serait un jumeau de la regle du temps du moral (deux sources de la meme fin), et ne corrigerait rien en jeu.

CORRECTIF : HOLDamnedWarriorTests.cpp, RunStep : avant l'execution du combat, `HOLMorale::Tick(UnitMorale->State, HOLCombat::ToMilliseconds(StepSeconds))` sur le fragment de moral de chaque unite (GetAllUnitIds + FindEntity), meme appel que le processeur ; includes `Simulation/HOLCombatRules.h` et `Simulation/HOLMoraleRules.h`. Aucun code de jeu touche. Commit sur socle-ai (voir git log, dernier commit, `Agent: GALADRIEL-1124d`), apres `git merge main` (17fe112, premier geste).

RELECTURE_CPP : HOLMorale::Tick(FHOLMoraleState&, int32) declare HOLMoraleRules.h:119 ; HOLCombat::ToMilliseconds(float) HOLCombatRules.h:82 (HEROESOFLIBERTY_API) ; FHOLMoraleFragment lu comme dans HOLRageMoraleTests.cpp:101 (GetFragmentDataPtr, State) avec les memes includes (HOLSimulationFragments.h) ; `UnitEntity.IsSet() ? ... : nullptr` garde l'entite detruite ; aucune capture ajoutee au lambda RunStep (Units, EntityManager, StepSeconds deja captures) ; indentation a tabulations ; pas de C++ de jeu neuf, pas de UPROPERTY/UFUNCTION. Ecart a connaitre : le test ne fait pas `ClearAuras` ni la pose des auras (aucune source d'aura dans ce scenario).

APPELS_DECLARES: ~26 (arret a 25 vus ; arret dur 11:50)
HEURE_DEBUT/HEURE_FIN: 11:35 / ~11:42
PREMIER_BUILD: 11:36:56   BUILD: NON COMPILE (en cours a 11:41)   LAUNCHCHECK_DEBUT: non lance (binaire de 11:27 sans les modifications de ce lot ; tests uniquement)
TEST_ROUGE: HeroesOfLiberty.Combat.DamnedWarrior.Rage vu rouge a 11:27 par GALADRIEL-1124c (2199/2200, attendu 16 obtenu 22,4), aucun commit de bouchon (le test existait, c'est lui le rouge) ; assertit l'effet : oui   TEST_ROUGE_SEPARE: non (le test rouge est anterieur, d53c29d ; ce commit est le seul correctif)
COMMITS: 1 (correctif test + journal MoraleInProgress.md), merge_main_fait: oui (17fe112)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (valeurs calculees par la chaine reelle CastRage / Tick / combat)
ESSAIS: aucun   QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lance par moi, en cours)   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun   DIAGNOSTICS_AS: sans objet
RESTE : build PASS + `quick_check -Parts tests` (attendu 2200/2200) ; lignes EtapeDefini.md / Auto_Journey_File.md non touchees par manque d'appels (journal MoraleInProgress.md a jour) ; etape 3 de la rage (lectures de RageMs).
NON VERIFIE : compilation, execution (le test corrige n'a pas ete rejoue), absence de regression de Morale.* / Combat.* ; le raisonnement « le jeu est juste » repose sur la lecture du code (jamais joue).
PROCESSUS_DETACHES: build.ps1 du worktree en cours (non arrete, regle 3e) ; stray_processes lance en lecture
ANOMALIES_PROCESSUS: le build est passe en arriere-plan apres 180 s de file/compilation et n'etait pas fini a l'arret.
GIT_STATUS_PRINCIPALE: non relu (docs seulement ecrits : ce rapport) : a confirmer par la session principale.
