# [À RÉPONDRE] Le panneau du scénario (notes) cache la scène dans ai_evolving (L1-12b, Faramir-10)

Contexte : la ligne « HUD rects: » du jeu contient maintenant le panneau du scénario (`scenario=`), preuve par `hud_rects_game_test` (vert). Aucun widget n'en recouvre un autre (preflight ai_evolving et duel_challenge PASS), MAIS dans `ai_evolving_hard` (stade 3) le panneau à 4 notes (~920 x 255 px, en haut au centre) cache le haut du forum de l'IA et le début de l'attaque : la note du point dit « en partie sous le panneau » ; `ai_evolving_normal` est dans le même cas. Ce n'est pas un recouvrement de widgets, c'est le panneau qui couvre le MONDE à l'endroit à voir.

Options :
- **A (défaut = état actuel, rien changé)** : le panneau reste affiché aux captures d'ai_evolving, la note du point l'annonce.
- B : le scénario cache le panneau juste avant les captures `ai_evolving_normal` et `ai_evolving_hard` (`panel hide`, puis `panel show`), comme duel_challenge. Conséquence : on voit tout le forum et le combat, mais pas les notes pendant la capture ; l'utilisateur lit les notes avant l'Entrée.
- C : recadrer la caméra du scénario plus bas (`hol.Camera.Focus` décalé, ~ 800 cm vers le haut de l'écran) pour que le forum et la troupe tombent sous le panneau, pas dedans. Conséquence : les notes restent visibles, la scène est centrée plus bas.
- Autre : un panneau plus compact (3 notes max), ou les notes repliées après l'Entrée.

Recommandation : B pour les captures (la plus sûre : rien à régler à l'œil, même geste que duel_challenge) ; C si tu veux garder les notes lisibles pendant l'essai.

Valeur par défaut codée : A (aucune ligne changée). B = deux lignes `panel hide` / `panel show` autour des `capture ai_evolving_normal` et `capture ai_evolving_hard` de `Tools\Scenarios\ai_evolving.txt` ; C = les lignes `hol.Camera.Focus` du même fichier.
