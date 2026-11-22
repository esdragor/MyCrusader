# CELEBORN-1122k : mesure BSHOT (F8 T4 BuildingShotCadence) - NON COMPILE

APPELS_DECLARES: ~14 (arret a 18)
HEURE_DEBUT/HEURE_FIN: 2026-11-22T18:29:19 / voir Get-Date de fin
PREMIER_BUILD: sans build (consigne : NE COMPILE PAS)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de compilation)
TEST_ROUGE: b52d356 (BuildingShotNoUnit) ; rouge LU dans le code, non vu a l'execution ; assertit l'effet : oui (EntriesAfterDeath == 0 apres destruction des tours, unite morte)   TEST_ROUGE_SEPARE: oui (b52d356 test, b0d089d correctif ; le test compile sans bouchon)
COMMITS: 3 sur integ-41 (b6fa4f0 diag, b52d356 rouge, b0d089d purge), merge_main_fait: non (pas demande)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune

## FAIT
- A b6fa4f0 : cvar hol.Combat.BuildingShotDebug (HOLCombatProcessor.cpp, espace de noms Combat, 0 par defaut ; mis a 1 par RunScenario, remis a 0 en fin). Lignes :
  - `BSHOT step=%d StepMs=%d entries=%d remaining=%d arrows=%d units=%d target=%d` a chaque lecture du compte a rebours par tour : target=-1 compte > 0 (attente, pas de recherche) ; target=0 prete et personne a portee (compte remis a 0) ; target=<UnitId> fleche partie (remaining = valeur AVANT += intervalle).
  - `BSHOT execute step= StepMs= entries= fighting=` a l'entree d'Execute apres StepMs (donc apres les retours `!Units || !Commands`) ; `BSHOT execute early exit (no fighter) ...` au retour Fighters.Num()==0 ; `BSHOT call step= entries= units= buildings=` juste avant ReleaseBuildingArrows.
  - Cote test : `BSHOT test runstep=N step=S` a chaque RunStep (= un appel Execute voulu) et `BSHOT test end execute_calls= steps= released= entries=` en fin de scenario.
- B b52d356 (test rouge) puis b0d089d (correctif, une ligne + commentaire) : `BuildingCooldownMs.Reset()` dans le bloc Fighters.Num()==0 ; test BuildingShotNoUnit (victime a 1 PV tuee par la 1re fleche, 2 tours, tours detruites : 0 entree attendue ; rouge par lecture : 2 entrees restent).
- Aucune autre regle modifiee. Le retrait du diag : `git revert b6fa4f0` (ou supprimer les 6 blocs `if (BuildingShotDebug != 0)` + le cvar + les lignes ShotDebug/RunStepCalls du test).

## HYPOTHESES (a lire dans le journal du test ; run : quick_check -Parts tests, filtre `BSHOT` ; comparer `runstep` et `execute` d'abord)
H1 Execute sauté : si les lignes `BSHOT test runstep=N` sont plus nombreuses que `BSHOT execute step=` (ou `BSHOT call`) sur les pas du trou, alors Execute est appelé mais sort avant StepMs (`!Units || !Commands`, l.942) ou n'est pas lancé par l'Executor ; si `execute` present mais `call` absent pour un pas, alors un retour precoce entre l.1113 et l.1519 (cause a nommer par bisection de ces retours). Periode 96-108 au lieu de 33 = pas sautes sans `call`.
H2 StepMs faux : si entries=1 et `call` present a chaque pas mais `remaining` baisse par pas de StepMs different de 50 (ex. 16 ou 33 ou varie), alors le delta du contexte Mass n'est pas la valeur attendue (Context.GetDeltaTimeSeconds != StepSeconds). Si StepMs=50 constant et remaining baisse de 50 par pas, H2 est fausse.
H3 Remis a 0 sans cible : si `target=0` apparait avec `entries=1` entre deux fleches (remaining remis a 0 puis une fleche au pas suivant), alors la cible n'est pas candidate certains pas (units=0 ou victime filtree par Vision/portee/Hostile) : remaining=0 efface le report de 1,625 s. Si units=0 sur `call`, alors la victime a disparu de Fighters (naissance echelonnee, entite hors requete).
H4 Entree perdue / purge : si `entries=0` apres avoir ete 1 (ou `execute early exit (no fighter)` suivi d'un retour de `entries=1` a 0), alors une purge SeenStep ou Reset a eu lieu : le compte repart a 0 = « pret tout de suite » = fleche plus tot, pas plus tard ; donc n'explique que des fleches EN TROP. Si entries reste 1 sans decrement, la purge n'est pas en cause.
H5 Fleche en vol bloquante : si `arrows>0` pendant les pas ou remaining ne baisse pas ou target=-1 sans fleche, alors une fleche en vol retient le compte ; si `arrows=0` aux pas du trou, H5 fausse. (Le code lu ne conditionne pas le tir aux fleches en vol : ce serait un effet hors ReleaseBuildingArrows.)
H6 Deux tours qui se disputent l'entree : si `entries=1` alors que deux tours sont armees (Towers=2, scenario `two towers`), alors les deux tours partagent la meme cle Step.Id (identifiant du batiment identique ou 0) : un seul compte decremente deux fois par pas (periode moitie) ou ecrase ; si `entries=2`, fausse. Lire les lignes BSHOT `remaining` de la meme `step` : deux lignes par pas avec decrement double d'un meme compte = confirmation. (Cela explique 96 vs 108 entre un et deux tours.)
H7 (lue dans le code, non parmi les cinq) : le test fixe `First <= 1` ; si le premier `BSHOT call` a `step` 3 avec `units=0` aux pas 1-2, alors c'est la naissance (ServeBirthRequests) et non la regle : ajuster First, pas la tolerance.

## NON VERIFIE
Tout : aucune compilation ni execution ; le rouge de BuildingShotNoUnit est lu (le test compile sans bouchon, probable : Mass retire l'unite morte avant le pas suivant, donc Fighters.Num()==0 ; si l'unite reste jusqu'a FlushDestructions, un pas sans unite vient quand meme avant les 2 RunStep finaux). `%d` avec static_cast<int32> partout (UE_LOG).

PROCESSUS_DETACHES: aucun (pas de lancement)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: non verifie apres edit (seules les lignes EtapeDefini.md touchees)