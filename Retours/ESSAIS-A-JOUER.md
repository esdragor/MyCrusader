NETTOYAGE 2026-11-11 : fiches E-* encore dans Retours\ = E-disconnect_1v1 (non rejoue sur main), E-neutrals et E-playtest_hero_army (LOGIQUE SEULEMENT, Sanity D641), E-audio_compare_buildings (index), _stone, _metal, E-audio_panel (SON, a ecouter par l'utilisateur seul). Les 5 essais numerotes ci-dessous n'ont pas de fiche E-* : leurs scenarios sont dans Tools\Scenarios ; unit_stats_attack_enemy, unit_stats_attack, sites_win, duel_challenge_accept, disconnect_review sont deja joues le 2026-11-09 (voir ligne ci-dessus), donc a ne PAS redonner tels quels.
ATTENDU : a rejouer sur le main du 2026-11-10 avant de le donner (session_gate -Phase Essay) ; les essais arquebusier et fog_trees sont faits.
(Fiche datee du 2026-11-09, build 60013e8 : ne pas la donner telle quelle ; unit_stats_attack_enemy, unit_stats_attack, sites_win, duel_challenge_accept, disconnect_review, glade_sweep ont ete joues le 2026-11-09, voir REPONSES-2026-11-09.md.)

# ESSAIS À JOUER (2026-11-11, intégrateur indépendant Faramir, 15:09-15:25, main 521adcb fusionné, binaire du 15:03)

Les 7 scénarios ont été rejoués par un agent qui n'a écrit aucune de ces fonctionnalités : preflight PASS pour chacun, PNG ouverts (talents_heal en plein format ; les comparaisons en planches de contact 800x450 par image), fiches Retours\E-*.md mises à jour. Les commandes sont sans -Hidden (fenêtre pour toi). GESTE interne partout : aucun essai ne prouve la souris réelle ; l'objet de chacun est ce que tu VOIS.

## 1. talents_heal (1 min) : trait de soin du Moine doré « ++ »
Pourquoi : la nouvelle option de talent « Mains du guérisseur » doit changer le trait de soin, vert « + » sans talent, doré « ++ » avec. VU À L'ÉCRAN par moi sur les 3 PNG. Geste : T puis la case Capitaine rangée 3 (le scénario le fait par hol.HUD.Click, refais-le à la main si tu veux). À regarder : le « ++ » est petit ; défaut vu : il est dessiné PAR-DESSUS la fenêtre Arbre de talents. Fiche : Retours\E-talents_heal.md ; question liée : Retours\Q-talents-heal-visual.md.
Commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 talents_heal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`

## 2. legions_roster_gesture (1 min) : noms des Légions FR / EN / JA
Pourquoi : cliquer la Caserne des Légions montre Lancier squelette (A) et Garde noire (Z), le Cavalier funeste et le Bélier d'os portent leur nom, la Caserne d'Aube n'a aucune grille Légions ; EN et JA idem. VU À L'ÉCRAN sur 9 PNG. Défaut vu : la 3e case de la grille (E) affiche « Damned Warrior » en anglais dans l'interface FR et JA.
Commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 legions_roster_gesture -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`

## 3. legions_roster_gesture_dawn (1 min) : l'Aube garde ses noms
Pourquoi : la Caserne d'Aube montre Lancier, Homme d'armes, Hallebardes trempées ; le Lancier des Légions rouge cliqué montre « Lancier squelette ». VU À L'ÉCRAN sur 3 PNG. Défaut vu : un message d'éditeur « Shaders en préparation (3) » apparaît sur un PNG (absent du jeu empaqueté).
Commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 legions_roster_gesture_dawn -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`

## 4. talents_icons_compare (1 min) : icônes des talents, 2 options
Pourquoi : A = fenêtre Arbre de talents, cases prises teintées avec le nom en texte, sans image ; B = pastilles-lettres B M P sous la liste des talents pris dans le bloc du héros. VU À L'ÉCRAN sur 2 PNG. Dis aime / pas aime par lettre. Défaut vu : le bandeau du scénario affiche le texte de l'option A pendant l'option B.
Commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 talents_icons_compare -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`

## 5. trees_rocks_compare (2 min) : arbres et rochers sous 5 biomes
Pourquoi : mêmes arbres, sol, lumière et teinte des rochers de A (herbe fraîche) à E (neige) ; dis aime / pas aime par lettre. VU À L'ÉCRAN sur 10 PNG (planches). À savoir : A montre un sol gris-beige sans vert ; le bosquet est coupé par le bord droit du cadre.
Commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 trees_rocks_compare -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`

## 6. resources_compare (3 min, limite) : dix styles de ressources A à J
Pourquoi : arbre, baies, filon de pierre, filon d'or en 10 styles ; dis aime / pas aime par lettre. VU À L'ÉCRAN sur 10 PNG (planches). Dix pressions d'Entrée. À savoir : les baies sont petites (environ 20 px).
Commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 resources_compare -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir`

## Rejoués mais NON listés
- orders_marks_compare : marques A, B, C vues mais FRAGILES : anneau du clic d'environ 12 px et très pâle (A_click), ligne jaune fine, unités hors cadre sur A_click et B_click, bandeau resté sur le texte des options A et B pendant C (voir E-orders_marks_compare.md). À reprendre avant de le donner.
- Non rejoués ici : disconnect_1v1, neutrals, playtest_hero_army, audio_* (voir les sections plus bas).

---

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

