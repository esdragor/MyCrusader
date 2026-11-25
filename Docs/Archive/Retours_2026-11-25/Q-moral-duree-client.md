[À RÉPONDRE] Durée restante d'un effet de moral côté client (R1)

Contexte : le masque réseau ne porte que les ids des effets (1 bit + 16 bits), donc la pastille de moral d'une unité de notre camp ne connaît pas le temps restant (avant GALADRIEL-1125mb elle affichait la durée complète de la table comme « temps restant » ; corrigé en option A, sans secondes).

Options :
- A : rien sur le réseau, pastille sans secondes. Coût 0 octet. En jeu : on voit l'effet, jamais combien de temps il reste.
- B : temps restant de l'effet temporaire le plus long, 1 octet (secondes entières, 255 max), envoyé derrière le masque seulement (+8 bits pour une unité sous effet, +0 sinon). En jeu : la pastille décompte l'effet le plus long ; les autres effets d'une même unité n'ont pas leur propre décompte.
- C : immunité seule en 1 bit (l'unité est protégée ou non). En jeu : on sait qu'elle ne peut plus subir de malus, pas combien de temps.
- Autre : un octet par effet (jusqu'à 4 octets), décompte exact de chacun, trafic ×4.

Recommandation : B (le plus utile pour un coût borné).

VALEUR PAR DÉFAUT CODÉE (GALADRIEL-1125md, branche morale-r1-1125m) : B, derrière `HOLUnitReplication::FHOLNetUnitState::bEffectSecondsOnWire` (`HOLUnitReplicationRules.h`, une ligne : `false` = option A, le fil revient à 16 bits sous effet). L'immunité (C) n'est pas codée. Le champ n'entre pas dans `IsSameAction` (un changement par seconde serait du trafic) : le client reçoit donc la valeur avec le prochain état envoyé, pas à chaque seconde.

Reponse: B (2026-11-25 13:0x, DOCS-1125me, GDD D916)
