# [À RÉPONDRE] Sites sacrés S1 : cas limites de la règle de prise (soldat seul, compte commencé par un autre camp, temps de prise)

Contexte (Glorfindel, branche `net-playtest`, `Script\Sites\HOLSiteRules.as`, 2026-11-05) : D698 dit « prendre = unité sainte ou héros, geler = toute unité adverse sauf Paysan » mais ne dit pas ce qui se passe quand **un camp n'a qu'un soldat (sans preneur) dans la zone** alors qu'un compte est en cours, ni quand le preneur change de camp en cours de compte. Les valeurs de temps sont celles du relais, jamais choisies par vous (D345).

**Q1. Un soldat adverse seul (aucun preneur) sur un site dont le compte a été commencé par un autre camp**
- **A.** Il **gèle** le compte (le compte reste où il est) ; le soldat du camp même du preneur ne gèle pas (le compte recule). Visible : un Lancier posté sur le site « garde » une prise abandonnée contre l'ennemi, sans la prendre.
- **B.** Il **n'a aucun effet** : le compte recule comme si personne n'était là. Visible : la prise abandonnée se défait toute seule malgré le Lancier.
- **C.** Il **fait reculer** le compte deux fois plus vite (il « nettoie »). Visible : un soldat qui défait activement la prise d'un autre. Demande une valeur de plus.
- **Autre** : ______

**Q2. Un preneur d'un autre camp arrive seul alors qu'un compte est commencé par un premier camp**
- **A.** Le compte **continue** là où il en est, au profit du nouvel arrivant (comportement du relais). Visible : le site se prend plus vite quand le premier a laissé du compte.
- **B.** Le compte **repart de zéro** pour le nouvel arrivant. Visible : plus de « vol » de compte, mais la prise abandonnée n'aide personne.
- **Autre** : ______

**Valeurs de temps** (inchangées, à garder tant que vous ne choisissez pas) : prise 60 s comme le relais, recul de 2 pas par pas ; victoire 300 s (D699).

Recommandation : **Q1 = A, Q2 = B** (A est lisible, B évite qu'un arrivant hérite d'un effort qui n'est pas le sien ; le relais garde son comportement actuel, intact).

Valeur par défaut codée : **Q1 = A, Q2 = A** (ce que fait la règle aujourd'hui, comportement du relais) ; la bascule de Q2 en B est une ligne dans `HOLSite::Step` (remettre `Next.Held` à 0 quand `Next.Taker` change de camp), à faire avec le C++ de S2 pour que le test d'accord garde les deux copies identiques.

Reponse: Q1 = A (gele, defaut code), Q2 = B (le preneur d'un autre camp REPART DE ZERO : changer le defaut A -> B, une ligne dans HOLSite::Step + test). 2026-11-05 20:38, utilisateur.
