# ESSAIS À JOUER (agent d'intégration Elrond, 2026-11-09 10:50-11:05, build de main 60013e8 fusionné, binaire de 10:37)

Tous préflightés PASS sur le worktree Elrond (= main 60013e8), PNG ouverts et vus (fiches `Retours\E-*.md`), `session_gate -Phase Essay -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` sans FAIL pour chacun. Total environ 8 min. Réponse attendue pour chacun : `OK`, `BUG: ...` ou `REGLAGE: ...`. Aucun n'a été joué par un humain ; ce sont tous des essais d'AFFICHAGE (GESTE: interne dans les fiches : le clic ou le défi du scénario est un appel interne, c'est à toi de faire le vrai geste).

## 1. unit_stats_attack_enemy (1 min) : TON bug du 2026-11-08 (siege_wave)
Pourquoi : bug que tu as signalé (ligne Attaque absente du bloc de stats d'une unité ennemie) ; rejoué sur main avant de te le donner.
Geste : clique chacune des 3 unités rouges (Bélier, Mangonneau, Lancier) : le bloc de stats porte « Attaque bâtiments 60 » / « Attaque 45 » / « Attaque 8 » ; le Lancier au contact d'un des tiens montre en plus la ligne d'activité « Attaque ».
Commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 unit_stats_attack_enemy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`

## 2. unit_stats_attack (1 min) : le même bloc, tes unités
Pourquoi : D750, la ligne Attaque pour tout type d'unité (Bélier « Attaque bâtiments 60 », Moine : aucune ligne Attaque).
Geste : clique tes 6 types d'unités (Bélier, Mangonneau, Lancier, Homme d'armes, Archer, Moine) et lis le bloc sous les PV. À confirmer : le Mangonneau n'affiche qu'« Attaque 45 », pas de ligne « bâtiments » (voulu ?).
Commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 unit_stats_attack -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`

## 3. sites_win (2 min) : Victoire sacrée
Pourquoi : nouvelle condition de victoire des sites sacrés, jamais vue par un humain ; libellés et cloche provisoires.
Geste : regarde seulement (rien à cliquer) : bandeau « Victoire sacrée : Vous dans m:ss » sous Triches, il se fige « (en attente) » quand un Lancier rouge entre dans la zone, annonces « Vous gagne dans une minute » puis « dans 10 secondes », puis fenêtre « Victoire ». Le son de la cloche n'est pas jugé (non entendu).
Commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 sites_win -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`

## 4. duel_challenge_accept (1 min) : fenêtre de défi du duel des héros
Pourquoi : le duel de héros (fenêtre « Défi envoyé au héros du joueur 2 », anneau rouge, PV) ; le bouton de défi n'existe pas encore (défi par commande).
Geste : Entrée au milieu ; regarde les unités autour de ton héros pendant la marche de 25 s (elles ne doivent pas trembler), la barre jaune de la fenêtre, l'anneau rouge.
Commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 duel_challenge_accept -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`

## 5. disconnect_review (2 min) : Déconnexion en 2 contre 2
Pourquoi : pastille « Connexion perdue » + bandeau à compte à rebours de Bob, pause (compte à rebours figé), pastille « IA » à la fin de la fenêtre, retour (la pastille disparaît). Le départ est simulé (`hol.Net.FakeAbsent`), le vrai départ réseau reste à jouer par `lobby_match.ps1`.
Geste : Entrée = temps suivant (4 temps).
Commande (`-PlayerName Alice` obligatoire) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 disconnect_review -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond -PlayerName Alice`

## Prêts mais NON donnés (non rejoués par moi, 50 appels d'outils atteints, ou LOGIQUE SEULEMENT)
- disconnect_1v1 : même scénario en 1 contre 1 (Bob perd, « Bob a quitté la partie », écran « Victoire ») ; préflight PASS vu par Glorfindel le 2026-11-08 sur la branche, pas rejoué sur main ; à ajouter après le 5 si tu veux.
- legions_roster_gesture, legions_roster_gesture_dawn : scénarios et .look présents dans main, jamais rejoués ni ouverts par moi (plus de budget) : NON DONNÉS, à préflighter par l'agent suivant (geste réel à vérifier d'abord).
- tech_armor (Forge F2) : pas de fichier .look, test de chaîne seulement : LOGIQUE SEULEMENT, jamais compté prêt.
- E-audio_panel, E-audio_compare_* : SON, jamais entendu par un agent : LOGIQUE SEULEMENT à vie pour les agents ; à écouter par toi seul, quand tu veux.
