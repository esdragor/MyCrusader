# Auto_Journey : la file de travail
CELEBORN-18 : sa ligne de l'ancien fichier est dans Docs\Archive\Auto_Journey_File_2026-11-07.md (le texte long de chaque ligne ouverte y est copié tel quel).
Mode d'emploi 1/5 : procédure dans `Auto_Journey.md` (« La file de travail ») ; une tâche = UNE ligne courte (≤ 300 caractères), le détail va au journal `*InProgress.md` ou dans `Retours\`, jamais ici.
Mode d'emploi 2/5 : états `[ ]` à faire, `[~]` en cours, `[x]` faite (à archiver au prochain nettoyage), `[?]` attend l'utilisateur (voir `Auto_Journey_Retours.md`), `[B]` bloquée par une autre tâche.
Mode d'emploi 3/5 : étiquettes `sans C++` (relançable sans toucher au plafond de 3 agents C++), `C++`, `attend l'utilisateur` ; lignes regroupées par worktree.
Mode d'emploi 4/5 : `session_gate.ps1 -Phase Launch` exige au moins 3 lignes `sans C++` ouvertes ; le dernier agent d'un worktree ajoute la suivante qu'il voit venir.
Mode d'emploi 5/5 : nettoyage du 2026-11-07 (leçon L6-2) : lignes faites ou périmées dans `Docs\Archive\Auto_Journey_File_2026-11-07.md` ; encodage UTF-8 avec BOM et fins de ligne CRLF partout.

## Arwen — `heal-sleep` — Audio, IA, outils
- [?] (sans C++, attend l'utilisateur) Sons doux : écouter les 5 familles audio_compare_buildings_*, audio_compare_training et les 6 alertes (candidats faits), choisir ; `E-audio_compare*.md`, `Q-audio-candidates.md`.
- [ ] (C++, après l'écoute) Audio B5 : transmettre le type du bâtiment sélectionné au script, CueNameFor puis UHOLAudioPlayer.Play ; `AudioInProgress.md`.
- [~] (sans C++) Neutres N6 (IA du joueur, attend NQ1/NQ2 de Retours) et N7 ; N0-N5 faites ; `NeutralsInProgress.md`.
- [ ] (sans C++) Technologies (GDD § 11) ou Arbre de talents (§ 9.6) : ouvrir le `*InProgress.md` et le plan si la file se vide.

## Celeborn — `hero-j8-close` — Mêlée et clôture de J8 (`NeutralsInProgress.md`, `DuelInProgress.md`)
- [?] (C++, attend l'utilisateur) Mêlée : 4361123 NON validé (melee_stuck ON 0/8 contre ALLOFF 6/8) ; Celeborn-18 cherche le correctif fautif ; décision dans `Retours\Q-melee-stall.md`.
  - Celeborn-19 : aucune combinaison de cvars <= ALLOFF partout (fautif = SlotBlockedStopDistance 250, mais le couper casse watch) ; piste : distance 120/150 a mesurer (cvar, sans C++) ; voir NeutralsInProgress.md.
- [ ] (C++, agent neuf) BUG-melee-stuck-attack : 2 soldats figés devant la cible (diplomacy_qground) ; A/B SlotDetourMaxSteps et FarPlaceRetarget ; test rouge = le soldat frappe ; `Retours\BUG-melee-stuck-attack.md`.
- [ ] (C++, test) L8-2 : audit `hol.Test.Sanity` « cible à portée, aucune attaque depuis N s » ; c'est le test rouge de BUG-melee-stuck-attack.
- [ ] (C++) Neutres 2/4 : hypothèse SelfSide (place du côté du soldat, FindReachableEnemy), puis tremblement de cap de la marche en bloc ; c5f2279 partiel.
- [ ] (C++) D641 : soldats qui tournent en cercle en mêlée (playtest_hero_army, neutrals, diplomacy_qground) ; rejouer sur main fusionné ; mêmes causes que les lignes ci-dessus.
- [ ] (C++) Mouvement de groupe à la vitesse du plus lent (D617) : audit group_march instable (3 Paysans de l'IA, shakes) ; eeaab39 partiel ; `DuelInProgress.md`.
- [B] (sans C++) Supprimer les scénarios de repro neutrals_d3, nv1, nv2, nv3, nvpb0, nvpw1, neutrals_trace quand la mêlée est close (`-Parts net` 2 fois).

## Elrond — `tiers` — Intégration
- Rien d'ouvert dans la file (Paliers, Talents, Relais, perf S2-S4 : voir `EtapeDefini.md`).

## Faramir — `fog` — Brouillard, Siège (`FogInProgress.md`)
- [?] (sans C++, attend l'utilisateur) `Retours\Q-scenario-panel-covers-world.md` : le panneau du scénario couvre le monde dans ai_evolving_hard ; défaut A codé.

## Galadriel — `galadriel-tab` — Interface, Marché
- [?] (attend l'utilisateur) Légions : relire le japonais des 7 noms (proposition de Galadriel-8) ; `Retours\E-legions_roster_names.md` ; numéro D au GDD par la revue.

## Glorfindel — `net-playtest` — Légions, Sites, Déconnexion, Réseau
- [?] (attend l'utilisateur) Réseau : essai E5 avec un ami ; ensuite seulement remesure MaxServers (D502), r9, R8d ; `NetPlaytestInProgress.md`.
- [ ] (C++) Légions : modèle client (noms et filtre Hallebardes faits) ; `LegionsInProgress.md`.
- [?] (C++, dire à l'utilisateur AVANT) Palier 3 G1 : points d'appel de MinTier (production, grille, IA) ; Homme d'armes, Arbalétrier, Cavalier léger au palier 1 ; `Palier3InProgress.md`.
- [ ] (C++) Sites sacrés : S5 vue répliquée et visibilité, S6 or, S10 victoire, S7 option de partie, colonne unité sainte ; `SacredSitesInProgress.md`.
- [ ] (sans C++) Déconnexion : scénarios disconnect_hold, disconnect_team, disconnect_1v1, disconnect_pause ; `DisconnectInProgress.md`.
- [ ] (C++) Déconnexion : Report sans mode IA ; `DisconnectInProgress.md`.
- [B] (sans C++) Playtest PT3 `match_report.py` et PT6 `match_batch.ps1` : bloqués par PT2 (C++ de match) ; `PlaytestInProgress.md`.

## Haldir — `maps` — Cartes (`MapsInProgress.md`)
- [?] (attend l'utilisateur) Choisir une lettre A-H dans `Retours\E-glade-sweep.md` (clairières, essai non joué, resynchroniser Haldir d'abord) et répondre à `Q-sites-nombre.md` (défaut 1 anneau).
- [ ] (C++) Sites sacrés S4 : classe AHOLSiteLayout + test Map.Table ; puis spawn_sites/export_layout dans common.py ; `SacredSitesInProgress.md`.
- [B] (sans C++, éditeur seul) UNE régénération des cartes (plan P1-P5, `MapsInProgress.md`) après lettre + Q-sites-nombre + AHOLSiteLayout ; puis perf S5 à 14 400 arbres, hinterland_paths, aiduel.
- [?] (attend l'utilisateur) El Dorado : valeurs finales après CQ2 (`hol.Terrain.MarshSpeedPercent`, `HOL_ELDORADO_GOLD`), marche en bloc dans le marais non testée, essai map_eldorado.

## Sauron — `visual` — Biomes (`BiomeInProgress.md`)
- [ ] (sans C++, machine calme, ~25 min) Mesures S1/S2 du décor générique des Biomes (2128 pièces HISM sur la Plaine) par `biome_decor_cost.ps1`.
- [?] (attend l'utilisateur) Biomes B4 : colonne biome de Maps.csv après son jugement des cinq looks (Q2 = B) ; `E-biome_decor_tour.md`, `Q-biomes-look.md`.

## Transverse (processus et outils, worktree libre)
- [ ] (sans C++) Refaire en essais conformes les essais ratés du 2026-10-31 encore ouverts (duel_challenge, neutrals, map_eldorado, market, talents, fog_orders, fog_trees) ; vérifier à l'étape 0 lesquels restent.
- [ ] (sans C++, outil) L8-9 : sync_worktrees.ps1 au début ET à la fin de chaque Auto_Journey ; ajouter l'étape dans `Auto_Journey.md` (fait en partie le 2026-11-06).
- [ ] (sans C++, outil) `quick_check -Parts tests` seul joue aussi une « sequence » (7 frames), cause non cherchée.
