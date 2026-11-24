# GALADRIEL-1124c (S2-a : Rage necrotique dans le moteur de moral, ETAPE 2 : le Rate +25 % passe par le moral) : rapport

Impact joueur : la cadence +25 % de la Rage necrotique du Guerrier damne vient maintenant du meme effet de moral que ses +40 % de degats (un seul effet NecroticRage, ligne d'Effects.csv), au lieu d'un test sur RageMs dans le combat. Aucun changement attendu en jeu a technologie 0.

BUILD : voir la section « Statut du build » en fin de rapport (lance 11:25:46, worktree Galadriel, `Tools\build.ps1 -LiveCoding=0 -Wait`, sortie dans `<worktree>\GALADRIEL_build_out.txt`). Tests NON JOUES.

RELECTURE_CPP : HOLUnitSubsystem.cpp CastRage : `HOLMorale::MakeEffectFromRow(HOLMoraleTableRows::HOLMoraleNecroticRage)` (declare dans HOLMoraleRules.h:78, defini dans HOLMoraleRules.cpp ; includes HOLMoraleRules.h et HOLMoraleTableRows.h deja la depuis l'etape 1) ; HOLCombatProcessor.cpp ~2533-2538 : la variable `RageIntervalMs` et son appel `HOLUnitAbility::GetRageIntervalMs` sont supprimes de cette ligne, `IntervalMs = Max(1, BaseIntervalMs * 100 / Max(MoraleRatePercent, 1))` (meme formule que GetRageIntervalMs : Base*100/125 pour Multiplier(Rate)=125) ; plus aucun usage de `RageIntervalMs` dans les .cpp (grep) ; `HOLUnitAbility::GetRageIntervalMs` reste utilise par `UHOLUnitAbilityReadLibrary::GetRageIntervalMs` (liaison script, non touchee) ; HOLRageMoraleTests.cpp : `EffectsBefore` declare avant le cast, 5 assertions neuves, aucune nouvelle inclusion ; aucun `case`, aucun jumeau C++/script, aucun UPROPERTY/UFUNCTION neuf.

APPELS_DECLARES: ~17 (arret a 25 vus ; arret dur 11:39)
HEURE_DEBUT/HEURE_FIN: 11:23:46 / voir fin
PREMIER_BUILD: 11:25:46   BUILD: voir « Statut du build »   LAUNCHCHECK_DEBUT: vide (binaire de 11:22:26 a jour avant mes modifications ; `Get-StaleBinaryProblem` en tete du build 11:25 : perime a cause de MES modifications de 11:25:25, attendu)
TEST_ROUGE: a371040 (HeroesOfLiberty.Morale.NecroticRageIsAMoraleEffect etendu : Total(LeverRate) == RageRatePercent, Multiplier(LeverRate) == 100 + RageRatePercent, un seul effet pose (Num == avant + 1), Total(LeverRate) == 0 une fois la rage finie) ; assertit l'effet : oui (valeurs calculees par HOLMorale::Total / Multiplier sur le fragment de moral reel apres Units->CastRage)   TEST_ROUGE_SEPARE: oui (a371040 test seul, sans bouchon : il compile contre le code existant ; 21facdc correctif). Rouge NON VU a l'execution (un seul build, lance apres les deux commits, comme l'etape 1) ; rouge attendu sur l'ancien code : Total(Rate) = 0 apres CastRage.
COMMITS: 3 (a371040 test, 21facdc correctif, d53c29d journal MoraleInProgress.md), dernier 11:26, merge_main_fait: oui (premier geste, 11:2x : 2a7554c, main 4547160, docs seulement)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (les valeurs viennent de la chaine reelle CastRage -> HOLMorale::Apply -> Total / Multiplier)
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lance par moi)
TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun   DIAGNOSTICS_AS: sans objet (aucun .as touche)

FAIT
- CastRage pose l'effet de la ligne NecroticRage (Dealt 40 + Rate 25, Temporary 10000 ms) d'un bloc ; plus de Rate 0 ni de doublon possible (le combat ne lit plus RageMs pour la cadence).
- HOLCombatProcessor.cpp : l'intervalle entre coups ne depend que du levier Rate du moral (rage, banniere, malus ensemble, une seule source).
- Test etendu (rouge d'abord) ; journal MoraleInProgress.md ; ligne de la File (Auto_Journey_File.md l.52) et EtapeDefini.md l.31.

RESTE
- Etape 3 (ligne [ ] de la File) : autres lectures de RageMs (decompte HOLCombatProcessor.cpp:1155, visuel HOLRageVisual.cpp:31, HOLInterpolatedInstancesProcessor.cpp:583, HOLSelectionMarkers.cpp:791, empreinte HOLSimulationSubsystem.cpp:154-156, tests HOLDamnedWarriorTests.cpp:235-273).
- Apres build : `quick_check -Parts tests` du worktree (le test de moral et `HeroesOfLiberty.Combat.DamnedWarrior.Rage` : cadence +25 % mesuree, qui prouve que le combat lit le levier Rate seul ; ce test est le garde-fou contre un doublon ou une perte).

ECARTS A CONNAITRE (non tranches par moi)
- Le Rate de la rage plafonne avec les autres bonus de cadence (BonusCap du levier Rate) : rage 25 + une source de cadence peut depasser le cap (30 % annonce dans HOLMoraleRules.h:21) et etre ecretee ; avant, la rage s'ajoutait hors cap. Identique si la rage est seule.
- La duree de l'effet vient de la ligne du CSV (10000), celle de RageMs de la constante RageDurationMs (10000) : les deux sont dans le meme etat, derive d'un pas possible (deja note a l'etape 1).
- `HOLUnitAbility::RageRatePercent` et `GetRageIntervalMs` restent dans HOLUnitAbilityRules.h : plus lus par le combat (test de donnees HOLDamnedWarriorTests.cpp:106 les compare au CSV), a retirer avec l'etape 3.

NON VERIFIE : compilation et execution (voir statut du build), rouge jamais vu a l'execution, HOLMoraleCombatTests / HOLHeroAbilityMatchTests (le combat lit toujours un fragment de moral), interaction du cap Rate avec la banniere.
PROCESSUS_DETACHES: voir fin
ANOMALIES_PROCESSUS: build lance par `Start-Process` masque (sortie redirigee dans le worktree) au lieu d'un appel bloquant : ecart de methode de ma part, le build.ps1 a bien demarre (cl.exe vus a 11:26). Fichiers `GALADRIEL_build_out.txt` / `GALADRIEL_build_err.txt` a la racine du worktree, non commites.
GIT_STATUS_PRINCIPALE: inchange hors docs voulus (Auto_Journey_File.md l.52, EtapeDefini.md l.31, ce rapport) : oui (a confirmer par la session principale)

## Statut du build
BUILD: PASS a 11:27 (83 s, Tools\build.ps1 -LiveCoding=0 -Wait, un seul build pour le lot ; dll 11:27:11). Tests : Tools\Tests\quick_check.ps1 -Root <worktree> -Parts tests 11:27 -> 11:32 : **2199/2200**, le seul KO = HeroesOfLiberty.Combat.DamnedWarrior.Rage (HOLDamnedWarriorTests.cpp:287 fter the rage: the blow of before : attendu 16, obtenu 22,4 = x1,4). Mon test Morale.NecroticRageIsAMoraleEffect PASSE (donc le Rate est pose, un seul effet, et retire a StopHp). Cadence mesuree (more blows in the same time, l.278) PASSE : le combat lit bien le levier Rate seul.

CAUSE PROBABLE du KO (NON VERIFIEE, a confirmer) : le test fait tourner le seul processeur de combat ; l'effet de moral de la rage n'expire par le temps que dans le processeur de moral (HOLMorale::Tick), absent de cette chaine de test, alors que RageMs (combat) retombe a 0 : apres les 10 s le bonus Dealt reste pose (x1,4). Ce defaut existe depuis l'etape 1 (5d58440, jamais joue) ; il n'est pas cause par l'etape 2. Dans le jeu le processeur de moral tourne (DefaultGame.ini). Correctifs possibles (a trancher / faire au prochain passage, un build 83 s + quick_check 286 s) : (a) test : appeler HOLMorale::Tick(WarriorMorale->State, pas) a chaque RunStep de HOLDamnedWarriorTests (ou lancer le processeur de moral) ; (b) code : le combat retire l'effet (HOLMorale::RemoveById) quand RageMs tombe a 0 par le temps (comme a StopHp), a faire avec l'etape 3 qui retire RageMs.

HEURE_FIN: 11:34 ; APPELS : ~25 vus (arret).

