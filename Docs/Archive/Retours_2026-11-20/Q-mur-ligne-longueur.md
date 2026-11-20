# [À RÉPONDRE] Q-mur-ligne-longueur : combien de cases au plus pour une file de murs posée d'un glisser ?

Contexte : D691 (F4) décide la pose en ligne au glisser (axes et diagonales à 45°, grille de 4 m) mais ne donne aucune longueur maximale ; la validation serveur `HOLWallLine::Validate` (HALDIR-1120h, `Script\Buildings\HOLWallLineRules.as`) refuse tout glisser plus long que le plafond qu'on lui passe.
Aujourd'hui aucune valeur n'est codée : le paramètre `MaxCells` est obligatoire et aucun appelant n'existe encore (pas de geste de glisser), donc rien ne se joue tant que cette réponse manque.

**Question : quelle longueur maximale (en cases de 4 m) pour une file posée d'un seul glisser, et que fait le jeu quand le joueur dépasse ?**

| Option | Ce que le joueur voit en jeu | Conséquence technique |
|---|---|---|
| **A. 16 cases (64 m), glisser plus long refusé** | Le fantôme de la file devient rouge au-delà de 16 cases et rien n'est posé au relâchement ; pour un long rempart il enchaîne plusieurs glissers (Shift). Coût d'une file au plus 16 segments. | Une valeur dans un réglage (cvar ou CSV) lue par l'appelant ; la règle refuse déjà (`TooLong`, tout ou rien). |
| **B. 32 cases (128 m), glisser plus long refusé** | Un rempart de village en un ou deux gestes ; plus de risque d'une file posée par erreur qui vide le stock d'un coup (32 segments payés). | Même mécanisme que A, une autre valeur. |
| **C. Pas de plafond de longueur (seule la carte et le stock limitent)** | Le joueur trace d'un bord à l'autre ; le stock insuffisant ou la carte refuse. Une file de 60 cases coûte 60 segments d'un coup. | La règle exige quand même un nombre : un plafond technique très haut (taille de la carte) serait passé ; le coût d'une pose par le serveur croît avec la file (une pose par case). |
| Autre | à préciser | |

**Recommandation : A (16).** Le coût est payé segment par segment (FQ-3) : un plafond court limite l'erreur de geste ; Shift enchaîne déjà les fondations, donc un long mur reste faisable ; la valeur est un seul réglage, facile à relever après essai.

Valeur par défaut codée (HALDIR-1120i) : cvar `hol.Wall.MaxLineCells` = 16 (option A), lu par `ServerPlaceWallLine` ; B = `hol.Wall.MaxLineCells 32`, C = une très grande valeur.

Reponse: (2026-11-20 ~12:30, interface de choix) Pas de plafond, mais un mur ne se pose pas directement : c'est comme pour tout chantier, c'est un fantôme qu'il faut construire. Voir GDD D867.
