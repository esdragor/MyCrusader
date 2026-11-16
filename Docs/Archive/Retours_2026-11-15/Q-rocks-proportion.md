# [À RÉPONDRE] Rochers des cartes : quelle part de rochers à facettes ? et le sol des autres scénarios

Posée par SAURON-1115a (2026-11-15, branche `visual`, commit 36a4be8). Réponds par « Q1 : A/C/D/Autre, Q2 : A/B/C/Autre ».

## Q1. Part de rochers à facettes sur les cartes (D802)

Contexte : tu as aimé A, C et D de `rocks_compare2` (rochers à facettes dominants). Les cartes portent maintenant les deux sortes de rochers
(à facettes = cubes penchés gris-bleu foncé ; ronds = sphères brun foncé, les couleurs du tour 2) ; la part se règle par une seule ligne.

- **A** : rochers à facettes seuls (8 sur 8) : plus aucun rocher rond sur les cartes (sauf les rochers de sable et de mousse, inchangés).
- **C** : surtout à facettes (6 sur 8, 75 %) : un rocher rond de temps en temps.
- **D** : moitié-moitié (4 sur 8) : les deux formes se voient autant.
- **Autre** : une autre part (dis-la en huitièmes ou en %).

Recommandation : **C** (le milieu des trois, et « facettes dominantes » comme tu l'as dit). **Codé par défaut : C** :
`ROCK_STYLE = "C"` dans `Tools\Editor\maps\common.py` (`ROCK_STYLES = {"A": 1.0, "C": 0.75, "D": 0.5}`).
Pour revoir les trois côte à côte (balayage déjà fait : vues A, C et D ; Entrée = vue suivante, Retour = précédente) :

    powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 rocks_compare2 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron

Conséquence commune : le changement ne se voit en partie qu'après la **régénération des cartes** (éditeur, seul sur la machine), pas encore faite.

## Q2. Les 196 scénarios de jeu sans `mapSize`

Contexte : un scénario sans `mapSize` charge la carte de TEST `Plain.umap` (sol à dalles grises, pas de sol de biome). Les deux revues du décor
(`rocks_compare`, `resources_compare`) ont reçu `mapSize Small` (sol vert vérifié sur `rocks_compare`) ; 196 scénarios de JEU sont restés sur la carte de test.

- **A** : les laisser sur la carte de test : rien ne bouge, leurs essais gardent le sol gris.
- **B** : leur mettre `mapSize Small` à tous : sol de biome partout, mais la carte générée a d'autres arbres, ressources et relais aux mêmes
  coordonnées (vu : un relais et un arbre sur l'emplacement de `rocks_compare`) : certains scénarios peuvent casser, à rejouer un par un.
- **C** : seulement à ceux qu'on te montre (un fichier `.look`), au fil de leurs essais, en rejouant chacun.
- **Autre**.

Recommandation : **C**. **Codé par défaut : A** (rien changé ; `Tools\Tests\scenario_mapsize_check.ps1 -List` donne la liste).

Reponse (2026-11-15) : Q1 C : 6 rochers à facettes sur 8 (75 %, ROCK_STYLE C, défaut codé). Q2 C : mapSize Small seulement aux scénarios montrés à l'utilisateur, au fil de leurs essais, en rejouant chacun (les autres restent sur la carte de test). (réponse donnée dans la conversation, 2026-11-15) Suite : reportée dans le GDD (D820) et le journal du chantier.
