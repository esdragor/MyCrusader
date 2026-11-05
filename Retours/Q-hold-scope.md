# [À RÉPONDRE] Q-hold-scope : que fait au juste l'IA qui « tient la position » (D717) ?

Contexte : D717 dit « économie de base et défense, sans attaque lointaine, sans expansion, sans duel accepté ». Le script (`Script/AI/HOLAIHold.as`, 1946a92) a dû choisir les limites ; la liste est dans une seule fonction (`HOLAIHold::Allows`) et un rayon (`FHOLAIHoldRules::BaseRadius`).

Valeur par défaut codée (celle qui tourne si tu ne réponds pas) : Paysans, maisons, camps, moulin, fermes, technologies, bâtiments et unités de défense = OUI ; Marché, Atelier de siège, Cavalerie, relais, escouade d'autel, vagues d'attaque, duel proposé ou accepté = NON ; aucun bâtiment à plus de 3000 cm du centre (camps de bois, de pierre, d'or, moulin compris).

Options :
- A (défaut) : comme ci-dessus.
- B : comme A, mais sans technologies (économie pure et défense par les unités).
- C : comme A, mais la cavalerie légère est produite (défense mobile).
- D : rayon de base plus large (6000 cm) : les camps un peu loin sont permis, l'IA reste exposée.
- Autre : à préciser.

Conséquences visibles : A garde la base vivante et sûre sans lancer l'IA dans une partie qu'elle n'a pas préparée ; B laisse le stock s'accumuler (le joueur retrouve plus de ressources, moins d'améliorations) ; C et D rendent l'IA plus active mais plus risquée.

Recommandation : A.

Réglage : une ligne dans `HOLAIHold::Allows` (la liste) et `BaseRadius` (le rayon) ; le C++ n'est pas concerné.

Reponse: C (2026-11-05 14:3x) : comme A, mais la cavalerie legere est produite (defense mobile). Pas encore appliquee : une ligne dans HOLAIHold::Allows, a faire au prochain Auto_Journey avec le reste C++ de la deconnexion.
