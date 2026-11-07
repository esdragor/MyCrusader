# [À ESSAYER] E-legions_roster : joue les Légions et vérifie que chaque unité, bâtiment-grille et bloc de sélection porte son nom des Légions (Legion Spearman, Black Guard, Legion Archer, Legion Crossbowman, Dread Rider, Bone Ram, Bone Mangonel), pendant que l'Aube garde les siens

STATUT : VU À L'ÉCRAN   (noms de la grille et du bloc de selection vus dans 8 PNG, par clics et touches reels ; reserve : l'infobulle n'est verifiee que par le texte du journal, aucune souris ne la survole dans les PNG)
GESTE: réel      (hol.Select.ClickGround = vrai clic gauche sur le bâtiment, hol.Select.Grid 0 / 1 = vraies touches A / Z, hol.Select.ClickUnit = vrai clic sur le corps de l'unité ; EXCEPTIONS INTERNES : le Bélier est posé par hol.Units.SpawnHere (la touche A de l'Atelier de siège répond « tier too low »), le Mangonel, l'Arbalétrier et la bulle « Produit : ... » du Paysan ne sont vus que par l'ancien scénario legions_roster en commandes internes)
SON: sans objet
branches: tiers (fcea736 : Légions Galadriel-7 3c45d66 et noms par faction Glorfindel-8 6cf6bd2, dans main 80e3c07) ; scénarios neufs legions_roster_gesture et legions_roster_gesture_dawn de ELROND-9, commit sur la branche tiers du worktree Elrond, pas dans main

Résultat de l'intégrateur sur le build de main (80e3c07) : le REGLAGE est tenu. L'utilisateur avait dit que seul le Dread Rider changeait de nom ; ce n'est plus vrai.
- Légions, Caserne cliquée : cases « A: Legion Spearman, Z: Black Guard » ; infobulles « Legion Spearman | F 45 | W 15 » et « Black Guard | F 75 | G 15 » (texte du journal) ; pas de case W.
- Légions, Champ de tir : « Legion Archer », « Legion Crossbowman » ; Écurie : « Dread Rider » ; Atelier de siège : « Bone Ram », « Bone Mangonel » (+ W « Bélier renforcé Bientôt », X « Munitions lourdes Bientôt »).
- Bloc de sélection après un vrai clic sur le corps : Black Guard (touche Z), Legion Archer (touche A), Dread Rider (touche A), Bone Ram (posé par cheat). Production aux coûts des Légions (nourriture 5200 -> 5155 après le Lancier : 45).
- CONTRE-ÉPREUVES pour faire échouer : un Lancier de l'Aube (rouge) cliqué en ennemi par les Légions : bloc « Lancier » ; une Caserne de l'Aube cliquée : bloc « Caserne », aucune case. Joueur Aube (legions_roster_gesture_dawn) : Caserne « Lancier / Homme d'armes » (+ W « Hallebardes trempées Bientôt ») à F 60 W 20 et F 100 G 20, Champ de tir « Archer / Arbalétrier », Écurie « Cavalier léger », bloc « Lancier » ; le Lancier des Légions cliqué en ennemi : « Legion Spearman ». Aucun nom croisé.
- Observation (à trancher par l'utilisateur, pas un défaut établi) : les noms de l'Aube sont en français (Lancier, Homme d'armes, Cavalier léger, Arbalétrier) et ceux des Légions en anglais ; la Caserne de l'Aube a une case W « Hallebardes trempées » que celle des Légions n'a pas, alors que l'Atelier de siège des Légions garde les cases W / X de l'Aube.

Pré-vol (2026-11-07 10:15 et 10:16, agent ELROND-9 intégrateur, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, main 80e3c07, binaire 10:10) : `preflight.ps1 legions_roster_gesture -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (33 s, 5 PNG, HUD sans chevauchement) ; `preflight.ps1 legions_roster_gesture_dawn -Root ...` PASS (27 s, 3 PNG). WARN : pas d'audit « Sanity: » dans ces scénarios.
PNG OUVERTS et vus :
- legions_gesture_barracks : Caserne bleue sélectionnée (cadre blanc), bloc du bas « Caserne / Vous », grille avec A « Legion Spearman » et Z « Black Guard », pas de W.
- legions_gesture_blackguard : Black Guard produit par Z et cliqué : bloc « Black Guard », PV 155, attaque 12, cases Q Attaque, S Arrêt, V Tenir.
- legions_gesture_rider : Dread Rider cliqué : bloc « Dread Rider », PV 120, vitesse 8,8 m/s.
- legions_gesture_aube_unit : Lancier rouge de « IA Normal 2 » cliqué : bloc « Lancier », PV 80, pas de grille.
- legions_gesture_aube_barracks : Caserne rouge cliquée : bloc « Caserne », propriétaire « IA Normal 2 », PV 1500, grille vide.
- legions_gesture_dawn_barracks : joueur Aube : grille A « Lancier », Z « Homme d'armes », W « Hallebardes trempées Bientôt ».
- legions_gesture_dawn_unit : Lancier produit par A et cliqué : bloc « Lancier », PV 80.
- legions_gesture_dawn_enemy : Lancier rouge des Légions cliqué en ennemi : bloc « Legion Spearman ».
NON VÉRIFIÉ : l'infobulle survolée à la souris (le texte vient du journal HUD, pas d'un PNG) ; le bloc de sélection du Legion Crossbowman et du Bone Mangonel par un vrai clic (couverts seulement par l'ancien legions_roster, commandes internes) ; la bulle « Produit : Legion Spearman, Black Guard » du Paysan (même limite) ; l'écran de production (barre de file) ; une partie de plusieurs minutes (les scénarios durent 15 s) ; les noms dans d'autres écrans (stats, fin de partie, alertes, minicarte, Forge) ; la langue (français / anglais) voulue par l'utilisateur.

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 legions_roster_gesture -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 2 min (les scénarios n'ont aucune attente ; la version Aube : `...play.ps1 legions_roster_gesture_dawn -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`, 1 min). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
