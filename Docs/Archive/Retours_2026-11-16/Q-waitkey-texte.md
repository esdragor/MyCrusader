# [À RÉPONDRE] Backspace dans les scénarios *_compare : texte de l'attente, et début d'une option (FARAMIR-1116b, 2026-11-16)

Contexte : Backspace (règle 15 de « Qualité des essais », main 53c106f) marche : rejoué par vraie touche le 2026-11-16 (`keys_tree_spacing_compare`, journal « Scenario key: back to the previous option (step 26: hol.Camera.Focus -5200 -3500) », PNG recentré sur 0,3 m). Mais rien ne l'annonce au joueur, et dans 8 scénarios le retour ne remet pas l'option précédente à l'écran (Q2).

## Q1. Où écrire « Backspace = option précédente » ?
Aujourd'hui chaque attente affiche son propre texte (« Entree : style suivant ») ; ~400 `waitkey` dans 33 `*_compare` (dont 19 `audio_compare_*` retouchés par Arwen ce jour).
- **A. Dans les .txt** : ajouter « , Backspace : précédente » au texte de chaque `waitkey` concerné. En jeu : texte exact par scénario. Coût : ~400 lignes, conflits avec les branches qui touchent ces fichiers ; tout nouveau scénario doit y penser.
- **B. Par le panneau (C++, HOLTestSequence.cpp)** : quand le scénario est un `_compare` ET qu'une option précédente existe, le panneau ajoute seul « · Backspace : option précédente » au texte de l'attente (absent à la 1re option, où la touche ne fait rien). En jeu : toujours juste, aucun .txt touché. Coût : petit C++ + un test.
- **C. Une `note` en tête de chaque _compare** (« Entrée = suivante, Backspace = précédente ») : 33 lignes ; vue au début seulement.
- **Autre** : à préciser.
- **Recommandation : B.** Valeur par défaut codée : rien (texte inchangé ; la touche marche mais n'est pas annoncée).

## Q2. Backspace saute la commande qui règle l'option (8 scénarios)
`FindPreviousOptionStart` fait commencer une option à son premier `notereset`. Or dans `rocks_compare`, `rocks_compare2`, `resources_compare`, `forest_stretch_compare`, `trees_rocks_compare`, `orders_marks_compare`, `talents_icons_compare` et une option de `legions_warrior_compare`, la commande qui règle l'option (ex. `hol.Rocks.Review C`) est AVANT le `notereset`. Vu le 2026-11-16 (`keys_rocks_compare`) : Backspace pendant l'attente de D → « back to the previous option (step 30: notereset STYLE C (3/4)) », la note dit C mais l'écran montre toujours ROCHERS D (PNG `keys_rocks_C` rejoué).
- **A. C++** : une option commence juste après l'attente précédente (la règle du `notereset` ne sert plus qu'à la 1re option, pour ne pas rejouer la mise en place). En jeu : le retour remet bien l'option. Risque : une option qui fait apparaître des unités avant sa note les refait apparaître.
- **B. Dans les .txt** : déplacer le `notereset` avant la commande de réglage dans ces 8 fichiers (~45 lignes) + une phrase dans `Tools\Scenarios\README.md`. Sans C++ ; la règle reste une convention à tenir.
- **C. Laisser** : Backspace ne marche que dans les _compare sans `notereset` ou à `notereset` en tête d'option (ex. `tree_spacing_compare`, les `audio_compare_*`).
- **Autre** : à préciser.
- **Recommandation : A** (pas de convention à retenir). Valeur par défaut codée : C (rien changé).
Reponse: Q1 = B (le panneau l ajoute, C++), Q2 = A (option = juste après l attente, C++) ; utilisateur, 2026-11-16 16:0x, par l interface de choix ; D827, D828 ; ligne [ ] en tête de la file de Faramir.
