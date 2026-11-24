# [RÉPONDU] Q-sp3-libelles : que lit le joueur sur le bouton de spécialisation ?

Reponse: B = « Spécialisation : à choisir » / « Spécialisation : Voie A (palier 1) » (utilisateur, 2026-11-23 18:1x, interface de choix, D905)

Contexte : le bouton permanent de spécialisation (D823 = B, près de la jauge de faction) doit dire l'état « à choisir » et, une fois le palier appliqué, la voie prise. Le GDD ne donne ni le texte de l'état « à choisir » ni la forme d'une voie acquise ; la table de textes (HOLGameNames.cpp) ne contient que « Voie A / B / C du palier N » (provisoire) et le bouton portait le libellé fixe « Specialisation » (sans accent).

Codé en attendant (PROVISOIRE, ELROND-1123pi) : état « à choisir » = « Specialisation » (texte existant, inchangé) ; voie acquise = une ligne par palier appliqué sous le bouton, avec le texte de la table (« Voie A du palier 1 »).

Options :
- A (défaut codé) : bouton « Specialisation » tant qu'un palier est ouvert ; en dessous, « Voie A du palier 1 » (une ligne par palier pris). Le joueur lit : `Specialisation` puis `Voie A du palier 1`.
- B : bouton « Spécialisation : à choisir » (accent, texte neuf FR/EN/JA dans la table) ; voie prise en une ligne « Spécialisation : Voie A (palier 1) ». Le joueur lit : `Spécialisation : à choisir`, puis `Spécialisation : Voie A (palier 1)`.
- C : après le choix, le nom de la voie seul, sans palier (« Voie A »), et plus aucun mot « Spécialisation » ; avant : `Spécialisation`. Le joueur lit : `Spécialisation`, puis `Voie A`.

Recommandation : B (le joueur lit l'état sans deviner, accent correct) une fois les noms GDD 13 par faction branchés (les noms remplaceront « Voie A »).
Valeur par défaut codée : A.
