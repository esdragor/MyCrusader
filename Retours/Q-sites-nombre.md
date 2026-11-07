# [À RÉPONDRE] Sites sacrés : combien de sites par carte (D697 : « sans formule », à toi de poser)

Contexte : le générateur de cartes pose maintenant des sites sacrés (`sites(recipe)` dans les huit cartes, `Tools\Tests\sites_test.py` vert sur 8 cartes x 5 tailles : symétrie par un tour d'une place, hors des bases, des relais, de l'autel et des coulées de lave). Il faut un nombre par carte ; la valeur ci-dessous est PROVISOIRE (rien n'est posé dans les cartes .umap, la classe de données C++ n'existe pas encore).

Options :
- **A (défaut codé)** : un site par joueur, en anneau entre deux bases (au milieu de l'arc, plus loin du centre que les relais) : 2, 4, 6, 8 sites selon la taille. Conséquence visible : chaque voisin de base a un site « entre deux » à disputer ; compte à rebours de 300 s avec 2 à 8 sites à tenir en même temps.
- B : A plus une paire symétrique (2 sites de plus, tournant de 180 degrés) : 4, 6, 8, 10 sites. Plus de choses à tenir, partie sacrée plus longue.
- C : deux anneaux (2 x joueurs) : 4, 8, 12, 16 sites. Très dense sur les grandes cartes.
- Autre : un nombre différent par carte (Forêt moins, Couronne plus, etc.).

Recommandation : A (le plus simple à lire et à défendre ; l'équité vient de la symétrie, pas du nombre, D697).

Valeur par défaut codée : `SITE_RINGS = 1` dans `Tools\Editor\maps\common.py` (une ligne à changer ; un anneau de plus s'obtient par `common.ring_sites(..., rings=2)` dans la carte concernée). Rayon : `SITE_RADIUS = 1200` cm (comme le relais), à confirmer aussi.

**Reponse (utilisateur, 2026-11-07 15:4x) : Autre** - 'selon la map, 2 a 3 de base'. Precision demandee et donnee : **2 a 3 sites AU TOTAL par carte** (pas par joueur), le nombre exact varie selon la carte (pas de SITE_RINGS = 1 par joueur). A coder : sites(recipe) avec un nombre fixe par carte (2 ou 3), symetrique, test de symetrie conserve ; rayon 1200 cm non conteste (provisoire).
