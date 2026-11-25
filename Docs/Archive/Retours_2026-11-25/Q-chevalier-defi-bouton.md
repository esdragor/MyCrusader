# [À RÉPONDRE] Défi du Chevalier : case de grille (libellé court et touche)

Contexte : la règle `HOLKnightChallenge::CastChallenge` (SAURON-1125b, e1ceacd) n'a aucun appelant ; le bouton qui la lance manque. Le GDD
(§ Chevalier Vertueux, D19) donne le nom « Défi du Chevalier » (ligne `KnightChallenge` de `Tools/Data/UnitAbilities.csv`, noms en/ja dans
`Game.po`) mais ni libellé court de case ni touche (le GDD ligne 385 « touche dédiée dans la grille du héros » parle du duel du héros).
Les cases connues : A = Rangs (cell 0), Z = Bannière (1), E = Rage (2), R = Sceau (3) ; les cases Stop / Hold / AttackMove / Deploy ont leurs numéros dans `HOLCommandGridRules.as`.

Question 1 : quelle case pour le Défi sur la grille d'un Chevalier Vertueux (et d'une sélection qui en contient un) ?
- A. Cell 4 (touche T, juste après le Sceau) : jamais en conflit avec le Moine (Sceau R), le Guerrier damné (Rage E) ou le héros (A Z) quand la sélection les mélange.
- B. Cell 2 (E, comme la Rage) : même emplacement mental « capacité d'unité », sans conflit (la Rage est réservée au Guerrier damné, faction différente), mais le même `GetCommand` doit alors choisir selon la faction.
- C. Cell 3 (R, comme le Sceau) : même défaut ; conflit dès qu'un Moine et un Chevalier de l'Aube sont sélectionnés ensemble (cas courant).
- Autre : ta touche.
Recommandation : A. Valeur par défaut codée : cell 4 derrière `HOLCommandGridRules::GetKnightChallengeCell()` (une ligne à changer) ; à vérifier libre par le balayage `HOLCommandGridSweeps_Test`.

Question 2 : libellé court de la case (sous le nom complet « Défi du Chevalier » dans l'infobulle) ?
- A. « Défi » (sur le modèle « Rage », « Sceau », « Rangs »).
- B. « Défi du Chevalier » en entier (long pour la case).
- Autre.
Recommandation : A. Valeur par défaut codée : « Défi » (+ recharge en secondes sous le libellé, comme la Rage), infobulle tirée de la ligne `KnightChallenge` (8 s, 8 m, 45 s lus par `HOLDawnValues`).

Reponse: Q1 = case T (cell 4), Q2 = libelle court « Defi » ; deja le defaut code de la question (2026-11-25 18:1x, DOCS-1125c, GDD D923)
