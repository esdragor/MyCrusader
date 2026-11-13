# E-deploy : la Mangonnelle (substitut du Trebuchet) se monte et se demonte par la touche D (case D : Monter / Demonter)

STATUT : LOGIQUE SEULEMENT (geste reel et 4 PNG vus, MAIS la question C:\Users\baudo\Documents\MyCrusader\Retours\Q-deployed-engine-autofire.md n'a pas de reponse : l'essai montre le comportement du defaut code hol.Siege.DeployedAutoFire 0 = A, l'engin monte ne tire que sur ordre ; si la reponse est B ou C, le scenario change)
GESTE: réel         (vraie touche D = hol.UI.Key D, evenement clavier Slate ; vrai clic droit = curseur de test pose sur la Maison par hol.Camera.TestCursorGround -9600 0 puis hol.UI.Key RightMouseButton, appui et relachement par HandleOrder / HandleOrderCompleted)
SON: non entendu    (-nosound)
branches: tiers

Pre-vol 2026-11-13 18:57 (ELROND-1113f, tiers, binaire du build PASS 18:21) : scenario PASS en 49 s (ready, aucun avertissement, HUD sans recouvrement, sanity) ; preflight FAIL (1) pour la seule raison 'deploy.txt non commite' (commite ensuite, voir Palier3InProgress.md). 4 PNG ouverts et vus (planche 2x2) : deploy_packed (case D = Monter, Maison rouge a portee intacte), deploy_deployed (case D = Demonter, l'engin ne tire pas), deploy_fire (camera recentree entre l'engin et la Maison : la Maison est VISIBLE sous le panneau des notes, trait de tir rouge de l'engin vers la Maison, statut Attaque, case D = Demonter ; les PV de la Maison ne sont pas affiches en chiffres, le coup de 45 est prouve par le journal), deploy_packed_again (case D = Monter).
Commande pour l'utilisateur (quand la question sera tranchee) : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 deploy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond

Pre-vol 2026-11-13 18:35 (ELROND-1113e, tiers 8762f6c, build PASS 18:21) = PASS. 4 PNG ouverts et vus : deploy_packed (case D = Monter, Maison rouge a portee intacte), deploy_deployed (case D = Demonter), deploy_fire (case D = Demonter, statut Attaque ; le tir est prouve par le journal 'Mangonel hits a building for 45' mais la Maison est CACHEE par le panneau des notes sur le PNG : PV de la Maison non vus a l'ecran), deploy_packed_again (case D = Monter).
Cause du rouge 16:45 : l'engin monte au repos ne tire pas seul (AggressionRadius 0 = seulement sur ordre) : donnee, pas un bug. Question C:\Users\baudo\Documents\MyCrusader\Retours\Q-deployed-engine-autofire.md ; reglage hol.Siege.DeployedAutoFire 0 (defaut) / 1 ; le scenario donne un clic droit sur la Maison.
Reste avant [A ESSAYER] : vraie touche D et vrai clic droit (modele neutrals_qground), camera de deploy_fire qui montre la Maison sous le panneau.

Pre-vol 2026-11-13 13:56 (ELROND-1113c, tiers 27d91a4, build 13:52) = FAIL (6), 93 s, timeout etape 35 'waitlog D=Pack'.
- Le journal ajoute au HOLDeployProcessor montre que l'engin SE MONTE : 'Deploy: #13 phase 0 -> 1 (order 3...)' puis 8 s apres 'phase 1 -> 2 (delay over)'.
- Le defaut est dans le SCENARIO : hol.HUD.Report ne dit pas 'D=Pack' une fois monte (probablement le nom de commande fixe). A corriger dans deploy.txt (waitlog sur la ligne 'Deploy: #') ou dans le rapport HUD.
- La piste 'Maison a 5250 > portee 5000' est fausse : la Maison est a ~2200 cm.
- Correctif voisin fait : D presse pendant une marche ne s'annule plus (27d91a4).

NON VERIFIE : tir sur la Maison, demontage, PNG (seul deploy_packed produit, NON OUVERT).

Pre-vol 2026-11-13 16:45 (ELROND-1113d, tiers, build 16:43) = FAIL (6), PLUS LOIN : la case D dit bien 'Demonter' une fois monte (correctif HUD, PNG deploy_deployed ouvert et vu) ; nouveau rouge etape 38 : l'engin monte ne tire pas sur la Maison en 60 s (non diagnostique). Statut inchange : LOGIQUE SEULEMENT, NE PAS JOUER.
