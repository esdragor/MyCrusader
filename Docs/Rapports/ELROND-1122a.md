# ELROND-1122a : D877 moitie A rendue visible par le scenario (sans C++)

Impact joueur : aucun changement de jeu ; le scenario `deploy` verifie maintenant que l'engin demonte envoye vers une Maison lointaine se monte puis tire.

APPELS_DECLARES: 19 (arret a 25 vus ; heure d'arret dur 16:47)
HEURE_DEBUT/HEURE_FIN: 16:31:56 / ~16:38 (Get-Date PowerShell)
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (aucun build, sans C++)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 90ff8d3 (assertions du scenario) ; assertit l'effet : oui (montage puis PV de la Maison) ; ROUGE NON VU : le rejeu est VERT   TEST_ROUGE_SEPARE: non (pas de rouge : rien a corriger en C++, le commit est un seul commit de scenario)
COMMITS: 1 (90ff8d3, 16:35), merge_main_fait: sans objet (integ-25 = main 1447448 + 1 commit, rien a fusionner)
TESTS: `play.ps1 deploy -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond -TimeoutSec 300` : VERT, 96,4 s, le jeu quitte sur la derniere etape `quit` ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: deploy (preflight non relance) ; PNG ouverts : deploy_far_start, deploy_far_end ; statut LOGIQUE SEULEMENT (non liste a essayer, la fiche E-deploy reste PAS PRET)
QUESTIONS: aucune
FAIT / RESTE:
- deploy.txt l. 133-136 : `# ELROND-1122a ...`, `waitlog phase 0 -> 1 (order`, `waitlog Mangonel hits a building` apres le `wait 1` qui suit l'ordre lointain (l. 130-132). IMPORTANT : `waitlog` est un texte LITTERAL (HOLTestSequence.cpp:35, seul `$` final est special) : `Deploy: .* phase 0 -> 1` de l'enonce ne matcherait jamais ; texte exact du journal : `Deploy: #%u phase %d -> %d (order %d, action %d, reason %d, distance %d cm, walk %d)` (HOLDeployProcessor.cpp:139) et `Combat: step %lld #%u %s hits a building for %d` (HOLCombatProcessor.cpp:2599).
- deploy.look : 4 points deploy_far_start / walking / midway / end.
- Reste : etape 9 moitie B (attendre `order lines 0` a l'arrivee du deplacement, capture, puis ordre d'attaque ; le waitlog `(delay over)` de l'etape 9 expire si l'engin s'est monte tout seul a l'arrivee) ; capture deploy_far_* : la camera (FocusNear 3000 -2500) laisse la 2e Maison hors cadre sous le panneau des notes (deploy_far_start : seul l'engin est visible), a recadrer.
NON VÉRIFIE: la distance de marche reelle et la vitesse de l'engin (1,8 m/s a l'ecran) ; la position de depart de l'engin avant l'ordre ; si le defaut de la recette 1121se (build 1276d54) vient d'un correctif posterieur (ApplyCombatOrder, ELROND-1121se) ou seulement de la fenetre de 15 s ; PNG deploy_far_walking et deploy_far_midway non ouverts ; preflight deploy non relance.
PROCESSUS_DETACHES: voir sortie de stray_processes en bas
ANOMALIES_PROCESSUS: aucune vue ; le jeu s'est arrete seul
GIT_STATUS_PRINCIPALE: modifs de docs voulus (Auto_Journey_File.md, EtapeDefini.md, Palier3InProgress.md, Retours\E-deploy.md) ; Auto_Journey_Jour.md et les Docs\Rapports\* d'autres agents ne sont pas de moi : oui hors ces fichiers

## Ce que le journal dit vraiment (log HeroesOfLiberty.log du rejeu 15:33-15:34 ; heures locales du journal)

Aucun rouge : la chaine reelle MONTE et TIRE sur l'ordre lointain, en 16 s apres l'ordre.

| Heure | Ligne |
|---|---|
| 15:34.02 | `Order: 1 units pack up` ; `Deploy: #13 phase 2 -> 3 (order 2, action 3, reason 0, distance 1742 cm, walk 3)` (demontage par D, avant l'ordre lointain) |
| 15:34.03 | `Deploy: #13 phase 3 -> 0 (delay over)` : engin Packed |
| 15:34.04 | `Test capture: deploy_far_start` ; `Order: 1 units attack House of slot 1 (other)` + `RpcOrder: attack building received by the server: slot 0 units 1 queue false` ; la 2e Maison est a (-13200, -5600) |
| 15:34.20 | `Deploy: #13 phase 0 -> 1 (order 1, action 2, reason 0, distance 3177 cm, walk 2)` : PendingOrder = 1 pose par le combat, distance a la cible 3177 cm (la portee de montage n'est pas depassee : Decide plafonne a MaxRangeCm 5000), l'engin marchait encore (walk 2) |
| 15:34.21 | `Deploy: #13 phase 1 -> 2 (delay over)` : monte |
| 15:34.24 | `Combat: step 979 #13 Mangonel hits a building for 45` ; deploy_far_walking (clock +20,1 s apres far_start) |
| 15:34.29 / .34 | coups aux steps 1079, 1179 (5 s d'intervalle) ; deploy_far_end 15:34.38 : HUD `D=Demonter` (cases `Q: Attaque; S: Arret; D: Demonter; V: Tenir`), activite `Attacking`, `order lines 1` |

Lecture pour l'agent C++ suivant : (1) la marche d'approche dure ~16 s de temps de jeu entre l'ordre et le montage ; l'ancienne fenetre du scenario etait 1 + 4 + 10 = 15 s apres l'ordre, donc une capture `deploy_far_end` pouvait se prendre juste AVANT le montage (case D = Monter, aucune ligne Deploy: encore), ce qui ressemble exactement au constat de RECETTE-1121se. (2) Entre l'ordre (15:34.04) et `phase 0 -> 1` il n'y a AUCUNE ligne `Deploy:` : PendingOrder = 1 n'est donc pose qu'en fin d'approche, quand le combat juge la cible a portee (conforme au commentaire D861 a, HOLDeployProcessor.cpp:127-131). (3) Si la recette 1121se reste FAIL sur un build plus ancien que 1447448, le correctif ApplyCombatOrder (HOLUnitOrderProcessor.cpp:3218-3228) l'a probablement deja traite ; ne pas ecrire de C++ sans un nouveau rouge reel. Piste utile si la duree de 16 s gene : le combat pose PendingOrder = 1 seulement a portee d'attaque du batiment (3177 cm = distance a la cible a ce moment), alors que le montage accepte jusqu'a MaxRangeCm = 5000 : le montage pourrait commencer des 5000 cm, plus tot de ~1800 cm de marche (~10 s a 1,8 m/s). C'est un choix de design (a ne pas trancher ici).

## PROCESSUS_DETACHES
(voir la sortie colee ci-dessous par le dernier geste)

aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Elrond
