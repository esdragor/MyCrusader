# E-sites_option : la case « Sites sacrés » (Non / Oui) de l'écran Nouvelle partie active les sites sacrés et leur victoire

STATUT : LOGIQUE SEULEMENT   (le choix Non -> Oui est fait par `hol.UI.Choose SacredSitesDropdown`, pas par un vrai clic ; le point 1 est joué avec hol.Site.Enabled 0 et la case cochée)
GESTE: interne      (hol.UI.Choose SacredSitesDropdown 0 / 1 : appel interne, pas un clic sur la liste)
SON: non entendu    (-nosound)
branches: aucune (main)

Recette indépendante 2 21:26 sur main 15bbd25 (worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, branche fog) : `preflight.ps1 sites_option -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` PASS (31 s, scénario ready en 17,4 s, 3 PNG). WARN du preflight : le scénario n'appelle pas hol.Test.Sanity (seulement hol.Sanity.Watch 1). 3 « LogAutomationTest: Error: Condition failed » au démarrage = test d'erreur unifiée du moteur (UE::UnifiedErrorTest), sans lien avec l'essai.
PNG OUVERTS et vus :
- sites_option_on : sous le panneau Triches, la ligne « Victoire sacrée : Vous dans 1:04 » (lisible, non tronquée) ; 2 paires de Moines bleus, à gauche et à droite du centre. Les 2 sites eux-mêmes ne sont PAS dessinés au sol (aucun cercle, aucun marqueur de zone) : on voit les Moines, pas la zone qu'ils tiennent.
- sites_option_box_off : écran Nouvelle partie, ligne des options Mode / Nomade / Régicide / Catastrophe (grisés) puis « Sites sacrés » sur « Non » (actif, non grisé) ; rien de tronqué.
- sites_option_box_on : même écran, « Sites sacrés » sur « Oui ».
NON VÉRIFIÉ : vrai clic sur la liste (geste interne) ; que « Lancer » avec Oui donne bien la partie du point 1 (le point 1 est préparé par commandes, pas lancé depuis cet écran) ; la victoire elle-même à 0:00 (le compte à rebours est vu à 1:04, pas son terme) ; son.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 sites_option -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
