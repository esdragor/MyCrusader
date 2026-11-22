# GLORFINDEL-1122k : follow_* scenarios (noms .look, heal_done, geste interne)

APPELS_DECLARES: 14 (arret a 24 ou 18:48)
HEURE_DEBUT/HEURE_FIN: 18:33:29 / 18:35:04
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build, aucun quick_check)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scenarios seuls, aucun code) ; assertit l'effet : oui (waitlog sur la ligne du processeur de soin)   TEST_ROUGE_SEPARE: non (pas de code a corriger)
COMMITS: 1 (8463f9f, 18:34, ability-columns), merge_main_fait: non (integ-52 apporte du C++ : dll perimee)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: follow_short, follow_crowd : NON REJOUE (editeur permis apres 18:50, arret 18:48 ; pas de preflight). LOGIQUE SEULEMENT. A rejouer : play.ps1 follow_short -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel ; idem follow_crowd ; preflight d'abord.
QUESTIONS: aucune

## Fait
1. follow_short.look : la ligne declaree follow_short_walk2 devient follow_short_second_leg (le .txt prend start, walk, second_leg, cancelled). Comparaison de TOUS les noms (.look contre .txt) des deux scenarios par script : follow_short 4/4 identiques apres correction (walk2 etait le seul ecart), follow_crowd 6/6 identiques (start, body_peasant, body_selected, ground, heal_start, heal_done), aucun ecart.
2. follow_crowd.txt : apres hol.Select.OrderNear 300 3300 (clic sur #22), le wait 6 + capture est remplace par waitlog #22 (slot 0) is healed to 155 (ligne UE_LOG de HOLHealProcessor.cpp:344, ecrite une seule fois quand la cible atteint MaxHealth), hol.Units.Where, hol.Select.ClickUnit 21 (= #22 : ClickUnit n selectionne l'id n+1, comme ClickUnit 20 = Moine #21), frames 10, puis capture follow_crowd_heal_done. Unite soignee : #22, Homme d'armes (slot 0, a (-14500, 3300), a droite du Moine #21), blessee a 77/155 par hol.Cheat.Wound 50. Wound blesse TOUTES les unites (22 a ce moment) : le Moine en soigne d'autres (#19 Paysan constate en 1122c) ; #22 reste la cible du clic. Le .look le dit. waitlog est place juste apres la commande (la marque du journal bouge a chaque commande, HOLTestSequence.cpp:35), timeout de pas 60 s.
3. Clics restes OrderNear ; note GESTE: interne ajoutee en fin des deux .look (bascule au vrai clic hol.Select.RightClickGround apres la fusion d'integ-52).

## NON VERIFIE
Le rejeu n'a pas eu lieu : risque que la ligne #22 ... is healed to 155 soit deja passee avant le clic (alors le waitlog expire a 60 s : visible, pas silencieux) ; la barre de vie pleine de #22 sur le PNG non vue ; les 6 spheres vertes de follow_* (RECETTE-1122c) non traitees. Aucune assertion affaiblie.

## Suivi
OrdersPatrolInProgress.md ligne 50, EtapeDefini.md ligne 37, Auto_Journey_File.md ligne 145 ecrites (relues).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel

ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE (hors Docs) : 
 M Auto_Journey_File.md
 M Auto_Journey_Jour.md
 M EtapeDefini.md
 M FortificationsInProgress.md
 M OrdersPatrolInProgress.md
 M Retours/E-deploy.md
 M Retours/E-order_lines_blocks.md


  inchangee hors docs voulus : oui