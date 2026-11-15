# [À RÉPONDRE] Alertes mineures (D202) : lesquelles, avec quoi ?

Auteur : GALADRIEL-1115f, 2026-11-15. Chantier : `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\AlertsMinorInProgress.md` (inventaire du code).

**Contexte.** D202 promet des alertes mineures « fil discret, ping et son court propre à chaque type » ; aujourd'hui le fil ne porte que le tribut,
le signal reçu, le héros prêt et le niveau du héros ; « population pleine », « recherche terminée », « bâtiment achevé » manquent, et « attaqué ailleurs » n'a qu'un point qui clignote sur la mini-carte.

**Question regroupée : quelles alertes mineures ajouter, et avec quoi (ping, son, anti-répétition) ?**

- **A. Trois lignes muettes, sans ping** (population pleine, recherche terminée, bâtiment achevé) ; une même ligne pas plus d'une fois toutes les 30 s.
  Vu : une ligne de plus dans le fil à droite, ~8 s ; aucun son, aucun point sur la mini-carte. Conforme à D449 (« les lignes du fil restent muettes »), contraire à la lettre de D202 (ping et son).
- **B. A + un son court par type et un ping sur la mini-carte** (bâtiment achevé et recherche : là où est le bâtiment ; population pleine : pas de lieu, pas de ping) ; « attaqué ailleurs » gagne une ligne « Vos troupes sont attaquées » + son, au plus une fois toutes les 30 s.
  Vu : un « tic » distinct par alerte (sons provisoires du moteur, à remplacer), un point bref sur la mini-carte ; plus de bruit dans une grosse bataille. La lettre de D202.
- **C. Seulement « population pleine » et « attaqué ailleurs »** (les deux qui coûtent si on les rate), avec un son court ; recherche et bâtiment achevés restent visibles par le bouton et la file.
  Vu : le fil reste calme ; on rate la fin d'une recherche lancée loin de l'écran.
- **Autre** : votre liste (ex. unité perdue, production terminée, ressource épuisée, villageois inactif, à la manière d'AoE4), votre durée.

**Recommandation : B**, sauf la ligne « attaqué ailleurs » (garder le point clignotant de la mini-carte, déjà là) : c'est la promesse de D202 et le joueur en
bataille entend une alerte sans quitter le terrain des yeux ; la répétition à 30 s évite le fil saturé.

**Défaut codé en attendant** : A pour « population pleine » seulement (règle pure `Script\UI\HOLAlertRules.as`, ligne muette, une fois quand la
population devient pleine, pas de nouvelle ligne avant `HOLAlertRules::PopulationRepeatSeconds = 30.0`, une ligne à changer). Pas encore branchée au HUD (C++ à faire).
