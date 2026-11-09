# [À RÉPONDRE] Q-neutrals-camp-cache : le camp neutre du scénario `neutrals` est posé dans le bosquet central, on ne le voit pas au repos

Contexte : sur `neutrals_camp` les 3 Lanciers gris (camp en (0, 0)) sont cachés derrière les houppiers du bosquet de la carte Plain. L'essai ne peut pas être « VU À L'ÉCRAN » pour ce point tant que c'est ainsi.

Options :
- A. Déplacer le camp du scénario hors du bosquet (par ex. à 25 m au sud, `hol.Camp.Add 0 -2500 ...`), les `waitlog "Camp 1 at (0, 0)"` suivent. Le jeu ne change pas, seul le scénario.
- B. Garder le camp au centre et prendre la capture de repos de plus haut / plus près, au-dessus des houppiers (Zoom).
- C. Retirer les arbres autour du camp dans le scénario (commande de test de coupe), le camp est vu dans une clairière.
- Autre : ...
Conséquence visible : A et C montrent clairement le camp gris avant la charge ; B garde le décor mais les houppiers peuvent encore cacher un Lancier.
Recommandation : A (le plus simple, aucune dépendance au jeu).
Valeur par défaut codée : aucune modification pour l'instant (le scénario reste à (0, 0), `E-neutrals.md` reste LOGIQUE SEULEMENT) ; un agent qui applique A ne change que la ligne `hol.Camp.Add` et les `waitlog` du camp 1 dans `Tools\Scenarios\neutrals.txt`.

Reponse (utilisateur, 2026-11-09 16:44) : C (retirer les arbres autour du camp dans le scenario : camp vu dans une clairiere). Applique par Sauron-4 (worktree Sauron).
