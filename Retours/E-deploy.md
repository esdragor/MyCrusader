# [À ESSAYER] E-deploy : le Mangonneau (substitut du Trebuchet) se monte et se demonte par la touche D (case D : Monter / Demonter) ; monte, il tire seul sur la Maison ennemie, dont les PV baissent

STATUT : VU À L'ÉCRAN (preflight PASS 2026-11-13 21:40, ELROND-1113 reprise, Faramir fog = tiers 1e32b46, binaire frais de main ; scenario et .look dans main 1639d37).
Le tir est vu par la baisse de PV de la Maison ; il n'y a pas de projectile à l'écran (coup de siège immédiat) : à décider, C++ de présentation.
GESTE: réel (touche D, deux fois : hol.UI.Key D, evenement clavier Slate) + clic de sélection réel (clic gauche sur la Maison, puis clic sur le Mangonneau)
SON: non entendu    (-nosound)
branches: aucune (main)

Pre-vol 2026-11-13 21:40 : `preflight.ps1 deploy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` PASS (scenario ready, 37 s, aucun avertissement, HUD sans recouvrement, sanity, pan exposure OK). 5 PNG OUVERTS et vus :
- deploy_packed : Mangonneau selectionne (200/200, Attaque 45), grille Q Attaque / S Arret / D **Monter** / V Tenir ; la Maison rouge est visible en entier juste sous le panneau court « Scenario deploy en preparation ».
- deploy_deployed : case D = **Demonter**, statut « Attaque » ; panneau court (notereset avant la capture), Maison visible en entier.
- deploy_fire : camera sur la Maison, Maison selectionnee (cadre blanc), panneau « Maison / IA Normal 2 / Points de vie **555 / 600** » ; Maison visible sous le panneau des notes (notes 3 et 3b) ; aucun projectile, aucun trait de tir.
- deploy_fire_hit : meme vue, Points de vie **510 / 600** (un coup de 45 de plus).
- deploy_packed_again : Mangonneau reselectionne, case D redit **Monter**, statut « Attaque » parti ; Maison visible en entier sous le panneau court (note 4).
Le Mangonneau est dessine par le mannequin gris (provisoire). NON VÉRIFIÉ : son, jeu en fenetre par l'utilisateur, projectile (absent).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\Scenarios\play.ps1 deploy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`   (Faramir = fog = main 1639d37, binaire frais : c'est la que le pre-vol a ete joue)
Duree : environ 1 min. Reponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

## Historique
Recette independante 2 21:25 sur main 15bbd25 (Faramir, fog) : preflight PASS (34 s, scenario ready en 18,6 s, HUD sans recouvrement, sanity), PNG vus : deploy_packed (Mangonneau 200/200, case D « Monter », Maison rouge intacte), deploy_deployed (case D « Démonter », statut « Attaque » ; la Maison est a moitie cachee par le panneau des notes), deploy_fire (case D « Démonter », statut « Attaque », Maison visible en entier MAIS aucun trait de tir, aucun projectile, aucune barre de PV ni degat visible sur la Maison : la capture suit le waitlog 'Mangonel hits a building for 45', le coup n'est prouve que par le journal ; le .look promet un « trait de tir vers la Maison » : NON VU), deploy_packed_again (case D redit « Monter », statut « Attaque » parti ; la Maison est cachee par le panneau des notes, « il ne tire plus » non visible). Cause du verdict : regle 9 (effet de gameplay non visible) + point du .look non vu. Pistes (non tranchees) : capturer quelques dixiemes de seconde avant l'impact (projectile en vol), selectionner la Maison ou afficher ses PV au moment de deploy_fire, placer la camera / le panneau pour que la Maison reste visible aux 4 captures. Reserve de nom : les notes disent « Mangonnelle », le panneau de selection dit « Mangonneau » (regle 9 : un meme nom partout). Preflight du scenario final PASS (20:05, voir ci-dessous) ; reponse B a Q-deployed-engine-autofire appliquee (hol.Siege.DeployedAutoFire defaut 1, commit 73adb69, test HeroesOfLiberty.Siege.Deploy.DeployedAutoFire vert, tests 1238/1238, net 16/16).
(ancien statut 21:25 : LOGIQUE SEULEMENT, branches tiers ; remplace par l'en-tete ci-dessus)

Pre-vol 2026-11-13 20:05 (ELROND-1113h, tiers 99f6387, binaire du build PASS 19:57, fenetre cachee) = PASS en 42 s (scenario ready en 18,6 s de jeu, aucun avertissement, HUD sans recouvrement, sanity). Le risque 'premier tir pendant le wait 9 avant le waitlog' ne s'est pas produit : montage fini a la note 3 (34,9 s), waitlog du coup de 45 satisfait 1,6 s plus tard. 4 PNG OUVERTS et vus :
- deploy_packed : Mangonneau selectionne (panneau : PV 200/200, Attaque 45), grille Q Attaque / S Arret / D **Monter** / V Tenir ; la Maison rouge ennemie est au-dessus de l'engin, intacte.
- deploy_deployed : case D = **Demonter** ; le panneau ajoute la ligne de statut 'Attaque'.
- deploy_fire : camera recentree, la Maison rouge est VISIBLE en entier sous le panneau des notes ; case D = Demonter, statut 'Attaque'. Aucun trait de tir ni barre de PV de la Maison n'est visible sur cette image : le coup de 45 est prouve par le journal (waitlog 'Mangonel hits a building for 45'), pas par l'image.
- deploy_packed_again : case D redit **Monter**, la ligne de statut 'Attaque' a disparu (l'engin ne tire plus).
Commande pour l'utilisateur (apres la fusion de tiers) : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 deploy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond

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
