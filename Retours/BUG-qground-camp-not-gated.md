# BUG-qground-camp-not-gated : le réglage AttackMoveEngagesNeutral 0 ne change rien pour un camp de carte (à confirmer : voulu ou non)

Scénario : neutrals_qground (copie « avant » = même scénario + `hol.Combat.AttackMoveEngagesNeutral 0` après `hol.Combat.LogHits 1`, `waitlog hits #` -> `frames 150`). Build : main b39fafd (Elrond).
Geste exact : `hol.Select.Army`, `hol.Select.Grid 4` (touche Q), `hol.Select.LeftClickGround -8000 0` (le camp de 3 Lanciers est à -9800, mes 6 Hommes d'armes partent de -13800).
Observé : réglage 0 ET réglage 1 : « Order: 6 units attack-move to (-7995, 0) » puis « #7 ManAtArms hits #2 Spearman for 12, 68 HP left » (step ~196-207) et « Camp 0: fighting (members 3, adversaries near 6) ». Contraste : diplomacy_qground (joueur Neutre) avec le réglage 0 = 0 coup en 150 images, avec le réglage 1 = 36 coups.
Conséquence : neutrals_qground ne prouve pas le correctif D733 (il passe aussi « avant »). Question ouverte pour l'auteur, sans décision de ma part : le camp de carte est-il volontairement hors du réglage (relation propre au camp) ? Si oui, il faut renommer l'attendu du scénario ; sinon le réglage ne couvre pas les camps.

Reponse: A (voulu : un camp de carte garde sa regle propre, le reglage ne concerne que le joueur Neutre ; D731). 2026-11-05 21:41, utilisateur. Suite : corriger l'attendu de neutrals_qground (il ne prouve rien avant), pas de C++.
