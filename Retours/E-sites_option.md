# E-sites_option : la case « Sites sacrés » (Non / Oui) de l'écran Nouvelle partie active les sites sacrés et leur victoire

STATUT : LOGIQUE SEULEMENT   (le choix Non -> Oui est fait par `hol.UI.Choose SacredSitesDropdown`, pas par un vrai clic ; le point 1 est joué avec hol.Site.Enabled 0 et la case cochée)
GESTE: reel (GALADRIEL-1115d : clics hol.Mouse.Click, Non par Haut+Entree, Oui par vraies touches Entree, Bas, Entree ; check 4/4, preflight PASS 12:03)
Essai GALADRIEL-1115b (2026-11-15 09:47-09:58, galadriel-tab) : Oui par vraies touches (Entree sur la liste, Bas, Entree apres le choix clavier de Non) = INSTABLE : check PASS 09:47 (1 fois), preflights 09:52, 09:56, 09:58 en echec (la 1re Entree n'ouvre pas la liste, timeout sur list opened), avec ou sans survol de la souris avant ; controle sans Bas : timeout sur value 1 chosen (le test assertit bien l'effet). Scenario remis a hol.UI.Choose (98ff3df). Cause a chercher cote UHOLDropdown / Common UI (focus apres Choose -> CloseList -> SetFocus), C++ : non fait.
SON: non entendu    (-nosound)
branches: aucune (main)

Recette indépendante 3 23:07 sur main 091df3c : LOGIQUE SEULEMENT (cause : geste interne `hol.UI.Choose`, pas de vrai clic sur la liste ; preflight PASS 34 s, WARN sans hol.Test.Sanity), PNG vus : sites_option_on (« Victoire sacrée : Vous dans 1:04 » entier sous Triches, 2 paires de Moines bleus minuscules, aucune zone de site dessinée au sol), sites_option_box_off (« Sites sacrés » sur « Non », actif), sites_option_box_on (« Oui ») ; réserves : sur l'écran Nouvelle partie, Nomade et Régicide restent GRISÉS « pas encore dans le jeu » alors que main joue maintenant un départ Nomade (nomad_start) et le Régicide (Roi) : libellé à revoir ; le titre du panneau court se replie en deux lignes (« en / préparation… »).
Recette indépendante 2 21:26 sur main 15bbd25 (worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, branche fog) : `preflight.ps1 sites_option -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` PASS (31 s, scénario ready en 17,4 s, 3 PNG). WARN du preflight : le scénario n'appelle pas hol.Test.Sanity (seulement hol.Sanity.Watch 1). 3 « LogAutomationTest: Error: Condition failed » au démarrage = test d'erreur unifiée du moteur (UE::UnifiedErrorTest), sans lien avec l'essai.
PNG OUVERTS et vus :
- sites_option_on : sous le panneau Triches, la ligne « Victoire sacrée : Vous dans 1:04 » (lisible, non tronquée) ; 2 paires de Moines bleus, à gauche et à droite du centre. Les 2 sites eux-mêmes ne sont PAS dessinés au sol (aucun cercle, aucun marqueur de zone) : on voit les Moines, pas la zone qu'ils tiennent.
- sites_option_box_off : écran Nouvelle partie, ligne des options Mode / Nomade / Régicide / Catastrophe (grisés) puis « Sites sacrés » sur « Non » (actif, non grisé) ; rien de tronqué.
- sites_option_box_on : même écran, « Sites sacrés » sur « Oui ».
NON VÉRIFIÉ : vrai clic sur la liste (geste interne) ; que « Lancer » avec Oui donne bien la partie du point 1 (le point 1 est préparé par commandes, pas lancé depuis cet écran) ; la victoire elle-même à 0:00 (le compte à rebours est vu à 1:04, pas son terme) ; son.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 sites_option -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
