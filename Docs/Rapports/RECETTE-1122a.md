# RECETTE-1122a : recette independante E-deploy (D877, moitie A et B)

Debut 17:01:07 (Get-Date), fin 17:05 ; ~15 appels. Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, integ-25, tip 57082ea. Aucun code, scenario, build ni commit. Seul ecrit : la ligne `recette:` de Retours\E-deploy.md (ajoutee apres celle de ELROND-1122d) et ce rapport. Les rapports ELROND-1122* n'ont pas ete lus avant le jugement.

Impact joueur : un engin demonte donne un ordre sur une cible lointaine : il marche, se monte, tire ; un deplacement fini le monte tout seul. Vu dans le journal et a l'ecran sur le Mangonneau, mais avec un geste interne.

## (a) preflight deploy (une fois)
`Tools\Scenarios\preflight.ps1 deploy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` : code 0, `preflight deploy : PASS`, `check deploy : PASS (86 s)`, duree totale 92 s, 19 PNG. Lignes FAIL : 0. Lignes WARN : 0. Notes : `20 'Condition failed'` des tests moteur UnifiedErrorTests (-culture=fr, non comptes par l'outil) ; `OK no warning or error in the log`, `OK sanity audit passed`. Log : Saved_Tests\Logs\scenario_deploy.log (horloge du journal = heure locale - 1 h : 16:02 = 17:02).

## (b) les 19 PNG (ouverts avec Read en 4 planches 2x3 de 960x540, ordre gauche-droite puis haut-bas ; les planches sont dans le scratchpad de la session)
- deploy_packed : VU. Mangonneau selectionne, D = Monter, Maison intacte.
- deploy_deploying : VU. D = Montage [#-------] (1 diese), engin pas encore monte.
- deploy_deployed : VU. D = Demonter.
- deploy_fire : VU. Maison 555/600.
- deploy_fire_hit : VU. Maison 510/600.
- deploy_packing : VU. D = Demontage [#-------] (1 diese ; le .look attend 3, il dit « au moins 1 »).
- deploy_packed_again : VU. D = Monter.
- deploy_move_start : VU. D = Demonter ; camera differente (4200 0).
- deploy_move_unmounting : VU. D = Demontage [##-----], trait de destination jaune, engin pas parti.
- deploy_move_walked : VU. Engin decale ~80 px a gauche (~530 cm) de move_unmounting (meme camera), D = Monter. DOUTEUX : le .look dit « plus bas qu'a deploy_move_start » alors que les deux cameras different ; la preuve est la comparaison avec move_unmounting, et le mouvement est vers la gauche (-Y), pas vers le bas.
- deploy_arrive_mounting : VU. D = Montage [-------] (barre vide), aucun nouvel ordre ; engin pres des rochers.
- deploy_arrive_mounted : VU. D = Demonter.
- deploy_attack_mounting : VU. D = Montage, trait rouge vers la Maison.
- deploy_attack_mounted : VU. D = Demonter.
- deploy_attack_hit : VU. Maison 465/600 (3 coups de 45).
- deploy_far_start : VU. Engin demonte (D = Monter) a droite, 2e Maison a gauche, Maison 1 en haut a droite.
- deploy_far_walking : VU avec reserve. D = Monter, pas d'activite Attaque ; l'engin n'a bouge que d'~60 px (~400 cm) de far_start : la marche est visible mais modeste, la preuve de marche est le journal.
- deploy_far_midway : VU. D = Demonter, activite Attaque, ligne rouge horizontale vers la 2e Maison ; engin ~170 px (~1100 cm) a gauche de far_walking.
- deploy_far_end : DOUTEUX. Meme position que far_midway, D = Demonter, Attaque ; les PV de la 2e Maison ne sont pas affiches (non selectionnee) : aucun PNG ne montre un coup sur la 2e Maison.

## (c) journal (scenario_deploy.log)
- Ordre lointain (`Order: 1 units attack House of slot 1`) : 16:02:26.817. Engin demonte au repos pres de (-13390, -1011) en coordonnees absolues ; 2e Maison creee en (-13200, -6400). Distance de depart centre a centre : ~5390 cm (calcul, la position de l'engin n'est pas dans le journal).
- `Deploy: #13 phase 0 -> 1 (order 1, action 2, reason 0, distance 3176 cm, walk 2)` : 16:02:38.364 = 11,55 s apres l'ordre ; `delay over` 16:02:39.811 ; 1er coup (`hits a building for 45`, step 1161) 16:02:42.263 (3,9 s apres la phase 0 -> 1) ; 2e coup 16:02:47.263 (+5,0 s).
- Longueur de marche : 11,5 s a 1,8 m/s = ~2080 cm ; 5390 - 3176 = ~2210 cm si « distance » est centre a centre ; mais pour la 1re attaque (engin a ~2410 cm de la Maison 1 en centre a centre) le journal dit `distance 1841 cm` : la « distance » n'est donc pas le centre a centre (~570 cm de moins), ce qui donnerait ~1650 cm de marche. Les deux lectures vont de 1650 a 2210 cm ; le journal ne les departage pas.
- Moitie B : `Order: 1 units to (-13390, -1011)` 16:02:00.880, pack up 16:02:01.8, puis AUCUN ordre ni D, et `Deploy: #13 phase 0 -> 1 (order 3, action 2, reason 0, distance 0 cm, walk 0)` 16:02:15.665 (14,8 s apres l'ordre), `delay over` 16:02:17.116, HUD `D=Pack` 16:02:18.1. Code lu : HOLDeployProcessor.cpp:98-105, fin de marche avec phase Packed et PendingOrder == 0 -> PendingOrder = 3 (Toggle), sans condition d'ennemi a portee.
- Erreurs Angelscript : 0 (LogAngelscript Error 0, Warning 0). 20 lignes `LogAutomationTest: Error: Condition failed` = tests moteur, hors scenario.
- Sanity : OK (1259 steps, 0 stall, 0 no-strike). Fin : `Scenario quit` normale.

## (d) ce que l'essai ne prouve PAS
1. Le geste : `hol.Select.OrderGround` appelle Controller->OrderAt (meme fonction que le clic) mais n'est pas un evenement souris OS ; la resolution cible sous le curseur (picking) n'est pas exercee. `hol.UI.Key D` est un evenement Slate, pas une touche OS. Donc LOGIQUE SEULEMENT (regle 18) pour les deux moities.
2. Un seul type d'engin (Mangonneau, profil du Trebuchet par defaut, HOLDeployProcessor.cpp:63) alors que D877 vise tous les engins montables.
3. Cible batiment seulement ; pas d'unite ennemie, pas de cible qui bouge, pas de cible qui meurt pendant la marche.
4. Moitie B : un seul cas (fin de marche avec une Maison ennemie a portee, 2410 cm) ; pas de fin de marche sans ennemi (le code ne le conditionne pas, mais l'essai ne le montre pas) ; pas de marche interrompue (Stop, D882 : Stop laisse l'engin demonte, non teste), pas d'ordre de marche donne a un engin deja demonte-en-marche (bMarching deja a 1), pas de file d'ordres (Maj).
5. Moitie A : la cible touchee par les coups lointains n'est lue nulle part : `hits a building` ne nomme pas la cible ; conclusion par la geometrie (Maison 1 a ~3900 cm de l'engin, hors portee) et par le trait rouge de far_midway, pas par les PV de la Maison 2.
6. Montage a 3176 cm alors que la portee batiment annoncee est 3000 : ecart non explique (rayon du batiment ? ) ; les coups ont bien porte.
7. Le son (-nosound), reseau (essai local, pas de client), un jeu de deux joueurs, la marche visible a l'oeil (les deux captures de marche montrent un deplacement de 400 a 1100 cm).
8. Resultat obtenu en UN rejeu (preflight) : pas de repetition ; l'horloge des rejeux anterieurs est serree (A 16 s puis 11,5 s selon le rejeu).

## VERDICT
- Moitie A (cible lointaine : marche, montage, tir) : LOGIQUE SEULEMENT. Preuve : effet vert au journal (phase 0 -> 1 a 11,5 s, distance 3176 cm, coups de 45 a +3,9 s et +8,9 s), far_walking (demonte en marche) et far_midway (monte, Attaque) vus ; geste interne, PV de la cible lointaine non lus.
- Moitie B (fin de deplacement : montage auto) : LOGIQUE SEULEMENT. Preuve : `phase 0 -> 1 (order 3)` 14,8 s apres l'ordre de marche, sans nouvel ordre ; arrive_mounting (Montage en cours) et arrive_mounted (Demonter) vus ; geste interne ; un seul cas.
- Ni FAIL ni PASS plein : aucune defaillance de l'effet n'a ete provoquee, mais la regle 18 interdit de compter l'essai pret. Fiche inchangee : titre PAS PRET (la session principale le change).

## Comparaison avec les rapports ELROND-1122* (lus apres)
Lus apres jugement (ELROND-1122a et d, extraits). Concordant : distance au montage 3176 cm (eux 3172-3177), 11,5 s entre l'ordre et phase 0 -> 1 (eux 11,5 s au rejeu 2), coups de 45 espaces de 5 s, geste interne, position de depart de l'engin non imprimee par le journal (meme limite, marche ~2070 cm a 1,8 m/s ou ~2300 geometrique). Difference : ELROND-1122d dit que preflight n'avait jamais fini PASS sur l'etat final ; ici PASS au premier essai (92 s) sur 57082ea. ELROND-1122d explique 3177 > 3000 par MaxRangeCm 5000 (HOLDeployProcessor.cpp:127-131) : l'ecart 6. est donc seulement l'ecart portee d'attaque (3000) / portee de montage (5000) du Decide, non une anomalie prouvee ; je n'ai pas verifie ce point dans le code.

## Champs
APPELS_DECLARES: ~15 ; HEURE_DEBUT/HEURE_FIN: 17:01:07 / 17:05 ; PROCESSUS_DETACHES: aucun (stray_processes.ps1 -Stop sur le worktree Elrond : `aucun processus detache`) ; ANOMALIES_PROCESSUS: aucune ; TESTS_TAUTOLOGIQUES: sans objet (aucun test ecrit).
