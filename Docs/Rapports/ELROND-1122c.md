APPELS_DECLARES: ~28 (au-dela de 25 : lectures de verbes du sequenceur plus longues que prevu)
HEURE_DEBUT/HEURE_FIN: 16:45:43 / 16:51
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: preflight deploy 16:49 ROUGE (timeout step 155 waitlog phase 0 -> 1 (order) apres reordonnancement naif ; assertit l'effet : oui)   TEST_ROUGE_SEPARE: non (scenario, pas de code)
COMMITS: 2 (086fcb8, dd42862) sur integ-25, 16:50, merge_main_fait: non
TESTS: preflight deploy 1x : FAIL (timeout step 155) ; correctif ensuite NON REJOUE ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: deploy : preflight ROUGE puis non rejoue, PNG far_walking et far_midway ouverts (autres non rouverts : step 155 avant eux, anciennes captures), LOGIQUE SEULEMENT

CE QUI EST FAIT
- OrderNear remplace par hol.Select.OrderGround (3 appels : -13400 -1000 deplacement, -11100 0 attaque Maison 1, -13400 -5500 attaque lointaine). Lu dans HOLControllerCommandsC10a.as:291-335 : OrderNear et OrderGround appellent la meme fonction, Controller->OrderAt(fraction), la fonction du clic souris ; seule difference = le repere du point (OrderNear relatif au centre, absolu = near - 16400 en X, deduit des journaux 15.33/19.24 : (3000,-1000)->(-13400,-1000), (5300,0)->(-11100,0), (3000,-5500)->(-13400,-5500)). GESTE reste `interne` : aucun verbe du sequenceur n'emet un evenement souris OS ; neutrals_qground est reel seulement pour la touche (Select.Grid) et le clic gauche via LeftClickGround, qui est aussi un appel de controleur. Aucun verbe clic droit au sol different de OrderGround/ClickUnit order n'existe.
- Premier essai de preflight : ROUGE au step 155 (waitlog phase 0 -> 1 expire). CAUSE (vue) : avec les captures far_walking/far_midway AVANT les waitlog (wait 3 + wait 6), le montage (phase 0 -> 1) a lieu pendant les wait, avant que waitlog ne demarre, donc la ligne n'est jamais revue (waitlog ne relit pas le passe). Contrainte reelle : ces deux waitlog doivent suivre IMMEDIATEMENT l'ordre ou etre places avant la capture qui les depasse.
- far_walking (PNG ouvert, 3 s apres l'ordre) : Mangonneau DEMONTE et en marche (case D = Monter, vitesse 1,8 m/s, pas d'activite Attaque), a droite de la 2e Maison (a gauche), pas encore a portee. C'est bien l'engin demonte en marche.
- far_midway (PNG ouvert, 9 s apres l'ordre) : Mangonneau MONTE (case D = Demonter, activite Attaque, ligne rouge vers la Maison), position ~100 px a gauche de far_walking : la marche est tres courte (montage a 3177 cm pour une portee de 3000 : ~180 cm a parcourir + demontage 1 s) ; il ne peut pas etre vu 'en marche' a 9 s.
- Correctif final (commit 086fcb8, NON REJOUE) : waitlog phase 0 -> 1 puis waitlog Mangonel hits a building places ENTRE far_walking (wait 3 apres l'ordre, demonte et en marche) et far_midway (apres les deux waitlog : monte et tire) ; far_end 5 s apres. Toutes les assertions gardees.
- deploy.look (dd42862) : far_walking decrit comme vu (demonte en marche) ; far_midway decrit comme attendu (monte, Attaque), marque NON REVU. far_start, far_end, deploy_arrive_* : non rouverts (le run s'est arrete au step 155 avant far_end ; far_end absent, arrive_* anciens PNG d'ELROND-1122b).
- Retours\E-deploy.md : GESTE: interne (mis a jour, honnete) ; ligne recette: ajoutee ; reste PAS PRET.

RESTE : rejouer preflight deploy une fois (le reordonnancement doit rendre le run vert ; verifier que le waitlog phase 0 -> 1 attrape bien le montage entre 3 s et 9 s) ; rouvrir far_start, far_midway, far_end, deploy_arrive_* ; une vraie souris n'existe pas dans le sequenceur.
NON VERIFIE : correctif de reordonnancement (aucun rejeu) ; far_start, far_end, deploy_arrive_* non vus cette seance ; son non entendu ; EtapeDefini.md, Auto_Journey_File.md, Palier3InProgress.md NON mis a jour (budget d'appels depasse) : a faire par la session principale.
PROCESSUS_DETACHES: non lance (preflight termine de lui-meme)
ANOMALIES_PROCESSUS: aucune vue
GIT_STATUS_PRINCIPALE: non relu ; seuls Retours\E-deploy.md et ce rapport touches par moi dans la copie principale