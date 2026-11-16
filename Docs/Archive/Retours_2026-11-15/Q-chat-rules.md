# [À RÉPONDRE] Chat : que lit et qu'écrit un spectateur SANS siège ? (GLORFINDEL-1115f, 2026-11-15)

**Contexte.** La règle pure `Script\Chat\HOLChatRules.as` ne connaît que les sièges (un joueur vaincu = spectateur) ; un spectateur sans siège (pur observateur, D376) n'y existe pas. Le miroir C++ (`HOLChatRules.cpp` `FSeats::IsSpectator` : hors des sièges = spectateur, et `AHOLPlayerController::ServerPostChat` : « deux spectateurs sans siège partagent tout canal non privé ») a été choisi par l'agent précédent, sans décision.

**Options**
- **A (défaut codé aujourd'hui)** : le spectateur sans siège est un spectateur comme un vaincu : il parle et lit avec les spectateurs (vaincus et autres sans siège) sur Tous et Alliés, jamais en privé ; il lit le Tous des joueurs vivants (D792 1 A). Conséquence : deux observateurs bavardent, les joueurs ne les lisent jamais.
- **B** : Tous seulement entre spectateurs ; son Alliés ne part vers personne (il n'a pas de camp).
- **C** : le spectateur sans siège lit seulement (Tous des vivants, spectateurs), il n'écrit pas (le serveur refuse).
- **Autre** : à préciser.

**Recommandation : A** (même traitement qu'un vaincu, cohérent avec D376) ; puis aligner `HOLChatRules.as` (cas « sans siège » + test). Le défaut est dans le code (`ServerPostChat`, une ligne), pas encore derrière un réglage : à mettre derrière une cvar si B ou C est choisi.

Reponse (2026-11-15) : A : le spectateur sans siège est un spectateur comme un vaincu : il parle et lit avec les spectateurs sur Tous et Alliés, jamais en privé, et lit le Tous des joueurs vivants. (réponse donnée dans la conversation, 2026-11-15) Suite : reportée dans le GDD (D806) et le journal du chantier.
