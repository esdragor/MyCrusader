# [À RÉPONDRE] Q-sites-grisage : d'où l'écran « Nouvelle partie » sait-il qu'une carte a des sites sacrés ?

Contexte : la case « Sites sacrés » doit être grisée sur une carte sans couche de sites (S7, D699), mais le nombre de sites n'est connu qu'au chargement de la carte, après le salon.
Aujourd'hui toutes les cartes du jeu posent 2 sites (votre réponse Q-sites-nombre : « 2 à 3 selon la map », défaut 2) : la case n'est donc grisée sur aucune carte de la liste.

- **A (recommandé, codé par défaut)** : le tableau des cartes du code (`FHOLMapInfo::NumSites`, `Source/HeroesOfLiberty/Map/HOLMapRules.cpp`) porte le nombre de sites que le générateur pose (2 partout, comme `common.py`). Une carte à 0 est grisée. Conséquence visible : aucune carte grisée aujourd'hui ; une carte pas encore régénérée avec ses sites montre la case active, qui n'active rien (le jeu exige des sites présents).
- **B** : lire la vraie couche de sites du fichier de la carte (une étiquette écrite à la génération, lue sans charger la carte). Conséquence : toute carte pas encore régénérée est grisée ; plus de travail (générateur + lecture).
- **C** : ne jamais griser ; la case cochée sur une carte sans site n'a simplement aucun effet (déjà le cas).
- **Autre** : à préciser.

Recommandation : **A** (une ligne par carte à changer le jour où une carte n'a pas de sites ; le jeu reste sûr grâce à la vérification au chargement).
Valeur par défaut codée : A (`NumSites = 2` dans `HOLMapRules.h`, une valeur par carte dans `MakeMaps`).
Posée par GLORFINDEL-1115a, 2026-11-15, chantier `SacredSitesInProgress.md` (S7).

Réponse :
