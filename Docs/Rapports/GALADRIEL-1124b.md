# GALADRIEL-1124b (S2-a restes : Rage nécrotique dans le moteur de moral, ÉTAPE 1) : rapport

NON COMPILÉ au moment de ce rapport (build `Tools\build.ps1 -LiveCoding=0 -Wait` lancé à 11:03 sur 5d58440, en file derrière Glorfindel puis Sauron ; voir la fin du rapport pour son statut). Tests NON JOUÉS.

RELECTURE_CPP : HOLRageMoraleTests.cpp (neuf, test simple, pas de .generated.h, bloc WITH_DEV_AUTOMATION_TESTS fermé, includes de HOLUnitAbilityRules.h / HOLMoraleRules.h / HOLSimulationFragments.h) ; HOLMoraleRules.h/.cpp : `RemoveById` déclaré et défini dans namespace HOLMorale, après RemoveAt (même fichier) ; HOLUnitSubsystem.cpp : includes HOLMoraleRules.h + HOLMoraleTableRows.h ajoutés, `FHOLMoraleFragment*` pris par GetFragmentDataPtr ; HOLCombatProcessor.h : FFighter::Morale n'est plus const ; HOLCombatProcessor.cpp : requête Morale ReadWrite (optionnelle), vue `TArrayView` mutable, aucun `case`, aucun jumeau C++/script ; RagePercent n'existe plus (grep).

APPELS_DECLARES: ~17 (arrêt à 25 vus ; arrêt dur 11:15)
HEURE_DEBUT/HEURE_FIN: 10:59:47 / 11:0x (voir fin)
PREMIER_BUILD: 11:03 (en file)   BUILD: NON COMPILÉ (voir fin)   LAUNCHCHECK_DEBUT: binaire périmé (dll 10:58:29 plus vieille que HOLCombatProcessor.cpp 11:02:50 : cause = mes propres modifications, build lancé juste après)
TEST_ROUGE: f04bb00 (HeroesOfLiberty.Morale.NecroticRageIsAMoraleEffect) ; assertit l'effet : oui (le fragment de moral du guerrier porte Total(Dealt) = RageDamagePercent après CastRage ; à StopHp la rage ET l'effet partent ; les valeurs sont calculées par HOLMorale::Total / Multiplier)   TEST_ROUGE_SEPARE: oui (f04bb00 test seul, sans bouchon : il compile contre le code existant ; 5d58440 correctif). Rouge NON VU à l'exécution (un seul build, lancé sur le correctif) ; rouge attendu sur l'ancien code : Total(Dealt) = 0 après CastRage.
COMMITS: 4 (f04bb00 test, 5d58440 correctif, a864248 journal, e98d910 correction de MA propre erreur : mon remplacement regex avait coupe la condition `if (... <= RageStopHp) { Combat.RageMs = 0;` du correctif, relue a 11:08 et reparee avant que le build en file ne demarre ; le build compile e98d910), dernier 11:08, merge_main_fait: oui (premier geste, 10:59 : fusion 54f022c, fast-forward de docs)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lancé par moi, en file)
TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun   DIAGNOSTICS_AS: sans objet (aucun .as touché)

FAIT (étape 1 = le bonus de dégâts +40 %, le seul site)
- CastRage (HOLUnitSubsystem.cpp) pose HOLMorale::Apply(MakeEffect(HOLMoraleNecroticRage, RageDamagePercent, 0, 0, RageDurationMs, false)) sur le fragment de moral de l'unité ; Rate = 0 exprès (le Rate est l'étape 2, pas de doublon).
- HOLMorale::RemoveById (neuf, HOLMoraleRules.h/.cpp) : retire l'effet d'un id.
- HOLCombatProcessor.cpp ~3314-3330 : plus de `RagePercent` additif dans GetDamage ; le combat lit Multiplier(LeverDealt) AVANT la fin éventuelle (le dernier coup avant StopHp garde le bonus, comme avant) ; coût de 3 PV et fin à StopHp restent sur RageMs et retirent l'effet (RemoveById). FFighter.Morale et la requête passent en ReadWrite.
- Test neuf HOLRageMoraleTests.cpp (chaîne réelle : CastRage, RunStep du combat). Le test existant HeroesOfLiberty.Combat.DamnedWarrior.Rage (coup x1,4 mesuré) devient la preuve que le combat lit le levier seul : si la pose manquait, il rougirait.
RESTE (lignes [ ] ajoutées à la File, Auto_Journey_File.md l.49-50)
- Étape 2 : Rate (+25 %) : MakeEffectFromRow(NecroticRage) à la pose, HOLCombatProcessor.cpp:2535 ne lit plus RageMs.
- Étape 3 : lectures de RageMs (décompte :1155, visuel HOLRageVisual.cpp:31, HOLInterpolatedInstancesProcessor.cpp:583, HOLSelectionMarkers.cpp:791, empreinte HOLSimulationSubsystem.cpp:154-156, tests HOLDamnedWarriorTests.cpp:235-273).

ÉCARTS DE RÈGLE à connaître (non tranchés par moi, conformes à Q4 = A) : (1) le bonus était additif avec la technologie d'attaque (tech + 40), il est maintenant multiplicatif (x1,4) : identique à technologie 0, légèrement différent avec une technologie d'attaque ; (2) le bonus est plafonné par BonusCap(Dealt) = 40 avec les autres bonus de dégâts (bannière, cri) : la rage seule vaut 40, la rage + un autre bonus plafonne à 40 au lieu de s'empiler ; (3) l'effet de moral est décompté par le processeur de moral, RageMs par le combat : les deux durées (10 s) partent du même instant, une dérive d'un pas est possible.

NON VÉRIFIÉ : build, rouge jamais vu à l'exécution, l'ensemble des tests ; HOLHeroAbilityMatchTests / HOLMoraleCombatTests (le combat lit maintenant un fragment de moral mutable) ; interaction du plafond avec la bannière (écart 2).
PROCESSUS_DETACHES: voir fin
ANOMALIES_PROCESSUS: build en file derrière deux autres worktrees (Glorfindel, Sauron), attente de plus de 180 s : passé en tâche de fond (b7x9ympf3).
GIT_STATUS_PRINCIPALE: inchangé hors docs voulus (Auto_Journey_File.md l.49-50, EtapeDefini.md l.28, ce rapport) : oui
