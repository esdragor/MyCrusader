# RECETTE-1121se (21:56 - 22:10, arret dur 22:12) : deploy (D877) et secondary_centre sur main 1276d54

APPELS_DECLARES: ~19 (arret 25)
HEURE_DEBUT/HEURE_FIN: 21:56:49 / 22:09:12
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (recette) ; TESTS: preflight deploy PASS (61 s et 68 s, 13 points), preflight secondary_centre PASS (37 s, 5 points) ; TESTS_TAUTOLOGIQUES: sans objet

## 1) deploy (D877) : FAIL, worktree Elrond (integ-25 = main 1276d54)

Reponse de l'utilisateur : « clic sur cible en demonte : se mettre a portee si besoin, se monter, tirer ; fin d'un deplacement : montage auto ».

Preflight : PASS. Planches de contact des 17 PNG ouvertes (13 points du .look + 4 deploy_far_*, ces 4 n'ont AUCUN point dans deploy.look : le scenario les capture, personne ne les regarde).

Moitie A (clic sur cible lointaine, demonte -> approche, montage, tir) : FAIL.
- Ordre accepte (journal 20:59:13 « Order: 1 units attack House of slot 1 »), l'engin marche (order lines 1, deploy_far_walking et deploy_far_midway : il se rapproche de la Maison).
- 15 s plus tard (deploy_far_end) : case D = « Monter » (HUD report « D=Deploy »), AUCUNE ligne « Deploy: #13 phase 0 -> 1 » depuis l'ordre, AUCUN « Mangonel hits a building ». Maison a 4500 cm, portee 3000, il faut ~1500 cm de marche (~8 s a 1,8 m/s) : le montage aurait du commencer avant 15 s.
- Le scenario livre n'a aucune assertion sur ce cas (note 12 « Attendu : ... » sans waitlog) : il passe au vert sans prouver l'effet.
- Replay de verification avec 45 s d'attente : bloque en amont (voir 2e point), la moitie A n'a donc pas pu etre rejouee plus longtemps. Reste NON PROUVEE, conclusion FAIL (l'utilisateur la verrait : il marche, ne se monte pas, ne tire pas dans la fenetre prevue).

Moitie B (fin d'un deplacement -> montage auto) : PASS au journal, FAIL dans le scenario livre.
- deploy_move_walked (PNG vu) : capture 4 s apres le demontage, engin encore en marche (order line 1), case D = « Monter » : ne montre pas la fin du deplacement.
- Replay (copie privee du scenario avec « wait 20 » apres move_walked, supprimee ensuite) : « Deploy: #13 phase 0 -> 1 (order 3, action 2, distance 0 cm, walk 0) » a 22:07:32, 10 s apres move_walked, SANS aucun ordre, puis « delay over » 22:07:33, tirs « Mangonel hits a building » des 22:07:36, rec_move_end : case D = « Demonter » (HUD cells). Le montage auto a l'arrivee existe donc. Reserve : « order 3 / reason 0 » est le meme code que la touche D, non distingue d'un autre declencheur (tir auto) ; PNG rec_move_end non ouvert (journal seulement).
- Le scenario livre ne le montre pas : le clic d'attaque de l'etape 9 interrompt la marche avant l'arrivee.
- Consequence du replay : l'etape 9 du scenario (waitlog « (delay over) ») a expire (step 107) parce que l'engin etait deja monte : le scenario depend du FAIT que l'engin n'est PAS monte a ce moment, il serait a corriger pour inclure la moitie B (arrivee, capture, puis ordre d'attaque).

Corrections de scenario a faire (pas du code) : (1) ajouter a deploy.look des points deploy_far_start / far_walking / far_midway / far_end et un waitlog « Deploy: .* phase 0 -> 1 » + « hits a building » apres l'ordre lointain (timeout 30 s), (2) ajouter une capture a l'arrivee du deplacement (attente que « order lines 0 »), D = Demonter attendu.

Statut de la fiche : reste sans [À ESSAYER] / VU A L'ECRAN pour D877 (FAIL). Ligne `recette: RECETTE-1121se FAIL ...` ajoutee a Retours\E-deploy.md. NB : la commande play.ps1 de la fiche pointe sur le worktree Arwen (non teste ici) ; commande jouee ici :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 deploy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`

## 2) secondary_centre : PASS, worktree Haldir (wall-line = main 1276d54)

Preflight PASS (37 s, sanity ok). 5 PNG ouverts (planche) :
- sc_grille_age2 : 5 Paysans, page II allumee, case Z « Centre secondaire », ressources 5200, Population 5/10.
- sc_grille_infobulle : infobulle visible a la case Z : « Centre secondaire, bois 150, pierre 400, +10 population, Depot : nourriture, bois, pierre, or, Produit : Paysan ».
- sc_pendant : fondation, 5 Paysans au pied, « 5 en construction », bois 5050 / pierre 4800, Population 5/10.
- sc_apres : centre fini seul selectionne, 2400/2400 PV, seule la case A Paysan, « Batiment acheve », Population 5/20.
- sc_forum : Centre principal (A Paysan, D/F voies palier 1, W cloche) ; difference de grille deja en Retours\Q-secondary-centre-grid.md.
Ligne `recette: RECETTE-1121se PASS ...` ajoutee a Retours\E-secondary_centre.md. GESTE reste « reel » selon la fiche (non reverifie ici).
Commande :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 secondary_centre -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`

QUESTIONS: aucune
FAIT / RESTE: voir ci-dessus ; reste : corriger deploy (moitie A, comportement ou delai) et le scenario ; rejouer.
NON VERIFIE: son ; moitie A du deploy sur plus de 15 s (replay bloque par le waitlog de l'etape 9) ; PNG rec_move_end ; vraie touche/vrai clic pour deploy (appels internes) ; l'ordre « Deploy: ... order 3 » comme declencheur du montage auto.
PROCESSUS_DETACHES: aucun (stray_processes -Stop lance sur Elrond et sur Haldir : « aucun processus detache »). Un premier play.ps1 deploy_recette a ete lance (timeout, hero non spawn : deploy.json manquant pour le nom copie) puis un check.ps1 ; copies deploy_recette.txt/.json supprimees d'Elrond.
ANOMALIES_PROCESSUS: une tentative Stop-Process sur UnrealEditor refusee par le classifieur (non contournee) ; Elrond Saved\Logs\HeroesOfLiberty.log et Saved_Tests\Logs\scenario_deploy_recette.log ecrases par mes replays.
GIT_STATUS_PRINCIPALE: non modifie par moi hors Retours\E-deploy.md et Retours\E-secondary_centre.md (une ligne `recette:` chacun) et ce rapport.
