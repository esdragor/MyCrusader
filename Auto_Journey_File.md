# Auto_Journey : la file de travail
Nettoyage du 2026-11-10 : lignes faites ou périmées déplacées telles quelles dans `Docs\Archive\Auto_Journey_File_2026-11-10_nettoyage.md` (précédent : `Docs\Archive\Auto_Journey_File_2026-11-07.md`, qui garde le texte long de l'ancienne ligne Celeborn-18).
Mode d'emploi 1/5 : procédure dans `Auto_Journey.md` (« La file de travail ») ; une tâche = UNE ligne courte (≤ 300 caractères), le détail va au journal `*InProgress.md` ou dans `Retours\`, jamais ici.
Mode d'emploi 2/5 : états `[ ]` à faire, `[~]` en cours, `[x]` faite (à archiver au prochain nettoyage), `[?]` attend l'utilisateur (voir `Auto_Journey_Retours.md`), `[B]` bloquée par une autre tâche.
Mode d'emploi 3/5 : étiquettes `sans C++` (relançable sans toucher au plafond de 3 agents C++), `C++`, `attend l'utilisateur` ; lignes regroupées par worktree.
Mode d'emploi 4/5 : `session_gate.ps1 -Phase Launch` exige au moins 3 lignes `sans C++` ouvertes ; le dernier agent d'un worktree ajoute la suivante qu'il voit venir.
Mode d'emploi 5/5 : nettoyage du 2026-11-10 (demande de l'utilisateur) : fichier en UTF-8 SANS BOM, fins de ligne CRLF ; l'ancienne consigne « avec BOM » de la ligne 7 est archivée.

## Prochaine séance (par ordre de valeur ; détail dans les lignes ci-dessous)
1. Mêlée (Celeborn) : A/B N=16 melee_stuck contre melee_stuck_on_bf avant de changer le défaut ; puis les lignes mêlée ouvertes.
2. neutrals dans une clairière (Sauron) : `hol.Decor.ClearNear` dans le scénario, preflight, PNG, E-neutrals.
3. Rejouer sur main fusionné les essais de `Retours\ESSAIS-A-JOUER.md` et nommer un intégrateur de recette indépendant (Elrond).
4. GDD : écrire D753-D766 au § 22 puis purger les 15 fiches de `Retours\` déjà reportées (session principale).
5. Outils : commit_docs.ps1, play.ps1 -Hidden, curseur virtuel centré, session_gate Launch, build.ps1 -Wait.
6. Glorfindel : `MI_Ground_Plain.uasset` modifié non commité (question à l'utilisateur, puis restaurer ou commiter).
7. Modèle et animations de l'Arquebusier (Sauron, après le lot ressources et décor PQ-1).
8. Régénération unique des 7 cartes (TÂCHE À PART, Haldir) : `bSitesLaidByMaps = true`, styles de `Q-glade-style.md`, puis tests, crossings, hinterland_paths, aiduel.
9. Build de rattrapage des worktrees (`sync_worktrees.ps1` puis `Tools\build.ps1`) avant tout essai.
10. Mesures S1/S2 (machine calme, seules) : `biome_decor_cost.ps1 -Rounds 3` du décor générique des Biomes.

## Arwen — `heal-sleep` — Audio, IA, Neutres
- [ ] (IA, D751) l'IA doit marcher en groupe a la vitesse du plus lent pour ses vagues (escorte etiree sur 70 m, tetes a 300 cm/s contre 180-220) : ouvrir le chantier, test rouge sur la marche de la vague.
- [~] (sans C++) Neutres N6 (IA du joueur, attend NQ1/NQ2 de Retours) et N7 ; N0-N5 faites ; `NeutralsInProgress.md`.
- [ ] (sans C++, Arwen, ~25 min) Audio : scénario `audio_select_gain` (avant/après limiteur, Q-audio-select-gain = B, 558fd57 sur d751-root) : preflight PASS, PNG ouverts, fiche `Retours\E-audio_select_gain.md` (GESTE, SON, branches: d751-root) ; `AudioInProgress.md`.
- [ ] (C++, Arwen) Neutres N7 : exposer GetCampState (vivant / nettoyé) au script pour l'indicateur de camp ; `NeutralsInProgress.md` ; 1er test rouge : un test script qui lit l'état d'un camp (absent aujourd'hui).

## Celeborn — `hero-j8-close` — Mêlée et clôture de J8 (`NeutralsInProgress.md`, `DuelInProgress.md`)
- [ ] (C++, agent neuf) BUG-melee-stuck-attack : 2 soldats figés devant la cible (diplomacy_qground) ; A/B SlotDetourMaxSteps et FarPlaceRetarget ; test rouge = le soldat frappe ; `Retours\BUG-melee-stuck-attack.md`.
- [ ] (C++, test) L8-2 : audit `hol.Test.Sanity` « cible à portée, aucune attaque depuis N s » ; c'est le test rouge de BUG-melee-stuck-attack.
- [ ] (C++) Neutres 2/4 : hypothèse SelfSide (place du côté du soldat, FindReachableEnemy), puis tremblement de cap de la marche en bloc ; c5f2279 partiel.
- [ ] (C++) D641 : soldats qui tournent en cercle en mêlée (playtest_hero_army, neutrals, diplomacy_qground) ; rejouer sur main fusionné ; mêmes causes que les lignes ci-dessus.
- [ ] (C++) Mouvement de groupe à la vitesse du plus lent (D617) : audit group_march instable (3 Paysans de l'IA, shakes) ; eeaab39 partiel ; `DuelInProgress.md`.
- [B] (sans C++) Supprimer les scénarios de repro neutrals_d3, nv1, nv2, nv3, nvpb0, nvpw1, neutrals_trace quand la mêlée est close (`-Parts net` 2 fois).
- [ ] (sans C++, Celeborn, ~25 min) Mêlée : `repeat_scenario melee_stuck_old -Runs 8` au défaut final (KeepStuckCount 0 ; watch 8/8 et melee_stuck 7/8 faits à la fusion du 2026-11-09 13:1x) ; `NeutralsInProgress.md` ; critère informatif >= 5/8.
- [ ] (sans C++, Celeborn, ~25 min) Mêlée : mesurer PlaceBoxedIn 1 + KeepStuckCount 1 combinés (jamais mesurés, Celeborn-24) sur melee_stuck, N=8 ; `NeutralsInProgress.md` ; témoin : scénario `melee_stuck_on_ksc` 8/8.
- [ ] (sans C++, Celeborn, ~20 min) D641 : rejouer `playtest_hero_army` et `neutrals` sur main fusionné (Sanity spin) et noter dans `Retours\BUG-melee-spin-sanity.md` et `BUG-playtest_hero_army-spin.md` lesquels passent ; 1er test : preflight playtest_hero_army.
- [ ] (2026-11-10, Celeborn, sans C++, ~30 min) Melee : A/B N=16 (ab_matrix.ps1) melee_stuck vs melee_stuck_on_bf (RearKeepBothFull 1) avant de changer le defaut ; reporter les chiffres de playtest_hero_army 0/4 et neutrals 2/4 dans Retours\BUG-melee-spin-sanity.md et BUG-playtest_hero_army-spin.md.

## Elrond — `tiers` — Intégration, Paliers, Talents
- (état du 2026-11-09 : Paliers, Talents, Relais, perf S2-S4 : voir `EtapeDefini.md`).
- [ ] (C++, Elrond ou Galadriel) Palier 3 G2 (pénétration d'armure, AUTORISÉE) puis G3 Arquebusier (70 PV, vitesse 720, attaque 40, intervalle 4,0 s, portée 1500, 60 N + 80 O, 25 s, pénétration 50 %, pas contre un bâtiment) puis G4 ; test rouge d'abord ; `Palier3InProgress.md`.
- [ ] (sans C++, Elrond, ~15 min) Valeurs : après la fusion de `d751-root` (ebe6d57, valeurs orphelins), `quick_check -Parts values` doit repasser au vert (rouge connu du 2026-11-09 13:1x) ; sinon ligne dans `Tools\Tests\known_reds.txt`.
- [ ] (C++, Elrond ou Galadriel) Talents T4b « Mains du guérisseur » (Moine : soins +20 % dans l'aura) ; `TalentsInProgress.md` (lot écrit par Haldir-10) ; 1er test rouge : PV rendus par le Moine dans l'aura, sur la vraie chaîne.
- [ ] (2026-11-10, Elrond, sans C++) Rejouer sur main fusionne (-Phase Essay) les essais de Retours\ESSAIS-A-JOUER.md (date du 2026-11-09) et nommer un integrateur de recette independant.

## Faramir — `fog` — Brouillard, Siège (`FogInProgress.md`) : aucune ligne ouverte (toutes faites, archivées le 2026-11-10)

## Galadriel — `galadriel-tab` — Interface, Marché, Légions
- [?] (attend l'utilisateur) Légions : relire le japonais des 7 noms (proposition de Galadriel-8) ; `Retours\E-legions_roster_names.md` ; numéro D au GDD par la revue.
- [ ] (C++, Galadriel) Légions L4 : Guerrier Damné profil A + rage nécrotique A ; plan en 8 étapes dans `LegionsInProgress.md`.
- (Galadriel : le reste de ses journaux est du C++ (code de L4, tribut vers un client distant) ou attend l'utilisateur ; rien d'autre de sans C++ n'en sort.)

## Glorfindel — `net-playtest` — Légions, Sites, Déconnexion, Réseau
- [?] (attend l'utilisateur) Réseau : essai E5 avec un ami ; ensuite seulement remesure MaxServers (D502), r9, R8d ; `NetPlaytestInProgress.md`.
- [ ] (C++) Légions : modèle client (noms et filtre Hallebardes faits) ; `LegionsInProgress.md`.
- [?] (C++, dire à l'utilisateur AVANT) Palier 3 G1 : points d'appel de MinTier (production, grille, IA) ; Homme d'armes, Arbalétrier, Cavalier léger au palier 1 ; `Palier3InProgress.md`.
- [ ] (C++) Sites sacrés : S5 vue répliquée et visibilité, S6 or, S10 victoire, S7 option de partie, colonne unité sainte ; `SacredSitesInProgress.md`.
- [B] (sans C++) Playtest PT3 `match_report.py` et PT6 `match_batch.ps1` : bloqués par PT2 (C++ de match) ; `PlaytestInProgress.md`.
- [ ] (sans C++, Glorfindel, ~15 min) Scénarios `legions_roster_gesture` et `_dawn` (ClickUnit durci 798d1d3) : `play.ps1 <nom> -Hidden -Root <worktree>`, PNG ouverts ; 1er test : `pan_exposure_test.ps1` PASS ; `LegionsInProgress.md`.
- [ ] (sans C++, Glorfindel, ~20 min) Déconnexion : `quick_check -Full` à la fusion de net-playtest (« Reste » de Glorfindel-18) ; `-Parts tests,net` verts avant ; `DisconnectInProgress.md`.
- [ ] (2026-11-10, Glorfindel, sans C++) sync_worktrees saute Glorfindel : Content/Rendering/MI_Ground_Plain.uasset modifie non commite (artefact de regeneration probable, sauvegarde Temp\MI_Ground_Plain.glorfindel.uasset) : question a l utilisateur, puis restaurer ou commiter.

## Haldir — `maps` — Cartes (`MapsInProgress.md`)
- [ ] (cartes, Haldir, TÂCHE À PART, éditeur seul) clairières variées par carte (réponse « oui » du 2026-11-09 sur `glade_sweep`) : choisir le style de chaque carte générée (B/C Forêt, D/E Plaine, G Steppe...), réglage par carte dans `Tools\Editor\maps\common.py`, UNE seule régénération des 7 cartes (`-Multiprocess`, au plus 2 éditeurs, avec `SITES_CLEAR_SOURCES = True` et `bSitesLaidByMaps`), puis `tests,sizes`, `crossings`, `hinterland_paths`, `aiduel`, mesure de perf et scénario `maps_review` ; les forêts peintes à la main ne sont pas touchées.
- [B] (sans C++, éditeur seul) UNE régénération des cartes (plan P1-P5, `MapsInProgress.md`) après lettre + Q-sites-nombre + AHOLSiteLayout ; puis perf S5 à 14 400 arbres, aiduel. (2026-11-08 Haldir-6 : lenteur de Hinterland résolue, sites_test complet 24 s, hinterland_paths Petite PASS ; reste hinterland_paths Moyenne/Grande à rejouer après régénération.)
- [?] (attend l'utilisateur) El Dorado : valeurs finales après CQ2 (`hol.Terrain.MarshSpeedPercent`, `HOL_ELDORADO_GOLD`), marche en bloc dans le marais non testée, essai map_eldorado.
- [ ] (C++ puis éditeur, Haldir) `bSitesLaidByMaps = true` (une ligne de `HOLMapTableTests.cpp`, test rouge = couche de sites exigée par carte) avant la régénération unique des 7 cartes ; `MapsInProgress.md` (P3).
- [ ] (cartes, C++ pour bSitesLaidByMaps) regeneration unique des 7 cartes apres la lettre glade_sweep de l'utilisateur ; bSitesLaidByMaps = true ; hinterland_paths Moyenne/Grande.
- (Haldir : avec les lignes EN TÊTE 12 (éditeur) et 13 (SITES_CLEAR_SOURCES), ses « Reste » sans C++ sont couverts ; la régénération attend les styles de `Q-glade-style.md`.)

## Sauron — `visual` — Biomes, Placeholders, Neutres (`BiomeInProgress.md`, `PlaceholdersInProgress.md`)
- [ ] (sans C++, machine calme, ~25 min) Mesures S1/S2 du décor générique des Biomes (2128 pièces HISM sur la Plaine) par `biome_decor_cost.ps1`.
- [?] (attend l'utilisateur) Biomes B4 : colonne biome de Maps.csv après son jugement des cinq looks (Q2 = B) ; `E-biome_decor_tour.md`, `Q-biomes-look.md`.
- [ ] (C++, Sauron, EN TETE, reponse utilisateur 2026-11-09 17:07 Q-neutrals-trees-decor = A) commande de test C++ qui retire les arbres (decor de la carte, pas des sources) autour d'un point, pour que le camp du scenario `neutrals` soit vu dans une clairiere ; puis refaire `neutrals` (preflight + PNG ouverts) et E-neutrals ; journal ScenariosInProgress.md, fiche Retours\Q-neutrals-trees-decor.md.
- [ ] (sans C++, Sauron, ~25 min) Placeholders lot ressources et décor (PQ-1 = A) : dix échantillons vus dans le jeu, l'utilisateur dit aimé / pas aimé ; `PlaceholdersInProgress.md` ; 1er test : `Tools\check_placeholders.ps1` liste les lignes du lot.
- [ ] (sans C++, Sauron, ~20 min) `Placeholders.txt` : trancher à l'écran les 6 lignes `[?]` (arbres Herbe fraîche, Steppe sèche, Forêt sombre ; rochers par biome ; trajets d'ordres ; icônes des talents), PNG ouverts ; 1er test : `Tools\check_placeholders.ps1`.
- [?] (sans C++, attend l'utilisateur) `Retours\Q-neutrals-camp-cache.md` : camp du scénario `neutrals` caché dans le bosquet ; défaut = scénario inchangé ; E-neutrals reste LOGIQUE SEULEMENT.
- [ ] (2026-11-07 mesure, machine calme, Sauron/visual) biome_decor_cost.ps1 -Rounds 3 (~25 min) : S1/S2 du decor generique ; lancer seul, sans autre editeur.
- [ ] (machine calme, HORS SEANCE) biome_decor_cost.ps1 -Rounds 3 (aucun chiffre le 2026-11-08 : machine jamais libre).
- [ ] (2026-11-10, Sauron, sans C++, ~20 min) neutrals dans une clairiere : ajouter `hol.Decor.ClearNear 0 0 2500` + `waitlog Decor.ClearNear 0 0 2500: took` avant `hol.Camp.Add 0 0 3 Spearman` dans `Tools/Scenarios/neutrals.txt`, preflight, PNG, E-neutrals ; la commande n a jamais ete lancee en jeu (VisualInProgress.md).
- [ ] (2026-11-10, Sauron, apres le lot ressources et decor PQ-1) Modele de l'Arquebusier ET ses animations (marche, tir a poudre, rechargement, mort ; aujourd'hui celles de l'Arbalétrier) : corps propre, fumee de poudre, son de tir (Placeholders.txt ligne 27) ; dix echantillons vus dans le jeu, l'utilisateur dit aime / pas aime ; Palier3InProgress.md.

## Outils et processus (worktree libre)
- [ ] (sans C++) Refaire en essais conformes les essais ratés du 2026-10-31 encore ouverts (duel_challenge, neutrals, map_eldorado, market, talents, fog_orders, fog_trees) ; vérifier à l'étape 0 lesquels restent. 2026-11-09 (Sauron-tools) : duel_challenge, talents, market, fog_orders, map_eldorado déjà refaits et VU À L'ÉCRAN (Auto_Journey_Retours / Jour) ; duel_challenge_accept refait (`Retours\E-duel_challenge_accept.md`, preflight PASS, GESTE interne) ; restent LOGIQUE SEULEMENT : neutrals (tremblement, rouge ~2/5, mêlée) et fog_trees (gris à juger, grésillement non reproduit).
- [ ] (fait en partie : 232d6eb dans main (qground) ; ClickUnit durci par 798d1d3 et 4c1db9f sur net-playtest, pas encore dans main) (sans C++) neutrals_qground et diplomacy_qground_watch : meme risque pan_exposure que fog_trees (clic a 125 images de la camera) ; corriger la camera avant le clic ; ClickUnit -1 enemy fragile (legions_roster_gesture(_dawn)).
- [ ] (sans C++) rejouer unit_stats_attack(_enemy) sur un binaire d'apres G1 (PNG ouverts) avant de les donner a l'utilisateur ; E-audio_panel / E-disconnect_review a donner apres les fusions.
- [ ] (processus) plafond d'appels : 45 vus par l'agent respecte a +-4 sauf 3 agents (115, 81, 55) ; l'enonce d'un agent ne demande JAMAIS play.ps1 fenetre ; un agent de fusion 'sans C++' verifie la date du binaire ; les PNG des pre-vols de fusion sont ouverts par l'agent d'integration avant de donner un essai.
- [ ] (2026-11-10, outil, sans C++) commit_docs.ps1 doit couvrir Placeholders*.txt/md, *InProgress.md suivis, Docs/SessionEnd.txt ; play.ps1 -Hidden doit fermer ses editeurs ; curseur virtuel centre en mode cache (pas seulement dans fog_trees) ; session_gate Launch : plafond C++ en WARN si la tache est sans C++, binaire perime = FAIL si la tache joue un scenario ; build.ps1 -Wait (un seul appel bloquant).

## Session principale (docs)
- [ ] (docs, session principale) écrire D753 à D766 au GDD § 22 (réponses Q1 à Q14) ; prochain numéro D767.
- [ ] (2026-11-10, session principale, docs) GDD : decisions D753-D766 a ecrire (relire § 22 avant) ; purger les 15 fiches de Retours\ qui portent une reponse deja reportee (liste : session_gate -Phase Close).

