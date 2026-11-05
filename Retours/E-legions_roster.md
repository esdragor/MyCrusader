# [À ESSAYER] E-legions_roster : grille de la Caserne Légions (Lancier A, Homme d'armes Z), coût du Lancier 45 N / 15 B, unités bleues vs Aube rouge

STATUT : VU À L'ÉCRAN (preflight PASS, 3 PNG ouverts et vus, 21 s, 0 waitkey). GESTE : la Caserne est posée par `hol.Cheat.SpawnBuilding`, sélectionnée par `hol.Select.Category Military` et le Lancier mis en file par `hol.Prod.Queue Spearman` (appels internes, pas de clic sur la grille) ; ce que montrent les PNG (grille, barre de ressources, couleurs) est l'état réel du jeu.

Pré-vol (2026-11-05 21:16, agent d'intégration n°2, worktree HeroesOfLiberty-Elrond = main eb7b5fe, SANS build, binaire du 21:07) : `preflight.ps1 legions_roster -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (check 21 s, journal sans warning/erreur, HUD sans recouvrement). WARN : le scénario n'appelle pas `hol.Test.Sanity` (audit neutres/rotation/tremblement non fait).

PNG OUVERTS et vus :
- legions_roster_barracks : Caserne bleue sélectionnée (panneau « Caserne / Vous », PV 1500/1500). Grille : A « Lancier », Z « Homme d'armes », W « Hallebardes trempées / Bientôt » (case de recherche grisée). Pas de Hallebardier. Ressources 5200 partout.
- legions_roster_row : après la production, Nourriture 5155 (-45), Bois 5185 (-15), Population 19/70 : le coût payé est bien celui des Légions (45/15). Rangée de 4 groupes de 3 BLEUS (Lancier, Homme d'armes, Archer, Arbalétrier) et un 5e groupe GRIS (le Cavalier léger : pas bleu, comme dans le scénario cavalry).
- legions_roster_dawn : les Lanciers bleus (bas) et, en haut, un petit groupe ROUGE (Aube) ; je ne vois qu'un amas rouge là où le .look annonce « trois Lanciers de l'Aube à côté de ceux des Légions » : ils sont éloignés des bleus (pas « côte à côte »), petits à ce zoom.

Écart au .look (à décider par l'utilisateur, pas tranché ici) : le 5e groupe est gris, pas bleu ; les Lanciers de l'Aube ne sont pas à côté des Légions.

NON VÉRIFIÉ : le coût 60/20 de l'Aube (pas de Caserne de l'Aube dans le scénario) ; le clic réel sur la case A ; les noms/coûts des 4 autres unités du socle (seul le Lancier est payé dans le scénario) ; les corps provisoires (note 3 du scénario).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 legions_roster -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 10 s (tout se déclenche d'office). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
