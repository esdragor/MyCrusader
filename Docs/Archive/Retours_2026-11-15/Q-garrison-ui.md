# [À RÉPONDRE] Garnison : bouton « Sortir tous », liste des occupants, clic droit sur un bâtiment qui est aussi un dépôt

Contexte (F7 lots 4a / 4b, `FortificationsInProgress.md`, D693, D793) : la garnison marche côté règle (entrer, sortir tous, éjection, abri, réseau).
Le GDD tranche « entrer et sortir par un ordre sur un bâtiment » mais ni l'apparence du bouton « Sortir tous », ni la liste des occupants, ni ce que fait un clic droit sur un bâtiment qui serait à la fois dépôt et garnison (le forum plus tard, D351).

Branche `tiers` (worktree Elrond), agent ELROND-1115f, 2026-11-15.

## Q1. Où sont le bouton « Sortir tous » et la liste des occupants ?

- **A. Une case de la grille du bâtiment sélectionné** (« Sortir tous », touche de la case comme les autres ordres de la grille) ; le nombre d'occupants s'affiche « 2 / 5 » dans le cadre du bâtiment, sans portrait.
  Conséquence visible : un clic ou une touche vide la tour ; on voit combien sont dedans, pas qui.
- **B. Portraits des occupants dans le cadre de sélection du bâtiment** (comme AoE4) : clic sur un portrait = cette unité sort ; un bouton « Sortir tous » à côté.
  Conséquence visible : on voit qui est dedans (héros, Roi plus tard, D365) et on peut en faire sortir un seul ; plus d'interface à faire.
- **C. A + B** : la case de grille (touche) ET les portraits cliquables.
  Conséquence visible : le plus complet, comme AoE4 ; le plus long à faire.
- **Autre** : à préciser.

**Recommandation : C** (conforme à AoE4 ; la case de grille donne la touche, les portraits montrent le héros et plus tard le Roi).
**Codé en attendant** : l'ordre est prêt (`HOLPlayerOrders::UngarrisonAll`, serveur, propriétaire seul, test `HeroesOfLiberty.Garrison.RightClick`) ; aucun bouton ni liste visible tant que Q1 n'est pas tranchée.

## Q2. Clic droit d'unités sur un bâtiment à la fois dépôt et garnison (forum, D351)

Aujourd'hui aucun dépôt n'a de places (`GarrisonSlots` 0 partout) : la question ne se posera qu'au chantier victoire, mais la règle est déjà écrite.

- **A. Le dépôt d'abord** : les porteurs déposent, les autres marchent jusqu'au point (comme aujourd'hui) ; on entre en garnison par le bouton (ou Alt + clic droit, à confirmer).
  Conséquence visible : rien ne change pour l'économie ; il faut un autre geste pour garnir le forum.
- **B. La garnison d'abord** : tout clic droit d'unités sur son forum les fait entrer.
  Conséquence visible : garnir vite en cas d'attaque, mais un Paysan chargé qui clique le forum disparaît dedans au lieu de déposer.
- **C. Selon l'unité** : les porteurs chargés déposent, les autres entrent.
  Conséquence visible : les deux gestes en un, mais un soldat envoyé « au forum » s'y enferme.
- **Autre** : à préciser.

**Recommandation : A** (l'économie reste comme elle est, la garnison du forum garde son geste à part, comme la Cloche d'AoE).
**Codé (défaut A)** : dans `AHOLPlayerController::OrderAt`, la garnison passe après les chantiers, les attaques et les dépôts ; réglage `hol.Garrison.RightClickEnters` (1 : un clic droit sur sa propre tour finie avec des places = garnison ; 0 : un simple déplacement).

Reponse (2026-11-15) : Q1 C : case de grille « Sortir tous » (touche) ET portraits cliquables des occupants dans le cadre du bâtiment (clic = cette unité sort). Q2 A : un clic droit sur un bâtiment à la fois dépôt et garnison dépose d'abord ; on garnit par le bouton (ou Alt + clic droit, à confirmer). (réponse donnée dans la conversation, 2026-11-15) Suite : reportée dans le GDD (D813) et le journal du chantier.
