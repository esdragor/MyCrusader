# [À RÉPONDRE] Suivi mutuel : A suit B et B suit A (REVUE-1122 mineur 12)

Contexte : deux unités alliées qui se suivent l'une l'autre s'arrêtent chacune à 300 cm (HOLFollowRules.as `FollowDistance`) : pas de boucle, mais plus personne ne bouge ensuite (ni vers un ennemi hors portée, ni en suivant un tiers). Le comportement n'est pas défini par le GDD (§ 7.3).

Options :
- A. Ne rien changer : l'immobilité est le comportement voulu (deux gardes du corps qui se tiennent).
- B. Le suivi de l'unité qui a l'identifiant le plus bas est annulé à l'instant où la boucle se ferme (la plus haute suit toujours ; le joueur le voit : l'icône de suivi de la plus basse disparaît).
- C. Le dernier ordre donné gagne : donner « suivre B » à A annule le suivi de B sur A si B suivait A (le joueur voit B s'arrêter de suivre).
- Autre : à préciser.

Conséquences visibles : A = aucun changement ; B = une des deux reste libre mais perd son ordre sans que le joueur l'ait demandé ; C = l'ordre le plus récent est respecté, l'ancien est annulé (comme « tout ordre manuel met fin au suivi »).

Recommandation : C (cohérent avec « tout ordre manuel met fin au suivi », D30 / D340).

Valeur par défaut codée : aucune pour l'instant (A, comportement actuel) ; le correctif C tiendra dans `HOLFollowOrders::Begin` derrière un réglage `hol.Follow.MutualCancels` (0 = comportement actuel).
