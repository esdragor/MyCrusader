# [À RÉPONDRE] Trébuchet : délais de montage, portées, interruption du montage (Palier 3 G5, D712)

Contexte : la règle pure de l'engin déployable est codée en script (`Script/Siege/HOLDeployRules.as`, testée, commit sur la branche `galadriel-tab`). D712 fixe les deux états, le montage automatique sur les ordres, le bouton, la portée minimale ; il ne fixe aucun chiffre. Les valeurs ci-dessous sont des PROPOSITIONS, codées par défaut, une ligne chacune à changer (`HOLDeploy::Proposed*` dans `HOLDeployRules.as`, puis les colonnes de `UnitTypes.csv` en G6).

## Q1. Les quatre chiffres du Trébuchet

Repères : le Mangonneau (palier 2) tire sur un bâtiment à 30 m ; le Bélier est au contact.

| | Montage | Démontage | Portée minimale | Portée maximale (bâtiments) | Contrepoids (D715) |
|---|---|---|---|---|---|
| **A (défaut codé)** | 8 s | 5 s | 12 m | 50 m | -25 % sur les deux délais |
| B (plus nerveux) | 5 s | 3 s | 8 m | 45 m | -30 % |
| C (plus lourd) | 12 s | 8 s | 20 m | 60 m | -20 % |

Autre : donner les cinq chiffres.

Conséquence visible : A et C laissent au Cavalier léger le temps d'arriver pendant le montage ; B rend le Trébuchet moins vulnérable mais moins distinctif face au Mangonneau.
**Recommandation : A** (assez lent pour que le montage compte, assez court pour rester jouable ; la portée maximale dépasse nettement le Mangonneau).

## Q2. Un ordre de marche pendant le montage

| | Règle | Conséquence |
|---|---|---|
| **A (défaut codé)** | L'ordre ANNULE le montage : l'engin est de nouveau démonté tout de suite, le temps déjà passé est perdu | Simple, lisible ; on peut se replier d'un clic ; coût = le temps perdu |
| B | Le montage va à son terme, puis l'engin se démonte (donc délai de montage + délai de démontage) | Sanction forte du montage mal choisi ; l'engin est bloqué longtemps |
| C | L'engin se démonte en un temps proportionnel à ce qui était déjà monté | Plus juste, plus difficile à lire pour le joueur |

Autre : à préciser.
**Recommandation : A.** Le même choix vaut pour le bouton Monter / Démonter pendant le montage (annule).

Reponse: Q1 = A (8 s / 5 s / 12-50 m / Contrepoids -25 %), Q2 = A (la marche annule le montage). 2026-11-05 20:38, utilisateur. Defauts codes : rien a changer.
