# [À ESSAYER] E-sites_win : Victoire sacrée : le bandeau « Victoire sacrée : Vous dans m:ss » à gauche, ses annonces à 1 min et 10 s, puis l'écran de victoire

STATUT : VU À L'ÉCRAN (objet de l'essai = l'AFFICHAGE du bandeau et de ses annonces, pas un geste)
GESTE: interne      (`hol.Units.Spawn`, `hol.Site.*`, aucune touche ni clic : les Moines sont posés par cheat)
SON: non entendu    (-nosound ; la cloche d'attente « AlertVolcano » des annonces est provisoire et n'a pas été entendue)
branches: net-playtest

Règle (L7-6) : essai d'affichage ; ni le geste ni le son ne sont jugés.

Pré-vol (2026-11-09 10:51, agent Elrond-integration, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, main 60013e8, rejoué sur le build de main fusionné) : `preflight.ps1 sites_win -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (105 s dont scénario environ 85 s, 5 PNG). Seul WARN : pas d'audit `Sanity:`.
PNG OUVERTS et vus :
- sites_count : les deux sites sacrés à gauche et à droite de la vue, 2 Moines bleus à chacun, bandeau sous Triches « Victoire sacrée : Vous dans 1:04 »
- sites_frozen : zoom sur un site, 2 Moines bleus et un Lancier rouge au même endroit, bandeau « Victoire sacrée : Vous dans 1:04 (en attente) » (l'horloge ne descend plus tant qu'un adversaire est dans la zone)
- sites_announce60 : bandeau « Vous dans 0:59 » et en dessous « Sites sacrés : Vous gagne dans une minute »
- sites_announce10 : bandeau « 0:08 » et « Sites sacrés : Vous gagne dans 10 secondes »
- sites_won : bandeau « Victoire sacrée : Vous » et fenêtre « Victoire » (Continuer / Quitter vers le menu / Statistiques), IA Normal 2 « Vaincu »
Constat : les notes du scénario (panneau « en préparation… ») couvrent le haut de l'écran mais pas le bandeau ; le panneau reste sur « en préparation… ». Libellés et annonces provisoires (Placeholders.txt).
NON VÉRIFIÉ : le son de l'annonce ; l'occupation par de vrais clics (Moines envoyés à la main sur un site) ; une partie naturelle ; le comportement en réseau.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 sites_win -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 2 min (scénario d'environ 90 s). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
