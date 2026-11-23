# RECETTE-1123pa (2026-11-23, 15:40 - 15:47), worktree HeroesOfLiberty-Haldir, integ-58 = main b5a285b

## 1. faction_click_cast : PASS après retouche du scénario (21e336f) ; DÉFAUT DU JEU en dessous
- Rejeu 1 (scénario d'origine : héros + 5 paysans) : `RpcPower: cast refused (4) for slot 0 power 0` à (-17528, 299), le héros étant à (-17535, 300) : la même panne que ce matin, PAS corrigée pour ce cas. Cause lue : `HOLFactionPowerServer::TryCastPower` (Script/Factions/HOLFactionPowerServer.as:25-37) prend les points de `HOLAISummary::ReadAISummary(Slot).Army`, or `Source/HeroesOfLiberty/AI/HOLAISummary.cpp:105` exclut les paysans, le roi ET le héros : sans soldat `Own` est vide, `CastOriginOf` rend (0, 0), distance 17 535 cm > portée du sceau -> refus 4.
- Rejeu 2 (scénario retouché : 4 Hommes d'armes du joueur avant le lancer) : `FactionAim: aim 0`, `Sacred Light put down by slot 0, Honor now 75`, `Sacred Light placeholder: golden ring of 600 cm for 5.0 s`, `HUD faction slot: slot gauge 75 / 100 ... cooling 20 s`. 3 PNG ouverts : before = « 100 / 100, Lumière sacrée : coût 25, prêt », sans anneau ; aim = contour de la case armé ; after = deux anneaux dorés autour du héros et des soldats, panneau « 75 / 100 », « recharge 20 s ».
- Fiche : Retours/E-faction_click_cast.md (gate Essay : `=> aucun FAIL`).
- NON VÉRIFIÉ : guérison des blessés (aucun blessé dans ce scénario, pas de ligne de points de vie) ; curseur de visée non visible (seul le contour de la case) ; le clic sol est interne (hol.Select.LeftClickGround).

## 2. ai_sacred_light : PASS
- preflight PASS. Journal : step 24 `Honor 100` (HonorSet), step 39 `wounded 1 of 7, decision cast=true row 0, nearest wounded 253 cm, range 800 cm`, `Sacred Light put down by slot 1, Honor now 75`, `Sacred Light cast on 1 wounded unit(s)`.
- PNG : before = forum rouge, figures rouges rassemblées, aucun combat ni anneau ; after = mêlée rouge/bleu avec anneau doré (2 cercles) centré dessus.
- Fiche : Retours/E-ai_sacred_light.md (gate Essay : `=> aucun FAIL`).
- NON VÉRIFIÉ : points de vie du blessé avant/après ; le Chevalier vertueux n'est pas distingué sur le PNG.

## 3. wall_line_long : NON JOUÉ (hors délai, la version de main est l'ancienne).

## Défauts du jeu proposés
- [ ] Lumière sacrée du joueur refusée (code 4) quand il n'a ni bannière plantée ni soldat (héros + paysans seuls, le début de partie) : l'origine du lancer lit `ReadAISummary(Slot).Army` qui exclut héros/roi/paysans ; origine (0, 0). Fichiers : Script/Factions/HOLFactionPowerServer.as (TryCastPower, lignes 25-37, passer aussi la position du héros / de la sélection), Source/HeroesOfLiberty/AI/HOLAISummary.cpp:105. Un test script doit couvrir « hero + peasants only ».
- [ ] Aucune ligne de journal de guérison par la zone (points de vie avant/après) : impossible de lire l'effet ; ajouter un log par unité soignée (HOLZoneEffect) ou un scénario avec un blessé et `hol.Units.Report`.

## Commits / état
- Scénario retouché : commit 21e336f sur integ-58 (« Scenario faction_click_cast: spawn 4 Men-at-arms ... », dernière ligne `Agent: RECETTE-1123pa`) ; à faire entrer dans main à la passe suivante. Aucun code de jeu touché.
- stray_processes -Stop : aucun processus détaché.
- Anomalie : le preflight de faction_click_cast signale FAIL « 1 uncommitted file » tant que le scénario retouché n'était pas commité (corrigé par le commit) ; le preflight n'a pas été rejoué après ce commit (budget d'appels) mais le log du rejeu 2 était sans erreur.
