# [À RÉPONDRE] Q-decide-jauge : garder ou retirer la jauge d'Honneur dans la règle du Sceau de lumière

Contexte : depuis D862 le Sceau ne coûte aucun Honneur, mais `HOLFactionPowerCast::Decide` (`Script\Factions\HOLFactionPowerCast.as:59`) prend encore la jauge en paramètre et la recopie dans `NewGaugeValue` (l. 63 et 96) ; les tests D862 (`HOLFactionPowerCast_Test.as`, `HOLFactionPowerCastEdge_Test.as`, 12 appels, ~25 assertions « jauge inchangée ») s'appuient dessus. `HOLRpcPlayer.as:1077` construit donc le `FHOLFactionResource` pour rien d'autre que cet écho (le RPC ne lit pas `NewGaugeValue`).

- A (recommandé) : on garde le paramètre et `NewGaugeValue` : le fait « le Sceau ne touche pas l'Honneur » reste prouvé par la règle pure (rouge 9b68d8a) ; la ligne 1077 reste (une ligne).
- B : on retire le paramètre `Gauge` et `NewGaugeValue` de `Decide`, on supprime la construction du RPC ; les ~25 assertions « jauge inchangée » disparaissent (la garantie ne repose plus que sur le RPC, non testable en script).
- Autre : à préciser.

Conséquence visible en jeu : aucune (comportement identique). Seule la couverture de test change.

Valeur par défaut codée : A (rien n'est changé ; le nettoyage de la ligne 1077 attend la réponse). Une ligne à changer : la signature de `Decide`.

## Réponse de l'utilisateur (2026-11-20 ~16:0x, interface de choix)

**A** : on garde le paramètre `Gauge` de `Decide` et la ligne 1077 de `HOLRpcPlayer.as` ; rien à changer dans le code. Enregistré dans `AubeInProgress.md` (choix de code, pas de décision D).
