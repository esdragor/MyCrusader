# CELEBORN-1122l : cause du rouge F8 T4 BuildingShotCadence PROUVEE par la mesure - NON COMPILE

APPELS_DECLARES: ~19 (arret a 22)
HEURE_DEBUT/HEURE_FIN: 2026-11-22T18:39:21 / voir Get-Date de fin
PREMIER_BUILD: sans build (consigne : NE COMPILE PAS)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de compilation)
TEST_ROUGE: f85a2b4 (BuildingShotIdleVictim) ; rouge vu par la MESURE du journal 17:37:26 (BuildingShotCadence : 3 fleches au lieu de 10), pas a l'execution du nouveau test ; assertit l'effet : oui (ecart entre les deux premieres fleches = 1 intervalle a un pas pres, nombre de fleches)   TEST_ROUGE_SEPARE: oui (f85a2b4 test, 5b4cc49 correctif)
COMMITS: 3 sur integ-41 (f85a2b4 test, 5b4cc49 correctif, 3b5ed92 retrait du diag), merge_main_fait: non
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (ecart et compte calcules par ExpectedArrows / IntervalMs, pas recopies)
ESSAIS: aucun
QUESTIONS: aucune

## MESURE (portion BuildingShotCadence de quick_tests.log : lignes 30651 a 36390, 5710 lignes BSHOT ; scenario 'steady fire' = les 70 premieres)
Un tour, victime inerte UnitId 4 (tough), StepMs=50 constant.
- ligne 33 (log) `BSHOT call step=12 ... ` puis, pas 13, 14, 15 : `BSHOT test runstep=13` + `BSHOT execute step=13 StepMs=50 entries=1 fighting=1` SANS `BSHOT call` ni `BSHOT step=` ; pas 16 : `BSHOT call step=16 entries=1 units=1 buildings=1` + `BSHOT step=16 ... remaining=1175 arrows=0 target=-1`. Meme motif 17-19 (call 20, remaining=1125), 21-23 (call 24) : un appel sur 4, exactement les pas ou (UnitId 4 + step) % SearchSteps == 0.
- Pas 5 a 11 : arrows=1 (une fleche en vol), call a CHAQUE pas, remaining baisse de 50 par pas (1575, 1525 ... 1225) ; des que arrows=0 (pas 12), l'appel ne vient plus que tous les 4 pas et remaining ne baisse que de 50 par appel (1225, 1175, 1125) : le compte a rebours est lu un pas sur quatre.
- Premiere fleche : `call step=4 entries=0` puis `target=4` (pas 4) alors que les pas 1 a 3 ont `execute` sans `call` : meme cause (premiere lecture au premier pas de recherche de l'unite), d'ou 'first arrow step 3 (pas <=1)'.

## VERDICT
- CONFIRMEE : H1, branche 'execute present, call absent' : sortie d'Execute entre l.964 (execute) et l.1529 (call), sans la ligne 'early exit (no fighter)' (donc pas l.1129) et avec fighting=1 (donc pas le `!bFighting` l.1184) : le SEUL retour restant est le saut FastIdle (HOLCombatProcessor.cpp l.1212-1241, `FastIdle != 0 && Arrows.Num() == 0`, hol.Combat.FastIdle = 1 par defaut) : tous les combattants au repos (victime sans cible) et aucune fleche en vol => TickIdle et return ; ReleaseBuildingArrows (l.1531) est APRES ce retour. Le pas de recherche de la victime (bSearchStep, UnitId+step modulo SearchSteps=4) rend bIdle faux, d'ou un passage sur 4.
- EXCLUES : H2 (StepMs=50 constant, remaining -50 par appel) ; H3 (jamais target=0 entre deux fleches ; units=1 sur chaque call) ; H4 (entries 1 constant, jamais 0 apres 1 ; la purge n'explique que des fleches en trop) ; H5 (arrows=0 aux pas du trou mais c'est la CONDITION du saut, pas un blocage du compte) ; H6 (entries=1 avec une tour, 2 avec deux tours, pas de cle partagee) ; H7 (la naissance n'est pas en cause : la meme cause explique le pas 4).
- Cible a -1 : normale (compte > 0, pas de recherche), pas un defaut. 'call' absent alors que 'execute' present : voir ci-dessus. Compte remis a 0 : non.
- Effet du saut : periode = 4 pas par lecture de 50 ms, soit ~108-130 pas au lieu de 33 (les valeurs 96-108 de 1122j) ; entree jamais purgee quand les tours sont detruites (2 au lieu de 0 : le meme saut passe avant ReleaseBuildingArrows / StepBuildings.IsEmpty()).

## FAIT
- 5b4cc49 (HOLCombatProcessor.cpp ~14 lignes) : `bBuildingsLive = BuildingCooldownMs.Num() > 0` puis, si FastIdle et pas de fleche, parcours FHOLBuildingIterator : un batiment arme (BuildingAttack > 0, BuildingFiresEmpty, IsFinished, PV > 0) ou un compte encore tenu (a purger) interdit le saut FastIdle ; valeurs attendues du test inchangees.
- f85a2b4 : test BuildingShotIdleVictim (victime tough inerte, 6 s : ecart des 2 premieres fleches = 1 intervalle a un pas pres, compte = ExpectedArrows) ; echouerait sans le correctif (ecart ~4 intervalles).
- 3b5ed92 : `git revert b6fa4f0` adapte a la main (conflit avec b0d089d/611638a) : cvar, 6 blocs de journal, using namespace ajoute par 611638a, compteurs RunStepCalls du test retires.

## RESTE / NON VERIFIE
Rien compile ni lance. Commande pour la session principale : Tools\build.ps1 -LiveCoding=0 -Wait puis quick_check -Parts tests ; attendre BuildingShotCadence vert, BuildingShotIdleVictim vert, BuildingShotNoUnit vert. Risque : le cout du parcours des batiments a chaque pas inerte (mesure de perf a faire sur une grande carte si FastIdle compte beaucoup) ; si perf mauvaise, memoriser dans un membre le dernier etat 'arme' au lieu du parcours. Le premier pas (premiere fleche pas <= 1) suppose que la naissance de la victime laisse l'unite dans Fighters au pas 1.
Placeholders.txt : aucun provisoire ajoute ni retire.

PROCESSUS_DETACHES: voir derniere sortie
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: seuls EtapeDefini.md et FortificationsInProgress.md (docs voulus) touches par cette seance