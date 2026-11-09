# [À RÉPONDRE] Q-sites-arbres-zone : arbres et mines dans la zone d'un site sacré

Contexte : les sites (2 à 3 par carte) sont posés loin des bases, des relais, de l'autel et de la lave, mais PAS des arbres : une forêt peut couvrir une partie de la zone de 12 m, et les arbres sont des obstacles.
La régénération unique des cartes va poser les sites ; le plan (MapsInProgress.md, risque 4) demande de décider quoi faire des sources déjà dans le disque d'un site.

Options :
- A (défaut codé) : ne rien retirer. Le site est où il est, les arbres restent ; certaines zones seront partiellement boisées (visible, prise possible sur le sol libre).
- B : après coup, retirer de la liste des sources celles dont le centre est dans le disque d'un site (comme `drop_reserved` le fait pour les relais, les camps et l'autel) : zone toujours dégagée, le compte d'arbres baisse de quelques unités par site.
- C : réserver le disque avant le tirage des bosquets (comme les relais, `RELAY_ZONES`) : le résultat se décale autour du site, mais cela peut déplacer les tirages et faire échouer les cartes déjà à la limite (Forêt, Cols, Arrière-pays).
- Autre : ta proposition.

Conséquences visibles : A = sites parfois dans un bosquet ; B = clairière nette de 12 m au site, sans autre changement ; C = les forêts se réorganisent autour des sites.
Recommandation : B (zone dégagée sans décaler les tirages).
Réglage : une seule ligne à changer, `SITES_CLEAR_SOURCES = False` (A) dans `Tools\Editor\maps\common.py` (la valeur du code reste A tant que tu n'as pas répondu).
