# [À RÉPONDRE] Défi du Chevalier (passif) : part redirigée et rayon

Contexte : D19 dit « à régler en test : le pourcentage redirigé, le rayon, la recharge ». D859 a tranché l'ACTIF (8 s, 8 m, recharge 45 s, tout est redirigé), pas le PASSIF. La règle pure `HOLDamageRedirect::Redirect` prend `PercentPerKnight` et `RadiusCm` en paramètres : rien n'est codé en dur, la valeur ira dans une colonne de table (`FHOLUnitTypeConstants`, A2 sous-étape suivante).

Part redirigée du passif :
- **A. 20 %** : le Chevalier encaisse un peu ; l'infanterie protégée meurt à peine plus lentement, le Chevalier tient longtemps.
- **B. 30 %** : un tiers environ ; effet net visible (3 alliés = presque un Chevalier de PV en plus), le Chevalier fond vite sous un tir concentré sans Moine.
- **C. 50 %** : moitié ; très fort, proche de l'actif, risque de rendre l'actif peu distinct.
- **Autre** : à dire.

Rayon du passif (cm) :
- **A. 6 m** ; **B. 8 m** (même que l'actif) ; **C. 10 m**.

**Recommandation : 30 % et 8 m** (l'actif à 100 % sur 8 m reste clairement plus fort ; même rayon donc un seul anneau au sol à la sélection).
**Valeur par défaut codée** : non codée dans la règle (paramètres) ; les tests utilisent 30 % et 5 m à titre d'exemple d'appel seulement ; la valeur de table viendra avec la sous-étape « champ de table + anneau », réglable après essai (D345).

Reponse: (2026-11-20 ~12:30, interface de choix) 30 % et 8 m pour le passif du Défi du Chevalier (recommandation retenue). Voir GDD D865.
