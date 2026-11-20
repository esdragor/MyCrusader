# [À RÉPONDRE] Q-sceau-cout : combien d'Honneur coûte le Sceau de lumière ?

Contexte : D859 fixe la *Lumière sacrée* à 25 d'Honneur et 20 s de recharge, et le *Sceau de lumière* du Moine Lumineux à 40 s de recharge, mais aucune décision ne dit ce que le Sceau coûte en Honneur (D687 : « capacité active du Moine », sans coût).
Aujourd'hui le code (`HOLFactionPowerCast::Decide`, FARAMIR-1120c) fait payer le Sceau avec la ligne 0 du livre de pouvoirs : il coûte donc 25 par accident, avec 40 s de recharge, alors que le livre affiche 20 s pour cette ligne.

**Question : que paie le joueur de l'Aube quand son Moine pose un Sceau de lumière ?**

| Option | Ce que le joueur voit en jeu | Conséquence technique |
|---|---|---|
| **A. Coût 25, comme la Lumière sacrée** | Le Sceau et le soin se paient pareil sur la jauge d'Honneur (plafond 100) : au mieux 4 lancers par jauge pleine ; l'Honneur se partage entre le soin de zone et le ralentissement. Le Sceau devient un pouvoir de l'Aube (barre d'Honneur), pas seulement un sort de Moine. | Une ligne de plus dans `HOLDawnPowers.Rows()` (clé `LightSeal`, coût 25, recharge 40 s) ; la règle de pose lit sa propre ligne au lieu de la ligne 0 ; plus d'écart de recharge. |
| **B. Coût 40 (ou autre valeur à donner)** | Le Sceau coûte plus que le soin : au plus 2 lancers par jauge pleine ; plus rare, un vrai choix entre sauver une ligne et bloquer une charge. | Même changement que A, un autre nombre. |
| **C. Aucun Honneur : capacité de Moine seule** | La jauge d'Honneur ne bouge pas au lancer ; seule la recharge de 40 s limite le Sceau. Le bouton est dans les capacités du Moine, pas dans la barre de pouvoirs de la faction ; il y en a donc un par Moine. | Le Sceau sort du livre de pouvoirs : la pose passe par le cadre des capacités d'unité (`HOLUnitAbilityTable`, ligne `LightSeal`) sans jauge ; `HOLFactionPowerCast::Decide` ne gère plus que les vrais pouvoirs de faction. |
| Autre | à préciser | |

**Recommandation : C.** D687 présente le Sceau comme une capacité du Moine (cadre de capacités, recharge de 40 s déjà posée dans `HOLUnitAbilityTable`) ; l'Honneur est déjà gagné par ce même Moine en soignant (+1 par 50 PV), et faire payer le sort de zone avec le soin de zone par la même jauge de 100 rendrait les deux pouvoirs concurrents. Si l'utilisateur veut quand même une dépense, A (25) garde le moins de surprises.

Valeur par défaut codée : aucune (en pratique le code paie 25 via la ligne 0 du livre, par accident, jusqu'à la réponse).

Reponse: (2026-11-20 ~12:30, interface de choix) C : le Sceau de lumière ne coûte AUCUN Honneur ; capacité du Moine seule (cadre des capacités du Moine, un bouton par Moine, recharge 40 s, ligne `LightSeal` de `HOLUnitAbilityTable`), pas un pouvoir du livre de faction. Voir GDD D862.
