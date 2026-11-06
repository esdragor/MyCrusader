# [À ESSAYER] E-cavalry : Écurie, Cavalier léger, 3 couloirs (Archers / Piquiers / Mangonneau) — cavalier debout au centre (recette indépendante 2026-11-06, ancien échec BUG-cavalry-flat = cadrage du scénario, corrigé d611d94)

STATUT : preflight PASS (92 s, 0 waitkey, 8 PNG ouverts et vus) mais le défaut décrit dans BUG-cavalry-flat.md est visible : NON listé [À ESSAYER]. GESTE : Écurie posée par `hol.Cheat.SpawnBuilding`, production par `hol.Prod.Queue LightCavalry`, attaque par `hol.Select.AttackMoveNear` (internes).

Pré-vol (2026-11-05 21:17, agent d'intégration n°2, worktree HeroesOfLiberty-Elrond = main eb7b5fe, SANS build) : `preflight.ps1 cavalry -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (check 91 s, journal sans warning/erreur, HUD sans recouvrement, audit Sanity OK).

PNG OUVERTS et vus :
- cavalry_stable : Écurie bleue sélectionnée, grille A « Cavalier léger » seule. Ressources 5200.
- cavalry_produced : Nourriture 5120 (-80), Or 5160 (-40) ; le cavalier (silhouette grise, debout) devant l'Écurie. Pop 6/50.
- cavalry_vs_archers_start : cavalier GRIS (pas bleu) en bas à droite, PENCHÉ/COUCHÉ (corps incliné à ~45°, anneau de sélection) ; 3 archers rouges en haut. Panneau : « Cavalier léger, PV 120/120 ». Message écran « Shaders en préparation (2) » (transitoire).
- cavalry_vs_archers : le cavalier est debout au milieu des archers, PV 42/120 (blessé mais debout : conforme au .look).
- cavalry_vs_spearmen_start : cavalier COUCHÉ À PLAT, presque horizontal (bas à droite), PV 120/120 ; les 2 piquiers rouges à droite en haut ; à gauche un cavalier debout + 3 archers du combat précédent.
- cavalry_vs_spearmen : plus de cavalier à son emplacement (mort), les piquiers à sa place ; conforme à « mort (ou presque) ».
- cavalry_vs_mangonel_start : cavalier INCLINÉ (bas à droite), le Mangonneau (grosse silhouette grise) en haut à droite.
- cavalry_vs_mangonel : le Mangonneau a disparu (détruit), le cavalier debout à sa place ; un corps allongé à gauche (restes du combat précédent).

Défauts : (1) cavalier à plat / incliné au repos dans les trois « start », debout une fois en mouvement (voir BUG-cavalry-flat.md). (2) Texte de la note 4 : « 80 PV : 9 coups » alors que le panneau affiche PV 120/120 (note périmée). (3) Cavalier gris et non bleu (assumé « placeholder » par la note 6, non tranché ici).

NON VÉRIFIÉ : les valeurs exactes de dégâts (19/24/29) et les durées (~5 s, ~8 s, ~10 s) : seul le journal des coups les dirait, non lu ; le clic réel sur la case A ; le Cavalier vu de près (zoom par défaut, petit).

Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 cavalry -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 90 s. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Recette indépendante 2026-11-06 12:12 : OK (main 2bb0d6b, preflight PASS 110 s, sanity OK). Le cavalier est DEBOUT (pose en T, anneau de sélection à ses pieds) et centré horizontalement dans les trois « start » (archers, piquiers, mangonneau), vu en gros plan (recadrage x4) ; cavalry_produced le montre debout de profil devant l'Écurie. Il est dans le tiers bas de l'écran (au-dessus du panneau), pas au milieu exact. cavalry_vs_archers : cavalier debout parmi les archers, PV 42/120 ; cavalry_vs_spearmen : cavalier absent, 2 piquiers là (mort : conforme) ; cavalry_vs_mangonel : Mangonneau détruit, cavalier debout. Plus aucun cavalier couché à plat : BUG-cavalry-flat = cadrage du scénario, résolu. PNG vus (8) : cavalry_stable, cavalry_produced, cavalry_vs_archers_start, cavalry_vs_archers, cavalry_vs_spearmen_start, cavalry_vs_spearmen, cavalry_vs_mangonel_start, cavalry_vs_mangonel. Statut : VU À L'ÉCRAN (preflight PASS, 110 s, 0 waitkey). Cavalier gris et sans animation = placeholder assumé.
GESTE: interne
SON: non entendu