# [À RÉPONDRE] Raccourcis reconfigurables (B6, GDD § 7.3 et § 15, D30, D245) — GALADRIEL-1115c, 2026-11-15

**Contexte (2 lignes)** : D30 dit « tout est reconfigurable » mais l'onglet Commandes des réglages est grisé « jusqu'à M4 » (D245) et la grille 3 × 4 a ses touches en dur. La règle pure est prête sans C++ (`Script\Input\HOLRebindRules.as`, branche `galadriel-tab`) : table action -> touche (les 49 raccourcis de D30), refus de deux actions sur la même touche, refus d'`Échap` (menu) et `Entrée` (discussion, D385), bouton « Valeurs par défaut ».

**Une question en deux volets : quand ouvrir l'onglet, et pour quels raccourcis ?**

| Option | Ce que vous verrez | Conséquences |
|---|---|---|
| **A** (défaut codé) | Onglet Commandes toujours grisé jusqu'à M4 (« Changer les touches : pas encore dans le jeu ») ; à M4, TOUS les raccourcis de D30 changeables (grille, F1-F4, Tab, `.`, `CTRL + A`, groupes) | Rien ne change à l'écran aujourd'hui ; aucun C++ maintenant ; la règle attend son branchement |
| **B** | Onglet ouvert dès maintenant, **grille 3 × 4 seule** (12 cases : ordres Q/S/V, paliers A Z E R...) ; F1-F4, groupes, `CTRL + A` restent fixes | Un chantier C++ maintenant (Enhanced Input, liste des 12 touches, sauvegarde, grille qui lit la table) ; les joueurs QWERTY sont servis tout de suite ; D30 « tout » reste partiel jusqu'à M4 |
| **C** | Onglet ouvert dès maintenant, **tous les raccourcis** de D30 | Le plus gros chantier C++ (liste de 49 lignes à faire défiler, conflits affichés, groupes à 3 combinaisons) ; tient D30 à la lettre tout de suite |
| **Autre** | à préciser | — |

**Recommandation : A** — D245 a déjà fixé « grisé jusqu'à M4 », et D30 dit « tout est reconfigurable » : ouvrir à M4 avec tous les raccourcis évite de refaire l'écran deux fois. Si les joueurs QWERTY vous pressent, **B** est l'étape intermédiaire la moins chère.

**Valeur par défaut codée (une ligne à changer)** : `HOLRebind::Scope = ScopeAll` dans `Script\Input\HOLRebindRules.as` (B = `ScopeGrid` : toute action hors de la grille est refusée « OutOfScope ») ; l'onglet reste grisé (`HOLSettingsScreen.cpp` l. 246, C++ non touché).

Reponse (2026-11-15) : A : onglet Commandes grisé jusqu'à M4, puis TOUS les raccourcis de D30 changeables (HOLRebind::Scope = ScopeAll, défaut codé). (réponse donnée dans la conversation, 2026-11-15) Suite : reportée dans le GDD (D819) et le journal du chantier.
