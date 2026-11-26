# [À ESSAYER] E-art_lab : une seule commande hol.Art.Lab pose tout l'atelier (galerie, éclairage Herbe fraîche, construction rapide, ressources, combat coupé, caméra sur les unités, aide à l'écran)

STATUT : VU À L'ÉCRAN
GESTE: interne      (commande `hol.Art.Lab` tapée par le scénario)
SON: sans objet
branches: art-lab-1125ar

Pré-vol (2026-11-25 23:33, agent CELEBORN-1125ar12, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, base main f4d3344 + 331ed75) : `preflight.ps1 art_lab -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn` PASS (19 s, 2 PNG).
PNG OUVERTS et vus :
- al_units : galerie des unités au centre (Arquebusier, Peasant, King, Archer, ManAtArms, LightInfantry... étiquettes jaunes au sol, barres rouges 1,80 m), sol vert Herbe fraîche, ressources 1200 partout (triche +1000), aide des 8 commandes hol.Art.* en cyan en haut à gauche ; le haut de l'image montre déjà les premiers bâtiments de la galerie.
- al_buildings : moitié sud des bâtiments (Barracks/Caserne, Farm/Ferme, House/Maison, Siege...), contours cyan d'emprise, poteaux de hauteur, aucune forêt dans la grille, aide toujours affichée.
NON VÉRIFIÉ : `hol.Art.Lab Neige` (autre biome) seulement en test unitaire, pas en capture ; l'aide reste 30 s et recouvre le haut gauche de la galerie (durée à régler si gênante) ; la construction rapide n'est pas montrée en capture (commande lancée, journal « Art lab: step hol.Cheat.FastBuild 1 »).

Commande complète (règle 13, 2026-11-14) :
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Scenarios\play.ps1 art_lab -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
