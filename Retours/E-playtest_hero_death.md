# E-playtest_hero_death : mort du héros, fenêtre de 54 s, bouton « Ramener le héros » — PREFLIGHT PASS mais 2 captures sur 4 prises trop tôt, voir BUG-playtest_hero_death-captures.md

STATUT : preflight PASS (check 76 s, 1 waitkey Entrée voulu, 30 s d'attente explicite, journal sans warning/erreur, audit Sanity OK), 4 PNG ouverts et vus ; 2 points du .look (fallen, back) NON vus à l'écran : NON listé [À ESSAYER], LOGIQUE SEULEMENT pour la mort et le retour. GESTE : `hol.Hero.Kill 0`, `hol.Hero.Return` (internes ; ni mort au combat, ni clic sur « Ramener le héros »).

Pré-vol (2026-11-05 21:22, agent d'intégration n°2, worktree HeroesOfLiberty-Elrond = main eb7b5fe, SANS build) : `preflight.ps1 playtest_hero_death -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS. Journal : `Hero return: slot 0 ready at step 1115` (54 s après la mort, comme annoncé), `Hero return order: slot 0 queued`, `Hero return: slot 0 by its main forum`, `Hero return: slot 0 bought for 250 gold`, héros ré-apparu à (-15935, 300).

PNG OUVERTS et vus :
- playtest_death_before : héros (anneau doré) et 4 Lanciers bleus devant le forum ; MAIS le bloc héros dit « Niveau 1, PV 450/450 » alors que la bannière dit « Paladin-Commandant : niveau 3 / Palier 1 atteint » (rafraîchissement du bloc en retard de quelques images, non bloquant).
- playtest_death_fallen : le héros est VIVANT (PV 522/522, aucun délai ni prix), pas de Lanciers rouges, note 1 seule dans le panneau : la capture a lieu dans les 0,2 s après `hol.Hero.Kill 0` (journal : capture à 20:21:44.116, ordre à .1), avant que le HUD ne montre la mort : NON conforme au .look (« le bloc du bas affiche le délai et le prix, 6 Lanciers ennemis arrivent »).
- playtest_death_ready : héros mort (PV 0/522), bouton « Ramener le héros — 250 or », alerte « Votre héros est prêt à revenir », 6 Lanciers rouges autour du forum, mes 4 Lanciers à terre (Population 0/70), notes 1-3 : CONFORME.
- playtest_death_back : Or 5950 (6200 - 250) mais le bloc héros dit encore PV 0/522 avec le bouton « Ramener le héros — 250 or » et l'alerte « prêt à revenir » ; aucun héros près du forum : la capture est prise 2 images après l'achat (journal : achat step 74, capture step 76) : NON conforme au .look (« le héros est revenu près du forum »).

Remarque : la note 4 « Ton héros est revenu » est écrite avant que le HUD ne le montre.
Remarque : le .look et la note 3 contiennent des guillemets « » mal encodés à la lecture par preflight (« ® Ramener ¯ ») ; dans le jeu le panneau les affiche bien.

NON VÉRIFIÉ : le héros réellement revenu à l'écran ; le prix qui baisse sur 60 s (un seul prix, 250, vu) ; le geste réel (mort au combat, clic sur le bouton) ; l'issue des 6 Lanciers contre les 4 (les 4 meurent, la note 2 demande « fenêtre de faiblesse ou défense jouable » : pas tranché).

Lancer UNE commande (non prête) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 playtest_hero_death -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 1 min 20 + touche Entrée. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
