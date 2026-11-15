# [À RÉPONDRE] Zones de furtivité (D383, § 8.2) : distance « tout proche », cartes, aspect

Contexte : D383 dit qu'une unité dans une zone de furtivité de la carte est cachée, sauf pour un ennemi tout proche ou entré
dans la zone. La règle est codée (ARWEN-1115c, `HOLStealthRules.as`, sans branchement au jeu) ; il reste trois réglages.

## 1. La distance « tout proche » (N)

- **A. 2 cases (800 cm)** : un ennemi qui longe la zone à deux pas voit ce qu'il y a dedans ; une embuscade se découvre au
  dernier moment, la mêlée (portée 1 case) touche avant d'avoir vu de loin.
- **B. 1 case (400 cm)** : il faut presque toucher l'unité cachée ; embuscades très fortes, les archers (portée 5 cases) ne
  peuvent pas tirer dans un bosquet sans y envoyer quelqu'un.
- **C. 4 cases (1600 cm)** : la furtivité ne sert qu'à passer loin des yeux ; une armée qui avance découvre les bosquets
  avant d'y arriver.
- **Autre** : une valeur au choix.

**Recommandation : A (2 cases)**, proche des bosquets d'AoE4. Codé par défaut : `hol.Stealth.RevealDistance 800` (une ligne
à changer).

## 2. Quelles cartes posent des zones

- **A. Toutes les cartes standard**, 2 à 4 zones symétriques par carte : la furtivité fait partie de chaque partie.
- **B. Certaines cartes seulement** (celles à bosquets, par exemple une carte forestière) : une carte « ouverte » reste sans
  embuscade, la furtivité devient un trait de carte.
- **C. Aucune au prototype** : la règle attend, les cartes restent sans zone tant que le combat n'est pas réglé.
- **Autre**.

**Recommandation : B**, quelques cartes à bosquets, pour garder des cartes ouvertes lisibles. Par défaut codé : aucune carte
n'en pose (le placement est du level design, `MapsInProgress.md`).

## 3. L'aspect à l'écran

- **A. Bosquet d'arbres bas** distinct de la forêt (on voit où est la zone, pas qui s'y trouve), sans contour.
- **B. Bosquet + contour discret au sol** quand on survole ou qu'une de ses unités y entre (lisible, un peu moins naturel).
- **C. Herbes hautes ou roseaux** : se lit comme une cachette, mais D383 écarte les hautes herbes comme règle générale (ici
  seulement l'aspect des zones posées).
- **Autre**.

Ses propres unités dans la zone : à demi transparentes (comme AoE4) dans les trois cas.

**Recommandation : B** (bosquet + contour discret), pour qu'un joueur sache toujours où il peut se cacher. Rien n'est
dessiné à ce stade (aucune forme provisoire).
