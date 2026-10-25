# Journey in progress

**Journal du chantier « parcours du joueur » (D252)**, ouvert le 2026-10-13. Après M2, le jeu avance dans l'ordre où
le joueur découvre une partie, chaque étape étant jouable : **J1** emplacement de départ et caméra RTS ; **J2** centre
principal par joueur, sélection ; **M2.5** réseau ; **J3** panneau du bâtiment et production d'un Paysan ; **J4** ordres
de déplacement et chemins ; **J5** récolte et ressources ; puis construction, armée et combat.

Cohabitation (mêmes règles que le chantier menu) : M2 tourne dans une autre session sur le module `HeroesOfLiberty` ;
décisions numérotées **D252 et suivantes**, tenues ici pendant le plan puis reportées au GDD (§ 21) ; commits sur `main`
par chemins explicites, **sauf travail en parallèle (règle de l'utilisateur, 2026-10-16) : une session qui doit
travailler en même temps qu'une autre dans la copie principale prend le worktree de réserve
`C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`** (le chantier animations a le sien, `HeroesOfLiberty-J5c`, branche `j5c-anims`).
Mode d'emploi de `HeroesOfLiberty-Elrond` : vérifier qu'il est libre (pas de fichier `IN_USE.txt` à sa racine), y écrire
`IN_USE.txt` (session, chantier, date), remettre sa branche au niveau de `main`, travailler sur une branche à soi,
compiler et tester là-bas (binaires et `Saved_Tests` séparés : un jeu d'essai ouvert ailleurs ne bloque pas la
compilation), fusionner dans `main` après validation de l'utilisateur, puis effacer `IN_USE.txt`. Les journaux
(`*InProgress.md`, `GDD.md`) restent écrits dans la copie principale. **Compiler et tester sans attendre les autres
sessions (2026-10-22)** : on compile toujours par `Tools\build.ps1 [-Root <copie>]`, qui se met seul dans la file
derrière une compilation en cours (un seul UnrealBuildTool à la fois sur le moteur commun, quelle que soit la copie) et
ne s'arrête (`BUSY`) que si Unreal tourne **dans la même copie** (DLL verrouillées). Les jeux, tests et éditeurs d'une
autre copie ne bloquent rien : pas d'annonce, pas d'attente. Tout lancement scripté d'Unreal porte `-LiveCoding=0`
(sinon il bloque les compilations de toutes les copies). Seules les mesures au repos (S1-S7, fin de chapitre) demandent
la machine pour elles seules. Jamais de code qui ne compile pas ; ne pas changer l'environnement du stress test
(GameMode global, pion, caméra) sans le vérifier sur S1 et S2.

---

## Où on en est

| Étape | État |
|---|---|
| D252 : ordre d'avancement par le parcours du joueur | **fait** (2026-10-13, au GDD § 16.6 et § 21) |
| J1 — plan (Q1 à Q5, D253 à D257) | **fait** (2026-10-13) |
| J1.1 Socle : contrôleur de partie, caméra locale, `IMC_Game`, réglages caméra | **fait** (2026-10-13) : compile, non essayé (pas encore de carte à emplacements) |
| J1.2 Carte : emplacement de départ, limites, carte neuve par script, caméra au-dessus de son emplacement | **fait** (2026-10-13) : `Plain`, caméra essayée (placement, flèches, molette, bord, borne) |
| J1.3 Réglages > Commandes : vitesse de caméra et défilement au bord dégrisés | **fait** (2026-10-13) |
| D258 appliquée (aucune ombre) | **fait** (2026-10-13) |
| J1.4 Vérification (D257) : tests, bout en bout à 2 et 8 joueurs, S1 et S2 courts, essai de l'utilisateur | **fait** (2026-10-13) : tests 14/14, `check_camera.ps1` PASS à 2 et 8, menu PASS en fr / en / ja, essai de l'utilisateur validé (repères d'échelle) ; **S1 / S2 au repos reportés** (machine occupée) |
| J1.5 Clôture : D253 à D257 au GDD | **fait** (2026-10-13) : D253 à D257 et paragraphe J1 au GDD, tests 14/14 sur la carte recréée, mémoire à jour |
| J2 — plan (Q1 à Q5, D259 à D264) | **fait** (2026-10-13) |
| J2.1 Base des bâtiments : classe C++ répliquée, table des types depuis un CSV | **fait** (2026-10-13) : compile sans avertissement, `Buildings.Types` et `Buildings.Appearance` verts (16 tests) |
| J2.2 Centre principal : apparition sur les emplacements occupés, couleur, repères de J1 retirés | **fait** (2026-10-13) : centres sur les 8 emplacements, D265 appliquée, forme reprise (toit plat, couleur non éclairée, deux tons) ; tests 16/16, caméra PASS à 2 et 8, menu PASS fr / en / ja, S1 et S2 complets |
| D266 (listes déroulantes dans « Nouvelle partie ») et D267 (vérification rapide `quick_check.ps1`) | **fait** (2026-10-13) : tests 16/16, `quick_check.ps1` PASS en ~30 s |
| J2.3 Sélection : canal, clic, Échap, cercle, survol | **fait** (2026-10-13) : tests 17/17 (`Buildings.Selection`), `quick_check.ps1` PASS avec la sélection à 2 et 8 joueurs (son centre, un centre adverse, le vide, Échap) |
| J2.4 Vérification (D264) | **fait** (2026-10-14) : essai de l'utilisateur → cercle et survol retirés (D259, D262 précisées) ; tests 17/17, `quick_check.ps1` PASS |
| J2.5 Clôture : D259 à D264 au GDD | **fait** (2026-10-14) : D259 à D264 au § 21, paragraphe « Parcours J2 » au § 16.6. **J2 close.** |
| M2.5 — plan (Q1 à Q5, D268 à D273) | **fait** (2026-10-14) |
| M2.5.1 Lancement : script serveur dédié + 2 clients sur `Plain`, partie à 2 clients qui marche | **fait** (2026-10-14) : `Tools/Tests/net_match.ps1` ; serveur cuit et mis en place, 2 clients côte à côte, chacun sur son emplacement, centres à la bonne couleur, sélection ; reste le regard de l'utilisateur |
| M2.5.2 Règles pures (réduction, règle d'envoi, prolongement, lissage) et leurs tests | **fait** (2026-10-14) : `Net/HOLUnitReplicationRules`, 6 tests `HeroesOfLiberty.Net.*`, `quick_check` PASS |
| M2.5.3 Serveur : acteur partagé, tableau rapide, marquage selon D269 | **fait** (2026-10-14) : 2 000 unités reçues par 2 clients ; 1,5 envoi/unité/s ; **~42 Ko/s par client** (> 32 Ko/s, à régler en M2.5.5) |
| M2.5.4 Client : entités d'affichage, horloge du serveur, prolongement et lissage, branchement de l'affichage | **fait** (2026-10-14) : 2 000 unités affichées par chaque client, animées, à la couleur du joueur ; corrections ~24 cm en moyenne, aucun placement direct |
| M2.5.5 S5 : script, rapport (bande passante, envois, écart affiché) | **fait** (2026-10-14) : morceaux en acteurs (fin du chemin des objets énormes) ; D274 (35 cm, 10 mises à jour/s) : **S5 PASS, 30,3 Ko/s par client, écart affiché 19,7 cm** ; tests 28/28 |
| M2.5.6 Partie à 2 clients dans `quick_check.ps1`, essai de l'utilisateur | **en partie faite** (2026-10-14) : partie « net » de `quick_check.ps1` (D275 : serveur = éditeur en `-server`, 2 clients hors écran) PASS, `quick_check` PASS en 44 s, tests 28/28 ; essai de l'utilisateur sur `Plain` **validé** (« tout est bon », après correction du retard des ressources)  ; essai sur S5 **validé** |
| M2.5.7 Clôture : D268 à D275 au GDD | **fait** (2026-10-14) : paragraphe marqué fait, ligne M2.5 du tableau du § 16.6 complétée. **M2.5 close.** Avant : D268 à D274 au § 21, paragraphe « Tranche réseau M2.5 » au § 16.6 (marqué « en cours ») ; reste, après M2.5.6, à le marquer fait et à compléter la ligne M2.5 du tableau du § 16.6 |
| J3 — plan (Q1 à Q5, D280 à D284 ; mené en parallèle de la fin de M2.5, dans une autre session) | **fait** (2026-10-14) |
| J3.1 Données et règles pures | **fait** (2026-10-14) : `Peasant` et colonnes de production dans les tables, `UHOLMatchSettings` (stock de départ), `Buildings/HOLProductionRules`, 3 tests `HeroesOfLiberty.Production.*` ; tests 28/28, `quick_check` PASS |
| J3.2 Économie du joueur (ressources, population) | **fait** (2026-10-14) : `AHOLPlayerState` (stock, population, au seul propriétaire), stock de départ au `PostLogin`, population par propriétaire dans `UHOLUnitSubsystem` |
| J3.3 File de production, ordre au serveur, apparition du Paysan | **fait** (2026-10-14) : file sur `AHOLBuilding` (propriétaire réseau = contrôleur), ordres par la file de commandes, `UHOLProductionSubsystem`, 12 actions de grille, `hol.Prod.*` ; vérifié sans fenêtre en solo et à 2 clients |
| J3.4 HUD (barre du haut, sélection, grille, file) | **fait** (2026-10-14) : `AHOLGameHUD` + `UHOLGameScreen` (module d'interface, écrit par un sous-agent), textes fr / en / ja, `hol.HUD.Report` / `hol.HUD.Click` |
| J3.5 Vérification (D284) | **fait** (2026-10-14) : `quick_check` PASS (production solo, et production à 2 clients : un Paysan par client vu chez les deux, ressources de l'autre jamais reçues), essai de l'utilisateur « très bien » puis plantage du ramasse-miettes corrigé et confirmé disparu, unités ×1,5, S1 / S2 courts PASS |
| J3.6 Clôture : D280 à D284 au GDD | **fait** (2026-10-14) : D280 à D284 au § 21 (D283 précisée : unités ×1,5), paragraphe « Parcours J3 » au § 16.6. **J3 close.** |
| J4 — plan (Q1 à Q5, décisions à partir de D285) | **fait** (2026-10-14) : Q1 tranchée (D285) ; plan validé (D286 à D289) |
| J4.1 Navmesh dynamique et obstacles | **fait** (2026-10-14) : `DynamicModifiersOnly`, agent de 60 cm, zone interdite sous les bâtiments, rochers et bosquet sur `Plain`, navmesh construit et sauvegardé par les scripts des cartes (`Plain` et `StressTest` refaites), `hol.Nav.Path`, partie « move » de `quick_check` PASS (40 s), S1 / S2 courts PASS |
| J4.2 Règles pures (paquets, places, attribution) | **fait** (2026-10-14) : `Simulation/HOLGroupMoveRules`, 3 tests `HeroesOfLiberty.Movement.*`, tests 31/31 |
| J4.3 Chemins dans la simulation | **fait** (2026-10-14) : ordre de groupe, places, paquets, couloir partagé avec place de côté, dernier tronçon vers la place ; 40 Paysans contournent centre et arête, arrivés en disque de ~5 m ; `quick_check` PASS (48 s) ; **S2 à remesurer machine au repos** (3,0-3,8 ms sous charge, voir journal) |
| J4.4 Ordre réseau et ralliement | **fait** (2026-10-14) : appel serveur avec contrôle du propriétaire dans la simulation, point de ralliement (file de commandes, répliqué au propriétaire), naissance avec ordre, places déjà prises sautées ; partie à 2 clients de `quick_check` : chacun voit les deux Paysans là où leurs propriétaires les ont envoyés (13 cm), le serveur ignore l'unité de l'autre ; `quick_check` PASS (46 s) |
| J4.5 Sélection des unités | **fait** (2026-10-14) : `Player/HOLUnitPicking` (projection, clic, cadre), sélection d'unités dans `AHOLPlayerController` (unité avant bâtiment, cadre au relâchement, Maj, unité adverse seule, unités disparues retirées), clic droit → ordre ou ralliement (sur le centre : effacé), `IA_SelectAdd` / `IA_Order` ; essai scripté solo conforme ; tests 32/32, `quick_check` PASS (39 s) |
| J4.6 Retour visuel et HUD | **fait** (2026-10-14) : `AHOLSelectionMarkers` (cercles sous les unités sélectionnées, carré sous le bâtiment, repère d'ordre de 0,5 s, fanion de ralliement), `M_GroundRing` / `M_GroundFrame` (`make_marker_materials.py`), rectangle du cadre dessiné par `AHOLGameHUD`, bloc de sélection d'une unité ou d'un groupe (fr / en / ja) ; essai scripté avec captures conforme ; tests 32/32, `quick_check` PASS (43 s) ; **1er essai de l'utilisateur** → retouches faites (D290, D291 : marqueurs blancs, repère réduit, Paysan à 6 m/s, triches de développement) ; `quick_check` PASS (42 s) ; 2e à 4e essais → D290 complétée (arrêtée ou pleine vitesse), D292 (aucun corps n'en traverse un autre), D293 (contourner avant de bousculer) ; `quick_check` PASS (45 s) ; **5e essai validé** (« super ») |
| J4.7 Vérification (D289) | **fait** (2026-10-15) : partie solo des ordres dans `quick_check` (PASS, 65-75 s), défauts corrigés (D294), S1 PASS ; **S2 à remesurer au repos**, **S5 PASS à 29,97 Ko/s** (39,7 avant) : D300 (identifiant de 10 bits par élément d'un tableau rapide, retiré par des tableaux simples : 35,6) puis D301 (hauteur posée au sol par le client, état de 59 bits) ; S2 court FAIL (4,41 ms par pas, seuil 3) → **D302 fait (2026-10-15) : S2 court PASS à 2,71 ms** (évitement, pilotage et orientation de Mass en parallèle, regard en avant allégé, mêmes résultats), S1 PASS (2,24 ms), `quick_check` PASS (64 s), S5 court PASS (30,0 Ko/s, 23,0 cm) |
| J4.8 Clôture : D285 à D294, D300 à D302 au GDD | **fait** (2026-10-15) : lignes au § 21 (D294 complétée, D274 précisée : les ~8 Ko/s d'en-têtes), paragraphe « Parcours J4 » et ligne M3 au § 16.6. **J4 close.** |
| J5 — plan (Q1 à Q5, D295 à D299 ; mené en parallèle de la fin de J4, dans une autre session) | **fait** (2026-10-14) : D295 à D299 ; trois parties jouables J5a (bois), J5b (4 ressources et HUD), J5c (vie de la récolte) ; code après la clôture de J4 |
| J5a.1 Données et règles pures | **fait** (2026-10-15) : table des sources (`SourceTypes.csv` → `/Game/Resources/SourceTypes`, `make_source_types.py`), taux et charge dans la table des unités, `Simulation/HOLGatherRules`, 4 tests `HeroesOfLiberty.Gather.*` ; tests 36/36, `quick_check` PASS (87 s) |
| J5a.2 Sources dans la partie | **fait** (2026-10-15) : registre `UHOLSourceSubsystem`, disposition `AHOLSourceLayout` posée par le script de `Plain` (40 arbres par emplacement, 320 en tout), affichage par instances (`AHOLSourceField`), un seul fournisseur de zones interdites à cylindres par instance (`UHOLSourceNavComponent` : seule la tuile d'une source épuisée se refait), morceaux répliqués de 63 sources (`AHOLSourceChunk`) ; `quick_check` PASS (83 s) avec les parties « sources » et « net » ; arbres de `StressTest` reportés à J5a.3 (créés par S7) |
| J5a.3 Boucle de récolte dans la simulation | **fait** (2026-10-15) : `UHOLGatherProcessor` (places au contact, récolte, dépôt le plus proche, épuisement, inactif ; ses marches passent par la file d'ordres), dépôts en emprise (`Accepts` dans la table des bâtiments), `hol.Units.Gather`, `hol.Gather.Report` ; **S7 PASS à 2,96 ms** (machine chargée ; avec 3 camps de bûcherons provisoires par joueur), S2 inchangé (mêmes recouvrements) ; `quick_check` PASS (89 s), tests 36/36 ; **S1 / S2 / S7 et le rendu à remesurer au repos** |
| J5a.4 Ordre réseau et ralliement | **fait** (2026-10-15) : clic droit sur une source (visée à l'écran comme une unité) → ordre de récolte vérifié par le serveur, repère bref sur la source ; ralliement sur une source (le Paysan né y récolte) ; activité « récolte » et charge dans l'état répliqué (1 bit, +7 pour un porteur) ; bloc du Paysan « Récolte — Bois 4/10 » (fr / en / ja) ; couronne des arbres remontée (les récolteurs se voient) ; `quick_check` PASS (caméra 8p : un raté d'à-coup, PASS au second essai), **S5 PASS à 31,3 Ko/s** (machine chargée) |
| J5a.5 Vérification et essai | **fait** (2026-10-15) : 1er essai → D303 (naissance vers le ralliement, bloc du Paysan à chaque changement, charge exacte au seul propriétaire de l'unité sélectionnée seule) ; 2e essai → D304 (orientation instantanée) ; 3e essai validé ; `quick_check` PASS (68 s) ; solo vérifié à la main par l'utilisateur ; **partie à 2 clients de la récolte** dans `quick_check` (charge suivie de D303, arbre épuisé chez les deux, stock de l'autre jamais reçu) PASS, `quick_check` PASS (70 s) ; essai de l'utilisateur (arbres, S7) validé ; D305 (réutilisation des chemins) faite ; **S1 / S2 / S7 au repos et S5 reportés** (machine chargée, choix de l'utilisateur) |
| J5a.6 Clôture : D295 à D299, D303 à D305 au GDD | **fait** (2026-10-15) : lignes au § 21 (D297 et D299 précisées par ce qui a été fait), paragraphe « Parcours J5a : le bois » au § 16.6, S7 dans les seuils des stress tests. **J5a close.** |
| J5b.1 Baies et filons | **fait** (2026-10-15) : `Plain` refaite (376 sources : 6 buissons de baies en face du bois de chaque emplacement, à 20-23 m ; 8 filons entre voisins, pierre et or alternés, à 60 m de chacun) ; places autour d'une source au pas de deux récolteurs (16 autour d'un filon au lieu de 12, arbres et baies inchangés) ; essai scripté solo : baies, pierre, or déchargés, charge perdue au changement ; `quick_check` PASS (69 s) ; essai de l'utilisateur → **D306** (charge perdue au premier point de la nouvelle ressource, pas à l'ordre), faite, `quick_check` PASS (72 s), **validée** ; D306 au GDD (§ 21, D296 et « Parcours J5a » précisés) |
| J5b.2 HUD de la récolte | **fait** (2026-10-16, **validée** par l'utilisateur, commitée) : D307 (activité « en tournée de récolte » dans les 3 bits de l'état, comptes de récolteurs au `PlayerState` du seul propriétaire) ; source sélectionnable (nom, « Reste : 1840 », anneau au sol), bloc d'un groupe (« 3 en récolte · 2 inactifs »), comptes par ressource « (2) » dans la barre du haut, compteur « Inactifs : N » cliquable et touche « . » (`IA_NextIdle` : « . », la touche « ; . » de l'AZERTY, « . » du pavé) ; textes fr / en / ja ; tests 36/36 (`Movement.Picking` étendu), essai scripté solo conforme, `quick_check` PASS (77 s) ; **essai de l'utilisateur** : « super », mais des Paysans font d'énormes détours dans leurs allers-retours → **D308** (suivi d'un chemin sans grand virage, détour gardé vérifié, sortie d'une poche du navmesh) : marches anormalement longues 37 → 2, `quick_check` PASS (75 s) ; 2e essai : moins de détours, mais les Paysans se gênent devant le centre → **D309** (choix de l'utilisateur : en aller-retour, un Paysan traverse toutes les unités ; plus de place autour d'une source → même ressource à 8 m, sinon inactif devant), faite, compile sans avertissement, `quick_check` PASS (75 s) ; 3e essai → empilement à la source et amas récolteurs / inactifs corrigés (réservations sur le point réel, toutes sources, points d'attente des inactifs), **massifs** (recherche de proche en proche dans tout le bois, D309 précisée), places serrées **à 100 cm** tournées vers le dépôt (capture d'AoE4 de l'utilisateur) ; `quick_check` PASS, tests 38/38, S7 court PASS (2,39 ms) ; **validée** (« très bien », `hol.Gather.SpotSpacing 100`) |
| J5b.3 Vérification et essai | **fait** (2026-10-16) : D309 précisée (point 6 : bosquet voisin à ~20 m, rangées d'attente sans limite), **validée**, commitée (`516ac75`) ; parties « gather » solo et 2 clients de `quick_check` (PASS en 77-78 s, tests 39/39, verbe `waitidle`, `SoftQuit`), correctif de `Stop` (inactifs empilés au dépôt : 29-56 → 115-120 cm), essai de l'utilisateur **validé**, commitées (`a067c11`) ; mesures au repos (`684045f`) : S1 2,184 ms, S2 2,738 ms, S7 2,155 ms par pas, S5 31 227 o/s par client (écart 21,6 cm), tous PASS ; 632 bois/s du S7 court = effet de fenêtre (régime 1 201 bois/s sur 180 s) ; Paysan sans place inactif **en gardant sa charge**, comme dans AoE4 : **confirmé** par l'utilisateur |
| J5b.4 Clôture : D307 à D309 au GDD | **fait** (2026-10-16) : lignes D307 à D309 au § 21 (D296 précisée par D309), paragraphe « Parcours J5b : les 4 ressources et le HUD » au § 16.6 (« Parcours J5a » précisé : les 15 m remplacés par les massifs). **J5b close.** |
| J5c.1 Animations de récolte | **fait** (2026-10-16, worktree `HeroesOfLiberty-J5c`, détail dans `AnimationsInProgress.md`) : source tranchée (faites par Claude, option A) ; chaîne de fabrication par script ; 6 séquences (hache, pioche, cueillette, marches chargées nourriture / bois / pierre-or) ; ressource récoltée dans les valeurs libres de l'activité (0 bit) ; choix chez le client (`HOLUnitAnimation::Choose`), Paysan tourné vers sa source ; pas de geste au dépôt ; tests 41/41, `quick_check` PASS ; essai de l'utilisateur **validé** (« c'est bon »), commitée (a9e4269), **fusionnée dans `main`** (8d18407, tests 47/47) ; A/B S7 : +0,15 ms par image au fil de jeu |
| J5c.2 Charge visible | **fait** (2026-10-16, worktree) : choix de l'utilisateur (hache, pioche, mains nues pour les baies ; panier, fagot, hotte de pierre ou d'or ; charge montrée à l'arrêt : repos chargés) ; `SKM_Peasant` (pièces fusionnées, outils sur `ik_hand_gun`, 4e float par instance, WPO), `TP_Peasant` ; tests 50/50, `quick_check` PASS ; essai **validé** (« très bien »), **fusionnée dans `main`** (fad246d, tests 54/54) ; A/B S7 : CPU inchangé, GPU +1,06 ms |
| J5c.3 « +10 » au dépôt | **fait** (2026-10-16, 98858d0) : « +N » au seul propriétaire (appel client non fiable par pas, 0 bit dans l'état des unités), `hol.Gather.LogPopups`, capture `gather_popup` et contrôles solo et 2 clients dans `quick_check` (PASS en 78 s, tests 39/39) ; essai **validé** (« très bien ») |
| J5c.4 Vérification et clôture : D318 à D323 au GDD | **fait** (2026-10-16) : mesures au repos de J6a.6 (`main` bfda9b4, toutes les animations et le bâtisseur) : S1 2,276 / S2 2,882 / S7 2,303 ms, S5 30 937 o/s par client (écart 23,3 cm), tous PASS, GPU de S7 3,34 ms (2,15 avant J5c) ; tests 62/62 et `quick_check` PASS à la dernière fusion ; partie « anims » de `quick_check` ajoutée en parallèle (partie « anims », 16 s : rapport `hol.Units.AnimReport` ; geste et pièce de chaque Paysan dessiné au travail, en marche chargée et debout chargé ; `quick_check` PASS en 163 s) ; D318 à D323 au § 21 (D298 précisée : fait en J5c), paragraphe « Parcours J5c : la vie de la récolte » au § 16.6, numéro suivant de la review (§ 22) passé à D324. **J5c close, J5 close.** |
| J6b.1 Camps | **fait** (2026-10-16, **validée** à l'essai le 2026-10-17 : « tout m'a l'air bon », commitée) : camp fini → ses bâtisseurs file vide récoltent la source acceptée la plus proche (`hol.Build.CampGatherRange`, 25 m, emprise au bord de la source), sinon chantier voisin / inactif ; ralliement sur un camp (fini ou non) → récolte ; 0 bit ; tests 63/63 (`Construction.CampSource`), partie `build camp` de `quick_check`, `quick_check` PASS en 202 s ; S7 inchangé (J6b.3) |
| J6b.2 Chemins coupés par une emprise | **fait** (2026-10-17, **validée** à l'essai : « super », commitée) : emprise nouvelle → chemins gardés de D305 qui la traversent oubliés, marcheurs dont le chemin la traverse recalculés une fois ses tuiles refaites (générateur + sondes), destination sous l'emprise sortie ; ligne `Buildings (D310): footprint ...` ; bouclage des Paysans enfermés corrigé ; 0 bit ; tests 64/64 (`Construction.CutPaths`), partie `build cut`, `quick_check` PASS en 259 s ; S7 2,289 / S2 2,758 ms PASS |
| J6b.2b Bâtiments rentrés dans leurs cases (D324) | **fait** (2026-10-17, **validée** à l'essai : « c'est bon », commitée) : forme visible à **90 %** des cases (`VisualInset=30`, choix de l'utilisateur après « trop grosses allées »), corps qui bloque rentré de **115 cm** (`BodyInset=115` : un Paysan mord ~30 cm dans le mur, « une épaule ») ; au premier essai, corps rentré de 105 cm (écart 2,1 m ; 90 cm fermait le passage), passage à la file et par l'écart ; tests 65/65 (`Construction.Body`), partie `build gap`, `quick_check` complet PASS en 364 s au réglage final ; A/B S2 / S7 sans surcoût (machine chargée) |
| J6b.3 Vérification | **fait** (2026-10-18, **validée** à l'essai : « tout est bon », commitée) : `quick_check` complet PASS (311 s, 20 parties, tests 65/65) ; S7 avec camps d'abord FAIL (3,41 ms : `HOLBuildProcessor` 1 ms, affichage des chantiers dans le pas) → `RiseStepRatio`, pièces `QueryOnly`, puis **D325** (apparence des bâtiments hors du pas, aucune pièce sur le serveur dédié) ; **mesures de fin de J6** par le nouvel outil `chapter_measures.ps1` (7,4 min) : S1 2,37, **S2 2,995 (moyenne de 3, au seuil)**, S7 2,60 ms, S5 31 005 o/s, 22,9 cm, tous PASS. Avant : 1re boucle faite, partie **2 clients des camps** dans `quick_check` (`net` + `build`) : chaque client bâtit un camp de bûcherons près de son bois et y dépose, voit le camp de l'autre (reçu au premier coup, puis fini), le serveur refuse un camp sur un arbre (`hol.Build.PlaceAtSource`), stock de l'autre jamais reçu ; aucun code C++ ; `quick_check` complet 324 s : tout PASS (`net 2 clients` compris, tests 65/65) sauf `build cut` (solo, non touché : aléa à relancer) ; puis `build cut` revue (contrôles de remises réalignés, blocage au coin corrigé : nouveau chemin d'une unité tenue sur son chemin, `PathStuckSteps`), `quick_check` complet PASS (350 s) ; **S7 bâtit ses 24 camps pendant la mesure** (D314, `hol.StressTest.S7BuildCamps`), fermeture sans bruit, `AHOLMapBounds::Find` corrigé ; S7 court OK |
| J6b.4 Clôture : D310 à D314, D324, D325 au GDD | **fait** (2026-10-18) : lignes D310 à D314, D324, D325 au § 21 (précisées par ce qui a été fait : case de 3 m, fondation fantôme, camp → récolte, F1 / F2 / F3 et Tab, deux marges de D324…), paragraphe « Parcours J6 : la construction » au § 16.6, § 7.3 précisé (F1 / F2 / F3 : tous les bâtiments de la catégorie ; Tab), § 22 : numéro suivant **D326**. **J6 close.** |
| J7a.1 Données et règles pures | **fait** (2026-10-18, commitée ; rien de visible en jeu) : `Simulation/HOLFormationRules` (largeurs Paysans / carré / mixte, largeur imposée, types par ligne, places, attribution sans croisement), les 9 scénarios de l'utilisateur reproduits ; colonnes de combat dans la table des unités (non lues), lignes `Spearman`, `ManAtArms`, `Barracks` (militaire, `BuildGridSlot` 12 = A de la 2e page) ; tests 69/69 (4 `Formation.*`, `*.Tables` étendus) |
| J7b.5b Lignes de la file d'ordres (D328) | **en cours** (2026-10-20) : lignes (un seul ordre compris), ordre de la file, 3 correctifs de la revue validés à l'essai et commités (c2b44e4) ; lignes de ralliement (bâtiment → fanion, unité née) et traits de 6,25 cm validés et commités (ed37f00) ; contrôles des lignes solo, ralliement, 2 clients (94eb04e), `-Full` PASS. **J7b.5b close** |
| J7b.6 Vérification et clôture de J7b | **fait** (2026-10-20) : contrôle du couteau du Paysan, `-Full` PASS (609 s, 83/83), essai complet du combat validé (A clic gardé pour plus tard). **J7b close** ; GDD à la clôture de J7 |
| J7a.2 Caserne | **fait** (2026-10-18, **validée** à l'essai : « c'est bon », commitée) : Caserne sur **R** (pas de page militaire ; pages par palier plus tard), grille A Lancier / Z Homme d'armes, infobulle « Produit : … », textes fr / en / ja ; soldats : ralliement ou clic droit sur une source ou un chantier = marche ; vitesses de la fiche (Paysan 1,41, Lancier 1,37 = 583 cm/s, Homme d'armes 1,23 = 523 cm/s ; règle × 1,098214) ; partie `build barracks` de `quick_check`, `quick_check` complet PASS (347 s), tests 69/69 ; **reste : partie 2 clients de la Caserne** (à faire en J7a.6) |
| J7c.1 Archer | **fait** (2026-10-20, mycrusader-37, worktree `HeroesOfLiberty-Elrond`, branche `j7c-archer`, b739547 ; `-Full` PASS 593 s ; `main` avancé à 4dc01ad) : détails de l'Archer tranchés (A : tout garder, voir journal) ; **(a) faite** (sous-agent, 10 min, 120k tokens ; non commitée) : `SKM_Archer`, `TP_Archer`, `A_Archer_Idle` / `Walk` / `Attack` / `Fall` fabriqués, relectures PASS, flèche cachée par échelle 0,001 de `ik_hand_gun` (images 25-35 du tir, toujours au repos, en marche, en chute) ; **(b) faite** (ligne `Archer` : 584 cm/s, 70 PV, 30 nourriture + 50 bois, 15 s ; Caserne **E** ; textes fr / en / ja) ; **(c) faite** : flèche cachée vue en jeu (captures), `AnimReport` (repos, marche, tir), `build barracks` produit un Archer, `-Parts build` PASS (270 s), rapide PASS, scénario `archers` ; `j7c-archer` sur `main` 02a8963 ; **essai validé** (« tout est bon » ; retour 8 pour plus tard : hit and run impossible, geste de tir trop long) |
| J7c.2 Tir | **en pause** (2026-10-20, mycrusader-cc, `HeroesOfLiberty-Elrond`, branche `j7c-shot`) : dégât différé et flèches en solo (4167f03), rangs d'un seul type et places par profondeur (2e3754b), validés à l'essai ; flèches chez le client réseau (b2) commencées, non commitées ; reprise le 2026-10-21 à 9 h (fin du journal) |

## Prochaine étape exacte

**J2 close** (2026-10-14). Suite, dans l'ordre de D252 :
1. **M2.5 close** (2026-10-14) : D268 à D275 au GDD, essais de l'utilisateur sur `Plain` et S5 validés. La partie
   « net » de `quick_check.ps1` reste la vérification à 2 clients de fin d'étape (D216) ; pour un essai sur S5, passer
   `-Seconds 60` à `net_match.ps1` (10 minutes jugées trop longues).
2. ~~S1 et S2 au repos, nouvelle référence sans ombres ni post-traitement~~ : **fait** (2026-10-14, nouvelle référence
   sur 5.8.3 dans `ImplementationInProgress.md`).
3. **Plan de J3 en cours** (en parallèle de M2.5, autre session ; décisions à partir de **D280**, D274 à D279 laissées
   à M2.5) : plan fait (D280 à D284) ; **J3.1 à J3.4 faites** ; **J3 close le 2026-10-14** (D280 à D284 au GDD). Suivant dans l'ordre de D252 : **plan de J4** (ordres de déplacement et chemins, M3), à ouvrir dans une nouvelle séance ; décisions à partir de **D285**.
4. **Plan de J4 en cours** (ouvert le 2026-10-14) : questions une par une (tableau « Plan de J4 » ci-dessous), décisions
   à partir de **D285** (D276 à D279 inutilisées). **Plan fait** (D285 à D289, sous-étapes J4.1 à J4.8 dans « Plan de
   J4 »). Reprise : la première sous-étape qui n'est pas « fait » dans le tableau « Où on en est ».
5. **Plan de J5 fait** (2026-10-14, en parallèle de la fin de J4, autre session) : D295 à D299 (D290 à D294 laissées à
   J4) ; J5 coupée en **trois parties jouables** (J5a le bois, J5b les 4 ressources et le HUD, J5c la vie de la
   récolte), chacune avec l'essai de l'utilisateur. **Aucun code avant la clôture de J4** ; reprise ensuite à J5a.1.
6. **J4 close le 2026-10-15** (D285 à D294, D300 à D302 au GDD). Suite : **J5a.1** (première sous-étape de J5a, le
   bois ; voir « Plan de J5 »).
7. **J5a.1 à J5a.4 faites** (2026-10-15). Suite : **J5a.5** (vérification : `quick_check` solo — ralliement sur un
   arbre, stock qui ne monte qu'au dépôt, arbre voisin, inactif — et à 2 clients — récolte vue, arbre épuisé disparu chez
   les deux, stock de l'autre jamais reçu ; S1 / S2 / S7 courts **au repos** ; **essai de l'utilisateur**). À faire
   valider à l'essai : la disposition de S7 (camps provisoires), l'abandon pour l'instant de la réutilisation des chemins
   (journal de J5a.3), la forme des arbres ; marge de S5 devenue mince (31,3 Ko/s machine chargée).
8. **J5a.5 commencée** (2026-10-15) : essais de l'utilisateur → D303, D304, validés ; solo vérifié à la main, partie à
   2 clients de la récolte dans `quick_check` faite ; essai final validé (arbres, S7) ; D305 faite ; reprise au point
   « Reprise » du dernier journal (S1 / S2 / S7 au repos, S5, clôture de J5a).
9. **J5a close le 2026-10-15** (D295 à D299, D303 à D305 au GDD). **Mesures reportées** (choix de l'utilisateur, machine
   chargée) : S1 / S2 / S7 courts au repos (ProjectShiva fermé), serveur recuit puis S5 court ; à faire à la première
   occasion au repos, au plus tard à la vérification de J5b. Suite : **J5b.1** (baies et filons ; voir « Plan de J5 »).
10. **J5b.1 faite et validée** (2026-10-15, avec D306 ; **ne rien committer avant la validation de l'utilisateur**). Suite : **J5b.2** (HUD : sélection d'une source, bloc d'un groupe, comptes par ressource,
   inactifs, touche « . », textes fr / en / ja ; voir « Plan de J5 »). **S1 / S2 / S7 courts au repos faits** (2026-10-15,
   tous PASS : S1 2,23 ms, S2 2,78 ms, S7 2,91 ms) ; **S5 court au repos fait** (serveur recuit, PASS à 31,08 Ko/s,
   marge mince). Plus aucune mesure reportée.
11. **Plan de J6 (construction) fait** (2026-10-15, en parallèle de J5b.2, autre session) : D310 à D314 (D307 à D309
   laissées à J5) ; deux parties jouables **J6a** (maison et chantier, **correctif du navmesh du centre principal en
   premier**, J6a.1) et **J6b** (camps, S7 sur des camps bâtis) ; voir « Plan de J6 ». **Aucun code avant la clôture de
   J5** ; reprise ensuite à J6a.1.
12. **J5b.2 faite** (2026-10-15, avec D307, D308 et D309 ; **non commitée** : essai de l'utilisateur attendu). Suite :
   **3e essai de l'utilisateur** (D309 : Paysans en aller-retour qui traversent tout, inactif devant une source pleine ;
   HUD déjà jugé « super », détours de D308 jugés moindres), puis **commit** de J5b.2 + D307 à D309, puis **S2 et S7
   courts au repos** (D308 et D309 changent le suivi des chemins et les corps ; machine chargée, A/B seulement),
   puis **J5b.3** (vérification : `quick_check` solo — baies au ralliement, bois vidé → inactifs et « . » — et 2 clients ;
   S5 et S7 courts, l'activité répliquée ayant changé de sens).
13. **J5b.2 validée et commitée** (2026-10-16, `039b6ca`, avec D307 à D309, D309 précisée : massifs, places à 100 cm
   vers le dépôt, points d'attente). Suite : **J5b.3** (voir « Préparation de J5b.3 » dans le journal du 2026-10-16), puis J5c.
   ~~Question en suspens : inactifs en trop à 175~~ → **tranchée le 2026-10-16** (D309 précisée, point 6) : massif
   plein → **bosquet voisin** de la même ressource à ~20 m au plus du massif, sinon **rangées d'attente sans limite**
   devant la source visée. Faite, **validée** par l'utilisateur et commitée (2026-10-16). Suite : **J5b.3**.
   **J5b.3, points 1 et 2 faits, validés, commités** (2026-10-16, a067c11) : parties « gather » solo et 2 clients de
   `quick_check` (PASS en 77-78 s, tests 39/39), verbe `waitidle`, correctif des inactifs empilés au centre (`Stop` →
   `bSettling`). **Point 3 fait** (au repos) : S1 2,18 / S2 2,74 / S7 2,16 ms, S5 31,2 Ko/s (erreur 21,6 cm), tous PASS ;
   les 632 bois/s de S7 étaient un effet de fenêtre (vagues d'allers-retours ; régime 1 201 bois/s).
   **J5b close** (2026-10-16 : D307 à D309 au GDD, paragraphe « Parcours J5b »). Suite : **J5c** (la vie de la
   récolte), en commençant par **J5c.1** (voir « Plan de J5 »). **Ordre choisi par l'utilisateur (2026-10-16)** :
   J5c.1 (animations) menée par la session `mycrusader-23` ; cette session fait **J5c.3** (« +10 »), essai, commit, puis
   **J6a** (à partir de J6a.1) tant que les animations ne sont pas prêtes ; J5c.2 et J5c.4 après les animations. **Source des animations
   tranchée** (2026-10-16, option A, la recommandation) : **faites par Claude**, boucles courtes écrites pose par pose
   en Python sur le mannequin UE4, sur place (hache, pioche, cueillette, marche chargée, pose au dépôt) ; remplaçables
   plus tard (Mixamo, Fab) sans toucher au code, la table des types les nommant. Écartés : B Mixamo (compte Adobe et
   reciblage), C pack Fab (achat). Numéro donné à la clôture de J5c : **D318**. En sous-agents (règle ci-dessous).
   La fabrication des animations (tâche récurrente) est tenue dans **`AnimationsInProgress.md`** (méthode, catalogue,
   journal) ; J5c.1 y renvoie. **J5c.1a faite** (2026-10-16) : 6 séquences fabriquées et relues (hache, pioche,
   cueillette, marches chargées nourriture / bois / pierre-or), jugées sur les aperçus ; pas encore branchées.
   **Worktree** (2026-10-16, demande de l'utilisateur) : le code de J5c.1 b à f se fait dans
   `C:\Users\baudo\Documents\HeroesOfLiberty-J5c`, branche **`j5c-anims`** (depuis cd39c6c), fusionnée dans `main` après
   validation de l'utilisateur ; les journaux restent dans la copie principale. Raisons : le jeu d'essai de
   l'utilisateur ne bloque plus la compilation, pas de code à moitié écrit dans les compilations de l'autre session.
   Fichiers binaires pris par J5c.1 (à ne pas régénérer sur `main`) : table des types d'unités, fournisseurs
   `TP_Mannequin_Classic*`, `Content/Units/Mannequin/Animations`.
   **J5c.1 b, c, d faites, essai de l'utilisateur validé** (« c'est bon », 2026-10-16), **commitées sur `j5c-anims`**
   (a9e4269). Reste : S1 / S2 / S7 courts dans le worktree (rendu touché), fusion dans `main` après le commit de J6a.2
   par l'autre session (fichiers communs non commités là-bas : `HOLPlayerController.cpp`, `quick_check.ps1`), S5 court
   depuis `main` (serveur recuit), puis J5c.2 (outils et charge visibles). **Fusionnée dans `main`** (2026-10-16,
   avance rapide jusqu'à 8d18407, après fusion de J6a.2 dans la branche : compilation, tests 47/47, `quick_check` PASS ;
   A/B S7 : coût faible, +0,15 ms par image sur le fil de jeu). Restent les mesures au repos et S5.
   **J5c.2 faite** (2026-10-16, worktree, détail dans `AnimationsInProgress.md`) : hache et pioche dans les mains,
   panier de baies, fagot, hotte de pierre ou d'or, repos chargés ; essai de l'utilisateur validé (« très bien ») ;
   **fusionnée dans `main`** (fad246d ; tests 54/54, `quick_check` PASS ; A/B S7 : CPU inchangé, GPU +1 ms par image).
   ~~Restent pour J5c : S1 / S2 / S7 au repos, S5 court, J5c.4 (clôture) ; animation du bâtisseur pour J6a.4.~~ →
   mesures au repos faites par J6a.6 (tous PASS), bâtisseur fait en J6a.4 ; **J5c close** (2026-10-16 : D318 à D323
   au GDD, paragraphe « Parcours J5c ») : **J5 close**.
   **J5c.3 faite** (98858d0) ; **J6a.1 faite** (2026-10-16, navmesh des bâtiments, validée ; S1 / S2 / S7 au repos à
   refaire avant la clôture de J6a). **J6a.2 faite** (2026-10-16, validée : table, règles de construction, case de
   3 m, forum 12 m, maison et camps 6 m, camps 100 bois). **J6a.3 faite** (2026-10-16, validée : pose, fondation,
   annulation). **J6a.4 faite** (2026-10-16, validée : chantier, bâtisseurs, fondation fantôme, file Maj avec
   récolte, ralliement sur un chantier, chantier voisin ; S2 / S7 au repos à refaire). **J6a.5 faite** (2026-10-16, validée : grille du Paysan,
   infobulles, messages, bloc du chantier, F1 / F2 / F3, groupes et Tab). **J6a.6 faite, J6a close** (2026-10-16 : mesures au repos
   PASS, essai complet validé). Suite : **J6b.1** (camps ; voir « Plan de J6 »). **J6b.1 faite** (2026-10-16,
   sous-agent, rien de commité) : camp fini → récolte, ralliement sur un camp ; tests 63/63, `quick_check` PASS (202 s,
   partie `build camp`) ; essai de l'utilisateur **validé** (2026-10-17 : camp avant le chantier voisin, portée de 25 m, ralliement sur un
   camp fini, gardés tels quels), commitée (journal « J6b.1 : les camps »). Suite :
   **J6b.2** (chemins coupés par une emprise). **J6b.2 faite** (2026-10-17, sous-agent, rien de commité ; journal
   « J6b.2 : chemins coupés par une emprise ») : tests 64/64, `quick_check` PASS (259 s, partie `build cut`), S7 2,289 /
   S2 2,758 ms PASS ; essai de l'utilisateur **validé** (« super »), commitée ; à l'essai, **nouvelle règle** demandée :
   un bâtiment occupe ses cases mais n'en remplit que ~90-95 %, un Paysan passe entre deux bâtiments voisins, pas deux
   (réglage en cours de discussion, avant J6b.3). Suite : **J6b.3** (vérification, S7 avec camps bâtis).
   **D324 choisie** (marge fixe) ; **J6b.2b faite** (2026-10-17, sous-agent, rien de commité ; journal « J6b.2b ») : marge
   **105 cm** (écart 2,1 m), à la file dans l'écart, chemin par l'écart ; tests 65/65, `quick_check` PASS (306 s). Suite :
   essai de l'utilisateur **validé** après deux retouches (forme visible à 90 %, corps à 115 cm), `quick_check` complet
   PASS (364 s), **commitée**. Suite : **S1 / S2 / S7 au repos** (S2 en échec sous forte charge), puis **J6b.3**.
   Petit reste : le commentaire du test `Gather.Spots` décrit encore l'écart de 70 cm (le test passe l'écart
   explicitement : il reste juste, à reformuler à l'occasion). Hors de ce chantier, non commités : `GDD.md` (review du
   GDD, autre session) et les lignes du plan de J6 dans ce fichier (autre session).
   **J6b.3 faite, validée et commitée** (2026-10-18 ; journal « J6b.3 : vérification ») : D325 (apparence des bâtiments
   hors du pas), mesures de fin de J6 toutes PASS par `chapter_measures.ps1`, essai « tout est bon ».
   **J6b.4 faite, J6 close** (2026-10-18, journal « J6b.4 : clôture ») : D310 à D314, D324, D325 au GDD.
   **Plan de J7 relu avec l'utilisateur** (2026-10-18 : J7a validée, forme des rangs précisée, voir J7-Q3). S2 : sans
   réponse de l'utilisateur, mesuré à la fin de J7 (règle des mesures). **J7a.1 faite** (2026-10-18). **Pages de la grille du Paysan** (choix de l'utilisateur, 2026-10-18 ; *précise D313*,
   qui prévoyait une seconde page pour les bâtiments militaires) : **pas de page militaire** ; les pages viendront **par
   palier, comme les âges d'AoE4** ; pour l'instant la Caserne prend une case libre de la page actuelle : **R**
   (`BuildGridSlot` 3). **J7a.2 faite et validée** (2026-10-18). **J7a.3 faite et validée** (8280f6a). **J7a.4 faite et validée** (2026-10-18, cb8227d ; journal « J7a.4 : la
   formation » et les essais 1 à 5) : rangs, marche en bloc façon AoE4, flèche du clic droit (Ctrl : largeur), Maj en
   file, retour à sa place, couleur du joueur. Orientation au front chez le client reportée à J7b (choix B).
   **J7a.5 faite et validée** (groupes de contrôle, Ctrl + chiffre retiré). **J7a.6 faite, J7a close** (2be3be1 ;
   armée à 2 clients, correctif de `gather`). **J7b.1 commitée** (ecc260e). **J7b.2 à J7b.5 (1re partie) faites dans le
   worktree `HeroesOfLiberty-J5c`, non commitées** (journal « J7b.2 » à « J7b.5 »).
   **➡ Reprise au 2026-10-21 (9 h) : J7c.2 en pause** dans `HeroesOfLiberty-Elrond`, branche `j7c-shot` : voir le point « Séance
   mise en pause » de l'entrée « 2026-10-20 — J7c.2 : le tir » (fin du journal). Avant :
   **J7c.1 faite et validée** (journal « J7c.1 : l'Archer », `main` 4dc01ad) ; suite :
   **J7c.2** (tir : dégât différé selon la distance, flèches déduites chez le client, mort par flèche ; J7c-Q1 à Q6 dans
   « Plan de J7c »), à penser avec le retour d'essai 8 (hit and run) ; corps gardés 30 s : mycrusader-a6 (`HeroesOfLiberty-J5c`).
   **Reprise au 2026-10-20 (soir) : J7b close** (journal « J7b.6 : clôture de J7b ») ; suite : **J7c** (plan de J7) ;
   7 retours d'essai en attente (journal du 2026-10-20) ; GDD et mesures à la fin de J7. Ce qui suit est l'historique.
   **Prochaine étape exacte (nouvelle séance)** : J7b.2 à J7b.5 commitées (ce6a4d2), D327 (ee63932), gestes polis,
   préchargement et ralliement sur fantôme (258c5d0). **Coup du Paysan validé en aperçu** (2026-10-19) : coup de
   pointe au couteau, posture de peur (`PEASANT_ATTACK` dans `PROPOSED`, pièce `make_knife()` id 8), commité en aperçu
   seulement (f7b0b27). Suite : (1) **fabriquer** (sous-agent, un livrable) : maillage du Paysan refait avec le couteau
   (`make_peasant_mesh.py`), geste passé dans `GESTURES` et fabriqué, colonne `AttackProp` (8 pour le Paysan) sur le
   modèle de `BuildProp` (`HOLUnitTypes.h`, `HOLUnitSubsystem.cpp`, `HOLUnitAnimation.cpp`, tests), table refaite,
   `quick_check` rapide + `-Parts combat` ; (2) essai de l'utilisateur, `-Full`, commit ; (3) option : décalage des
   gestes par unité ; (4) J7b.6. **(1) et (2) faits (2026-10-20, 33f8d63)**. Suite proposée
   à l'utilisateur : les lignes ci-dessous, puis J7b.6 (réponse attendue sur l'ordre).
   ~~**À faire (demande de l'utilisateur, 2026-10-20)** : lignes entre les actions en file (Maj), rouge attaque, blanc
   construction, jaune déplacement~~ → **D328 faite, validée, commitée (c2b44e4)** ; restent la ligne blanche du
   ralliement à la naissance et les contrôles des lignes (journal du 2026-10-20) ; **5 retours d'essai à faire plus
   tard** (journal : groupe qui attend le retardataire, A clic validé trop tôt, pose sur une case occupée, pose
   unique sauf Maj, marcheurs bloqués par des corps). **Et** : grille du Paysan façon AoE4 (capture de l'utilisateur, journal du 2026-10-20) : ordres (Q attaque-déplacement, S arrêt, W construire…) **à gauche**, panneau de construction **à droite** avec un onglet par âge (I à IV) choisi par **A Z E R** ; donc attaque-déplacement **revient de A à Q** (retour de l'essai de J7b) pour ne pas entrer en conflit avec l'onglet I. Sous-agents : un livrable chacun, 15 min / 300k tokens, minuteur à chaque lancement.
   **Corps au sol 30 s (décision de l'utilisateur, 2026-10-20, autre session ; précise D86 et J7-Q5 « pas de
   cadavre »)** : le serveur détruit toujours l'unité à 1 s (`DyingSteps`) ; **chaque client garde le corps** : il finit
   de jouer la chute (sa durée n'est donc plus bornée à 1 s), garde la dernière pose **30 s**, puis le fait s'enfoncer
   dans le sol (~1 s). 0 bit, rien au serveur ni au serveur dédié ; un maillage instancié de corps chez le client (coût
   à mesurer en fin de J7, S3). Durée fixe, sans plafond de nombre (choix de l'utilisateur ; « 15 s au minimum », 30
   retenu). Écartés : **serveur qui garde l'unité 15 s** (`DyingSteps` 300 : places d'unité, S3, S5) ; **cadavres objets
   de jeu** (prépare D86 / Légions, à faire avec elles ; ce corps-ci pourra s'y brancher). Les chutes sont refaites en
   aperçu (`AnimationsInProgress.md`, catalogue « Arc… ») : la dernière image est la pose tenue 30 s.
   **Correctifs de la revue de code de J7b, à faire avant J7b.6** (revue en lecture seule, autre session, 2026-10-20 ;
   diff 48affca..HEAD ; demande de l'utilisateur de les écrire ici) :
   - **Confirmés, à corriger** — **1 à 3 faits, validés à l'essai, commités (c2b44e4, 2026-10-20)** :
     1. Unité figée après avoir tué elle-même sa cible ordonnée : le coup mortel (`HOLCombatProcessor.cpp` ~l. 404)
        efface `Target` et `bAttacking` mais pas `bOrderedTarget` (remis à zéro seulement dans la branche « cible
        perdue », qui exige `Target.IsSet()`). Effets : `bMaySeek` faux (plus de recherche ni de riposte) et
        `StartQueuedMoves` (`HOLUnitOrderProcessor.cpp` ~l. 1861) ne lance jamais la file Maj. Masqué dans
        `quick_check` par S (`Disengage`). Correctif : `Combat.bOrderedTarget = false;` au coup mortel + un test du
        processeur (clic droit sur A, Maj + clic droit sur B : B est attaqué après la mort de A).
     2. Ligne des PV d'un bâtiment restée affichée sur un groupe d'unités : `RefreshSelection` (bâtiment) vide
        `ShownHealth` et montre la ligne (`HOLGameScreen.cpp` ~l. 562) ; `RefreshUnitHealth(nullptr)` (~l. 834) calcule
        "" = `ShownHealth` et sort avant `SetShown(SelectionHealthRow, false)`. Idem pour une unité seule de type
        inconnu (`MaxHealth == 0`).
     3. Grille fausse après une mort dans une sélection mixte : `PurgeSelection` (`HOLPlayerController.cpp` ~l. 2430)
        appelle `SetUnitSelection` (qui remet `ActiveUnitType` à `NAME_None` et diffuse), puis remet le type sans
        rediffuser : sous-groupe « Paysan », grille « Tous » affichée (Attaque), A pose une Maison. Correctif : remettre
        le type avant la diffusion.
   - **À soumettre à l'utilisateur** :
     4. Demi-tour sec à la mort en solo / hôte : le regard vers la cible (`bFacingSource`, posé tant que `bStrike`,
        `HOLInterpolatedInstancesProcessor.cpp` ~l. 397-424) tombe au début de la chute, l'unité revient au lacet de la
        simulation (`FrontYaw` de sa place) : jusqu'à 180°. Un client garde `Display.Yaw` : les deux vues divergent.
        (Le Paysan sans geste ne se tournait pas vers sa cible en solo : à revoir depuis le couteau, 33f8d63.)
     5. Client : unité un peu blessée affichée pleine (99/100 → palier 15 → « 100/100 », barre pleine, mais barre
        montrée car `bFullHealth` faux). Proposition : plafonner à max − 1 quand « PV pleins » est faux.
   - **Plus tard** : `LastAttacker` n'expire jamais (riposte sans limite de distance : poursuite à travers la carte ;
     borner à ~2 × rayon d'agression ou ~5 s ; **choix de l'utilisateur, 2026-10-21 : l'oublier quand il disparaît dans
     le brouillard de guerre, donc attendre M7** ; ni durée ni vue provisoire d'ici là ; pas de ping-pong entre deux
     paires d'Archers de part et d'autre d'un Homme d'armes : tous le visent, il prend le plus proche et s'en
     rapproche) ; mort hors mêlée (`HOLCombatProcessor.cpp` ~l. 186 : ni `StopWalk`, ni
     `Disengage`, ni activité `Dying`) **à compléter en J7c.2** (flèches) ; `hol.Combat.Enabled 0` saute aussi la
     destruction des mourantes (**corrigé le 2026-10-21**, branche `combat-dying-off` de `HeroesOfLiberty-J5c` : combat
     coupé, personne ne combat et les flèches en vol attendent, mais une unité à 0 PV tombe et part 1 s après ;
     reproduit par la partie `smoke` de `quick_check`, qui coupait le combat juste après la mort du Paysan : « 1 left
     dying » avant, 0 après ; `-Parts tests,smoke,combat` PASS, 211 s) ; place 4095 = « cible invalide » mais vraie place au-delà de 4 096 entrées
     (**corrigé le 2026-10-21**, branche `review-fixes` de `HeroesOfLiberty-Galadriel` : 4 095 vraies places, 0 à 4 094, en
     65 blocs ; le serveur ramenait déjà toute place ≥ 4 095 à « inconnue », mais le client lisait 4 095 comme la place
     du 66e bloc ; `FHOLNetUnitState::HasTargetSlot`, cas ajoutés à `Net.CombatSendRule`) ;
     `AttackerCount` de la victime d'une unité tuée trop haut d'un pour la fin du pas (sans effet visible ; **corrigé le
     2026-10-21**, même branche : une unité tuée au pas, par coup ou par flèche, décompte aussitôt sa propre cible).
   - **Relecture de 9debb86 (scénarios de test)**, par la session mycrusader-35 (2026-10-20, rien modifié) :
     1. *À corriger* : `HOLGameHUD.cpp` `BeginPlay` (~l. 84) pose `HOLTestHooks::OnScenarioChanged()` (lambda sur
        `WeakPanel`) mais `EndPlay` ne le remet jamais à vide : après une partie, `NotifyScenarioChanged` appelle une
        lambda au panneau mort et le message de secours (`AddOnScreenDebugMessage`) n'apparaît jamais (retour au menu,
        `hol.Scenario <nom>`, échec avant la carte → erreur seulement dans le journal). Correctif : dans
        `AHOLGameHUD::EndPlay`, avant de retirer `ScenarioPanel`, `HOLTestHooks::OnScenarioChanged() = nullptr;` (même
        `#if` que `BeginPlay`).
     2. *Faible* : `HOLTestSequence.cpp` `~FTestSequence` (~l. 135) : `Sequences` est une `TMap` globale ; jeu fermé
        pendant une séquence → `Finish` appelle `GLog->RemoveOutputDevice` / `FTSTicker::RemoveTicker` pendant la
        destruction statique (ordre indéfini, surtout en monolithique). Piste si plantage à la fermeture : vider sur
        `FCoreDelegates::OnPreExit`, ou rien dans le destructeur si `IsEngineExitRequested()`. **Corrigé le
        2026-10-21** (branche `review-fixes`) : les séquences en cours s'arrêtent sur `OnEnginePreExit` (GLog encore
        là), le destructeur ne touche plus à rien à la sortie ; testé sur un indicateur « en cours » et non sur
        `Ticker`, que le moteur vide juste avant `OnEnginePreExit`. Vérifié : `hol.Scenario combat_off_dying` puis
        `quit` à 3 s, sans fenêtre → « the run is stopped at step 2 », sortie propre (code 0).
        Avec la même branche : commentaire du test `Gather.Spots` reformulé (plus de 70 cm présenté comme l'écart en
        vigueur). `quick_check -Parts tests,smoke,combat,net` PASS (228 s, 87/87), puis sur d3f1408 PASS (338 s). Le
        `-Full` y était coupé à 660 s (`GameLimitSeconds`) : depuis le retardataire (d3f1408, ~105 s de plus), la
        séquence solo dure ~660 s, les rapports des corps, flèches et ordres manquaient (c'était aussi « le jeu arrêté
        de l'extérieur » du `-Full` de d3f1408) ; limite portée à 780 s. Nouvelle règle (choix de l'utilisateur) : au
        commit, seulement les parties touchées ; `-Full` aux clôtures de chapitre.
     Le couteau du Paysan (33f8d63) : aucune remarque.
   Mesures au repos (fermer Opera) : S2 à la fin de J7.
   1. ⚠️ **S2 au seuil** (2,816 / 3,024 / 3,145 ms, moyenne 2,995) : gardé pour la fin de J7 (choix de l'utilisateur) ;
      J7a.4 a touché le mouvement (marche en bloc, `AssignPlaces`) : à surveiller aux mesures.
   2. Non commité, **hors de ce chantier** : `AnimationsInProgress.md` (session mycrusader-32 : pièces allégées au LOD 3,
      essai de l'utilisateur en attente, worktree `HeroesOfLiberty-J5c`) ; `Build/` (non suivi).

**Règle de cohabitation avec le stress test** : seule une carte qui a des emplacements de départ reçoit la caméra RTS ;
une carte sans emplacement (`StressTest`, `BattleMap`) garde le pion par défaut, donc le stress test et son fichier
(chantier M2) ne changent pas.

**Règle de tenue** (permanente) : mettre à jour ce fichier à **chaque étape** — tableau ci-dessus, prochaine étape
exacte, entrée de journal — avant de passer à la suivante.

**Règle des sous-agents** (permanente, demande de l'utilisateur du 2026-10-15) : le chantier se mène **en sous-agents
autant que possible** (code, essais scriptés, `quick_check`, mesures S1 / S2 / S5 / S7, lectures larges du code ou du
moteur) ; la session principale garde le fil : elle lit ce fichier, confie chaque tâche à un sous-agent avec un
énoncé complet (contexte, fichiers, règles de compilation et de test, ce qu'il doit rapporter), relaie les résultats,
pose les questions à l'utilisateur et tient ce fichier. Restent dans la session principale : les questions de plan
(une à la fois), le lancement de la session d'essai de l'utilisateur et les commits après sa validation. Deux
sous-agents ne compilent ni ne lancent le jeu en même temps (DLL verrouillées).

**Règle des boucles de 15 minutes** (permanente, demande de l'utilisateur du 2026-10-17) : on travaille **en boucles de
15 minutes au plus** ; une tâche (sous-agent compris) qui dure plus longtemps signale que quelque chose cloche. Chaque
sous-agent reçoit un but précis tenant dans une boucle et rend la main sous ~15 minutes avec ce qui est fait et ce qui
reste ; `quick_check` (~4-5 min) et les mesures S1 / S2 / S7 comptent dans la boucle, ou font une boucle à part.

**Règle des mesures** (permanente, demande de l'utilisateur du 2026-10-17, confirmée et étendue le 2026-10-18) : **toutes
les mesures de performance** (S1 / S2 / S3 / S4 / S5 / S7, au repos ou non, courtes ou non) se font **une seule fois, à
la fin de chaque chapitre** (fin de J6, fin de J7…), juste avant sa clôture, **par `Tools/Tests/chapter_measures.ps1`**
(choix de l'utilisateur, 2026-10-18), lancé par la session principale en tâche de fond, sans sous-agent : chaque stress
test **deux fois, PASS les deux** ; si elles ne s'accordent pas, une 3e course et la moyenne des trois décide ; S5 une
fois, le serveur dédié n'étant recuit que si `Source`, `Config` ou `Content` ont changé depuis sa cuisson ; **jamais pendant le chapitre**, ni à la
vérification d'une partie (J7a, J7b…) ni à une sous-étape. En cours de route : `quick_check` seulement. Un seuil
dépassé à la fin du chapitre se corrige avant la clôture. Les plans de chapitre qui prévoient une mesure en cours de
route sont lus à la lumière de cette règle (la mesure passe à la fin du chapitre).

## Commandes utiles

Compilation : `powershell -ExecutionPolicy Bypass -File Tools\build.ps1 [-Root ..\HeroesOfLiberty-Elrond]` (file
d'attente automatique, `BUSY` si Unreal tourne dans la même copie ; journal dans `<copie>\Saved\Logs\build.log`).
Traductions : voir `FrontEndInProgress.md` (« Commandes utiles »). Toujours depuis **PowerShell** (Git Bash réécrit
`/Game/...`).

**Vérification de tous les jours (D267) : `quick_check.ps1`**, ~30 s : tests d'automatisation en parallèle d'un seul
lancement du jeu (menu fr / en / ja avec bascule de langue en direct, liste déroulante, partie à 2 puis 8 joueurs avec
les contrôles de caméra de D257). Chaque étape part dès que la précédente est finie (`hol.Test.Run`), chaque capture est
vérifiée (écran affiché au moment de la prendre). Une ligne PASS / FAIL par partie. Les stress tests (S1 à S7) ne se
lancent qu'à la fin d'un chapitre (règle des mesures ci-dessus).

```powershell
# Vérification rapide (D267) ; -Parts tests,menu,camera pour n'en lancer qu'une partie
powershell -ExecutionPolicy Bypass -File Tools\Tests\quick_check.ps1

# Lancer une session d'essai (deuxième écran, réglages de test) ; Jouer → Lancer
Start-Process "C:\UE\UE_5.8_AS_3\Engine\Binaries\Win64\UnrealEditor.exe" -ArgumentList '"C:\Users\baudo\Documents\MyCrusader\HeroesOfLiberty.uproject" -game -windowed -ResX=1600 -ResY=900 -WinX=3940 -WinY=100 -saveddirsuffix=Tests -nosplash -LiveCoding=0'

# Stress test court sur le deuxième écran (S1 puis S2), rapport dans Saved_Tests\Logs\HeroesOfLiberty.log
& "C:\UE\UE_5.8_AS_3\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\baudo\Documents\MyCrusader\HeroesOfLiberty.uproject" /Game/Maps/StressTest `
  -game -windowed -ResX=1920 -ResY=1080 -WinX=3840 -WinY=0 -saveddirsuffix=Tests -nosplash `
  -ExecCmds="r.VSync 0,t.MaxFPS 0,scalability 2,hol.Units.SpawnBudget -1,hol.StressTest S1 2000 3 quit"

# Refaire les ressources (éditeur complet pour la carte, commandlet pour les touches)
#   UnrealEditor.exe <uproject> -nosplash -unattended -ExecCmds="py <projet>/Tools/Editor/create_match_map.py"   (supprimer Content/Maps/Plain.umap avant)
#   UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script=<projet>\Tools\Editor\make_game_input.py -unattended -nop4 -nosplash
```

Commandes de jeu pour les essais par script : `hol.Camera.Report [délai]`, `hol.Camera.TestCursor X Y` (curseur simulé
du défilement au bord, `off` = vrai curseur), `hol.Delay <secondes> <commande>`, `hol.UI.Key <touche> [délai] [durée]`
(`MouseScrollUp` / `MouseScrollDown` = un cran de molette), `hol.UI.NewGame.Launch <fichier JSON|-> [délai]`,
`hol.UI.Focus <nom du widget> [délai]` (focus sur un widget nommé de l'écran de menu ouvert, puis `hol.UI.Key`),
`hol.UI.Language <fr|en|ja>`, `hol.Test.Run <fichier>` (enchaîneur d'étapes : `frames`, `wait`, `waitlog`, `waitmap`,
`waitui`, `capture`, `mark`, commandes ; voir `Source/HeroesOfLiberty/Dev/HOLTestSequence.cpp`).

## Plan de J1 (à valider question par question)

| # | Question | État |
|---|---|---|
| Q1 | **Périmètre** de J1. | **tranché (D253)** : caméra RTS, emplacement de départ, et réglages vitesse de caméra et défilement au bord dégrisés ; remappage plus tard, chaque étape ajoutant ses actions |
| Q2 | **Comportement de la caméra**. | **tranché (D254)** : façon AoE4, sans rotation, plongée fixe ~55°, hauteur 30-40 m par défaut, zoom ~15 à ~70 m |
| Q3 | **Fabrication** de la caméra. | **tranché (D255)** : caméra locale sans pion, pilotée par `AHOLPlayerController` ; actions dans `IMC_Game` par script ; réglages dans `UHOLGameUserSettings` ; le stress test place la caméra par une fonction dédiée |
| Q4 | **Emplacements de départ** et limites de la carte. | **tranché (D256)** : acteurs posés (emplacement dérivé de `PlayerStart` avec son numéro, limites de la carte) sur une **nouvelle carte** faite par script |
| Q5 | **Vérification**. | **tranché (D257)** : tests d'automatisation, bout en bout par script, S1 et S2 courts, essai de l'utilisateur |

## Plan de J2 (à valider question par question)

| # | Question | État |
|---|---|---|
| Q1 | **Périmètre** de J2 : qui reçoit un centre, quelle sélection, quel retour à l'écran. | **tranché (D259)** : un centre par emplacement occupé, à sa couleur ; clic gauche sur un bâtiment, clic dans le vide ou Échap pour désélectionner ; cercle au sol et surlignage au survol ; pas de panneau |
| Q2 | **Fabrication** du centre principal : acteur répliqué ou entité Mass, où vivent ses données (points de vie, propriétaire). | **tranché (D260)** : classe C++ commune répliquée, créée par le serveur ; propriétaire = numéro du joueur ; table des types de bâtiment depuis un CSV ; GAS en M5 |
| Q3 | **Apparence** provisoire et **couleur du joueur**. | **tranché (D261)** : bloc et tour en volumes du moteur (~15 × 15 m, ~10 m), toit ou bandeau à la couleur du joueur, valeurs en table |
| Q4 | **Sélection** : où vit l'état de sélection (client seul ou serveur), comment on vise (rayon sous le curseur), retour visuel. | **tranché (D262)** : client seul, rayon sur un canal de sélection, cercle au sol, survol par teinte du matériau |
| Q5 | **Vérification**. | **tranché (D264)** : tests d'automatisation, bout en bout à 2 et 8 joueurs avec clics simulés, essai de l'utilisateur |

## Plan de M2.5 (à valider question par question)

Repères : D216 (tranche réseau tôt, puis vérification à 2 clients à chaque étape), D217 (pas de 20 Hz, envoi à 10 ou
20 Hz selon le budget mesuré ici), D218 / D231 (≤ 32 Ko/s par client, ≤ ~256 Ko/s d'envoi du serveur à 8 clients),
D231 (serveur dédié, cible `HeroesOfLibertyServer`, simulation `Server | Standalone`), D233 (`OwnerPlayer`), D243
(configuration par `-MatchConfig=` pour un serveur sans salon), D255 (caméra jamais envoyée au serveur), D260
(bâtiments répliqués). Déjà en place : Iris activé (`net.Iris.UseIrisReplication=1`, `SetupIrisSupport`), cible
Server compilée. Seul S2 / le stress test fait naître des unités ; une partie sur `Plain` n'en a pas encore.

| # | Question | État |
|---|---|---|
| Q1 | **Périmètre** : ce qui tourne en réseau à la fin de M2.5 (partie sur `Plain`, stress test S5), comment on lance et rejoint. | **tranché (D268)** : partie à 2 clients sur `Plain` et S5, serveur dédié et clients lancés par script, `open <adresse>` ; menu inchangé |
| Q2 | **Réplication des unités** : ce qui est envoyé, et quand (2 000 unités en marche sous 32 Ko/s par client). | **tranché (D269)** : position et vitesse réduites, prolongées par le client ; envoi seulement au-delà d'un seuil d'erreur (~25 cm) ou à un changement d'action, rappel lent (~2 s) |
| Q2b | **Transport** : par quoi passent ces données (plugin MassReplication, acteur répliqué maison avec sérialiseur Iris, RPC). | **tranché (D270)** : tableau rapide maison sur un seul acteur partagé, éléments marqués selon D269, format d'élément maison ; repli : paquets maison non garantis |
| Q3 | **Côté client** : entités de présentation créées depuis les données reçues, prolongement, corrections. | **tranché (D271)** : entités Mass d'affichage chez le client, instant présent estimé, corrections lissées en ~0,2 s |
| Q4 | **Solo** : partie locale autonome ou serveur dédié local. | **tranché (D272)** : le solo reste autonome ; même code d'affichage nourri par la simulation (solo) ou par la réception (client) ; chemin réseau vérifié à 2 clients à chaque étape |
| Q5 | **Vérification** : S5, partie à 2 clients dans `quick_check.ps1`, essai de l'utilisateur. | **tranché (D273)** : tests des règles pures, S5 avec bande passante, envois et écart affiché, partie à 2 clients dans `quick_check.ps1`, essai de l'utilisateur |

## Plan de J3 (à valider question par question)

Repères : D30 (grille de commandes `A Z E R / Q S D F / W X C V`), D83 (maisons, base de population du centre), D03
(plafond 175), D201 (HUD façon AoE4), D241 / D242 / D247 (Common UI en C++, module que le serveur ne charge pas), D252
(toute action passe par la file de commandes, donc le serveur), D259 / D262 (sélection chez le client, le panneau de J3
la montre), D260 (bâtiments répliqués, table des types), D269 à D272 (unités répliquées, solo autonome). Déjà en
place : file de commandes de M1 (déplacements seuls, `UHOLCommandSubsystem`), table des unités depuis
`Tools/Data/UnitTypes.csv` (pas encore de Paysan). Aucune unité n'apparaît encore dans une partie sur `Plain`.

| # | Question | État |
|---|---|---|
| Q1 | **Périmètre** de J3. | **tranché (D280)** : ressources et population en haut, bloc de sélection, grille de commandes avec « Paysan », coût débité, file avec progression et annulation, Paysan immobile près du centre, tout par le serveur ; en plusieurs sous-étapes |
| Q2 | **Fabrication du HUD** : quels blocs de D201 maintenant, comment (Common UI, C++), place des blocs à venir. | **tranché (D281)** : écran de jeu Common UI en C++, mise en page de D201, seuls barre du haut, bloc de sélection, grille de commandes et file de production ; le HUD lit l'état, ses clics passent par la file de commandes |
| Q3 | **Production côté serveur** : ordre envoyé au serveur, vérification du coût et de la population, file de production, ce qui est répliqué (ressources, population, file). | **tranché (D282)** : appel du contrôleur → file de commandes → pas suivant ; coût débité à la mise en file, remboursé à l'annulation ; population vérifiée au début de la production ; ressources dans le `PlayerState` et file sur le bâtiment, répliquées au propriétaire |
| Q4 | **Le Paysan** : type d'unité, apparence provisoire, lieu d'apparition, valeurs provisoires (coût, temps, stock de départ). | **tranché (D283)** : ligne `Peasant`, mannequin de M2, apparition en arc devant le centre, immobile ; 50 nourriture, 20 s, 200 de chaque au départ, population de base 10, file de 5 |
| Q5 | **Vérification**, et découpage en sous-étapes. | **tranché (D284)** : tests des règles, partie solo et partie à 2 clients dans `quick_check.ps1`, essai de l'utilisateur, S1 et S2 courts à la clôture ; sous-étapes J3.1 à J3.6 ci-dessous |

**Sous-étapes de J3** (chacune compile, se vérifie et se commite seule) :

1. **J3.1 Données et règles pures** : `Peasant` dans `UnitTypes.csv` (colonnes coût en 4 ressources, temps, population) ; `BuildingTypes.csv` (population de base, longueur de file, unités produites) ; stock de départ dans les réglages ; tables refaites par leurs scripts ; règles pures de production (`Buildings/HOLProductionRules` : mise en file, débit, remboursement, avance d'un pas, attente population pleine) et leurs tests.
2. **J3.2 Économie du joueur** : ressources et population dans `AHOLPlayerState`, répliquées au seul propriétaire, remplies par le mode de jeu au début de la partie ; population comptée par le serveur (unités du joueur, par propriétaire dans `UHOLUnitSubsystem`) ; commande console d'affichage.
3. **J3.3 File de production et ordre** : file sur `AHOLBuilding` (type et pas de début, répliquée au propriétaire : propriétaire réseau = contrôleur du joueur) ; ordres « produire » et « annuler » dans la file de commandes, appels serveur du contrôleur ; traitement à chaque pas (sous-système de production) ; Paysan créé par `UHOLUnitSubsystem` à une position précise en arc devant le centre ; commandes console `hol.Prod.*` pour essayer sans HUD ; vérifié en solo et par `net_match.ps1` (le Paysan arrive chez les 2 clients).
4. **J3.4 HUD** : `AHOLGameHUD` dans le module d'interface (classe donnée au mode de jeu par chemin souple : le serveur ne la charge pas) ; écran de jeu Common UI : barre du haut, bloc de sélection, grille 3 × 4 avec ses touches (12 actions de grille dans `IMC_Game` par `make_game_input.py`, positions AZERTY de D30), file avec progression calculée chez le client (horloge du serveur de M2.5, pas de la simulation en solo) et annulation au clic ; un clic sur le HUD ne sélectionne pas à travers ; textes en fr / en / ja.
5. **J3.5 Vérification (D284)** : partie « production » dans `quick_check.ps1` (solo), partie à 2 clients une fois M2.5.6 faite, essai de l'utilisateur, S1 et S2 courts.
6. **J3.6 Clôture** : D280 à D284 au GDD (§ 21, paragraphe « Parcours J3 » au § 16.6).

## Plan de J4 (à valider question par question)

Repères : § 7.3 (sélection par clic, multiple, par cadre ; ordres de déplacement), D30 (touches), D215 (Mass pour le
suivi de chemin et l'évitement, couche RTS maison pour les ordres de groupe et formations), D216 / D252 (toute action
par la file de commandes, donc le serveur ; vérification à 2 clients en fin d'étape), D228 (ordre de M1 en cible
directe, chemins laissés à M3), D229 (S2 requalifié, que M3 doit rendre de nouveau exigeant), D230 (évitement : deux
unités qui se croisent se traversent, à instruire avant M5), D262 (sélection chez le client seul), D269 à D272
(réplication des unités), D283 (Paysan immobile, ralliement en J4). Déjà en place : file de commandes avec l'ordre
« va à cet endroit » (`UHOLCommandSubsystem::IssueMoveOrder`, cible directe, destination ramenée au navmesh), sélection
des seuls bâtiments (`AHOLPlayerController`), `Plain` plate avec navmesh, les centres principaux apparus en cours de
partie (le navmesh ne les contourne pas encore).

| # | Question | État |
|---|---|---|
| Q1 | **Périmètre** de J4 : sélection d'unités, ordres, ralliement, obstacles. | **tranché (D285)** : déplacer ses Paysans seuls ou à plusieurs (clic, cadre, Maj ; clic droit ; chemins ; arrivée répartie ; ralliement ; bloc de sélection de groupe) |
| Q2 | **Chemins** : comment une unité ou un groupe trouve et suit son chemin (navmesh, obstacles posés en cours de partie). | **tranché (D286)** : un chemin par paquet, couloir partagé, navmesh à modificateurs dynamiques, obstacles sur `Plain` |
| Q3 | **Ordre de groupe** : où arrive chaque unité d'un groupe (pas d'empilement), formation ou non. | **tranché (D287)** : arrivée en tas serré, ordre relatif gardé, sans formation |
| Q4 | **Sélection des unités et retour visuel** (unités sans collision : choix sous le curseur, cadre, ce que l'écran montre). | **tranché (D288)** : projection à l'écran, clic droit vérifié par le serveur, ralliement ; **cercle sous les unités sélectionnées et carré sous le bâtiment sélectionné** (revient sur D259) |
| Q5 | **Vérification** (tests, `quick_check` solo et 2 clients, S2 avec chemins), et découpage en sous-étapes. | **tranché (D289)** : tests, solo et 2 clients dans `quick_check`, S2 avec chemins, S5 court, essai |

L'utilisateur a demandé le plan d'implémentation complet après Q1 (2026-10-14) : Q2 à Q5 proposées d'un bloc, puis
**validées** (« je valide »), avec pour le retour visuel l'option (a) **et** un carré sous le bâtiment sélectionné.

**Tranché — D286 Chemins : un chemin par groupe, sur le navmesh, calculé par le serveur.**
- Au pas où l'ordre s'applique, le serveur coupe les unités visées en **paquets** (unités à moins de ~15 m l'une de
  l'autre) et fait **une requête de chemin par paquet** (`FindPath` sur le navmesh, du centre du paquet à la
  destination), avec un **budget de requêtes par pas** (les suivantes attendent le pas d'après, comme les naissances,
  D224). Le chemin devient un **couloir** (`FNavCorridor`, ~6 m de large) **partagé** par les unités du paquet ;
  chacune le suit par le chemin court de Mass (`FMassNavMeshShortPathFragment`, `MassNavMeshPathFollowProcessor`,
  déjà dans la liste des processeurs de simulation) : c'est la recette de l'errance d'essai de M1 (commit e8fa244),
  partagée au lieu d'une par unité. Au bout du couloir, chaque unité rejoint **sa place** (D287).
- Navmesh **dynamique, modificateurs seuls** (`RuntimeGeneration = DynamicModifiersOnly`) : chaque bâtiment porte un
  modificateur de navigation (zone interdite) sur son emprise, tirée de la table (D260) ; les tuiles touchées se
  refont au serveur à l'apparition, comme ce sera le cas pour la construction et les murs (M8). Le client n'a pas
  besoin du navmesh : ses clics visent le sol par un rayon.
- `Plain` reçoit quelques **obstacles fixes** (rochers, bosquet) par `create_match_map.py`, pour que les chemins se
  voient ; `StressTest` garde son sol.
- Destination ramenée au navmesh (D228) ; dans un bâtiment ou un obstacle, le point atteignable le plus proche.
- Écartés : **un chemin par unité** (simple, mais 175 requêtes pour un grand ordre et S2 très cher au départ ; reste
  le cas d'un paquet d'une unité) et **champ de flux** (bon pour de très grandes foules vers un seul point, gros
  chantier, à garder si les couloirs ne tiennent pas en M4-M5).

**Tranché — D287 Ordre de groupe : arrivée en tas serré, sans formation.**
- Une unité seule va au point exact. Un groupe reçoit des **places** autour du point : disque serré en nid d'abeille
  (écart = diamètre du corps + ~20 %), orienté selon la marche ; chaque place est ramenée au navmesh, une place dans
  un obstacle passe à la suivante libre. Les places sont **attribuées en gardant l'ordre relatif** des unités (celle
  de gauche arrive à gauche), pour que les chemins ne se croisent pas (D230 : deux unités qui se croisent se
  traversent).
- Chaque unité marche à sa propre vitesse (un seul type pour l'instant). Les **formations** (rangs, orientation en
  glissant le clic droit, vitesse du plus lent) restent pour l'étape « armée » (M4).
- Règles pures dans `Simulation/HOLGroupMoveRules` (places, attribution, paquets), testées seules.
- Écartés : **toutes au même point** (empilement, évitement à l'arrêt qui pousse sans fin) et **formation en rangs dès
  J4** (sans objet pour des Paysans).

**Tranché — D288 Sélection des unités : chez le client, par projection à l'écran.**
- L'état de sélection reste dans `AHOLPlayerController` (D262) : **une liste d'identifiants d'unités** (D223) ou un
  bâtiment. Les unités n'ont pas de collision : on les vise en **projetant à l'écran** leur position affichée (en solo
  les entités de la simulation, chez un client les entités d'affichage de M2.5 ; coût négligeable). Clic : l'unité
  dont le corps (rayon et hauteur × 1,5) contient le curseur, la plus proche de la caméra, **avant** un bâtiment ;
  cadre (glisser au clic gauche au-delà de ~6 pixels) : ses unités dont le centre projeté est dedans ; Maj + clic ou
  cadre ajoute ou retire ; une unité adverse se sélectionne seule ; Échap ou clic dans le vide vident ; une unité
  disparue sort de la sélection.
- Ordres : **clic droit** sur le sol avec ses unités sélectionnées → appel serveur du contrôleur (identifiants +
  point, ~4 octets par unité) ; le **serveur vérifie le propriétaire** de chaque unité et ignore les autres, puis
  file de commandes (D252). Centre sélectionné + clic droit → **point de ralliement** (sur le bâtiment, répliqué au
  propriétaire, D282) ; le Paysan produit y reçoit un ordre à sa naissance (sans ralliement : naissance en bas, D283).
- Actions dans `IMC_Game` par `make_game_input.py` : clic droit (ordre), Maj (modificateur), glisser.
- **Retour visuel — choix de l'utilisateur : (a) + carré sous le bâtiment sélectionné** (il revient sur le retrait de D259) :
  (a) **cercle au sol sous chaque unité sélectionnée**, à la couleur du joueur (instances d'un anneau plat, comme
  AoE4) — recommandé : avec 10 Paysans identiques, le HUD ne dit pas lesquels sont pris ; (b) rien sur les unités,
  seulement le bloc de sélection ; (c) unités sélectionnées éclaircies (paramètre par instance). **Retenu : (a), et un
  carré au sol sous le bâtiment sélectionné, à la couleur de son propriétaire.** Dans tous les cas :
  le **rectangle du cadre** pendant le glisser, un **repère bref** au point du clic droit (~0,5 s), un **fanion** au
  point de ralliement quand le centre est sélectionné.
- Bloc de sélection : une unité → nom, propriétaire, points de vie ; plusieurs → nom × nombre par type. Textes fr / en
  / ja.
- Écartés : **sélection par collision** (une capsule par unité, contraire aux unités légères de D29) et **sélection
  connue du serveur** (D262).

**Tranché — D289 Vérification : tests, partie solo et à 2 clients, S2 avec chemins.**
- Tests d'automatisation : places d'arrivée (aucun recouvrement, nombre, orientation, place dans un obstacle),
  attribution (ordre relatif gardé), paquets, projection et cadre de sélection.
- `quick_check.ps1`, partie **solo** : ralliement posé, 3 Paysans produits qui y marchent ; cadre simulé sur les 3,
  clic droit derrière un rocher : arrivée sans recouvrement, chemin plus long que la ligne droite (contournement),
  aucun Paysan dans un rocher ou un centre ; Maj + clic ; Échap ; captures. Nouveaux verbes ou commandes au besoin
  (`hol.UI.Drag`, positions dans `hol.Units.Report`, `waitidle`).
- Partie **à 2 clients** : chacun déplace son Paysan, l'autre le voit arriver (position affichée à < 50 cm) ; un ordre
  sur l'unité de l'autre est ignoré par le serveur.
- **S2 rendu exigeant** (promesse de D229) : ses ordres passent par les chemins (budget par pas), seuil de 3 ms ;
  S1 inchangé ; **S5 court** (des chemins courbes peuvent coûter plus d'envois). Essai de l'utilisateur.

**Sous-étapes de J4** (validées ; chacune compile, se vérifie et se commite seule) :

1. **J4.1 Navmesh dynamique et obstacles** : `DynamicModifiersOnly`, modificateur de navigation sur `AHOLBuilding`
   (emprise de la table), rochers et bosquet dans `Plain` (`create_match_map.py`), commande `hol.Nav.Path` (longueur
   et points d'un chemin) ; vérifier qu'un chemin contourne un centre ; S1 / S2 courts (le mode du navmesh change
   aussi pour `StressTest`).
2. **J4.2 Règles pures** : `Simulation/HOLGroupMoveRules` (paquets, places, attribution) et tests
   `HeroesOfLiberty.Movement.*`.
3. **J4.3 Chemins dans la simulation** : `UHOLUnitOrderProcessor` → file de requêtes de chemin au budget par pas,
   couloir partagé par paquet, chemin court de Mass, dernier tronçon vers la place, arrivée ; retrait du maintien
   manuel de `DistanceToGoal` là où Mass l'entretient ; commande `hol.Units.Move` ; S2 avec chemins mesuré.
4. **J4.4 Ordre réseau et ralliement** : appel serveur du contrôleur avec contrôle du propriétaire ; point de
   ralliement sur `AHOLBuilding` ; la production l'utilise ; commande `hol.Prod.Rally` ; vérifié en solo et par
   `net_match.ps1`.
5. **J4.5 Sélection des unités** : projection, clic, cadre, Maj, priorité unité > bâtiment, Échap ; actions
   `IMC_Game` ; clic droit → ordre ou ralliement.
6. **J4.6 Retour visuel et HUD** : cercles sous les unités sélectionnées, carré sous le bâtiment sélectionné ; rectangle du cadre, repère d'ordre, fanion de ralliement ;
   bloc de sélection d'un groupe ; textes fr / en / ja.
7. **J4.7 Vérification (D289)** : parties solo et 2 clients dans `quick_check.ps1`, S1 / S2 / S5 courts, essai de
   l'utilisateur.
8. **J4.8 Clôture** : D285 à D289 au GDD (§ 21, paragraphe « Parcours J4 » au § 16.6).

Risques connus : croisements dans un groupe (D230, réduits par l'attribution qui garde l'ordre relatif) ; coût des
requêtes de chemin au départ de S2 (budget par pas) ; navmesh dynamique sur le serveur dédié (mémoire, temps de
reconstruction d'une tuile, à mesurer en J4.1).

## Plan de J5 (à valider question par question)

Mené **en parallèle de la fin de J4, dans une autre session** (ouvert le 2026-10-14) ; aucun code avant la clôture de
J4. Décisions à partir de **D295** (D290 à D294 laissées à J4).

Repères : § 5.1 (4 ressources : nourriture, bois, pierre, or ; pierre contestable), § 5.3 (population, ~70 à 90
travailleurs en fin de partie), § 6 (camps de collecte, ferme, construction libre, D22), § 7.3 (touche « . » : Paysans
inactifs, D30), § 11 (technologies de collecte et Brouette, D129, D180), D66 / D91 / D39 (variantes de travailleurs :
collecte plus lente, plus rapide), D83 (bâtiments économiques du prototype : maison, camps, ferme), D252 (J5 récolte et
ressources, puis construction), D282 (ressources dans le `PlayerState`, au seul propriétaire), D285 à D289 (ordres et
chemins de J4). Déjà en place : 4 ressources et population dans `AHOLPlayerState`, coût du Paysan, HUD du haut, ordres
de déplacement par la file de commandes, navmesh à modificateurs dynamiques et obstacles sur `Plain`.

| # | Question | État |
|---|---|---|
| Q1 | **Périmètre** de J5 : quelles sources, quels dépôts, sans construction (elle vient après). | **tranché (D295)** : 4 ressources (arbres, baies, filons de pierre et d'or), dépôt au centre seul, épuisement ; troupeaux gardés pour plus tard |
| Q2 | **Boucle de récolte** : charge portée, aller-retour, dépôt le plus proche, nœud épuisé, Paysan inactif. | **tranché (D296)** : aller-retour façon AoE4, charge de 10, dépôt le plus proche, source voisine du même type ou inactif |
| Q3 | **Gisements** dans la partie : ce qu'est un arbre ou un filon (acteur, entité, instance), placement sur `Plain`, épuisement, réplication, navmesh. | **tranché (D297)** : registre serveur, instances par type, acteur partagé à tableau rapide, un fournisseur de zones interdites |
| Q4 | **Commande et retour visuel** : clic droit sur une ressource, ce que l'écran et le HUD montrent, touche « . ». | **tranché (D298)** : option C (contre la recommandation) : le nécessaire (source sélectionnable, charge, comptes, inactifs, « . ») plus animation de récolte, charge visible, « +10 » au dépôt |
| Q5 | **Vérification** (tests, `quick_check` solo et 2 clients, coût à 2 000 unités) et découpage en sous-étapes. | **tranché (D299)** : tests, solo et 2 clients dans `quick_check`, stress test S7 (2 000 Paysans en récolte), S5 court, essai ; trois parties jouables J5a, J5b, J5c |

**Tranché — D295 Périmètre : les 4 ressources, une source simple chacune, dépôt au centre seul.**
- Sources : **arbres** (bois), **buissons de baies** (nourriture), **filons de pierre** et **filons d'or**. Chaque source a
  une quantité et **s'épuise**.
- Dépôt : le **centre principal**, seul tant que la construction n'existe pas ; les camps de l'étape construction
  ajouteront des dépôts sans changer la boucle.
- `Plain` : sources posées par script, **symétriques** : un bois et des baies près de chaque centre, pierre et or plus
  loin, contestés entre voisins (§ 15).
- **Nuance de l'utilisateur : garder les troupeaux façon AoE pour plus tard** (moutons repérés, ramenés près du centre,
  abattus) : ils demandent points de vie, mort et carcasse, donc viennent avec M5.
- Écartés : bois et baies seuls (pierre et or non éprouvés, deux compteurs figés) ; troupeaux dès J5.

**Tranché — D296 Boucle de récolte : aller-retour façon AoE4.**
- Récolte **au contact** de la source, à un **taux par type d'unité et par ressource** (~0,7/s, indicatif ; les variantes
  de D66, D91, D39 ne changeront que ce taux) ; **charge portée de 10** (la Brouette, § 11, l'augmentera).
- Plein, le Paysan va au **dépôt le plus proche** (le centre pour l'instant) ; le stock du joueur monte **au dépôt** ;
  puis il revient à **la même source**.
- Source épuisée → la plus proche **du même type** dans ~15 m ; sinon **inactif** (touche « . », § 7.3).
- Ordre de déplacement : la récolte s'arrête, **la charge reste** ; ordre de récolte sur une autre ressource : **charge
  perdue** (comme AoE).
- Aucune limite de Paysans par source ; ils se répartissent sur des places autour d'elle (règles de J4, D287).
- Écartés : revenu au contact sans trajet (la distance ne compte plus, camps sans objet, contraire à D83) ; places
  limitées par source façon StarCraft (attente, gestion fine, illisible avec des unités qui se traversent).

**Tranché — D297 Sources : registre côté serveur, affiché par instances.**
- Ordre de grandeur : à 8 joueurs, un bois de ~40 arbres et ~6 buissons par centre, plus les filons entre voisins :
  **400 à 500 sources**, 9 sur 10 des arbres.
- Un **sous-système** tient le tableau des sources (type, position, rayon, quantité restante) ; la simulation le lit et
  le modifie au serveur.
- Affichage : **un composant d'instances par type de source** (quelques appels de rendu en tout).
- Réplication : **un acteur partagé à tableau rapide** (recette des morceaux de M2.5) : liste complète à l'arrivée d'un
  client, puis seulement les quantités changées et les épuisements.
- Navmesh : **un seul fournisseur de zones interdites** (une boîte par source) ; une source épuisée retire sa boîte et
  seule sa tuile se refait (mécanisme de J4.1). Une forêt recule arbre par arbre.
- Quantités indicatives (à régler en test) : arbre 100 bois, buisson 200 nourriture, filon 2 500. Formes provisoires et
  sobres : tronc et cône vert, boule à baies rouges, blocs gris (pierre) ou dorés (or).
- Écartés : **un acteur répliqué par source** (500 acteurs : réseau, mémoire, arrivée d'un client ; contraire à D29 et
  D190) ; **mixte** acteurs pour filons et baies, registre pour les arbres (deux chemins pour un même comportement).

**Tranché — D298 Ordre de récolte et retour visuel : option C, contre la recommandation (A).**
- **Clic droit sur une source** avec ses Paysans sélectionnés → ordre de récolte, vérifié par le serveur (D288) ;
  **ralliement sur une source** → le Paysan né y va récolter.
- **Clic gauche sur une source** → bloc de sélection : nom et quantité restante (« Filon d'or — 1 840 »).
- Bloc d'un **Paysan** : activité et charge (« Bois 7/10 ») ; d'un groupe : nombre de Paysans par activité.
- **Barre du haut** : à côté de chaque ressource, le nombre de Paysans qui la récoltent ; compteur de **Paysans
  inactifs** (clic ou touche **« . »** : inactif suivant, caméra centrée, D30).
- **Repère bref** de J4.6 sur la source visée.
- **En plus (choix de l'utilisateur)** : **animation de récolte** (coup de hache, de pioche, cueillette), **charge
  visible** sur le Paysan (fagot, panier, bloc de pierre ou d'or), **« +10 » qui monte** au dépôt.
- Conséquences techniques : l'activité répliquée (déjà dans l'état réduit de M2.5) gagne « récolte » et le type de
  charge (quelques bits ; S5 à remesurer) ; l'animation est choisie par le client d'après l'activité (comme D234) ; le
  « +10 » est montré au seul propriétaire.
- Risques : animations de récolte à trouver ou à faire pour le mannequin ; charge posée sur le squelette instancié de M2
  (pièce fixée à un os ou instance placée d'après l'unité, à essayer).
- Écartés : **A seul** (le nécessaire, sans animation ni charge visible) ; **le strict minimum** (clic droit et
  ralliement seuls).

**Tranché — D299 Vérification : tests, partie solo et à 2 clients, stress test de récolte S7.**
- Coût nouveau : **chaque aller-retour demande un chemin** (à 2 000 unités et ~20 s par trajet, ~100 chemins par
  seconde) ; le chemin entre une source et son dépôt est **réutilisé** plutôt que recalculé à chaque trajet.
- Tests d'automatisation des règles pures : taux et charge, dépôt le plus proche, source voisine du même type ou
  inactif, épuisement, charge perdue au changement de ressource.
- `quick_check.ps1`, partie **solo** : ralliement sur des baies, les Paysans produits y récoltent ; le stock **ne monte
  qu'au dépôt** ; arbre épuisé → arbre voisin ; bois vidé → inactifs, touche « . » ; captures de l'animation et de la
  charge.
- Partie **à 2 clients** : la récolte de l'autre se voit, une source épuisée disparaît chez les deux, le stock de
  l'autre n'est jamais reçu.
- **Nouveau stress test S7 : 2 000 Paysans en récolte** sur `StressTest` (sources posées par script), seuil de
  simulation de 3 ms (D218) ; **S5 court** (état répliqué grossi de quelques bits) ; essai de l'utilisateur.
- Écarté : `quick_check` seul, sans stress test de récolte (coût des trajets découvert à l'étape « armée »).

**Découpage de J5 en trois parties jouables** (demande de l'utilisateur, option A, conforme à la recommandation ;
précise D299) : chaque partie finit par une vérification et **un essai de l'utilisateur** ; chaque sous-étape compile,
se vérifie et se commite seule ; **aucune avant la clôture de J4**. Le gros risque (coût des trajets, S7) est mesuré dès
J5a, sur une seule ressource ; la partie la plus incertaine (animations, charge sur le squelette) vient en dernier et
peut glisser sans bloquer l'économie.

**J5a — Le bois** : à la fin, le joueur envoie ses Paysans couper du bois (clic droit ou ralliement), les voit faire
l'aller-retour au centre et le bois monter ; la forêt recule ; un Paysan sans arbre devient inactif.

1. **J5a.1 Données et règles pures** : table des sources (CSV → table, comme `BuildingTypes`), taux et charge dans la
   table des unités, `Simulation/HOLGatherRules` (taux, charge, dépôt le plus proche, source voisine, épuisement,
   charge perdue), tests `HeroesOfLiberty.Gather.*`.
2. **J5a.2 Sources dans la partie** : registre au serveur, composants d'instances par type, acteur partagé à tableau
   rapide, fournisseur de zones interdites ; **arbres** posés par script sur `Plain` (un bois par centre) et
   `StressTest`.
3. **J5a.3 Boucle de récolte dans la simulation** : machine à états (aller, récolter, rapporter, revenir), dépôt au
   `PlayerState`, épuisement, inactif, chemin source-dépôt réutilisé ; commande `hol.Units.Gather` ; **S7 mesuré**.
4. **J5a.4 Ordre réseau et ralliement** : clic droit sur un arbre → ordre vérifié par le serveur ; ralliement sur un
   arbre ; activité et charge dans le bloc du Paysan.
5. **J5a.5 Vérification** : `quick_check` solo (ralliement, stock qui ne monte qu'au dépôt, arbre voisin, inactif) et
   2 clients (récolte vue, arbre épuisé disparu chez les deux, stock de l'autre jamais reçu), S1 / S2 / S7 courts,
   **essai de l'utilisateur**.

**J5b — Les 4 ressources et le HUD** : à la fin, le joueur récolte baies, pierre et or (filons contestés entre
voisins), sélectionne une source pour voir ce qu'il reste, suit ses Paysans par ressource et retrouve les inactifs.

1. **J5b.1 Baies et filons** : buissons et filons de pierre et d'or posés par script sur `Plain` (symétriques, D295) ;
   récolte des 4 ressources (charge perdue au changement de ressource).
2. **J5b.2 HUD** : sélection d'une source (nom, quantité restante), bloc d'un groupe (Paysans par activité), comptes
   par ressource dans la barre du haut, compteur d'inactifs, touche « . », repère bref sur la source visée ; textes
   fr / en / ja.
3. **J5b.3 Vérification** : `quick_check` solo (baies au ralliement, bois vidé → inactifs et « . ») et 2 clients,
   **essai de l'utilisateur**.

**J5c — La vie de la récolte** : à la fin, le joueur voit les coups de hache et de pioche, la cueillette, la charge
portée et le « +10 » au dépôt.

1. **J5c.1 Animations de récolte** : trouvées ou faites pour le mannequin, choisies par le client d'après l'activité
   (comme D234) ; l'activité répliquée gagne « récolte » et le type de charge. **Faites par Claude** (2026-10-16 ;
   fabrication dans `AnimationsInProgress.md`). Sous-étapes (2026-10-16, après repérage) :
   a. **Outil et premier geste** : `Tools/Editor/make_peasant_animations.py` (poses clés, mise à jour en place,
      contrôles : racine immobile, boucle fermée), aperçu image par image, coup de hache d'abord ; regard de
      l'utilisateur, puis pioche, cueillette, marche chargée (jambes d'une marche, bras réécrits).
   b. **Fournisseur et table** : séquences ajoutées au fournisseur, colonnes de la table des types (récolte par
      ressource, marche chargée, vitesses de lecture), recompiler, régénérer. **Fait** (2026-10-16, worktree, rien de
      commité ; détail dans `AnimationsInProgress.md`) : indices 2 à 7, `GatherSequences` / `CarryWalkSequences`,
      `CarryWalkPlayRate` 3,57 (joué 3,57 / 1,5 = 2,38 à l'écran : le client divise déjà par `MeshScale`).
   c. **Ressource connue du client** : **tranché (2026-10-16, option A, la recommandation)** : « récolte + ressource »
      dans les 4 valeurs libres de l'activité sur le fil (3 bits), exact, immédiat, 0 bit de plus. Écartés : B source la
      plus proche chez le client (ambigu entre sources voisines), C 2 bits de plus (~+1 Ko/s, S5 en échec). **Fait**
      (2026-10-16, worktree) : `GatherResource` à côté de l'activité, valeurs 4 à 7 sur le fil, 60 / 62 bits inchangés.
   d. **Choix chez le client** : fonction pure testée, récolte prioritaire sur la vitesse, marche chargée si porteur,
      Paysan tourné vers sa source. **Fait** (2026-10-16, worktree) : `HOLUnitAnimation::Choose`, tests 41/41,
      `quick_check` PASS, captures en jeu ; reste f (S1 / S2 / S7 / S5 courts, essai de l'utilisateur, commit).
   e. **Pose au dépôt** : **tranché (2026-10-16, option A, la recommandation)** : **pas de geste**, comme AoE4 (le
      Paysan touche le dépôt et repart ; la charge qui disparaît, J5c.2, et le « +10 », J5c.3, montrent le dépôt).
      Écarté : B pause de ~0,4 s au serveur (récolte ralentie, S7 et tests à refaire). Sous-étape supprimée.
   f. **Vérification** : tests, captures en gros plan (`hol.Camera.Zoom` à ajouter), S1 / S2 / S7 / S5 courts, essai.
2. **J5c.2 Charge visible** : fagot, panier, bloc de pierre ou d'or (pièce fixée à un os ou instance placée d'après
   l'unité, à essayer).
3. **J5c.3 « +10 » au dépôt** : montré au seul propriétaire.
4. **J5c.4 Vérification et clôture** : captures dans `quick_check`, S5 court, **essai de l'utilisateur** ; paragraphe
   « Parcours J5 » au § 16.6 (D295 à D299 au § 21 et S7 aux seuils des stress tests depuis la clôture de J5a).

Risques connus : coût des trajets à 2 000 Paysans (S7) ; animations de récolte à trouver pour le mannequin ; charge
posée sur le squelette instancié ; reconstruction des tuiles quand une forêt recule vite.

**Repérage pour J5c.1 et J5c.2** (2026-10-15, lecture seule, pendant J5b.2 ; rien de décidé) :
- **Squelette** : mannequin UE4 de `CloseCombat` (`make_skinned_mannequin.py`), douilles `Socket_Weapon` (main droite),
  `Socket_HandL`, `Socket_WeaponSpine`. Un fournisseur `UAnimSequenceTransformProviderData` par maillage = liste de
  séquences par indice ; la table des types ne connaît que `IdleSequence` / `WalkSequence` ; le client choisit d'après
  la vitesse seule (`HOLInterpolatedInstancesProcessor.cpp` ~l. 148), toujours en boucle. Séquences **sur place**
  obligatoires (l'os racine est joué tel quel). Pendant la récolte, le client ne connaît la ressource qu'une fois la
  charge commencée (`bCarrying`) : ~1,4 s sans savoir hache / pioche / cueillette.
- **Animations dans le projet** : **aucune de récolte**. Pis-aller : coups d'épée de `CloseCombat/.../Swordsman/Attack`
  (`A_SW_HeavyAttack`, `A_SW_Attack_01..05`) en hache ou pioche (racine à figer) ; rien de bon pour la cueillette, la
  marche chargée ou la pose au dépôt (`AnimStarterPack` : accroupi au fusil, marche au fusil, autre squelette à
  recibler). Moteur (`C:/UE/UE_5.8_AS_3`) et cache du lanceur (Lyra seul) : rien de mieux (Manny, reciblage UE5 → UE4).
  Pistes **à vérifier** : Mixamo (gratuit, récolte / minage / port, à recibler), packs « paysan / ouvrier » sur Fab,
  courtes boucles faites au Control Rig.
- **Pièces** (`FantasyVillage/Meshes`) : bois `SM_WoodChunks01..05`, `SM_Planks` ; pierre `SM_StoneWallSingle01..03`,
  `SM_Rock01/02` ; **ni panier, ni or, ni hache, ni pioche** (à faire en formes simples, comme les arbres).
- **Fixer une pièce** : le fournisseur est sur GPU seul, donc pas de maillage statique attaché à un os. Voie la plus
  simple : pièce en **maillage squelettique lié à `hand_r` / `spine_03`** sur le même squelette, ajoutée à la liste des
  maillages du type d'unité (Mass lui envoie la même animation), cachée par une donnée par instance ; coût : chaque
  Paysan dessine toutes les pièces, à mesurer sur S7. Repli : instance statique posée d'après l'unité (épaule, dos),
  sans suivre les mains.

## Plan de J6 — la construction (à valider question par question)

Mené **en parallèle de J5b.2, dans une autre session** (ouvert le 2026-10-15) ; **aucun code avant la clôture de J5**.
Décisions à partir de **D310** (D307 à D309 laissées à J5). Ce plan n'écrit que dans cette section ; le GDD ne change
qu'à la clôture de J6.

Repères : § 5.3 et D83 (maisons ~ +10 de population, plafond 175), § 6 et D22 (construction libre, vision requise, rien
près d'un centre principal ennemi, règles propres à certains bâtiments), D128 (production militaire par catégorie),
§ 7.3 et D30 (F2 bâtiments économiques, grille `A Z E R / Q S D F / W X C V`), § 15 (« ce qu'il peut construire »),
§ 17 (bâtiments économiques du prototype : maison, camps de collecte bois / pierre / or, ferme, marché), D252
(construction après J5), D260 (bâtiment = acteur répliqué, valeurs en table), D261 (formes provisoires), D281 (HUD :
grille 3 × 4, blocs à leur place définitive), D286 (navmesh à modificateurs dynamiques : une emprise posée refait ses
seules tuiles), D296 (dépôt le plus proche). Déjà en place : `AHOLBuilding` répliqué apparu par le serveur, table
`BuildingTypes` (emprise, hauteur, points de vie, `PopulationProvided`, `Produces`, `Accepts`), plafond de population
calculé d'après les bâtiments (`GetPopulationCap`, **10 places avec le seul centre**), `LumberCamp` provisoire (S7
seulement), grille de commandes avec la case « Paysan », ordres par la file de commandes vérifiés par le serveur.

| # | Question | État |
|---|---|---|
| Q1 | **Périmètre** de J6 : quels bâtiments, ce qui marche à la fin. | **tranché (D310)** : maison, camp de bûcherons, camp de mineurs (pierre et or) ; ferme et marché à l'étape suivante ; **+ avertissement : le navmesh frôle ou entre dans le centre principal, à corriger** |
| Q2 | **Pose** : fantôme, emplacement valide (grille ou libre, recouvrements, sources, rayon anti-rush, vision), coût payé quand, pose en file (Maj). | **tranché (D311)** : fantôme vert / rouge calé sur une grille de ~1 m, sans rotation, payé à la pose, Maj pour poser en file, serveur qui revérifie |
| Q3 | **Bâtir** : fondation, Paysans bâtisseurs (seuls ou à plusieurs), durée, annulation, ce que font les Paysans après. | **tranché (D312)** : plusieurs bâtisseurs à rendement décroissant ; camp fini → récolte de la source la plus proche qu'il accepte ; maison finie → chantier suivant ou inactif |
| Q4 | **Commande et retour visuel** : grille de construction du Paysan, touches, coût affiché, manque de ressources, bloc d'un chantier. | **tranché (D313)** : grille directe du Paysan (A, Z, E), infobulle et coût, case grisée et message bref, « population maximale », bloc du chantier avec annulation, F2 |
| Q5 | **Vérification** (tests, `quick_check` solo et 2 clients, stress tests) et découpage en parties jouables. | **tranché (D314)** : deux parties jouables J6a (maison et chantier, correctif du navmesh en premier) et J6b (camps, S7 sur des camps bâtis en cours de test), chacune finie par l'essai de l'utilisateur |

**Tranché — D310 Périmètre : maison et camps de collecte** (choix de l'utilisateur, conforme à la recommandation).
- **Maison** (~ +10 de population, en bois ; valeurs indicatives) : le joueur dépasse les 10 places du seul centre.
- **Camp de bûcherons** (dépôt du bois) et **camp de mineurs** (dépôt de la pierre **et** de l'or, un seul camp façon
  AoE4, lecture du § 17 « camps de collecte (bois, pierre, or) ») : dépôts au plus près des sources (filons à 60 m du
  centre sur `Plain`). Le `LumberCamp` provisoire de S7 devient le vrai camp.
- À la fin de J6 : le joueur choisit où bâtir, ses Paysans bâtissent, la population monte, les allers-retours
  raccourcissent (dépôt le plus proche de D296, colonne `Accepts` déjà là).
- Plus tard : **ferme** (source infinie de nourriture quand les baies sont épuisées, 6 × 200 par emplacement) et
  **marché** (D70, D84), à l'étape suivante ; militaire avec l'armée (D128).
- Écartés : **A + ferme** (la ferme est un bâtiment-source, touche le registre et la boucle de récolte : J6 presque
  doublée) ; **maison seule** (pas de vraie décision de placement sans les camps).
- **⚠️ Avertissement de l'utilisateur (2026-10-15) : le navmesh du centre principal (« forum ») passe très près, voire
  dedans** ; les Paysans frôlent ou entrent dans le bâtiment. **À corriger dans J6**, pour le centre et chaque bâtiment
  posé (maisons et camps serrés les uns contre les autres l'aggraveront). Piste à vérifier : la zone interdite
  (`UNavArea_Null`) est une boîte **exactement** de l'emprise (`AHOLBuilding::FitNavObstacle`) ; un modificateur de zone
  n'est pas érodé du rayon de l'agent comme un mur, donc le centre d'un Paysan longe le bord et son corps entre à moitié
  dans le bâtiment. Remède probable : élargir la boîte du rayon du corps et d'une marge, la même règle pour tous les
  bâtiments ; vérifier aussi les cylindres des sources (D297) et l'arrivée au dépôt (places au bord de l'emprise, D296).
  Vérification attendue : aucun Paysan dans l'emprise d'un bâtiment (contrôle scripté dans `quick_check`).

**Tranché — D311 Pose : grille, sans rotation, payé à la pose, Maj pour poser en file** (choix de l'utilisateur,
conforme à la recommandation, façon AoE4).
- **Fantôme** du bâtiment sous le curseur, **vert** si posable, **rouge** sinon ; calé sur une **grille de ~1 m**
  (valeur indicative) ; **orientation fixe**. Les bâtiments peuvent se toucher (rangées de maisons en obstacle, D83 ;
  prépare les murs).
- Pose refusée si l'emprise **chevauche un bâtiment ou une source**, sort de la **carte** ou du **navmesh**, ou entre dans
  le **rayon anti-rush** d'un centre principal ennemi (D22, rayon à régler en test). Le **serveur revérifie** tout
  (D288). La **vision** de D22 viendra avec le brouillard de guerre.
- **Coût retiré à la pose** de la fondation ; **remboursé en entier** si annulée avant le premier coup de travail, **à
  moitié** ensuite. On ne réserve jamais plus que ce qu'on a.
- **Maj** : plusieurs fondations à la suite, bâties dans l'ordre par les Paysans envoyés.
- Écartés : **pose libre et rotation** (alignements approximatifs, trous de 30 cm illisibles, murs plus durs, molette
  déjà prise par le zoom) ; **payé au premier Paysan arrivé** (chantier bloqué faute de ressources, règle et message en
  plus).

**Tranché — D312 Bâtir : plusieurs bâtisseurs à rendement décroissant, enchaînement façon AoE4** (choix de
l'utilisateur, conforme à la recommandation).
- Les Paysans envoyés prennent des **places autour de l'emprise** (règles des places autour d'une source, J5b.1) et
  travaillent **au contact** ; **durée de base** par bâtiment dans la table (indicatif : maison ~20 s, camp ~25 s pour un
  bâtisseur).
- **Plusieurs bâtisseurs** : chacun de plus accélère, **de moins en moins** (indicatif : 3 Paysans ≈ 2 fois plus vite
  qu'un seul) ; combien en retirer de la récolte reste un choix.
- L'emprise **bloque le navmesh dès le premier coup de travail** (pas à la pose) ; les unités qui s'y trouvent en
  sortent ; la marge de D310 s'applique.
- Le bâtiment **monte du sol** avec l'avancement ; ses **points de vie** aussi (prépare M5) ; il ne sert (dépôt,
  population) qu'une fois fini.
- **Clic droit** sur un de ses chantiers inachevés : aider.
- Après : **camp** fini → ses bâtisseurs **récoltent la source la plus proche qu'il accepte** ; **maison** finie →
  chantier suivant de leur file (Maj), sinon **inactifs** (touche « . »).
- Écartés : **chaque bâtisseur aussi utile que le premier** (10 Paysans, maison en 2 s : plus de choix) ; **un seul
  bâtisseur** façon StarCraft (pas d'urgence possible, contraire à AoE4).

**Tranché — D313 Commande et retour visuel : grille directe du Paysan, coûts visibles, messages brefs, F2** (choix de
l'utilisateur, conforme à la recommandation, façon AoE4).
- Un ou plusieurs **Paysans sélectionnés** : la grille 3 × 4 (D281) montre **directement** les 3 bâtiments, sur les
  touches de la grille de D30 (**A** maison, **Z** camp de bûcherons, **E** camp de mineurs) ; clic ou touche → fantôme
  (D311) ; Échap ou clic droit l'annule.
- **Survol** d'une case : infobulle (nom, coût, effet : « +10 population », « Dépôt : pierre, or »).
- Ressources insuffisantes : case **grisée**, coût **en rouge** ; un clic → **message bref** au-dessus de la grille
  (« Pas assez de bois »).
- **Population pleine** : la production de Paysans s'arrête avec un message bref « Population maximale — construisez des
  maisons » (premier mur du joueur).
- **Repère bref** de J4.6 sur la fondation visée ; le chantier se **sélectionne** comme un bâtiment ; son **bloc** :
  nom, barre d'avancement, nombre de bâtisseurs, case **« Annuler »** (remboursement de D311).
- **F2** : bâtiment économique suivant (D30), caméra centrée.
- Textes fr / en / ja. Les bâtiments militaires ajouteront une **seconde page** à la grille du Paysan (AoE4).
- Écartés : **grille à deux niveaux** façon AoE2 (une touche de plus par construction pour 3 bâtiments) ; **sans
  messages ni F2** (le joueur devine pourquoi rien ne se passe).

**Tranché — D314 Vérification : deux parties jouables, S7 refait sur des camps bâtis** (choix de l'utilisateur,
conforme à la recommandation).
- Nouveau : les bâtiments **apparaissent en cours de partie** ; chaque pose refait des tuiles du navmesh ; les Paysans
  dont le chemin (ou le chemin réutilisé de D305) traverse une emprise nouvelle **recalculent** ; une base serrée met à
  l'épreuve la marge de D310.
- Tests d'automatisation des règles pures : grille, emplacement valide (chevauchement, source, carte, anti-rush),
  rendement des bâtisseurs, remboursement, plafond de population.
- `quick_check` **solo** : maison posée et bâtie à 3, population de 10 à 20, annulation remboursée, poses refusées,
  **aucun Paysan dans une emprise** ; camp au bois → ses bâtisseurs récoltent, trajets plus courts.
- **2 clients** : le chantier de l'autre se voit monter, le serveur refuse une pose interdite, les ressources de l'autre
  jamais reçues.
- **S7 refait avec des camps bâtis en cours de test** (au lieu des camps provisoires) : tuiles refaites et chemins
  recalculés sous charge, seuil de 3 ms (D218) ; **S5 court** ; S1 / S2 courts après le correctif du navmesh.
- Écartés : **un seul bloc** (un seul essai, à la fin, contraire à la préférence de l'utilisateur de jouer souvent) ;
  **sans refaire S7** (coût du navmesh refait découvert à l'étape « armée »).

**Découpage de J6 en deux parties jouables** : chaque sous-étape compile, se vérifie et se commite seule (après
validation de l'utilisateur) ; **aucune avant la clôture de J5**.

**J6a — La maison et le chantier** : à la fin, le joueur pose une maison, ses Paysans la bâtissent, sa population
dépasse 10 ; plus aucun Paysan n'entre dans un bâtiment.

1. **J6a.1 Correctif du navmesh des bâtiments (D310)** : emprise interdite élargie du rayon du corps et d'une marge,
   même règle pour tous les bâtiments ; vérifier les cylindres des sources et les places au dépôt ; contrôle scripté
   « aucun Paysan dans une emprise » dans `quick_check` ; S1 / S2 courts (mouvement touché).
2. **J6a.2 Données et règles pures** : table des bâtiments (coût, durée de construction ; lignes `House`, `LumberCamp`
   vrai, `MiningCamp`), `Buildings/HOLConstructionRules` (grille, emplacement valide, rendement décroissant,
   remboursement, plafond), tests `HeroesOfLiberty.Construction.*`.
3. **J6a.3 Pose** : fantôme chez le client (grille, vert / rouge), appel serveur (vérification, coût retiré), fondation
   répliquée (`AHOLBuilding` apparu en cours de partie, état « fondation »), Maj en file, annulation remboursée.
4. **J6a.4 Chantier dans la simulation** : ordre de bâtir par la file d'ordres, places autour de l'emprise, avancement à
   rendement décroissant, navmesh bloqué au premier coup et unités sorties de l'emprise, bâtiment qui monte, points de
   vie, population à la fin ; maison finie → chantier suivant ou inactif ; clic droit sur un chantier : aider.
5. **J6a.5 HUD** : grille du Paysan (A, Z, E), infobulle, case grisée et message bref, « Population maximale », bloc du
   chantier avec « Annuler », repère sur la fondation ; textes fr / en / ja.
6. **J6a.6 Vérification** : `quick_check` solo et 2 clients (partie maison), **essai de l'utilisateur**.

**J6b — Les camps** : à la fin, le joueur pose un camp près du bois ou d'un filon, ses bâtisseurs s'y mettent à
récolter, les allers-retours raccourcissent.

1. **J6b.1 Camps** : camp de bûcherons et camp de mineurs (dépôts de D296), enchaînement camp fini → récolte de la source
   la plus proche qu'il accepte, F2 ; le `LumberCamp` provisoire de S7 disparaît.
2. **J6b.2 Chemins coupés par une emprise** : marches et chemins réutilisés de la récolte (D305) recalculés quand une
   emprise nouvelle les coupe.
3. **J6b.3 Vérification** : `quick_check` solo et 2 clients (partie camps), **S7 avec des camps bâtis en cours de
   test**, S5 court, **essai de l'utilisateur**.
4. **J6b.4 Clôture** : D310 à D314 au § 21 du GDD, paragraphe « Parcours J6 » au § 16.6.

Risques connus : tuiles refaites en rafale quand plusieurs joueurs bâtissent en même temps ; chemins (et chemins
réutilisés de D305) traversant une emprise nouvelle ; Paysans enfermés par une rangée de maisons (pose à refuser si elle
enferme des unités ? à voir à l'essai) ; marge de S7 déjà mince (0,09 ms au repos).

## Plan de J7 — l'armée et le combat (M4, M5)

Préparé **le 2026-10-16 dans une session cloud, en parallèle de J6** (menée sur le PC de l'utilisateur) ; **aucun code
avant la clôture de J6**. Ce plan n'écrit que dans cette section ; le GDD (§ 21, § 16.6) ne change qu'à la clôture de J7.

**Numéros de décision : à attribuer à la clôture**, à la suite des derniers pris (au 2026-10-16, D318 à D323 sont pris
par J5c et le numéro suivant du § 22 est **D324** ; J6b peut en prendre d'autres). Ci-dessous, les décisions sont notées
**J7-Q1** à **J7-Q6**. Au 2026-10-18, **D326** est pris par les pièces allégées du LOD 3 (mycrusader-e3, précision de
D323) : J7 numérotera à partir du numéro suivant du § 22 à sa clôture (D327 ou plus).

Repères : D20 et D21 (socle commun, matrice de contres façon AoE4), D30 (groupes de contrôle, F1, Ctrl + A, grille),
D62 (aucune unité anti-héros), D74 (moral : effets nommés, plus tard), D86 (cadavres : Légions, avec les factions),
D128 (un bâtiment par catégorie, **bâtiment unique au prototype**), § 7.1 à 7.3, § 16.4 (D29 : unités légères en Mass,
couche RTS maison), § 16.6 (M4 : sélection, ordres de déplacement et d'attaque, groupes de contrôle ; M5 : cible,
mêlée, tir, dégâts et armure, mort ; S3 1 000 contre 1 000 en mêlée, S4 armées mixtes), § 17 (unités du prototype),
D218 (seuils : 3 ms par pas, 32 Ko/s par client), D224 (naissances et destructions différées), D269 à D274 et D300,
D301 (état répliqué, 59 bits), D287 (arrivée en tas serré, formations reportées à M4), D292 et D293 (séparation,
contournement ; **reprise de place reportée aux formations**), D313 (les bâtiments militaires ajoutent une **seconde
page** à la grille du Paysan). Déjà en place : sélection au clic, au cadre et Maj (J4.5), groupes et Tab (J6a.5),
F1 / F2 / F3 (J6a.5), production et ralliement (J3, J4.4), grille hachée de la séparation (D292), types `LightInfantry`
et `Archer` dans la table des unités (stress tests seulement), activité sur 3 bits **pleine** depuis J6a.4
(`Building` = la dernière valeur libre).

| # | Question | État |
|---|---|---|
| Q1 | **Périmètre** de J7 : l'armée seule, ou l'armée et le combat. | **tranché (J7-Q1)** : armée et combat, en trois parties jouables J7a, J7b, J7c |
| Q2 | **Bâtiment et soldats** de J7a. | **tranché (J7-Q2)** : une Caserne ; Lancier et Homme d'armes ; l'Archer s'y ajoute en J7c |
| Q3 | **Commandes d'armée** avant le combat. | **tranché (J7-Q3)** : groupes de contrôle, F1, Ctrl + A, formation simple en rangs ; **la formation s'applique aussi aux Paysans** (nuance de l'utilisateur) |
| Q4 | **Modèle de combat** dans la simulation. | **tranché (J7-Q4)** : processeur maison, règles façon AoE4 |
| Q5 | **Réseau du combat** (PV, activité « attaque »). | **tranché (J7-Q5)** : activité sur 4 bits, PV en 16 paliers |
| Q6 | **Vérification et découpage**. | **tranché (J7-Q6)** : S3 en J7b, S4 en J7c, chaque partie close par l'essai de l'utilisateur |

**Tranché — J7-Q1 Périmètre : l'armée et le combat, en trois parties jouables** (choix de l'utilisateur, conforme à la
recommandation).
- **J7a** l'armée : Caserne, soldats, groupes de contrôle, formation. **J7b** la mêlée : cible, attaque, dégâts et
  armure, mort, S3. **J7c** le tir : Archer, projectiles, S4.
- On peut se battre dès J7b, chaque partie se joue.
- Écartés : **l'armée seule, le combat en J8** (J7 se jouerait peu : des soldats qui marchent sans rien pouvoir faire) ;
  **le combat d'abord, avec des unités apparues par triche** (le plus tôt pour le risque de S3, mais hors du parcours du
  joueur, D252).

**Tranché — J7-Q2 Une Caserne, Lancier et Homme d'armes** (choix de l'utilisateur, conforme à la recommandation).
- **Caserne** : le bâtiment militaire unique du prototype (D128, § 17), posée et bâtie comme une maison (D311, D312), sur
  la **seconde page** de la grille du Paysan (D313) ; F1 la sélectionne (D30). Données conçues pour le découpage de D128
  (catégorie de production par type d'unité). Valeurs indicatives : emprise 9 × 9 m (3 cases de 3 m), 150 bois, ~40 s
  pour un bâtisseur.
- **Lancier** (anti-cavalerie bon marché, infanterie légère) et **Homme d'armes** (tank, infanterie lourde) : deux mêlées
  qui permettent de tester un contre dès J7b (l'Homme d'armes gagne à l'usure contre le Lancier, D21). L'**Archer**
  s'ajoute à la même Caserne en J7c.
- **Apparence** : le mannequin, à la couleur du joueur, avec un accessoire (lance ; épée et bouclier) fabriqué par script
  comme les outils du Paysan (`peasant_props.py`) ; marche et repos repris ; geste d'attaque en J7b
  (`AnimationsInProgress.md`).
- Écartés : **Caserne + Champ de tir dès J7a** (un bâtiment de plus avant que les combats soient validés, contraire au
  § 17) ; **un seul soldat** (aucun contre à tester avant J7c).

**Tranché — J7-Q3 Groupes de contrôle et formation simple, aussi pour les Paysans** (choix de l'utilisateur, conforme à
la recommandation ; nuance de l'utilisateur : la formation simple s'applique aussi aux Paysans).
- **Groupes de contrôle** de D30 : Ctrl + chiffre (assigner), chiffre (sélectionner), deux appuis (centrer la caméra),
  Maj + chiffre (ajouter), Ctrl + Maj + chiffre (retirer). **Ctrl + A** : toute l'armée (les soldats, pas les Paysans).
  **F1** : bâtiment militaire suivant.
- **Formation simple** : un ordre de déplacement au sol donné à un groupe le fait arriver **en rangs**, orientés selon
  la marche (ou selon le **glisser du clic droit**, qui donne aussi la largeur du front) ; le groupe avance à la
  **vitesse du plus lent** tant qu'il est en formation ; une unité **bousculée reprend sa place** ensuite (reporté depuis
  D293). Règles pures (places en rangs, attribution, largeur) dans `Simulation/HOLFormationRules`, testées à part.
- **Paysans** : la même formation remplace le tas serré de D287 pour **tout ordre de déplacement au sol** d'un groupe.
  **Inchangés** : les places de récolte (D309) et de chantier (D312), qui gardent leurs règles. Lecture à confirmer à
  l'essai.
- **Forme des rangs** (règle de l'utilisateur, 2026-10-18, à la relecture du plan) :
  - **Paysans** : **une seule ligne jusqu'à 5**, **deux lignes jusqu'à 10**, 15 → **6, 6, 3**, 20 → **7, 7, 6** : de
    vraies lignes, plus larges que profondes. Règle générale déduite (à confirmer) : jusqu'à 5, une ligne ; au-delà,
    largeur = ⌈√(2N)⌉ (6 → 4, 10 → 5, 15 → 6, 20 → 7, 30 → 8, 50 → 10), lignes remplies de l'avant vers l'arrière, la
    dernière centrée.
    **Les Paysans sont une exception** : leur règle ne sert pas aux soldats.
  - **Soldats d'un seul type** (que des Lanciers) : **formation en carré** par défaut, largeur **⌈√N⌉** (confirmé :
    12 → 4, 4, 4 ; 20 → 5 × 4).
  - **Plusieurs types** (Lanciers et Hommes d'armes) : **formation en ligne**, chaque type à sa ligne selon une
    **priorité de ligne par type** (le type prioritaire devant) ; exemple de l'utilisateur, 12 Hommes d'armes et 12
    Lanciers : ligne 1 : 7 Lanciers ; ligne 2 : 2 Lanciers, 2 Hommes d'armes, 3 Lanciers (**voulu** : le type de
    derrière au centre de la ligne partagée, celui de devant sur les ailes) ; ligne 3 : 5 Hommes d'armes ; ligne 4 :
    5 Hommes d'armes. 12 Hommes d'armes et 12 Archers donnent la même chose. Formule générale (largeur, lignes du
    dernier type) : **à trouver par des scénarios** soumis à l'utilisateur.
  - **Scénarios dessinés par l'utilisateur** (2026-10-18 ; L Lancier, H Homme d'armes, A Archer, devant d'abord) :
    10 L + 10 H → 5 L / 5 L / 5 H / 5 H ; 14 L + 5 H → 5 L / 5 L / 2 L 1 H 2 L / 4 H ; 7 L + 17 H → 7 L / 7 H / 5 H /
    5 H ; 3 L + 21 H → 2 H 3 L 2 H / 7 H / 5 H / 5 H ; 8 L + 8 H + 8 A → 7 L / 1 L 6 H / 1 H 3 A 1 H / 5 A ; 3 L + 1 H →
    3 L / 1 H ; 30 L + 30 H → 11 L / 11 L / 4 L 3 H 4 L / 9 H / 9 H / 9 H. Ordre de priorité lu : **L, puis H, puis A**.
  - **Lecture proposée** (à confirmer) : la forme ne dépend que du nombre N ; **R = ⌊√N⌋ lignes**, largeurs **impaires**
    (un soldat au milieu) : largeur moyenne N / R impaire → lignes égales ; paire → moitié devant à +1, moitié derrière
    à −1 ; sinon la plus petite largeur impaire au-dessus, la dernière ligne plus courte. Remplissage par priorité,
    de l'avant vers l'arrière, chaque ligne centrée ; dans une ligne partagée, **le type qui commence au centre, celui
    qui finit sur les ailes** (première ligne : le type de devant au centre). Reproduit 6 scénarios sur 7 ; 60 soldats
    donnerait 7 lignes (9 × 6 puis 6) au lieu des 6 dessinées (11, 11, 11, 9, 9, 9).
  - 8e scénario de l'utilisateur : 20 L + 20 H → 9 L / 9 L / 1 L 6 H 1 L / 7 H / 7 H (ligne 3 corrigée par
    l'utilisateur : 8 de large). Aile impaire : **le soldat en plus à gauche** (choix de l'utilisateur).
  - **Formule retenue** (reproduit les 8 scénarios) : W = plus petit impair ≥ √N ; R = ⌊N / W⌋ lignes de W ; reste
    e = N − R·W. e = 0 : R lignes de W. Si R ≥ 2 et e ≤ R : les ⌊e / 2⌋ premières lignes passent à W + 2 et, e impair,
    la ligne suivante à W + 1. Sinon : une ligne de plus, de e, centrée. Exemples : 4 → 3 / 1 ; 19 → 5 / 5 / 5 / 4 ;
    20 → 5 × 4 ; 24 → 7 / 7 / 5 / 5 ; 40 → 9 / 9 / 8 / 7 / 7 ; 60 → 11 / 11 / 11 / 9 / 9 / 9. **Confirmée** par les
    dessins de l'utilisateur (2026-10-18).
  - Le glisser du clic droit (orientation, largeur) reste prévu ; il remplace la largeur par défaut.
- **F1** : depuis D313 (J6a.5), sélectionne **tous** les bâtiments militaires (appuis suivants : caméra sur le suivant).
- L'**attaque**, l'**arrêt** et le **maintien de position** viennent en J7b, avec le combat.
- Écartés : **groupes seuls, tas serré** (une armée qui arrive en vrac rend J7b moins lisible) ; **jeu complet façon
  AoE4** (patrouille, suivre, plusieurs formations : J7a s'allonge sans combat pour en profiter).

**Tranché — J7-Q4 Combat : processeur maison, règles façon AoE4** (choix de l'utilisateur, conforme à la
recommandation).
- Dans la **couche RTS maison** (D215, § 16.4), sur le serveur : **recherche de cible** par la grille hachée de la
  séparation (D292), **à cadence réduite** (~5 Hz, liste de D220) ; **approche** par la file d'ordres (ordre « de
  tâche », comme la récolte et la construction) ; **frappe au contact** (portée de mêlée), **cadence d'attaque par
  type**.
- **Dégâts** = attaque + **bonus de catégorie d'armure** (D21) − **armure** (mêlée ou distance), **1 au moins**. Valeurs
  dans la table des unités (colonnes nouvelles : catégorie d'armure, attaque, cadence, portée, armures, bonus par
  catégorie, rayon d'agression). Pas de moral (D74) ni d'effets en J7.
- **Mort** : PV à 0 → **destruction différée** à la queue du pas (D224), population libérée ; pas de cadavre (D86 :
  Légions, avec les factions).
- **Ordres** : clic droit sur un ennemi (attaquer), **attaque-déplacement** (touche de la grille), **arrêt**,
  **maintien de position** ; le serveur revérifie (D288). **Riposte automatique** dans un rayon d'agression (une unité
  attaquée ou qui voit un ennemi proche s'en prend à lui, sauf en maintien de position).
- Règles pures (dégâts, choix de cible, cadence) dans `Simulation/HOLCombatRules`, tests
  `HeroesOfLiberty.Combat.*`.
- Écartés : **StateTree de Mass** (plus générique, mais coût par entité inconnu à 2 000 unités et rien dans le projet ne
  l'utilise encore) ; **le système d'effets d'abord** (prépare héros et moral, mais retarde le premier combat jouable).

**Tranché — J7-Q5 Réseau du combat : activité sur 4 bits, PV en 16 paliers, cible pendant l'attaque** (choix de
l'utilisateur, conforme à la recommandation ; nuance de l'utilisateur sur l'affichage des barres de vie). Point de
départ : état de 60 bits (62 pour un porteur), S5 à 30,9 Ko/s (~1 Ko/s de marge), 3 bits d'activité pleins (D319).
- **PV** : le serveur garde les PV exacts et envoie un **palier sur 4 bits** (0 à 15, seizièmes des PV max) dans l'état
  de l'unité ; un **changement de palier** compte comme un changement d'action (envoi immédiat), un coup qui ne change
  pas de palier n'envoie rien. Un bit **« PV pleins »** précède le palier, envoyé seulement pour une unité blessée.
- **Activité sur 4 bits** (16 valeurs, 0 à 7 pris) : **attaque** (geste de frappe chez le client) et **mourant** (à
  0 PV, l'unité garde cette activité ~1 s avant sa destruction différée, D224, pour une chute ; pas de cadavre, D86) ;
  6 valeurs restent libres (maintien de position, fuite, siège…).
- **Cible** : seulement quand l'activité vaut « attaque », l'état porte l'**identifiant de la cible** (comme les bits du
  porteur) ; un changement de cible déclenche un envoi. Sert à tourner vers sa cible une unité arrêtée au contact (dès
  J7b) et à déduire la flèche chez le client (J7c.2).
- **Barres de vie** (nuance de l'utilisateur) : **toujours affichées pour les unités sélectionnées** ; hors sélection,
  **affichées seulement pour une unité blessée**, alliée comme ennemie.
- Coût : 60 → **62 bits** pour une unité qui ne se bat pas ; ~66 bits plus l'identifiant pour une unité blessée qui
  attaque. En mêlée, les unités presque arrêtées envoient moins de positions, les PV prennent le relais (~un palier tous
  les 1 ou 2 coups). **S5 en combat (J7b.6) tranche** ; repli : 8 paliers, ou barres lissées chez le client.
- Écartés : **PV exacts** (~10 bits, un envoi par coup : au-delà de 32 Ko/s en mêlée de 2 000) ; **PV aux seules
  unités sélectionnées ou survolées** (envoi propre à chaque client, contre D270 ; pas de barres de vie dans une
  mêlée) ; **4e bit sans « mourant »** (unités qui disparaissent d'un coup) ; **cible jamais envoyée** (orientation
  fausse en mêlée, J7c.2 à reprendre).

**Tranché — J7-Q6 Vérification : S3 en J7b, S4 en J7c** (choix de l'utilisateur, conforme à la recommandation).
**Révisé par la règle des mesures (2026-10-18)** : S2, S3, S4, S5 et S7 se mesurent **tous à la fin de J7**, avant sa
clôture (J7c.4) ; les vérifications de J7a, J7b et J7c se font par `quick_check` et l'essai de l'utilisateur. Les
mentions de stress tests ci-dessous désignent ce qui sera mesuré à la fin du chapitre.
- **J7a** : tests des règles pures (formation, groupes), `quick_check` solo (Caserne posée, bâtie, Lancier et Homme
  d'armes produits, groupes de contrôle, formation, Paysans en rangs) et 2 clients (soldats de l'autre vus en rangs) ;
  **S2 refait** avec la formation (le chemin chaud du mouvement change).
- **J7b** : tests `Combat.*`, `quick_check` solo et 2 clients (combat, mort vue chez les deux) ; **S3 : 1 000 contre
  1 000 en mêlée**, seuil de 3 ms (D218) ; **S5 en combat**.
- **J7c** : **S4 : armées mixtes** (mêlée et tir).
- Chaque partie close par l'**essai de l'utilisateur**, comme J5 et J6.
- Écarté : **S3 dès J7a, à vide** (détecte tôt un souci de densité, mais mesure un scénario incomplet).

**Découpage de J7 en trois parties jouables** : chaque sous-étape compile, se vérifie et se commite seule (après
validation de l'utilisateur).

**J7a — L'armée** : à la fin, le joueur bâtit une Caserne, produit des Lanciers et des Hommes d'armes, les range en
groupes de contrôle et les déplace en rangs ; ses Paysans aussi marchent en rangs.

1. **J7a.1 Données et règles pures** : colonnes de combat dans la table des unités (remplies, pas encore lues), lignes
   `Spearman`, `ManAtArms` ; ligne `Barracks` dans la table des bâtiments (catégorie militaire, `Produces`) ;
   `Simulation/HOLFormationRules` (places en rangs, attribution sans croisement, largeur au glisser) ; tests
   `Formation.*`.
2. **J7a.2 Caserne** : seconde page de la grille du Paysan, pose et chantier (code de J6), production des deux soldats,
   ralliement, F1 ; textes fr / en / ja.
3. **J7a.3 Apparence des soldats** : accessoires par script (lance ; épée et bouclier), couleur du joueur, lignes de la
   table.
4. **J7a.4 Formation** : ordre de groupe en rangs, vitesse du plus lent, glisser du clic droit (orientation, largeur),
   reprise de place après bousculade ; **Paysans en rangs** sur un ordre au sol (S2 refait à la fin de J7).
5. **J7a.5 Groupes de contrôle et HUD** : Ctrl + chiffre et le reste de D30, Ctrl + A, bloc d'un groupe mixte (soldats
   et Paysans) ; textes fr / en / ja.
6. **J7a.6 Vérification** : `quick_check` solo et 2 clients (partie armée), **essai de l'utilisateur** (S2 : à la fin de J7).

**J7b — La mêlée** : à la fin, deux armées se battent, les contres se voient, les morts disparaissent ; S3 tient 3 ms.

1. **J7b.1 Règles pures du combat** : dégâts, bonus, armure, cadence, choix de cible ; tests `Combat.*`.
2. **J7b.2 Processeur de combat** : recherche de cible à ~5 Hz, approche, frappe, riposte, mort et population libérée.
3. **J7b.3 Ordres** : clic droit sur un ennemi, attaque-déplacement, arrêt, maintien de position ; vérifiés par le
   serveur ; repère d'ordre (J4.6) rouge sur une cible.
4. **J7b.4 Réseau** : 4e bit d'activité (« attaque », « mourant »), bit « PV pleins » et PV en 16 paliers, cible
   pendant l'attaque, mort chez le client ; tests `Net.*` étendus (S5 : à la fin de J7).
5. **J7b.5 Présentation** : geste d'attaque (fabriqué comme ceux du Paysan), chute de l'unité mourante, unité tournée
   vers sa cible, barres de vie (toujours dans la sélection, hors sélection si blessée, alliée ou ennemie), PV au bloc
   de sélection.
6. **J7b.6 Vérification** : `quick_check` solo et 2 clients (combat), **essai de l'utilisateur** (S3 et S5 en combat : à la fin de J7).

**J7c — Le tir** : à la fin, des Archers sortent de la Caserne et tirent ; S4 tient.

1. **J7c.1 Archer** : ligne de la table (vitesse : référence **1,25** donnée par l'utilisateur le 2026-10-18, × 1,098214 = **1,373**, soit **584 cm/s** ; portée, projectile), production à la Caserne, accessoire (arc).
2. **J7c.2 Tir** : projectile simulé au serveur (vol à durée fixe, dégât à l'arrivée, peut manquer une cible qui
   bouge), affiché chez le client sans envoi par flèche (déduit de l'activité « attaque » et de la cible). *Lecture à
   confirmer avant le code.*
3. **J7c.3 Vérification** : `quick_check`, **essai de l'utilisateur**.
4. **J7c.4 Mesures de fin de chapitre** (règle des mesures) : S1, S2 (formation), **S3** (mêlée), **S4** (armées mixtes),
   S5 (en combat), S7, au repos ; un seuil dépassé se corrige ici.
5. **J7c.5 Clôture** : décisions J7-Q1 à J7-Q6 (numérotées) au § 21, paragraphe « Parcours J7 » et lignes M4 / M5 au
   § 16.6.

Points à trancher en cours de route :
- **Les Paysans combattent-ils ?** (AoE4 : oui, faiblement ; à défaut, ils fuient ou restent passifs.) À poser avant
  J7b.2.
- **Attaquer un bâtiment** : les bâtiments ont des PV depuis J6 ; mêlée contre bâtiment en J7b, ou avec le siège
  (Bélier, D77) ? À poser avant J7b.3.
- ~~**Le projectile de J7c.2** (lecture ci-dessus) : à confirmer avant J7c.2.~~ → **tranché (J7c-Q1, 2026-10-20)**,
  voir « Plan de J7c » ci-dessous.

**Plan de J7c (questions une par une, 2026-10-20, autre session pendant J7b)** : J7c-Q1 projectile ; Q2 durée du vol ;
Q3 Archer au contact d'une mêlée ; Q4 affichage des flèches chez le client ; Q5 bonus de hauteur (D23) ; Q6 choix de
cible des tireurs.
- **Tranché — J7c-Q1 La flèche touche toujours** (choix de l'utilisateur, B ; recommandation : A, flèche visée sur un
  point qui peut manquer). Le dégât tombe à coup sûr sur la cible à la fin du vol ; la flèche n'est qu'un affichage.
  Pas d'esquive en marchant. *Remplace* « peut manquer une cible qui bouge » de J7c.2. Écartés : **flèche visée sur un
  point** (esquive lisible, liste de flèches au serveur) ; **flèche simulée qui touche n'importe qui** (coût dans S4,
  tir ami frustrant, envoi par flèche). Rappel de la revue de J7b : la mort hors mêlée (`HOLCombatProcessor.cpp`
  ~l. 186) est à compléter pour les flèches.
- **Tranché — J7c-Q2 Vol selon la distance** (choix de l'utilisateur, conforme à la recommandation). Vitesse fixe
  (~25 m/s, indicative : ~0,6 s à 15 m) ; le serveur garde une courte liste de dégâts différés (cible, montant, pas
  d'arrivée) ; le client calcule le même vol depuis les positions, rien de plus envoyé. Cible morte pendant le vol :
  flèche perdue ; dégât parti même si le tireur recule ensuite. Écartés : **durée fixe** (faux de près) ; **dégât au
  lâcher** (barre qui baisse avant l'arrivée de la flèche).
- **Tranché — J7c-Q3 Tir à bout portant, sans portée minimale** (choix de l'utilisateur, conforme à la recommandation ;
  règle d'AoE4). L'Archer au contact d'une mêlée continue de tirer ; le repli se fait à la main. Écartés : **passage en
  mêlée** (second profil, geste de plus, effet nul sur les contres) ; **repli automatique** (retire la micro, fuit quand
  le joueur ne le veut pas, coût dans S4).
- **Tranché — J7c-Q4 Flèches déduites chez le client** (choix de l'utilisateur, conforme à la recommandation). Lâcher au
  même moment du geste de tir que le serveur (comme le coup de mêlée à 50 %), vol à la vitesse de Q2 en cloche légère
  vers la cible suivie (elle touche toujours, Q1), disparition à l'impact ; un maillage instancié pour toutes les
  flèches (~600 en vol dans S4), rien sur le serveur dédié ; 0 bit de plus. Écartés : **événement « tir » envoyé**
  (~600 envois/s dans S4, au-delà de 32 Ko/s) ; **pas de flèche en J7c** (tir illisible à 15 m).
- **Tranché — J7c-Q5 Bonus de hauteur (D23) plus tard** (choix de l'utilisateur, conforme à la recommandation) : avec
  le relief ou les remparts (D24), rien en J7c (cartes plates, aucune colonne). Écartés : **règle pure sans terrain**
  (code que l'essai ne juge pas) ; **règle et butte d'essai sur `Plain`** (carte, pentes, environnement des stress
  tests touchés, hors du but de J7c).
- **Tranché — J7c-Q6 Cible des tireurs : la règle de la mêlée, sans `MaxAttackers`** (choix de l'utilisateur, conforme
  à la recommandation). `ChooseTarget` inchangé pour la mêlée ; pour un tireur, pas de passage à un ennemi « moins
  entouré » : le plus proche, cible gardée tant qu'à portée (tir concentré, flèches perdues sur une cible morte en vol,
  comme dans AoE4). Écartés : **limite comprise** (Archer qui part plus loin sans raison visible) ; **tir
  « intelligent »** (bonus d'abord, dégâts en vol évités : micro automatisée, coût dans S4).
- **Plan de J7c complet** (Q1 à Q6). J7c.2 se lit désormais : dégât différé selon la distance, toujours sur la cible,
  flèche déduite chez le client ; mort par flèche à brancher sur le chemin « mourant » complet.
- **Valeurs** (PV, attaque, armures, coûts, durées) : indicatives, inspirées d'AoE4, à régler en essai ; elles iront à
  la fiche de valeurs (§ 22).

Risques connus :
- **S3** : 2 000 unités au contact, la séparation de D292 et l'évitement à l'arrêt (déjà ~6 ms un pas sur 6 dans S7)
  deviennent le pire cas ; la recherche de cible s'y ajoute. Marge de S2 déjà mince (0,12 ms au repos).
- **S5** : marge de ~1 Ko/s ; le combat change souvent l'action et les PV (plus d'envois).
- **Formation des Paysans** : ne doit pas gêner les allers-retours de récolte (D309), qui ne passent pas par l'ordre au
  sol ; à vérifier dans S7.

## Décisions du chantier (à reporter au § 21 du GDD)

| # | Date | Sujet | Décision |
|---|---|---|---|
| D253 | 2026-10-13 | Technique — Périmètre de J1 | **Caméra, emplacement de départ et réglages de la caméra** (choix de l'utilisateur, conforme à la recommandation) : le joueur arrive dans la partie à son emplacement de départ et déplace une caméra RTS ; dans Réglages > Commandes, **vitesse de la caméra** et **défilement au bord de l'écran** sont dégrisés et agissent ; les touches de la caméra passent par Enhanced Input (remappables plus tard). « Changer les touches » reste grisé : le remappage viendra quand il y aura assez d'actions, chaque étape déclarant les siennes (sélection en J2, groupes de contrôle…). Écartés : **caméra et emplacement seuls** (onglet entièrement grisé, valeurs fixes) et **remappage complet de D30 dès maintenant** (actions sans effet, à redessiner quand leurs systèmes arriveront). |
| D254 | 2026-10-13 | Technique — Comportement de la caméra | **Façon AoE4** (choix de l'utilisateur, conforme à la recommandation) : **pas de rotation**, plongée fixe (~55°), hauteur de jeu par défaut 30-40 m (la vue de jeu de D236), **zoom à la molette** entre ~15 et ~70 m ; déplacement aux **flèches** (ZQSD est pris par la grille de commandes de D30), au **bord de l'écran** et en **glissant au clic molette** ; caméra **bornée à la carte** ; au début de la partie, au-dessus de son **centre principal** (emplacement de départ en attendant J2). Valeurs indicatives, à régler en test. Motif : lisible et identique pour tous (compétitif, cartes symétriques, mini-carte toujours orientée pareil) ; le dézoom maximal reste sous la vue large mesurée (~100 m, D237), donc dans les performances déjà tenues. Écartés : **rotation façon BFME** (mini-carte qui suit l'orientation, formations à lire sous tous les angles) et **caméra cinématique** qui s'aplatit en zoomant (horizon visible, coût de rendu hors des mesures, lisibilité moindre). |
| D255 | 2026-10-13 | Technique — Fabrication de la caméra | **Caméra locale, sans pion** (choix de l'utilisateur, conforme à la recommandation) : le serveur ne crée aucun pion ; le contrôleur de partie `AHOLPlayerController` (C++, curseur visible) fait apparaître **chez le joueur seul** un acteur caméra (bras et caméra) qu'il pilote ; rien n'est envoyé au serveur quand la caméra bouge (ce que chacun reçoit dépendra du brouillard de guerre, M7, pas de sa caméra). Actions Enhanced Input de la partie (déplacement, zoom, glisser) dans un contexte `IMC_Game` créé par script, comme `IMC_UI` ; vitesse et défilement au bord enregistrés dans `UHOLGameUserSettings` ; le stress test place la caméra pour ses vues de mesure (D236, D237) par une fonction dédiée, hors des limites de jeu. Écartés : **pion caméra répliqué** (un pion par joueur sur le serveur, mouvements envoyés pour rien, à-coups possibles avec la latence) et **gestionnaire de caméra seul** (moins courant, sans composants réutilisables, plus dur à régler dans l'éditeur). |
| D256 | 2026-10-13 | Technique — Emplacements de départ et limites de la carte | **Acteurs posés dans la carte, sur une nouvelle carte** (choix de l'utilisateur, conforme à la recommandation ; l'utilisateur autorise l'assistant à créer des cartes) : un acteur **emplacement de départ** dérivé du `PlayerStart` d'Unreal porte son numéro (0 à 7, D244 : l'emplacement est la position de départ) ; un acteur **limites de la carte** borne la caméra (puis la mini-carte et la construction). Une **carte de partie neuve**, créée par script comme `MainMenu` : sol plat d'environ 400 m × 400 m couvert par le navmesh, lumière et ciel, 8 emplacements symétriques sur un cercle dans un ordre qui écarte les premiers (en 1 contre 1, face à face), les limites ; elle devient la carte proposée par « Nouvelle partie ». `BattleMap` (carte d'ouverture de l'éditeur) et `StressTest` ne changent pas. Les vraies cartes, faites à la main, placeront ces acteurs de même. Écartés : **emplacements calculés par le code** (ne tient pas pour des cartes dessinées avec leur terrain et leurs ressources), **`PlayerStart` étiquetés** (numéro tapé sans contrôle, pas de place pour des données propres à l'emplacement) et **modifier `BattleMap`**. |
| D257 | 2026-10-13 | Technique — Vérification de J1 | **Tests, bout en bout et stress tests** (choix de l'utilisateur, conforme à la recommandation) : tests d'automatisation sur la logique (caméra bornée aux limites, zoom entre ses bornes, emplacements symétriques et dans l'ordre, réglages vitesse et bord enregistrés puis relus) ; bout en bout par script sur le deuxième écran (partie lancée depuis le menu à 2 puis 8 joueurs, caméra au-dessus du bon emplacement, flèches et molette envoyées comme en D246, captures ; défilement au bord essayé avec une position de souris simulée dans le jeu, jamais en pilotant le bureau) ; S1 et S2 courts pour vérifier que les vues de mesure n'ont pas bougé ; puis une session d'essai de l'utilisateur. Écarté : essai manuel seul (rien de rejouable). |
| D259 | 2026-10-13 | Technique — Périmètre de J2 | **Centre principal et sélection au clic** (choix de l'utilisateur, conforme à la recommandation) : au début de la partie, un **centre principal** sur chaque emplacement **occupé** (joueur humain ou IA ; rien sur un emplacement fermé), à la **couleur de son joueur** (D244) ; il remplace le repère de l'emplacement, et les repères d'échelle de J1 partent. **Clic gauche** sur un bâtiment : il est sélectionné, le sien comme celui d'un adversaire (pour le regarder) ; **clic dans le vide** ou **Échap** : plus rien de sélectionné. Retour visuel : **cercle au sol** à la couleur du propriétaire sous le bâtiment sélectionné, **surlignage léger au survol**. Pas de panneau ni de bloc de sélection (J3, HUD de D201). *Précisé à l'essai (2026-10-14, choix de l'utilisateur : « pas besoin »)* : **ni cercle ni survol** ; la sélection n'a pas de retour visuel sur le bâtiment, le panneau de J3 la montrera. Écartés : **bloc de sélection provisoire du HUD** (anticipe un HUD redessiné en J3) et **sélection par cadre et multiple** (sans objet sans unités, prévue en M4). |
| D260 | 2026-10-13 | Technique — Fabrication des bâtiments | **Acteur répliqué, valeurs en table, GAS plus tard** (choix de l'utilisateur, conforme à la recommandation ; les bâtiments sont des acteurs classiques, § 16.4) : une **classe C++ commune** à tous les bâtiments, **répliquée**, que le **serveur** fait apparaître au début de la partie sur chaque emplacement occupé (D259) ; son **propriétaire** est le numéro du joueur (un octet, comme `OwnerPlayer` des unités, D233) ; ses valeurs (nom, emprise, points de vie, maillage en référence souple) viennent d'une **table des types de bâtiment** fabriquée depuis un **CSV** par script, comme les unités (D221, D227). Le composant **GAS** et ses attributs viendront quand un bâtiment aura une capacité ou prendra des dégâts (M5). Écartés : **GAS dès J2** (code et réplication pour des valeurs que rien ne lit avant M5) et **un Blueprint par bâtiment** (valeurs dispersées au lieu d'une table à équilibrer). |
| D261 | 2026-10-13 | Technique — Apparence provisoire du centre principal | **Bloc simple coloré** (choix de l'utilisateur, conforme à la recommandation) : volumes de base du moteur, un bloc et une tour centrale, d'environ **15 × 15 m au sol et 10 m de haut** (valeurs de la table, D260), avec un **toit ou bandeau à la couleur du joueur**, tirée de la même palette que les unités (`HOLPlayerColors`, D233, D244). *Précisé à l'essai (remarques de l'utilisateur sur les captures)* : les pièces à la couleur du joueur sont **non éclairées** (la même couleur sur toutes les faces, sans côté sombre ; la pierre reste éclairée) ; la tour porte un **toit plat** un peu plus large qu'elle (un cône éclairé montrait un côté sombre, non éclairé il devenait un disque) ; le dessus du bloc est d'un **ton plus sombre** de la couleur du joueur, pour que le toit s'en détache. Lisible de loin, rapide, évidemment provisoire ; le vrai modèle se branchera par la table. Écartés : **assemblage des pièces de château de FantasyVillage** (plus long, pièces qui s'emboîtent mal, toujours provisoire) et **modèle à télécharger sur Fab** (dépend d'un choix et d'une licence). |
| D262 | 2026-10-13 | Technique — Sélection | **Chez le client seul, survol par teinte** (choix de l'utilisateur, conforme à la recommandation) : l'état de sélection vit dans le contrôleur du joueur ; le serveur ne le connaît pas, les futurs ordres lui porteront la liste visée par la file de commandes (comme AoE). On vise par un **rayon sous le curseur** sur un **canal de collision propre à la sélection**. Sélectionné : **cercle au sol** à la couleur du propriétaire ; survolé : le bâtiment **s'éclaircit** légèrement (paramètre du matériau, sans coût de rendu). *Précisé à l'essai (2026-10-14)* : cercle et éclaircissement **retirés** (D259) ; restent l'état chez le client, le rayon et le canal. Écartés : **contour en post-traitement** (passe de rendu en plus, à mesurer quand il s'étendra aux unités) et **sélection connue du serveur** (aller-retour réseau à chaque clic, sans usage tant que les alliés ne voient pas la sélection des autres). |
| D263 | 2026-10-13 | Technique — Angelscript | **Pas d'Angelscript : tout en C++** (choix de l'utilisateur, après avoir d'abord retenu un hybride puis demandé ce qu'il permettait) : le moteur du projet est celui d'Angelscript (Hazelight), mais le projet n'en écrit pas ; le gameplay reste en **C++** comme le cœur (D242, D255, D260), les valeurs en tables et les outils d'éditeur en Python. Le moteur du **serveur dédié** (D231) reste le build **Epic 5.8.3** en cours. Motif : un seul langage, aucun moteur supplémentaire à compiler pour le serveur, aucune dépendance au suivi des versions par Hazelight. Écartés : **hybride** (cœur C++, gameplay en script : rechargement à chaud, mais moteur du serveur à recompiler depuis les sources de Hazelight et deux langages) et **décider à M2.5**. |
| D264 | 2026-10-13 | Technique — Vérification de J2 | **Comme J1** (choix de l'utilisateur, conforme à la recommandation ; l'essai Angelscript prévu d'abord tombe avec D263) : **tests d'automatisation** (centres sur les seuls emplacements occupés, couleur de la palette du joueur, valeurs lues dans la table, rayon de sélection qui touche le centre) ; **bout en bout par script** à 2 et 8 joueurs (captures ; clic simulé sur son centre, sur un centre adverse, dans le vide, puis Échap) ; **essai de l'utilisateur**. S1 et S2 ne sont pas concernés (cartes sans emplacements, aucun centre). Écartés : **sans bout en bout** (sélection au clic non rejouable) et **essai de l'utilisateur seul**. Clôt le plan de J2 (D259 à D264). |
| D268 | 2026-10-14 | Technique — Périmètre de M2.5 | **Partie à deux et S5, lancés par script** (choix de l'utilisateur, conforme à la recommandation) : à la fin de M2.5, un **serveur dédié** (cible `HeroesOfLibertyServer`, D231) démarre sur `Plain` avec une configuration de partie en JSON (`-MatchConfig=`, D243) ; **deux clients** le rejoignent par adresse (`open <adresse>`), chacun voit les deux centres principaux à la bonne couleur, sa caméra arrive à son emplacement, la sélection marche. **S5** : sur la carte du stress test, le serveur fait marcher 2 000 unités, les deux clients les reçoivent et les affichent interpolées ; on mesure la bande passante par client (≤ 32 Ko/s, D218) et l'envoi du serveur. Un **script** lance serveur et clients sur le deuxième écran. Le menu ne change pas, le solo non plus. Écartés : **« Héberger » et « Rejoindre » dans le menu** (écran provisoire que le salon de M6 remplacera, à traduire et tester) et **S5 seul** (le parcours du joueur ne serait essayé en réseau qu'en J3, contre D252). |
| D269 | 2026-10-14 | Technique — Données envoyées pour les unités | **Envoi seulement quand le client se tromperait** (choix de l'utilisateur, conforme à la recommandation) : le serveur envoie d'une unité sa **position et sa vitesse réduites** (à ~10 cm près, environ 8 octets avec l'identifiant) ; le client **prolonge** le mouvement en ligne droite ; le serveur garde ce qu'il a envoyé et ne renvoie l'unité que si la position prolongée s'écarte de plus d'un **seuil** (~25 cm, réglable) ou si son **action** change ; un **rappel lent** (chaque unité au moins toutes les ~2 s) rattrape les pertes. Estimation : ~1,5 envoi par unité et par seconde, ~24 Ko/s à 2 000 unités ; S5 tranche, le seuil se règle. Motif : le coût suit ce qui change vraiment, place laissée aux états de M5. Écartés : **toutes les unités à cadence fixe, en écarts** (~40 Ko/s à 10 Hz, ~20 Ko/s à 5 Hz mais mouvement plus haché et plus en retard, payé même à l'arrêt) et **priorité selon la caméra du client** (revient sur D255, pire cas inchangé en vue large ; gardée en réserve si l'envoi au seuil ne tient pas). Écarté d'avance : simulation identique partout et ordres seuls (calcul de Mass non reproductible, contraire à D29). |
| D270 | 2026-10-14 | Technique — Transport des données des unités | **Tableau rapide maison sur un seul acteur partagé** (choix de l'utilisateur, conforme à la recommandation) : un acteur répliqué, toujours pertinent, porte un **tableau rapide** (`FFastArraySerializer`, pris en charge par Iris) des unités : identifiant, position et vitesse réduites, action, numéro du pas (le client prolonge depuis le bon instant) ; le serveur ne **marque** une unité modifiée que selon D269, Iris n'envoie que celles-là et garantit que le client finit par avoir le dernier état ; **format d'élément maison** (nombre de bits exact) ; un seul tableau pour tous, préparé **une fois** quel que soit le nombre de clients (seuil d'envoi du serveur, D231). Risque mesuré en S5 : surcoût d'Iris par élément d'un tableau de 2 000 ; **repli** : paquets maison non garantis. Limite connue : avec le brouillard (M7), un tableau par équipe ou par client. Écartés : **paquets maison non garantis, un acteur par client** (découpage, ordre et reprise à écrire, préparation par client ; gardé en repli) et **plugin MassReplication** (expérimental, une bulle par client, pertinence par la vue du joueur contre D255, logique d'envoi autre que D269). |
| D271 | 2026-10-14 | Technique — Affichage des unités chez le client | **Instant présent estimé, corrections lissées** (choix de l'utilisateur, conforme à la recommandation) : chez le client, chaque unité reçue devient une **entité Mass d'affichage** (identifiant, type, propriétaire, dernier état reçu, position affichée), créée, mise à jour et retirée au fil du tableau rapide (D270) ; tout l'affichage de M2 sert tel quel (instances, couleur D233, animation d'après la vitesse affichée D234) ; pas de simulation chez le client. Le client estime l'horloge du serveur (numéro du pas) et affiche chaque unité **là où le serveur l'a sans doute à cet instant** (dernier état + vitesse × temps écoulé) ; une correction est **lissée en ~0,2 s** (écart au plus ~25 cm, D269). Aucun retard ajouté (ordres de M4 aussi vifs que possible). Écartés : **retard d'affichage de ~100 ms** (moins de glissements mais tout en retard, ordres plus lents à partir) et **correction sans lissage** (sauts jusqu'à ~25 cm, scintillement dans la mêlée, animation qui hoquette). |
| D272 | 2026-10-14 | Technique — Le solo | **Le solo reste autonome** (choix de l'utilisateur, conforme à la recommandation) : une partie lancée du menu tourne en un seul processus, simulation et affichage ensemble, sans réseau (conforme à D268, menu inchangé) ; l'affichage reste **un seul code**, nourri en solo par l'état des entités de la simulation, chez un client par celui des entités d'affichage que la réception remplit avec les mêmes données (position, vitesse) ; le chemin réseau est vérifié par la **partie à 2 clients** de fin d'étape (D216), qui entre dans `quick_check.ps1`. Écartés : **serveur dédié local même en solo** (le menu lancerait et rejoindrait un processus serveur, contre D268 ; démarrage plus lent, deux processus, un bug réseau bloque le solo) et **affichage toujours par le tableau rapide en boucle locale** (entités en double en solo, stress test changé de nature, références S1 et S2 à refaire). |
| D273 | 2026-10-14 | Technique — Vérification de M2.5 | **Tests, S5 avec l'écart affiché, partie à 2 clients dans la vérification rapide, essai** (choix de l'utilisateur, conforme à la recommandation) : **tests d'automatisation** des règles pures (réduction des positions et vitesses à ~10 cm près, règle d'envoi de D269 — seuil, changement d'action, rappel lent —, prolongement et lissage de D271) ; **S5** lancé par script (serveur dédié sur la carte du stress test, nuée de S2, 2 000 unités, graine fixe, 2 clients sur le deuxième écran), rapport : octets reçus par client et par seconde (≤ 32 Ko/s), envoi du serveur, envois par unité et par seconde, temps de pas du serveur, images par seconde des clients, et **écart affiché** (positions d'un échantillon d'unités notées au même pas par le serveur et les clients, comparées : moins de ~25 cm attendus) ; **partie à 2 clients dans `quick_check.ps1`** (serveur dédié sur `Plain`, chaque caméra au-dessus de son emplacement, les deux centres à la bonne couleur des deux côtés, clics de sélection), vérification de fin d'étape de D216 et D272 ; **essai de l'utilisateur**, les deux clients côte à côte sur `Plain` puis sur S5. Écartés : **sans l'écart affiché** (justesse de l'affichage jugée à l'œil seulement) et **S5 et essai seuls** (partie à 2 clients non rejouable, contre D216). Clôt le plan de M2.5 (D268 à D273). |
| D274 | 2026-10-14 | Technique — Seuil d'envoi et cadence des unités (S5) | **35 cm et 10 mises à jour par seconde** (choix de l'utilisateur, conforme à la recommandation ; D269 laissait le seuil « réglable, S5 tranche », D217 la cadence entre 10 et 20 Hz) : à 2 000 unités en marche, **30,3 Ko/s par client** (sous les 32 Ko/s de D218) et écart affiché moyen **~19 cm** (sous les ~25 cm de D273) ; à 25 cm, 34,1 Ko/s. Les morceaux d'emplacements sont des **acteurs répliqués** de 63 emplacements (un sous-objet par morceau faisait passer toute la mise à jour par le chemin des objets énormes d'Iris). Reste ~8 Ko/s d'en-têtes non expliqués, à rechercher par une trace réseau d'Iris quand les états de M5 s'ajouteront. Écartés : **chercher d'abord ces en-têtes** (garder 25-30 cm ; travail plus long, gain incertain) et **30 cm avec un état réduit** (passage de justesse, moins de marge, moins robuste aux paquets en retard). |
| D275 | 2026-10-14 | Technique — Serveur de la vérification rapide | **L'éditeur en mode serveur dédié** (choix de l'utilisateur, conforme à la recommandation) : dans la partie « net » de `quick_check.ps1`, le serveur dédié est l'éditeur lancé en `-server` (contenu non cuit, toujours le code du moment, aucune cuisson) ; le serveur cuit (cible `HeroesOfLibertyServer`, D231) reste pour S5 et les essais de l'utilisateur (`net_match.ps1`). Motif : J3 change le code sans cesse, la cuisson prend ~2 min et la vérification rapide doit rester sous la minute. Risque accepté : un écart propre au serveur cuit ne se voit qu'en S5. Écartés : **serveur cuit avec contrôle d'âge** (échec « recuire » presque à chaque fois) et **serveur cuit recuit automatiquement** (~2 min 30 après chaque changement de code). |
| D280 | 2026-10-14 | Technique — Périmètre de J3 | **Production avec coût, en plusieurs sous-étapes** (choix de l'utilisateur : option A, à découper en sous-étapes) : à la fin de J3, le HUD de D201 montre en haut les **4 ressources** (stock de départ provisoire) et la **population** (base donnée par le centre, plafond) ; en bas au centre, le **bloc de sélection** (nom, points de vie, propriétaire du bâtiment sélectionné) ; en bas à droite, la **grille de commandes** (D30) avec **« Paysan »** (clic ou touche `A`). Produire **débite le coût** et prend une place de population ; la **file de production** montre sa progression et s'annule au clic (remboursement) ; le Paysan **apparaît près du centre** et reste immobile (ses ordres en J4). Tout passe par la **file de commandes**, donc par le **serveur**, qui vérifie coût et population ; en solo comme à 2 clients. Les sous-étapes sont fixées à la fin du plan (Q2 à Q5). Écartés : **production gratuite** (ressources et population en J5 ; plus rapide, mais produire ne serait pas encore un choix) et **A avec point de ralliement et Paysans de départ** (le ralliement est un ordre de déplacement, J4). |
| D281 | 2026-10-14 | Technique — Fabrication du HUD | **Blocs utiles seuls, à leur place définitive** (choix de l'utilisateur, conforme à la recommandation) : un **écran de jeu Common UI en C++** dans le module d'interface (D241, D242, D247), avec la mise en page de D201 ; seuls les blocs dont J3 a besoin sont dessinés : **barre du haut** (4 ressources, population), **bloc de sélection** en bas au centre, **grille de commandes** 3 × 4 en bas à droite (touches de D30 affichées, seule la case « Paysan » active) et **file de production** au-dessus de la grille. Mini-carte, bloc du héros et emplacement de faction viendront avec leurs systèmes, à leur place déjà prévue. Le HUD **lit** l'état de la partie, ne le modifie jamais : ses clics passent par le contrôleur, puis la file de commandes (D252). Écartés : **squelette complet** (blocs morts à l'écran pendant des mois) et **HUD provisoire de débogage** (à refaire, contraire à D242). |
| D282 | 2026-10-14 | Technique — Production côté serveur | **Le serveur seul décide, façon AoE** (choix de l'utilisateur, conforme à la recommandation) : un clic du HUD part au serveur par un **appel du contrôleur**, entre dans la **file de commandes** et s'applique au **pas suivant** ; le serveur vérifie le **propriétaire**, le **coût** et la **longueur de la file** (valeur en table). Le coût est **débité à la mise en file**, **remboursé à l'annulation** ; la **population** est vérifiée **au début de chaque production** : pleine, la file attend (comme AoE). **Ressources et population** vivent dans le `PlayerState`, répliquées **au seul propriétaire** (D203) ; la **file** vit sur le **bâtiment**, répliquée au propriétaire (types et pas de début), le client en calcule lui-même la progression. Écartés : **prédiction chez le client** (retour en arrière à gérer pour ~50-150 ms gagnées sur un ordre de production) et **GAS dès maintenant** (lourd pour une file, D260 le réserve à M5). |
| D283 | 2026-10-14 | Technique — Le Paysan | **Mannequin, valeurs en tables** (choix de l'utilisateur, conforme à la recommandation) : une ligne **`Peasant`** dans `UnitTypes.csv`, avec le **mannequin de M2** à la couleur du joueur (le vrai modèle se branchera par la table) ; il **apparaît devant le centre**, du côté du milieu de la carte, **en arc** pour ne pas s'empiler, et reste **immobile** (ordres en J4). *Précisé à l'essai (2026-10-14, choix de l'utilisateur : « en bas du forum si pas de rally point »)* : il naît **en bas du centre vu de la caméra** (−X : la caméra ne tourne pas et regarde vers +X pour tous, D254), jamais caché derrière ; le point de ralliement de J4 le remplacera. Valeurs provisoires : **50 nourriture, 20 s** ; **200 de chaque ressource** au départ ; **population de base du centre 10**, plafond **175** (D03) ; **file de 5** au plus. Coût, temps et population de l'unité dans la table des unités ; population de base, file et unités produites dans la table des bâtiments ; stock de départ dans les réglages de partie. Écarté : **apparence distincte dès maintenant** (aucun soldat sur `Plain` dont le distinguer, temps passé sur du provisoire). |
| D284 | 2026-10-14 | Technique — Vérification de J3 | **Tests, solo et 2 clients** (choix de l'utilisateur, conforme à la recommandation) : **tests d'automatisation** des règles de production (débit, refus faute de ressources, remboursement, file pleine, attente population pleine, progression) ; dans **`quick_check.ps1`**, une partie **solo** (clic sur son centre, touche `A` plusieurs fois, Paysans apparus, ressources baissées, annulation remboursée, captures du HUD en fr / en / ja) et, une fois M2.5.6 en place, la **partie à 2 clients** (le Paysan apparaît chez les deux, les ressources ne sont vues que par leur propriétaire, D216) ; **essai de l'utilisateur** ; **S1 et S2 courts** à la clôture (un type d'unité ajouté). Écarté : **solo seul, réseau essayé à la main à la fin** (contraire à D216). Clôt le plan de J3 (D280 à D284). |
| D285 | 2026-10-14 | Technique — Périmètre de J4 | **Déplacer ses Paysans, seuls ou à plusieurs** (choix de l'utilisateur, conforme à la recommandation) : **sélection d'unités** au clic, au **cadre** et avec **Maj** pour ajouter ou retirer (ses unités seules à plusieurs ; une unité adverse seule, pour la regarder) ; **clic droit** sur le sol : l'ordre part au serveur par la file de commandes, les unités suivent un **chemin sur le navmesh** qui contourne les centres et quelques **obstacles posés sur `Plain`** ; un **groupe arrive réparti** autour du point, sans formation en rangs ; **point de ralliement** du centre (centre sélectionné + clic droit), où le Paysan produit se rend ; le **bloc de sélection** montre une unité ou un groupe. Retour visuel des unités sélectionnées et repère du clic droit tranchés à part (Q4). Écartés : **un Paysan à la fois** (ni cadre ni groupe : à reprendre dès J5, empilement à l'arrivée non réglé) et **commandes d'armée façon AoE4** (formations, ordres enchaînés à Maj, groupes de contrôle, `.` : sans objet sans soldats, étape longue avant d'être jouable ; pour l'étape « armée », M4). |
| D286 | 2026-10-14 | Technique — Chemins | **Un chemin par paquet, couloir partagé** (plan validé par l'utilisateur, conforme à la recommandation) : au pas où l'ordre s'applique, le serveur coupe les unités visées en **paquets** d'unités proches (~15 m) et fait **une requête de chemin par paquet** sur le navmesh, avec un **budget de requêtes par pas** (les suivantes attendent, comme les naissances de D224) ; le chemin devient un **couloir** (`FNavCorridor`, ~6 m) **partagé** par les unités du paquet, que chacune suit par le chemin court de Mass (`MassNavMeshPathFollowProcessor`) ; au bout, chaque unité rejoint sa place (D287). Navmesh **dynamique à modificateurs seuls** : chaque bâtiment porte une zone interdite sur son emprise (table, D260), refaite au serveur ; le client n'a pas besoin du navmesh. `Plain` reçoit des **obstacles fixes** (rochers, bosquet). Destination ramenée au navmesh (D228), au point atteignable le plus proche. Écartés : **un chemin par unité** (175 requêtes pour un grand ordre, S2 très cher au départ) et **champ de flux** (gros chantier, gardé si les couloirs ne tiennent pas en M4-M5). |
| D287 | 2026-10-14 | Technique — Ordre de groupe | **Arrivée en tas serré, sans formation** (plan validé par l'utilisateur, conforme à la recommandation) : une unité seule va au point exact ; un groupe reçoit des **places** en nid d'abeille autour du point (écart = diamètre du corps + ~20 %), orientées selon la marche, ramenées au navmesh (une place dans un obstacle passe à la suivante libre), **attribuées en gardant l'ordre relatif** des unités pour que leurs chemins ne se croisent pas (D230) ; chaque unité à sa vitesse. Règles pures dans `Simulation/HOLGroupMoveRules`. Formations (rangs, orientation au glisser, vitesse du plus lent) pour l'étape « armée » (M4). Écartés : **toutes au même point** (empilement, évitement à l'arrêt sans fin) et **formation en rangs dès J4** (sans objet pour des Paysans). |
| D288 | 2026-10-14 | Technique — Sélection des unités et ordres | **Chez le client, par projection à l'écran** (plan validé par l'utilisateur, conforme à la recommandation) : la sélection reste dans le contrôleur (D262), liste d'identifiants d'unités (D223) ou un bâtiment ; les unités, sans collision, sont visées en **projetant à l'écran** leur position affichée (simulation en solo, entités d'affichage chez un client) ; clic : l'unité sous le curseur **avant** un bâtiment ; **cadre** au glisser (~6 pixels) : ses unités ; **Maj** ajoute ou retire ; une unité adverse seule ; Échap ou vide : rien. **Clic droit** : appel serveur (identifiants + point), le **serveur vérifie le propriétaire** de chaque unité, puis file de commandes (D252) ; centre sélectionné + clic droit : **point de ralliement** (sur le bâtiment, répliqué au propriétaire), où le Paysan produit reçoit un ordre à sa naissance. Retour visuel (**choix de l'utilisateur**) : **cercle au sol sous chaque unité sélectionnée** à la couleur de son propriétaire, et **carré au sol sous le bâtiment sélectionné** (*revient sur D259 / D262*, qui avaient retiré le cercle du bâtiment) ; rectangle du cadre au glisser, repère bref (~0,5 s) au point du clic droit, fanion au ralliement quand le centre est sélectionné. Bloc de sélection : une unité (nom, propriétaire, points de vie) ou un groupe (nom × nombre par type), fr / en / ja. Écartés : **rien sur les unités** (10 Paysans identiques, le HUD ne dit pas lesquels sont pris), **unités éclaircies**, **sélection par collision** (contraire à D29) et **sélection connue du serveur** (D262). |
| D289 | 2026-10-14 | Technique — Vérification de J4 | **Tests, solo et 2 clients, S2 avec chemins** (plan validé par l'utilisateur, conforme à la recommandation) : tests d'automatisation (places, attribution, paquets, projection et cadre) ; dans `quick_check.ps1`, partie **solo** (ralliement, 3 Paysans qui s'y rendent, cadre, clic droit derrière un rocher : contournement, arrivée sans recouvrement, rien dans un obstacle ; Maj, Échap, captures) et partie **à 2 clients** (chacun voit le Paysan de l'autre arriver, à < 50 cm ; un ordre sur l'unité de l'autre ignoré) ; **S2 rendu exigeant** par les chemins (promesse de D229, seuil 3 ms), S1 inchangé, **S5 court** ; essai de l'utilisateur. Sous-étapes J4.1 à J4.8. Clôt le plan de J4 (D285 à D289). |
| D290 | 2026-10-14 | Technique — Essai de J4.5 / J4.6 | **Retouches de l'utilisateur** : cercles sous les unités et carré sous le bâtiment sélectionnés en **blanc neutre** (*précise D288*, qui les voulait à la couleur du propriétaire ; le fanion garde la couleur du joueur) ; repère du clic droit **2× plus petit et 2× plus bref** (0,9 → 0,25 m de rayon en 0,25 s) ; **Paysan à 6 m/s** (choix de l'utilisateur parmi 4,5 / 6 / 9 m/s, contre la recommandation de 4,5 ; était 3 m/s), marche jouée ×2 (2,44) pour que les pieds ne glissent pas ; **accélération proportionnelle à la vitesse** (pleine vitesse en 1,2 s quel que soit le type ; l'infanterie du stress test garde ses 250 cm/s²). *2e essai* : **arrêtée ou pleine vitesse, rien entre les deux** (`UHOLFullSpeedProcessor` : vitesse d'un ordre toujours pleine, dans la direction du pilotage et de l'évitement ; dans les 3 derniers mètres droit sur la place ; dernier pas posé exactement sur elle ; une unité à l'arrêt ne bouge plus du tout, les marcheurs la contournent) ; arrivée : vitesse annulée, point de stationnement de Mass mis là où l'unité s'arrête, tolérance d'arrivée 50 → 5 cm (fin du dépassement puis du lent retour sur la place). Vaut pour tous les types, stress test compris (S1 / S2 en J4.7). |
| D291 | 2026-10-14 | Technique — Triches pour les essais | **Commandes console, en développement seulement** (choix de l'utilisateur : « pas dans la build ») : `hol.Cheat.Resources [N]`, `hol.Cheat.Food / Wood / Stone / Gold [N]` (N peut être négatif, stock jamais sous 0), `hol.Cheat.FastBuild [1/0]` (production en un pas pour tous), `hol.Cheat.Population [N]` (places ajoutées au plafond, 175 au plus) ; envoyées au serveur par le contrôleur, donc valables en solo et à 2 clients ; **aucune commande enregistrée et appels refusés par le serveur dans une build Shipping** (`UE_BUILD_SHIPPING`). Hors file de commandes : outil de développement, pas une action de jeu. Écartés : **codes tapés en jeu façon AoE** (boîte de saisie, textes traduits) et **les deux**. |
| D292 | 2026-10-14 | Technique — Aucun corps n'en traverse un autre | **Séparation dure après le mouvement** (3e essai de l'utilisateur : « un paysan traverse un paysan » ; *précise D230*, qui laissait deux unités qui se croisent se traverser jusqu'à M5) : à chaque pas, une fois les unités déplacées, tout recouvrement auquel prend part une unité en marche est défait sur place, **moitié chacune** : un marcheur **pousse** une unité à l'arrêt hors de son chemin (comme dans AoE ; elle ne bouge que de ce qu'il faut, sans glisser ensuite, D290), deux marcheurs s'écartent ; une unité poussée regarde à son tour ses voisines ; deux tours par pas ; les unités à l'arrêt laissées seules ne coûtent rien (`UHOLUnitSeparationProcessor`, table de cases hachée, `hol.Units.Separation 0/1/2`). L'évitement de Mass reste pour dévier tôt. Avec : places en double supprimées (plus de candidats), jamais à reculons à pleine vitesse (de côté au plus), blocage compté seulement à moins de 5 m de sa place. Coût mesuré sur machine chargée : ~0,8 ms par pas à 2 000 unités en marche (S2), **à remesurer au repos en J4.7** (seuil de 3 ms). Écartés : **capsules de collision** (contraire à D29) et **évitement seul** (tourne trop tard à 6 m/s). |
| D293 | 2026-10-14 | Technique — Contourner avant de bousculer | **Regard en avant à chaque pas** (4e essai de l'utilisateur : « un paysan ne traverse plus un paysan mais le bouscule » ; il doit le contourner et ne le bousculer que s'il ne peut pas faire autrement ; *précise D292*) : dans `UHOLFullSpeedProcessor`, un marcheur cherche les corps qu'il toucherait dans les **0,5 s** à venir (temps de contact d'après sa vitesse et celle de l'autre, unités à l'arrêt comme en marche, celles derrière lui et au-delà de sa place ignorées) ; si son chemin est pris, il tourne du **plus petit angle** libre, par pas de 10°, d'abord du côté opposé au corps (le même à chaque pas pendant qu'il le dépasse ; de face, les deux gardent la même main et se croisent) ; au-delà de **60°**, le détour est trop grand : il continue tout droit et pousse (séparation de D292). Cause : l'évitement de Mass ne tourne qu'un pas sur 6 (D220) et ne regarde qu'à 2,5 m, soit 1,8 m parcourus à 6 m/s entre deux passages. Réglages `hol.Units.Avoid 0/1/2`, `hol.Units.AvoidTime`, `hol.Units.AvoidMaxAngle`. Mesuré (essai scripté, avant → après) : Paysan à l'arrêt poussé de 2,75 m → **0** (contourné à 1,3 m) ; groupe de 7 poussé de 1,41 m → **0** (2,3 m de détour) ; deux marcheurs de face au contact → 1,64 m ; mur serré de 5 Paysans (6,5 m) traversé par 6 marcheurs : poussé de 4,84 m → 1,37 m (4 le contournent, 2 partis du milieu poussent). Coût S2 (foule de 2 000 en marche, machine chargée) : ~0,5 ms par pas, **à remesurer au repos en J4.7** ; 0,8 s ou 75° essayés, sans gain net. 5e essai validé. **Reporté** (choix de l'utilisateur) : une unité bousculée **reprend sa position** ensuite ; à faire avec les **formations** (étape « armée », M4, voir D287). |
| D294 | 2026-10-14 | Technique — Arrivée au milieu des autres, vitesse envoyée | Corrections trouvées par la partie solo de J4.7 et par S5 : **dernier tronçon droit sur la place à toute distance** (le pilotage de Mass ne tourne qu'à ~50°/s : après chaque virage du regard en avant, le Paysan tournait autour de ceux déjà posés à côté de sa place, puis le compteur de blocage l'arrêtait à 3-5 m) ; **le chemin est quitté pour la place** dès que le marcheur est aussi près de sa fin que sa place l'est (au moins 3 m) et que le navmesh le laisse aller droit (le chemin finit sur le point visé, souvent occupé) ; regard en avant avec une **marge de 10 %** sur les rayons (`hol.Units.AvoidClearance`, 1,1 ; sans retard du pilotage, les corps se frôlaient) ; la **vitesse envoyée aux clients est le déplacement du dernier pas**, poussées de la séparation comprises (`hol.Net.StepVelocity`, 1 ; S5 : 2,02 → 1,72 envoi par unité et par seconde, écart affiché 27,8 → 23,5 cm) ; tolérance de recouvrement de la séparation réglable (`hol.Units.SeparationSlop`, 0 par défaut : 20 cm ne gagnent que 2 %). **S5 reste au-dessus de D218** (~39 Ko/s par client contre 32) : la séparation dans la foule serrée de S2 en est la cause principale (32,3 Ko/s sans elle), **décision de l'utilisateur attendue**. |
| D300 | 2026-10-14 | Technique — S5 au-dessus de 32 Ko/s après J4 | **Chasser d'abord les ~8 Ko/s d'en-têtes non expliqués** (choix de l'utilisateur, conforme à la recommandation ; reprend la piste écartée par D274) : trace réseau d'Iris ou lecture du code pour savoir ce que coûte chaque écriture d'un morceau (~30 octets mesurés contre ~10-15 attendus), puis réduire ; mouvement de J4 inchangé (séparation de D292 gardée). Repli si le gain ne suffit pas : seuil d'envoi plus large dans les foules. Écartés : **séparation adoucie entre marcheurs** (recouvrements dans les foules denses, défaut du 3e essai), **accepter ~39 Ko/s jusqu'à M5** et **seuil de 45 cm partout** (écart affiché au-delà de 25 cm). D295 à D299 sont au plan de J5. |
| D301 | 2026-10-14 | Technique — Hauteur des unités sur le réseau (S5) | **La hauteur n'est plus envoyée : le client pose les unités au sol lui-même** (choix de l'utilisateur, conforme à la recommandation ; suite de D300). L'état passe de 71 à **59 bits** ; le client prend pour sol la surface statique la plus basse sous l'unité (on passe sous la couronne d'un arbre, jamais dessus), à chaque état reçu, et la correction lissée de D271 lisse aussi un changement de hauteur ; la règle d'envoi mesure l'écart horizontal. Plus tard, les tireurs postés sur les remparts (D23) demanderont un bit « hors sol » et leur hauteur. Avec D300 (tableaux simples au lieu de tableaux rapides) : **S5 PASS à 29,97 Ko/s par client** (39,7 avant), écart affiché 23,6 cm. Écartés : **seuil d'envoi plus large dans les foules** (écart affiché plus grand dans les foules) et **D301 plus moins d'en-têtes de morceaux** (gain incertain, M2.5.5 n'avait rien gagné). |
| D302 | 2026-10-14 | Technique — S2 au-dessus de 3 ms après J4 | **Accélérer le code de mouvement de J4** (choix de l'utilisateur, conforme à la recommandation), sans changer le mouvement validé au 5e essai (D290-D294) : S2 court à 4,41 ms par pas contre 3 ms, dont ~2 ms ajoutées par J4 (séparation, regard en avant, ordres). Écartés : **remesurer d'abord sur une machine au repos** (S2 resterait sans doute au-dessus de 3 ms) et **relever le seuil de S2**. *Fait (2026-10-15)* : les processeurs de Mass qui tournaient sur un seul cœur (évitement en marche, pilotage, orientation) font tourner leurs morceaux **en parallèle**, par des sous-classes qui laissent le code de Mass intact (`Simulation/HOLParallelMassProcessors`, `hol.Simulation.ParallelMass`) ; le regard en avant ne visite que les cases utiles, s'arrête au premier contact pour juger un virage et répartit ses morceaux au fil de l'eau. **Résultats identiques** (même graine : mêmes recouvrements au début et à la fin de S2). **S2 court : 4,41 → 2,71 ms par pas, PASS.** |
| D303 | 2026-10-15 | Technique — Charge du Paysan en temps réel, naissance vers le ralliement | **Retours du 1er essai de J5a** (choix de l'utilisateur) : (1) le Paysan **naît du côté du point de ralliement** (sur l'arc de D283, tourné vers le ralliement ; une seule rangée puisqu'il part aussitôt) ; sans ralliement, en bas du centre comme avant ; (2) le bloc d'un Paysan seul se **met à jour à chaque changement** d'activité ou de charge (il ne se redessinait qu'au dépôt ou à un changement de sélection) ; (3) **proposition de l'utilisateur, par souci d'optimisation** : la charge exacte n'est envoyée **en temps réel qu'à son propriétaire, pour son unité sélectionnée seule** (le contrôleur dit au serveur quelle unité il suit, le serveur lui renvoie sa charge à chaque changement, au seul propriétaire) ; l'état envoyé à tous ne dit plus que « porte quelque chose » et quelle ressource (**2 bits** pour un porteur au lieu de 7 : ce que montrera la charge visible de J5c). *À terme* (utilisateur) : « inactif », « en marche »… seront cachés, seule la charge restera affichée ; gardés pour l'instant (essais). | § 16.6 |
| D304 | 2026-10-15 | Technique — Orientation instantanée | **2e essai de J5a** (choix de l'utilisateur : « la rotation du Paysan doit être instantanée, comme la vitesse précédemment », D290) : une unité en marche **regarde là où elle va dès le pas où elle part ou tourne** ; à l'arrêt elle garde son orientation (poussée de côté, elle ne tourne pas). `UHOLInstantOrientationProcessor` (vitesse de marche, pas le déplacement du pas) remplace l'orientation lissée de Mass (mélange de la marche et du chemin, ~0,3 s ; `UHOLParallelSmoothOrientationProcessor` et `UMassSmoothOrientationTrait` retirés) ; chez le client, plus de rotation à 540°/s : la direction reçue tout de suite, pour une unité « en marche » seulement. Vaut pour tous les types (comme D290). Le pilotage et l'évitement à l'arrêt de Mass lisent l'orientation : recouvrements de S2 légèrement changés (522 / 374 → 547 / 387 paires). | § 16.6 |
| D305 | 2026-10-15 | Technique — Réutilisation des chemins de la récolte | **Faite dès J5a** (choix de l'utilisateur, **contre la recommandation** de la reporter jusqu'à ce que S7 approche 3 ms ou jusqu'aux murs de M8) : les marches que la récolte se donne (source ↔ dépôt) reprennent un chemin déjà calculé entre les mêmes points au lieu d'en redemander un au navmesh ; le chemin gardé est oublié quand le navmesh change sur son trajet (source épuisée, bâtiment posé). Gain attendu faible aujourd'hui (~0,2 ms par pas dans S7, chemins presque tous droits), plus grand sur de vraies cartes (bâtiments, murs, relief). Écarté : **attendre** (recommandation). *Fait (2026-10-15)* : chemin gardé par cases de **6 m** du départ et de l'arrivée (1 m : 8 % de reprises, 3 m : 45 %, 6 m : 83 %) ; un chemin gardé droit n'est repris que si un **rayon sur le navmesh** du Paysan à sa place est libre (un arbre peut se trouver entre) ; un chemin avec détour garde son couloir, partagé comme celui d'un paquet (D286). S7 court, machine chargée : **77 % des marches reprennent un chemin**, partie « chemins » des ordres **0,157 → 0,108 ms par pas**, récolte inchangée. | § 16.6 |
| D295 | 2026-10-14 | Technique — Périmètre de J5 | **Les 4 ressources, une source simple chacune, dépôt au centre seul** (choix de l'utilisateur, conforme à la recommandation) : **arbres** (bois), **buissons de baies** (nourriture), **filons** de pierre et d'or ; le Paysan rapporte au **centre principal**, seul dépôt tant qu'il n'y a pas de construction (les camps de l'étape construction ajouteront des dépôts) ; chaque source **s'épuise** ; sur `Plain`, posées par script, symétriques : bois et baies près de chaque centre, pierre et or plus loin, contestés entre voisins (§ 15). **Nuance de l'utilisateur : garder les troupeaux façon AoE pour plus tard** (repérés, ramenés près du centre, abattus ; ils demandent points de vie, mort et carcasse : avec M5). Écartés : bois et baies seuls (pierre et or non éprouvés) ; troupeaux dès J5. | § 5.1, § 16.6 |
| D296 | 2026-10-14 | Technique — Boucle de récolte | **Aller-retour façon AoE4** (choix de l'utilisateur, conforme à la recommandation) : récolte **au contact** à un taux par type d'unité et par ressource (~0,7/s, indicatif) ; **charge de 10** (la Brouette, § 11, l'augmentera) ; plein, le Paysan va au **dépôt le plus proche**, le stock du joueur monte **au dépôt**, puis il revient à **la même source** ; source épuisée → la plus proche **du même type** dans ~15 m, sinon **inactif** (touche « . ») ; un ordre de déplacement arrête la récolte et **garde la charge**, une autre ressource la **fait perdre** ; aucune limite par source, places autour d'elle par les règles de J4 (D287). Écartés : revenu au contact sans trajet (la distance ne compte plus, contraire à D83) ; places limitées par source façon StarCraft (attente, illisible avec des unités qui se traversent). | § 5.1, § 16.6 |
| D297 | 2026-10-14 | Technique — Sources dans la partie | **Registre côté serveur, affiché par instances** (choix de l'utilisateur, conforme à la recommandation) : un sous-système tient le tableau des sources (type, position, rayon, quantité restante) ; **un composant d'instances par type de source** chez chaque machine ; réplication par **un acteur partagé à tableau rapide** (recette des morceaux de M2.5 : liste complète à l'arrivée, puis quantités et épuisements) ; **un seul fournisseur de zones interdites** pour le navmesh (une boîte par source, retirée à l'épuisement, seule sa tuile se refait, comme les bâtiments de J4.1). Ordre de grandeur : 400 à 500 sources à 8 joueurs, 9 sur 10 des arbres ; quantités indicatives : arbre 100 bois, buisson 200 nourriture, filon 2 500 ; formes provisoires (tronc et cône, boule à baies, blocs gris ou dorés). Écartés : un acteur répliqué par source (réseau, mémoire, arrivée d'un client, contraire à D29 / D190) ; mixte acteurs + registre (deux chemins pour un même comportement). | § 16.6 |
| D298 | 2026-10-14 | Technique — Ordre de récolte et retour visuel | **Le nécessaire, plus ce qui donne vie à la récolte façon AoE4** (choix de l'utilisateur : option C, **contre la recommandation** A) : clic droit sur une source = ordre de récolte vérifié par le serveur (D288) ; ralliement sur une source ; source sélectionnable (nom, quantité restante) ; bloc d'un Paysan : activité et charge (« Bois 7/10 »), d'un groupe : Paysans par activité ; barre du haut : Paysans par ressource et compteur d'**inactifs** (clic ou touche « . » : inactif suivant, caméra centrée, D30) ; repère bref de J4.6 sur la source visée ; **en plus** : **animation de récolte** (hache, pioche, cueillette), **charge visible** sur le Paysan (fagot, panier, bloc), **« +10 » qui monte** au dépôt. L'activité répliquée (D269, déjà dans l'état réduit) gagne la récolte et le type de charge (quelques bits, S5 à remesurer). Risques : animations à trouver, charge sur le squelette instancié de M2. Écartés : A seul ; le strict minimum (clic droit et ralliement). | § 7.3, § 16.6 |
| D299 | 2026-10-14 | Technique — Vérification de J5 | **Tests, solo et 2 clients, stress test de récolte S7** (choix de l'utilisateur, conforme à la recommandation) : tests d'automatisation des règles pures (taux et charge, dépôt le plus proche, source voisine ou inactif, épuisement, charge perdue) ; dans `quick_check.ps1`, partie **solo** (ralliement sur des baies, stock qui ne monte qu'au dépôt, arbre épuisé → arbre voisin, bois vidé → inactifs et touche « . », captures de l'animation et de la charge) et partie **à 2 clients** (récolte de l'autre vue, source épuisée disparue chez les deux, stock de l'autre jamais reçu) ; **nouveau stress test S7 : 2 000 Paysans en récolte** sur `StressTest`, seuil de simulation de 3 ms (coût des trajets ; chemin source-dépôt réutilisé plutôt que recalculé) ; **S5 court** ; essai de l'utilisateur. Trois parties jouables J5a (bois), J5b (4 ressources et HUD), J5c (vie de la récolte), chacune avec l'essai de l'utilisateur (demande de l'utilisateur). Écarté : `quick_check` seul, sans stress test de récolte. | § 16.6 |
| D306 | 2026-10-15 | Technique — Charge perdue au changement de ressource | **Perdue au premier point de la nouvelle ressource, pas à l'ordre** (essai de J5b.1, règle de l'utilisateur ; *précise D296*, qui la perdait dès l'ordre) : un Paysan qui porte 9 nourriture et reçoit l'ordre de couper du bois garde ses 9 nourriture en route et au contact de l'arbre, tant qu'il n'a pas récolté un bois entier ; au premier bois, la nourriture est perdue et il porte 1 bois. Un Paysan plein d'une ressource envoyé sur une autre va droit à la nouvelle source (il ne dépose pas d'abord) et continue de récolter jusqu'à ce premier point. Une charge vide prend la nouvelle ressource dès l'ordre (le bloc la montre en route). Règles pures : `HOLGather::Gather` reçoit la ressource récoltée et rend la charge perdue, `IsFullOf` (plein de cette ressource) ; la progression vers le point suivant compte pour la ressource récoltée seulement ; source épuisée → voisine de la ressource de la source, non de la charge. | § 16.6 |
| D307 | 2026-10-15 | Technique — Inactifs et comptes de récolteurs | **Choix d'exécution de J5b.2** (dans le cadre de D298, sans question à l'utilisateur) : (1) l'activité répliquée gagne une 4e valeur, **« en tournée de récolte »** (vers la source, vers le dépôt, en attente d'une place) : elle tient dans les **3 bits** déjà envoyés, **aucun bit de plus** (S5 à 0,9 Ko/s du seuil) ; un récolteur n'est plus jamais « inactif » le temps d'un pas (arrivée au dépôt, attente d'une place), donc le **compteur d'inactifs et la touche « . » se calculent chez le client** d'après l'état reçu, sans clignoter ; marche dessinée comme « en marche » ; (2) les **Paysans par ressource** (barre du haut) sont comptés par le serveur à chaque pas et envoyés dans le `PlayerState` **au seul propriétaire** (comme le stock, D282 ; envoi seulement quand un compte change) : la ressource d'un récolteur vide sur le chemin du retour n'est pas dans l'état envoyé à tous ; (3) le **bloc d'un groupe** compte ses Paysans par activité (récolte, en marche, inactifs) d'après l'état reçu. La touche « . » prend l'inactif suivant par identifiant, revient au premier après le dernier, centre la caméra ; l'action `IA_NextIdle` répond à « . » (QWERTY), à la touche « ; . » de l'AZERTY (le moteur la nomme « ; », avec ou sans Maj) et au « . » du pavé. Écartés : **un bit « au travail » de plus** (S5) ; **comptes et inactifs envoyés par le serveur** (aller-retour pour la touche « . »). | § 7.3, § 16.6 |
| D308 | 2026-10-15 | Technique — Détours des allers-retours | **Essai de J5b.2** (l'utilisateur : « des Paysans font énormément de détours » entre le centre et les ressources). Mesuré par la distance réellement marchée à chaque marche de récolte (`hol.Gather.LogWalks`) : ~35 marches sur ~70 déchargements faisaient 2 à 3 fois la ligne droite (jusqu'à 44 m de trop). **Cause principale** : une unité qui suit un chemin du navmesh (contourner le centre, sortir du bois) gardait la direction du pilotage de Mass, qui ne tourne qu'à ~50°/s : à pleine vitesse (D290) elle s'écartait de **5 à 12 m** à chaque coin ; la marche droite vers sa place avait déjà été corrigée ainsi (D294), pas le suivi d'un chemin. **Fait** : (1) sur un chemin, l'unité vise à chaque pas le point que le suivi de Mass avance devant elle, plus 1 m le long du chemin, puis le regard en avant de D293 (`hol.Units.PathAim`) ; (2) un **détour gardé** (D305) n'est repris que si cette marche ne peut pas aller droit, voit son premier coin, si son dernier coin voit sa cible, et si elle ne marche pas plus loin que la marche gardée (`hol.Units.ReuseCheckDetour`) ; (3) une unité poussée dans une **poche fermée du navmesh** (entre les arbres de devant du bois : chemin partiel qui finit où elle est) en sort droit vers sa place, au lieu de voir chaque marche finir aussitôt et d'abandonner sa charge. Résultat (20 Paysans, bois puis baies) : marches anormalement longues **37 → 2** (autour du massif de baies, légitimes), mètres de trop **647 → 18**, dépôts manqués **15 → 0**. Coût nul mesuré (S2 et S7 en A/B dans les deux ordres) ; recouvrements de S2 inchangés. | § 16.6 |
| D309 | 2026-10-15 | Technique — Récolteurs fantômes, source sans place | **2e essai de J5b.2** (l'utilisateur : moins de détours, mais les Paysans « se gênent devant le forum »). Choix de l'utilisateur : (1) **en aller-retour automatique** (activité « en tournée de récolte » de D307 : vers la source, vers le dépôt), un Paysan **traverse toutes les unités** — choix « toutes », **contre la recommandation** « les autres récolteurs seulement » — : il ne regarde pas en avant pour les éviter (D293), la séparation ne le pousse pas et il ne pousse personne (D292), personne ne se détourne pour lui ; un ordre donné par le joueur garde les collisions ; à la source, chacun récolte à **sa place réservée** (pas d'empilement) et un Paysan qui récolte redevient un corps (`hol.Units.GhostTrips`) ; (2) **plus aucune place libre** autour de la source visée (ordre, point de ralliement, ou places manquées), quelle que soit la ressource : une source de la **même ressource à 8 m au plus** (choix conforme à la recommandation : les arbres voisins, le même massif de baies, jamais un autre filon), sinon le Paysan **s'arrête inactif devant la source**, sa charge gardée (compté dans « Inactifs », touche « . ») ; fini l'attente invisible de 3 s en boucle. Écartés : traverser seulement les autres récolteurs (recommandation) ou seulement ceux en aller-retour ; 15 m ou 4 m. **Précisée au 3e essai** (2026-10-15/16, sans nouveau numéro : D315 à D317 prises par la review du GDD) : (3) **massifs** — demande de l'utilisateur : « si 175 Paysans vont sur une forêt, ils vont entièrement entourer la forêt » : les sources de même ressource à 8 m l'une de l'autre, **de proche en proche**, forment un massif (souches comprises : une source épuisée ne le coupe pas) ; sans place autour de la source visée, le Paysan prend la première source du massif qui a une place, par ordre de distance à l'emprise du dépôt + 0,5 × distance à la source visée (la **lisière tournée vers le dépôt** d'abord) ; inactif devant seulement si tout le massif est plein ; un filon isolé ne mène jamais à un autre filon ; même recherche pour une source épuisée (**remplace les 15 m de D296**) ; (4) **places serrées** façon AoE4 (capture de l'utilisateur, 37 villageois au bois) : places réservées sur le point réel, jamais à moins de **100 cm** l'une de l'autre, même sur deux sources voisines (`hol.Gather.SpotSpacing`, réglé à l'essai), un rang régulier autour de la source, tourné d'abord vers le dépôt ; deux unités arrêtées dont un récolteur à sa source ne sont écartées que jusqu'à cet écart (un marcheur pousse toujours à deux corps) ; (5) les **inactifs devant une source pleine** attendent sur des points réservés en rangées hors de l'anneau des places (plus jamais sur une place de récolte) ; (6) *(2026-10-16, choix de l'utilisateur, entre la recommandation A « rangées sans limite » et sa propre proposition)* **massif entier plein** → le Paysan cherche un **bosquet voisin** (autre massif de la même ressource dont une source est à ~20 m au plus d'une source du massif plein, réglable : `hol.Gather.NearbyMassifRange`) et y prend la première place libre selon le même ordre ; sinon il attend devant la source visée, sur des **rangées sans limite** (plus jamais deux inactifs sur un même point) ; un filon ne mène toujours pas à un autre filon (60 m). | § 16.6 |
| D325 | 2026-10-18 | Technique — Coût des chantiers dans S7 (J6b.3) | **L'apparence des bâtiments sort du pas de simulation** (choix de l'utilisateur, conforme à la recommandation ; écartés : pièces allégées dans le pas, ~0,1-0,2 ms seulement ; gagner sur l'évitement à l'arrêt ; scénario S7 revu). S7 avec ses 24 camps bâtis pendant la mesure était à ~3,0 ms (seuil 3) : le surcoût des chantiers était de l'**affichage** fait pendant le pas (~10 pièces créées par chantier au premier coup et à la fin, 2-3 ms chacun ; montée qui déplace les pièces, ~0,1-0,2 ms par pas). Désormais : la simulation ne change que l'**état** (étape, travail, bâtisseurs) et ce qui compte pour elle (zone interdite du navmesh, relevance), et marque l'apparence à refaire ; la **présentation** reconstruit les pièces, les couleurs et la montée **à l'image**, hors du pas (D219), au plus une fois par image et par bâtiment ; **le serveur dédié ne crée aucune pièce** (personne n'y regarde ; la sélection est chez le client, D262). Le coût passe au fil de jeu du client, sous le seuil de rendu (≥ 144 i/s). | J6b.3 |
| D327 | 2026-10-19 | Technique — Durée de `quick_check` (J7b) | **Deux niveaux** (choix de l'utilisateur, conforme à la recommandation) : `quick_check` par défaut = tests d'automatisation + **une seule partie solo courte** (menu, caméra, sélection, déplacement, petit combat), **moins d'1 min** ; la version complète (toutes les parties, ~8 min) passe derrière **`-Full`**, lancée seulement **avant un commit d'étape et à une clôture**. Les sous-agents lancent la version rapide plus les parties qu'ils touchent (`-Parts`). Motif : le script, à ~35 s à D267, avait grossi à chaque étape jusqu'à 8 min, lancé en entier à chaque boucle. Écartés : parties ciblées seulement (complet inchangé) ; alléger tout le script (gros chantier). *Précisé (2026-10-21, choix de l'utilisateur)* : **au commit, seulement les parties touchées** (`-Parts`) ; **`-Full` seulement aux clôtures de chapitre**. | J7b |

## Journal d'avancement

### 2026-10-13 — Chantier ouvert (D252)

- Proposition de l'utilisateur après la clôture du menu : avancer par le parcours du joueur (« je spawn, je bouge ma
  caméra, je clique sur mon centre, son interface apparaît… ») plutôt que par couches techniques. Option A retenue :
  réseau (M2.5) inséré après J2, avant que le joueur change l'état de la partie. **D252** consignée directement au GDD
  (§ 16.6 et § 21), comme D248, car elle concerne tous les chantiers.
- Constats pour J1 : la carte de bataille tourne avec le pion par défaut du moteur ; le stress test le déplace pour ses
  vues de mesure (`HOLStressTestSubsystem`, D236, D237) ; le GDD n'a sur la caméra que les raccourcis de D30 (centrer
  sur un groupe, sur le héros) ; le centre principal s'appelle « centre principal » au GDD (§ 17).

### 2026-10-13 (suite) — J1, Q1 tranchée (D253)

- Périmètre : caméra RTS, emplacement de départ, vitesse de caméra et défilement au bord dégrisés ; remappage plus tard.
- Pour Q2 : D30 ne donne aucune touche de déplacement de caméra, et sa grille de commandes (`A Z E R / Q S D F /
  W X C V`) exclut ZQSD ; repères de hauteur dans D236 et D237 (vue de jeu 30-40 m, vue large ~100 m, toutes deux
  mesurées au seuil de 144 i/s).

### 2026-10-13 (suite) — J1, Q2 tranchée (D254)

- Caméra façon AoE4 : sans rotation, plongée fixe ~55°, 30-40 m par défaut, zoom ~15 à ~70 m, flèches, bord, clic
  molette, bornée à la carte, départ au-dessus de son emplacement.

### 2026-10-13 (suite) — J1, Q3 tranchée (D255) ; constat sur les cartes

- Caméra locale sans pion, pilotée par le contrôleur de partie.
- Contenu des cartes (liste des acteurs dans l'éditeur) : **`BattleMap`** = sol plat de 1 km × 1 km (`Floor`), navmesh
  sur 400 m × 400 m, **un seul `PlayerStart`** (x = -5,6 m, z = 11 m), lumière, ciel ; **`StressTest`** = sol de
  400 m × 400 m, un `PlayerStart` au centre. Aucune donnée d'emplacement de départ ni de limite de carte.

### 2026-10-13 (suite) — J1, Q4 tranchée (D256)

- Réponse de l'utilisateur : « tu as l'autorisation de faire des maps si besoin » ; précision demandée, puis choix d'une
  nouvelle carte avec les acteurs de l'option A.

### 2026-10-13 (suite) — J1, Q5 tranchée (D257) : plan de J1 complet

- Vérification : tests d'automatisation, bout en bout par script, S1 et S2 courts, essai de l'utilisateur.
- **Précision d'implémentation de D255** : la caméra RTS ne s'applique qu'aux cartes qui ont des emplacements de
  départ ; `StressTest` et `BattleMap` n'en ont pas et gardent le pion par défaut, que le stress test déplace comme
  avant. Le fichier du stress test (chantier M2, en cours) n'est donc pas touché, et la « fonction dédiée » de D255
  n'est pas nécessaire pour l'instant.
- Ordre d'implémentation : J1.1 socle, J1.2 carte, J1.3 réglages, J1.4 vérification, J1.5 clôture.

### 2026-10-13 (suite) — J1.1 fait : socle de la caméra

- **`Map/HOLStartSlot`** (dérivé de `APlayerStart`, numéro 0 à 7) et **`Map/HOLMapBounds`** (boîte vue de dessus, visible
  dans l'éditeur seulement) ; `AHOLStartSlot::WorldHasSlots` dit si une carte est une carte de partie.
- **`Player/HOLRTSCamera`** : acteur local non répliqué (racine = point visé au sol, bras sans collision ni retard,
  caméra) ; plongée 55°, hauteur 15 à 70 m (35 m au départ), 5 m par cran de molette ; vitesse de déplacement
  proportionnelle à la hauteur (~40 m/s à 35 m en vitesse normale ; lente ×0,6, rapide ×1,6) ; point visé borné aux
  limites. Règles pures (`ClampFocus`, `GetPanSpeed`) pour les tests.
- **`Player/HOLPlayerController`** : curseur visible, souris libre, pas de pion ; ajoute `IMC_Game`, fait apparaître la
  caméra chez le joueur seul, la place au pied de l'emplacement du joueur dès que l'index de joueur est connu (journal
  `Camera above the start of slot N`) ; flèches, molette, glisser au clic molette (le point saisi reste sous le
  curseur), défilement au bord (1 % de la largeur ; position simulée par `hol.Camera.TestCursor "X Y"` pour les essais
  par script) ; `hol.Camera.Report` écrit le point visé et la hauteur.
- **Mode de jeu** : sur une carte à emplacements, contrôleur de partie et aucun pion ; sinon rien ne change.
- **Réglages** : `CameraSpeed` et `bEdgeScroll` dans `UHOLGameUserSettings` (onglet Commandes branché à J1.3).
- **Lancement depuis le menu** : routeur de Common UI en mode jeu **souris libre** (avant : capturée, ce qui cachait le
  curseur).
- `Tools/Editor/make_game_input.py` : `/Game/Input/IA_Camera_Left`, `_Right`, `_Up`, `_Down` (flèches), `_Zoom` (molette),
  `_Drag` (clic molette), `IMC_Game`.
- Au passage : `HOLFonts.cpp` passe aux accesseurs non dépréciés de la police composite (avertissement C4996 manqué en
  F4 du chantier menu, le filtre de compilation ne montrait que les erreurs).

### 2026-10-13 (suite) — J1.2 fait : carte `Plain`, caméra essayée

- **`Tools/Editor/create_match_map.py`** (éditeur complet) crée **`/Game/Maps/Plain`** : sol de 500 m (grille du moteur),
  soleil, ciel et éclairage du ciel mobiles (rien à construire), brouillard, **limites 400 m** (`AHOLMapBounds`), volume
  de navmesh sur les limites, **8 emplacements** sur un cercle de 150 m tournés vers le centre, dans l'ordre 180°, 0°,
  270°, 90°, 225°, 45°, 315°, 135° (0 et 1 face à face, 0 à 3 aux quatre points cardinaux), un repère de 8 m × 2 m sur
  chacun en attendant le centre principal de J2. Navmesh non vérifié (utile à J4).
- `Plain` remplace `BattleMap` dans la liste de « Nouvelle partie » ; nom affiché « Plaine » / Plain / 平原 (94 textes).
- **Essai** (deuxième écran, partie lancée depuis le menu) : `Match map: RTS camera, no pawn` ; caméra au-dessus de
  l'emplacement 0 (-150 m, 0) ; flèche Haut tenue 1 s → +40 m, 3 s → +120 m (40 m/s attendus) ; trois crans de molette
  → hauteur 35 → 50 m ; curseur simulé au bord droit 1 s → +40 m ; curseur au bord dès le départ → point visé arrêté à
  la limite (+200 m). Captures `j1_*.png`.
- **Correction** : le défilement au bord avec le vrai curseur n'agit que si la fenêtre de jeu est au premier plan et que
  le curseur est dedans (au premier essai, le point visé avait dérivé de 23 cm, curseur hors de la fenêtre). Un premier
  essai de la flèche n'avait avancé que de 6,6 m en 1 s, non reproduit ensuite (perte de focus probable de la fenêtre).
- Outils d'essai : `hol.Delay <secondes> <commande>` (commande différée), `hol.Camera.Report [délai]`, `hol.UI.Key
  <touche> [délai] [durée d'appui]`, `MouseScrollUp` / `MouseScrollDown` envoient un cran de molette au milieu de la
  fenêtre de jeu.

### 2026-10-13 (suite) — D258 : aucune ombre dans le jeu

- Demande de l'utilisateur pendant J1.3 : « afin de ne pas avoir de problèmes de perfs, écris-le maintenant, aucune
  ombre dans le jeu ». Consigné directement au GDD (§ 16.6 et § 21, D258), comme D252, car il touche tous les chantiers ;
  note laissée à M2 (les mesures du stress test changent quand le réglage sera appliqué).

### 2026-10-13 (suite) — J1.3 fait, D258 appliquée

- **J1.3** : onglet Commandes, vitesse de la caméra et défilement au bord **dégrisés**, lus et enregistrés dans
  `UHOLGameUserSettings` ; « Changer les touches » reste grisé, note « Changer les touches : pas encore dans le jeu. »
  (traduite). Vérifié aux touches (capture `j1_controls_changed.png`, `CameraSpeed=0` enregistré puis remis à 1 dans
  le fichier de test).
- **D258 appliquée** (choix de l'utilisateur : maintenant, partout) : `DefaultEngine.ini` `[SystemSettings]` coupe
  toutes les ombres au-dessus des niveaux de qualité (`r.ShadowQuality = "0" LastSetBy: SystemSettingsIni` vérifié en
  partie) ; soleil et ciel de `Plain` sans ombres (carte recréée) ; groupe « Ombres » retiré de Réglages > Vidéo
  (93 textes). Capture `j1_noshadow.png` : le repère ne projette plus d'ombre.
- **S1 et S2 courts** sur `StressTest` (deuxième écran, `-saveddirsuffix=Tests`) : complets, 2 000 unités, 0 restante,
  registre sans écart ; la carte sans emplacements garde le pion par défaut (aucune ligne `Match map`). Chiffres hors
  seuils mais non significatifs (compilation du moteur 5.8.3 en parallèle, fil de jeu limitant) : S1 3,53 ms / 110 i/s,
  S2 3,38 ms / 119 i/s, GPU ~3,9 ms. Référence sans ombres à reprendre au repos.
- Constat : « Réglages » ne s'ouvre pas en partie (pas encore de menu pause, prévu plus tard).

### 2026-10-13 (suite) — J1.4 : vérifications automatiques

- **Tests d'automatisation** (`Tests/HOLCameraTests.cpp`) : `Camera.Bounds`, `Camera.PanSpeed`, `Camera.Zoom` (caméra
  dans un monde de test : bornes du zoom, cran de molette, écran droite = +Y, écran haut = +X, bornes), `Settings.Camera`
  (aller-retour par un `.ini` temporaire), `Map.StartSlots` (`Plain` lue sans la démarrer : 8 emplacements uniques,
  à égale distance du centre, dans les limites, 0 et 1 face à face, 2 et 3 face à face, 2 à un quart de tour de 0 ;
  positions lues dans les transformations relatives, les composants d'une carte non démarrée n'étant pas enregistrés).
  **14/14 verts.**
- **`Tools/Tests/check_camera.ps1`** : partie lancée depuis l'écran avec une configuration JSON, à 2 joueurs (joueur
  local en emplacement 0) et à 8 (sept IA, joueur local en emplacement 7) : caméra au-dessus du bon emplacement
  (-150, 0) et (-106, 106) m, flèche 1 s → +40 m, molette → 40 m de haut, bord droit 1 s → +46 m (vitesse plus
  grande à 40 m), aucune erreur ; **PASS** dans les deux cas.
- **`capture_screens.ps1`** : contrôle de lancement généralisé (« Match on <carte> ») ; **PASS** en fr, en, ja ; planche
  relue (Plaine, plus d'onglet Ombres, Commandes actif, bataille sans ombre).

### 2026-10-13 (suite) — J1.4 : session d'essai de l'utilisateur ouverte

- Machine pas au repos (compilation du moteur 5.8.3, compilations et éditeurs d'un autre projet) : S1 et S2 au repos
  reportés. Session d'essai lancée sur le deuxième écran ; retours de l'utilisateur attendus.
- Retour : « tout est bien », mais sans unité de mesure à l'écran le zoom ne se juge pas. Choix de l'utilisateur :
  **repères d'échelle sur `Plain`** à côté de l'emplacement 0 (jusqu'au centre principal de J2) : bloc de 4 × 5
  mannequins de 1,8 m espacés de 1,2 m (`SM_Mannequin`, l'unité du stress test), maison FantasyVillage (rez-de-chaussée
  3,1 × 6,2 × 3 m et toit 5,3 × 7,8 × 3,2 m, approximative). `create_match_map.py` complété, `Plain` recréée (ancienne
  copie dans le dossier temporaire de la session). Captures `scale_35m`, `scale_15m`, `scale_70m` : au départ (35 m,
  ~85 m de sol en largeur), un soldat fait ~20-30 px de haut sur 1600 px ; à 70 m, quelques pixels, et les emplacements
  voisins entrent dans le cadre. Nouvelle session d'essai ouverte.

### 2026-10-13 (suite) — J1.4 fait (essai de l'utilisateur) ; J1.5 commencée

- Essai sur les repères : « ça marche bien oui » ; valeurs de la caméra gardées (35 m au départ, 15 à 70 m, 55°, 90°
  de champ horizontal, ~40 m/s à 35 m, bord à 1 %).
- Machine toujours occupée (compilation du moteur 5.8.3, autre projet) : S1 et S2 au repos reportés, sans bloquer J2.
- **J1.5** : D253 à D257 reportées au journal du GDD (§ 21) et paragraphe « Parcours J1 » au § 16.6.
- Tests d'automatisation relancés sur `Plain` recréée : **14/14 verts**. **J1 close.**

### 2026-10-13 (suite) — J2 ouvert : plan en cinq questions

- Repères du GDD : centre principal (§ 17, un par joueur au départ en mode Standard, sa destruction fait perdre) ;
  sélection au § 9 (clic, multiple, cadre, double-clic) et D30 (groupes, `F4`…) ; HUD de D201 (sélection au centre de
  la barre du bas : J3) ; couleur du joueur par instance (D233) et choix de couleur de la configuration (D244) ; M4 garde
  la sélection d'unités et les ordres.

### 2026-10-13 (suite) — J2, Q1 tranchée (D259)

- Centre principal par emplacement occupé, à sa couleur ; sélection d'un bâtiment au clic, désélection au clic dans le
  vide ou à Échap ; cercle au sol, surlignage au survol ; pas de panneau.

### 2026-10-13 (suite) — J2, Q2 tranchée (D260)

- Bâtiments : acteur C++ commun répliqué, créé par le serveur, propriétaire = numéro du joueur, table des types depuis
  un CSV ; GAS en M5.

### 2026-10-13 (suite) — J2, Q3 tranchée (D261)

- Centre provisoire : bloc et tour en volumes du moteur, ~15 × 15 m et ~10 m, toit ou bandeau à la couleur du joueur.

### 2026-10-13 (suite) — J2, Q4 tranchée (D262)

- Sélection chez le client seul, rayon sur un canal de sélection, cercle au sol, survol par teinte.
- Question de l'utilisateur avant Q5 : pourquoi Angelscript n'est-il pas évoqué ?
- Constats : les moteurs `UE_5.8_AS_1` (projet) et `AS_2` sont le moteur **Angelscript** de Hazelight (plugins
  Angelscript, AngelscriptEnhancedInput, AngelscriptGAS, activés par défaut ; dossier `Script/` du projet avec seulement
  `Binds.Cache`) ; le GDD ne l'évoque nulle part, tous les plans supposaient du C++ (oubli de l'assistant). **Le moteur
  5.8.3 en compilation pour le serveur dédié (D231, `C:\UE\src\UE_5.8.3_src`) est celui d'Epic, sans Angelscript**
  (0 plugin) : du gameplay en Angelscript n'y tournerait pas.
- **D263** : hybride dès J2 (cœur C++, gameplay en Angelscript), essai borné d'abord ; moteur du serveur à recompiler
  depuis les sources de Hazelight (note ajoutée à `TMP.md`, la compilation Epic en cours n'est pas touchée).

### 2026-10-13 (suite) — J2, Q5 tranchée (D264) : plan de J2 complet

- Vérification : essai Angelscript d'abord, tests, bout en bout à 2 et 8 joueurs avec clics simulés, essai de
  l'utilisateur. Ordre d'implémentation : J2.0 essai Angelscript, J2.1 base des bâtiments, J2.2 centre principal,
  J2.3 sélection, J2.4 vérification, J2.5 clôture.

### 2026-10-13 (suite) — D263 révisée : pas d'Angelscript

- Question de l'utilisateur : « pourra-t-on tout faire si on choisit AS ? » ; réponse : tout le gameplay oui, le cœur
  Mass et la réplication compacte restent en C++ de toute façon ; coûts : moteur du serveur depuis les sources de
  Hazelight, deux langages, suivi des versions. Décision : **« on abandonne AS »** ; D263 réécrite (tout en C++), D264
  sans essai Angelscript, J2.0 retirée ; note de `TMP.md` retirée (le build Epic 5.8.3 convient au serveur).

### 2026-10-13 (suite) — J2.1 faite : base des bâtiments

- **`Buildings/HOLBuildingTypes`** : `FHOLBuildingTypeRow` (nom affiché en texte traduisible, emprise, hauteur, points
  de vie, maillage final en référence souple ; vide = forme provisoire) et `UHOLBuildingSettings` (réglage
  `BuildingTypesTable` dans `DefaultGame.ini`, recherche d'une ligne avec erreur au journal si elle manque).
- **`Buildings/HOLBuilding`** : acteur répliqué (sans mouvement, toujours pertinent jusqu'au brouillard de M7, une mise
  à jour par seconde) ; seuls le type, le propriétaire et la couleur voyagent, chaque machine construit l'apparence
  depuis la table. Forme provisoire (D261) en formes de base du moteur, chacune ajustée à sa taille par ses bornes :
  bloc de 45 % de la hauteur, dessus du bloc à la couleur du joueur, tour de 40 % de l'emprise sur 80 % de la hauteur,
  toit conique à la couleur ; pas d'ombre (D258) ; formes et matériau en références dures (emballés avec le jeu).
- **Table** : `Tools/Data/BuildingTypes.csv` → `/Game/Buildings/BuildingTypes` par `Tools/Editor/make_building_types.py`
  (calqué sur les unités) ; une ligne `MainCentre` « Centre principal », 15 × 15 m, 10 m, 2 400 points de vie
  (provisoires).
- **Tests** : `Buildings.Types` (ligne présente et valeurs) et `Buildings.Appearance` (acteur répliqué, quatre pièces,
  boîte de 15 × 15 × 10 m posée au sol et centrée, deux pièces à la couleur du joueur, repeintes sans reconstruction).
  Piège : dans un monde de test, `InitializeActorsForPlay` est nécessaire pour que les acteurs soient initialisés.

### 2026-10-13 (suite) — J2.2 en cours ; D265 : rendu sans traitement

- **J2.2** : `AHeroesOfLibertyGameModeBase::StartPlay` fait apparaître un centre principal (`AHOLBuilding`,
  `MainCentre`) sur chaque emplacement occupé, tourné comme l'emplacement, à la couleur de l'emplacement ;
  `AHOLStartSlot::GetGroundLocation` partagé avec la caméra ; repères retirés de `create_match_map.py` (version d'avant
  les repères d'échelle, sans les cylindres), `Plain` recréée. Tests 16/16, `check_camera.ps1` PASS à 2 et 8 joueurs,
  journal `Main centre of slot N (colour N)` pour les 8 emplacements.
- Captures : couleurs **délavées** (exposition automatique éclaircie par le sol sombre). Essais en partie :
  `r.DefaultFeature.AutoExposure.Bias -1` ou exposition automatique coupée → couleurs franches (`expo_*.png`).
- Remarque de l'utilisateur : « le bâtiment en rose semblait avoir une réfraction de lumière alors que je ne veux aucun
  traitement » (c'est un reflet spéculaire du soleil). Sens précisé par une question : **D265**, matériaux mats et aucun
  effet de post-traitement, partout ; consignée directement au GDD (§ 16.6 et § 21) comme D258, et note pour M2 dans
  `ImplementationInProgress.md` (matériau des unités `M_UnitPlayerColor` à rendre mat, mesures à reprendre).

### 2026-10-13 (suite) — J2.2 faite ; D265 appliquée ; forme du centre reprise

- **D265 appliquée** : `DefaultEngine.ini` `[SystemSettings]` (réflexions, réflexions en espace écran, exposition
  automatique, halo, flou de mouvement, reflets d'objectif, vignettage et grain par `r.Tonemapper.Quality=0`,
  aberration chromatique, profondeur de champ, occlusion ambiante : tous à 0) et `[/Script/Engine.RendererSettings]`
  (valeurs par défaut du post-traitement coupées, **compensation d'exposition à 0** : le moteur ajoute +1 cran par
  défaut, ce qui délavait les couleurs). Matériaux faits par `Tools/Editor/make_matte_materials.py` dans
  `/Game/Rendering` : `M_MatteColor` (mat, paramètre `Color`), `M_FlatColor` (non éclairé), `M_MatteGround` (sol gris
  mat, une ligne tous les 10 m) ; `Plain` recréée avec ce sol. Groupes « Réflexions » et « Post-traitement » retirés de
  Réglages > Vidéo (91 textes, traductions refaites).
- **Piège** : un matériau référencé par le C++ (`ConstructorHelpers`) est chargé au démarrage de l'éditeur et protégé ;
  le vider par script (`DeleteAllMaterialExpressions`) fait planter l'éditeur (`!IsRooted()`). Le script ne crée donc
  que les matériaux absents : supprimer le `.uasset` pour en refaire un.
- **Forme du centre**, reprise sur trois remarques de l'utilisateur : « une sorte d'ombre sur le toit » (face du cône
  opposée au soleil) → couleur du joueur non éclairée ; « on ne reconnaît pas le cône, c'est juste moche » → toit plat ;
  toit et dessus du bloc fondus dans la même couleur → dessus du bloc à 55 % de la couleur. Captures `cam_*_start.png`.
- Vérifications : tests 16/16 ; `check_camera.ps1` PASS à 2 et 8 (un échec isolé à 8 joueurs, flèche 27 m au lieu de 40,
  non reproduit au passage suivant : perte de focus de la fenêtre, déjà vue en J1.2) ; `capture_screens.ps1` PASS en
  fr, en, ja (onglet Vidéo sans les deux groupes) ; S1 et S2 courts complets (2 000 unités, registre sans écart,
  0 unité restante ; machine occupée, pas de chiffres retenus).

### 2026-10-13 (suite) — Listes déroulantes dans « Nouvelle partie » (D266), push

- Les 17 commits locaux poussés sur `origin/main` (demande de l'utilisateur ; `TMP.md` reste hors dépôt).
- Demande de l'utilisateur : des listes déroulantes plutôt que les sélecteurs ‹ › pour les paramètres de partie.
  **D266** consignée directement au GDD (§ 15 et § 21). Widget `UHOLDropdown` (`Widgets/HOLDropdown.h`) : bouton du
  menu avec un `UMenuAnchor` qui ouvre la liste dans la fenêtre courante (même échelle que l'interface), valeurs
  grisables (IA, couleurs prises), carré de couleur par valeur ; la navigation reste dans la liste (boucle Haut / Bas),
  Échap et B la ferment sans fermer l'écran. Flèche : le chevron « › » de la police tourné de 90° (la police n'a pas de
  flèche vers le bas). Réglages inchangés (sélecteurs ‹ ›).
- Nouvelle commande d'essai `hol.UI.Focus <widget> [délai]`.
- Vérifications : compile sans avertissement, tests 16/16, `capture_screens.ps1` PASS en fr, en, ja ; essais par
  script (captures `dd_*.png`) au clavier (Entrée ouvre, Bas, Entrée choisit, Échap ferme la liste seule) et à la
  manette (A ouvre, bas en boucle, B ferme la liste seule, A choisit « Humain », couleur Bleu grisée pour le joueur 2).
- **Piège d'essai** : une touche clavier envoyée juste après des touches manette change le mode d'entrée ; Common UI
  remet alors le focus sur la cible de l'écran (la liste se ferme) et la touche atteint l'écran (Échap ferme l'écran,
  A lance la partie). Toujours rester dans un seul mode d'entrée par essai, ou envoyer d'abord une touche du mode voulu.
- Remarque de l'utilisateur : les tests prennent énormément de temps, à accélérer au maximum (à traiter avant J2.3).

### 2026-10-13 (suite) — Vérification rapide (D267)

- Mesures avant : tests d'automatisation ~43 s dont ~40 s de démarrage (les 16 tests prennent moins d'une seconde) ;
  bout en bout du menu ~40 s par langue (démarrage ~12-15 s, attentes fixes ~25 s) ; caméra, deux lancements de même.
- Option A choisie par l'utilisateur : **D267** consignée directement au GDD (§ 21), comme D252.
- Fait : `hol.Test.Run` (`Source/HeroesOfLiberty/Dev/HOLTestSequence.cpp`, catégorie de journal `LogHOLTest`),
  crochets `HOLTestHooks` (fondus en cours, écrans affichés) renseignés par `UHOLRootLayout`, `hol.UI.Language`,
  `Tools/Tests/quick_check.ps1` ; `capture_screens.ps1` et `check_camera.ps1` retirés.
- **Piège** : les piles d'écrans de Common UI ont un **fondu de 0,4 s** ; une capture prise quelques images après
  l'ouverture montre l'écran précédent qui s'efface. Le premier passage disait PASS avec des captures fausses (il ne
  vérifiait que l'existence des fichiers) : d'où `waitui` et le contrôle de l'écran affiché à chaque capture.
- Résultat : `quick_check.ps1` PASS en 30-33 s (jeu fini à ~30 s, tests en parallèle), planches `qc_fr/en/ja.png` relues.

### 2026-10-13 (suite) — J2.3 sélection faite (D262)

- **Canal** `Selection` (`ECC_GameTraceChannel1`, `Config/DefaultEngine.ini`, `HOLCollision.h`) : ignoré par défaut,
  bloqué par les pièces des bâtiments seulement.
- **Touches** : `IA_Select` (clic gauche) et `IA_Deselect` (Échap) dans `IMC_Game` (`make_game_input.py` relancé).
- **Bâtiment** : `SetSelected` (anneau au sol fait à la première sélection : plan du moteur et `M_SelectionRing`, non
  éclairé et masqué, fait par `make_matte_materials.py` ; diamètre 1,15 × la diagonale de l'emprise, ~1 m de large,
  sans collision ni ombre), `SetHovered` (toutes les couleurs × 1,3, anneau compris).
- **Contrôleur** : état de sélection local (`TWeakObjectPtr`), rayon sous le curseur sur le canal, survol à chaque
  image, journal `Selection: MainCentre of slot N (own|other)` ou `none`. Commandes d'essai : `hol.Select.Click X Y`,
  `hol.Select.Report`, `hol.Camera.FocusSlot N`.
- Vérifications : test `Buildings.Selection` (rayon qui touche le centre et rien à côté, anneau absent avant la
  première sélection puis à la couleur du propriétaire, autour de l'emprise, sans collision, caché une fois
  désélectionné, éclaircissement au survol) ; `quick_check.ps1` étendu (son centre, un centre adverse, le vide, Échap,
  à 2 et 8 joueurs, captures `sel_*`) : PASS, 17/17, ~38 s.
- Constat : à la première partie après la création de `M_SelectionRing`, l'anneau s'affiche un instant en carré gris
  (« Shaders en préparation » : compilation à la demande du mode éditeur), puis normalement.
- Coordination : une session « serveur dédié » (M2) attend une machine au calme pour ses mesures S1 / S2 ; prévenue
  quand ce chantier a fini de compiler et de lancer le jeu.

### 2026-10-14 — Passage au moteur AS_3

- La session serveur dédié a passé le projet sur `C:\UE\UE_5.8_AS_3` (5.8.3 avec la cible Server, D231) et l'a
  recompilé entièrement. `quick_check.ps1` prend maintenant le moteur du `.uproject` (registre) ; commandes utiles de ce
  fichier et de `FrontEndInProgress.md` passées sur AS_3. `quick_check.ps1` PASS sur AS_3 (17/17, 35 s).
- J2 inchangé : reste l'essai de l'utilisateur (J2.4), puis la clôture J2.5.

### 2026-10-14 (suite) — J2.4 : session d'essai de l'utilisateur ouverte

- Deux éditeurs d'un autre projet (ProjectShiva, moteur AS_2) ouverts, rien du projet ; session d'essai lancée sur le
  deuxième écran (moteur AS_3). Retours de l'utilisateur attendus.
- Remarques de l'utilisateur : « enlève le hover, pas besoin ; enlève le cercle, pas besoin ». **D259 et D262
  précisées** (pas de retour visuel sur le bâtiment ; le panneau de J3 montrera la sélection).
- Fait : `AHOLBuilding` sans anneau ni survol (plus de `SetSelected`, `SetHovered`, ni couleurs de pierre gardées pour
  l'éclaircissement) ; contrôleur sans survol (plus de rayon à chaque image) ; `hol.Select.Report` n'écrit plus que la
  sélection ; test `Buildings.Selection` réduit au rayon ; `make_matte_materials.py` sans l'anneau, `M_SelectionRing`
  supprimé. La sélection elle-même (clic, vide, Échap, journal) ne change pas.
- Vérifications : compile sans avertissement ; `quick_check.ps1` PASS (17/17, 43 s) ; capture `sel_2p_own` relue (centre
  sélectionné, sans anneau ni teinte). **J2.4 faite.**

### 2026-10-14 (suite) — J2.5 faite : J2 close

- D259 à D264 reportées au journal du GDD (§ 21, entre D258 et D265), D259 et D262 avec leur précision d'essai ;
  paragraphe « Parcours J2 » au § 16.6, après celui de J1. **J2 close.** Prochaine étape du parcours : M2.5 (réseau).

### 2026-10-14 (suite) — Push ; plan de M2.5 ouvert

- `main` poussé sur `origin/main`.
- Choix de l'utilisateur pour la suite : **planifier M2.5 ici, sans code**, le code attendant la clôture de M2 (S1 / S2
  au repos sur 5.8.3 ; deux éditeurs d'un autre projet tournaient). Plan en cinq questions (tableau « Plan de M2.5 ») ;
  Q1 posée.

### 2026-10-14 (suite) — M2.5, Q1 tranchée (D268)

- Partie à 2 clients sur `Plain` et S5, serveur dédié et clients lancés par script, rejoint par `open <adresse>` ; menu
  et solo inchangés. Ordre de grandeur pour Q2 : un envoi direct (≈ 9 octets par unité, 10 fois par seconde) coûte
  ≈ 180 Ko/s à 2 000 unités, pour un seuil de 32 Ko/s (≈ 16 octets par unité et par seconde). Question « transport »
  (Q2b) séparée de Q2.

### 2026-10-14 (suite) — M2.5, Q2 tranchée (D269)

- Position et vitesse réduites, prolongées par le client, envoi au-delà d'un seuil d'erreur ou à un changement d'action,
  rappel lent.

### 2026-10-14 (suite) — M2.5, Q2b tranchée (D270)

- Constat dans le moteur 5.8.3 : `MassReplication` (dans `MassGameplay`, marqué expérimental) prépare une bulle par
  client et choisit selon la vue du joueur ; Iris prend en charge les tableaux rapides (`IrisFastArraySerializer.h`).
- Tableau rapide maison sur un seul acteur partagé ; repli sur des paquets maison non garantis.

### 2026-10-14 (suite) — M2.5, Q3 tranchée (D271)

- Entités Mass d'affichage chez le client, instant présent estimé, corrections lissées en ~0,2 s.

### 2026-10-14 (suite) — M2.5, Q4 tranchée (D272)

- Le solo reste autonome ; un seul code d'affichage ; chemin réseau vérifié à 2 clients à chaque étape.

### 2026-10-14 (suite) — M2.5, Q5 tranchée (D273) : plan de M2.5 complet

- Vérification : tests des règles pures, S5 avec bande passante, envois et écart affiché, partie à 2 clients dans
  `quick_check.ps1`, essai de l'utilisateur.
- Ordre d'implémentation : M2.5.1 lancement et partie à 2 clients, M2.5.2 règles pures, M2.5.3 serveur, M2.5.4 client,
  M2.5.5 S5, M2.5.6 vérification rapide et essai, M2.5.7 clôture. **Le code attend la clôture de M2.**
- Ligne M2.5 de `ImplementationInProgress.md` renvoyée à ce fichier.

### 2026-10-14 (suite) — M2 clos ; M2.5.1 commencée

- L'utilisateur garde un éditeur d'un autre projet ouvert et demande de considérer la machine comme libre. Série S1 / S2
  sur 5.8.3 (deux passages, deux vues) : **tous les seuils tenus**, recouvrements identiques à 5.8.2 ; détail et
  nouvelle référence dans `ImplementationInProgress.md`. **M2 clos** (tableau du § 16.6 du GDD mis à jour).

### 2026-10-14 (suite) — M2.5.1 faite : partie à 2 clients sur un serveur dédié

- **Serveur cuit et mis en place** (`RunUAT BuildCookRun -server -noclient -build -cook -stage`, cartes `Plain` et
  `StressTest`, 1 min 48 s) : `Saved/StagedBuilds/WindowsServer/HeroesOfLiberty/Binaries/Win64/HeroesOfLibertyServer.exe`.
  À refaire après tout changement de code, de contenu ou de configuration lu par le serveur (`net_match.ps1 -Stage`).
- **`Tools/Tests/net_match.ps1`** : configuration JSON à 2 humains (emplacements 0 et 1), serveur dédié réduit dans la
  barre des tâches, attente de l'écoute (port 7777), puis 2 clients (éditeur en `-game`, contenu non cuit) qui
  rejoignent `127.0.0.1`, côte à côte sur le deuxième écran ; chaque client : capture interne `net_client<N>`, clic au
  milieu de l'écran (son centre), rapport de sélection. Journaux `Saved_Tests/Logs/net_server.log`, `net_client<N>.log`.
- Résultat : le mode de jeu existant suffit — chaque client reçoit le premier emplacement humain libre (`holds slot 0`
  puis `1`, dans l'ordre d'arrivée), sa caméra se place au-dessus de son emplacement, les centres répliqués arrivent à la
  bonne couleur (rouge pour l'emplacement 1, bleu pour le 0, captures relues) et le clic sélectionne son centre.
  Client en éditeur non cuit contre serveur cuit : accepté sans erreur.
- **Correction** : Iris demande `net.SubObjects.DefaultUseSubObjectReplicationList=1` (`[SystemSettings]`) ; sans lui,
  alertes au démarrage et sous-objets de `GameStateBase` non répliqués. Ajouté, plus aucune alerte. Reste un
  avertissement d'Iris sans effet constaté (`BroadcastLoadedModulesUpdated() called while there are 1 active
  ReplicationSystems`, modules chargés après le démarrage du système de réplication).
- **Remarque de l'utilisateur** : les fenêtres des clients s'empilaient. Cause : le moteur recentre une fenêtre sur
  l'écran enregistré (`DisplayIndex` des réglages) quand il applique la résolution, ce qui annule `-WinX`. Correction :
  chaque client a son propre dossier (`-saveddirsuffix=Tests_Net<N>`, `Saved_Tests_Net<N>`) avec des réglages tirés de
  `Tools/Tests/GameUserSettings.ini`, taille de la fenêtre et `DisplayIndex=-1` ; vérifié par les rectangles des deux
  fenêtres (x 3 832-4 788 et 4 792-5 748).
- Note : `Saved_Tests/Config/WindowsEditor/GameUserSettings.ini` (réglages partagés des essais) dit maintenant
  1920 x 1080 et `DisplayIndex=0` (réécrit par une session de mesure), au lieu du modèle 1600 x 900 sur l'écran 1.

### 2026-10-14 (suite) — M2.5.2 faite : règles pures de la réplication des unités

- **`Net/HOLUnitReplicationRules.h/.cpp`** (nouveau dossier `Net`), sans état du moteur, communes au serveur et au
  client (le serveur mesure l'écart exact que le client montrerait) :
  - **État réduit** `FHOLNetUnitState` : position à 10 cm (X, Y sur 15 bits, ±1 638 m ; Z sur 12 bits, ±204 m),
    vitesse **horizontale** à 5 cm/s (9 bits par axe, ±12,8 m/s), action (3 bits), 8 bits bas du pas (12,8 s à
    20 Hz) : **71 bits, ~9 octets** (D269 estimait ~8 avec l'identifiant ; S5 mesurera le surcoût d'Iris par
    élément). Pas de vitesse verticale : les unités suivent le sol du navmesh, une hauteur qui dérive en pente est
    rattrapée par la règle d'envoi. Valeurs hors bornes tenues au bord.
  - `UnwrapStep` (pas complet depuis ses 8 bits bas et l'horloge estimée), `PredictPosition` (ligne droite).
  - **Règle d'envoi** `GetSendReason` : changement d'action, sinon écart prolongé > 25 cm, sinon **rappel lent**.
    Écart à D269 sur le rappel : il tombe au **pas propre à l'unité** d'un cycle de 2 s (fixé par son identifiant),
    sauté si elle a été envoyée moins d'un quart de cycle avant. Une première version (« au plus un cycle ») aurait
    envoyé ~1 000 unités au même pas puis les aurait gardées en phase : le test des 2 000 unités l'a montré avant
    d'écrire. Unité au repos : un envoi toutes les 2 s, réparti ; attente au plus **2,5 s** (au lieu de ~2 s), sans
    conséquence puisque le tableau rapide d'Iris livre de toute façon le dernier état (D270).
  - **Horloge du serveur** `FHOLServerClock` (D271) : échantillon = pas reçu + délai aller (moitié du ping) ; suit
    d'un dixième de l'écart par échantillon, saute à l'échantillon s'il est à plus de 0,25 s.
  - **Correction lissée** `FHOLDisplayCorrection` : écart entre position affichée et nouvelle prédiction, effacé
    linéairement en 0,2 s ; au-delà de 5 m, pas de lissage (placement direct).
- **Tests** `Tests/HOLNetTests.cpp` : `HeroesOfLiberty.Net.Reduction`, `.UnwrapStep`, `.Prediction`, `.SendRule`,
  `.ServerClock`, `.Correction` — tous passent. Mesures du test de la règle : unité qui tourne en rond (cercle de
  20 m à 3 m/s) **1,3 envoi/s, écart au plus 24,7 cm** ; 2 000 unités au repos nées ensemble : **au plus 50 envois
  par pas** (la part égale), 0,48 envoi par unité et par seconde.
- `quick_check.ps1` : un premier passage juste après la compilation a échoué sur les parties caméra (lancement du
  jeu fini à 76 s au lieu de ~30 s ; journal écrasé par le passage suivant, cause non établie) ; le second :
  **PASS en 32 s, 23 tests sur 23**. À surveiller si un premier lancement après compilation échoue encore.

### 2026-10-14 (suite) — M2.5.3 faite : côté serveur de la réplication des unités

- **Constat dans Iris (5.8.3)** : un tableau rapide (`FIrisFastArraySerializer`) n'a que **63 bits de changement**,
  l'élément i sur le bit i mod 63 (`IrisFastArraySerializer.cpp`, `ArrayPropertyNetSerializer.cpp`) : dans un seul
  tableau de 2 000 unités, marquer une unité renverrait les ~32 qui partagent son bit. La compression par écart d'Iris
  n'y remédie pas (nouvelle base au plus toutes les 60 images, `net.Iris.MinimumNumberOfFramesBetweenBaselines`).
- **Mise en œuvre de D270 qui en tient compte** (même principe : un acteur partagé, tableaux rapides, marquage selon
  D269, format maison) : `Net/HOLUnitReplicator.h/.cpp`.
  - `AHOLUnitReplicator` : l'acteur partagé, toujours pertinent, 20 mises à jour par seconde (réglage), porte le
    **numéro du pas** du serveur et des **morceaux** `UHOLUnitChunk` (sous-objets répliqués) de **63 emplacements
    exactement** : chaque unité a son bit, Iris n'envoie que les unités marquées et renvoie ce qui s'est perdu.
  - Dans un morceau, deux tableaux rapides : **identité** (identifiant, type, propriétaire ; écrite à l'arrivée, au
    départ et à une conversion) et **état** (`FHOLNetUnitState`, écrit seulement quand la règle d'envoi le dit).
    Emplacements fixes : une unité partie libère le sien, rien n'est retiré, les indices ne bougent pas.
  - Type envoyé comme **indice** dans la liste triée des types de la table (`UHOLUnitSubsystem::GetTypeNames`).
  - **Sérialiseur Iris** `Net/HOLNetUnitStateNetSerializer` : l'état sur ses 71 bits exacts (au lieu de 16 bits par
    entier). Test `HeroesOfLiberty.Net.ElementFormat` : aller-retour par Iris, 71 bits, valeurs aux bornes.
- **`Net/HOLUnitReplicationSubsystem`** (serveur d'une partie en réseau seulement, D272) : à la fin de chaque pas
  (nouveau `UHOLSimulationSubsystem::GetOnStepEnded`, hors du chrono du pas), requête Mass sur toutes les unités,
  règle d'envoi, emplacement pris ou libéré. Réglages `UHOLUnitReplicationSettings` (seuil, cycle du rappel, mises à
  jour par seconde) ; `hol.Net.ErrorThreshold`, `hol.Net.ReportInterval`, `hol.Net.Report` (serveur : envois par
  raison, coût, octets envoyés par client ; client : unités reçues, pas du serveur, octets reçus). Catégorie de
  journal `LogHOLNet`.
- **`net_match.ps1 -Units N`** : serveur sur `StressTest`, S2 lancé 15 s après le démarrage (clients entrés), rapport
  du serveur toutes les 5 s, deux rapports par client.
- **Essai à 2 clients, 2 000 unités en marche (S2, 40 s)** :
  - les deux clients reçoivent **2 000 unités en 32 morceaux**, le pas du serveur suit ;
  - **1,48 à 1,58 envoi par unité et par seconde** (estimation de D269 : ~1,5) : sur 5 s, ~11 000 pour l'écart,
    ~2 000 pour l'action (S2 relance sans cesse des ordres), ~1 800 rappels ;
  - coût au serveur : **0,08 ms par pas** ;
  - **débit : ~42 Ko/s par client**, au-dessus des 32 Ko/s de D218. Les états eux-mêmes font ~27 Ko/s
    (3 000 × 71 bits) ; le reste (~15 Ko/s, ~24 octets par morceau et par mise à jour) est surtout le masque de
    changement que chaque morceau marqué envoie (1 + 64 + 64 bits) et son en-tête, 32 morceaux × 20 fois par
    seconde. Pistes pour M2.5.5 : **10 mises à jour par seconde** (moitié de ce surcoût, ~34 Ko/s estimés),
    **identités dans des morceaux à part** (masque de 64 bits au lieu de 129), puis le seuil. S5 tranchera.
- Rien ne s'affiche encore chez les clients (M2.5.4). Avertissement d'Iris déjà noté en M2.5.1 (modules chargés après
  le démarrage) seul, sans autre alerte. `quick_check.ps1` : **PASS en 33 s, 24 tests sur 24**.

### 2026-10-14 (suite) — M2.5.4 faite : affichage des unités chez le client

- **Gabarit d'affichage** : `UHOLUnitSubsystem::FindTemplate(Type, EHOLUnitTemplateKind::Display)` construit pour
  chaque type un gabarit **sans simulation** : trait de l'unité, trait d'affichage réseau, LOD et visualisation de
  M2 (mêmes instances animées, même couleur D233) ; ni déplacement, ni évitement, ni navigation.
- **`Net/HOLUnitReceptionSubsystem`** (client seulement) : chaque image, lit les morceaux de l'acteur partagé ;
  une unité qui arrive dans un emplacement devient une entité d'affichage (créées par lots, par type), une unité
  partie est détruite, un changement de propriétaire suit (D37) ; nourrit l'**horloge du serveur** à chaque nouveau
  pas reçu (délai aller = moitié de `AvgLag` de la connexion) ; à chaque **nouvel état**, la correction démarre de
  l'endroit où l'unité est dessinée (ancien état prolongé + ancienne correction) vers le nouvel état prolongé.
- **`Net/HOLNetUnitDisplay`** : fragment `FHOLNetUnitDisplayFragment` (dernier état, son pas complet, correction,
  vitesse, orientation), trait, et processeur par image `UHOLNetUnitDisplayProcessor` (client, avant le LOD et la
  représentation) : position = dernier état prolongé jusqu'à l'horloge estimée (au plus 3 s) + correction lissée ;
  **orientation** tournée vers la marche (540°/s, au-dessus de 20 cm/s), l'orientation n'étant pas envoyée. Le
  dessin de M2 lit ensuite la transformation comme pour une unité simulée (D272 : un seul code d'affichage).
- **Processeur d'affichage de M2** (`HOLInterpolatedInstancesProcessor`) : pour une entité d'affichage, l'allure de
  l'animation vient de la **vitesse reçue** (pas de pas fixe chez le client pour la mesurer) ; inchangé en solo.
- `hol.Net.Report` côté client ajoute une ligne « Net (display) » : unités affichées, états reçus par seconde,
  correction moyenne et maximale, placements directs, avance de l'horloge sur le dernier pas reçu.
- Test `HeroesOfLiberty.Net.DisplayedPosition` (prolongement borné à 3 s, pas de saut à un nouvel état, nouvel
  état rejoint en 0,2 s).
- **Essai à 2 clients, 2 000 unités en marche** (`net_match.ps1 -Stage -Units 2000`) : chaque client affiche **les
  2 000 unités** (capture : mannequins aux couleurs des joueurs, en marche, tournés dans leur sens de marche) ;
  ~2 650 à 2 970 états reçus par seconde ; **correction moyenne 24 cm, au plus ~46 cm, aucun placement direct**
  (attendu : la plupart des envois partent quand l'écart passe 25 cm) ; horloge 0,5 à 1,2 pas en avance sur le
  dernier pas reçu. Serveur inchangé : 1,5 envoi par unité et par seconde, ~41 Ko/s par client.
- `quick_check.ps1` : **PASS en 33 s, 25 tests sur 25**. Le dessin commun ayant changé, **S2 court en solo**
  (2 000 unités, 10 s) : simulation 2,35 ms par pas, 204 i/s (1 % bas 76), **PASS**.

### 2026-10-14 (suite) — M2.5.5 en cours : S5 mesuré, horloge corrigée, débit encore trop haut

- **Rapport S5 complet** : `net_match.ps1 -Units 2000` attend le rapport de S2, ferme les clients et lance
  `Tools/Tests/s5_report.py` (PASS / FAIL). Octets par client **moyennés sur les totaux** des connexions (l'ancien
  chiffre `OutBytesPerSecond` ne valait que la dernière seconde), paquets/s, états réellement écrits sur le fil
  (compteur dans le sérialiseur), coût du pas, images par seconde des clients, **écart affiché** (`hol.Net.Sample 100` :
  le serveur note les vraies positions de 20 unités à chaque pas, les clients les positions affichées 4 fois par
  seconde ; comparées sur l'horloge commune de la machine) et **avance de l'horloge** des clients. Réglages d'essai :
  `-UpdateFrequency`, `-Threshold`, `-RefreshSteps`, `-Lag`, `-Loss` (`hol.Net.UpdateFrequency`, `hol.Net.RefreshSteps`).
- **Serveur cuit avec `-pak`** : sans lui, le serveur lisait ses données dans Zen avec le mot de passe de l'instance
  du moment ; l'éditeur de l'autre projet relancé (nouvelle instance de Zen), le serveur attendait sans fin.
- **Horloge du serveur corrigée** (`FHOLServerClock`) : après une image de 0,5 s chez le client (capture), le premier
  pas lu était en retard et l'horloge **sautait de 11 pas en arrière** (erreurs affichées jusqu'à 190 cm). Un
  échantillon très en retard est maintenant ignoré, sauf s'il dure (10 de suite) ; un essai « enveloppe basse »
  (suivre les plus précoces) mettait l'horloge un demi-pas en avance, abandonné. Test `ServerClock` étendu.
- Mesures (2 000 unités en marche, S2, 40 s) :

  | Réglage | Envois/unité/s | Ko/s par client | Écart affiché moyen (95 %) |
  |---|---|---|---|
  | 20 mises à jour/s, 25 cm | 1,49 | 41,0 | 15,0 cm (27) |
  | 10 mises à jour/s, 25 cm | 1,49 | 36,1 | 16,5 cm (32) |
  | 10 mises à jour/s, 35 cm | 1,30 | 31,6 | 20,4 cm (41) |
  | 10/s, vitesse moyennée sur 4 pas | 1,61 | 39,0 | 17,0 cm — abandonné |

- **Constat** : à 10/s, **30,6 Ko/s sont les états eux-mêmes** (3 449 états sur le fil par client et par seconde ×
  71 bits), le reste (en-têtes des morceaux, masques, paquets) seulement 5,4 Ko/s. Il faut donc moins d'états, pas
  moins d'en-têtes. Anomalie : **3 449 états sur le fil pour ~2 980 écrits** par seconde, alors qu'à 10/s des
  écritures devraient se fondre ; à comprendre d'abord.
- Reste vu : un écart maximal de ~2 m isolé (à expliquer), images par seconde des clients côte à côte ~130-180
  (1 % bas ~10-17, captures comprises). Simulation du serveur ~2,7-3,0 ms par pas, réplication ~0,17 ms.

### 2026-10-14 (suite) — M2.5.5 : anomalie expliquée, morceaux en acteurs, S5 PASS à 35 cm (seuil à trancher)

- **Anomalie expliquée** (lecture d'Iris, `ReplicationWriter.cpp`) : Iris écrit un acteur et **tous ses sous-objets en
  un seul lot** ; à 10/s, une mise à jour (~300 états, ~2,7 Ko) dépassait un paquet. Le lot était annulé, réécrit dans
  un tampon et envoyé par le **chemin des objets énormes** (morceaux fiables, mise à jour suivante bloquée jusqu'à
  l'accusé de réception de tout). Le compteur du serveur comptait donc des écritures annulées : ce n'étaient pas des
  états en trop sur le fil.
- **Morceaux en acteurs** : `AHOLUnitChunk` (63 emplacements, un acteur répliqué toujours pertinent, propriétaire
  l'acteur partagé) remplace le sous-objet `UHOLUnitChunk` ; chaque morceau est son propre lot et tient dans un paquet.
  `AHOLUnitReplicator` ne porte plus que le pas du serveur (et la liste des morceaux, côté serveur). Chez le client, un
  morceau s'inscrit auprès de `UHOLUnitReceptionSubsystem` à son `BeginPlay`. Principe de D270 inchangé (tableaux
  rapides, marquage selon D269, format maison, préparés une fois pour tous les clients).
- **Mesure exacte chez le client** : compteur dans `Deserialize` (« états lus sur le fil ») ; `s5_report.py` en tire
  la part des états et le reste. `hol.Net.WhenClients <N> <commande>` (serveur) : S2 part dès que les clients sont dans
  la partie (un client lent à démarrer faussait la mesure) ; `net_match.ps1` s'en sert.
- Mesures (2 000 unités en marche, S2, 40 s, 10 mises à jour/s, serveur restagé) :

  | Réglage | Envois/unité/s | États lus/s | Ko/s par client | Écart affiché moyen (95 %) |
  |---|---|---|---|---|
  | 25 cm | 1,48 | 2 810 | 34,1 | 15,2 cm (31) |
  | 30 cm | 1,38 | 2 617 | 32,5 | 17,3 cm (36) |
  | **35 cm** | 1,29 | 2 425 | **30,1** | 18,9 cm (41) — **S5 PASS** |
  | 25 cm, 4 suites de 63 par acteur | 1,49 | 2 824 | 34,7 | 15,3 cm — abandonné |

  Horloge des clients stable (±0,25 pas, au lieu de sauts de 3-4 pas), aucun placement direct, réplication 0,15 ms par
  pas au serveur. Trafic fixe d'une partie à 2 clients (63 unités) : ~1,3 Ko/s.
- **Reste non expliqué** : ~8 Ko/s qui suivent le nombre d'écritures de tableaux (~30 octets chacune), alors que la
  lecture d'Iris en prévoit ~10-15 (masque codé par indices, nombre d'éléments, en-tête du lot). Regrouper 4 suites
  par acteur n'a rien gagné (et a multiplié les lots annulés en bout de paquet) : retiré. Seule une trace réseau
  d'Iris (Unreal Insights, `-NetTrace`) le dirait.
- **Écueil** : remplacer seulement l'exécutable du serveur sur une ancienne cuisson a fait planter le serveur
  (`GetPopulationByOwner` → `UDataTable::FindRow`, table des unités changée par J3) : toujours `-Stage` après un
  changement du code des autres. *Corrigé le 2026-10-14 : cause probable autre, la table des types mise en cache par
  `UHOLUnitSubsystem` n'était pas une `UPROPERTY` et le ramasse-miettes la libérait (~60 s) ; corrigé par J3 (0fb58ea).
  Recuire après un changement de code reste nécessaire.*
- `quick_check.ps1` : **PASS en 32 s, tests 28/28**. À trancher par l'utilisateur : le seuil (35 cm passe ; D269
  disait ~25 cm « réglable »).
- **D274** (choix de l'utilisateur, conforme à la recommandation) : **35 cm et 10 mises à jour par seconde** par
  défaut (`UHOLUnitReplicationSettings`). S5 sur les valeurs par défaut, sans réglage passé au script : **PASS,
  30,3 Ko/s par client**, 1,30 envoi par unité et par seconde, écart affiché moyen **19,7 cm** (95 % : 40,6 cm),
  aucun placement direct, horloge à ±0,3 pas. Test `Net.SendRule` : les poussées essayées suivent le seuil (±5 cm)
  au lieu de 20 et 30 cm écrits en dur. `quick_check.ps1` : tests **28/28** ; la partie « production » de J3 (autre
  session, en cours) échoue sur son propre minutage. **M2.5.5 faite.**

### 2026-10-14 (suite) — M2.5.7 avancée : D268 à D274 au GDD

- Pendant que J3 avance dans l'autre session, report au GDD sans compiler : **D268 à D274 au § 21** (texte du tableau
  des décisions ci-dessus, colonne « § 16.6 ») et **paragraphe « Tranche réseau M2.5 »** au § 16.6, après « Parcours
  J2 » : lancement par script, règle d'envoi, morceaux de 63 emplacements en acteurs, affichage chez le client, solo
  autonome, valeurs de D274 et chiffres de S5. Marqué « en cours » tant que M2.5.6 n'est pas faite.
- Reste pour clore M2.5.7 : après M2.5.6, enlever « en cours » du paragraphe et compléter la ligne M2.5 du tableau des
  étapes du § 16.6 (« Fait le … »).

### 2026-10-14 (suite) — M2.5.6 : partie à 2 clients dans `quick_check.ps1` (D275)

- **D275** (choix de l'utilisateur, conforme à la recommandation) : le serveur de la partie « net » est l'**éditeur en
  `-server`** (pas de cuisson, toujours à jour) ; le serveur cuit reste pour S5 et les essais.
- **Partie « net »** de `quick_check.ps1` (dans les parties par défaut) : le serveur part au début du script
  (`Plain`, 2 humains, `-saveddirsuffix=Tests_NetSrv`, journal `Saved_Tests/Logs/quick_net_server.log`), en parallèle
  des tests et du lancement solo ; dès qu'il écoute, 2 clients hors écran (côte à côte sous la fenêtre solo avec
  `-Visible`), chacun dans son dossier `Saved_Tests_Net<N>`. Chaque client : caméra au-dessus de l'emplacement tenu,
  `hol.Buildings.Report`, clic sur le centre de l'emplacement 0 puis 1 (l'un « own », l'autre « other »), clic dans le
  vide ; captures `qcnet_<N>_start/slot0/slot1`. Contrôles : séquence finie, emplacements 0 et 1 tenus, caméra à ±2 cm,
  les 2 centres reçus chacun à sa couleur, les 3 rapports de sélection, aucune erreur de partie, de réseau ou de
  séquence chez les clients et au serveur.
- **`hol.Buildings.Report`** (nouveau `Buildings/HOLBuildingReport.cpp`) : bâtiments reçus par la machine, emplacement,
  indice de couleur et « shown » si une pièce porte bien cette couleur (toit de la tour).
- **`waitplayers <N>`** (nouveau verbe de `hol.Test.Run`) : attend que l'état de partie connaisse N joueurs. Sans lui, un
  client fini et parti libérait l'emplacement 0, que l'autre client, plus lent, recevait à son tour.
- Écueils : `waitlog` ne lit que les lignes écrites depuis la dernière commande (rien entre `waitmap` et lui) ; le
  curseur de test doit être fixé dès le démarrage (`-ExecCmds`), sinon la caméra défile au bord avant la première
  étape (5 m mesurés).
- Résultat : `quick_check.ps1` **PASS en 44 s** (au lieu de ~32 s), tests 28/28, toutes les parties (dont
  « production » de J3) ; capture relue : le client de l'emplacement 1 voit le centre bleu de l'emplacement 0, sélectionné
  comme « Joueur 1 ». Reste l'essai de l'utilisateur.

### 2026-10-14 (suite) — M2.5.6 : essai de l'utilisateur sur `Plain`, retard des ressources corrigé

- Serveur recuit (`net_match.ps1 -Stage`, avec la production de J3 et le correctif du ramasse-miettes 0fb58ea), deux
  clients côte à côte sur `Plain`.
- **Remarque de l'utilisateur** : « énorme latence » de la barre des ressources quand il met un Paysan en file ou
  l'annule. Cause : un `APlayerState` n'envoie qu'**une fois par seconde** (`SetNetUpdateFrequency(1)` du moteur),
  alors que la file du centre est poussée tout de suite (`ForceNetUpdate`). Correction : `ForceNetUpdate()` dans
  `AHOLPlayerState::SetResources` et `SetPopulation`, après leurs retours anticipés (la population est écrite à chaque
  pas mais change rarement). Recompilé, recuit, relancé : **« tout est bon »**.
- Règle pour la suite : toute donnée d'un `PlayerState` que le joueur voit changer à son geste doit pousser sa mise à
  jour (ou le `PlayerState` recevoir une fréquence plus haute) ; la récolte de J5 changera les ressources souvent.
- Reste : l'essai sur S5.

### 2026-10-14 (suite) — M2.5.6 et M2.5.7 faites : M2.5 close

- **Essai sur S5** (`net_match.ps1 -Units 2000 -Seconds 600 -Keep`) : 2 000 unités en marche reçues par les deux
  clients ; validé par l'utilisateur (« oui » : fluidité, couleurs, mêmes positions des deux côtés). Remarque : 10
  minutes de marche, c'est trop long ; pour un essai, `-Seconds 60`.
- **M2.5.7** : paragraphe « Tranche réseau M2.5 » du § 16.6 marqué fait le 2026-10-14, ligne M2.5 du tableau des étapes
  complétée. **M2.5 close.**


### 2026-10-14 (suite) — J3, plan ouvert ; Q1 tranchée (D280)

- La fin de M2.5 (M2.5.5 à M2.5.7) continue dans une autre session ; le plan de J3 est mené ici en parallèle. Décisions
  de J3 numérotées **à partir de D280** (D274 à D279 laissées à M2.5). Plan de J3 : Q1 périmètre, Q2 HUD, Q3 production
  côté serveur, Q4 le Paysan, Q5 vérification et sous-étapes.
- **Q1 → D280** : production avec coût (option A), découpée en plusieurs sous-étapes, fixées à la fin du plan.
- **Q2 → D281** : écran de jeu Common UI en C++, seuls les blocs utiles à leur place de D201.
- **Q3 → D282** : le serveur seul décide (file de commandes, coût à la mise en file, population au début), état répliqué au propriétaire.
- **Q4 → D283** : `Peasant` au mannequin de M2, en arc devant le centre, valeurs provisoires en tables.
- **Q5 → D284** : tests, solo et 2 clients dans `quick_check.ps1`, essai, S1 et S2 courts. **Plan de J3 complet**, découpé en J3.1 à J3.6.
- Suite : **J3.1**. Cohabitation : l'autre session code M2.5 dans le même module ; ne compiler que si elle ne compile pas, commits par chemins explicites, ne pas toucher aux fichiers `Net/` qu'elle modifie.

### 2026-10-14 (suite) — J3.1 faite : données et règles pures de la production

- **Tables** : colonnes `Cost` (`FHOLResources`, nouveau `Match/HOLResources.h`), `BuildTime` (s) et `Population` dans
  `UnitTypes.csv`, ligne **`Peasant`** (mannequin de M2, 50 nourriture, 20 s, 1 place ; rangé en dernier par ordre
  alphabétique, donc les index de type envoyés aux clients, D270, ne bougent pas) ; colonnes `PopulationProvided` (10),
  `Produces` (`Peasant`) et `MaxQueue` (5) dans `BuildingTypes.csv` ; tables refaites par leurs scripts.
- **Stock de départ** : `UHOLMatchSettings::StartingResources` (200 de chaque, `DefaultGame.ini`), hors de la
  configuration de partie (D244 : pas de réglage de salon sans sa décision).
- **Règles pures** `Buildings/HOLProductionRules` : `TryEnqueue` (file pleine, ressources insuffisantes, débit),
  `Cancel` (remboursement entier), `Advance` (la tête démarre s'il reste de la place dans la population, sinon attend ;
  finie après `BuildSteps` pas, la suivante démarre au même pas), `GetStartedPopulation`, `GetProgress`,
  `GetPopulationCap` (175 au plus). Tests `HeroesOfLiberty.Production.Queue`, `.Advance`, `.Tables`.
- Vu en explorant, pour la suite : (1) la couleur des unités vient de `OwnerPlayer` pris comme index de palette, pas de
  la couleur choisie pour l'emplacement (`ColorIndex`, celle des bâtiments) : à corriger en J3.3 pour que le Paysan ait
  la couleur de son centre ; (2) aucun propriétaire réseau sur les bâtiments : à poser pour la réplication au seul
  propriétaire (D282) ; (3) les emplacements IA n'ont pas de `PlayerState` : seuls les humains ont une économie pour
  l'instant ; (4) `quick_check.ps1` attend « no interface » sur les captures de partie : à adapter en J3.4.
- Suite : **J3.2**.

### 2026-10-14 (suite) — J3.2 et J3.3 faites : économie, file de production, Paysan, sans HUD

- **Économie** (`AHOLPlayerState`) : stock, population utilisée et plafond, `COND_OwnerOnly`, délégué
  `OnEconomyChanged` ; stock de départ donné au `PostLogin`. `UHOLUnitSubsystem` garde le propriétaire de chaque unité
  et donne la population par propriétaire (naissances en attente comprises).
- **File** sur `AHOLBuilding` (`COND_OwnerOnly`, `OnProductionChanged`) ; le mode de jeu donne aux bâtiments d'un
  emplacement le **contrôleur de son joueur comme propriétaire réseau** (à l'apparition et au `PostLogin`).
- **Ordres** : `ServerQueueProduction` / `ServerCancelProduction` (contrôleur) → `FHOLProductionCommand` dans la file de
  commandes → `UHOLProductionSubsystem`, au **début de chaque pas** (`OnStepStarted`, serveur ou solo) : vérifie le
  propriétaire, le type produit, la file et le coût, compte population et plafond, avance les files ; un Paysan fini
  naît au même pas, **en arc devant le centre**, du côté du milieu de la carte. Grille : 12 actions `IA_Grid_00..11`
  (A Z E R / Q S D F / W X C V) dans `IMC_Game` ; la case N produit le N-ième type de `Produces`.
- **HUD** : le contrôleur local remplace le HUD par défaut par `/Script/HeroesOfLibertyUI.HOLGameHUD` (chemin souple,
  le serveur dédié ne charge pas le module d'interface).
- **Vérifications sans fenêtre** (consigne de l'utilisateur : le plus discret possible, `-nullrhi -nosound`) :
  solo sur `Plain` (`Saved_Tests/prod_solo.seq`) : 3 Paysans mis en file (200 → 50), le 3e annulé (→ 100), Paysans finis
  aux pas 402 et 802 (20 s), population 2/10 ; **2 clients** sur un serveur lancé par l'éditeur (`-server -nullrhi`,
  sans recuire le serveur dédié) : le client 0 voit son stock et sa file, le client 1 garde 200 de chaque, **les deux
  affichent le Paysan**. Tests 28/28.
- Reste connu : couleur des unités = `OwnerPlayer` comme index de palette (le Paysan n'a la couleur de son centre que si
  l'emplacement garde sa couleur par défaut) ; refus d'un ordre seulement journalisé (le HUD grise la case).

### 2026-10-14 (suite) — J3.4 faite (HUD) ; J3.5 en cours ; `quick_check` hors écran

- **HUD** (sous-agent, module d'interface seul) : `AHOLGameHUD` crée `UHOLGameScreen` (Common UI, arbre en C++, mode
  d'entrée All, pas de gestion de Retour : Échap désélectionne toujours, aucun focus clavier) ; barre du haut (4
  ressources, population), bloc de sélection (nom, « Vous » / « Joueur N » et sa couleur, points de vie), grille 3 × 4
  avec ses lettres (seule la case A active, grisée si stock insuffisant ou file pleine), file au-dessus (barre de la tête
  calculée à chaque image, « Population pleine », « Clic : annuler »). Un clic sur un panneau s'arrête au HUD, ailleurs il
  atteint le jeu. Noms localisés par type (`HOLGameNames` : Paysan, Centre principal). 12 textes traduits (`fill_po.py`,
  deux collectes). `DescribeInterface` vaut `hud=HOLGameScreen` en partie (attendu par `quick_check`).
- **`quick_check` hors écran** par défaut (`-RenderOffScreen`, aucune fenêtre, consigne de l'utilisateur ; `-Visible`
  pour l'ancien comportement) ; nouvelle partie **production** (2 joueurs) : clic sur la case, touche A deux fois, clic
  sur la file (annulation remboursée), deux Paysans nés (`hol.Prod.BuildTime 2` : 2 s au lieu de 20). Le journal d'une
  naissance porte « (N born here) » (deux `waitlog` identiques se suivant relisaient la même ligne). PASS complet en 38 s.
- `AdvanceQueues` sort tout de suite sur une carte sans bâtiment (stress test) ; une recherche de ligne par type dans
  `GetPopulationByOwner` (la session M2.5 avait vu un plantage du serveur dédié dans `FindRow` sur une ancienne cuisson).
- **Vu sur les captures** : les Paysans nés (population 2/10, 3/10) **ne s'affichent pas** sur `Plain` en solo, même
  dézoomé. À comprendre avant de clore J3.5. Note : hors écran, sans `hol.Camera.TestCursor 0.5 0.5`, le vrai curseur est
  au bord et le défilement emporte la caméra.

### 2026-10-14 (suite) — Paysans « invisibles » : nés derrière le centre

- Nouvelle commande `hol.Units.Report [N]` (nombre d'unités, et pour les premières : type, propriétaire, position,
  niveau de détail, visibilité, représentation). Les 3 Paysans sont à 11,5 m devant le centre côté milieu de la carte
  (+X pour l'emplacement 0), visibles pour Mass, dessinés en instances.
- Caméra déplacée vers eux puis zoomée : **ils s'affichent, bleus, en arc**. Ils étaient cachés : la caméra ne tourne
  pas (D254) et regarde vers +X, donc pour l'emplacement 0 (milieu de la carte vers le haut de l'écran) ils naissent
  **derrière** le bloc de 4,5 m, et sont petits à la hauteur de jeu. Pas un défaut de rendu ; question posée à
  l'utilisateur sur le côté de naissance (D283 dit « du côté du milieu de la carte »).
- **Tranché par l'utilisateur** : « en bas du forum si pas de rally point » → naissance **en bas du centre vu de la
  caméra** (−X), D283 précisée ; le point de ralliement de J4 prendra le relais.

### 2026-10-14 (suite) — Essai de l'utilisateur : « très bien », mais un plantage corrigé

- Essai de J3 sur le deuxième écran : jugé **très bien**, puis **plantage** dans `UDataTable::FindRowInternal` appelé par
  `UHOLUnitSubsystem::FindTypeRow` depuis le HUD. Cause : `CachedTypeTable` n'était pas un `UPROPERTY`, le ramasse-miettes
  (environ toutes les minutes) libérait la table et le pointeur gardé pointait dans le vide. Les vérifications par script
  duraient moins d'une minute. Très probablement aussi la vraie cause du plantage du serveur dédié vu par la session M2.5
  (attribué alors à une ancienne cuisson).
- Corrigé (`UPROPERTY(Transient)`) ; la partie production de `quick_check` force un passage du ramasse-miettes
  (`obj gc`) avant de produire. `quick_check` complet PASS en 40 s (dont production et la partie réseau à 2 clients de
  la session M2.5).

### 2026-10-14 (fin de séance) — Taille des unités ; reprise

- Essai à 2 clients (mené par la session M2.5) : retard de la barre de ressources (le `PlayerState` part à 1 mise à jour
  par seconde) ; la session M2.5 ajoute `ForceNetUpdate` dans `AHOLPlayerState::SetResources` / `SetPopulation`.
- L'utilisateur trouve les **Paysans minuscules** (comparaison avec une capture d'AoE4) : même zoom, mais bâtiment /
  hauteur d'un villageois à l'écran ~9 dans AoE4 contre ~16 chez nous (unités à l'échelle réelle, plongée de 55° qui
  écrase la hauteur). **Choix de l'utilisateur : unités ×1,5** (colonne `MeshScale` par type, rayon de corps agrandi
  d'autant, marche ralentie du même facteur). Écartés : centre ramené à ~12 m en plus, plongée ~50° (revient sur D254),
  rien. Code écrit, **pas encore compilé** (la session M2.5 occupait les DLL) : à compiler, refaire la table,
  `quick_check`, puis S1 / S2 courts en fenêtre visible.

### 2026-10-14 (suite) — Unités ×1,5 compilées ; S1 / S2 courts

- `MeshScale` compilé (binaire déjà à jour, compilé par la session M2.5), table refaite depuis le CSV (4 lignes),
  `quick_check` **PASS en 42 s** (production et 2 clients compris). Capture `prod_done` : les deux Paysans nés sous le
  centre, bien visibles ; rapport hauteur du centre / hauteur d'un Paysan à l'écran ≈ 9, celui de la capture d'AoE4.
- **S1 et S2 courts** (3 s, vue large, deuxième écran ; deux éditeurs ProjectShiva ouverts, contre un pour la
  référence 5.8.3) : complets, 2 000 unités, registre sans écart, 0 restante.

  | Scénario | Pas (ms) | i/s (1 % bas) | Jeu / rendu / GPU (ms) | Recouvrements | Référence 5.8.3 (vue large) |
  |---|---|---|---|---|---|
  | S1 | 2,69 ✅ | 145 (55) ✅ | 6,25 / 3,29 / 3,60 | 1 399 → 1 399 | 2,47 ms, 179-198 i/s, 649 → 649 |
  | S2 | 2,59 ✅ | 168 (66) ✅ | 5,49 / 2,36 / 3,53 | 882 → 751 | 2,28-2,37 ms, 204 i/s, 337 → 227 |

  Seuils tenus. Le rayon passe de 40 à 60 cm sur la même densité de naissance (foule de 45 m de rayon) : plus de paires
  se recouvrent, et l'évitement à l'arrêt coûte 10,4 ms par passage (1 pas sur 6) contre ~7 ms ; d'où le pas ~10 %
  plus lourd. Le GPU ne bouge pas. Le fil de jeu plus lent (5,5-6,3 contre 4,9-5,2 ms) suit surtout la machine
  chargée ; mesures courtes, pas une nouvelle référence.
- Reste de J3.5 : confirmation par l'utilisateur que le plantage du ramasse-miettes a disparu.

### 2026-10-14 (suite) — J3.5 close : production à 2 clients dans `quick_check`

- L'utilisateur confirme : **plus de plantage** à l'essai (« c'est bon »).
- D284 demandait aussi la production dans la partie à 2 clients, qui ne contrôlait encore que caméras et sélection.
  Ajouts : verbe de séquence **`waitunits <N>`** (unités simulées en solo et au serveur, affichées chez un client) ;
  `hol.Units.Report` chez un client décrit les unités **affichées** par type et propriétaire ; `hol.Prod.Report` ajoute
  l'économie des **autres joueurs** telle que cette machine la voit. Chaque client fait produire un Paysan à son centre
  (`hol.Prod.Queue`, 2 s par `hol.Prod.BuildTime 2` passé au serveur) ; contrôlés : les deux Paysans affichés chez
  chacun, son propre stock débité (F 150, population 1/10), celui de l'autre **jamais reçu** (F 0, population 0/0),
  un Paysan né par emplacement au serveur. `quick_check` **PASS en 43 s**.
- **J3.5 close.** Suite : J3.6 (D280 à D284 au GDD).

### 2026-10-14 (suite) — J3.6 faite : J3 close

- D280 à D284 au § 21 du GDD ; D283 précisée (unités dessinées ×1,5, écartés : centre plus petit, plongée ~50°).
  Paragraphe « Parcours J3 » au § 16.6, après celui de M2.5, avec le plantage du ramasse-miettes et les limites connues
  (couleur des unités = numéro d'emplacement comme index de palette ; pas d'économie pour les emplacements IA).
- **J3 close.** Suite : plan de J4 (ordres de déplacement et chemins, M3), décisions à partir de D285.

### 2026-10-14 (suite) — Plan de J4 ouvert

- Plan de J4 (ordres de déplacement et chemins, M3) ouvert : 5 questions prévues (périmètre, chemins, ordre de groupe,
  sélection des unités, vérification). Q1 (périmètre) posée à l'utilisateur.
- **Q1 tranchée : D285** (option A, déplacer ses Paysans seuls ou à plusieurs). L'utilisateur demande ensuite le
  **plan d'implémentation** complet : Q2 à Q5 proposées d'un bloc (D286 à D289, « proposé » dans le tableau du plan),
  sous-étapes J4.1 à J4.8. Relevé dans le moteur : le plugin `MassNavMeshNavigation` (chemin court, suivi de couloir)
  est toujours dans la liste des processeurs de simulation ; l'errance d'essai de M1 (supprimée en e8fa244) en donne
  la recette (`FindPath` → `FNavCorridor` → `RequestShortPath`). Le navmesh du projet est statique (aucun réglage
  `RuntimeGeneration`). Reprise : validation du plan par l'utilisateur, et son choix du retour visuel (D288).
- **Plan validé** (D286 à D289) ; retour visuel : cercle sous les unités sélectionnées **et carré sous le bâtiment
  sélectionné** (l'utilisateur revient sur le retrait de D259). **J4.1 commence.**

### 2026-10-14 (suite) — J4.1 faite : navmesh à modificateurs, obstacles sur `Plain`

- **Navmesh** (`DefaultEngine.ini`) : `RuntimeGeneration=DynamicModifiersOnly` sur `RecastNavMesh` ; l'agent se déclare
  dans `NavigationSystemV1` (`+SupportedAgents`, **rayon 60 cm**, hauteur 270 cm : le Paysan ×1,5), car le rayon du
  `RecastNavMesh` seul est écrasé par celui de l'agent (35 cm par défaut, vu au journal de construction). Les
  modificateurs ne s'appliquent que si le navmesh a été construit **avec** ce mode (il garde alors ses couches
  compressées) : les deux cartes ont été refaites par leurs scripts.
- **Constat** : l'ancienne `Plain` n'avait **aucun navmesh sauvegardé** (seulement le volume). `create_match_map.py`
  construit maintenant le navmesh comme `create_stress_test_map.py` : attente de ~5 s (l'éditeur verrouille la
  construction après le chargement), mise à jour des limites (`on_navigation_bounds_updated` : l'échelle posée depuis
  Python ne prévient pas la navigation, d'où un navmesh taillé pour le cube de 2 m, « 3 tile limit »), suppression du
  navmesh mal taillé, `BUILDPATHS` synchrone, sauvegarde. Le rappel de Slate pouvait passer deux fois (une boîte de
  dialogue modale fait retourner Slate) : drapeau `done`. Une boîte « Unable to Check Out From Revision Control » s'ouvre
  à la sauvegarde d'une carte supprimée puis recréée ; sans effet (contrôle de version coupé), la carte est écrite.
- **Bâtiments** : `AHOLBuilding` porte une boîte `NavObstacle` sans collision, `bDynamicObstacle`, zone
  `NavArea_Null`, taillée sur l'emprise et la hauteur de la table puis `FNavigationSystem::UpdateComponentData` ; les
  pièces ne touchent plus la navigation. Le navmesh élargit la zone du rayon de l'agent.
- **Obstacles de `Plain`** (symétriques) : devant chaque emplacement, à 120 m du milieu (30 m devant le centre), une
  arête de 3 rochers (sphères aplaties, ~20 m de long) ; au milieu, un bosquet de 12 arbres (troncs de 5 m, couronnes à
  partir de ~4,75 m : on passe dessous). Matériaux `MI_Rock`, `MI_Trunk`, `MI_Crown` (instances de `M_MatteColor`,
  faites par le script s'ils manquent).
- **`hol.Nav.Path X1 Y1 X2 Y2`** (`Map/HOLNavReport.cpp`) : chemin entre deux points du sol, sur les machines qui ont
  un navmesh (serveur, solo). Partie **« move »** de `quick_check.ps1` (dans le cas 2 joueurs) : à travers le centre de
  l'emplacement 0 (né en cours de partie) **37,6 m pour 30 m (×1,25)**, à côté **×1,00**, à travers l'arête **×1,49**,
  à travers le bosquet **×1,01** (entre les troncs). `quick_check` **PASS en 40 s** (104 s au premier passage :
  matériaux neufs à compiler).
- **S1 / S2 courts** hors écran (`-RenderOffScreen`, fenêtre de 888 × 500 : images par seconde non comparables à la
  référence) : S1 **2,16 ms**, 216 i/s, 1 399 → 1 399 recouvrements ; S2 **2,31 ms**, 214 i/s, 882 → 751 ; seuils
  tenus, recouvrements identiques à J3.5 : le nouveau navmesh ne change rien au stress test.
- Le serveur cuit de `net_match.ps1` n'a pas été recuit : `-Stage` à passer avant le prochain essai à 2 clients.
- Suite : **J4.2** (règles pures `Simulation/HOLGroupMoveRules` et tests).

### 2026-10-14 (suite) — J4.2 faite : règles pures de l'ordre de groupe

- `Simulation/HOLGroupMoveRules` (espace `HOLGroupMove`) : **paquets** (`MakeClusters` : union des voisins à moins de
  15 m, une chaîne d'unités fait un seul paquet ; numérotés dans l'ordre de leur première unité) ; **places**
  (`MakeSlots` : nid d'abeille d'écart 1,2 diamètre, les plus proches du but d'abord, la première étant le but ;
  demander plus de places que d'unités laisse sauter celles qui tombent dans un obstacle) ; repère de la marche
  (`ToWorld`) ; **attribution** (`AssignSlots` : unités et places ramenées au même centre et à la même dispersion,
  paires les plus proches d'abord : un groupe déjà en forme la garde, l'unité de gauche arrive à gauche).
- Tests `HeroesOfLiberty.Movement.Clusters`, `.Slots` (7, 30, 175 places : aucun recouvrement, disque serré, même
  résultat à chaque fois), `.Assign` ; `quick_check -Parts tests` : **31/31**.
- Suite : **J4.3** (chemins dans la simulation).

### 2026-10-14 (suite) — J4.3 faite : chemins dans la simulation

- **Ordre de groupe** : `FHOLMoveOrder` (identifiants + but) remplace l'ordre par unité ; `IssueMoveOrder` garde sa
  signature (le stress test n'a pas changé). Fragment `FHOLMoveOrderFragment` par unité (place, phase, place de côté,
  numéro d'ordre, pas sans progrès) ; phases `WaitingPath` → `Path` → `FinalLeg` → `None`.
- `UHOLUnitOrderProcessor` réécrit : (1) **ordres** : places autour du but (une unité seule : le but ; un groupe : nid
  d'abeille orienté selon la marche, chaque place gardée si le navmesh l'a là où elle est **et** l'atteint en ligne
  droite depuis le but, `Raycast`), attribution (`AssignSlots`), paquets (`MakeClusters`), une requête de chemin par
  paquet ; l'activité passe à `Moving` tout de suite (S2 n'ordonne que les unités au repos) ; (2) **requêtes** au
  budget par pas (`PathRequestsPerStep` = 64, console `hol.Units.PathBudget`), `FindPathSync` du centre du paquet
  ramené au navmesh ; un ordre plus récent remplace un chemin pas encore calculé (numéro d'ordre) ; (3) **suivi** : un
  chemin de **plus de 2 points** (un détour) devient un couloir de 6 m partagé ; chaque unité y suit son chemin court
  (Mass) **décalé de sa place de côté** dans le paquet (mesurée sur la droite propre du chemin, bornée par le portail),
  et repart de sa propre avancée (une unité devant le centre du paquet ne revient pas en arrière) ; un chemin
  **droit** (2 points, le cas en terrain dégagé) ou absent : **droit à la place**, sans couloir ; (4) **dernier
  tronçon** droit vers la place, debout à 50 cm, ou après 40 pas sans se rapprocher (place prise, poussé).
- Commandes : `hol.Units.Move X Y [Slot]`, `hol.Units.Spawn Count X Y [Slot] [Type] [Radius]` (essai, hors règles),
  `hol.Units.DirectPaths 1` (aucun chemin, pour mesurer leur coût) ; `hol.Units.Report` dit `idle` / `moving`.
  Dépendance `NavCorridor` remise dans `Build.cs`.
- **Essais sur `Plain`** (sans fenêtre) : 4 Paysans produits, ordre derrière l'arête : contournent le centre puis
  l'arête, arrivés écartés de 1,35 à 1,9 m ; retour à l'emplacement : pareil. **40 Paysans** nés à 30 m derrière le
  centre, envoyés derrière l'arête (~75 m de chemin) : tous arrivés en ~40 s, disque de 5,1-5,4 m (attendu ~4,8 m),
  paire la plus proche 1,10-1,15 m pour des corps de 1,20 m (3 paires).
- **S2 et le coût des chemins** (`-RenderOffScreen`, machine chargée : éditeur ProjectShiva ~11 % du processeur,
  git-lfs, lanceur Epic) : le même S2 varie de 3,0 à 4,3 ms d'un passage à l'autre (évitement en marche de 10 à 14 ms
  par passage) ; S2 valait 2,31 ms à J4.1, une heure plus tôt. A / B alternés sur le même binaire : couloir suivi
  pour tous les chemins → +0,6 ms (suivi +0,3, évitement +0,2) ; d'où le **chemin droit sans couloir** : reste
  **+0,1 ms** (les requêtes de chemin, ~8 par pas) et un évitement dans le bruit (passages : chemins 3,83 / 3,34 ms,
  direct 3,31 / 2,97 ms). Les arêtes du couloir données à l'évitement ne coûtent rien (A / B sans le processeur de
  bords : identique). **À remesurer machine au repos avant la clôture (J4.7)** : le seuil de 3 ms bloque l'étape.
- `quick_check` **PASS en 48 s**.
- Suite : **J4.4** (ordre réseau et point de ralliement).

### 2026-10-14 (suite) — J4.4 faite : ordre par le serveur, point de ralliement

- **Ordre du joueur** : `AHOLPlayerController::RequestMoveUnits` → appel serveur `ServerMoveUnits` (identifiants +
  point quantifié, 200 unités au plus) → `IssueMoveOrder(..., PlayerIndex)`. `FHOLMoveOrder` porte le joueur ; le
  **contrôle du propriétaire** se fait dans la simulation, au pas où l'ordre s'applique (comme la production, D282) :
  les unités d'un autre sont ignorées (« Move order of slot N: K units ignored, not its own »). Les ordres du serveur
  lui-même (ralliement, essais, stress test) n'ont pas de joueur.
- **Point de ralliement** : `FHOLProductionCommand::EKind::Rally` (file de commandes, propriétaire vérifié par la
  production) ; `AHOLBuilding` : `RallyPoint` et `bHasRallyPoint` répliqués au seul propriétaire, `OnRallyChanged`
  pour le fanion de J4.6 ; un ordre sur l'emprise du bâtiment **efface** le ralliement. Le Paysan naît toujours au pied
  du centre (D283), puis `FHOLUnitSpawnRequest::bHasMoveTarget` lui donne un ordre vers le ralliement au pas de sa
  naissance.
- **Places déjà prises** : trois Paysans envoyés un par un au même ralliement s'y empilaient (35 cm d'écart : un ordre
  d'une seule unité allait au point exact). Désormais toute place passe par le nid d'abeille, et une place est
  sautée si une unité **au repos hors de l'ordre** s'y trouve (grille d'obstacles de Mass) ou si elle est **déjà
  donnée** à une unité en route (registre des places réservées par cases de 5 m, vérifié à la lecture, balayé toutes
  les ~10 s). Essai : 4 Paysans au ralliement, à 1,33 m au plus près, dans 1,6 m.
- **Vue des unités** (`Simulation/HOLUnitView`) : les unités connues de cette machine (simulées en solo et au serveur,
  affichées chez un client), avec position et rayon ; servira à la sélection (J4.5).
- Commandes : `hol.Units.Order X Y [all]` et `hol.Units.OrderNear Dx Dy [all]` (par le contrôleur, donc le serveur ;
  `all` envoie aussi les unités des autres), `hol.Prod.Rally X Y`, `hol.Units.Where [Slot]`.
- **`quick_check`, partie à 2 clients** (si `move`) : après la production, chaque client envoie **toutes** les unités
  qu'il connaît à 25 m de son départ ; contrôles : chaque client voit les deux Paysans à moins de 1,5 m de là où leurs
  propriétaires les ont envoyés (mesuré : 11-13 cm), le serveur a ignoré une unité pour chacun. **PASS en 46 s.**
- Suite : **J4.5** (sélection des unités : projection, clic, cadre, Maj, clic droit).

### 2026-10-14 (suite) — J4.5 commencée, séance arrêtée par l'utilisateur

- Écrits, **non compilés, non commités** : `Player/HOLUnitPicking.h/.cpp` (`PickAt` : corps dessiné sous le curseur, le
  plus proche de la caméra ; `PickInBox` : milieu du corps dans le cadre ; `IsBox` : au-delà de 6 pixels) et le test
  `HeroesOfLiberty.Movement.Picking` dans `Tests/HOLMovementTests.cpp`.
- Reste de J4.5 (prévu, rien d'écrit) : dans `AHOLPlayerController`, sélection d'unités (`TArray<uint32>`) exclusive
  du bâtiment ; clic résolu au relâchement (`IA_Select` Started / Completed : clic ou cadre), unité avant bâtiment,
  Maj (`IA_SelectAdd`, LeftShift / RightShift) ajoute ou retire ses unités, une unité adverse seule ; clic droit
  (`IA_Order`, RightMouseButton) : ses unités → `RequestMoveUnits`, son bâtiment → `RequestRallyPoint` (sur le
  bâtiment : efface) ; candidats projetés depuis `HOLUnitView::Gather` (pieds, tête à +270 cm, demi-largeur du
  rayon × 1,3) ; unités disparues retirées toutes les ~0,25 s ; `DescribeSelection` (garder « MainCentre of slot N
  (own) » et « none » pour `quick_check`) ; `GetSelectionBox` pour le rectangle de J4.6 ; délégué d'ordre donné pour
  le repère de J4.6 ; commandes `hol.Select.Click X Y [add]`, `hol.Select.Box X1 Y1 X2 Y2 [add]`,
  `hol.Select.Order X Y` ; actions ajoutées à `make_game_input.py` puis le relancer (commandlet).
- **Reprise** : compiler, `quick_check -Parts tests` (32 tests attendus), puis le contrôleur. S2 toujours à
  remesurer machine au repos avant J4.7.

### 2026-10-14 (suite) — J4.5 faite : sélection des unités et clic droit

- Contrôleur fait comme prévu ci-dessus. Précisions : le corps visé va des pieds à 180 cm × l'échelle du maillage
  (`FHOLUnitView::MeshScale`, ajouté, lu de la table), demi-largeur = rayon × 1,3 projeté ; **Maj sur le vide garde**
  la sélection d'unités ; **Maj + cadre** ajoute les unités prises, ou les **retire toutes** si elles l'étaient déjà ;
  Maj sur une unité adverse ou depuis un bâtiment sélectionné repart de zéro ; sélection bornée à 200 unités (comme
  l'ordre). Clic droit sur son centre sélectionné : la position du centre est envoyée (le sol vu derrière le toit
  serait hors de l'emprise), donc le ralliement s'efface. `DescribeSelection` : « Peasant #12 of slot 0 (own) »,
  « 3 units (own): Peasant x3 », bâtiment et « none » inchangés. Le HUD ne montre encore rien pour des unités (J4.6).
- Commandes : `hol.Select.Click X Y [add]`, `hol.Select.ClickUnit N [add]` (clic au milieu du corps dessiné de la
  N-ième unité du joueur : un script ne sait pas où elles sont à l'écran), `hol.Select.Box X1 Y1 X2 Y2 [add]`,
  `hol.Select.Order X Y` ; `hol.Select.Report` décrit aussi les unités.
- `make_game_input.py` relancé : `IA_SelectAdd` (Maj gauche / droite), `IA_Order` (clic droit).
- Essai scripté solo (`-RenderOffScreen`, hors `quick_check`) : ralliement, 3 Paysans ; cadre plein écran → 3 ;
  clic droit → « Order: 3 units », arrivés autour du point ; clic sur #1 → #1 ; Maj + #2 → 2 ; Maj + #1 → #2 seul ;
  Maj sur le vide → inchangé ; Maj + cadre → 3, de nouveau → aucune ; clic droit sur le centre sélectionné → ralliement
  effacé (serveur), puis posé ailleurs. Captures `selt_*`. À intégrer à `quick_check` en J4.7.
- Tests 32/32, `quick_check` **PASS en 39 s**.
- Suite : **J4.6** (cercles sous les unités sélectionnées, carré sous le bâtiment, rectangle du cadre via
  `GetSelectionBox`, repère d'ordre via `OnOrderGiven`, fanion de ralliement, bloc de sélection d'un groupe).

### 2026-10-14 (suite) — J4.6 faite : retour visuel de la sélection

- L'utilisateur préfère essayer J4.5 et J4.6 ensemble : J4.6 faite d'un trait, l'essai de l'utilisateur suit.
- **`Player/AHOLSelectionMarkers`**, acteur local fait par le contrôleur (comme la caméra, jamais répliqué) : deux
  maillages instanciés sur le plan de 1 m du moteur, sans collision ni ombre, données par instance 0-2 couleur,
  3 trait en part du demi-côté. **Cercles** sous les unités sélectionnées (rayon × 1,4, trait 12 cm), à la couleur
  choisie par leur propriétaire (`AHOLPlayerState::FindColorIndex`, ajouté) ; **repère d'ordre** : cercle blanc qui
  passe de 1,8 m à 0,5 m de rayon en 0,5 s (`OnOrderGiven`, ordre d'unités ou ralliement) ; **carré** autour de
  l'emprise du bâtiment sélectionné (marge 1 m, trait 30 cm), à la couleur de son propriétaire ; **fanion** (mât de
  5 m, toile à la couleur du joueur) au ralliement de son bâtiment sélectionné. `hol.Select.Report` ajoute « Selection
  markers: rings N, frame N, flag N ».
- Matériaux `M_GroundRing` et `M_GroundFrame` (`Tools/Editor/make_marker_materials.py`, commandlet) : sans éclairage,
  masqués, forme tirée des UV, faits seulement s'ils manquent.
- Les cercles suivent les unités dessinées : `FHOLUnitView::Location` est désormais **la position dessinée** (en solo,
  interpolée entre les deux derniers pas comme le dessin ; chez un client, inchangée) ; le clic vise donc aussi ce que
  le joueur voit.
- **Rectangle du cadre** : `AHOLGameHUD::DrawHUD` (fond blanc à 8 %, contour clair), d'après `GetSelectionBox`.
- **Bloc de sélection** : une unité → nom, propriétaire, points de vie (comme un bâtiment) ; plusieurs → « Paysan × 3 »
  par type, propriétaire, sans points de vie. Texte `UnitCount` « {0} × {1} » traduit (`fill_po.py`, 104 textes).
  `hol.HUD.Report` : « selection Peasant x3 (own) ».
- Essai scripté (`-RenderOffScreen`) : centre sélectionné → carré et fanion ; cadre → 3 cercles et « Paysan × 3 » ;
  clic droit → cercle blanc au point ; clic sur un Paysan → 1 cercle et « Paysan, Vous, 50 / 50 » ; centre adverse →
  carré rouge, pas de fanion. Au premier affichage, « Shaders en préparation » : carrés gris le temps de compiler.
  Le rectangle du cadre demande un vrai glisser (non scripté) : à voir à l'essai.
- Remarque hors étape : les unités sont dessinées avec la couleur d'index = leur emplacement (`GetPlayerColor
  (OwnerPlayer)`), les bâtiments et les cercles avec la couleur choisie ; identiques tant que chaque joueur garde la
  couleur de son emplacement (cas des essais), différentes sinon.
- Tests 32/32, `quick_check` **PASS en 43 s**.
- Suite : **essai de l'utilisateur** (J4.5 et J4.6), puis **J4.7** (vérification D289 dans `quick_check`, S1 / S2 /
  S5 courts ; S2 à remesurer machine au repos).

### 2026-10-14 (suite) — 1er essai de J4.5 / J4.6 : retouches (D290) et triches (D291)

- Retours de l'utilisateur : unités « ultra lentes », contour de sélection à mettre en blanc neutre, repère du clic
  droit 2× trop grand et 2× trop long, besoin de triches pour les essais.
- **Vitesse** mesurée (`hol.Units.Where` chaque seconde) : 3 m/s en régime, c'était la valeur de la table. Paysan passé
  à 6 m/s (`UnitTypes.csv`, `make_unit_types.py` relancé), `WalkPlayRate` 2,44. Il mettait alors 2,4 s à atteindre sa
  vitesse (accélération de Mass fixe à 250 cm/s²) : `MaxAcceleration = vitesse / 1,2 s` ; mesuré : 2,9 / 5,8 / 6,0 /
  6,0 m/s les quatre premières secondes. Les types du stress test (300 cm/s) gardent 250 cm/s² : S1 / S2 inchangés en
  principe, mesurés en J4.7.
- **Marqueurs** : cercles et carré blancs (0,85), repère d'ordre de 0,9 à 0,25 m en 0,25 s (trait 8 cm).
- **Triches** (D291) : `AHOLPlayerController::RequestCheat*` → `ServerCheat*` (corps vide en Shipping), commandes
  `hol.Cheat.*` sous `#if !UE_BUILD_SHIPPING` ; population par `UHOLProductionSubsystem::AddCheatPopulation`. Essai :
  +500 de tout, +100 bois, −2000 nourriture (→ 0), +1000 nourriture, +5 places (plafond 15), production en un pas
  (3 Paysans en 3 pas) ; captures `selw_*` (cercles blancs, repère réduit, carré blanc).
- Tests 32/32, `quick_check` **PASS en 42 s**.
- Suite : **2e essai de l'utilisateur**, puis J4.7.

### 2026-10-14 (suite) — 2e essai : arrêtée ou pleine vitesse (D290 complétée)

- Retours : « pas de courbe de mouvement, c'est idle ou max speed » ; l'unité dépasse souvent le point visé puis s'y
  remet petit à petit.
- Causes (moteur, `MassSteeringProcessors.cpp`) : le pilotage de Mass est une force (temps de réaction 0,3 s) et
  ralentit sur la dernière seconde du trajet (`ArrivalSpeedEnvelope`, 6 m à 6 m/s) ; à l'arrêt (`Stand`), il remet le
  point de stationnement sur le centre de l'ordre et y ramène l'unité lentement ; l'évitement à l'arrêt écarte les
  voisins de quelques cm/s pendant des secondes ; la tolérance d'arrivée (50 cm) arrêtait l'unité avant sa place.
- **`UHOLFullSpeedProcessor`** (entre `MassApplyForceProcessor` et `MassApplyMovementProcessor`) : en marche, vitesse
  toujours pleine dans la direction donnée par le pilotage, l'évitement et le bord du navmesh ; à moins de 3 m de sa
  place (dernier tronçon), droit dessus (sinon, à pleine vitesse, le virage de 0,3 s la fait tourner autour : vu avec
  8 Paysans, qui ne se posaient jamais) ; dernier pas posé pile sur la place ; à l'arrêt, vitesse nulle. Arrivée
  (`UHOLUnitOrderProcessor`) : vitesse annulée, `MoveTarget.Center` = position d'arrêt ; `OrderArrivalTolerance` 5 cm.
- Mesures : un Paysan seul à ~5,8 m/s dès la première demi-seconde, posé exactement sur le point visé puis immobile ;
  8 Paysans posés en ~6,5 s, plus aucun mouvement ensuite (contre une dérive de 2 à 20 cm/s avant).
- Risque : les unités à l'arrêt ne s'écartent plus (chevauchements de S2 ?), à mesurer en J4.7 avec S1 / S2.
- Tests 32/32, `quick_check` **PASS en 43 s**.
- Suite : **3e essai de l'utilisateur**, puis J4.7.

### 2026-10-14 (suite) — 3e essai : un Paysan en traverse un autre (D292)

- Retour : « très bien niveau vitesse, arrêt » ; reste : un Paysan traverse un Paysan.
- **`UHOLUnitSeparationProcessor`** (après `MassApplyMovementProcessor`) : voir D292. Essai de croisement (scratchpad,
  `hol.Units.Spawn` de 10 Paysans par emplacement à ±30 m sur x = −80 m, terrain dégagé ; distances mesurées sur les
  positions affichées toutes les 0,25 s) :
  - sans séparation : traversées (D230) ;
  - séparation seule : **un groupe à l'arrêt devient un mur**, le groupe envoyé au travers reste derrière → poussée
    des unités à l'arrêt ;
  - diagnostic par **`hol.Units.Orders [Slot]`** (nouvelle commande : phase, place, distance, blocage) : 4 Paysans
    avaient **la même place** (unités sans place → le point visé ; les places proches étaient prises par l'autre
    groupe) → `MakeOrderSlots` essaie 3 × N + 20 places ; 3 Paysans **fuyaient à reculons** à pleine vitesse sous la
    poussée de l'évitement puis étaient arrêtés à 110 m par le compteur de blocage → jamais à reculons (de côté), blocage
    compté à moins de 5 m seulement ;
  - une passe : traversée d'un groupe à l'arrêt OK mais deux Paysans arrêtés poussés l'un sur l'autre (8 cm) → une
    unité poussée regarde ses voisines ; croisement de deux groupes en marche 36 cm de recouvrement → deux tours.
  - Résultat : croisement 107 cm au plus près (contact à 120 cm), traversée d'un groupe à l'arrêt 118 cm, tous posés
    sur leurs places ensuite.
- Coût (S2, 2 000 unités, machine chargée par un éditeur ProjectShiva) : 0,43 → 0,88 ms selon les essais de grille,
  **0,8 ms** pour la version retenue ; A / B du pas complet trop bruités (3,0 à 4,6 ms dans les deux cas). Grille plate
  écartée (cases trop grandes sur la carte du stress test), table hachée de 8 192 cases retenue. **À remesurer au repos
  en J4.7** ; pistes si trop cher : un tour au-delà de N marcheurs, calcul en float, découpage parallèle.
- Tests 32/32, `quick_check` **PASS en 41 s**.
- Suite : **4e essai de l'utilisateur**, puis J4.7.
- 4e essai (fin de séance) : l'utilisateur signale **encore un problème**, qu'il décrira dans la séance suivante.
  **Reprise** : lui demander ce problème, le corriger (tout est commité, b5f168b), puis J4.7.

### 2026-10-14 (suite) — 4e essai : un Paysan en bouscule un autre au lieu de le contourner (D293)

- Retour : un Paysan ne traverse plus un Paysan mais le **bouscule** ; il devrait le **contourner**, et ne le bousculer
  que s'il ne peut pas faire autrement (détour trop grand).
- Cause : l'évitement de Mass ne tourne qu'**un pas sur 6** (D220, ~3 fois par seconde) et ne regarde qu'à 2,5 m ; à
  6 m/s le Paysan fait 1,8 m entre deux passages : il ne dévie presque jamais à temps, la séparation (D292) le fait
  alors pousser.
- **Regard en avant à chaque pas** dans `UHOLFullSpeedProcessor` : voir D293. Grille hachée des positions et vitesses
  avant le mouvement (0,08 ms à 2 000 unités), puis, par marcheur en parallèle, temps de contact avec les voisins devant
  lui ; virage le plus petit qui dégage les 0,5 s à venir, jusqu'à 60°, sinon tout droit (la séparation pousse).
- Essai scripté (scratchpad, `Plain`, terrain dégagé autour de x = −95 à −115 m, `hol.Units.Where` toutes les 0,05 s ;
  `hol.Units.Avoid 0` → `1`) : Paysan seul à l'arrêt poussé 275 → 0 cm (passé à 130 cm, écart d'1,6 m, trajectoire
  lisse puis retour sur sa place) ; groupe de 7 à l'arrêt poussé 141 → 0 cm ; deux marcheurs de face 117 → 164 cm au
  plus près ; mur serré de 5 (6,5 m) traversé par 6 marcheurs : poussé 484 → 137 cm. Variantes 0,8 s / 75° : même
  résultat à peu de chose près, réglage 0,5 s / 60° gardé.
- Coût (S2, A / B, machine chargée à 40-50 %) : pas complet 3,96 ms sans, 4,18 ms avec (seuil de 3 ms dépassé dans les
  deux cas, MassMovingAvoidance à 7-8 ms par passage) ; `HOLFullSpeedProcessor` 0,08 → ~0,57 ms. Dans la foule de S2,
  ~40 % des marcheurs ont un corps devant et ~20 % essaient les 12 virages avant de pousser. Pistes si trop cher au
  repos : regard un pas sur deux avec virage gardé, moins de virages essayés dans une foule.
- Tests 32/32, `quick_check` **PASS en 45 s**.
- 5e essai de l'utilisateur : **validé** (« super »).
- Demande reportée aux **formations** (étape « armée », M4, D287) : les unités bousculées reprennent leur position
  (aujourd'hui elles restent là où on les a poussées). Noté dans D293.
- Suite : **J4.7** (vérification D289 : `quick_check` solo et 2 clients, S1 / S2 / S5 courts, S2 machine au repos).

### 2026-10-14 (suite) — J4.7 Vérification (D289) : partie solo des ordres, S1 / S2 / S5 (D294)

- **`quick_check.ps1`, partie « move » solo** (après la production, besoin de `camera` et `production`) : ralliement
  posé entre le centre et la crête (−135 m, 0), 3 Paysans produits (`hol.Cheat.Food`, 2 s chacun) qui s'y rendent ;
  caméra au-dessus de la crête ; **cadre** autour d'eux → « 3 unités » et 3 cercles ; **Maj + clic** sur l'un →
  2, puis 3 ; **clic droit derrière la crête** (−105 m, 0) ; positions toutes les 0,25 s pendant 11 s ; **Échap**.
  Contrôles : 3 Paysans à ≤ 3 m du ralliement (0, 144, 144 cm), arrivés à ≤ 3 m du point (159, 159, 17 cm), trajet
  1,33 à 1,57 × la ligne droite (contournement), 144 cm au plus près entre eux (corps 120), **aucun hors du navmesh**
  (`hol.Nav.Units`), sélection vide après Échap, captures `move_box` / `move_arrived`. Nouvelles commandes de test :
  `hol.Camera.Focus X Y`, `hol.Select.BoxGround`, `hol.Select.OrderGround` (projection du sol à l'écran puis le même
  chemin qu'un vrai cadre / clic droit), `hol.Select.ClickUnit -1` (depuis la dernière), `hol.Nav.Units`.
  `quick_check` **PASS en 65-75 s** (machine chargée ; la partie solo ajoute ~20 s).
- **Défauts trouvés et corrigés (D294)** : le 3e Paysan, dont la place est collée aux deux premiers, en faisait le tour
  sans y entrer puis s'arrêtait à 3-5 m (compteur de blocage) → dernier tronçon droit sur la place ; chemin quitté
  pour la place près de sa fin ; marge de 10 % du regard en avant. Essai de contournement de D293 refait : Paysan seul
  136 cm, groupe de 7 non poussé, de face 141 cm, mur poussé 123 cm (meilleur qu'avant).
- **S1 court PASS** (2,78 ms par pas, 165 fps). **S2 court FAIL sous charge** (4,97 ms par pas contre 3 ;
  ProjectShiva ouvert et antivirus, 45-79 % de charge ; MassMovingAvoidance 8,8 ms par passage, `HOLFullSpeed`
  0,74 ms, séparation 0,81 ms, ordres 0,69 ms) : **à remesurer au repos**.
- **S5 court FAIL** (`net_match.ps1 -Stage -Units 2000 -Seconds 40`) : 45,6 Ko/s par client, 2,02 envois par unité et
  par seconde, écart affiché 27,8 cm (M2.5 : 30,3 Ko/s, 1,30, 19,7 cm). Mesures A / B (nouvelle option
  `-ServerSettings` de `net_match.ps1`) : sans regard en avant 2,11 (pas lui) ; **sans séparation 1,41, 32,3 Ko/s,
  17,2 cm** (c'est elle : elle déplace les unités sans changer leur vitesse, les clients prolongent la marche seule) ;
  tolérance 10 / 20 cm : 1,88 / 1,77 ; **vitesse du pas envoyée : 1,72, 39,4 Ko/s, 23,5 cm** (gardée) ; avec en plus
  20 / 30 cm de tolérance : 1,68 / 1,66 (~38 Ko/s). Envois par motif (5 s) : erreur ~10-14 000, activité ~2 500,
  rappel ~2 500. **Reste au-dessus de 32 Ko/s : décision de l'utilisateur.**
- **D300** : l'utilisateur choisit de chasser d'abord les ~8 Ko/s d'en-têtes non expliqués (D274).
- Suite : en-têtes d'Iris (D300), S5 de nouveau, S2 au repos, puis J4.8 (clôture).
- **Arrêt demandé par l'utilisateur** (reprise dans une autre séance). Tout est commité. Piste chiffrée, à vérifier :
  ~32 morceaux × 10 mises à jour/s × ~30 octets ≈ 9,6 Ko/s, soit les ~8 Ko/s inexpliqués : l'en-tête de chaque
  écriture d'un morceau (`AHOLUnitChunk`). Une lecture du code d'Iris par un agent (disposition des bits d'une
  écriture d'acteur et d'un `FIrisFastArraySerializer`, propriétés d'`AActor` répliquées par défaut, options pour
  réduire) a été **interrompue sans résultat** : à relancer. Pistes : un seul sérialiseur maison par morceau (masque
  des emplacements changés + états) au lieu du tableau rapide ; couper les propriétés d'acteur inutiles du morceau ;
  mesure exacte par `-NetTrace` et Unreal Insights. Mesurer S5 avec `net_match.ps1 -Stage -Units 2000 -Seconds 40`
  (`-ServerSettings "réglage valeur"` pour un A / B, un réglage par chaîne).

### 2026-10-14 (suite) — J4.7, D300 : les en-têtes d'Iris trouvés et retirés (39,7 → 35,6 Ko/s)

- **Sonde chez le client** (`FHOLUnitStateBitProbe`, ligne « Net (bits) » du rapport toutes les 5 s) : position de
  chaque état dans le flux reçu (`GetAbsolutePosBits` au début et à la fin de `Deserialize`) ; l'écart entre deux
  états est exactement ce qu'Iris a écrit entre eux. Le serveur n'a pas changé : mesure sans recuire.
- **Résultat** : **aucun état collé au précédent** ; **10 bits exactement** avant chaque état d'une même écriture de
  morceau, ~130-150 bits entre deux morceaux, premier état d'un paquet à ~190 bits. Cause (lecture d'Iris,
  `ReplicationStateDescriptorBuilder.cpp`) : *« For FastArraySerializerItems we currently include the ReplicationId even
  though it is not replicated »* : chaque élément d'un tableau rapide part avec son `ReplicationID` (entier compacté,
  10 bits pour nos identifiants < 256), soit ~4,2 Ko/s à ~3 400 états/s. C'était l'essentiel des « ~30 octets par
  écriture de morceau » (~9 états × 10 bits) que la lecture d'Iris de M2.5.5 n'expliquait pas.
- **Correctif** : les morceaux portent des **tableaux simples** (`TArray<FHOLNetUnitIdentity>`, `TArray<FHOLNetUnitState>`)
  au lieu de tableaux rapides. Iris donne aussi 63 bits de changement par élément à un tableau simple
  (`net.Iris.UseChangeMaskForTArray`, actif par défaut) et trouve lui-même les éléments changés en les comparant à
  chaque mise à jour (`IsEqual` de notre sérialiseur). Le client lisait déjà tous les emplacements sans rappel du
  tableau rapide : rien d'autre à changer. Après : 2 874 états sur 3 102 collés au précédent, ~185 écarts par seconde
  de ~128 bits (en-tête de lot, masque de changement, nombre d'éléments).
- **S5 court** (`net_match.ps1 -Stage -Units 2000 -Seconds 40`) : **35,6 Ko/s par client** (39,7 avant, mêmes
  ~3 350 états lus par seconde), écart affiché 22,3 cm, 1,74 envoi par unité et par seconde, aucune erreur. Reste
  chez le client : états 29,7 Ko/s (83 %), en-têtes de morceaux ~4,0 Ko/s, paquets (UDP/IP 28 octets × 46/s) et le
  reste ~2 Ko/s. Les en-têtes ne peuvent plus donner que 1-2 Ko/s (moins de morceaux) : il faut **~3,6 Ko/s dans les
  états** (bits par état ou nombre d'envois) : décision de l'utilisateur.
- `quick_check` **PASS en 65 s** (tests 32/32, partie à 2 clients comprise).
- Suite : décision de l'utilisateur sur les ~3,6 Ko/s restants, S5 de nouveau, S2 au repos, puis J4.8.

### 2026-10-14 (suite) — J4.7, D301 : la hauteur n'est plus envoyée, S5 PASS à 29,97 Ko/s

- Question à l'utilisateur (A hauteur au sol / B seuil plus large dans les foules / C A puis moins d'en-têtes) :
  **A**, la recommandation (**D301**). Le GDD prévoit des tireurs sur les remparts (D23) : il faudra alors un bit
  « hors sol » et leur hauteur ; noté dans D301 et dans le commentaire de `FHOLNetUnitState`.
- **État de 59 bits** : `Z` retiré de `FHOLNetUnitState` et de son sérialiseur ; `GetPosition` et `PredictPosition`
  horizontales ; règle d'envoi en `DistSquared2D` (une hauteur différente n'envoie plus rien). Au serveur, la hauteur
  d'une unité ne change de toute façon pas après sa naissance (projection sur le navmesh, marche horizontale).
- **Client** : `UHOLUnitReceptionSubsystem::GetGroundHeight` (tir vertical multiple par type d'objet `WorldStatic`
  entre ±200 m ; une requête par type d'objet rend toutes les surfaces, vérifié dans `CollisionQueryFilterCallback.cpp` ;
  la plus basse est le sol, l'unité passe sous la couronne des arbres du bosquet), à l'arrivée et à chaque état reçu
  (~3 350 par seconde à 2 000 unités) ; `FHOLNetUnitDisplayFragment::GroundZ` ajouté à la position affichée, la
  correction lissée lisse aussi un saut de hauteur. Captures `qcnet_*_moved` : Paysans debout sur le sol (penchés vers
  l'extérieur comme les arêtes verticales du centre : la perspective).
- Tests : `Net.Reduction` (59 bits, hauteur nulle), `Net.ElementFormat` (59 bits sur le fil), `Net.SendRule` (une
  hauteur changée n'envoie rien), `Net.DisplayedPosition` (sol trouvé ajouté) ; `s5_report.py` : `STATE_BITS = 59`.
- **S5 court PASS** (`net_match.ps1 -Stage -Units 2000 -Seconds 40`) : **29 970 o/s par client**, 1,73 envoi par
  unité et par seconde, écart affiché 23,6 cm (médiane 21, 95 % 52), 41 paquets/s, réplication 0,15 ms par pas.
  Client : états 24,7 Ko/s, le reste 5,8 Ko/s.
- `quick_check` **PASS en 63 s** (tests 32/32).
- **S2 court** (hors écran, 888 × 500, 10 s ; machine à ~24 % de charge : éditeur ProjectShiva, GitKraken, lanceur
  Epic…) : **simulation 4,41 ms par pas, FAIL** (seuil 3 ms ; J4.1 : 2,31 ms dans des conditions voisines), rendu
  212 i/s PASS. Détail : `MassMovingAvoidance` 7,6 ms un pas sur 6 (~1,3 ms en moyenne), `HOLUnitSeparation` 0,80,
  `HOLFullSpeed` (regard en avant) 0,70, `HOLUnitOrder` 0,58 (1 730 ordres en 10 s), `MassSmoothOrientation` 0,26,
  `MassSteerToMoveTarget` 0,20. Les ~2 ms ajoutées par J4 sont réelles, pas seulement la charge.
- Suite : **D302** (accélérer le mouvement de J4), puis **J4.8** (clôture : D285 à D294, D300, D301 au GDD ; D274 à préciser : les ~8 Ko/s d'en-têtes étaient l'identifiant
  des éléments des tableaux rapides).

### 2026-10-14 (suite) — J4.7, D302 : accélérer le mouvement de J4 (analyse, rien de codé)

- Question à l'utilisateur sur S2 (A accélérer / B remesurer au repos / C relever le seuil) : **A** (**D302**), sans
  changer le mouvement validé (D290-D294) : seuls des changements de calcul, vérifiés par la partie « move » de
  `quick_check` et les essais de contournement de D293 (journal du 4e essai).
- **Lecture du code** (`Simulation/HOLSimulationProcessors.cpp`, `Commands/HOLUnitOrderProcessor.cpp`,
  `Simulation/HOLAvoidanceHoldProcessors.cpp`) et pistes, par coût mesuré à S2 :
  - **`MassMovingAvoidance` ~1,3 ms en moyenne** (7,6 ms un pas sur 6) : processeur de Mass **sur un seul cœur**
    (`ForEachEntityChunk`, `MassAvoidanceProcessors.cpp` ligne 331). Pistes : le faire tourner pour un sixième des
    unités à chaque pas (même cadence par unité, pic lissé, moyenne inchangée) ; ou une copie parallèle ; ou voir si le
    regard en avant (D293) le rend inutile entre unités (changerait le mouvement : à essayer en A / B seulement).
  - **`HOLUnitSeparation` 0,80 ms** : passes séquentielles (Gauss-Seidel), table hachée de listes chaînées, 9 cases
    par unité et par tour, 2 tours. Pistes : cases triées (tri par comptage) pour la mémoire, 3 plages par rangée au
    lieu de 9 cases, calcul en float.
  - **`HOLFullSpeed` (regard en avant) 0,70 ms** : déjà parallèle ; `BuildLookAhead` séquentiel (~0,08) ; à 3 m/s
    (S2), cases de 250 cm et portée ~420 cm → **25 cases** visitées par marcheur. Pistes : seulement les cases devant
    lui, cases triées / plages par rangée, regard un pas sur deux avec le virage gardé (D293 l'avait noté).
  - **`HOLUnitOrder` 0,58 ms** (sur le fil du jeu) : ~170 ordres par seconde dans S2 (ordres de flânerie du stress
    test) ; par ordre, requête de la grille d'obstacles, places (`ProjectPointToNavigation` + `Raycast`), chemin
    (`FindPathSync`, budget) ; par pas, `Raycast` du navmesh pour chaque unité en phase `Path` près de la fin de son
    chemin (D294). À **mesurer d'abord** (minuteurs par partie : `ApplyQueuedOrders`, `ServePathRequests`,
    `AdvanceUnits`, comme `hol.Units.Separation 2`).
- **Reprise** : mesurer les parties de `HOLUnitOrder` ; puis, dans l'ordre du gain attendu, la cadence étalée de
  l'évitement de Mass, la séparation, le regard en avant ; S2 court hors écran après chaque changement
  (`-RenderOffScreen -ResX=888 -ResY=500 -saveddirsuffix=Tests -abslog=…\Saved_Tests\Logs\s2_d302.log`,
  `-ExecCmds="r.VSync 0,t.MaxFPS 0,scalability 2,hol.Units.SpawnBudget -1,hol.StressTest S2 2000 10 quit"`), puis
  `quick_check`, S1 / S2, S5 (`net_match.ps1 -Stage -Units 2000 -Seconds 40`) et **J4.8**.

### 2026-10-15 — J4.7, D302 fait : S2 court 4,41 → 2,71 ms par pas (PASS), mêmes résultats

- **Mesure des parties de `HOLUnitOrder`** (`hol.Units.OrderTiming 1`, moyenne toutes les 200 pas) : 0,60 ms dont
  ordres appliqués 0,35 (~8,6 ordres d'une unité par pas dans S2 : projection et tir sur le navmesh pour la place),
  chemins 0,12, avancée des unités 0,13. Laissé tel quel (propre aux ordres de flânerie de S2). Séparation
  (`hol.Units.Separation 2`, parties ajoutées) : grille 0,05, passes 0,64 ms (Gauss-Seidel, séquentiel par nature).
- **Le seuil de S2 porte sur la moyenne par pas** : étaler l'évitement de Mass sur 6 pas aurait lissé le pic sans rien
  gagner. Gain retenu : **le faire tourner en parallèle**. Mass le passe morceau après morceau sur un seul cœur
  (`ForEachEntityChunk`) ; chaque unité n'écrit que sa force et sa vitesse voulue et ne lit chez ses voisines que
  position et vitesse : l'ordre ne change rien. Sans recopier le code de Mass : une sous-classe appelle son `Execute`
  sur une copie du contexte dont la **file de morceaux** (crochet de Mass 5.8, `SetChunkQueueFunc`) garde la fonction
  par morceau et la requête (`SourceQuery`) sans rien exécuter, puis relance chaque requête avec cette fonction en
  `ParallelForEachEntityChunk`. Même recette pour le **pilotage** et l'**orientation** de Mass
  (`Simulation/HOLParallelMassProcessors`, trois classes `UHOLParallel*Processor` dans `DefaultGame.ini`, noms ajoutés
  aux contraintes d'ordre des processeurs de maintien ; `hol.Simulation.ParallelMass 0` pour un A / B). Premier essai
  planté : l'exigence de journal de débogage (`DebugEnableEntityOwnerLogging`) manquait à une requête recopiée ; la
  version finale réutilise la requête de Mass elle-même.
- **Regard en avant (D293)** : seulement les cases que touche le carré de sa portée (9 à 16 au lieu de 25), arrêt au
  premier contact pour juger un virage, morceaux répartis au fil de l'eau (`AutoBalance`). Mêmes candidats dans le même
  ordre : même résultat.
- **Mêmes résultats** : à chaque passage de S2 (graine fixe), 522 paires l'une sur l'autre au début de la mesure et 374
  à la fin, écarts minimaux -15,0 / -15,8 cm, 1 730 ordres : identiques avant et après chaque changement. Les essais
  de contournement de D293 n'ont donc pas été refaits.
- Mesures (S2 court hors écran, 888 × 500, 10 s ; machine à ~29 % de charge, éditeur ProjectShiva ouvert), par pas :
  évitement en marche 7,6 → **1,14 ms par passage** (un pas sur 6), pilotage 0,19 → 0,12, orientation 0,24 → 0,11,
  regard en avant 0,70 → 0,52 ; **simulation 2,71 ms par pas, PASS** (2,57 avec les minuteurs, écart de bruit), rendu
  238 i/s PASS. **S1 court PASS** (2,24 ms, 250 i/s). **`quick_check` PASS en 64 s** (tests 32/32, « move » et « net »).
- Restent les plus gros postes : séparation 0,75 ms, ordres 0,51 ms (S2 seul), regard en avant 0,52 ms.
- **S5 court PASS** (`net_match.ps1 -Stage -Units 2000 -Seconds 40`, serveur recuit : sa configuration nomme les
  nouveaux processeurs) : 30 007 o/s par client, 1,72 envoi par unité et par seconde, écart affiché 23,0 cm (D301 :
  29 970 o/s, 23,6 cm). Simulation du serveur 6,4 ms par pas, mesurée avec les 2 clients sur la même machine.
- Suite : **J4.8** (clôture : D285 à D294, D300 à D302 au GDD ; D274 à préciser).

### 2026-10-15 (suite) — J4.8 : clôture, J4 close

- GDD § 21 : lignes D285 à D294 et D300 à D302 (colonne « Sections » : § 16.6) ; D294 complétée (sa décision
  attendue a été tranchée par D300 et D301) ; **D274 précisée** (les ~8 Ko/s d'en-têtes étaient l'identifiant des
  éléments des tableaux rapides). § 16.6 : paragraphe « Parcours J4 » après celui de J3, ligne **M3** du tableau marquée
  faite (sauf les formations, M4), phrase des en-têtes de la tranche M2.5 mise à jour.
- **J4 close.** Suite dans l'ordre de D252 : **J5a.1** (plan de J5 déjà fait, D295 à D299).

### 2026-10-15 (suite) — J5a.1 : données et règles pures de la récolte

- `Match/HOLResources.h` : `EHOLResource` (une des 4 ressources : ce que donne une source, ce que porte un Paysan),
  `FHOLResources[ressource]`, `FHOLResourceRates` (un taux par ressource, par seconde).
- Table des unités : colonnes `GatherRates` et `CarryCapacity` (Paysan 0,7/s pour les 4, charge 10 ; soldats 0),
  recopiées dans le fragment partagé `FHOLUnitTypeConstants` pour la simulation de J5a.3 ; `UnitTypes` réimportée.
- Table des sources (D297) : `Resources/HOLSourceTypes` (`FHOLSourceTypeRow` : nom, ressource, quantité, rayon, hauteur
  et maillage provisoires ; `UHOLSourceSettings::FindType`), `Tools/Data/SourceTypes.csv` (`Tree` bois 100, rayon 60 ;
  `BerryBush` nourriture 200, rayon 110 ; `StoneVein` / `GoldVein` 2 500, rayon 300), `Tools/Editor/make_source_types.py`
  → `/Game/Resources/SourceTypes`, déclarée dans `DefaultGame.ini`.
- Règles pures (D296) : `Simulation/HOLGatherRules` : charge (une ressource, unités entières et progression vers la
  suivante), `StartGathering` (autre ressource : charge perdue), `Gather` (un pas au taux, jusqu'à la charge ou la fin de
  la source), `IsFull`, `Deposit` (le stock ne monte qu'ici), distance au bord d'un corps et contact (à 30 cm près),
  `FindNearestDeposit` (au bord le plus proche), `FindNeighbourSource` (même ressource, non vide, centre à 15 m au plus,
  sinon inactif).
- Tests `HeroesOfLiberty.Gather.Load` / `.Deposit` / `.Choice` / `.Tables`. Compile sans avertissement ; tests 36/36,
  **`quick_check` PASS en 87 s**. S1 / S2 non concernés (rien dans la simulation encore).
- Suite : **J5a.2** (sources dans la partie).

### 2026-10-15 (suite) — J5a.2 : sources dans la partie

- **Registre** `Resources/HOLSourceSubsystem` (sous-système de monde) : au serveur ou en solo, lit au début de la partie
  les `AHOLSourceLayout` de la carte (données seules, pas chargées par les clients), `AddSource` (aussi pour du code :
  S7), `Take` (une source vidée quitte la carte : affichage, navmesh, clients), `FindNearestStanding` ; chez un client,
  remplit sa copie depuis les morceaux reçus. Les types sont numérotés dans l'ordre de la table (même ressource partout).
  Commandes : `hol.Sources.Report`, `hol.Sources.Nearest X Y`, `hol.Sources.Take Index [N]` (outil de développement, hors
  Shipping, en attendant la récolte de J5a.3).
- **Affichage** `AHOLSourceField` (acteur local, non répliqué) : un composant d'instances par pièce de chaque type
  (arbre = tronc + cône vert ; buisson = boule verte + baies rouges ; filon = trois blocs gris ou dorés), petite variation
  de rotation et de taille tirée de la position (la même partout) ; une source vidée passe à l'échelle 0 (les indices des
  autres ne bougent pas). `M_MatteColor` reçoit le drapeau d'usage des maillages instanciés (`make_matte_materials.py`
  pose désormais les drapeaux sur un matériau existant ; sans lui le jeu dessinait le matériau par défaut, gris).
- **Navmesh** (D297, « seule sa tuile se refait ») : `UHOLSourceNavComponent`, un seul élément de navigation, un
  modificateur cylindrique interdit « par instance » (le mécanisme des maillages instanciés) : le générateur demande les
  cylindres de chaque tuile qu'il refait ; ajout ou épuisement → `FNavigationSystem::OnObjectBoundsChanged` avec la seule
  boîte de la source ; inscription après la pose des premières sources (salir toutes leurs boîtes une fois). Vérifié : un
  arbre debout ferme sa place (« off the navmesh »), vidé il l'ouvre (chemin de 5,2 m), l'arbre derrière la ferme encore.
- **Réseau** : `AHOLSourceChunk`, recette des morceaux d'unités (63 emplacements, tableaux simples) : type et position
  une fois, reste sur 16 bits, 2 mises à jour/s (`HOLSourceSettings.NetUpdateFrequency`), envoi forcé à l'épuisement.
- **`Plain` refaite** (`create_match_map.py`) : un bois de 40 arbres (5 × 8, 2,2 m, décalages de 20 cm au plus, les
  mêmes partout) à gauche de chaque emplacement vu de lui, de 18 à 27 m, loin de la naissance des Paysans ; avec l'agent
  de 60 cm, un bois est un mur (comme AoE) et recule arbre par arbre. Le bosquet du milieu (obstacles de J4) reste.
  Arbre provisoire : 7,5 m de haut (CSV), couronne de 3,2 m.
- `quick_check` : partie **« sources »** (320 arbres posés, affichés, interdits ; le plus proche ; navmesh fermé, ouvert
  après `Take`, l'arbre derrière toujours fermé ; capture `src_wood`) et, dans **« net »**, le serveur vide le premier
  arbre 5 s après l'arrivée des 2 clients (`hol.Net.WhenClients 2 hol.Delay 5 hol.Sources.Take 0`) : chaque client voit
  319 arbres sur 320, sans navmesh. **PASS en 83 s**, tests 36/36.
- **Écart au plan** : pas d'arbres posés dans la carte `StressTest` (S1 et S2 auraient changé) ; S7 (J5a.3) posera les
  siens par code (`AddSource`). S1 / S2 / S5 non concernés (aucune source sur `StressTest`).
- Suite : **J5a.3** (boucle de récolte dans la simulation, S7).

### 2026-10-15 (suite) — J5a.3 : boucle de récolte dans la simulation, S7

- **Fragment** `FHOLGatherFragment` (source, place visée, charge, phase : vers la source, récolte, vers le dépôt) sur
  chaque unité. **Ordres** : `FHOLGatherOrder` dans la file de commandes (`IssueGatherOrder`) ; un ordre de déplacement
  porte `bFromTask` : ceux que la récolte se donne la laissent continuer, ceux du joueur l'arrêtent (charge gardée, D296).
- **`Simulation/HOLGatherProcessor`** (après les ordres, au serveur et en solo, fil de jeu) : applique les ordres de
  récolte (propriétaire vérifié, type d'unité qui récolte cette ressource ; autre ressource → charge perdue) ; choisit
  une **place libre au contact** de la source (tous les 30° depuis le côté qui fait face au Paysan, 15 cm à l'intérieur
  du navmesh, 1,4 m entre deux récolteurs, et le navmesh doit être ouvert vers l'extérieur sur 3 m : entre quatre arbres
  d'un bois restent des îlots que nul chemin n'atteint, un Paysan y était resté coincé) ; sinon la voisine la plus proche
  avec une place libre (dans 15 m), sinon il attend 3 s ; récolte au contact (règles de J5a.1) ; plein → **dépôt le plus
  proche de son propriétaire** qui prend la ressource (emprise rectangulaire, place contre le côté qui lui fait face,
  étalée sur 85 % du côté selon l'unité, glissée de 1,5 m à chaque raté ; déchargement à 1,5 m de l'emprise au plus) ;
  stock du `PlayerState` monté au dépôt seulement ; retour à la même source ; source épuisée → voisine du même type
  dans 15 m, sinon décharge puis inactif. Un récolteur bousculé continue jusqu'à 1,2 m de sa source.
- Règles pures complétées (`HOLGatherRules`) : dépôts en emprise tournée (`FHOLGatherDeposit`, distance, place, dépôt
  le plus proche) ; tests `Gather.Choice` et `Gather.Tables` étendus (le centre reçoit les 4 ressources).
- Table des bâtiments : colonne **`Accepts`** (le centre : les 4) et ligne provisoire **`LumberCamp`** (6 × 6 m, bois
  seul), pour S7 seulement ; l'étape construction donnera leur place aux camps (D83).
- Commandes : `hol.Units.Gather X Y [Slot]`, `hol.Gather.Report [Slot]`, `hol.Gather.LogDeposits 1`.
- Essai solo sur `Plain` (5 Paysans sur le premier arbre) : ils se répartissent sur les arbres voisins, **200 bois en
  83 s**, le stock monte de 10 à chaque déchargement.
- **S7** (`hol.StressTest S7 [N] [s] [quit]`, posé par code sur `StressTest`, la carte ne change pas) : 8 joueurs, un
  centre chacun à 50 m du milieu, 60 arbres isolés par joueur (anneaux de 60 à 92 m, 4 m d'écart, **400 bois** chacun :
  `hol.StressTest.TreeAmount`, sinon le champ est rasé avant la fin), Paysans nés au pied de leur centre, 4 par arbre,
  chauffe de 25 s. Rapport : bois déchargé par seconde, arbres épuisés, chemins par seconde, marches et ratés, phases.
  Mesures (S7 court hors écran, 888 × 500, 10 s ; machine chargée, ProjectShiva ouvert) :
  - 1er passage : **4,29 ms** (ordres 1,06 ms, 375 chemins/s ; 2 115 paires de corps l'un sur l'autre au dépôt :
    250 Paysans par centre) ;
  - les marches de la récolte prennent leur place telle quelle (ni recherche de place ni projection : « orders
    applied » 0,84 → 0,02 ms) ; arrivée à 60 cm, bousculé jusqu'à 1,2 m ; points de déchargement étalés : **4,15 ms** ;
  - **3 camps de bûcherons par joueur** dans ses arbres (~60 récolteurs par dépôt, ce que voient au plus les camps d'une
    vraie économie ; § 5.3 : 70 à 90 travailleurs par joueur) : foules normales (409 paires), séparation 1,06 → 0,41,
    regard en avant 1,0 → 0,33 ; puis assez d'arbres (480) et nouvelle tentative de place toutes les 3 s :
    **S7 PASS à 2,96 ms** ; 1 403 bois/s (0,70 par Paysan : le taux plein), 307 chemins/s, 75 % des Paysans récoltent.
  - Postes : évitement à l'arrêt 0,72 ms par pas (1 500 récolteurs immobiles), ordres 0,38, séparation 0,44, regard en
    avant 0,34, récolte 0,22. Rendu non mesurable (45 i/s machine chargée ; 149 i/s au 1er passage).
- **S2 court** : mêmes recouvrements qu'en D302 (522 puis 374 paires : mouvement inchangé), 2,98 ms machine chargée
  (2,71 en D302) ; la récolte coûte 0,04 ms sans récolteur. **S1 / S2 / S7 à remesurer au repos.**
- **Réutilisation des chemins source-dépôt (D299) pas faite** : les chemins eux-mêmes ne coûtent que ~0,2 ms par pas à
  300 par seconde (presque tous droits) ; à reprendre si S7 repasse au-dessus de 3 ms.
- `quick_check` PASS en 89 s, tests 36/36.
- Suite : **J5a.4** (ordre réseau et ralliement).

### 2026-10-15 (suite) — J5a.4 : ordre de récolte du joueur, ralliement, bloc du Paysan

- **Viser une source** : `AHOLPlayerController::FindSourceAt`, la recette de la sélection des unités (D288) : chaque
  source debout projetée à l'écran, du sol à son sommet, de la largeur dessinée (`AHOLSourceField::GetDrawnRadius` :
  la couronne d'un arbre), celle de devant l'emporte.
- **Clic droit** (`OrderAt`) : Paysans à soi sélectionnés + source sous le curseur → `ServerGatherUnits` (identifiants et
  index de la source ; le serveur ne garde que les unités du joueur, D288) → file de commandes ; le **repère bref de
  J4.6** se pose sur la source. Bâtiment à soi sélectionné + source → **ralliement sur la source**
  (`FHOLProductionCommand::SourceIndex`, `AHOLBuilding::RallySource` répliqué au propriétaire, vérifiée debout par le
  serveur) ; à la naissance, l'unité reçoit un ordre de récolte si la source est encore debout, sinon un déplacement.
- **État répliqué** (D298, avancé de J5c.1) : `EHOLUnitActivity::Gathering` (dans les 3 bits existants), charge
  (`LoadResource`, `LoadAmount`) : **1 bit** « porte quelque chose », puis 2 + 5 bits pour un porteur seulement
  (60 bits, 67 pour un porteur) ; la charge n'est pas une raison d'envoi (D269), le client la voit au rappel lent ;
  tests `Net.Reduction` et `Net.ElementFormat` étendus. La récolte met l'activité à « récolte » au contact de sa source.
- **Bloc du Paysan** (`UHOLGameScreen`) : une ligne pour un récolteur seul : « Récolte — Bois 4/10 », « En marche —
  Bois 10/10 », « Inactif » ; textes fr / en / ja (`fill_po.py`, 108 textes) ; `hol.HUD.Report` l'écrit.
- **Arbres** : couronne remontée (de 0,40 à 1,0 de la hauteur) et resserrée (2,9 m) : un Paysan au pied de son arbre se
  voit (caché sous la couronne avant).
- Commandes de test : `hol.Select.OrderSource X Y` (clic droit là où est dessinée la source debout la plus proche du
  point), le ralliement par le même chemin (centre sélectionné).
- Essais scriptés solo : ralliement sur un arbre → les 3 Paysans nés y récoltent ; cadre + clic droit au sol → récolte
  arrêtée, charge gardée ; clic droit sur l'arbre → reprise ; capture du bloc (« Récolte — Bois 4/10 »).
- `quick_check` PASS (caméra 8p : flèche tenue 1 s = 52 m au lieu de ~40, à-coup de la machine chargée ; PASS au second
  essai) ; **S5 court PASS : 31 283 o/s par client** (30 007 en D302 : 1 bit ≈ 0,5 Ko/s, le reste machine chargée,
  1,77 envoi par unité et par seconde contre 1,72), écart affiché 22,2 cm.
- Suite : **J5a.5** (vérification et essai de l'utilisateur).

### 2026-10-15 (suite) — J5a.5 : 1er essai de l'utilisateur, retouches (D303)

- Retours : le Paysan doit **naître du côté du point de ralliement** ; le bloc du Paysan n'est **pas à jour** (« marche »,
  puis « Bois 3/10 », puis « Bois 10/10 ») : il doit suivre chaque changement d'activité ou de charge ; à terme seules
  les ressources resteront affichées ; proposition : la charge en temps réel seulement pour l'unité sélectionnée seule.
- Cause du retard (solo) : `UHOLGameScreen` ne redessinait le bloc qu'aux délégués d'économie, de sélection et de
  production, donc au dépôt. Correctif : la ligne d'un récolteur sélectionné seul est relue à chaque image
  (`HOLUnitView::Find`, une seule unité, sans parcourir les 2 000) et réécrite quand elle change.
- **Réseau (D303)** : `AHOLPlayerController` envoie au serveur son unité suivie (`ServerWatchUnit`, à chaque changement
  de sélection : sa propre unité de récolte sélectionnée seule, ou 0) ; le serveur lit sa charge à chaque image et la
  renvoie au seul propriétaire quand elle change (`FHOLWatchedLoad`, `COND_OwnerOnly`, `ForceNetUpdate`) ; le client la
  donne à `UHOLUnitReceptionSubsystem::SetWatchedLoad`, qui l'applique à la vue de cette unité. `FHOLNetUnitState` :
  `LoadAmount` retiré, `bCarrying` + ressource (60 bits, **62** pour un porteur au lieu de 67) ; tant que la charge
  suivie n'est pas arrivée, le bloc montre l'activité seule. `s5_report.py` : `STATE_BITS = 60` (oublié en J5a.4).
- **Naissance** (`UHOLProductionSubsystem::GetBirthLocation`) : arc tourné vers le ralliement s'il est hors de l'emprise,
  une seule rangée ; sinon en bas du centre.
- Compile sans avertissement ; tests 36/36 (`Net.Reduction`, `Net.ElementFormat` revus) ; `quick_check` **PASS en
  64 s**. Partie à 2 clients de la charge suivie pas encore contrôlée par script (à ajouter avec J5a.5).
- Suite : **2e essai de l'utilisateur**, puis la suite de J5a.5.
- **2e essai** : « tout est bon », sauf la rotation du Paysan, à rendre **instantanée** (D304). Fait :
  `UHOLInstantOrientationProcessor` à la place de l'orientation lissée de Mass (classe parallèle et trait retirés,
  `DefaultGame.ini`), affichage du client sans vitesse de rotation. Compile sans avertissement ; `quick_check` **PASS en
  68 s** ; S2 court hors écran : orientation 0,10 ms (0,11 avant), recouvrements 547 → 387 paires (522 → 374), **3,33 ms
  FAIL machine chargée** (48 % sans rien du projet, éditeur ProjectShiva : tous les postes ~25 % au-dessus de D302) :
  à remesurer au repos avec S1 / S7. **Serveur cuit à recuire** avant le prochain S5 (sa configuration nommait la classe
  retirée).
- **3e essai validé** (« c'est bon »). Commit et push ; arrêt de la séance, reprise dans une autre.
- **Reprise** : reste de J5a.5 : `quick_check` solo (ralliement sur un arbre, stock qui ne monte qu'au dépôt, arbre
  voisin, inactif) et à 2 clients (récolte vue, arbre épuisé disparu chez les deux, stock de l'autre jamais reçu, **charge
  suivie de D303** reçue par le seul propriétaire) ; S1 / S2 / S7 courts **au repos** ; serveur recuit puis S5 court ;
  dernier essai de l'utilisateur pour clore J5a (disposition de S7, chemins non réutilisés, forme des arbres).

### 2026-10-15 (suite) — J5a.5 : récolte solo vérifiée à la main

- **Partie solo** (ralliement sur un arbre, stock qui ne monte qu'au dépôt, arbre voisin, inactif) : **vérifiée à la
  main par l'utilisateur** ; pas de partie « récolte » solo scriptée dans `quick_check` (choix de l'utilisateur).
- **Partie à 2 clients de la récolte** dans `quick_check` (partie « net », avec « sources ») : 6 s après l'arrivée des
  2 clients, le serveur laisse 5 bois dans l'arbre le plus proche de chaque emplacement (`hol.Sources.TakeNear X Y N`,
  outil de développement) ; chaque client met la caméra sur son emplacement (`hol.Camera.FocusSlot own`), sélectionne
  son Paysan seul, clic droit sur l'arbre le plus proche de son emplacement (`hol.Select.OrderSourceNear`), puis toutes
  les 2 s `hol.HUD.Report` et `hol.Units.Where` (qui dit désormais l'activité et la charge de chaque unité, « ? » quand
  la machine ne connaît pas la quantité). Contrôles par client : ordre passé, **charge exacte au HUD en temps réel**
  (0, 1, 2, 4, 5… /10, « Moving Wood 10/10 » vers le dépôt), Paysan de l'autre vu « Gathering » et « carries Wood ? »,
  **quantité de l'autre jamais reçue**, les 2 arbres visés **épuisés et disparus chez les deux** (317 debout), **bois à
  210** au dépôt et stock de l'autre jamais reçu, capture ; au serveur : chaque joueur a déchargé 10 bois. Les contrôles
  de la partie « net » lisent leur section (`Test mark: net` / `gather`).
- `-Parts` : passé par `-File`, la liste arrive en une seule chaîne (aucune partie ne tourne) ; passer par
  `-Command "& .\Tools\Tests\quick_check.ps1 -Parts net,sources,move"` (noté en tête du script).
- Compile sans avertissement ; `quick_check` **PASS en 70 s** (la partie « net » tourne en parallèle du lancement
  principal : rien d'ajouté), tests 36/36.
- **Reprise** : S1 / S2 / S7 courts **au repos** (ProjectShiva fermé) ; serveur recuit puis S5 court ; dernier essai de
  l'utilisateur pour clore J5a (disposition de S7, chemins non réutilisés, forme des arbres).

### 2026-10-15 (suite) — J5a.5 : essai de l'utilisateur, réutilisation des chemins (D305)

- **Essai de l'utilisateur** sur `Plain` : arbres « super ». Remarque : pouvoir cliquer sur un arbre pour voir ce qu'il
  lui reste, prévu en **J5b.2** (D298). **S7 regardé** sur le deuxième écran : disposition validée (« super »).
- **D305 (choix de l'utilisateur, contre la recommandation d'attendre)** : la réutilisation des chemins de la récolte,
  faite maintenant. Dans `UHOLUnitOrderProcessor` : une marche que la récolte se donne (une unité, `bFromTask`) cherche
  un chemin gardé par cases de 6 m de son départ et de son arrivée (`hol.Units.ReusePaths`, `hol.Units.ReuseCellSize`) ;
  un chemin gardé droit n'est repris que si un rayon sur le navmesh du Paysan à sa place est libre
  (`hol.Units.ReuseCheckStraight`) ; un détour reprend son chemin et son couloir. Une reprise ne compte pas dans le
  budget de requêtes par pas. Un chemin gardé est oublié quand le navmesh change sur son trajet (source épuisée par
  `UHOLSourceNavComponent`, bâtiment posé par `AHOLBuilding` → `UHOLCommandSubsystem::NotifyNavigationChanged`), et un
  chemin trouvé dans les 5 s qui suivent un changement sur son trajet n'est pas gardé (tuiles en cours de reconstruction) ;
  oublié aussi après 60 s sans usage. S7 compte les reprises (« kept paths reused »).
- Mesures (S7 court hors écran, 20 s, deux ProjectShiva ouverts : machine très chargée, S7 FAIL à ~3,3 ms avec ou sans) :
  cases de 1 m → 8 % de reprises ; 3 m → 45 % ; 6 m sans rayon → 83 %, chemins 0,154 → 0,070 ms ; **6 m avec rayon → 77 %,
  chemins 0,157 → 0,108 ms par pas** (gain ~0,05 ms, le rayon en coûte ~0,04), bois récolté inchangé (1 164 contre
  1 166 par seconde).
- Compile sans avertissement ; `quick_check` **PASS en 73 s**, tests 36/36. S2 non concerné (ordres du joueur).
- **Reprise** : S1 / S2 / S7 courts **au repos** (ProjectShiva fermé) ; serveur recuit puis S5 court ; clôture de J5a.

### 2026-10-15 (suite) — Clôture de J5a

- **Mesures reportées** (choix de l'utilisateur) : deux éditeurs ProjectShiva ouverts, processeur à 57 % ; S1 / S2 / S7
  courts au repos et S5 sur le serveur recuit restent à faire, à la première occasion au repos, au plus tard à la
  vérification de J5b.
- **Clôture** : D295 à D299 et D303 à D305 au § 21 du GDD (D295 à D299 avancées de J5c.4 : décisions prises, journal
  gardé complet et dans l'ordre) ; D297 précisée (morceaux en tableaux simples, cylindres par source), D299 précisée
  (solo vérifié à la main, 3 camps provisoires dans S7, réutilisation des chemins par D305) ; paragraphe « Parcours
  J5a : le bois » au § 16.6 (mesures à refaire au repos notées) ; S7 dans les seuils des stress tests (S1 à S7).
  J5c.4 n'a plus qu'à écrire le paragraphe « Parcours J5 ». **J5a close.**
- **Reprise** : S1 / S2 / S7 courts au repos et S5 (serveur recuit) si la machine est au repos ; puis **J5b.1** (baies et
  filons de pierre et d'or sur `Plain`, récolte des 4 ressources, charge perdue au changement de ressource).

### 2026-10-15 (suite) — J5b.1 : baies et filons

- Machine toujours chargée (ProjectShiva, 66 %) : mesures au repos encore reportées.
- **`Plain` refaite** (`create_match_map.py`) : après les 320 arbres (la source 0 reste le premier arbre de l'emplacement
  0), **6 buissons de baies** par emplacement (2 × 3, 3 m d'écart : 80 cm entre deux, fermé à une unité, récoltés par le
  bord), de l'autre côté que le bois, **à 20-23 m** : plus loin que les arbres les plus proches (17,9 m), que visent les
  contrôles de `quick_check` ; puis **8 filons** à 120 m du milieu, à l'angle médian de deux emplacements voisins,
  **pierre et or alternés** : chaque emplacement a un filon de chaque à **60 m**, à égale distance de son voisin
  (contestés, D295) ; le chemin vers celui du côté du bois passe à ~7 m du bois. 376 sources en tout.
- **Places autour d'une source** (`UHOLGatherProcessor::FindSourceSpot`) : le pas entre deux places essayées est celui
  de deux récolteurs côte à côte, 30° au plus : autour d'un filon (3 m de rayon), **16 places au lieu de 12** ; arbres
  et baies inchangés (30°).
- Rien d'autre à changer : la simulation, l'affichage (formes de J5a.2), la visée et les textes du bloc (Nourriture,
  Pierre, Or en fr / en / ja) étaient déjà génériques.
- **Essai scripté solo** (hors écran, 10 Paysans de l'emplacement 0, `hol.Units.Gather`) : baies → **130 nourriture en
  40 s** ; filon de pierre → **100 pierre en 55 s**, « 69 carried lost » (la nourriture portée, D296) ; filon d'or →
  **100 or**, « 41 carried lost » ; les 10 récolteurs tiennent ensemble au filon ; captures conformes.
- `quick_check` : comptes passés à 376 sources (« sources » et « net ») ; **PASS en 69 s**, tests 36/36. S1 / S2 / S7 non
  concernés (S7 : arbres seuls, pas des places inchangé autour d'un arbre).
- **Reprise** : **J5b.2** (HUD).

### 2026-10-15 (suite) — S1 / S2 / S7 courts au repos

- Machine au repos (ProjectShiva fermé, ~10 % du processeur, aucun Unreal ni compilation), binaires de 3c60912 (J5b.1),
  hors écran (`-RenderOffScreen -ResX=888 -ResY=500`, journaux `Saved_Tests\Logs\rest_s1.log`, `rest_s2.log`,
  `rest_s7.log` ; images par seconde non comparables à la référence en 1920 × 1080) :
  - **S1** (10 s) : **2,233 ms PASS** (2,24 en D302) ; évitement à l'arrêt de Mass 8,5 ms un pas sur 6 ;
  - **S2** (10 s) : **2,779 ms PASS** (2,71 en D302, 2,98 machine chargée) ; évitement en marche 1,29 (un pas sur 6),
    séparation 0,75, ordres 0,55, regard en avant 0,53 ; 547 puis 387 paires l'une sur l'autre ;
  - **S7** (20 s) : **2,906 ms PASS** (2,96 machine chargée), **marge mince** (0,09 ms) ; évitement à l'arrêt 5,3 ms un
    pas sur 6, séparation 0,42, regard en avant 0,31, ordres 0,27, récolte 0,22 ; 1 170 bois/s, 62,5 chemins/s,
    207,8 chemins gardés repris/s (77 %).
- GDD § 16.6 (« Parcours J5a ») : mesures au repos ajoutées à côté de celles sur machine chargée.
- **S5 court au repos** (`net_match.ps1 -Stage -Units 2000 -Seconds 40`, serveur recuit sur 3c60912, 211 s en tout) :
  **PASS à 31 082 o/s par client** (seuil 32 000 : **marge de 0,9 Ko/s**), 1,76 envoi par unité et par seconde, 42
  paquets/s, écart affiché 22,7 cm (seuil 25), réplication 0,149 ms par pas. Client : états 25,2 Ko/s, le reste 5,9 Ko/s.
  Machine chargée en J5a.4 : 31 283 o/s ; la hausse depuis D302 (30 007 o/s) n'est donc **pas** due à la charge : c'est
  l'état de la récolte (activité, porteur et ce qu'il porte, D303). Tout bit ajouté à l'état répliqué en J5b / J5c
  coûte ~0,5 Ko/s et peut faire échouer S5. Simulation du serveur 6,99 ms par pas (6,4 en D302), mesurée avec les 2
  clients sur la même machine : non comparable au seuil de S2.
- Plus aucune mesure reportée. Reprise inchangée : **J5b.2** (HUD).

### 2026-10-15 (suite) — Essai de J5b.1 ; D306 (charge perdue au premier point)

- Mesures au repos faites par un autre agent sur le code de J5b.1 (c234f84, 535b7c8) : S1 2,233 ms, S2 2,779 ms, S7
  2,906 ms, S5 31,1 Ko/s, toutes PASS ; plus aucune mesure reportée.
- **Nouvelle règle de l'utilisateur** : ne plus rien committer tant qu'il n'a pas validé que tout marche.
- **Essai de l'utilisateur** : tout est bon, sauf le changement de ressource : la charge ne doit être perdue qu'au
  **premier point de la nouvelle ressource** récolté, pas dès l'ordre (**D306**, précise D296).
- Fait : `HOLGatherRules` (`StartGathering` ne perd plus rien et donne sa ressource à une charge vide ; `Gather` prend la
  ressource récoltée, remplace la charge au premier point entier d'une autre ressource et rend la quantité perdue ;
  `IsFullOf`) ; `UHOLGatherProcessor` (plein « de la ressource de la source » pour partir au dépôt, voisine cherchée
  d'après la ressource de la source, perte journalisée avec `hol.Gather.LogDeposits 1`) ; tests `Gather.Deposit`
  étendus (charge gardée à l'ordre et pendant 1 s, perdue au premier point ; plein d'une autre ressource : il continue).
- Essai scripté solo : 3 s après l'ordre sur la pierre, les Paysans en route portent encore leur nourriture (10, 10, 9,
  10, 1) ; perte au premier point de pierre, 13 à 17 s plus tard ; de même pour l'or.
- `quick_check` : un FAIL de la partie « net » (le client 2 ne connaissait plus l'autre joueur à son dernier rapport :
  l'autre client était parti), PASS au second essai, puis **PASS complet en 72 s**, tests 36/36.
- **Non commité** (règle de l'utilisateur) : J5b.1 + D306 attendent sa validation. Reprise : essai de D306, puis J5b.2.
- **Essai de D306 validé** par l'utilisateur ; D306 portée au GDD (ligne au § 21, D296 et « Parcours J5a » précisés).
  Commit, arrêt de la séance. **Reprise** (nouvelle séance) : **J5b.2** (HUD).

### 2026-10-15 (suite) — J5b.2 : HUD de la récolte (D307)

- **D307** (choix d'exécution, voir « Décisions ») : `EHOLUnitActivity::GatherTrip` (« en tournée de récolte »), tenue
  par `UHOLGatherProcessor` à chaque pas d'après la phase de la récolte (`SyncActivity`) ; un récolteur arrêté sur une
  marche qu'il s'était donnée finit « en marche » ; `IsWalkingActivity` pour l'orientation chez le client. Comptes par
  joueur et par ressource (`CountGatherer`, `PublishGathererCounts`) → `AHOLPlayerState::Gatherers`, au seul propriétaire.
- **Contrôleur** : sélection d'une source (clic gauche : unité, sinon bâtiment, sinon source ; une source épuisée quitte
  la sélection), `GetIdleWorkers` (Paysans du joueur inactifs, relus 4 fois par seconde avec la purge de la sélection),
  `SelectNextIdleWorker` (règle pure `HOLUnitPicking::NextInCycle`, test `Movement.Picking` étendu), `IA_NextIdle`
  (`make_game_input.py` relancé), commandes `hol.Select.ClickSource X Y` et `hol.Select.NextIdle`. Anneau blanc autour
  de la source sélectionnée (`AHOLSelectionMarkers`).
- **HUD** : « (N) » après chaque ressource, bouton « Inactifs : N » (grisé à 0, `hol.HUD.Click IdleWorkers`) ; bloc d'une
  source (nom par `HOLGameNames::GetSourceName`, « Reste : N » relu à chaque image, sans propriétaire ni points de vie) ;
  bloc d'un groupe : « 3 en récolte · 2 inactifs » (relu 4 fois par seconde) ; un Paysan seul en tournée affiche
  « Récolte — Bois 0/10 ». `hol.HUD.Report` ajoute en fin de ligne « gatherers F 1 W 2 S 0 G 0, idle 2 » (les contrôles
  de J3 lisent le début). Textes fr / en / ja (`fill_po.py`, deux collectes).
- Compile sans avertissement. **Essai scripté solo** (hors écran, `Saved_Tests\j5b2.seq`) : 5 Paysans nés → « idle 5 » ;
  « . » → #1, puis #2, clic sur le compteur → #3 ; deux au bois, un aux baies → « gatherers F 1 W 2, idle 2 » ; cadre
  sur les 5 → « 3 en récolte · 2 inactifs » ; filon d'or → « Filon d'or, Reste : 2500 », anneau ; arbre → « Arbre,
  Reste : 100 » (l'anneau reste caché sous les couronnes) ; Échap → rien. Captures `j5b2_*` conformes.
- `quick_check` **PASS en 77 s**, tests 36/36 ; à 2 clients, chaque client voit « gatherers W 1, idle 0 » pour lui et
  les deux Paysans « GatherTrip » sur le chemin.
- Non mesurés (machine chargée : deux ProjectShiva ouverts) : S5 et S7 courts, à faire en J5b.3.
- **Non commité** (règle de l'utilisateur). Reprise : **essai de l'utilisateur**, puis J5b.3.

### 2026-10-15 (suite) — Essai de J5b.2 ; D308 (détours des allers-retours)

- **Essai de l'utilisateur** : HUD « super » ; mais quand plusieurs Paysans font l'aller-retour vers le centre, certains
  font d'énormes détours (l'utilisateur supposait un chemin calculé quand la ligne droite n'était pas libre).
- **Première piste, juste mais secondaire** : un détour gardé (D305) était repris sans contrôle par une autre marche
  entre les mêmes cases de 6 m (un chemin autour d'un arbre servait au voisin qui pouvait aller droit). Contrôle ajouté
  (`hol.Units.ReuseCheckDetour`, journal `hol.Units.LogDetours`) ; mesuré : 2 détours gardés sur ~60 s, pas la cause.
- **Mesure de ce qui est marché** (`hol.Gather.LogWalks` : distance parcourue, durée, écart à la ligne droite ; scénario
  `Saved_Tests\detours2.seq` : 20 Paysans au bois 60 s puis aux baies 40 s, hors écran) : ~35 marches 2 à 3 fois trop
  longues, avec ou sans le contrôle ; regard en avant coupé (`hol.Units.Avoid 0`) : encore 21. Tracé à 2 Hz
  (`hol.Units.Where`, `hol.Units.Orders`) : les grands écarts sont en phase **Path** (suivi d'un chemin du navmesh) ;
  le Paysan #17, du bois aux baies, passe à 7 m du coin du centre ; #18 sort du bois par un arc de 10 m de rayon.
- **Cause** : en phase Path, `UHOLFullSpeedProcessor` gardait la direction du pilotage de Mass (~50°/s). **Correctif**
  (`hol.Units.PathAim 1`) : viser le point du suivi de Mass plus 1 m le long du chemin, à chaque pas. A/B même scénario :
  marches anormalement longues **37 → 2**, mètres de trop 647 → 18, pire cas 44 → 9,6 m.
- **Poche du navmesh** : un Paysan à (-14366, -1845), entre les deux premiers arbres du bois, « manquait » le dépôt 6 fois
  sans bouger puis abandonnait sa charge (vu avec et sans les correctifs) ; `hol.Nav.Path` depuis ce point : chemin
  partiel de 0 m, une île fermée du navmesh. Correctif : chemin partiel finissant à moins de 2 m du départ → marche droite
  vers la place. Ensuite : dépôts manqués **15 → 0**, 4 sorties de poche, 71 déchargements (meilleur des essais).
- Un plantage pendant les mesures (19 h 28) : assertion du moteur au démarrage (`InheritedContext.cpp:130`, chargement des
  modules, avant tout code du jeu) ; même essai relancé aussitôt, sans problème. La fenêtre de plantage est apparue chez
  l'utilisateur : mesures lancées une par une ensuite.
- `quick_check` **PASS en 75 s**, tests 36/36. S2 et S7 courts en A/B (trois ProjectShiva ouverts : absolus non
  comparables, tous au-dessus de 3 ms) : le premier lancé de chaque paire est toujours le plus lent ; en moyenne sur les deux
  ordres, `HOLFullSpeedProcessor` 0,373 ms avec contre 0,368 sans ; S7 1 174 contre 1 158 bois/s ; S2 393 contre 387
  paires l'une sur l'autre à la fin. **S2 et S7 au repos à refaire** (S7 n'avait que 0,09 ms de marge).
- **Non commité**. Reprise : nouvel essai de l'utilisateur, puis commit de J5b.2 + D308, puis J5b.3.

### 2026-10-15 (suite) — 2e essai de J5b.2 ; D309 (récolteurs fantômes, source sans place) ; fin de séance

- **Mesures de D308 terminées** : `quick_check` PASS (75 s) ; S2 et S7 courts en A/B (trois ProjectShiva ouverts) :
  aucun coût mesurable, recouvrements inchangés ; détail dans l'entrée précédente.
- **2e essai de l'utilisateur** : moins de gros détours, mais les Paysans se gênent devant le centre. Demandes : (1) deux
  Paysans peuvent se traverser sur le chemin automatique ressource ↔ centre, pas sur un ordre du joueur, sans
  s'empiler à la ressource ; (2) ordre ou ralliement sur une ressource (n'importe laquelle) sans place libre autour :
  la même ressource tout près, sinon inactif devant. Deux questions posées → **D309** (« toutes les unités » traversées,
  8 m).
- **Fait** : `IsGhost` (activité `GatherTrip`, `hol.Units.GhostTrips`) dans `UHOLFullSpeedProcessor` (un fantôme ne
  regarde pas en avant et n'est pas un corps pour les autres) et `UHOLUnitSeparationProcessor` (ni ne pousse ni n'est
  poussé) ; `HOLGather::NoRoomSourceDistance` (800 cm) pour une source entourée et après 3 places manquées ;
  `UHOLGatherProcessor::StandBeforeSource` (fin de la récolte, ordre de marche ordinaire devant la source, 150 cm au-delà
  des deux corps, charge gardée) ; attente de 3 s retirée (`WaitSteps`) ; compteur `NoSpotStops` (S7 : « idle for want
  of a free spot »).
- Un essai de compilation refusé pendant l'essai de l'utilisateur (Live Coding actif tant que le jeu est ouvert) ; jeu
  fermé, **compile sans avertissement**, `quick_check` **PASS en 75 s**, tests 36/36. Pas encore d'essai scripté de D309.
- **Arrêt de la séance demandé par l'utilisateur.** Rien n'est commité (J5b.2, D307, D308, D309 attendent sa validation).
- **Reprise (nouvelle séance)** : lancer la session d'essai (deuxième écran) pour le **3e essai** de l'utilisateur ; au
  besoin d'abord un essai scripté de D309 (`Saved_Tests\detours2.seq` avec `hol.Gather.LogWalks 1` : marches longues,
  « no room » dans le journal) ; puis commit (chemins explicites, sans les lignes du plan de J6 tenues par l'autre
  session si elles ne sont pas commitées), S2 / S7 courts au repos, **J5b.3**.

### 2026-10-15 (nouvelle séance) — Règle des sous-agents ; essai scripté de D309

- **Règle des sous-agents** écrite (demande de l'utilisateur, voir sous « Prochaine étape exacte ») : le chantier se
  mène en sous-agents autant que possible.
- Essai scripté de D309 confié à un sous-agent (hors écran : `detours2.seq` avec `hol.Gather.LogWalks 1`, scénario
  « source sans place ») avant le 3e essai de l'utilisateur. Rien de commité.
- **Résultat de l'essai scripté** (hors écran, `detours2.seq` + `d309_sampled.seq`, `d309_noroom.seq`,
  `d309_loadkept.seq` dans `Saved_Tests`) : marches anormalement longues **2 → 0**, mètres de trop 18 → 0, dépôts
  manqués 0 ; « source sans place » conforme (filon à 40 Paysans : 11 récoltent, 29 inactifs devant ; buisson : 20
  placés sur lui et ses voisins à 8 m, 20 inactifs ; charge de baies gardée devant un filon plein). **Défaut** : des
  récolteurs **s'empilent** à la source (deux Paysans à 1 cm sur l'arbre 0, paires à 19-53 cm aux baies) : la
  séparation n'écarte plus deux Paysans arrêtés, et la réservation des places semble perdue (point non projeté
  comparé au point projeté, à 1 cm près) ou ignore les places des sources voisines. Aussi : une source épuisée mène à
  la voisine la plus proche à 15 m **sans vérifier qu'elle a de la place**. À signaler à l'essai : avec 20 Paysans sur
  un arbre, 4 ou 5 restent inactifs (les arbres à 8 m n'ont que 16 places), conforme à D309 mais visible ; des
  fantômes partis ensemble marchent superposés.
- **Correctif confié à un sous-agent** : places réservées sur le point réel, jamais deux places à moins de deux corps
  (même source ou voisines), source épuisée → voisine avec une place libre à 15 m. Puis 3e essai de l'utilisateur.
- **Correctif de l'empilement fait** (sous-agent, non commité) : trois causes confirmées (point réservé non projeté
  effacé à 1 cm près ; réservations rangées par source, les cercles de places de deux arbres à 2,2 m se recouvrant ;
  `OnSourceGone` sans contrôle de place). `HOLGather::FHOLSpotClaims` (grille de 200 cm, toutes sources, 3 × 3 cellules
  par recherche), `FindSourceSpot` réserve le point projeté, écart de 140 cm vérifié avant et après la projection,
  `IsSpotTaken`, source épuisée → plus proche à 15 m avec une place libre (sinon inactif devant) ; test
  `Gather.Spots` (tests 37/37). Distance minimale entre récolteurs arrêtés : bois **1 → 128 cm**, baies 24 → 142 cm ;
  marches longues et dépôts manqués 0. **Contrepartie** : bois à 20 Paysans 16 / 4-5 inactifs → **11 / 9** ; baies 15 / 5
  (géométrie réelle à 140 cm). Réglages possibles si trop : 120 cm entre sources différentes, ou rayon de 8 m élargi
  (à décider avec l'utilisateur). Compile sans avertissement, `quick_check` **PASS en 76 s**. S7 non mesuré (à faire
  avec S2 / S7 courts). Scénario `Saved_Tests\d309_gone.seq` (source épuisée). Les lancements doivent passer des
  chemins **absolus** à `-abslog` et `hol.Test.Run` (relatifs au dossier du moteur sinon).
- Suite : **3e essai de l'utilisateur** (session sur le deuxième écran).
- **3e essai de l'utilisateur** : « les slots se stackent (sûrement le multi ressources ?) » ; capture : amas de 3 à
  4 Paysans entre deux arbres d'une lisière, les autres bien espacés. Piste principale : les **inactifs devant une
  source pleine** (D309) reçoivent le même point et deux unités arrêtées ne s'écartent jamais ; autres pistes : fantôme
  arrêté sans place, changement de ressource. Enquête et correctif confiés à un sous-agent (compilation après la
  fermeture du jeu de l'utilisateur).
- **Correctif de l'amas fait** (sous-agent, non commité) : cause confirmée par `Saved_Tests\d309_mixed.seq` (36 Paysans
  sur un arbre, groupes envoyés aux baies, à un filon puis renvoyés au bois, ordres sur des arbres voisins) : les 54
  chevauchements étaient **tous un récolteur et un inactif** : le rang des inactifs devant la source débordait sur
  l'anneau des places de récolte (de la source ou d'un arbre voisin), un fantôme arrivait sur sa place occupée et deux
  unités arrêtées ne s'écartent jamais. Fait : `HOLGather::GetStandPoints` (rangées devant la source, 140 cm au-delà de
  l'anneau, pas de 141 cm) et `IsClearOfSpots` ; `UHOLGatherProcessor::FindStandPoint` (point d'attente réservé dans
  `FHOLSpotClaims`, tenu tant que l'inactif y va ou s'y tient) ; `FHOLGatherFragment::bSettling` : un Paysan qui
  arrive à sa place regarde une fois autour de lui dans la séparation (filet pour un inactif posé là par le joueur, peut
  le pousser d'~40 cm). `Gather.Spots` étendu. Distance minimale entre Paysans arrêtés **19 → 117-120 cm** ; quick_check
  **PASS en 70 s**, tests 37/37. Reste : beaucoup d'inactifs (36 Paysans sur un arbre : 11 récoltent), S7 non mesuré.
- Session d'essai relancée pour la suite du 3e essai.
- **Suite du 3e essai : D309 précisée** (pas de nouveau numéro : D315 à D317 sont prises par la review du GDD, dans
  une autre session). L'utilisateur : « quand tu cliques, tu vas sur la ressource ; si tu n'as pas de place, tu regardes
  autour ; s'ils n'ont pas de place, tu regardes à côté des voisins […] si 175 Paysans vont sur une forêt, ils vont
  entièrement entourer la forêt ». Règle : les sources de même ressource reliées de proche en proche (8 m) forment un
  **massif** ; sans place, recherche dans tout le massif en s'étendant depuis la source visée ; inactif devant seulement
  si tout le massif est plein ; un filon isolé ne mène jamais à un autre filon. Même recherche pour une source épuisée
  (au lieu des 15 m de D296). Confiée à un sous-agent (compilation après la fermeture du jeu de l'utilisateur).
- **Massifs faits** (sous-agent, non commité) : `HOLGather::FHOLSourceMassifs` (liens à 8 m sur toutes les sources,
  épuisées comprises : une souche ne coupe pas le massif ; calculés une fois par une grille de 8 m ; recherche par
  proximité de la source visée, première source avec une place), `FindMassifSpot`, sources trouvées pleines notées pour
  le pas ; ordre, ralliement, 3 places manquées et source épuisée passent par le massif (**les 15 m de D296
  disparaissent**, `FindNeighbourSource` retiré). Test `Gather.Massif` (tests 38/38). 36 Paysans sur un arbre :
  **11 / 25 → 33 / 3** ; 175 sur l'arbre d'angle : ~30 récoltent (capacité du bois au pas de 140 cm), bois ceinturé
  jusqu'à 17,9 m ; 175 sur un filon : 0 vers un autre filon ; source épuisée à 20 Paysans : 0 inactif en 180 s.
  Dépôts manqués 0 ; quelques marches longues = contournement du bois par ceux de l'autre côté. `quick_check`
  **PASS en 98 s**. S7 court 2,72 ms PASS sur machine chargée (récolte 0,221 → 0,310 ms) : **à refaire au repos**.
  Restent : (1) à 175, plus de points d'attente libres → les inactifs en trop reçoivent un même point devant la source
  et s'écrasent dans le bois (paires à 108-111 cm) : à décider avec l'utilisateur ; (2) le côté opposé du bois passe
  avant le bout du côté proche (ordre par distance à l'arbre cliqué).
- Session d'essai relancée.
- **Suite de l'essai** (capture d'AoE4, 37 villageois au bois) : « les unités peuvent être plus stackées ». Voulu :
  Paysans serrés contre la **lisière tournée vers le dépôt**, 2 à 4 par arbre, corps qui se chevauchent à moitié,
  aucun de l'autre côté du bois. Confié à un sous-agent : écart des places ~70 cm (`hol.Gather.SpotSpacing`), anneau
  plus dense, places et sources du massif triées vers le dépôt (répond aussi au point 2). Question des inactifs en trop
  à 175 (A rangées sans limite / B autour du massif / C tel quel) laissée en suspens.
- **Paysans serrés, lisière proche d'abord** (sous-agent, non commité) : écart des places **70 cm** (`hol.Gather.SpotSpacing`,
  140 = ancien), un rang régulier (`HOLGather::GetSourceSpots` : 11 places autour d'un arbre au lieu de 5, 16 autour d'un
  buisson, 33 autour d'un filon), places triées depuis la face tournée vers le dépôt (`FindSourceDeposit`), massif trié
  par distance à l'emprise du dépôt + 0,5 × distance à l'arbre cliqué (`MassifStartWeight`) ; séparation : un récolteur
  à sa source et une unité arrêtée ne sont écartés que jusqu'à 65 cm (un marcheur pousse toujours à 120 cm) ; points
  d'attente des inactifs inchangés (140 cm). Pas de second rang (hors contact, D296). Tests `Gather.Spots` et
  `Gather.Massif` mis à jour, 38/38. **37 Paysans sur un arbre de la lisière proche : 37 récoltent, 0 inactif, 0 de
  l'autre côté** (21 sur la lisière proche, 2 à 4 par arbre, 16 sur les petits côtés ; capture `near37.png`) ; filon
  à 40 : 11 / 29 → 28 / 12 ; 175 sur le bois : 52 récoltent. Distance minimale entre arrêtés 65-76 cm (61-64 cm
  récolteur / inactif sans point à lui, à 175). Marches longues 0 hors du cas à 175. `quick_check` **PASS en 97 s** ;
  **S7 court 2,394 ms PASS** (CPU ~40 %), récolte 0,214 ms. À lire : S7 compte ~1 390 paires « l'une sur l'autre »
  (voulu) ; S7 à 632 bois/s (fenêtre de 10 s entre deux vagues ? à vérifier sur un S7 plus long).
- Session d'essai relancée.
- **Essai validé** par l'utilisateur (2026-10-16) : « très bien » avec `hol.Gather.SpotSpacing 100` → valeur par
  défaut passée à 100 cm (sous-agent : compilation, `quick_check`). D309 précisée dans « Décisions » (massifs, places
  serrées vers le dépôt, points d'attente des inactifs ; remplace les 15 m de D296). **J5b.2 commitée** (`039b6ca`,
  `quick_check` PASS en 70 s à 100 cm, tests 38/38 ; sans les lignes du plan de J6, tenues par l'autre session).

### 2026-10-16 — Préparation de J5b.3 (vérification de J5b)

Ce que J5b.3 doit montrer (D299, plan de J5), mené en sous-agents (un seul à la fois compile ou lance le jeu) :
1. **`quick_check` solo** : ajouter à la partie « gather » : ralliement sur des baies (le Paysan né y récolte, stock de
   nourriture qui ne monte qu'au dépôt) ; un massif vidé (petit bois ou buissons épuisés par triche) → Paysans
   inactifs devant, compteur « Inactifs : N », touche « . » qui les parcourt ; un ordre sur un arbre entouré → arbre
   voisin du massif (D309 précisée) ; aucune paire de Paysans arrêtés sous ~90 cm.
2. **`quick_check` 2 clients** : comptes par ressource « (N) » et inactifs justes chez chaque client pour lui seul ;
   source sélectionnée chez l'un, « Reste » qui baisse chez les deux ; activité `GatherTrip` vue de l'autre.
3. **Mesures au repos** (ProjectShiva fermés si possible, sinon A/B) : S2 et S7 courts (D308, D309 changent suivi des
   chemins, corps et places) — **S7 sur une fenêtre plus longue** pour comprendre la chute à ~632 bois/s (contre
   1 158-1 403) ; S5 court sur serveur recuit (activité répliquée qui a changé de sens, marge 0,9 Ko/s).
4. **Essai de l'utilisateur** (4 ressources, HUD, inactifs), puis clôture de J5b (D306-D309 au GDD avec J5c.4 ou
   tout de suite, au choix).
En suspens : question A / B / C des inactifs en trop à 175 (voir « Prochaine étape exacte », point 13).
Ensuite **J5c** (animations, charge visible, « +10 ») : le repérage montre qu'aucune animation de récolte n'existe
dans le projet ; il faudra une décision de l'utilisateur sur la source (Mixamo, Fab, Control Rig, pis-aller des coups
d'épée).

### 2026-10-16 — Inactifs en trop à 175 (D309 précisée, point 6)

- Question A / B / C posée (pas de réponse aux deux fois précédentes, d'après la session `mycrusader-4c`). L'utilisateur
  propose « on accorde qu'ils cherchent un bosquet pas loin » ; recommandation : les deux, dans l'ordre. **Choix** :
  massif plein → **bosquet voisin** de la même ressource (~20 m au plus du massif, `hol.Gather.NearbyMassifRange`),
  sinon **rangées d'attente sans limite** devant la source visée. Sur `Plain` (un bois par emplacement) les rangées
  serviront souvent. Confié à un sous-agent.
- **Point 6 fait** (sous-agent, non commité) : `FHOLSourceMassifs::Build` numérote les massifs et relie une fois les
  **bosquets voisins** (même ressource, une source à `hol.Gather.NearbyMassifRange` au plus, 2 000 cm par défaut ; grille
  de cette taille, chaque paire de massifs gardée une fois) ; `Search` essaie, le massif entier refusé, les sources de
  tous ses bosquets voisins ensemble, dans le même ordre (emprise du dépôt + 0,5 × distance à la source visée : le
  meilleur bosquet d'abord) ; vaut pour l'ordre, le ralliement, les 3 places manquées et la source épuisée (devant
  laquelle on attend : toujours une source du massif d'origine ; massif épuisé et bosquets pleins → décharge puis
  inactif, comme avant). Rangées d'attente **sans limite** : `FindStandPoint` par paquets de 6 rangées tant qu'aucun
  point n'est libre (borne de sécurité 64 rangées, ~90 m). Compteur `NearbyGroves` (ligne « since the start » de
  `hol.Gather.Report`). Test `Gather.NearbyMassif` (bosquet à 15 et 19 m pris, 26 m non, baies non, deux filons non,
  ordre vers le dépôt, 300 inactifs sur 300 points distincts à 140 cm au moins) : tests **39/39** (`quick_check` en
  compte 38 : la ligne de fin du dernier test, `Settings.Volumes`, coupée à la sortie ; vert lancé seul).
  `quick_check` **PASS en 96 s**.
- **175 Paysans sur l'arbre d'angle** (`Saved_Tests\d309p6_175.seq`, hors écran) : 48 récolteurs (35 récoltent, 13 en
  tournée), 127 inactifs dont 3 coincés depuis leur naissance dans les rochers de la crête (déjà là avant) ; **0 vers
  un bosquet** (sur `Plain`, deux bois sont à 102 m) ; **127 points d'attente distincts**, aucun « sans point à lui »,
  rangées 0 à 9 (15,5 m au plus de l'arbre) ; distance minimale entre inactifs arrêtés **119 cm** (points à 140 cm,
  tolérance d'arrivée de 50 cm), entre arrêtés 99-103 cm (récolteurs à 100 cm) : fini l'amas dans le bois (paires à
  108-111 cm, 61-64 cm). Filon : 0 vers un autre filon. Marches longues 18 (au plus 34,7 m pour 20,9 m droits : le tour
  du bois par l'arrière, comme avant). Bosquet montré avec `hol.Gather.NearbyMassifRange 10300`
  (`d309p6_grove.seq`) : **97 envoyés** aux bois des emplacements 7 et 4, 30 inactifs (capacité des trois bois ~145).
- **S7 court** (20 s, hors écran) : **2,513 ms PASS** sur machine chargée (processeur 41-88 %), récolte 0,264 ms
  (0,214 avant) ; 0 inactif, 0 bosquet dans S7. Reste : essai de l'utilisateur, puis commit.
- Session d'essai lancée (deuxième écran, `Saved_Tests\j5b2.json`) ; essai de l'utilisateur attendu, puis commit.
- **Essai validé** par l'utilisateur (« ok, c'est bon ») ; commité (sans les lignes du plan de J6). Reprise : **J5b.3**.

### 2026-10-16 — J5b.3 : vérification (quick_check)

- Sous-agent, rien de commité. **Partie `gather`** (nouvelle, solo, dans la partie à 2 de `camera`, ~10 s de jeu) : 12
  Paysans (`hol.Units.Spawn`) encadrés, clic droit sur un arbre de la lisière (le clic tombe sur la source 5, couronne
  devant la source 3) → 3 le récoltent, 9 les arbres voisins (5,6 m au plus), 0 inactif ; 4 Paysans sur les baies ;
  ralliement du centre sur un buisson, un Paysan produit qui récolte le massif de baies (il porte 1 nourriture) ; stock
  de nourriture inchangé tant qu'ils portent (1900, 17 portés), puis les 6 buissons vidés (`hol.Sources.TakeNear`) :
  5 déchargements, stock 1900 → 1917 au dépôt seul, les 5 Paysans des baies inactifs ; HUD « gatherers F 5 W 12 …,
  idle 5 » puis « F 0 W 12 …, idle 10 » ; touche « . » trois fois puis clic sur le compteur : 4 inactifs différents
  (#1 à #4), caméra au-dessus de chacun, lui seul sélectionné ; aucune paire de Paysans arrêtés sous 90 cm (101 cm
  entre récolteurs). Captures `gather_berries`, `gather_wood`, `gather_idle` vérifiées.
- **2 clients** (même durée de 32 s) : HUD avant l'ordre « F 0 W 0 S 0 G 0, idle 1 », ensuite « F 0 W 1 S 0 G 0, idle
  0 » chez chacun (le Paysan de l'autre jamais compté) ; de T+4 à T+16 s les deux clients sélectionnent l'arbre le plus
  proche du départ du slot 0 (source 3) : « Reste » 5, 5, 4, 2, 1 chez les deux, puis retour à leur Paysan (charge
  exacte de D303 toujours vue) ; activité `GatherTrip` du Paysan de l'autre vue (7 relevés chacun).
- Nouveau verbe d'étape **`waitidle <N>`** (`HOLTestSequence.cpp` : N inactifs au moins, comptés comme le HUD).
- **Bug trouvé et corrigé** (à valider à l'essai) : après un massif épuisé, les Paysans qui déchargent s'arrêtaient
  inactifs l'un dans l'autre au dépôt (29 cm, 56 cm) : la fin de leur marche les rendait déjà inactifs, donc
  `UHOLGatherProcessor::Stop` ne posait rien, et des fantômes arrêtés ne sont jamais séparés. `Stop` pose désormais
  toujours `bSettling` (un regard de marcheur au pas d'arrêt) : 115-120 cm entre inactifs au dépôt.
- Tests : `-TestExit` (sortie forcée) coupait la dernière ligne du journal (38/39) ; remplacé par
  `Automation RunTests HeroesOfLiberty;SoftQuit` et un contrôle « chaque test commencé est fini » : **39/39**.
- `quick_check` **PASS en 78 s puis 77 s** (machine au repos ; 96 s au dernier passage, machine chargée).
- Reste pour J5b.3 : point 3 (S2 et S7 courts au repos, S7 plus long ; S5 court sur serveur recuit — `Stop` touche la
  simulation, S2 / S7 à refaire), puis essai de l'utilisateur (4 ressources, HUD, inactifs ; inactifs au dépôt séparés).
- Session d'essai lancée (deuxième écran) pour l'essai de J5b.3 (4 ressources, HUD, inactifs, correctif `Stop`).
- **Essai validé** par l'utilisateur (« c'est bon ») ; points 1 et 2 de J5b.3 commités (sans les lignes du plan de J6). Reprise : point 3 (S2 / S7 au repos, S7 long, S5 sur serveur recuit).

### 2026-10-16 — J5b.3 : mesures au repos

- Sous-agent, rien de commité, aucun code touché. Machine au repos : ~10-15 % du processeur avant les mesures, aucun
  Unreal, ProjectShiva ni compilation ; ouverts : VS Code (inactif, son installateur de mise à jour en attente),
  Discord (~3 %), Opera, Battle.net ; autres processus pendant les mesures 7-9 % en moyenne (au plus 16 %). Éditeur
  compilé à jour (rien à refaire), hors écran (`-RenderOffScreen -ResX=888 -ResY=500`, journaux
  `Saved_Tests\Logs\rest3_s1.log`, `rest3_s2.log`, `rest3_s7.log`, `rest3_s7long.log`) :
  - **S1** (10 s) : **2,184 ms PASS** (2,233 en c234f84) ; évitement à l'arrêt 8,1 ms un pas sur 6 ;
  - **S2** (10 s) : **2,738 ms PASS** (2,779) ; 583 puis 393 paires l'une sur l'autre (547 / 387) ;
  - **S7** (20 s) : **2,155 ms PASS** (2,906 ; marge 0,85 ms au lieu de 0,09) ; évitement à l'arrêt 5,6 ms un pas
    sur 6, récolte 0,204, ordres 0,157 (0,27), séparation 0,117 (0,42) ; **1 198 bois/s** (1 170), 13,1 chemins/s,
    220,8 chemins gardés repris/s ; 0 raté, 0 inactif ; 1 341 puis 1 503 paires l'une sur l'autre (voulu, D309).
- **S7 long** (180 s de mesure, `hol.Gather.LogDeposits 1`, `hol.Gather.Report` toutes les 10 s par
  `Saved_Tests\rest3_s7long.seq`) : les 632 bois/s étaient un **effet de fenêtre**, pas une baisse. Les 2 000 Paysans
  partent ensemble, donc les déchargements viennent par **vagues** (~15 s, un aller-retour) : de 3 620 bois/s à 0 sur
  1 s (aucun déchargement de 30 à 32 s après l'ordre). La fenêtre de 10 s qui suit la chauffe de 25 s (25-35 s) donne
  **647 bois/s** : c'est celle d'un S7 de 10 s (632). Sur 20 s (25-45 s) : 1 214 ; de 25 à 150 s : **1 201 bois/s**
  en moyenne, fenêtres de 10 s entre 454 et 1 812, vagues qui s'amortissent (792-1 499 de 85 à 140 s). Aucun inactif,
  aucune place ni dépôt manqué, 0 « sans place libre » jusqu'à 150 s. Ensuite le champ s'épuise (480 arbres de 400
  bois = 192 000 ; premier arbre épuisé à ~85 s, tous à ~180 s) : 399 bois/s de 150 à 205 s, 342 envoyés vers un
  bosquet, 1 937 arrêtés inactifs faute de place ; à la fin 1 998 inactifs dont **1 198 gardent leur charge** (ils
  ne vont pas décharger le reste quand plus rien n'est à récolter : à décider si on le veut). Moyenne de la mesure
  958 bois/s (épuisement compris) ; 2,325 ms par pas (journal de chaque déchargement compris : non comparable).
  11 Paysans nés hors du navmesh dans ce passage (avertissement, 0 dans les passages courts).
- **S5 court au repos** (`net_match.ps1 -Stage -Units 2000 -Seconds 40`, serveur recuit sur a067c11, 195 s en tout,
  processeur 8-10 % avant) : **PASS à 31 227 o/s par client** (31 082 en 535b7c8 : +145 o/s, `GatherTrip` de D307 ;
  seuil 32 000 : **marge de 0,77 Ko/s**, 0,92 avant), 1,78 envoi par unité et par seconde, 42 paquets/s, écart affiché
  **21,6 cm** (22,7 ; seuil 25), réplication 0,148 ms par pas, simulation du serveur 6,95 ms (2 clients sur la même
  machine). Client : états 25,5 Ko/s, le reste 5,9 Ko/s.
- Tout PASS : point 3 de J5b.3 fait. Reste : clôture de J5b (D306-D309 au GDD, au choix de l'utilisateur), puis J5c.
- **Charge gardée par un inactif sans place** : **confirmée** par l'utilisateur après ce S7 long (comme dans AoE4, il
  ne va pas la déposer). Rien à changer.

### 2026-10-16 — Clôture de J5b

- Sous-agent, documentation seule, rien de commité.
- GDD § 21 : lignes **D307** (inactifs et comptes de récolteurs), **D308** (détours des allers-retours) et **D309**
  (récolteurs fantômes, massifs, places à 100 cm, inactifs en rangées, point 6 : bosquet voisin à ~20 m et rangées
  sans limite ; charge gardée par un inactif sans place, confirmée), après D306 et avant D315. **D296 précisée par
  D309** (les ~15 m remplacés par les massifs et le bosquet voisin).
- GDD § 16.6 : paragraphe **« Parcours J5b : les 4 ressources et le HUD »** après « Parcours J5a » (baies et filons,
  376 sources, D306, HUD, D308, D309, 175 Paysans sur un arbre ; `quick_check` « gather » solo et 2 clients ; mesures
  au repos du 2026-10-16 ; effet de fenêtre de S7) ; « Parcours J5a » précisé (les 15 m remplacés par les massifs,
  D309). § 22 (review) non touché.
- **J5b close.** Reprise : **J5c.1** (source des animations déjà tranchée, voir « Prochaine étape exacte », point 13).

### 2026-10-16 — J5c.3 : « +10 » au dépôt

- Sous-agent, rien de commité (essai de l'utilisateur attendu). Fichiers : `HOLGatherProcessor.{h,cpp}`,
  `HOLPlayerController.{h,cpp}`, `HOLGameHUD.{h,cpp}` (module UI), `quick_check.ps1`. Rien dans les fichiers réservés
  à `mycrusader-23` (`HOLInterpolatedInstancesProcessor`, table des unités, `HOLUnitReplicationRules` et sérialiseur).
- **Comportement** : à chaque déchargement d'un Paysan du joueur, un « +N » (quantité réellement déchargée) avec le
  carré de couleur de la ressource de la barre du haut, texte clair cerné de noir (police F_HOL, 20 pt), 2 m au-dessus
  du point de déchargement ; il monte de 1,2 m en 1,2 s, opaque la première moitié puis s'efface. Deux « +N » à moins de
  2,5 m nés à moins de 0,5 s : le second une ligne plus haut (4 lignes au plus). 30 au plus à l'écran (les plus vieux
  partent) ; dessinés par le HUD au canvas (sous les panneaux), seulement ceux qui vivent : aucun coût par unité.
- **Canal réseau** : appel client **non fiable** `AHOLPlayerController::ClientShowDepositPopups`, un par pas et par
  joueur ayant déchargé, avec les déchargements du pas (`FHOLDepositPopup` : position `FVector_NetQuantize`, ressource,
  quantité ; 8 au plus par pas et par joueur). Un appel client ne va qu'à la connexion propriétaire : l'autre joueur ne
  reçoit rien ; en solo l'appel s'exécute sur place ; une IA n'a pas de contrôleur, rien n'est envoyé. **État des
  unités inchangé** (aucun bit). Déduire le « +10 » chez le client écarté : il ne connaît la charge exacte que de
  l'unité suivie (D303) et le stock (D282) arrive regroupé, donc ni quantité par Paysan ni lieu exacts.
- **Coût** : ~9 octets par déchargement (position ~63 bits, ressource 2, quantité 8) + en-tête d'appel ; un Paysan
  décharge ~0,06 fois par seconde (S7 : 1 201 bois/s pour 2 000) : 175 Paysans → ~10 déchargements/s → **~0,1 à
  0,2 Ko/s**, au seul propriétaire, seulement en récolte. **S5 non refait** : ses unités ne récoltent pas (aucun
  appel) et l'état répliqué n'a pas changé.
- Commande de développement **`hol.Gather.LogPopups 1`** (absente en Shipping) : « Deposit popup: +4 Food at (-15214,
  830) » à chaque « +N » montré sur la machine.
- **`quick_check`** : partie `gather` solo, caméra sur le côté baies du centre avant d'épuiser les buissons, attente du
  premier « +N », capture **`gather_popup`** (regardée : « +4 » et « +1 » avec le carré rouge de la nourriture,
  lisibles au zoom par défaut) ; contrôles : un « +N » par déchargement du slot 0 depuis la marque, même quantité et
  ressource (5 « +N » nourriture pour 5 déchargements), tous à moins de 12 m du départ du slot 0. Partie à 2 clients
  (`hol.Gather.LogPopups 1` aux clients) : chaque client a le « +10 Wood » de son Paysan près de son centre, aucun près
  du centre de l'autre, pas plus de « +N » que de déchargements de son joueur au serveur, aucun au serveur.
- `quick_check` **PASS en 78 s** (103 s au premier passage, premier lancement après la compilation) ; tests **39/39**.
- Reste : **essai de l'utilisateur** (récolte, « +10 » au dépôt, couleur du bois sur le sol), puis commit ; ensuite J6a.
- Session d'essai lancée (deuxième écran) pour l'essai de J5c.3.
- **Essai validé** par l'utilisateur (« très bien ») ; commité (sans les lignes des autres sessions). Reprise : **J6a.1** (navmesh des bâtiments).

### 2026-10-16 — J6a.1 : navmesh des bâtiments

- Sous-agent, rien de commité (essai de l'utilisateur attendu), GDD non touché, aucun fichier réservé à `mycrusader-23`.
- **Mesure d'abord** (nouvelle commande `hol.Nav.Footprints on|off|report|navmesh`, développement : à chaque pas, écart
  entre le corps de chaque unité et l'emprise la plus proche, entrées journalisées ; `navmesh` = rayons du navmesh vers
  les côtés et les coins). Scénario solo (12 Paysans au bois, 6 aux baies, ralliement + 3 produits, 10 envoyés autour du
  centre et retour, tous sur le point du centre lui-même, puis autour) :
  - **la piste de D310 était fausse** : une zone `UNavArea_Null` **est** élargie du rayon de l'agent (moteur,
    `MarkDynamicArea` : `ExpandBy = AgentRadius`) ; navmesh à **63-77 cm** des côtés, **82-95 cm** des coins : un corps
    de 60 cm **frôle** le mur (3 cm) ;
  - allers-retours de récolte : **jamais dans l'emprise** (20 cm au plus près, la place au dépôt) ;
  - **suivi de chemin** (D308) : le marcheur visait le point du suivi de Mass, jusqu'à ~5 m devant lui, **au-delà du
    coin** : la ligne droite coupait le coin du centre, **jusqu'à 1,9 m dedans** (séparation coupée : -173 / -193 cm) ;
  - **séparation** (D292) d'une foule contre le mur : ordre sur le centre lui-même, **19 Paysans debout dans le
    centre, jusqu'à 4,6 m** (hors navmesh) ; demi-tours du regard en avant (D293) et dernières lignes droites aussi.
- **Correctif, même règle pour tous les bâtiments** (réglage unique `BodyClearance=25` cm, `[HOLBuildingSettings]` de
  `DefaultGame.ini`) :
  1. **zone interdite = emprise + 25 cm** (`FitNavObstacle`), le navmesh y ajoute l'agent (60 cm) : navmesh à
     **86-96 cm** des côtés, 122 cm des coins (bâtiments tournés de S7 : 71-94, cellules du navmesh) ;
  2. **visée le long du chemin** (`GetAimOnPath`, `HOLFullSpeedProcessor`) : jamais au-delà du **point suivant** du
     chemin court (le suivant une fois à un pas de marche) ; seul (sans séparation ni regard en avant) : -173 → -21 cm ;
  3. **`UHOLFootprintKeepOutProcessor`** (après la séparation, `hol.Units.KeepOut 0|1|2`) : un corps entré dans une
     emprise est remis **contre le mur** (à 1 cm), par le côté le plus proche, son propre rayon ; pas à 25 cm : le
     navmesh des bâtiments tournés laisse les chemins à 11 cm du mur, il les repousserait à chaque pas ;
  4. **nouveau chemin** pour un marcheur remis contre le mur **10 pas de suite** (`BuildingPushes`, ordres) : son
     chemin traverse un bâtiment que le navmesh ne montrait pas encore (S7 : Paysans nés **dans** les centres, premiers
     chemins avant les tuiles refaites ; 63 restaient **bloqués contre le mur** sans cela, avant ils traversaient le
     centre) ; départ cherché à 10 m si rien à 5 m ;
  5. **place au dépôt** : rayon + 25 + 20 cm (sur le navmesh). Places des sources (D297) inchangées (cylindres déjà
     élargis de l'agent ; 29 cm au plus près d'un camp en S7).
- **Après** (même scénario) : **0 corps dans une emprise** ; récolte au plus près **45 cm** ; foule sur le centre : 1 155
  remises contre le mur, 69 nouveaux chemins, contre le mur à 1 cm (hors navmesh de 25 cm, normal).
- **`quick_check`** : partie **`footprints`** (solo : navmesh ≥ 85 cm des côtés du centre ; surveillance de la marque
  `gather` à la fin, plus un ordre de tous les Paysans **sur le centre** puis autour : 0 corps dans une emprise, écart ≥
  0) ; 2 clients : surveillance au serveur sur les allers-retours (36 cm au plus près, 0 dedans). Test
  `Buildings.Appearance` : boîtes des pièces seules, zone interdite = emprise + 25. Compte au rapport de S7
  (« Buildings (D310) »). **PASS en 87-91 s** (78 avant : +2 attentes de 5 s), tests **39/39**.
- **S1 / S2 / S7 courts** hors écran, **machine chargée** (25-44 % avant chaque mesure, Opera, Discord ; 10-15 % au
  repos) : **S1 2,365 ms** (2,184 au repos), **S2 2,789 ms** (2,738), **S7 2,567 ms** (2,155), tous **PASS** ;
  évitement à l'arrêt non touché 8,40 ms au lieu de 8,10 (charge). Coût propre : visée ~+0,03 ms en S2 (0,531 au lieu de 0,502, sous charge), garde des
  emprises 0,006 ms sans bâtiment, **0,11 ms en S7** (32 bâtiments, 2 000 unités) ; S7 : 1 214 bois/s, 0 raté, 0
  remise pendant la mesure. **À refaire au repos** pour des chiffres comparables.
- Cartes non refaites (bâtiments apparus en cours de partie : modificateurs dynamiques).
- Reste : **essai de l'utilisateur** (Paysans autour du forum, ordre sur le forum, allers-retours au dépôt, ralliement),
  puis commit ; mesures au repos.
- Session d'essai lancée (deuxième écran) pour l'essai de J6a.1.
- **Essai validé** par l'utilisateur (« c'est bon ») ; J6a.1 commitée (sans les lignes des autres sessions). Reste : S1 / S2 / S7 au repos (avant la clôture de J6a). Reprise : **J6a.2** (données et règles pures).

### 2026-10-16 — J6a.2 : données et règles de la construction

- **Table** (`Tools/Data/BuildingTypes.csv`, refaite par `make_building_types.py` en commandlet) : colonnes **`Cost`**
  (4 ressources), **`BuildTime`** (s, un bâtisseur), **`BuildGridSlot`** (case de la grille du Paysan, D313 : 0 = A,
  1 = Z, 2 = E ; −1 = non constructible, le centre). Valeurs indicatives façon AoE4 :

  | Ligne | Emprise | Hauteur | PV | Pop. | Accepte | Coût | Durée | Case |
  |---|---|---|---|---|---|---|---|---|
  | `MainCentre` | 15 × 15 m | 10 m | 2 400 | 10 | les 4 | — | — | −1 |
  | `House` | 5 × 5 m | 4 m | 600 | +10 | — | 50 bois | 20 s | A |
  | `LumberCamp` | 6 × 6 m | 4 m | 1 000 | 0 | bois | 50 bois | 25 s | Z |
  | `MiningCamp` | 6 × 6 m | 4 m | 1 000 | 0 | pierre, or | 50 bois | 25 s | E |

  `LumberCamp` devient la vraie ligne **sans changer emprise, hauteur, PV ni dépôt** : S7 (camps posés par le code)
  inchangé, non relancé.
- **Réglages** (`UHOLBuildingSettings`, `DefaultGame.ini`) : `PlacementGrid=100` cm, `AntiRushRadius=4000` cm (du milieu
  du centre ennemi au point le plus proche de l'emprise ; emplacements à 115 m à 8 joueurs, filons à ~60 m de deux
  emplacements : les deux voisins peuvent y bâtir), `BuilderExponent=0.63`.
- **`Buildings/HOLConstructionRules`** (fonctions pures, `HOLConstruction::`) :
  - `SnapToGrid` : **coin bas de l'emprise sur la grille** (bords sur les lignes ; milieu entre deux lignes si taille
    impaire) → deux bâtiments côte à côte se touchent exactement.
  - `CheckPlacement` (requête : emprise, emprises posées, cercles des sources, carte, centres ennemis, rayon anti-rush,
    réponse navmesh de l'appelant) → refus dans l'ordre `OutOfMap`, `Building` (se toucher permis, tolérance 0,5 cm),
    `Source` (disque du rayon de la source), `EnemyCentre`, `NavMesh`.
  - `TryPay` (tout ou rien) ; `GetRefund` : tout avant le premier travail, **moitié arrondie en dessous** après.
  - Travail en **entiers** : un bâtisseur = 1 000 par pas, total = `BuildSteps` × 1 000 ; N bâtisseurs =
    **1 000 × N^0,63** par pas (3 → ×2,0 ; 2 → ×1,55 ; 4 → ×2,4 ; 10 → ×4,3) ; `AddWork` vrai au seul pas qui finit.
  - `GetHealth` = PV max × travail / total, au moins 1 (l'appelant ajoute l'écart à chaque pas : les dégâts pris
    restent, un bâtiment intact finit à ses PV max) ; `GetPopulationProvided` : rien tant que non fini ; plafond 175
    par `HOLProduction::GetPopulationCap` (inchangé).
- **Tests** `HeroesOfLiberty.Construction.*` : `Grid`, `Placement`, `Payment`, `Builders`, `Population`, `Tables` (6) ;
  **45/45**. Rien de visible en jeu ; **quick_check PASS en 90 s**.
- **À régler à l'essai** : coûts (50 bois partout), durées (20 / 25 s), emprises (maison 5 m, camps 6 m), PV (600 /
  1 000), rayon anti-rush (40 m), exposant (0,63), demi-remboursement arrondi en dessous. Plus tard (J6a.4) : le
  plafond de `UHOLProductionSubsystem` devra ignorer les bâtiments non finis.
- **Tailles en cases** (choix de l'utilisateur, 2026-10-16, précise D311) : **une case = 3 m** ; forum (centre principal)
  **4 × 4 cases (12 m)** au lieu de 15 m ; maison et camps **2 × 2 cases (6 m)** ; la pose se fait **case par case**
  (grille de 3 m au lieu de ~1 m), bâtiments toujours alignés bord à bord. Confié à un sous-agent avant le commit de J6a.2.
- **Coût des camps** (choix de l'utilisateur, 2026-10-16) : **100 bois** (maison 50 bois).
- **Cases de 3 m faites dans le code** (sous-agent, non compilé ni essayé : fenêtre de compilation de `mycrusader-23`) :
  réglage **`TileSize=300`** (remplace `PlacementGrid`, source unique, `UHOLBuildingSettings::GetTileSize`) ; emprises
  en cm dans la table : centre **1 200 × 1 200**, maison et camps **600 × 600** ; camps à **100 bois**. **Forum sur la
  grille** sans refaire `Plain` : `AHOLStartSlot::GetMainCentreLocation` (point de l'emplacement porté sur la grille par
  `SnapToGrid`, ≤ 1,5 m par axe ; emplacements diagonaux : 1,07 m) et `GetMainCentreYaw` (quart de tour le plus proche,
  `HOLConstruction::SnapYaw`) ; servent à la pose du centre, à la caméra de départ et aux commandes `OrderNear` /
  `OrderSourceNear` / `FocusSlot`. `quick_check` : `Get-SlotStart` sur la grille, contrôle « centres sur la grille de
  3 m » (journal du centre avec son lacet). Tests `Construction.Grid` / `.Placement` / `.Tables` en cases (forum d'un
  emplacement diagonal sur la grille, maison collée à lui), `Buildings.Types` / `.Appearance` à 12 m. S7 inchangé
  (centres tournés posés par le code). **Reste** : compilation, table refaite, tests, `quick_check`, S7 court.
- Cases de 3 m (suite) : compilation OK, table refaite (commandlet), **tests 45/45** ; centres vus en jeu sur la grille
  (8 joueurs : (±15 000, 0), (0, ±15 000), (±10 500, ±10 500), lacets en quarts de tour). `quick_check` faussé (deux
  lancés en même temps dans ce dossier : clients en trop sur le serveur) ; contrôle « centres sur la grille » corrigé
  (groupes de l'expression aplatis par PowerShell). À relancer seul : `quick_check`, S7 court, captures.
- Cases de 3 m (vérification seule) : compilation à jour, table refaite, **`quick_check` PASS en 99 s** (toutes les
  parties, tests 45/45 ; essai de l'utilisateur ouvert depuis `HeroesOfLiberty-J5c` en même temps). Captures `cam_2p_start`
  et `gather_*` : forum de 12 m net, bosquet et baies un peu plus loin de ses bords (anneau de sources prévu pour 15 m),
  Paysans autour, aucun dans l'emprise, « +4 » au bord du centre. **S7 court non lancé** (essai en cours) : à faire.
- **S7 court après les cases de 3 m** (sous-agent, 20 s hors écran, `j6a2_s7.log`) : **2,991 ms PASS de justesse**
  (2,155 au repos, 2,567 en J6a.1), max 13,9 ms ; **machine chargée** (15-21 % avant, 64-89 % pendant : deux
  ProjectShiva montés de ~0 à 8-21 %, autres 11-21 %) ; tous les postes ~+25-30 % (récolte 0,267, ordres 0,201,
  séparation 0,142, évitement à l'arrêt 7,48 un pas sur 6), garde des emprises 0,127 ; 1 189 bois/s, 0 raté, 0
  inactif, « Buildings (D310) » 0 remis / 0 nouveau chemin ; 1 295 puis 1 376 paires. **À refaire au repos.**
- Session d'essai lancée (deuxième écran) : l'utilisateur veut voir le forum à 12 m avant le commit de J6a.2.
- **Essai validé** par l'utilisateur (« c'est bon », forum à 12 m) ; J6a.2 commitée (sans les lignes des autres sessions). S7 au repos à refaire à la clôture de J6a. Reprise : **J6a.3** (pose), après la fusion de J5c.1 par `mycrusader-23`.

### 2026-10-16 — J6a.3 : pose des bâtiments

- Sous-agent, rien de commité (essai de l'utilisateur attendu), GDD non touché, aucun fichier réservé à `mycrusader-23`.
- **État de construction** (`EHOLConstructionState` : `Foundation`, `UnderConstruction`, `Finished`) et **travail**
  (`int64`) répliqués à tous sur `AHOLBuilding` ; `Setup(..., État = Finished)` : centres et camps de S7 nés finis, une
  pose naît fondation. Règles pures ajoutées à `HOLConstruction` : `BlocksNavigation` (pas une fondation, D312),
  `IsServing` (fini seulement), `CanCancel`, `HasWorkStarted`, `GetCancelRefund`, `GetFootprintBounds` (emprise tournée),
  `GetNavProbes` (milieu de chaque case + coins rentrés de 1 m). Setter serveur `SetConstructionState` / `SetConstructionWork`
  prêt pour J6a.4 (aspect et navmesh suivent).
- **Fondation** : dalle de terre battue 12 cm, bordure à la couleur du joueur, 4 piquets ; **ne bloque pas le navmesh**
  (zone interdite comptée seulement si l'état la bloque : `NavObstacle` non pertinent au départ, activé par
  `FitNavObstacle`), ignorée par la garde des emprises (D310) et par `hol.Nav.Footprints` ; ni population (plafond de
  `UHOLProductionSubsystem` = bâtiments **finis** seulement), ni dépôt (`RefreshDeposits`), ni production / ralliement.
- **Serveur** : `UHOLConstructionSubsystem` (nouveau, début de pas comme la production) applique les ordres de la file
  (`FHOLConstructionCommand` : `Place`, `Cancel`) ; revérifie tout (D288) par `CheckPlacement` (type constructible,
  grille, carte, bâtiments **et fondations**, sources debout, centres des **autres** joueurs + rayon anti-rush, navmesh
  sous chaque case), `TryPay`, fondation apparue (propriétaire = contrôleur du joueur, couleur du joueur), journal
  `Construction: slot N placed / cannot place ...: <raison> / cancelled ..., refund ...`. Refus : rien de pris.
- **Client** : mode de pose (`BeginPlacement`), **fantôme** dans `AHOLSelectionMarkers` (cadre au sol, cases de 3 m,
  arêtes du volume) **vert / rouge** par la même vérification (le client n'a pas de navmesh : le serveur tranche celle-ci)
  et le stock ; clic gauche : pose et sort du mode ; **Maj + clic** : pose et reste ; **Échap / clic droit** : sortent du
  mode (sélection gardée) ; en mode pose, aucun clic ne sélectionne ni n'ordonne. RPC `ServerPlaceBuilding(type, point)`,
  `ServerCancelBuilding(bâtiment)`.
- **Sélection** : une fondation se sélectionne comme un bâtiment (cadre au sol) ; bloc du HUD « Fondation : Maison »,
  points de vie 1 / 600 (D312) ; textes fr / en / ja (Maison / House / 家, Camp de bûcherons / Lumber Camp / 伐採所,
  Camp de mineurs / Mining Camp / 採掘所, « Fondation : {0} » / « Foundation: {0} » / « {0}（基礎） »), GatherText relancé.
  Bâtiment annulé : il quitte la sélection.
- **Commandes** (développement) : `hol.Build.Place House|LumberCamp|MiningCamp` (mode pose, Paysans sélectionnés ou non) ;
  `hol.Build.Click [shift]` ; `hol.Build.PlaceNear <Type> <Dx> <Dy> [enemy]` (repère du centre, X vers le milieu de la
  carte ; envoyé même si refusé ici) ; `hol.Build.PlaceAtSource <Type> X Y` ; `hol.Build.CancelSelected` ;
  `hol.Build.Report` ; `hol.Select.ClickGround X Y`.
- **Tests** : `Construction.States`, `Construction.Probes`, `Buildings.Foundation` ; **50/50**.
- **`quick_check`** : partie **`build`** solo (120 bois : maison A par clic, B collée avec Maj, fantôme rouge dessus et
  clic sans effet, Échap ; refus serveur sur une maison, un arbre, près du centre ennemi, faute de bois, bois inchangé ;
  B choisie par un clic au sol, annulée, 50 rendus) ; 2 clients : chacun pose une maison derrière son centre, voit celle
  de l'autre, paie seul (bois −50 exact, stock de l'autre jamais reçu), maison près du centre ennemi refusée par le
  serveur. **Correctif du flottement** « others [] » : étapes `signal` / `waitfile` de `hol.Test.Run`, chaque client
  attend le dernier rapport de l'autre avant de quitter. **PASS en 141 s** (1er lancement après compilation, démarrage
  48 s) puis **120 s** ; **machine très chargée** (CPU 100 % : 3 ProjectShiva, Discord, Opera) ; la partie `build`
  coûte ~1,3 s, le net ~6 s.
- Captures vues : fantôme vert (cage verte derrière le forum), rouge sur les deux fondations, fondation choisie (cadre
  blanc, bloc « Fondation : Maison »), côté client la fondation derrière le forum de l'autre.
- Simulation à peine touchée (une condition par bâtiment dans la garde des emprises et les dépôts) : pas de stress test.
- À essayer : `hol.Cheat.Wood 500`, `hol.Build.Place House`, promener le fantôme (vert sur sol libre, rouge sur un arbre,
  le forum, une autre fondation, près du forum ennemi, sans bois), clic, Maj + clic en rangée, Échap / clic droit ;
  cliquer une fondation (bloc « Fondation : Maison »), `hol.Build.CancelSelected` (bois rendu). Réglage possible :
  épaisseur du cadre du fantôme (40 cm).
- Session d'essai lancée (deuxième écran) pour l'essai de J6a.3.
- **Essai validé** par l'utilisateur (« tout est bien ») avec une remarque : **une fondation n'est qu'un fantôme tant que le premier coup de marteau n'est pas donné** (pas de dalle avant le travail) → à faire en J6a.4 (le premier coup y arrive). J6a.3 commitée (sans les lignes des autres sessions). Reprise : **J6a.4** (chantier).

### 2026-10-16 — J6a.4 : chantier

- **Fondation fantôme** (choix de l'utilisateur, recommandation) : avant le premier coup de marteau, une fondation
  n'est qu'un **fantôme vu de son seul propriétaire** (AoE4 : l'adversaire ne découvre pas les plans ; les alliés la
  verront avec les équipes) ; au premier coup, elle devient un chantier visible de tous, bloque le navmesh (D312) et
  monte du sol. Confié à un sous-agent avec le reste de J6a.4.
- Sous-agent, rien de commité (essai de l'utilisateur attendu), GDD non touché. Activité répliquée : accord de
  `mycrusader-23` (relayé) pour **`EHOLUnitActivity::Building`**, valeur 4 en mémoire, **valeur 2 sur le fil**
  (`HOLUnitReplication::BuildingWireValue`, celle de `Gathering` jamais écrite : **la dernière libre** des 3 bits,
  commenté) ; 0 bit de plus ; présentation non touchée (geste du marteau : `mycrusader-23`, après J5c.2).
- **Fondation fantôme** (`AHOLBuilding`) : posée, elle n'est répliquée qu'à son propriétaire (`bOnlyRelevantToOwner`,
  filtre « propriétaire » d'Iris) ; dessinée en fantôme (cadre au sol + arêtes du volume, couleur du joueur
  éclaircie, pas de dalle) et une boîte invisible pour le clic ; ailleurs (ordinateur en solo, serveur) rien n'est
  dessiné (`IsSeenHere`). **Premier coup** : état `UnderConstruction`, filtre d'Iris levé (tous la reçoivent),
  navmesh bloqué (J6a.1 : unités sorties par la garde des emprises), dalle + bordure + piquets et le bâtiment qui
  **monte** (pièces sous `RiseRoot`, écrasées en hauteur à l'avancement, 4 % au moins) ; travail répliqué à chaque pas.
  Client : journal `Building arrived: <Type> of slot N, <état>`.
- **Bâtisseurs** (`Simulation/HOLBuildProcessor`, nouveau, après la récolte ; `FHOLBuildFragment` dans
  `HOLBuildFragments.h`, file des chantiers gardée par le processeur : un fragment ne tient pas de tableau) :
  - ordre `FHOLBuildOrder` par la file d'ordres ; le serveur revérifie (chantier du joueur non fini, ses seules
    unités, celles qui récoltent : `CarryCapacity > 0`, faute de colonne « bâtisseur » dans la table des unités) ;
    l'ordre arrête la récolte (charge gardée) ; un ordre de marche ou de récolte du joueur arrête la construction ;
  - pose en mode placement avec des Paysans choisis : ils partent la bâtir ; **Maj** : en fin de leur file, bâties
    dans l'ordre (`HOLConstruction::AddToBuildQueue` / `AdvanceBuildQueue`) ; **clic droit** sur un chantier ou une
    fondation à soi non finis : aider (RPC `ServerBuildUnits`, Maj = en file) ;
  - places autour de l'emprise (`HOLConstruction::GetBuildSpots` : à rayon + 25 + 20 cm des côtés, 130 cm entre deux,
    5 par côté d'une maison, côté le plus proche d'abord ; pas sur un autre bâtiment, sur le navmesh) ; marche par
    ordre « de tâche » (comme la récolte) ; au contact (corps à 100 cm de l'emprise au plus) : il travaille, corps
    solide (ni fantôme ni marcheur), activité `Building` ; poussé à plus de 150 cm : il y retourne ;
  - chaque pas : `AddWork` avec le nombre de bâtisseurs **au travail** (N^0,63) ; `hol.Build.Speed N` (travail × N,
    développement ; `hol.Cheat.FastBuild 1` le met à 10) ; journaux `first work`, `N builders at work` (changement),
    `finished ... after S steps of work (by builders: 1 xa, 2 xb, 3 xc)` ;
  - fini : état `Finished` (population, dépôt, forme et navmesh déjà en place) ; ses bâtisseurs : chantier suivant de
    leur file, sinon **inactifs** (comptés dans « Inactifs », touche « . ») ; enchaînement camp → récolte : J6b.1.
- **HUD** : « Construction » (unité seule), « N en construction » (groupe), fr / en / ja (`fill_po.py`, GatherText
  deux fois) ; population : le plafond suit les maisons finies (10 → 20 → 30 vu dans le HUD). `hol.Build.Report`
  donne l'avancement (`under construction 45%`) ; `hol.Select.Own` (sélectionne ses unités, scripts).
- **Tests** : `Construction.BuildSpots`, `Construction.BuildQueue` (nouveaux), `Buildings.Foundation` (fantôme du
  propriétaire, rien chez un autre, chantier qui monte à mi-hauteur à mi-travail), `Net.ElementFormat` (Building
  sur le fil, aller-retour, 8 valeurs sur 8) : **52/52**.
- **`quick_check` PASS en 119 s** (machine chargée, 73 % avant) ; parties nouvelles : **`build site`** solo (navmesh à
  travers la fondation A : droit 1,00, après le premier coup 1,27 ; A bâtie seule en **200 pas** à vitesse 2 ; plafond
  10 → 20 et HUD ; bâtisseur inactif ; C posée avec 3 Paysans puis D en Maj : C en **102 pas** (3 x98), soit 0,51 de A,
  puis D ; plafond 30 ; le 1er Paysan aide D par clic droit (4 bâtisseurs) ; D annulée : **25 bois rendus**, 4
  inactifs) ; **net** : chaque client fait bâtir sa maison par son Paysan ; la fondation de l'autre **jamais reçue**
  avant son premier coup (3 bâtiments), puis reçue « under construction » (4) ; serveur : un premier coup par slot.
  Avant la partie `build` de J6a.3 : rien de sélectionné (Échap), ses poses n'envoient personne.
- Captures vues : `build_foundations` (fantôme du propriétaire, cadre et arêtes), `build_site` (A montée ~40 %,
  « Construction »), `build_three` (3 Paysans au contact de C, D en fantôme, « 3 en construction »), `build_finished`
  (A et C finies) ; `qcnet_1_build1` (son fantôme derrière son forum), `qcnet_1_build0` (rien chez l'autre),
  `qcnet_1_site0` (chantier de l'autre reçu).
- **S2 / S7 courts** hors écran, **machine très chargée** (ProjectShiva ouvert, Discord, Opera ; 29-100 % avant,
  77-92 % pendant) : S2 **3,579 / 3,897 ms FAIL**, S7 **3,589 / 3,056 ms FAIL** ; tous les postes +15-40 % (copie des
  transformées 0,078 → 0,103 ms), **`HOLBuildProcessor` 0,004-0,005 ms** (sort tout de suite sans bâtisseur) ; S7 :
  1 195 bois/s, 0 raté, 0 inactif. **À refaire au repos** (S2 2,738, S7 2,155).
- À essayer : `hol.Cheat.Wood 500`, choisir des Paysans, `hol.Build.Place House`, clic (ou Maj + clic plusieurs
  fois) : ils bâtissent, la maison monte, population +10 ; clic droit d'autres Paysans sur le chantier ; un chantier
  choisi, `hol.Build.CancelSelected` (moitié rendue) ; `hol.Build.Speed 5` pour aller vite. À 2 clients : le fantôme
  de l'autre n'apparaît qu'au premier coup.
- Session d'essai lancée (deuxième écran) pour l'essai de J6a.4. S2 / S7 courts en échec sous forte charge (S2 3,58-3,90, S7 3,06-3,59 ms ; tous les processeurs 15-40 % plus lents, construction 0,005 ms) : **à refaire au repos**.
- **Retours de l'essai de J6a.4** (2026-10-16), à faire avant le commit :
  1. **Ralliement sur un chantier** (ou une fondation) = le Paysan né va le bâtir.
  2. **Chantier voisin** : un bâtisseur qui finit sans rien dans sa file va au chantier inachevé le plus proche du
     joueur s'il est proche (~20 m, réglable), sinon inactif.
  3. **`hol.Cheat.FastBuild 1`** : le chantier est **fini au premier coup** (choix de l'utilisateur : il faut encore
     qu'un Paysan arrive, façon « aegis » d'AoE2), au lieu d'une vitesse ×10.
  4. **Maj + clic droit sur une ressource** (bois, baies, filon) s'ajoute à la file comme un chantier : le Paysan
     bâtit ses chantiers puis va récolter (Maj + clic plusieurs fois, « très bien » pour les chantiers).
- **Retours de J6a.4 traités** (sous-agent, rien de commité, GDD non touché, présentation non touchée, 0 bit de plus) :
  1. **Ralliement sur un chantier** : centre choisi, clic droit sur un chantier ou une fondation à soi non finis →
     ralliement sur le chantier, drapeau en son milieu (comme une source) ; le serveur revérifie (à soi, non fini, pas
     le bâtiment lui-même ; `RallySite` côté serveur seul). Le Paysan né reçoit un ordre de bâtir ; chantier fini ou
     annulé entre-temps : marche vers son point comme un ralliement au sol (camp : inactif, enchaînement J6b).
  2. **Chantier voisin** : un bâtisseur qui finit un chantier, file vide, va au chantier non fini le plus proche de son
     joueur (fondations fantômes comprises) à **`hol.Build.NextSiteRange`** (cm, 2000) au plus, emprise à emprise
     (`HOLConstruction::GetGap` / `FindNextSite`) ; sinon inactif. Pas après une annulation.
  3. **`hol.Cheat.FastBuild 1`** : production en un pas et **chantier fini au premier coup** (`hol.Build.Instant`,
     absent en Shipping) ; `hol.Build.Speed N` reste à part. Journal `..., speed 1, instant`.
  4. **Maj + clic droit sur une ressource** : file mixte du bâtisseur (`HOLConstruction::TWorkItem`, chantier ou source ;
     `AdvanceWorkQueue`) ; l'ordre passe par la file des ordres de bâtir (`FHOLBuildOrder::SourceIndex`, ordre de clic
     gardé), revérifié (source debout, unités du joueur qui récoltent cette ressource). Venue en tête, la source est
     confiée à la récolte (`FHOLGatherFragment::StartTask`, phase `Gathering` du bâtisseur) jusqu'à épuisement du
     massif : massif plein avec une suite en file → arrêt sans se poster devant, élément suivant (`bQueueNext`) ;
     dernier élément : comme aujourd'hui. Sans Maj : la file est remplacée. Maj sur une unité sans file : la file
     commence par l'ordre (la récolte en cours est remplacée, comme pour un chantier).
  - Scripts : `hol.Select.Order / OrderGround / OrderSource ... shift`.
  - **Tests 54/54** (`Construction.WorkQueue`, `Construction.NextSite`). **`quick_check` PASS en 182 s** du premier
    coup (119 s avant ; partie build allongée). Partie solo **`build trial`** : ralliement sur le fantôme E (drapeau
    vu, capture `build_rally`), le Paysan né bâtit E en **1 pas** (`instant`), va à F (**900 cm**), pas à G (39 m),
    F en 1 pas, puis inactif ; Maj + H, Maj + I, Maj + clic droit sur un arbre (source 31) : H (100 pas à vitesse 4),
    puis I, puis `goes on to gather source 31 (Wood)`, HUD « Gathering Wood ». Chemin chaud de la récolte non touché
    (seul le cas rare « massif plein ») : pas de S7.
- Session d'essai relancée (deuxième écran) pour les 4 retours de J6a.4.
- **Essai validé** par l'utilisateur (« ok, tout fonctionne ») : J6a.4 et ses 4 retours commités (sans le panneau de triches de `mycrusader-bb` ni les lignes des autres sessions). Reste : S2 / S7 au repos. Reprise : **J6a.5** (HUD de la construction).
- **Panneau de triches** (session `mycrusader-bb`, cd75e85, D291) : en build de développement, sous la barre du haut à
  gauche, plié et déplié par son premier bouton (choix gardé dans GameUserSettings.ini) ; +1000 par ressource, +1000 de
  chaque, population +165, construction rapide oui / non ; `hol.HUD.Click CheatXxx` ; validé par l'utilisateur le
  2026-10-16.

### 2026-10-16 — J6a.5 : HUD de la construction

- Confié à un sous-agent (D313 : grille directe du Paysan A / Z / E, infobulle, case grisée et message bref,
  « Population maximale », bloc du chantier avec « Annuler », repère sur la fondation, F2 ; fr / en / ja).
- Sous-agent, rien de commité (essai de l'utilisateur attendu), GDD non touché, ni présentation ni table des unités.
- **Grille du Paysan** (règles pures `Player/HOLCommandGrid`) : un ou plusieurs Paysans à soi choisis (unités qui
  récoltent, comme le contrôle du serveur) → **A** maison, **Z** camp de bûcherons, **E** camp de mineurs (colonne
  `BuildGridSlot`, `UHOLBuildingSettings::GetBuildableTypes`) ; centre choisi → A = Paysan (même touche, autre sens,
  D281) ; chantier à soi choisi → **V « Annuler »** (loin des touches qui bâtissent ; le coût rendu sur la case : tout
  avant le premier coup, moitié après). Clic ou touche → fantôme de J6a.3. Contrôleur : `GetGridCommands`,
  `ActivateGridCell` (production, pose ou annulation), délégué `OnCommandFeedback`.
- **Case grisée** (D313) : faute de ressources (ou file pleine), case sombre (style `UHOLHUDButtonStyleGreyed`), nom
  atténué, **coût manquant en rouge** ; elle prend le clic : rien d'envoyé, **message bref** au-dessus de la grille
  (3 s, s'efface) : « Pas assez de bois / pierre / d'or / de nourriture », « File de production pleine ». Fantôme rouge
  cliqué : « Emplacement occupé », « Trop près d'un centre ennemi », « Hors de la carte », « Terrain impraticable ».
- **Infobulle** de chaque case (survol) : nom, coût (manquant en rouge), effet : « +10 population », « Dépôt : bois »,
  « Dépôt : pierre, or », Paysan « Récolte et bâtit », Annuler « Rend tout / la moitié du coût ».
- **Population maximale** : Paysan demandé sans place → mis en file quand même (il attend, comme avant) et message
  « Population maximale — construisez des maisons » ; même message quand une file du joueur se met à attendre.
- **Bloc du chantier** : « Fondation : Maison » (fantôme), puis « Chantier : Maison », points de vie, **barre
  d'avancement + « 15 % »**, « **Bâtisseurs : 2** » (au propriétaire seul : octet répliqué `BuildersAtWork` sur
  `AHOLBuilding`, écrit par `HOLBuildProcessor`, 0 quand personne n'y travaille ; rien sur l'état des unités).
- **Repère bref de J4.6** sur la fondation posée avec des Paysans choisis (clic droit sur un chantier : déjà là).
- **F2** : bâtiment économique suivant (centre d'abord, puis maisons, camps ; sites et fantômes compris), caméra
  centrée ; `IA_NextEconomic` (make_game_input.py relancé) ; F1 à F4 retirés des vues de débogage du moteur
  (`DefaultInput.ini`, sinon F2 = « viewmode unlit » en build de développement).
- Textes fr / en / ja (`fill_po.py`, GatherText deux fois ; 149 textes, 0 clé inconnue). Commandes : `hol.Select.Grid N`,
  `hol.Select.NextEconomic` ; `hol.HUD.Report` écrit aussi `HUD tooltips: ...` et `, message <Clé>` ;
  `hol.HUD.Click GridCell11` = Annuler.
- **Tests** `CommandGrid.Commands`, `.Feedback`, `.Cycle`, `.Tables` : **58/58** (54 avec J5c.2 fusionnée).
- **`quick_check` PASS en 155 s puis 157 s** (182 avant) ; partie nouvelle **`build hud`** (solo) : tous les Paysans
  sans bois → `A=House greyed Z=LumberCamp greyed E=MiningCamp greyed`, infobulles ; clic sur Z : pas de fantôme,
  « Pas assez de bois » capturé en fr / en / ja ; avec du bois, touche Z → fantôme du camp ; 2 Paysans, touche A, clic :
  maison J bâtie ; chantier choisi : « under construction 15%, builders 2 », `V=Cancel` ; clic → 25 bois rendus ;
  population pleine, touche A au centre : message ; F2 trois fois : centre, maison, autre maison, caméra dessus.
- Captures vues : `build_hud_fr/en/ja` (message à droite au-dessus de la grille, cases grisées, coûts en rouge),
  `build_hud_block` (bloc du chantier, barre, bâtisseurs, Annuler 25), `build_hud_popfull`, `build_hud_f2`,
  `build_hud_ghost`. Premier jet corrigé : noms des cases en 11 pt (« Camp de bûcherons » sur deux lignes), message
  non replié. **Infobulle non capturée** (pas de survol hors écran) : à voir à l'essai.
- À essayer : choisir des Paysans, A / Z / E (fantôme), survoler les cases ; `hol.Cheat.Wood -100000` puis Z : case
  grise, « Pas assez de bois » ; poser une maison avec 2-3 Paysans, cliquer le chantier (barre, bâtisseurs), V ou clic
  « Annuler » ; centre choisi à population pleine, A : message ; F2 plusieurs fois.
- Session d'essai lancée (deuxième écran) pour l'essai de J6a.5.
- **Retours de l'essai de J6a.5** (2026-10-16 ; « tout est bon sinon ») :
  1. **Pas de coût sur la grille** : une case ne montre que l'**icône** (le nom tant qu'il n'y a pas d'icône) ; coût et
     manque seulement dans l'infobulle. La case reste grisée si on ne peut pas payer.
  2. **Touches F** (précise D30, § 7.3) : **F1 sélectionne tous les bâtiments militaires**, **F2 tous les bâtiments
     économiques**, **F3 tous les bâtiments technologiques** (F4 héros) ; au lieu de « bâtiment suivant » seul. Appuis
     successifs : la sélection reste, la caméra passe au bâtiment suivant de la catégorie (lecture du § 7.3 gardée).
- **Retours appliqués** (sous-agent, rien de commité, GDD non touché) :
  - **Cases** : le nom seul (`HOLCommandGrid::GetCellFace` : l'icône quand il y en aura une, sinon le nom, jamais le
    coût) ; coût, manque en rouge et effet dans l'infobulle seulement ; case grisée et message au clic inchangés ;
    « Annuler » sans le montant (rendu dans l'infobulle). Nom en 11 pt, la boîte sur toute la largeur de la case :
    « Camp de / bûcherons » sur deux lignes, centré, sous la touche (vu à 1440p fr / en / ja ; en 13 pt, trois lignes
    collées à la touche).
  - **Catégorie** : colonne `Category` de la table des bâtiments (`EHOLBuildingCategory` Economic / Military /
    Technological ; les quatre types Economic), table refaite (make_building_types.py).
  - **F1 / F2 / F3** (`IA_MilitaryBuildings`, `IA_EconomicBuildings`, `IA_TechnologicalBuildings`, make_game_input.py ;
    `IA_NextEconomic` supprimé ; F4 non lié, débogage du moteur toujours retiré) : premier appui = tous les bâtiments à
    soi de la catégorie, finis et chantiers, pas les fantômes, caméra inchangée ; appui suivant tant que cette sélection
    reste = sélection gardée (rafraîchie), caméra au suivant (centre, puis maisons, camps). Aucun : message bref
    « Aucun bâtiment militaire / technologique / économique » (fr / en / ja, 152 textes), sélection inchangée.
    `hol.Select.Category <Military|Economic|Technological>` remplace `hol.Select.NextEconomic`.
  - **Sélection de plusieurs bâtiments** (minimale) : groupe de bâtiments à soi dans le contrôleur (`SetBuildingSelection`,
    `GetSelectedBuildings`) ; un cadre au sol sous chacun ; bloc « Maison × 6 / Centre principal × 1 / Vous » et
    « N en construction » ; grille = ce que tous produisent (plusieurs centres : Paysan, mis dans la file la moins
    chargée, `PickLeastBusy`), sinon vide ; jamais « Annuler » pour un groupe ; clic droit = point de ralliement de
    chacun de ceux qui produisent. Le clic, le cadre, Échap, « . » remplacent le groupe comme avant.
  - Tests `CommandGrid.Categories` et `.Group` ajoutés, `.Commands` (case) et `.Cycle` revus : **60/60**.
  - `quick_check` partie `build hud` : `HUD cells: A: Maison; Z: Camp de bûcherons; E: Camp de mineurs` (aucun
    chiffre), infobulles avec le coût ; F1 et F3 : rien, message, sélection vide ; F2 : 7 bâtiments (centre + 6 maisons,
    autant que le rapport des bâtiments ; pas de chantier à ce moment), 7 cadres, caméra inchangée ; F2 ×3 : centre,
    maison, maison, caméra dessus, sélection gardée. **PASS en 150, 167 puis 186 s** (machine chargée : une autre
    copie lançait ses essais). Captures vues : `build_hud_greyed`, `_f1`, `_f2`, `_next` (cadres sous les 7),
    `_block` ; `cells_*` à 1440p (fr / en / ja, grisé et prêt).
- Session d'essai relancée (deuxième écran) pour les retours de J6a.5.
- **2e retour de l'essai de J6a.5** (2026-10-16), à faire avant le commit (lecture retenue, à confirmer à l'essai) :
  1. Les **fondations pas encore commencées** comptent dans F1 / F2 / F3 et dans tous les groupes de sélection.
  2. **Groupe de bâtiments** : des sous-groupes parcourus par **Tab** : d'abord **« Tous »** (toutes les commandes du
     groupe ; à touche égale, le forum passe en premier, puis l'ordre de sélection : le forum produit sur A même si le
     moulin a une technologie sur A), puis un sous-groupe **par type** dans l'ordre de sélection (tous les moulins, tous
     les camps…) ; **chaque forum est un sous-groupe à lui** (« un forum n'a pas de catégorie, deux forums sont
     distincts »). Plusieurs bâtiments du même type visés par un ordre de production (sous-groupe ou « Tous ») :
     **répartition équitable** entre eux (forums compris dans « Tous », casernes plus tard).
- **2e retour appliqué** (sous-agent, rien de commité, GDD non touché) :
  - **Fantômes** : `IsSelectedByCategory(Type, Category)` (la catégorie seule) : F1 / F2 / F3 prennent aussi les fondations
    pas commencées ; groupe trié dans l'ordre du cycle (centre, puis par type et clé) = ordre de sélection. Le cadre ne
    sélectionne que des unités : rien d'autre à changer.
  - **Sous-groupes** (règles pures `HOLCommandGrid::SplitSubgroups`, `GetCommandPriority`, `MergeCommands`) : « Tous »,
    puis un par type dans l'ordre du groupe, chaque centre principal seul (`IsAloneInGroup`). Grille = cellules des
    bâtiments du sous-groupe actif fusionnées ; à touche égale, le centre d'abord, puis l'ordre du groupe. Sous-groupe
    d'un seul chantier : lu comme ce chantier choisi seul (V « Annuler ») ; « Tous » et sous-groupe de plusieurs :
    jamais. Ralliement : tout le groupe (clic droit), inchangé.
  - **Tab** : `IA_NextSubgroup` (make_game_input.py relancé ; Tab n'était lié à rien, seul Ctrl+Tab du moteur = fenêtres
    PIE), `CycleSubgroup`, `hol.Select.Subgroup` ; agit seulement avec un groupe ; nouveau groupe = « Tous » ; F2 répété
    ou bâtiment disparu : sous-groupe gardé (celui du même bâtiment), sinon « Tous ».
  - **HUD** : 1re ligne du bloc = sous-groupe actif (« Tous (8) », « Maison × 7 », « Centre principal »), puis le groupe
    par type (ordre du groupe) ; `SubgroupAll` fr / en / ja (fill_po.py, GatherText deux fois, 153 textes, 0 inconnue).
    Cadres : tous, ceux hors du sous-groupe actif en trait trois fois plus fin (un gris foncé se fondait dans le sol au
    premier essai). `hol.HUD.Report` / `hol.Select.Report` : `, subgroup All (8)` ; repères : `frames dimmed N`.
  - **Répartition équitable** (`PickEvenly`, remplace `PickLeastBusy`) : moins de file d'abord, à égalité le suivant du
    dernier choisi dans l'ordre du groupe (mémoire par type d'unité dans le contrôleur) : 4 appuis sur 2 centres = 2 + 2,
    même avant que les files arrivent. Testée en unitaire seulement (un centre par joueur sur Plain, pas de commande
    pour faire naître un bâtiment).
  - Tests `CommandGrid.Subgroups` et `.Distribution` ajoutés, `.Categories` et `.Group` revus : **62/62**.
  - `quick_check` partie `build hud` : F2 = 8 bâtiments, fantôme G compris (« sites 1 ») ; `build hud tab` : « Tous » →
    touche A → Paysan au centre ; Tab → `MainCentre` (A: Paysan, 7 cadres fins) ; Tab → `House x7` (cellules vides,
    1 cadre fin) ; Échap, F2 → « Tous (8) ». **PASS en 151 s (gather KO une fois : Paysans au repos à 70 cm, sans lien,
    PASS aux deux passages suivants), 151 s, 150 s.** Captures vues : `build_hud_f2`, `build_hud_tab_centre`,
    `build_hud_tab_houses`.
- À essayer : F2 avec un fantôme posé ; Tab plusieurs fois (titre du bloc, cases, cadres fins) ; A sur « Tous ».
- Session d'essai relancée (deuxième écran) pour le 2e retour de J6a.5 (sous-groupes, Tab, répartition).
- **3e retour de l'essai de J6a.5** : « tout est bien », sauf : en F2, sur le sous-groupe du forum ou sur « Tous », le **drapeau de ralliement n'apparaît pas** (il apparaît quand le forum est sélectionné seul). À corriger avant le commit.
- **3e retour appliqué** (sous-agent, rien de commité, GDD non touché) : cause, le drapeau suivait `GetOwnSelection` (un bâtiment choisi seul) et un seul mât existait ; correction dans `HOLSelectionMarkers` : un drapeau par bâtiment de `GetCommandBuildings` qui a un point de ralliement (choisi seul, sous-groupe actif, tous pour « Tous »), mâts faits au besoin, suivis à chaque image (Tab, clic droit sur le groupe) ; contrôleur inchangé ; repères : `flag 1 at (x, y)`. `quick_check` partie `build hud rally` : « Tous » et « Centre principal » → `flag 1 at (-16800, -1500)` (E), maisons → `flag 0`, clic droit au sol sur « Tous » → ralliement du centre et drapeau à (-17999, -1500). **PASS en 151 s du premier coup, 62/62** ; capture `build_hud_rally` vue (drapeau au point, « Tous (8) »).
- Session d'essai relancée (deuxième écran) pour le drapeau des groupes.
- **Essai validé** par l'utilisateur (« c'est bon ») : J6a.5 commitée (sans les lignes des autres sessions). Reste : S2 / S7 au repos, puis vérification et clôture de J6a (J6a.6).
- Fusion de `cheat-unit-speed` (d486f84, panneau de triches : curseurs de vitesse des unités et de la collecte, D291) sur main (435094f), en `--no-commit` : fusion automatique cohérente avec J6a.5, aucune correction ; build éditeur OK (47 s) ; `quick_check` PASS en 151 s, tests 62/62 (du premier coup) ; essai console hors écran : `hol.Cheat.UnitSpeed 2` → `hol.Units.SpeedScale = 2`, `hol.Cheat.GatherSpeed 3` → `hol.Gather.SpeedScale = 3`, 50 tenu à 10. Fusion non committée.

### 2026-10-16 — J6a.6 : mesures au repos

- Sous-agent, rien de commité, aucun code touché, main à bfda9b4 (éditeur compilé à jour). Mêmes commandes que le matin
  (`-RenderOffScreen -ResX=888 -ResY=500`, journaux `Saved_Tests\Logs\j6a6_s1.log`, `j6a6_s2.log`, `j6a6_s7.log`).
  Servent aussi à J5c (animations : J5c.1, J5c.2, marteau du bâtisseur).
- **Machine pas au repos** : l'éditeur ProjectShiva n'était pas fermé (4,6 % avant), il s'est fermé vers le début de S1
  puis a été **relancé deux fois par un autre processus** (via WmiPrvSE, 20 h 43 juste après la mesure de S1, puis
  20 h 53 avec `-log -abslog` pendant S5) : 3-9 % pendant S2/S7, ~11 % pendant S5, **12-13 % stable ensuite**
  (~2 cœurs). Processeur total : 9-38 % avant les mesures (10-15 % le matin), 40-50 % au calme après S5 ; pendant les
  passages (test compris) S1 61 %, S2 62 %, S7 73 %, S5 89 % (99 au plus). Autres : GitKraken 3-4 %, Opera ~6 %,
  Notepad, IMF, Battle.net, VS Code, Discord.
- **S1** (10 s) : **2,566 ms PASS** (2,184 le matin, +17 %) ; évitement à l'arrêt 9,4 ms un pas sur 6 (8,1) ; 186 i/s
  (241), fil du jeu 5,19 ms (4,02), rendu 2,46 (1,87), GPU 2,37 (2,30).
- **S2** (10 s) : **3,178 ms FAIL** (2,738, +16 %) ; évitement en marche 1,31 (1,00), séparation 0,91 (0,76),
  pleine vitesse 0,60 (0,50), ordres 0,59 (0,57) ; 598 puis 407 paires (583 / 393) ; 185 i/s (234), GPU 2,36 (2,32).
- **S7** (20 s) : **2,790 ms PASS** (2,155, +29 %, marge 0,21 ms) ; évitement à l'arrêt 7,2 (5,6), récolte 0,263
  (0,204), ordres 0,182 (0,157), évitement en marche 0,378 (0,261) ; **1 198 bois/s** (1 198), 10,9 chemins/s (13,1),
  219,8 repris/s ; 0 raté, 0 inactif ; « Buildings (D310) : 0 bodies put back out of a footprint, 0 walkers given a
  new path » ; 162 i/s (248), fil du jeu 5,88 (3,97), rendu 3,14 (1,93), **GPU 3,60 (2,15 : +1,45 ms, J5c.2)**.
- **Nouveaux processeurs** : `HOLBuildProcessor` 0,003-0,004 ms partout ; `HOLFootprintKeepOutProcessor` 0,006-0,007
  dans S1/S2, **0,124 ms dans S7** (les 32 dépôts ont une emprise) : seul surcoût propre du code (+0,13 ms sur les
  +0,64 de S7). Tous les anciens processeurs sont 10-30 % plus lents, uniformément : signature de la charge, pas du
  code. Le côté client / présentation (animations) ne figure pas dans le détail du pas, seulement dans le fil du jeu,
  le rendu et le GPU.
- **S5 court** (`net_match.ps1 -Stage -Units 2000 -Seconds 40`, serveur recuit sur bfda9b4 : 565 paquets cuits,
  176 s de préparation, 240 s en tout) : **PASS à 31 428 o/s par client** (31 227 : +201 o/s ; seuil 32 000, **marge
  0,57 Ko/s**), 1,78 envoi par unité et par seconde, 42 paquets/s, écart affiché **23,2 cm PASS** (21,6 ; seuil 25),
  réplication 0,203 ms par pas (0,148), simulation du serveur 9,36 ms (6,95) ; client : états 25,2 Ko/s, le reste
  5,9 Ko/s ; 128 et 145 i/s. Écart et temps du serveur probablement gonflés par la charge (89 %).
- Bilan : S1, S7, S5 PASS ; **S2 FAIL sous charge** (ProjectShiva relancé). À refaire au vrai repos (ProjectShiva
  fermé et pas relancé) pour conclure J6a.6 et J5c.

- **Mesures refaites au repos** (même jour, ~21 h 25-21 h 33, sous-agent, rien de commité, aucun code touché, main à
  bfda9b4, serveur cuit de bfda9b4 sans `-Stage`). Servent aussi à **J5c** (mycrusader-23 a suspendu ses lancements).
  **Charge** : aucun Unreal ni ProjectShiva avant ni pendant aucun passage (vérifié avant chaque lancement et toutes les
  2-3 s pendant) ; processeur total avant 8-20 % (10-15 le matin) ; pendant les passages (test compris) S1 33 % en
  moyenne (63 au plus), S2 39 % (52), S7 46 % (63 ; Defender 12 % un instant, sinon Opera ~3-4 %, VS Code ~2 %),
  S5 68 % (83). Mêmes commandes (journaux `Saved_Tests\Logs\j6a6b_s1.log`, `j6a6b_s2.log`, `j6a6b_s7.log`).
  - **S1** (10 s) : **2,276 ms PASS** (2,184 le matin, +4 %) ; évitement à l'arrêt 8,58 ms un pas sur 6 (8,10) ;
    226 i/s (241), fil du jeu 4,27 ms (4,02), rendu 1,98 (1,87), GPU 2,36 (2,30) ; 1 399 paires.
  - **S2** (10 s) : **2,882 ms PASS** (2,738, +5 % ; marge 0,12 ms) ; évitement en marche 1,207 (1,001), séparation
    0,813 (0,758), pleine vitesse 0,533 (0,502), ordres 0,555 (0,568) ; 598 puis 407 paires (583 / 393) ; 219 i/s
    (234), fil du jeu 4,41 (4,12), rendu 2,01 (1,94), GPU 2,34 (2,32).
  - **S7** (20 s) : **2,303 ms PASS** (2,155, +0,148 ms) ; évitement à l'arrêt 5,86 (5,60), récolte 0,220 (0,204),
    ordres 0,144 (0,157), séparation 0,108 (0,117), évitement en marche 0,286 (0,261) ; **1 198 bois/s** (1 198),
    10,9 chemins/s, 219,8 repris/s, 0 raté, 0 inactif ; « Buildings (D310) : 0 bodies put back out of a footprint,
    0 walkers given a new path » ; 223 i/s (248), fil du jeu 4,40 (3,97), rendu 2,18 (1,93), **GPU 3,34 (2,15 :
    +1,2 ms, animations de J5c)**.
  - **Nouveaux processeurs** : `HOLBuildProcessor` 0,003 ms partout ; `HOLFootprintKeepOutProcessor` 0,005 (S1),
    0,008 (S2), **0,105 (S7**, 32 dépôts à emprise) : il fait l'essentiel des +0,148 ms de S7 (le reste : évitement à
    l'arrêt +0,04 par pas, récolte +0,016). Anciens processeurs à ±5-10 % du matin (bruit).
  - **S5 court** (`net_match.ps1 -Units 2000 -Seconds 40`, sans `-Stage`, 64 s en tout) : **PASS à 30 937 o/s par
    client** (31 227 le matin, 31 428 sous charge ; seuil 32 000, marge 1,06 Ko/s), 1,76 envoi par unité et par
    seconde, 42 paquets/s, écart affiché **23,3 cm PASS** (21,6 le matin, 23,2 sous charge ; seuil 25, marge 1,7 cm ;
    23,2 et 23,4 par client, médiane 21,3), réplication 0,160 ms par pas (0,148), simulation du serveur 7,44 ms
    (6,95) ; client : états 25,3-25,4 Ko/s, le reste 5,9 Ko/s ; 168 et 164 i/s.
  - **Bilan : tout PASS au repos** (S1, S2, S7, S5) : le FAIL de S2 venait de la charge. J6a.6 (mesures) et J5c ont
    leurs mesures. À suivre : la marge de S2 (0,12 ms), le GPU de S7 (+1,2 ms) et l'écart de S5 (+1,7 cm, le même
    chargé ou non).
- Session d'essai lancée (deuxième écran) : **essai complet de J6a** (pose, chantier, HUD, animations, triches).
- **Essai complet validé** par l'utilisateur (« tout est bon ») : **J6a close** (2026-10-16). D310 à D314 iront au GDD à la clôture de J6 (J6b.4). Reprise : **J6b.1** (camps : camp fini → ses bâtisseurs récoltent la source la plus proche qu'il accepte).

### 2026-10-16 — J5c.4 : clôture

- Sous-agent, documentation seule, rien de commité, aucun code ni worktree touché.
- Mesures de clôture : celles de J6a.6 au repos (`main` bfda9b4, J5c.1, J5c.2 et le bâtisseur) : S1 2,276 ms, S2
  2,882 ms, S7 2,303 ms par pas, S5 30 937 o/s par client (écart affiché 23,3 cm), tous PASS ; GPU de S7 3,34 ms contre
  2,15 avant J5c (+1,2 ms, surtout les pièces). Tests 62/62 et `quick_check` PASS (151 s) à la dernière fusion (bâtisseur,
  3b750bc). Partie « anims » de `quick_check` ajoutée en parallèle par une autre tâche (partie « anims », 16 s : rapport `hol.Units.AnimReport` ; geste et pièce de chaque Paysan dessiné au travail, en marche chargée et debout chargé ; `quick_check` PASS en 163 s).
- GDD § 21 : **D318** (source des animations : faites par Claude, par script ; worktree fusionné après validation),
  **D319** (ressource récoltée dans les valeurs libres de l'activité, 0 bit ; les 3 bits pleins depuis `Building`),
  **D320** (pas de geste au dépôt), **D321** (gestes, marches chargées, choix chez le client, Paysan tourné vers sa
  source, vitesses de lecture), **D322** (outils et charges visibles : questions de J5c.2, charge montrée à l'arrêt
  contre la recommandation), **D323** (pièces fusionnées dans `SKM_Peasant`, `ik_hand_gun`, 4e float, WPO ; coût GPU
  +1,06 ms ; piste des LOD 2-3), après D317. **D298 précisée** (*fait en J5c* : « +N » au seul propriétaire par un appel
  client non fiable, sans bit ; J5c.3 n'a pas de numéro propre).
- GDD § 16.6 : paragraphe **« Parcours J5c : la vie de la récolte »** après « Parcours J5b » (ce que voit le joueur,
  D318 à D323, « +N », marteau du bâtisseur fait avec la même chaîne, vérifications, coût A/B, mesures au repos).
  § 22 : numéro suivant **D324** (D310 à D314 au plan de J6, D318 à D323 à J5c).
- **J5c close, J5 close.** Reprise inchangée : **J6b.1**.

### 2026-10-16 — J6b.1 : les camps

- Sous-agent, rien de commité, GDD non touché, ni présentation, ni table des unités, ni réplication (0 bit de plus :
  tout se passe côté serveur / simulation).
- **Camp fini → ses bâtisseurs récoltent** (D312) : un « camp » = bâtiment dont `Accepts` n'est pas vide et qui n'est
  pas un centre principal (`HOLConstruction::SendsBuildersToGather`, `AHOLBuilding::IsCamp`). Au pas qui suit sa fin,
  chaque bâtisseur dont la file est vide (après les chantiers finis ou annulés qu'elle tenait) reçoit la **source debout
  la plus proche que le camp accepte** (`HOLConstruction::FindCampSource` : distance de l'emprise au **bord** de la
  source ; camp de mineurs : la plus proche, pierre ou or ; à égalité, la première), choisie une fois pour tous ses
  bâtisseurs, dans **`hol.Build.CampGatherRange`** (cm, **2500** = 25 m ; négatif : jamais). Elle lui est donnée comme un
  ordre de récolte (`FHOLGatherFragment::StartTask`, file oubliée, activité « tournée de récolte ») : places, massifs,
  bosquet voisin et inactif devant de D309 inchangés, charge gardée jusqu'à la première unité d'une autre ressource
  (D306). Ceux qui ont encore quelque chose dans leur file (Maj) la suivent d'abord, comme pour une maison. Aucune source
  acceptée assez près (ou une unité qui ne récolte pas cette ressource) : la règle d'avant (chantier voisin à
  `hol.Build.NextSiteRange`, sinon inactif). Choix : le camp passe **avant** le chantier voisin.
- Journal (une ligne par camp, au pas où ses bâtisseurs repartent) : `Construction: slot 0 camp LumberCamp at (x, y)
  finished: 2 builders sent to gather Wood (source 8, 3.6 m)`, ou `... finished: no accepted source within 25 m, 2
  builders go on as after a house`.
- **Ralliement** : un Paysan né avec un ralliement sur un camp inachevé le bâtit ; fini avant son arrivée, il récolte
  comme les autres bâtisseurs. Camp **fini avant sa naissance** : ordre de récolte direct de la même source
  (`UHOLUnitSubsystem`, journal `Rally: 1 units of slot 0 born for the finished camp LumberCamp at (x, y) sent to gather
  Wood (source i, d m)`) ; aucune source : il marche vers le camp comme avant. Le ralliement peut désormais se poser sur un
  **camp fini** (contrôleur et serveur : `!IsFinished() || IsCamp()`), drapeau en son milieu. Cas rare couvert : un
  ordre de bâtir arrivé un pas après la fin d'un camp (`ApplyBuildOrders`) devient un ordre de récolte de sa source.
- **Camp inachevé** : n'accepte aucun dépôt (déjà le cas : `RefreshDeposits` ne prend que les bâtiments finis) ; vérifié
  dans `quick_check`. Journal des dépôts (`hol.Gather.LogDeposits 1`) complété : `... at LumberCamp (x, y), unit #N`.
- **S7** : ses camps posés par le code sont déjà la vraie ligne `LumberCamp`, nés finis, sans bâtisseur : rien ne change
  (S7 avec des camps bâtis en cours de test = J6b.3). Chemin chaud de la récolte non touché (seul le journal sous
  `LogDeposits`) : pas de S1 / S2 / S7.
- Fichiers : `Buildings/HOLConstructionRules.h/.cpp` (`FHOLCampSource`, `SendsBuildersToGather`, `GetSourceGap`,
  `FindCampSource`), `Buildings/HOLBuilding.h/.cpp` (`IsCamp`), `Simulation/HOLBuildProcessor.h/.cpp` (variable,
  `FindCampSource`, `GatherForCamp`, `LogCampSends`, ordre tardif), `Simulation/HOLUnitSubsystem.cpp` (naissance),
  `Buildings/HOLProductionSubsystem.cpp` et `Player/HOLPlayerController.cpp` (ralliement sur un camp fini),
  `Simulation/HOLGatherProcessor.h/.cpp` (journal des dépôts), `Tests/HOLConstructionTests.cpp`, `Tools/Tests/quick_check.ps1`.
- **Compilation** sans avertissement (44 s). **Tests 63/63** (62 avant ; nouveau `Construction.CampSource`).
- **`quick_check` PASS en 202 s** (151 avant). Un premier passage (même code) avait échoué en `gather` (le contrôle des
  « +10 » prenait le journal jusqu'au bout : borné désormais à la marque `build camp`) et en `net` (lancé en même temps
  que le `quick_check` de `mycrusader-23` : port du serveur partagé) ; second passage, machine libre : tout PASS.
  Partie nouvelle **`build camp`** (solo, après `build hud`) : tous les Paysans de l'emplacement 0 envoyés derrière le
  centre, file du centre vidée, `hol.Build.Speed 2`, `hol.Gather.SpeedScale 5` ; un Paysan envoyé à l'arbre le plus
  proche du futur camp ; deux Paysans posent le camp de bûcherons L à (-13500, -2100), à 4 m du bois ; ralliement du
  centre sur le chantier L, Paysan en file (12 s). Résultats : L fini en 163 pas, `2 builders sent to gather Wood
  (source 8, 3.6 m)`, ils déposent **au camp** (aucun au centre) ; le premier Paysan dépose **au centre tant que L n'est
  pas fini**, au camp ensuite ; le Paysan né après la fin de L : `Rally: 1 units ... sent to gather Wood`, dépose au
  camp. Camp de mineurs M à (-12000, -3900), à 4,3 m du filon de pierre (l'or à 60 m) : `2 builders sent to gather
  Stone (source 372, 4.3 m)`, pierre déposée à M. Camp de bûcherons à (-19500, -900), à 34 m de tout bois : `no
  accepted source within 25 m`, ses 2 bâtisseurs inactifs.
- Captures vues : `build_camp_wood` (camp fini derrière le bois, « +10 » au camp, « Bois (4) » ; le bois masque un peu
  les récolteurs, caméra fixe), `build_camp_stone` (camp de mineurs, 2 Paysans au filon, « 2 en récolte », « Pierre
  (2) »), `build_camp_idle` (camp loin du bois, « 2 inactifs », Inactifs 31).
- À essayer : `hol.Cheat.Wood 500`, choisir 2-3 Paysans, `hol.Build.Place LumberCamp` (ou Z) près du bois, clic : ils
  bâtissent puis récoltent le bois et déposent au camp ; idem `hol.Build.Place MiningCamp` (ou E) près d'un filon
  (pierre ou or, le plus proche) ; un camp loin de tout : inactifs. Maj : poser un camp puis une maison, ils font les deux
  avant de récolter. Ralliement : centre choisi, clic droit sur un camp (chantier ou fini), `hol.Prod.Queue` : le Paysan
  né récolte près du camp. `hol.Build.Speed 4` ou `hol.Cheat.FastBuild 1` pour aller vite ; `hol.Build.CampGatherRange
  1000` pour essayer une portée plus courte ; `hol.Gather.LogDeposits 1` pour voir où chacun dépose.
- À faire valider à l'essai : le camp avant le chantier voisin ; la portée de 25 m ; le ralliement sur un camp fini (une
  nouveauté : tout Paysan né y part récolter, comme AoE4).
- Session d'essai lancée (deuxième écran, 22 h 12) : **essai de J6b.1** (camps, enchaînement vers la récolte, ralliement sur un camp). À juger : la source du camp avant le chantier voisin, la portée de 25 m (`hol.Build.CampGatherRange`), le ralliement sur un camp déjà fini.

- **Essai validé** par l'utilisateur (2026-10-17, « tout m'a l'air bon ») : les trois points gardés tels quels (camp
  avant le chantier voisin, portée de 25 m, ralliement sur un camp fini). Commitée. Reprise : **J6b.2** (chemins coupés
  par une emprise).

### 2026-10-17 — J6b.2 : chemins coupés par une emprise

- Sous-agent, rien de commité, GDD non touché, ni présentation, ni table des unités, ni réplication (0 bit de plus).
- **État des lieux (avant)** :
  - chemins gardés de D305 : une emprise posée appelait `NotifyNavigationChanged` (boîte de la zone interdite) ; au pas
    suivant, tout chemin gardé dont la **boîte englobante + 4 m** touchait la zone était oublié, et un chemin trouvé dans
    les 5 s autour n'était pas gardé ; pas de compte, pas d'attente des tuiles (au-delà de 5 s, un chemin à travers des
    tuiles pas encore refaites pouvait être gardé) ;
  - marcheurs : **rien de préventif**. Seul le remède de J6a.1 : un marcheur remis contre le mur **10 pas de suite** par
    `HOLFootprintKeepOutProcessor` reçoit un nouveau chemin (`BuildingPushes`, compté « walkers given a new path for it »
    dans S7) ; il bute donc sur le bâtiment avant de le contourner ; ordres de groupe (couloir partagé) et détours gardés
    (D308) idem ;
  - **le moteur refaisait en place nos chemins** : tout `FNavigationPath` est enregistré auprès du navmesh ; quand des
    tuiles changent sous lui, le moteur le recalcule (depuis le départ de la marche) dans son tick, sans toucher au
    couloir (`FNavCorridor`) que nos marcheurs suivent : coût caché, points et couloir désaccordés (chemins gardés de D305
    compris).
- **Fait** (réglage `hol.Units.CutPaths 1` ; 0 = comportement d'avant, pour des A/B) :
  - `AHOLBuilding::FitNavObstacle` : une emprise qui **se met à bloquer** le navmesh (premier coup d'un chantier, bâtiment
    posé fini par le code : centres, camps de S7) appelle `UHOLCommandSubsystem::NotifyFootprintBlocked` (type, milieu,
    taille, lacet, zone interdite) au lieu de `NotifyNavigationChanged` (gardé pour le reste : source épuisée, chantier
    annulé) ;
  - `UHOLUnitOrderProcessor::UpdateNewFootprints` (pas suivant) : les **chemins gardés de D305 qui la traversent sont
    oubliés aussitôt** (tracé exact, ou segment départ-arrivée d'une marche droite gardée, contre l'emprise élargie de
    25 + 60 cm), comptés (M) ; puis l'emprise **attend ses tuiles** : refaites quand le générateur du navmesh n'a plus de
    tuile à refaire dans sa zone (`FRecastNavMeshGenerator::HasDirtyTiles`, module `Navmesh` ajouté) **et** que les 8
    sondes sous l'emprise (celles de la pose, D311, tournées avec le bâtiment) sont hors du navmesh ; au plus 100 pas
    (5 s), puis contrôle quand même (« tiles not seen rebuilt »). Pendant l'attente, aucun chemin trouvé à travers n'est
    gardé ;
  - tuiles refaites : dans le passage des unités (`AdvanceUnits`, seulement aux pas où une emprise est prête), chaque
    marcheur (phases `Path` / `FinalLeg`) dont le **chemin restant** (sa position, le reste de son couloir depuis la
    portion la plus proche, puis sa place ; ou la ligne droite vers sa place) passe à moins de **25 cm + son rayon + sa
    place en travers du couloir** de l'emprise reçoit un **nouveau chemin**, regroupé par ordre et par paquet (règles de
    J4.3, `RequestCutPaths`), sa place en travers recalculée sur la nouvelle marche ; une **destination sous l'emprise**
    est sortie le long du rayon depuis son milieu jusqu'à 105 cm (corps, marge, 20 cm), sur le navmesh (les places d'un
    groupe gardent leur écart) ;
  - journal, une ligne par emprise : `Buildings (D310): footprint House at (x, y): N walkers given a new path, M reused
    gather paths dropped[, K destinations moved out of it] (tiles rebuilt after S steps)` ; S7 : ligne « New footprints
    (J6b.2) » ;
  - **chemins figés** : `EnableRecalculationOnInvalidation(false)` sur nos chemins : le moteur ne les refait plus en
    place dans son tick (couloir et points restent d'accord ; le recalcul passe par la règle ci-dessus) ;
  - fonctions pures (`HOLConstructionRules`) : `GetSegmentDistance`, `DoesWalkCross`, `PushOutOfFootprint` ; test
    **`HeroesOfLiberty.Construction.CutPaths`**.
- **Cas limites** :
  - **unité dans l'emprise** (son centre dedans) : la garde des emprises la sort (D310) ; le contrôle ne regarde que la
    suite de son chemin (depuis son point suivant) : nouveau chemin seulement si cette suite traverse. Une unité qui frôle
    seulement le mur est contrôlée comme les autres. Vu : 5 Paysans sur la fondation au premier coup, sortis par la
    garde, 5 nouveaux chemins, tous arrivés ;
  - **destination dans l'emprise** : sortie le long du rayon (ci-dessus) ; la tâche (récolte, chantier) juge l'arrivée
    comme avant (place manquée → autre place, D309 ; chantier : autre place). Vu : 5 places d'un groupe déplacées, places
    distinctes, 5 arrivés et arrêtés (une première version, au point du navmesh le plus proche, en mettait deux au même
    point) ;
  - **Paysans enfermés** (anneau de 8 maisons, 3 Paysans dedans) : **bouclage préexistant trouvé** : un chemin partiel
    réduit à un point (déjà au plus près) comptait pour « pas de chemin » → marche droite vers la place, dans le mur, remis
    contre le mur, nouveau chemin identique tous les 10 pas, **sans fin** (1 138 remises en 20 s). Corrigé sans règle de
    jeu nouvelle : un tel chemin, un bâtiment en travers (ou un nouveau chemin pour un bâtiment), laisse l'unité **où elle
    est** ; la sortie droite d'une poche d'arbres (D308) reste, mais plus à travers un bâtiment ; un récolteur dont **6
    marches de suite** vers sa source finissent court de leur place (chemin partiel, `bShortOfSlot`) **s'arrête avec sa
    charge** (« Gather: a unit of slot N could not reach its source, it stands with its load »), comme le porteur qui
    n'atteint pas de dépôt (6) et le bâtisseur (6). Ordre de sortie : ils vont au plus près dans l'enclos et s'arrêtent.
    **À juger par l'utilisateur** : refuser une pose qui enferme des unités (question ouverte du plan) ; rien de tel fait ;
  - **plusieurs emprises dans le même pas** : chacune attend ses tuiles ; celles prêtes au même pas sont contrôlées en un
    seul passage, chaque marcheur compté pour la première qui coupe son chemin ; vu à l'anneau (8 maisons en quelques
    secondes) et au lancement de chaque partie (centres).
- **Mesures** : tuiles d'une maison refaites dès le pas suivant (« after 0 steps ») ; 2-3 pas pour les centres au
  lancement. Coût nul hors des pas où une emprise est prête.
- Fichiers : `Buildings/HOLBuilding.cpp`, `Buildings/HOLConstructionRules.h/.cpp`, `Commands/HOLCommandSubsystem.h/.cpp`,
  `Commands/HOLUnitOrderProcessor.h/.cpp`, `Simulation/HOLSimulationFragments.h` (`bShortOfSlot`, `UnreachableWalks` :
  serveur seul, 0 bit répliqué), `Simulation/HOLGatherProcessor.cpp`, `StressTest/HOLStressTestSubsystem.h/.cpp`,
  `Tests/HOLConstructionTests.cpp`, `HeroesOfLiberty.Build.cs` (`Navmesh`), `Tools/Tests/quick_check.ps1`.
- **Compilation** sans avertissement. **Tests 64/64** (nouveau `Construction.CutPaths`).
- **`quick_check` PASS en 259 s** (202 avant). Partie nouvelle **`build cut`** (solo, après `build camp`) : 6 Paysans
  récoltent le filon au nord-est de l'emplacement 0 et déposent au centre (50 m ; un filon ne s'épuise pas, alors que
  chaque arbre abattu oublie les chemins gardés autour de lui) ; 25 s après, maison K à (-12900, 2700) en travers,
  commencée : `6 walkers given a new path, 4 reused gather paths dropped (tiles rebuilt after 0 steps)`, puis 12 s
  surveillées : **0 corps remis par une emprise, 0 chemin de remède**, allers-retours qui continuent (dépôts au centre) ;
  6 Paysans envoyés de (-9000, 5000) à (-9000, -4000), maison N commencée à (-9000, 0) : `6 walkers given a new path`,
  0 remis, les 6 arrivés. Captures vues : `build_cut_gather` (récolteurs contournant K), `build_cut_move` (le groupe
  longeant N). Un passage complet a échoué une fois sur « no two Peasants at rest closer than 90 cm » (71 cm : deux
  porteurs arrêtés au mur du centre, partie `gather`, avant toute maison ; déjà vu une fois en J5c) ; repassé : tout PASS.
- **S2 / S7 courts** hors écran (`Saved_Tests\Logs\j6b2_s2.log`, `j6b2_s7.log`), aucun autre éditeur ouvert avant ni
  pendant, processeur 22-46 % avant, 57-64 % en moyenne pendant (test compris ; 39-46 % aux mesures au repos de J6a.6) :
  **S7 2,289 ms PASS** (2,303 au repos), ordres 0,156 (0,144), garde des emprises 0,107 (0,105), 1 190 bois/s, 218
  repris/s ; **S2 2,758 ms PASS** (2,882), ordres 0,542 (0,555). S7 ne contrôle aucune emprise pendant la mesure (camps
  posés au début ; J6b.3 les bâtira en cours de test).
- À essayer à la main : `hol.Cheat.Wood 500` ; des Paysans en aller-retour (bois ou filon) ; une maison posée en travers
  de leur trajet et commencée : ils la contournent sans buter (`hol.Units.KeepOut 2` journalise chaque remise contre un
  mur, `hol.Nav.Footprints on` puis `off` en fait le compte) ; un groupe envoyé loin, une maison commencée sur son chemin,
  ou **sur son point d'arrivée** ; des Paysans enfermés dans un anneau de maisons (`hol.Cheat.FastBuild 1`) puis un ordre
  (ils restent dedans ; ceux qui récoltent finissent inactifs, avec leur charge). A/B : `hol.Units.CutPaths 0`.
- Session d'essai lancée (deuxième écran, 11 h 25) : **essai de J6b.2** (maison en travers d'allers-retours ou d'un groupe, maison sur un point d'arrivée, enclos de maisons). À juger : refuser une pose qui enferme des unités ? récolteur arrêté après 6 marches manquées ; groupe qui frôle un coin.
- **Essai validé** par l'utilisateur (2026-10-17, « super ») : commitée. **Nouvelle règle** demandée à l'essai : un
  bâtiment prend X cases mais le bâtiment lui-même n'en fait que 90-95 % ; un Paysan peut passer entre deux maisons
  voisines, mais pas deux de front. À régler avant J6b.3 (écart à choisir : le corps d'un Paysan fait 1,2 m de large,
  rayon 60 cm, plus 25 cm de marge de J6a.1).

### 2026-10-17 — D324 : bâtiments rentrés dans leurs cases (nouvelle règle de l'utilisateur)

- Demande à l'essai de J6b.2 : « un bâtiment prend X cases mais le bâtiment en lui-même fait 90-95 % de cette taille, un
  Paysan peut passer entre deux maisons mais pas deux ». Le Paysan fait 1,2 m de large (rayon 60 cm) : à 90-95 %, deux
  maisons voisines (6 m) ne laissent que 30-60 cm. Question posée (marge fixe / marge fine et passage serré / 90-95 % et
  Paysan plus fin) : **choix A, la recommandation** : **D324** : chaque bâtiment est **rentré d'une marge fixe (~90 cm)
  sur chaque côté** de ses cases, quelle que soit sa taille ; la pose, le chevauchement et l'anti-rush restent sur les
  cases ; le corps (forme, zone interdite du navmesh, garde de J6a.1, places des bâtisseurs et des dépôts) sur le bâtiment
  rentré. Écart de ~1,8 m entre deux voisins : un Paysan passe, deux de front non. Maison à ~70 % de ses cases, forum à
  ~85 %. Numéro D324 réservé au § 22 du GDD (suivant : D325). Écartés : passage serré à 1,2 m (frôle les murs, marge de
  J6a.1 à retirer), Paysan plus fin (évitement, récolte et stress tests à reprendre).
- Main avancée à 7595c81 par mycrusader-32 (partie « anims » de `quick_check`, `stress_ab.ps1`) : binaires de la copie
  principale à recompiler. Code de D324 confié à un sous-agent (J6b.2b).

### 2026-10-17 — J6b.2b : bâtiments rentrés dans leurs cases (D324)

- Sous-agent, rien de commité, GDD non touché, ni présentation, ni table des unités, ni réplication (0 bit :
  le corps se déduit des cases et du réglage sur chaque machine).
- **Réglage** `BodyInset` (`UHOLBuildingSettings`, `DefaultGame.ini`), règle pure `HOLConstruction::GetBodySize` /
  `GetBody` (côté jamais sous 1 m), `AHOLBuilding::GetBodySize` / `GetBodyBounds` / `IsOnBody`.
- **Inventaire cases / corps** :
  - **cases** : `SnapToGrid` et pose (`UHOLConstructionSubsystem::SnapFootprint` / `CheckPlacement` : chevauchement entre
    bâtiments et fondations, sources, carte, anti-rush, sondes du navmesh sous chaque case), fantôme posable ou non et son
    cadre au sol case par case, forum sur la grille (`AHOLStartSlot`), `GetFootprintBounds` ;
  - **corps** : forme provisoire de D261, dalle et chantier qui monte, fondation fantôme, arêtes du volume du fantôme de
    pose, cadre de sélection au sol, zone interdite du navmesh (`FitNavObstacle` : corps + `BodyClearance`, puis l'agent),
    emprise signalée aux chemins coupés de J6b.2 (`FHOLBlockedFootprint::Size`, `DoesWalkCross`, `PushOutOfFootprint`,
    sondes des tuiles refaites), `DoesCrossBuilding`, garde `HOLFootprintKeepOutProcessor`, dépôts (D296 : places et
    arrivée), places des bâtisseurs et « au contact » (`GetBuildSpots`, `IsAtSite`, autres bâtiments : aucune place dans
    l'écart entre deux voisins), distances du chantier voisin et de la source d'un camp (mesurées du bâtiment lui-même),
    naissance au pied du bâtiment, ralliement « sur le bâtiment » (`IsOnBody`), `hol.Nav.Footprints` (contrôle « aucun
    Paysan dans une emprise »), visée au clic (pièces de la forme : le corps).
  - non touché (interdit) : `HOLInterpolatedInstancesProcessor` (orientation du marteau chez un client : boîte des cases,
    même direction) ; en solo il lit `SiteFootprint`, devenu le corps.
- **Mesure de l'écart** (`hol.Nav.Path` entre deux maisons posées côte à côte, puis 10 paires à divers endroits de la
  grille, dans les deux sens, 120 chemins) :
  - **90 cm** (écart 1,8 m) : la bande praticable disparaît (navmesh à 87-102 cm du corps : 85 + la quantification des
    voxels) ; le chemin contourne la paire (1,10) ;
  - **105 cm** (écart 2,1 m) : bande présente (chemin droit 1,00) ; mais **la recherche de Recast contourne souvent la paire**
    (41 chemins sur 120, 6 à 12 % plus longs, même avec la bande : coûts entre milieux d'arêtes de polygones étroits) ;
  - deux Paysans envoyés ensemble passaient **de front**, poussés contre les murs (11 à 22 remises, un nouveau chemin).
- **Corrections du mouvement** (aucune règle de jeu nouvelle) :
  - **à la file dans un passage étroit** : un portail du couloir plus étroit qu'un Paysan (120 cm) ramène chaque unité sur
    le chemin lui-même (place de côté ignorée) ; une unité d'un groupe dont la ligne droite vers sa place touche le bord du
    navmesh suit le couloir du chemin (même droit) près des bâtiments : **0 remise**, l'un derrière l'autre (133 cm) ;
  - **par l'écart** : un chemin plus long que la ligne droite est comparé au chemin par le milieu de chaque écart entre deux
    voisins proche de cette ligne (deux recherches, chemins raboutés, points et polygones ; `hol.Units.ThroughWays 0` pour
    A/B ; `hol.Nav.Path` montre le même) : **41 → 4 contournements sur 120** (les 4 restants à 1,03 : passage par l'écart avec
    un coude). Règle pure `HOLConstruction::FindWays`.
- **Marge retenue : 105 cm** (`BodyInset=105`, `BodyClearance` gardé à 25) : écart de 2,1 m entre deux voisins, toujours
  moins que deux Paysans de front (2,4 m) ; maison à 3,9 m de ses 6 m (65 %), forum à 9,9 m de 12 (83 %). À 90 cm le passage
  se fermait ; 100 et 105 cm avec `BodyClearance=10`, et 105 avec 25, gardent la bande sur les 10 paires mesurées : le choix
  le plus proche de la règle sans toucher la marge de J6a.1. Correctifs du mouvement : « à la file » seulement dans un
  **passage** (deux portails de suite plus étroits qu'un Paysan ; un portail étroit seul est un coin du chemin, où le groupe
  garde ses places : sans cela, un groupe qui contourne une maison se tassait contre le mur).
- **Mesures avec 105 cm** : 10 paires, 120 chemins : tous droits ou par l'écart (4 à 1,03, coude dans l'écart) ; un Paysan :
  droit (1,00), au milieu de l'écart, 45 cm des murs ; deux ensemble : l'un derrière l'autre (~125 cm), **0 remise, 0
  nouveau chemin**.
- Fichiers : `Buildings/HOLBuildingTypes.h/.cpp` (`BodyInset`, `GetBodySize`), `Buildings/HOLConstructionRules.h/.cpp`
  (`GetBodySize`, `GetBody`, `FindWays`), `Buildings/HOLBuilding.h/.cpp`, `Buildings/HOLProductionSubsystem.cpp`,
  `Commands/HOLCommandSubsystem.h`, `Commands/HOLUnitOrderProcessor.h/.cpp` (à la file, propre ligne droite bloquée → couloir,
  chemin par l'écart, `hol.Units.ThroughWays`), `Map/HOLNavReport.cpp` (`hol.Nav.Path` passe par la même recherche),
  `Player/HOLSelectionMarkers.cpp`, `Simulation/HOLBuildFragments.h`, `HOLBuildProcessor.h/.cpp`, `HOLGatherProcessor.cpp`,
  `HOLSimulationProcessors.h/.cpp`, `Tests/HOLBuildingTests.cpp`, `Tests/HOLConstructionTests.cpp`, `Config/DefaultGame.ini`,
  `Tools/Tests/quick_check.ps1`. Aucune carte ni ressource binaire refaite.
- **Compilation** sans avertissement. **Tests 65/65** (nouveau `Construction.Body` : corps d'une maison et du forum, écart
  entre deux voisins, un Paysan passe et pas deux, écarts trouvés par `FindWays` ; `Buildings.Appearance` / `.Foundation` au
  corps).
- **`quick_check` PASS en 306 s** (259 avant ; délai du lancement du jeu porté de 300 à 420 s). Contrôles adaptés : contour du
  forum `move` > 1,06 (1,12 mesuré, 1,2 autour de ses cases avant), chantier A `build site` ≥ 1,08 (1,14), chantier voisin F
  à **1 110 cm** (corps à corps : 900 + 2 × 105), groupe de `build cut move` : **aucun nouveau chemin et au plus 20 remises**
  (9 mesurées, déterministes : autour du corps plus petit de N, le groupe de six tasse son Paysan intérieur contre le mur
  ouest, la garde le glisse le long ; avant 0 ; même effet, moindre, à 0 cm de marge : 6). Partie nouvelle **`build gap`** :
  maisons P et Q aux cases qui se touchent, chemins `hol.Nav.Path` droits (1,00 et 1,00), un Paysan au milieu de l'écart, deux
  l'un derrière l'autre, 0 remise, 0 nouveau chemin, tous arrivés. Captures vues : `build_gap_two` (les deux en file à
  l'entrée de l'écart), `build_gap_houses` (gros plan : deux maisons rentrées dans leurs cases, l'écart, fantôme d'une troisième
  maison collée à Q : cadre des cases et arêtes du corps à l'intérieur).
- **S1 / S2 / S7 courts** hors écran, **machine très chargée** (Overwatch, Opera ouverts ; 18-44 % avant) : S1 2,529 / 2,636 ms
  PASS, S2 3,137 / 3,349 ms **FAIL**, S7 2,694 / 2,469 ms PASS. **A/B** (base 7595c81 compilée dans `HeroesOfLiberty-Elrond`, rendu
  à sa branche ensuite, 10 s, A B A B) : S2 base 3,368 / 3,499 contre 3,455 / 3,227 ; S7 base 2,686 / 2,546 contre 2,499 /
  2,537 : **aucun surcoût** ; l'échec de S2 vient de la charge. **S1 / S2 / S7 au repos à refaire.**
- À juger à l'essai : la taille des bâtiments (65 % des cases pour une maison), l'écart de 2,1 m, le glissement le long d'un
  mur d'un groupe qui contourne une maison, la file dans l'écart. Commandes : `hol.Cheat.Wood 500`, `hol.Cheat.FastBuild 1`,
  poser deux maisons collées (Maj + clic), envoyer un Paysan puis deux de l'autre côté ; `hol.Nav.Path X1 Y1 X2 Y2` ;
  `hol.Units.KeepOut 2` (chaque remise journalisée) ; `hol.Nav.Footprints on` / `off` ; `hol.Units.ThroughWays 0` (A/B du
  chemin par l'écart) ; `hol.Units.LogDetours 1` (« Detour: through the way between two buildings »).
- **Essai de J6b.2b** (2026-10-17) : refusé tel quel : « ça fait de trop grosses allées ». **D324 précisée** (choix de l'utilisateur) :
  la forme visible de **tout bâtiment fait au moins 90 % de ses cases** (marge visible ~30 cm par côté : maison et camps
  90 %, forum 95 %) ; le **corps qui bloque** (navmesh, garde, chemins coupés) reste rentré de 105 cm, et **les Paysans
  passent un peu dans la forme** des bâtiments ; toujours **un par un** entre deux maisons voisines. Boucle suivante
  (15 min) : séparer la marge visible de la marge du corps.
- **Marge visible séparée** (2026-10-17, boucle de 15 min) : `UHOLBuildingSettings::VisualInset` (**30 cm**, `DefaultGame.ini`)
  et `GetVisualSize` à côté de `BodyInset` (**105 cm**, inchangé). À la marge visible : forme provisoire (D261), dalle et
  chantier qui monte, fantôme d'un bâtiment prévu (et sa boîte invisible : la visée au clic suit les pièces), arêtes du
  volume du fantôme de pose, cadre de sélection. Au corps, sans changement : navmesh, garde, chemins coupés, places des
  bâtisseurs et des dépôts, `IsOnBody`, `hol.Nav.Footprints`. Maison et camps 5,4 m de 6 (90 %), forum 11,4 m de 12 (95 %).
  Fichiers : `HOLBuildingTypes.h/.cpp`, `HOLBuilding.cpp`, `HOLSelectionMarkers.cpp`, `DefaultGame.ini`,
  `Tests/HOLBuildingTests.cpp` (forme et fantôme à la taille visible, ≥ 90 %), `Tests/HOLConstructionTests.cpp`
  (`Construction.Body` : marge visible ≥ 90 % des cases, plus large que le corps).
- Compilation sans avertissement ; **tests 65/65**. `quick_check -Parts camera,production,build` hors écran (242 s ; `build`
  seul bloque au départ sur `waitlog Selection: none`, il lui faut `production` avant) : `build`, `build site`, `build trial`,
  `build hud`, `build camp`, **`build gap` PASS** ; **`build cut` FAIL** sur un seul contrôle (« they arrive » : 4 sur 6 à
  moins de 6 m) — sans doute l'état laissé par les parties absentes (`move`, `gather`), le mouvement n'ayant pas changé (les
  pièces ne touchent pas la navmesh) : **à revoir au `quick_check` complet**. Captures vues : `build_gap_houses` (deux
  maisons à 90 % de leurs cases, allée étroite, fantôme de pose aux arêtes à 90 %), `build_gap_two` (les deux Paysans l'un
  derrière l'autre à l'entrée de l'allée). Reste : `quick_check` complet, essai de l'utilisateur.
- **Essai** (2026-10-17) : taille à 90 % **validée** (« très bien pour la taille ») ; demande : laisser les Paysans **entrer un peu
  plus dans les murs, « une épaule »** (le milieu d'un Paysan peut venir jusqu'à la ligne du mur visible). Boucle suivante :
  rapprocher le corps qui bloque, toujours un par un entre deux maisons.
- **« Une épaule »** (2026-10-17, boucle de 15 min) : `BodyInset` **105 → 115 cm** (`DefaultGame.ini` seul, rien recompilé) ;
  `quick_check` : chantier voisin F à **1 130 cm** (900 + 2 × 115), mesure ajoutée **`build wall`** (sans contrôle : un Paysan
  longe le mur sud de P, 20 cm dans ses cases, garde des emprises allumée ; capture `build_wall_one`).
  - **Mesures** (garde des emprises : « closest body gap » = bord du Paysan au corps) : **allée** entre P et Q, avant 44 cm
    (milieu à 104 cm du corps), après 54 cm (114 cm) : dans les deux cas **le milieu suit le milieu de l'allée, à 30 cm de
    chaque mur visible** (le chemin passe au milieu ; seule la bande praticable s'élargit : ~20-40 cm → ~40-60 cm, sous
    120 cm, deux Paysans toujours l'un derrière l'autre, 0 remise) ; **le long d'un mur** après : 57 cm, milieu à 117 cm du
    corps, soit **32 cm devant le mur visible** (avant, même écart au corps : ~42 cm devant).
  - **Ce qui tient encore à distance**, au-delà du corps + `BodyClearance` (25) + agent (60) = 85 cm : l'arrondi aux voxels
    du navmesh (~+2 à +17 cm) et `PathOffsetFromBoundaries = 10` cm (`HOLUnitOrderProcessor.cpp`, coins des chemins) :
    ~117 cm au total. Pour amener le milieu au mur visible en gardant l'allée étroite, le levier n'est pas `BodyInset`
    (à ~145 cm l'allée passerait 2,9 m, deux de front) mais **`BodyClearance` 25 → ~0** (milieu ~7 cm devant le mur, allée
    ~75 cm) et/ou l'offset de 10 cm : à décider (D310 fixe 25 cm).
  - **Tests 65/65** ; `quick_check -Parts camera,production,build` **PASS en 247 s**, dont **`build cut` PASS** (garde : plus
    proche 1 cm, 5 remises, 0 nouveau chemin) et `build gap` PASS (chemins 1,00 / 1,00).
- **Essai validé** (2026-10-17, « c'est bon ») : forme visible à 90 % (`VisualInset=30`), corps qui bloque à 115 cm
  (`BodyInset=115`), `BodyClearance` gardé à 25 : un Paysan mord ~30 cm dans un mur visible, un par un entre deux maisons.
  `quick_check` complet lancé avant le commit de J6b.2b (D324).
- **`quick_check` complet** (2026-10-17, D324 dans l'arbre de travail) : **PASS en 364 s**, toutes les parties PASS, tests 65/65.
- **Mesures au repos après D324 : reportées** (Age of Empires IV ouvert ; règle des mesures : à la fin de J6, J6b.4).

### 2026-10-17 — J6b.3 : vérification

- Sous-agent, première boucle (15 min), rien de commité, aucun code C++ : les verbes existaient
  (`hol.Build.PlaceNear`, `hol.Build.PlaceAtSource`, `hol.Select.Own`, `signal` / `waitfile`).
- **`quick_check`, partie « net » avec « build » : les camps à 2 clients.** Après les maisons, chaque client attend
  l'autre (`quick_net_built*.flag`), marque `camp`, sélectionne son Paysan et pose un **camp de bûcherons** par
  `hol.Build.PlaceNear LumberCamp 1500 -2100` : le L de la partie solo (-13500, -2100) pour l'emplacement 0, (13500,
  2100) en miroir pour l'emplacement 1 (la carte l'est : source 48 à 4,7 m, comme la source 8 de l'autre côté). Le
  serveur bâtit 4 fois plus vite (`hol.Build.Speed 4`) et journalise les dépôts (`hol.Gather.LogDeposits 1`) quand
  « build » est lancé. Chaque client attend son premier « +10 », puis celui de l'autre (`quick_net_camp*.flag`), fait son
  rapport, vide sa sélection et demande un camp **sur l'arbre** le plus proche de L (`hol.Build.PlaceAtSource LumberCamp
  -14230 -1800`, comme un client tricheur).
- Contrôles ajoutés, par client : camp payé 100 bois puis rien pris par le refus ; ses « +10 » à son camp (≤ 10 m) ; le
  camp de l'autre reçu à son premier coup (`Building arrived: LumberCamp of slot N, under construction`), puis les deux
  finis ; pas de camp sur l'arbre ; stock de l'autre jamais reçu (`others [slot N F 0 W 0 S 0 G 0, population 0/0]` à
  chaque rapport de la section). Serveur : un camp payé par emplacement à sa place, fini, `1 builders sent to gather
  Wood`, dépôts `at LumberCamp (x, y)` des deux, chaque camp sur l'arbre refusé (`on a resource source`).
- Essais : `-Parts net,build` seul : FAIL **déjà là avant** (sans « sources », le Paysan est près de sa maison : la
  maison de l'autre arrive pendant le `wait 2`, avant le `waitlog Building arrived`) ; `-Parts net,sources,build` : tous
  les contrôles des camps PASS (91 s), seul KO « 376 sources received » (31900 au lieu de 31710 : sans « move », le
  rapport des sources passe avant le `TakeNear` du serveur, déjà là avant). Camps finis en 125 pas (1 bâtisseur,
  vitesse 4), premier dépôt au camp ~8 s après.
- **`quick_check` complet** (machine libre, Age of Empires IV ouvert) : **FAIL en 324 s**, toutes les parties PASS dont
  **`net 2 clients`** (camps compris) et tests 65/65, **sauf `build cut`** (solo, que ce changement ne touche pas : le
  serveur à part seul a changé) : « no walk into it » KO, 1 nouveau chemin et 10 remises dehors pour l'unité #48
  (`Keep-out: #48 Moving/Path at (-9187, 226) put back`) ; PASS au `quick_check` de D324 : à relancer pour savoir si
  c'est un aléa. Reste de J6b.3 : `build cut` à revoir, S7 avec des camps bâtis en cours de test (journal J6b.1).
- **`build cut` revue** (sous-agent, boucle de 15 min, Age of Empires IV ouvert, rien de commité, aucun C++) : deux
  défauts distincts. (1) **« no walk into it »** du `quick_check` complet : #48 **glisse** le long du mur ouest de N
  (10 pas de suite, 1 à 30 cm par pas, son milieu jamais dedans, 0 corps dans l'emprise) ; à 10 remises de suite la garde
  lui donne un nouveau chemin (`RepathBuildingPushes`, D310) : le seuil « 0 nouveau chemin » contredisait « ≤ 20 remises ».
  Contrôle changé : **0 unité-pas dans l'emprise, ≤ 20 remises, ≤ 1 nouveau chemin**. (2) **« they arrive » 4 sur 6** :
  reproduit (`-Parts camera,production,build`, 1 sur 2) ; **ce n'est pas le temps** : #25 et #30 restent `Moving` **16 s**
  à (-9238, 285) et (-9358, 287), au **coin nord-ouest** de N (mur nord y = 195), sans remise ni nouveau chemin (garde :
  plus proche 8 cm, 0 remise) : **blocage au coin, défaut du jeu**, cause à trouver. Script : attente finale 10 → 14 s
  (marge de l'horloge, ~16,5 s de marche) et `hol.Units.Orders 0` en fin d'étape (phase, place, `stuck` de chacun) pour le
  prochain échec. Deuxième passe (contrôles nouveaux) : PASS (258 s ; 5 remises, 0 nouveau chemin).
- **Blocage au coin : cause et correctif** (sous-agent, boucle de 15 min, Age of Empires IV ouvert, **non commité**) :
  reproduit au 1er lancement, **mêmes positions au cm près** ; `hol.Units.Orders 0` : #25 et #30 en phase **`Path`**, à
  42-43 m de leur place, `stuck 0`. Cause : Mass avance la cible du chemin tant que l'unité suit ; poussées en arrière au
  coin nord-ouest (le groupe tourne autour de N, voisin qui presse contre le bord), elles visent une cible déjà passée au
  coin, droit dans le bord du navmesh, et restent là ; le seul détecteur de blocage (`StuckSteps`) ne compte qu'en fin de
  trajet, à moins de 5 m de la place. **Correctif** : en phase `Path`, une unité restée à moins de 50 cm d'un point pendant
  60 pas (3 s) reçoit un nouveau chemin d'où elle est (`PathStuckSteps`, `PathStuckDistance`, `FHOLMoveOrderFragment::PathAnchor`,
  compteur `TotalStuckRepaths`, ligne « Detour: held on its path » sous `hol.Units.LogDetours 1`) ; fichiers
  `Commands/HOLUnitOrderProcessor.h/.cpp`, `Simulation/HOLSimulationFragments.h`. Compilation sans avertissement ;
  `quick_check -Parts tests,camera,production,build` **PASS en 257 s**, tests 65/65, `build cut` : les 6 arrivés (#30 à
  20 cm), 0 remise, 0 nouveau chemin de la garde. Non prouvé que le correctif a joué (journal des détours éteint). Reste :
  relancer `build cut` quelques fois, S2 / S7 courts (suivi des chemins touché), essai, commit.
- **Vérification du correctif** (sous-agent, boucle de 15 min, Age of Empires IV ouvert, non commité) : `hol.Units.LogDetours 1`
  pendant l'étape `build cut move` (remis à 0 en fin d'étape) ; rapport du stress test : nouvelle ligne « Held on their
  path (J6b.3) » (`GetTotalStuckRepaths`, fichiers `StressTest/HOLStressTestSubsystem.h/.cpp`), compilé sans avertissement.
  `quick_check -Parts camera,production,build` ×2 : 1er PASS (259 s), 0 ligne « held » (pas de blocage au coin ce
  coup-ci) ; 2e FAIL (260 s), `build cut move` bon (0 ligne « held ») mais KO « none put back by a footprint » de la
  maison K (partie cueillette, avant `build cut move` : 1 remise sur 236 pas, 0 nouveau chemin), aléa à revoir. S2 court
  (10 s, 2000 unités, hors écran) : **0** nouveau chemin « held » pendant la mesure, 0 ligne dans tout le journal : pas
  d'emballement dans la foule. Le correctif n'a pas encore été vu jouer en `quick_check`.
- **`build cut` : maison K alignée sur N** (sous-agent, boucle de 15 min, machine libre, non commité, aucun C++) : le
  contrôle « none put back by a footprint » de K (0 remise, 0 nouveau chemin) devient **« none in it, a few put back, a
  new path at most »** : 0 unité-pas dans l'emprise, ≤ 20 remises, ≤ 1 nouveau chemin de la garde (même règle que
  `build cut move`) ; gardés : ≥ 200 pas, K donne des nouveaux chemins et oublie des chemins gardés, ≥ 3 dépôts après.
  **`quick_check` complet PASS en 350 s**, toutes les parties PASS, tests 65/65. K : 239 pas, 0 remise, 0 nouveau chemin
  (6 marcheurs, 4 chemins gardés oubliés) ; N : 417 pas, 0 dans l'emprise, **10 remises et 1 nouveau chemin** (#48 glisse
  le long du mur ouest, comme au `quick_check` précédent, désormais admis) ; **0 ligne « held on its path »**.
- **S7 bâtit ses camps pendant le test (D314)** (non commité) : `hol.StressTest.S7BuildCamps` (défaut 1 ; 0 = ancien S7
  aux camps finis) ; les 3 camps de chaque joueur posés en fondations (`SnapFootprint` + `CheckPlacement`, D324), 3
  bâtisseurs par camp (`IssueBuildOrder`, `GatheringBuildersPerCamp`), `WatchCampSites` (« S7: camp of player N finished
  after X s »), ligne de rapport « Camps built (D314) » ; fichiers `StressTest/HOLStressTestSubsystem.h/.cpp`. Correctif
  `AHOLMapBounds::Find` (`Map/HOLMapBounds.cpp`) qui plantait sur la carte `StressTest` sans bornes.
- **S7 court** (sous-agent, machine libre, `stress_ab.ps1 -Test S7 -Seconds 40`, hors écran, journal `j6b3_s7build`) :
  sortie code 0 en 123 s. **24 camps posés sur 24** (aucun « not placed »), 72 bâtisseurs, 1928 Paysans aux arbres ;
  premier coup à ~6 s, **24 sur 24 finis entre 20,2 et 23,0 s** (≈ 255-280 pas de travail, vitesse 1), avant la mesure ;
  chaque camp fini envoie ses 3 bâtisseurs récolter (J6b.1). Depuis le début, les nouvelles emprises ont donné un nouveau
  chemin à 661 marcheurs et oublié 2 chemins gardés ; pendant la mesure : 0 nouvelle emprise, **0 corps remis hors d'une
  emprise**, 0 « held on their path », 0 Paysan sans récolte, 0 point manqué. Bois : 44 790 déchargés pendant la mesure
  (4479 dépôts). Aucun avertissement ni erreur du jeu. Bruit à la fermeture (déjà là avant D314 : `StopTest` retire les
  dépôts avant que la destruction des unités, différée d'un pas, soit appliquée) : 1664 lignes « slot N has no deposit
  for Wood, its gatherer stands with its load » après le rapport, sans effet.
- **Camps bâtis pendant la mesure, fermeture sans bruit** (sous-agent, boucle de 15 min, machine libre, non commité) :
  `OrderGathering` prépare les ordres de bâtir (`PendingBuildOrders`) mais envoie tout le monde récolter ; à la première
  étape de la mesure, `OrderCampBuilding` les donne (« 72 Peasants leave their trees to build 24 camps, at 25.0 s »).
  `WatchCampSites` note aussi le premier coup ; ligne de rapport « Camps built » : fenêtre de la mesure, premiers coups et
  fins. `StopTest` ne retire plus les dépôts ni les arbres tout de suite (`bTearDownPending`) : à la tête d'une étape,
  une fois les unités détruites (au plus 12 pas, et avant de quitter). `S7BuildCamps 0` inchangé. Fichiers
  `StressTest/HOLStressTestSubsystem.h/.cpp`, compilé sans avertissement.
- **S7 court** (`stress_ab.ps1 -Test S7 -Seconds 40`, hors écran, journal `S7_campsMeasure`) : code 0 en 82 s ; mesure de
  25 à 65 s ; **24 camps sur 24 commencés entre 27,1 et 31,1 s, finis entre 39,8 et 44,0 s**, tout dans la mesure.
  Pendant la mesure : nouvelles emprises J6b.2 **244 nouveaux chemins, 157 chemins gardés oubliés** (depuis le début :
  407 et 157, le reste vient des centres posés au préchauffage) ; 54 corps remis hors d'une emprise (garde D310), 0
  nouveau chemin de la garde, 0 « held on their path » ; bois 42 780 (4278 dépôts), 0 point manqué, 0 Paysan sans
  récolte. **0 ligne « has no deposit »** dans tout le journal ; après le rapport, seulement « Units left after the run:
  0 ». Erreurs : les 18 `LogAutomationTest` du démarrage du moteur, déjà dans le journal `j6b3_s7build`.
- **`quick_check` complet sur le code final** (sous-agent, 2026-10-18, rien de commité) : compilation à jour sans
  avertissement, **PASS en 346 s**, toutes les parties PASS (dont `build camp`, `build cut`, `build gap`, `net 2 clients`),
  tests 65/65.
- **Mesures au repos de fin de J6** (sous-agent, 2026-10-18, machine au repos, mêmes commandes que J6a.6, journaux
  `Saved_Tests\Logs\j6b3_s1.log`, `j6b3_s2.log`, `j6b3_s7.log`, `j6b3_s7old.log`) :
  - **S1** 2,265 ms PASS (2,276) ; **S2** 2,802 ms PASS (2,882, marge 0,20) ; **S7 ancien** (`S7BuildCamps 0`) 2,286 ms
    PASS (2,303 ; 1 172 bois/s contre 1 198) ; **S5 court** (serveur recuit sur le code actuel) PASS à 31 333 o/s par
    client (30 937, marge 0,67 Ko/s), écart affiché 22,9 cm (23,3).
  - **S7 avec camps bâtis pendant la mesure : 3,411 ms FAIL** (seuil 3) : **`HOLBuildProcessor` 1,016 ms par pas**
    (0,003) ; ordres 0,307 (0,144), évitement en marche 0,573 (0,286). 24 camps commencés de 27,1 à 31,1 s, finis de
    39,8 à 44,0 s ; 46 corps remis hors d'une emprise, 0 « held », 992 bois/s, 0 raté, 0 inactif ; GPU 3,33 ms.
  - Seuil dépassé : J6 bloquée. Correctif de `HOLBuildProcessor` confié à un sous-agent (pistes : montée du bâtiment à
    chaque pas, recherche des places, parcours des bâtiments).
- **Correctif du coût des chantiers** (sous-agent, 2026-10-18, non commité) : chronos temporaires sur 200 pas avec
  bâtisseurs : parcours des bâtiments 0,023 ms, places tenues 0,026, `Advance` 0,043 (`FindSpot` 0,001), **`ApplyWork`
  0,946**, dont **`SetConstructionWork` 0,661** (~27 µs par chantier et par pas : `UpdateRise` →
  `RiseRoot->SetRelativeScale3D` déplace toutes les pièces) et ~0,26 de premiers coups (`SetConstructionState` →
  `RefreshAppearance`, ~2 ms une fois par chantier). Correctif : `UpdateRise` ne remet à l'échelle que si la montée a
  grandi d'au moins 2 % (`AHOLBuilding::RiseStepRatio`, ~4 fois par seconde), si le bâtiment est entier ou si la
  hauteur voulue est plus basse que celle affichée ; simulation, réplication et délégués inchangés. **S7 avec camps :
  3,437 → 2,783 ms PASS**, `HOLBuildProcessor` 1,014 → 0,467 ms, 206 → 228 i/s ; « Camps built » identique. Reste :
  `quick_check -Parts tests,camera,production,build`, deux S7 (bruit des pas de fin), puis l'essai de l'utilisateur.
- **Vérification du correctif** (sous-agent, 2026-10-18, machine au repos, aucun code) : `quick_check -Parts
  tests,camera,production,build` **PASS en 234 s**, tests 65/65, captures des chantiers qui montent toujours
  (`build_site`, `build_three`). ⚠️ `quick_check.ps1 -Parts a,b` par `-File` ne passe qu'une chaîne : lancer par
  `powershell -Command "& .\Tools\Tests\quick_check.ps1 -Parts ..."`. **Deux S7 avec camps : 2,908 ms PASS et 3,038 ms
  FAIL** (journaux `j6b3_s7fix1`, `j6b3_s7fix2`) ; le 2,783 venait d'une compilation antérieure du correctif (même jeu) ;
  `HOLBuildProcessor` 0,49-0,51 ms, évitement à l'arrêt 4,6-4,9 ; « Camps built » identique ; lignes « finished ... after
  N steps » identiques à quelques pas près sur 3 camps du slot 0 (ordre d'arrivée des bâtisseurs : bruit). **Trop juste** :
  nouvelle boucle sur le `ForceNetUpdate` à chaque pas et la reconstruction des pièces au premier coup (~2 ms par chantier).
- **Deuxième correctif** (sous-agent, 2026-10-18, ~30 min, au-delà de la boucle) : chronos : `ForceNetUpdate`
  négligeable (~0,004 ms par pas) ; montée ~0,20 ms par pas (~68 µs par cran : pièces déplacées, corps de collision,
  chevauchements) ; premier coup 1,9-3,1 ms par chantier et fin 0,9-2,5 ms (création de ~10 pièces). Pièces en
  `QueryOnly`, sans événements de chevauchement : montée 0,20 → 0,11 ms ; **S7 avec camps 3,060 et 3,016 ms FAIL**
  (`j6b3_s7fixB1/B2`). Le FAIL de `net 2 clients` de cette boucle venait de `-Parts tests,build,net` sans « sources »
  (défaut connu du script, noté plus haut), pas du correctif.
- **D325 choisie** par l'utilisateur (apparence des bâtiments hors du pas de simulation) et **codée** (sous-agent,
  ~11 min, non commité) : `RefreshNavObstacle` (zone interdite, immédiate), `RefreshVisuals` (pièces, montée, couleurs),
  `MarkAppearanceDirty` (drapeau + tick de l'acteur activé, coupé une fois appliqué : au plus une reconstruction par
  image et par bâtiment) ; `BeginPlay` reste immédiat ; **serveur dédié : aucune pièce**. Tests
  (`HOLBuildingTests.cpp`) : `ApplyPendingAppearance()` après chaque changement. Compilé sans avertissement, tests 65/65.
  ⚠️ `quick_check -Parts ...` par `-Command` n'a lancé que « sequence » et « tests » : liste de parties mal transmise
  aussi ; lancer `quick_check` complet. Reste : `quick_check` complet, deux S7, essai.
- **Vérification de D325** (sous-agent, 2026-10-18, aucun code) : **`quick_check` complet PASS en 311 s**, les 20
  parties PASS, tests 65/65 ; `hol.Buildings.Report` n'est lu que chez les clients (le serveur dédié sans pièces ne casse
  aucun contrôle) ; captures des chantiers inchangées. **Deux S7 avec camps : 2,777 et 2,756 ms PASS** (`j6b3_s7d325a/b` ;
  3,06 / 3,02 avant), `HOLBuildProcessor` 0,13 ms (0,44), 195 et 203 i/s (seuil 144 ; ~210 avant, la 1re course sur une
  machine un peu chargée), GPU 3,37 ms, « Camps built » identique. À suivre à la mesure de fin de chapitre : le rendu.
- **Outil de mesure de fin de chapitre** (2026-10-18, demande de l'utilisateur : « ultra rapide ») :
  `Tools/Tests/chapter_measures.ps1`, lancé par la session principale en tâche de fond, sans sous-agent : S1, S2, S7
  deux fois en alternance, une 3e course si les deux ne s'accordent pas (la moyenne des trois décide), puis S5 une fois
  (serveur recuit) ; verdict par test et résumé dans `Saved_Tests\Logs\chapter_measures.txt` ; ~5 min sans S5, ~9 avec.
  Règle des 2 courses proposée à l'utilisateur, en attente de son accord.
- Session d'essai de J6b lancée (deuxième écran).
- **Mesures de fin de J6 par `chapter_measures.ps1`** (2026-10-18, 1er usage, session principale en tâche de fond,
  machine au repos, 9-18 % avant chaque course) : stress tests **4,9 min** (3e course de S2 comprise), S5 **2,5 min**
  (recuisson incrémentale comprise ; 1er lancement de S5 tombé sur un défaut du script, `-Stage` passé dans un tableau,
  corrigé par un `@{}`) : **7,4 min en tout**. **S1 PASS** 2,362 / 2,381 ms, 240 i/s ; **S2 PASS à la moyenne de 3**
  2,816 / 3,024 / 3,145 → 2,995 ms (S2 ne touche aucun bâtiment : bruit ±0,15 ms, marge déjà mince en J6a.6 ; ⚠️ à
  regagner avant la formation de J7) ; **S7 avec camps PASS** 2,450 / 2,748 ms, 216 i/s ; **S5 PASS** 31 005 o/s par
  client, écart affiché 22,9 cm. Journaux `chapter_j6end_*`, résumé `Saved_Tests\Logs\chapter_measures.txt`.
- Session d'essai de J6b relancée (deuxième écran).

### 2026-10-18 — J6b.4 : clôture

- Session principale, documentation seule, aucun code. GDD § 21 : **D310** (périmètre, précisé par J6a.1 : la piste de
  la zone interdite était fausse ; zone + 25 cm, visée jusqu'au point suivant, garde des emprises), **D311** (pose,
  précisée : case de 3 m, forum 12 m, maison et camps 6 m, coûts, fondation fantôme du seul propriétaire), **D312**
  (bâtir, précisée : N^0,63, camp → récolte à 25 m, chantier voisin à 20 m, ralliement sur un chantier ou un camp fini,
  Maj + source, FastBuild), **D313** (HUD, précisée : case sans coût, F1 / F2 / F3 = tous les bâtiments de la
  catégorie, sous-groupes et Tab, répartition équitable), **D314** (vérification, S7 aux camps bâtis pendant la mesure,
  règle des mesures de fin de chapitre), **D324** (deux marges : forme visible 30 cm, corps 115 cm), **D325** (apparence
  hors du pas). § 16.6 : paragraphe « Parcours J6 : la construction » (mesures de fin de J6). § 7.3 : F1 / F2 / F3 et Tab
  précisés par D313. § 22 : D310 à D314, D324, D325 pris par la clôture de J6 ; numéro suivant **D326**.
- **J6 close.** Reprise : **plan de J7**, à relire avec l'utilisateur ; S2 au seuil à lui proposer avant J7a.4.

### 2026-10-18 — Plan de J7 relu ; J7a.1, 1re boucle : règles de la formation

- Relecture du plan de J7 avec l'utilisateur : J7a validée ; **forme des rangs** précisée par 9 scénarios dessinés
  (J7-Q3 : Paysans à part, carré pour un seul type, lignes par priorité pour plusieurs types, formule retenue) ; le côté
  du soldat en plus sur une aile impaire est sans importance (gauche gardé). Animations des troupes ajoutées au
  catalogue de `AnimationsInProgress.md` (bf294a7).
- Sous-agent (coupé par la fin de la session précédente, repris) : `Simulation/HOLFormationRules.h/.cpp` (namespace
  `HOLFormation` : `MakePeasantRows`, `MakeSquareRows`, `MakeMixedRows`, `MakeRowsOfWidth`, `PlaceKinds`, `MakePlaces`
  — point visé au milieu du premier rang —, `AssignPlaces` — ordre latéral puis échanges qui raccourcissent ou
  décroisent), `Tests/HOLFormationTests.cpp` (`Formation.Rows`, `.Kinds`, `.Places`, `.Assign` : 40 tirages jusqu'à 60
  unités, aucun croisement). **9 scénarios sur 9** ; compilation sans avertissement ; **tests 69/69** (`quick_check
  -Parts tests` PASS en 27 s). Non commité.
- Reste pour J7a.4 : branchement dans l'ordre de groupe (Paysans compris, places de récolte et de chantier inchangées),
  espacements, place dans un obstacle, glisser du clic droit, vitesse du plus lent, reprise de place ; coût
  d'`AssignPlaces` (paires) à voir aux mesures de fin de J7.
- **2e boucle** (sous-agent, ~5 min) : table des unités, enum `EHOLArmorCategory` (LightInfantry, HeavyInfantry, Ranged,
  Cavalry, Siege, Peasant) et colonnes `ArmorCategory`, `Attack`, `AttackInterval` (s), `AttackRange` (cm, bord à bord),
  `MeleeArmor`, `RangedArmor`, `BonusCategory`, `BonusDamage`, `AggressionRadius` (cm, 0 = seulement sur ordre), **non
  lues** par la simulation (pas dans `FHOLUnitTypeConstants`). Valeurs indicatives (AoE4) : **Lancier** 80 PV, attaque
  8 toutes les 1,875 s, +17 contre la cavalerie, armures 0 / 0, 60 nourriture + 20 bois, 15 s ; **Homme d'armes** 155 PV,
  attaque 8 / 1,375 s, armures 4 / 4, 100 nourriture + 20 or, 22 s, 2,7 m/s ; **Paysan** attaque 3 / 2 s, aucune
  agression ; types des stress tests remplis (Archer : portée 15 m, +5 contre l'infanterie légère). **Caserne**
  (`Barracks`) : militaire, 3 × 3 cases, 6 m, 1 500 PV, 150 bois, 40 s, produit Lancier et Homme d'armes, file de 5 ;
  `BuildGridSlot` **12** (hors de la grille actuelle ; J7a.2 : page = case / 12, case = case % 12, donc A de la 2e page).
  Tables refaites par commandlet ; tests `Production.Tables`, `Construction.Tables`, `CommandGrid.Tables` étendus ;
  compilation sans avertissement ; **tests 69/69**. Commitée (rien de visible en jeu).

### 2026-10-18 — J7a.2 : la Caserne

- Choix de l'utilisateur : **pas de page militaire** dans la grille du Paysan ; pages **par palier** (âges d'AoE4) plus
  tard ; la Caserne sur **R** (`BuildGridSlot` 3).
- **1re boucle** (sous-agent, ~14 min, non commitée) : `BuildingTypes.csv` (R), table refaite ; grille du Paysan A / Z /
  E / R ; Caserne finie : **A Lancier, Z Homme d'armes** (index dans `Produces`, comme le centre), inachevée : seulement
  « Annuler » ; infobulle « Produit : Lancier, Homme d'armes » (`ProducesEffect`) ; textes fr / en / ja (Caserne /
  Barracks / 兵舎, Lancier / Spearman / 槍兵, Homme d'armes / Man-at-Arms / 重装歩兵 ; 157 textes) ; **correctif** :
  un soldat né avec un ralliement sur un chantier ou une source n'y bâtit ni n'y récolte, il marche au point (seuls
  les travailleurs, `CarryCapacity > 0`). Pose, chantier, file, naissance, ralliement et F1 : génériques depuis J6,
  vérifiés à la lecture. Compilation sans avertissement, **tests 69/69** ; `quick_check -Parts tests` FAIL sur
  « sequence » : jeu bloqué à la fermeture (nettoyage du cache de données dérivées), à confirmer.
- 2e boucle lancée : ordre d'un soldat sur une source ou un chantier côté serveur, parties `build barracks` (solo) et
  2 clients de `quick_check`, `quick_check` complet.
- **2e boucle** (sous-agent, non commitée) : un soldat envoyé sur une source ou un chantier était **refusé** par le
  serveur (rien ne se passait) → `SendNonWorkersTo` (`HOLPlayerController.cpp`) : les unités qui ne travaillent pas
  marchent vers la source ou le chantier (`Order of slot N: K units that do not work walk to (x, y)`). Partie
  **`build barracks`** solo de `quick_check` (R, pose, bâtie, grille A Lancier / Z Homme d'armes, ralliement, les deux
  nés et au ralliement, soldats sur un filon : marche sans récolte, F1 ; 5 captures) ; `build hud` revu pour R ; délai
  du jeu 420 → 480 s. `quick_check` complet : 1re course FAIL (ralliement à 24 cm du point attendu), 2e FAIL (soldats
  laissés près du filon de la partie `anims`), corrigés ; **3e PASS en 347 s**, toutes les parties, tests 69/69 ;
  blocage à la fermeture de la 1re boucle **non revu**. Reste : partie 2 clients de la Caserne. Caserne dessinée en
  bloc à la couleur du joueur (pas de forme propre).
- Session d'essai lancée (deuxième écran) : **essai de J7a.2**.
- **Essai de J7a.2** : « troupes extrêmement lentes ». **Vitesses de la fiche des unités** (règle de l'utilisateur) :
  **Paysan 1,41** (sa vitesse actuelle, 600 cm/s, D290), **Lancier 1,37**, **Homme d'armes 1,23** ; échelle : 1 = 600 /
  1,41 ≈ 425,5 cm/s. Table des unités (cm/s) : Lancier 300 → **583**, Homme d'armes 270 → **523** ; cadence de la marche
  à proportion (1,22 → 2,37 ; 1,1 → 2,13 ; pied qui ne glisse pas, D234). Table refaite (commandlet, 0 erreur), tests 69/69 ; session d'essai relancée.
- **Règle des vitesses** (utilisateur, 2026-10-18, permanente) : pour chaque nouveau type d'unité (Archer en J7c,
  cavalerie, héros…), **demander à l'utilisateur sa vitesse de référence** et appliquer **× 1,098214** (1,29 → ~1,41) ;
  ne jamais deviner une vitesse.
- **Essai validé** (« c'est bon ») avec les vitesses de la fiche ; commitée. Reprise : **J7a.3**.

### 2026-10-18 — J7a.3 : apparence des soldats

- Worktree `HeroesOfLiberty-J5c` cédé par mycrusader-e3 (rien en cours, conseils sur la chaîne relayés au sous-agent).
- Choix de l'utilisateur : **un maillage par type** (`SKM_Spearman`, `SKM_ManAtArms`, plus tard `SKM_Archer` ; armes
  toujours visibles, sans repli ni choix par instance) ; **lance droite, main droite** (verticale, talon près du sol au
  repos, pointe en l'air, droite en marche).
- 1re boucle lancée (sous-agent, worktree) : armes par script (versions légères au LOD 3), repos et marche armés des
  deux soldats, aperçus à montrer à l'utilisateur avant toute fabrication dans Unreal.
- **1re boucle faite** (sous-agent, worktree à c49417e, rien de commité, rien fabriqué dans Unreal) :
  `Tools/Editor/soldier_props.py` (lance 240 cm, fanion à la couleur du joueur, sur `ik_hand_gun` ; épée 92 cm ; bouclier
  60 × 46 cm sur `hand_l`, face à la couleur du joueur ; versions légères au LOD 3), `soldier_poses.py` (repos et marche
  armés des deux soldats ; bases `A_SW_Idle` et `A_SW_Unarmed_Walk`), `anim_rig.py` / `peasant_props.py` /
  `preview_animation.py` étendus sans changer les gestes du Paysan ; aperçus vérifiés (mains à 0 cm, rien dans le corps).
  Aperçus soumis à l'utilisateur (`Saved/AnimPreview/soldiers_compare.png` du worktree). Reste : fabrication dans
  Unreal, `SKM_Spearman` / `SKM_ManAtArms` (armes toujours visibles : masque à trancher dans le matériau), fournisseurs,
  table des unités, `quick_check -Parts camera,anims`, essai.
- Retouche de l'utilisateur : **épée droite vers le haut au repos, à 45° vers l'avant en marche** ; faite en aperçu
  (repos : pointe à 183 cm ; marche : 75 cm devant, 154 cm de haut ; 26-31 cm du bouclier et de la tête ; plat de la
  lame tourné vers l'avant, ajout du sous-agent). L'utilisateur fera la revue ; **suite à la prochaine séance**.
- Revue de la nouvelle planche par l'utilisateur : **la lance n'était pas à 45° en marche** (restée droite). Corrigé
  en aperçu (`soldier_poses.py` du worktree, `SPEAR_WALK_GRIP` : direction 45° vers l'avant) : pointe 113 cm devant à
  208 cm de haut, talon ~45 cm du sol derrière, main à 0 cm du manche, rien dans le corps ; épée inchangée (45° en
  marche). Aperçus refaits (`soldiers_compare.png`, `spear_walk_*`). Attente du nouvel avis de l'utilisateur.
- **Aperçus validés** par l'utilisateur (« très bien ») : lance droite au repos et à 45° en marche, épée droite au repos
  et à 45° en marche (plat de la lame vers l'avant gardé), fanion et bouclier tels quels. 2e boucle lancée (sous-agent,
  worktree) : fabrication dans Unreal des 4 séquences et de `SKM_Spearman` / `SKM_ManAtArms`.
- **2e boucle faite** (sous-agent, worktree, rien de commité) : `make_peasant_animations.py --soldiers` (les
  4 gestes de `soldier_poses.GESTURES`, un lancement de ~5 s) : **`A_Spearman_Idle`, `A_ManAtArms_Idle`** (11,333 s,
  340 images) et **`A_Spearman_Walk`, `A_ManAtArms_Walk`** (1 s, 30 images) créées ; relecture **écart ≤ 0,03 cm**
  (brute et compressée), racine à 0, boucle fermée, `ik_hand_gun` à 0,008 cm / 0,075°, pieds = source à 0,001 cm ;
  pied d'appui de la marche 168,2 cm/s (Lancier 583 → **3,47**, Homme d'armes 523 → **3,11**).
- `make_peasant_mesh.py --soldiers` : matériau **`M_UnitPlayerColorWeapons`** (celui du Paysan **sans repli** : pas de
  WPO, rien lu en donnée 3 ; alpha 0 = couleur du joueur, alpha > 0 = couleur du sommet) ; fanion et face du bouclier
  en alpha 0 (couleur du joueur), le reste de chaque arme en alpha = son bit. **`SKM_Spearman`** (sommets par LOD
  23 465 / 7 093 / 4 126 / 2 359) et **`SKM_ManAtArms`** (23 587 / 7 215 / 4 248 / 2 433), pièces légères au LOD 3 ;
  **relecture des 4 LOD OK** pour les deux (sommets par masque, os, masque 0 = corps + couleur du joueur).
  **`TP_Spearman`, `TP_ManAtArms`** : 0-7 de `TP_Mannequin_Classic`, puis 8 repos armé, 9 marche armée. 0 erreur.
- Reste (prochaine boucle) : table des unités (`SkinnedMesh`, `AnimationProvider`, `IdleSequence` 8, `WalkSequence` 9,
  `WalkPlayRate` 3,47 / 3,11), test `HOLAnimationTests` (« keeps the mannequin's mesh » à changer), tests,
  `quick_check -Parts camera,anims`, essai.
- Choix de l'utilisateur : fournisseurs des soldats à **4 séquences** (0 repos, 1 marche, 2 repos armé, 3 marche armée),
  sans les gestes du Paysan. 3e boucle lancée (sous-agent) : fournisseurs refaits, branchement dans la table, test,
  `quick_check -Parts camera,anims`.
- **3e boucle faite** (sous-agent, worktree, rien de commité) : `make_peasant_mesh.py` (`SOLDIER_BASE_SEQUENCES` = 2,
  `--soldiers --provider-only` refait les fournisseurs seuls, relecture après sauvegarde) : **`TP_Spearman`,
  `TP_ManAtArms` à 4 séquences** (0 `A_SW_Idle`, 1 `A_SW_Walk_F`, 2 repos armé, 3 marche armée), 0 erreur. Table des
  unités : Lancier et Homme d'armes sur leur `SKM_` et leur `TP_`, repos 2, marche 3, cadence **3,47** / **3,11** ;
  récolte, portage, repos chargé et chantier restent à -1, pièces à 0 (cohérent pour des soldats). Table refaite
  (commandlet) : la 1re fois avec d'anciens binaires du worktree (9 colonnes de combat ignorées, test
  `Production.Tables` en échec) ; refaite après la compilation : **0 problème**. `HOLAnimationTests` : Lancier et
  Homme d'armes (maillage, repos 2 / marche 3, 4 séquences nommées, fournisseur sur son maillage) ; test de l'Archer
  gardé. Compilation sans avertissement (31 s), **tests 69/69**, **`quick_check -Parts tests,camera,anims` PASS en
  35 s**. La partie `anims` ne montre **que des Paysans** : pour voir les soldats, il faudrait y faire naître 3 Lanciers
  et 3 Hommes d'armes (`hol.Units.Spawn … Spearman`), une capture au repos, un ordre de marche, et attendre dans
  `hol.Units.AnimReport` `A_Spearman_Idle` / `_Walk` et `A_ManAtArms_Idle` / `_Walk`. Reste : cette partie, essai.
- Session d'essai lancée depuis le worktree (deuxième écran) : **essai de J7a.3** (Caserne, Lanciers et Hommes d'armes :
  repos, marche, armes, couleur du joueur sur le fanion et le bouclier). Ensuite : soldats dans la partie `anims` de
  `quick_check`, commit sur `j5c-anims`, avance rapide de `main`.
- **Essai validé** par l'utilisateur (« c'est bon ») ; J7a.3 commitée sur `j5c-anims` et reportée sur `main`. Reste :
  soldats dans la partie `anims` de `quick_check` (boucle suivante), puis **J7a.4** (formation).

- **4e boucle faite** (sous-agent, worktree, rien de commité) : soldats dans la partie `anims` de `quick_check.ps1`.
  3 Lanciers en (-7800, 3000), 3 Hommes d'armes en (-7800, 4000), capture `anims_soldiers_idle` ; sélection par un
  cadre autour des deux groupes, envoyés 12 m à l'est (-6600, 3500), capture `anims_soldiers_walk`. Vérifications : repos
  armé (« working »), marche armée (« walking »), repos armé (« standing »), 2 sur 3 au moins ; états des soldats admis
  dans « every Peasant drawn ». Vérifications existantes inchangées. **`quick_check -Parts tests,camera,anims` PASS en
  37 s** : **3 sur 3** à chaque moment pour les deux types. Captures vues : lance et fanion, épée et bouclier, bleu du
  joueur. **`quick_check` complet FAIL (387 s)** : délai dépassé à `waitlog Selection: 3 units (own): Peasant x3` (groupe
  du bois, au début d'`anims`) : le cadre a pris **4 Paysans**, dont un resté de `build camp` (près de L et de son
  arbre). Pas dû aux soldats ; les échecs `camera 8p`, `move`, `anims` en découlent. Reste : écarter ce Paysan (ou le
  groupe du bois), refaire le `quick_check` complet, commit.
- **5e boucle faite** (sous-agent, worktree, rien de commité) : au début d'`anims`, une étape `clear wood` (avant
  `mark anims`) sélectionne par un cadre le bord du bois de l'emplacement 0 et le camp L (x -15600 à -12400, y -3000 à
  -700) et envoie ce qu'il contient au point lointain de la coupe (`CutTo`, -9000, -4000), puis attend 6 s. Elle a pris
  **4 Paysans** (`gather`, `build camp`). Vérifications inchangées. **`quick_check` complet PASS en 348 s** (toutes
  les parties, soldats compris, tests 69/69). Reste : commit sur `j5c-anims`, avance rapide de `main`, puis J7a.4.

### 2026-10-18 — J7a.4 : la formation

- Choix de l'utilisateur : **S2 gardé pour la fin de J7** (règle des mesures), pas de marge regagnée avant J7a.4.
- **1re boucle** lancée (sous-agent, worktree `HeroesOfLiberty-J5c`) : `HOLFormation` branché dans l'ordre de déplacement au
  sol d'un groupe (Paysans compris ; places de récolte et de chantier inchangées), rangs orientés selon la marche,
  espacements, place dans un obstacle.

- **1re boucle faite** (sous-agent, worktree, rien de commité). Un ordre au sol à **plusieurs unités** (hors marche
  d'une tâche) prend les places de `HOLFormation` au lieu du tas serré de D287 ; une seule unité garde D287 ; les
  places de récolte (D309) et de chantier (D312) ne changent pas (ce sont des marches d'une unité). Nouveau :
  `HOLFormation::MakeGroupRows`, `PlaceGroup` (rangs, types, places, projection, `AssignPlaces` par type) ;
  `GetFormationPriority`, `ProjectFormationPlace` dans `HOLUnitOrderProcessor.cpp`.
- Choix : rangs orientés du centre du groupe vers le point, point au milieu du premier rang. Espacements : **150 cm**
  entre colonnes (1,25 diamètre), **180 cm** entre rangs (1,5 diamètre), rayon 60 cm de la table. Groupe mixte : les
  Paysans sont un type de plus, **derrière les soldats** (règle des lignes). Place dans un obstacle ou hors navmesh :
  projetée comme D287 ; sinon ramenée vers le point (3/4, 1/2, 1/4), sinon le point lui-même. Les unités à l'arrêt
  sur une place ne la font pas sauter (bousculade).
- Test `Formation.Group` ajouté. Compilation sans avertissement ; **tests 70/70** ; **`quick_check -Parts
  tests,camera,move,anims` PASS** (43 s ; `move` inchangée : 3 Paysans à moins de 3 m du point, sans chevauchement).
- Reste : vitesse du plus lent, glisser du clic droit, reprise de place ; essai de l'utilisateur.
- **2e boucle** lancée (sous-agent, worktree `HeroesOfLiberty-J5c`) : vitesse du plus lent tant que le groupe est en
  formation, reprise de place après bousculade. Le glisser du clic droit suivra (3e boucle).
- **2e boucle faite** (sous-agent, worktree, rien de commité). **Vitesse du plus lent** : un ordre en formation donne à
  chaque unité `FHOLMoveOrderFragment::bFormation` et `MarchSpeed` (vitesse de la table du plus lent du groupe) ;
  `AdvanceUnits` refait le minimum par numéro d'ordre parmi les unités encore en route (`SlowestOfOrder` /
  `NextSlowestOfOrder`, servi au pas suivant, déterministe) ; arrivée ou nouvel ordre → vitesse propre ; groupe cassé →
  le reste accélère jusqu'au plus lent restant. `GetOrderWalkSpeed` (× `hol.Units.SpeedScale`) dans `StartFinalLeg`,
  `ServePathRequests`, `AdvanceUnits`. **Reprise de place** : `HOLFormation::FPlaceHold`, `EndWalkToPlace`,
  `WatchPlace` ; place gardée si l'arrivée est à moins d'**un rayon (60 cm)** ; repart en ligne droite si à plus de
  60 cm et immobile (< 2 cm par pas) **20 pas de suite** (1 s) ; un retour qui finit loin, ou un chemin direct coupé
  (raycast navmesh), lâche la place (pas d'oscillation) ; tout nouvel ordre ou une activité autre qu'Idle la lâche.
  Compteur `GetTotalPlaceReturns()`. Tests `Formation.Speed`, `Formation.Return` ; compilation sans avertissement ;
  **tests 72/72** ; **`quick_check -Parts tests,camera,move,anims` PASS** (44 s).
- Points ouverts : **pieds qui glissent** chez une unité ralentie (`WalkPlayRate` fixe par type, à multiplier par
  vitesse réelle / vitesse du type) ; aucune étape de `quick_check` pour un groupe mixte au même pas ni pour un retour
  à sa place (à voir à l'essai) ; un Paysan qui revient à sa place sort un instant du compte des inactifs du HUD (à
  confirmer).
- **3e boucle** lancée (sous-agent, worktree) : glisser du clic droit (orientation, largeur du front).
- **3e boucle faite** (sous-agent, worktree, rien de commité). **Glisser du clic droit** : règles `GetFrontWidth`
  (⌊longueur / 150⌋ + 1 colonnes, une à chaque bout du trait, entre 1 et le nombre d'unités ; « + 1 » à confirmer) et
  `GetFrontFacing` (face au travers du trait, du côté opposé au centre du groupe ; groupe sur la ligne → à gauche du
  trait vu d'en haut, gauche → droite fait face à +X) ; `PlaceGroup(…, FrontWidth)` (0 = largeur par défaut ; sinon
  rangs pleins, le dernier plus court). Ordre : `FHOLMoveOrder::FrontLine` (nul = clic simple), `IssueMoveOrder(…,
  FrontLine)`, appel serveur `ServerMoveUnitsInLine(UnitIds, Start, End)` (`FVector_NetQuantize`, propriétaire vérifié,
  orientation et largeur calculées par le serveur). Joueur : `IA_Order` lié aussi à `Completed`, décision au relâcher
  (`HandleOrderCompleted`, seuil de 6 px du cadre) ; clic simple inchangé mais au relâcher ; `OrderLineAt` retombe sur
  `OrderAt` pour moins de 2 unités, une source, un chantier ou un bâtiment ; Maj ne met pas les déplacements en file
  (récolte et construction gardent Maj). Commande `hol.Select.OrderLine X1 Y1 X2 Y2`. Aperçu local : anneaux blancs
  sur les places pendant le glisser (`HOLSelectionMarkers::UpdateRings`, `GetOrderLine`, sans projection navmesh).
  Test `Formation.Front` ; compilation sans avertissement ; **tests 73/73** ; **`quick_check -Parts
  tests,camera,move,anims` PASS** (42 s). Glisser **pas essayé** (aucune étape ne l'utilise).
- Points ouverts : les unités ne se tournent pas vers l'orientation du rang à l'arrivée ; étape `quick_check` du
  glisser et d'un groupe mixte ; pieds qui glissent.
- **4e boucle** lancée (sous-agent, worktree) : pieds qui glissent, unités tournées vers le front à l'arrivée.
- **4e boucle faite** (sous-agent, worktree, rien de commité). **Pieds qui glissent** :
  `FHOLUnitAnimationInput::WalkSpeed` (vitesse du type × `hol.Units.SpeedScale`), `HOLUnitAnimation::GetWalkRateShare`
  (vitesse / vitesse du type, bornée **0,5 – 1,2**) appliquée par `Choose` à la marche et à la marche chargée ;
  branché dans `HOLInterpolatedInstancesProcessor.cpp` (vitesse affichée, donc aussi chez le client). Limite : la
  cadence n'est remise à Mass qu'au changement de séquence (fixée au début de la marche). **Face au front** :
  `FHOLMoveOrderFragment::FrontYaw`, `bFacesFront` (réglés dans `ApplyQueuedOrders`, mis à vrai à l'arrivée d'une marche
  en formation ou d'un retour à sa place, remis à faux par tout ordre ou activité) ; `UHOLInstantOrientationProcessor`
  tourne vers `FrontYaw` sous 20 cm/s (virage immédiat comme J5a, affichage interpolé sur un pas). **Limite réseau** :
  l'orientation n'est pas répliquée, un client garde le sens de la dernière marche (presque le front par défaut, pas
  pour un glisser de travers). Test `Animation.Choice` étendu ; compilation sans avertissement ; **tests 73/73** ;
  **`quick_check -Parts tests,camera,move,anims` PASS** (42 s).
- Question à l'utilisateur : orientation au front chez le client (1 bit « face au front » + ~6 bits d'orientation pour
  une unité à l'arrêt, budget de D270).
- **5e boucle** lancée (sous-agent, worktree) : étape `quick_check` du glisser, d'un groupe mixte et de l'orientation.
- **5e boucle faite** (sous-agent, worktree, rien de commité). Partie **`formation`** de `quick_check` (après `anims`,
  dans la liste par défaut) ; commandes `hol.Units.Formation [Slot]` (état de formation par unité) et
  `hol.Select.OrderLineGround X1 Y1 X2 Y2`. 3 Paysans + 3 Hommes d'armes en (-6000, -5200/-5800) envoyés à 35 m.
  Vérifie : **même pas** (vitesse de marche = le plus lent, 523 cm/s, ≤ 1 cm/s ; personne au-dessus de +10 %) ;
  **arrivée** (≤ 60 cm de sa place, relevé 0-38 ; écart ≥ 110 cm, relevé 149 ; soldats un rang devant ; rangs en
  travers ; face à la marche ≤ 10°, relevé 0°) ; **glisser** de 3,6 m en diagonale (premier rang à ≤ 60 cm du trait,
  3 colonnes sur 3, entre les bouts, fait de soldats ; rangs à 180 ± 60 cm ; `FrontYaw` à 2° de 135° ; repos à 10°) ;
  **retour** (compteur 1 → 2 à l'arrivée ; un Paysan qui traverse le front ne déplace personne de plus de 28 cm).
  Captures `formation_mixed`, `formation_front`, `formation_back`. **`quick_check -Parts
  tests,camera,move,anims,formation` PASS** (72 s, tests 73/73). Aucun défaut trouvé. Notes : le trait relu après
  projection écran → sol est décalé de 15-20 cm et un peu plus court (3 m tombait sous 3 colonnes) ; un retour certain
  sous bousculade demanderait une poussée plus forte.
- Suite : **essai de l'utilisateur** (largeur « + 1 », Maj sans file pour les déplacements, cadence 0,5 – 1,2, virage
  immédiat ; orientation chez le client : A 7 bits / B reportée à J7b, recommandée B).
- Choix de l'utilisateur : **B**, orientation au front chez le client **reportée à J7b** (avec le combat). Essai de
  l'utilisateur lancé.

### 2026-10-18 — J7a.4 : 1er essai de l'utilisateur

- Retours de l'utilisateur :
  1. **Couleur** : le bonhomme, le fanion et le bouclier doivent prendre la couleur du joueur (toujours bleus
     aujourd'hui) ; l'emblème de la civilisation viendra plus tard sur le fanion et le bouclier.
  2. **Glisser du clic droit** : changer la largeur lui plaît mais casse la formation ; **le glisser ne donne que
     l'orientation** (largeur par défaut gardée) ; **Ctrl + glisser** donne orientation et largeur.
  3. **Même pas** : Hommes d'armes, Paysans et Lanciers ensemble ne marchent pas en gardant la formation du départ ni
     à la vitesse du plus lent (table : Paysan 600, Lancier 583, Homme d'armes 523 cm/s).
  4. **Bousculade** : le retour à sa place marche mais est **trop lent**.
  5. **Maj** doit mettre les ordres de déplacement **en file**.
- **6e boucle** lancée (sous-agent, worktree) : points 2, 4 et 5.
- Choix de l'utilisateur pour le point 3 : **bloc façon AoE4** — le groupe se met d'abord en rangs sur place (court
  arrêt), puis tout le bloc suit un seul chemin au pas du plus lent, rangs gardés, et pivote à l'arrivée. Boucle à
  suivre après la 6e ; puis la couleur du joueur (point 1).
- **6e boucle faite** (sous-agent, worktree, rien de commité). **Glisser = orientation seule**, **Ctrl + glisser =
  orientation et largeur** (`FHOLMoveOrder::bFrontWidth`, `ServerMoveUnitsInLine(…, bWidth, bQueue)`,
  `IsOrderWidthHeld()` au relâcher, aperçu qui lit Ctrl en direct ; priorité des rangs partagée dans
  `HOLFormation::GetRowPriority`). **Retour plus rapide** : cause, « immobile » (< 2 cm par pas) jamais vrai entre deux
  corps qui s'écartent ; désormais seule une vraie poussée (> 2 cm plus loin de sa place en un pas, ou > 8 cm,
  `PushStepDistance`) remet le compteur à zéro ; délai **5 pas (0,25 s)** ; 3 essais au plus (`MaxReturnTries`) puis
  place lâchée. Mesuré : poussé à 337 cm, sur sa place ~1 s après le pic. **Maj met les déplacements en file**
  (`FQueuedMove`, `StartQueuedMoves`, `ForgetMoveQueue` ; tout l'ordre attend tant qu'une unité du groupe marche
  encore ; places calculées au départ de l'ordre en file ; pas de marqueurs des points en file). `quick_check`
  `formation` étendue (glisser simple, Maj + clic, capture `formation_queued`). Compilation sans avertissement ;
  **tests 73/73** ; **`quick_check -Parts tests,camera,move,anims,formation` PASS** (91 s). Compiler ce worktree avec
  `C:\UE\UE_5.8_AS_3\Engine\Build\BatchFiles\Build.bat` (AS_1 : erreurs de PCH).
- **7e boucle** lancée (sous-agent, worktree) : marche en bloc façon AoE4.
- **Couleur du joueur** lancée en parallèle (demande de l'utilisateur ; sous-agent, branche `j7a4-colour` dans le
  dépôt principal, partie de 8280f6a, à fusionner dans `j5c-anims` ensuite).
- **Couleur du joueur faite** (sous-agent, branche `j7a4-colour`, rien de commité). **Cause** : `GetCustomData`
  (`Presentation/HOLInterpolatedInstancesProcessor.cpp`) passait le numéro d'emplacement (`OwnerPlayer`) à
  `HOLPlayerColors::GetLinearColor` au lieu de la couleur choisie au menu (`ColorIndex`, D244) : l'emplacement 0 du
  joueur humain était toujours bleu. **Correctif** (C++ seul) : `FSlotColors`, rempli une fois par image dans
  `Execute` (couleur des bâtiments de l'emplacement, répliquée, puis celle de l'`AHOLPlayerState`, sinon le numéro) ;
  corps, fanion et face du bouclier ; solo, serveur et client (même processeur). `quick_check` `anims` : 3 unités de
  l'emplacement 1 (`$AnimOthers`), capture `anims_colors` (bleu et rouge côte à côte, mais l'emplacement 1 était déjà
  rouge avant). Tests 69/69 ; **`quick_check -Parts tests,camera,anims` PASS** (44 s). À vérifier à l'essai : une
  couleur autre que le bleu choisie au menu ; gros plan du fanion et du bouclier.
- **7e boucle arrêtée par l'utilisateur** en cours de route (le sous-agent commençait les règles pures de la marche en
  bloc) : le worktree mêle les boucles 1-6 (vérifiées) et des changements partiels de la 7e, non compilés ni testés.
- Arrêt par erreur (l'utilisateur) : **7e boucle relancée** (nouveau sous-agent, repart des changements partiels).
- **7e boucle faite** (sous-agent, worktree, rien de commité). **Marche en bloc** (`hol.Units.BlockMarch 1`) :
  mise en rangs sur place (`PlaceGroup` tourné vers le début du chemin du groupe ; fin à 30 cm de sa place,
  `FormUpTolerance`, ou 60 pas = 3 s, `FormUpMaxSteps`) ; un seul chemin (centre → milieu des places d'arrivée), place
  de chacun = point du bloc + décalage tourné selon le chemin (direction lue sur ± 200 cm) ; place hors navmesh
  ramenée vers le chemin ; pas du plus lent ; le bloc attend une unité à plus de 40 cm, qui le quitte au bout de 60
  pas ; rattrapage 1,08 × le pas ; arrivée : places finales redistribuées, `StartFinalLeg`, puis la logique existante.
  Déplacement court (sous la taille du bloc) : droit aux places. **Paysans seuls : pas de bloc** (choix à confirmer).
  `GetSlotOnPath` et voisins (`HOLFormationRules`), phases `FormUp` / `Block`, `StartBlockMarch`, `AdvanceBlocks`,
  `SteerInBlock`, `FBlockMarch`, `FBlockReport`. Test `Formation.Block`. `quick_check` : étape « formation block »
  (9 unités sur 35 m : mise en rangs 31 pas = 1,55 s, marche 127 pas, 523 cm/s au plus, écart à sa place ≤ ~26 cm,
  personne qui quitte le bloc). Compilation sans avertissement ; **tests 74/74** ; **`quick_check -Parts
  tests,camera,move,anims,formation` PASS** (114 s). **Défaut** : au glisser simple et à Maj + clic, la mise en rangs
  va jusqu'à 3 s puis une unité retient le bloc 3 s et le quitte (écart 252 cm, 557 cm/s, ~10 s par ordre au lieu
  de 5) ; piste : place de mise en rangs bloquée.
- **8e boucle** lancée (sous-agent, worktree) : ce défaut.
- Choix de l'utilisateur : **Paysans seuls sans marche en bloc** confirmé (en rangs à l'arrivée ; mêlés à des
  soldats, ils suivent le bloc).
- **8e boucle faite** (sous-agent, worktree, rien de commité). **Cause** : au glisser simple et à Maj + clic, le bloc
  tourne d'environ 107° ; les soldats passent devant en traversant le rang des Paysans, chacun va tout droit à sa place
  et la séparation dure les remettait au même point à chaque pas (positions figées 5 s). **Correctif** : les unités
  d'une même mise en rangs (même `OrderSerial`, phase `FormUp`) se traversent (`UHOLUnitSeparationProcessor`,
  `FormUpSerials` ; `AvoidBodies(…, FormUpSerial)` dans `UHOLFullSpeedProcessor`) ; pas pendant `Block`. Mesures :
  glisser simple 60 + 142 pas, 1 partie, 557 cm/s → **13 + 83 pas**, 0 partie, 523 cm/s ; Maj 60 + 153 → **12 + 94**,
  0 partie ; écart à sa place ≤ 16 cm. `quick_check` resserré (mise en rangs ≤ 30 pas, personne ne quitte, ≤ 523 +
  10 %). **Tests 74/74** ; **`quick_check -Parts tests,camera,move,anims,formation` PASS** (115 s). À juger à
  l'essai : chevauchement bref pendant une mise en rangs qui tourne beaucoup.
- Couleur du joueur **fusionnée** dans le worktree (patch de `j7a4-colour` ; un morceau recollé à la main dans
  `Execute` : `FSlotColors` à côté de `SpeedScale`). Vérification lancée (sous-agent) : compilation, `quick_check`
  complet. Puis 2e essai de l'utilisateur.
- Vérification après fusion (sous-agent) : compilation sans avertissement ; **`quick_check` complet PASS en 417 s**
  (toutes les parties, `formation` et `net` comprises), **tests 74/74** ; `anims_colors` : emplacement 1 rouge,
  emplacement 0 bleu. 2e essai de l'utilisateur lancé.

### 2026-10-18 — J7a.4 : 2e essai de l'utilisateur

- Retours :
  1. **Orientation du glisser inversée** : la direction va de la base du clic à la souris (souris vers la droite →
     ils regardent à droite). Choix de l'utilisateur : le glisser est une **flèche** — point d'arrivée = base du clic,
     regard = base → souris ; **Ctrl + glisser** : même flèche, sa longueur donne la largeur du front (rang centré sur
     la base).
  2. **Arrivée** : la formation en marche est bien, mais à l'arrivée ils n'ont pas anticipé la rotation et se
     replacent en se bousculant dans tous les sens.
- **9e boucle** lancée (sous-agent, worktree) : ces deux points.
- **9e boucle faite** (sous-agent, worktree, rien de commité). **Flèche** : `HOLFormation::GetFrontFacing(Arrow)`
  (direction de la flèche, +X si nulle) ; `ServerMoveUnitsInLine` vise la base (`Start`), `FrontLine = End - Start`,
  Ctrl : largeur par `GetFrontWidth(longueur)` ; ping à la base ; aperçu : anneaux à la base, orientés par la flèche,
  points tous les ~80 cm de la base à la souris. **Pivot anticipé** : `AdvanceBlocks` tourne les décalages du bloc
  vers l'orientation finale sur les derniers « taille du bloc + 3 m » (au plus la moitié du chemin ;
  `GetBlockPivot`, smoothstep ; rotation bornée, `BlockPivotShare`, le bloc ralentit d'autant ; fini à 1° près, sinon
  sur place) ; places finales **une pour une** depuis les places du bloc (redistribution supprimée). Mesure
  `FArrivalWatch` / `WatchArrivals` (croisements, poussées, pas jusqu'à posés, écart en fin de bloc). Glisser simple
  (pivot ~165°) : posés en 24 → **3** pas, poussées 9 → **0**, écart 240 → **9 cm**, marche 99 → 128 pas ; Ctrl :
  5 → 3 pas, écart 61 → 9 cm ; croisements 0. `quick_check` : posés en ≤ 20 pas, 0 croisement, ≤ 2 poussées.
  **Tests 74/74** ; **`quick_check -Parts tests,camera,move,anims,formation` PASS** (114 s). 3e essai lancé.

### 2026-10-18 — J7a.4 : 3e essai de l'utilisateur

- Retours : ça fonctionne, mais (1) **retirer les points** de l'aperçu entre la formation et la souris ; (2) le pivot
  est **beaucoup trop long** (rotation entière du bloc) : s'inspirer d'AoE4 (chaque unité gagne directement la place
  la plus proche de son type, le bloc ne tourne pas d'un bloc ; un demi-tour retourne la formation sur place).
- **10e boucle** lancée (sous-agent, worktree).
- **10e boucle faite** (sous-agent, worktree, rien de commité). Points de l'aperçu retirés. Pivot rigide supprimé
  (`GetBlockPivot`, `BlockPivotMargin`, `BlockPivotShare`, champs de `FBlockMarch`). **Reformation façon AoE4** : sur
  les derniers **1,5 m** (`BlockReformDistance`), places finales réattribuées par type depuis les positions réelles
  (`HOLFormation::AssignPlacesByKind`), puis chacun va droit à sa place ; les unités d'un même ordre se traversent
  pendant la mise en rangs et la reformation (`FHOLMoveOrderFragment::bReforming`, `GetPassThroughSerial()`). Le premier
  rang reste devant selon la nouvelle orientation : au demi-tour, le type de devant **traverse le bloc** (chevauchement
  bref, voulu). Glisser simple (~165°) : marche 128 → **94** pas, posés en 3 → 15 pas (0,75 s), 0 poussée, 0
  croisement ; Ctrl 71 pas, posés en 10 ; Maj 112 pas, posés en 9 ; bloc de 9 : 122 pas, posés en 8. **Tests
  74/74** ; **`quick_check -Parts tests,camera,move,anims,formation` PASS** (107 s ; lancer avec `-EngineDir
  C:\UE\UE_5.8_AS_3\Engine\Binaries\Win64`). 4e essai lancé.

### 2026-10-18 — J7a.4 : 4e essai de l'utilisateur

- Retours : l'arrivée est mieux, **la pousser encore** si possible ; **pourquoi refaire une formation avant de partir**
  quand le groupe est déjà en formation ?
- **11e boucle** lancée (sous-agent, worktree) : pas de mise en rangs à l'arrêt pour un groupe déjà en formation
  (rangs repris en marchant), reformation d'arrivée commencée plus tôt et fondue dans la marche.
- **11e boucle faite** (sous-agent, worktree, rien de commité). **Mise en rangs en marchant** : si chacun est à
  **450 cm** au plus de sa place (`BlockJoinDistance`), le bloc part aussitôt (`bJoining`), les unités rejoignent leur
  place en marchant (traversée, ≤ 1,08 × le plus lent), le point du bloc ralentit selon le plus grand écart
  (`GetBlockSoftPace` : plein pas jusqu'à 40 cm, arrêt à 160 cm) ; règle valable pour tout groupe proche (un groupe
  éparpillé se rassemble d'abord jusqu'à 450 cm). **Arrivée fondue** : sur « taille du bloc + 2,5 m »
  (`GetBlockBlendDistance`, `BlockBlendMargin`), la place de chacun glisse de sa place dans le bloc à sa place finale
  (`GetBlockBlend`), places finales données une fois (`AssignPlacesByKind`) ; `BlockReformDistance` supprimé. Mesures
  (mise en rangs + marche / posés) : glisser simple ~12 + 94 / 15 → **0 + 116 / 3** ; Ctrl 71 / 10 → **0 + 89 / 3** ;
  Maj 112 / 9 → **0 + 127 / 2** ; bloc de 9 éparpillé → 12 + 148 / 2 ; 0 croisement, 0 poussée, posés à 0-5 cm,
  ≤ 565 cm/s. `quick_check` : mise en rangs ≤ 2 pas, posés ≤ 8 pas, marche ≤ chemin + 20 %. **Tests 74/74** ;
  **`quick_check -Parts tests,camera,move,anims,formation` PASS** (110 s). 5e essai lancé.
- **5e essai : validé** (« ça a l'air great »). Commit **cb8227d** sur `j5c-anims` (couleur du joueur comprise ; la
  branche `j7a4-colour`, sans commit, est supprimée), avance rapide de `main`. **J7a.4 close.** Reprise : **J7a.5**
  (groupes de contrôle et HUD).

### 2026-10-18 — J7a.5 : groupes de contrôle et HUD

- **1re boucle** lancée (sous-agent, worktree `HeroesOfLiberty-J5c`) : groupes de contrôle de D30 (Ctrl + chiffre, chiffre,
  deux appuis, Maj + chiffre, Ctrl + Maj + chiffre) et Ctrl + A (soldats seuls), clavier AZERTY compris. HUD et textes
  à la 2e boucle.
- **1re boucle faite** (sous-agent, worktree, rien de commité). `AHOLPlayerController` : `AssignControlGroup`,
  `SelectControlGroup`, `AddToControlGroup`, `RemoveFromControlGroup`, `GetControlGroup`, `SelectArmy`,
  `HandleGroupKey`, `ControlGroups[10]` ; **local seulement** (la sélection l'est déjà, D262 ; les ordres envoient les
  identifiants). Un groupe : unités à soi **ou** bâtiments à soi ; morts et disparus retirés à la lecture ; chiffre =
  remplace la sélection ; deux appuis en moins de 0,3 s : caméra sur le centroïde (`SetFocus`, comme F1). **Ctrl + A** :
  unités à soi hors Paysans (dans `HandleGridKey` : avec Ctrl, les autres touches de la grille ne font rien). Actions
  `IA_Group_0..9` (`make_game_input.py`, `IMC_Game` refait) : `One…Zero`, AZERTY `Ampersand`, `E_AccentAigu`,
  `Quote`, `Apostrophe`, `LeftParantheses`, `Hyphen`, `E_AccentGrave`, `Underscore`, `C_Cedille`, `A_AccentGrave`,
  pavé `NumPadOne…Zero` ; Ctrl et Maj lus par `IsInputKeyDown`. Commandes `hol.Select.Group <Assign|Select|Add|Remove|
  Double> <0-9>`, `hol.Select.Army`. `quick_check` : section `groups` dans `formation` (vraies touches injectées).
  **Tests 74/74** ; **`quick_check -Parts tests,camera,move,formation` PASS** (93 s). Limites : sur QWERTY,
  `Apostrophe` et `Hyphen` servent aussi aux groupes 4 et 6 ; Maj + pavé numérique ne marche pas avec Verr Num.
- **2e boucle** lancée (sous-agent, worktree) : HUD des groupes et du groupe mixte, textes fr / en / ja.
- **2e boucle faite** (sous-agent, worktree, rien de commité). **Barre des groupes** en C++
  (`HOLGameScreen.cpp/.h`, `GroupsPanel` en bas à gauche) : une case par groupe non vide (1 à 9 puis 0,
  `ControlGroup0..9`), « chiffre · effectif », infobulle « Groupe N » + types par nombre ; clic = touche, deux clics =
  caméra ; relue 4 fois par seconde ; groupe choisi mis en avant 0,5 s (`GetLastSelectedGroup`). Log `HUD groups: …`,
  commande `hol.HUD.Click ControlGroupN`. **Bloc mixte** : soldats d'abord, Paysans en dernier (était alphabétique).
  Textes `ControlGroupSlot`, `ControlGroupTooltip` (fr / en / ja, `fill_po.py`, GatherText). `quick_check` `groups`
  étendue, capture `groups_bar`. **Tests 74/74** ; **`quick_check -Parts tests,menu,camera,move,formation` PASS**
  (105 s). Essai de l'utilisateur lancé.
- **Essai de J7a.5** : « tout a l'air bon ». Choix de l'utilisateur : **Ctrl + chiffre retiré** — Maj + chiffre crée le
  groupe s'il est vide et l'agrandit sinon ; Ctrl + Maj + chiffre retire (précise D30, à reporter au tableau des
  touches du GDD § 7.3 à la clôture de J7). Boucle courte lancée (sous-agent, worktree).

### 2026-10-18 — J7a.6 : vérification

- **1re boucle** lancée (sous-agent, worktree) : partie armée à 2 clients dans `quick_check` (Caserne, soldats,
  formation, groupes chez un client), puis `quick_check` complet.
- **1re boucle faite** (sous-agent, worktree, rien de commité). Étape **`army`** de la partie `net` : chaque client pose
  une Caserne, produit 2 Lanciers et 2 Hommes d'armes ; « Units looks » (`DescribeLooks`, `hol.Units.Report`) : soldats
  de l'autre en `SKM_Spearman` / `SKM_ManAtArms`, couleur de l'autre ; groupe 1 par Maj + &, puis & (5 unités, bloc
  mixte) ; clic droit à 30 m : 10 unités à **5 cm** au plus du serveur, 5 par emplacement à ~2 m du clic, paire la plus
  proche 114 / 149 cm. Commandes `hol.Camera.FocusNear`, `hol.Select.OrderNear`, `hol.Prod.Queue [Type] [Bâtiment]`.
  Captures `net_army_c1/c2`, `qcnet_N_army_other`. **`quick_check` complet FAIL** (423 s, tests 74/74) : tout PASS
  (`net 2 clients` compris) **sauf `gather`** — « deux Paysans au repos à moins de 90 cm » : **83 cm** (#21 et #19).
  Le `quick_check` complet passait après la 8e boucle de J7a.4 : piste, la traversée ajoutée en J7a.4 (9e-11e
  boucles, `bReforming` / `GetPassThroughSerial`) qui resterait active.
- **2e boucle** lancée (sous-agent, worktree) : ce FAIL.
- **2e boucle faite** (sous-agent, worktree, rien de commité). Reproduit seulement en `quick_check` complet (83 puis
  70 cm, #21 et #19). **Cause** (pas la traversée de J7a.4) : deux Paysans des baies qui déchargent aux pas 952 et 953
  s'arrêtent côte à côte contre le mur de la Maison ; `UHOLGatherProcessor::Stop` ne les laissait se tasser
  (`bSettling`) qu'**un pas**, et `UHOLFootprintKeepOutProcessor` les remettait contre le mur ; le relevé du test était
  pris 5 pas après l'arrêt. **Correctif** : `FHOLGatherFragment::SettleSteps`, `StopSettleSteps = 10` (tassement
  pendant 10 pas) ; `wait 1` avant `hol.Units.Where 0` dans « gather used up ». Paysans au repos à 120 cm (avant 111).
  **`quick_check` complet PASS** (422 s), **tests 74/74**. Un 2e passage complet lancé pour confirmer.
- **2e passage complet : PASS** (421 s, tests 74/74, toutes les parties) ; `gather` : paire au repos la plus proche à
  102 cm (seuil 90). Essai de clôture de J7a lancé.
- **Essai de clôture de J7a : validé** (« c'est bon »). Commit **2be3be1** (étape `army` du `net`, correctif de
  `gather`) sur `j5c-anims`, avance rapide de `main`. **J7a close.** Reprise : **J7b** (la mêlée ; orientation au front
  chez le client, choix B ; S3 en J7b ; S2 à la fin de J7).

### 2026-10-18 — J7b.1 : règles pures du combat

- **1re boucle** lancée (sous-agent, worktree) : `Simulation/HOLCombatRules` (dégâts, bonus, armure, cadence, choix de
  cible), colonnes de combat lues dans `FHOLUnitTypeConstants`, tests `Combat.*`.
- **J7b.1 faite** (sous-agent). `Simulation/HOLCombatRules.h/.cpp` (`HOLCombat`) : `GetDamage` (attaque + bonus de
  catégorie − armure, 1 au moins), `GetHitsToKill`, cadence en millisecondes entières (`TickAttack`, `TickIdle` ; pas
  de 50 ms ; 1,875 s → coups à 37 ou 38 pas, sans dérive), **préparation du premier coup = 50 %** de la cadence
  (`StartAttack` : un changement de cible ne donne pas de coup gratuit), `IsInRange` / `GetEdgeDistance` (bord à bord),
  `GetSearchRadius` (maintien : portée seule ; rayon d'agression 0 → personne, sauf attaque-déplacement :
  `AttackMoveRadius` **1 200 cm**), `ChooseTarget` (cible actuelle gardée à portée ; unité avant bâtiment, à portée
  d'abord, qui m'attaque, étalement au-delà de **6 attaquants** (`MaxAttackers`) hors de portée, plus proche, plus petit
  identifiant ; indépendant de l'ordre). `FHOLUnitTypeConstants` lit les 9 colonnes de combat (format réseau inchangé).
  Exemples : Lancier → Homme d'armes 4 par coup, **39 coups (~72 s)** ; Homme d'armes → Lancier 8, **10 coups
  (13 s)** ; Lancier → cavalerie (armure 2) 23 ; Paysan → Homme d'armes 1. Tests `Combat.Damage`, `.Cadence`,
  `.Range`, `.Target` ; compilation sans avertissement ; **tests 78/78** ; `quick_check -Parts tests` PASS (21 s).
  Commitée (rien de visible en jeu). À revoir à l'essai : `AttackMoveRadius`, `MaxAttackers`, durée des duels.
  Reprise : **J7b.2** (processeur de combat).

### 2026-10-18 — Mesures à la demande de l'utilisateur (après J7b.1)

- `chapter_measures.ps1 -Tag j7b1_req` (demande de l'utilisateur, hors de la règle des mesures ; 7 min, serveur
  recuit ; dépôt principal recompilé avant) : **S1 PASS** 2,550 / 2,433 ms (moyenne 2,492 ; fin de J6 : 2,37) ;
  **S2 FAIL** 3,250 / 3,106 ms (**moyenne 3,178**, seuil 3 ; fin de J6 : 2,995 au seuil) ; **S7 PASS** 2,775 / 2,720
  (2,748 ; fin de J6 : 2,60) ; **S5 PASS** 30 947 o/s, écart affiché 22,7 cm. Rendu 201 à 218 fps partout.
- Lecture : **S2 a perdu ~0,18 ms** et S7 ~0,15 ms depuis la fin de J6 ; J7a.4 a touché le mouvement (marche en bloc,
  `AssignPlaces`, traversée dans la séparation, tassement à l'arrêt de J7a.6). À traiter avant la fin de J7 (S2 était
  déjà au seuil) : profil de S2 à comparer à la fin de J6 (`stress_ab.ps1`, côté A à 3141200^ ou à la fin de J6).
  Journaux `chapter_j7b1_req_*`, résumé `Saved_Tests\Logs\chapter_measures.txt`.

### 2026-10-19 — S2 : perte de 0,18 ms depuis la fin de J6

- Choix de l'utilisateur : **regarder S2 avant J7b.2**. Boucle lancée (sous-agent) : A / B en alternance entre la fin
  de J6 (c14d25b, worktree détaché) et `main`, profil de S2, cause et correctif si simple.
- **Résultat** (sous-agent ; côté A : worktree détaché `MyCrusader-S2A` à c14d25b, laissé en place). S2 de 10 s,
  A B A B A B au repos : **A 2,820 ms** de moyenne (2,915 / 2,816 / 2,730), **B (main) 2,905 ms** (2,925 / 2,965 /
  2,825) ; écart +0,085 ms, plus petit que la dispersion d'un même côté (0,19 ms) ; six passages PASS. Profil par
  processeur (Insights, un passage par côté) : simulation **+0,01 ms par pas** au total (séparation +0,003, plein
  régime +0,010, évitement +0,003 par pas). S2 ne donne que des ordres d'une unité : ni marche en bloc ni
  `SlowestOfOrder` ; `SettleSteps` ne touche que la récolte. **Ma piste J7a.4 était fausse** : le 3,178 ms de
  `j7b1_req` vient très probablement de l'état de la machine. Hors simulation : `HOLInterpolatedInstancesProcessor`
  +0,026 ms par image (`FSlotColors` parcourt bâtiments et joueurs à chaque image ; cache possible). Aucun correctif.
- S2 relancé seul par `chapter_measures.ps1 -Tests S2:10 -NoS5` (machine au repos) pour confirmer.
- S2 seul (`-Tag s2_rest`) : **FAIL 3,105 / 3,049 ms** (moyenne 3,077). Charge avant les passages 7 à 14 %, puis 31 % :
  Opera, Discord, GitKraken, Battle.net, IMF ouverts — machine **pas au repos**. À refaire après fermeture de ces
  applications (le sous-agent mesurait 2,905 ms sur le même code dans le worktree).
- **Mesures complètes refaites, Opera fermé** (`-Tag j7b1_rest2`, 5,3 min) : **toutes PASS** — S1 2,263 / 2,169
  (2,216 ms), **S2 2,829 / 2,850 (2,840 ms)**, S7 2,484 / 2,676 (2,580 ms), S5 30 897 o/s, 22,4 cm ; 232 à 253 fps.
  Le FAIL de S2 venait de la charge de la machine (Opera). Reprise : **J7b.2** (l'utilisateur est absent : boucles
  sans essai, rien de visible commité, résumé dans `SummaryToReview.md`).

### 2026-10-19 — J7b.2 : processeur de combat

- **1re boucle** lancée (sous-agent, worktree) : fragments de combat, recherche de cible à ~5 Hz, approche, frappe,
  mort ; cache de `FSlotColors`.
- **1re boucle faite** (sous-agent, ~25 min au lieu de 15 : deux relances de `quick_check` ; rien de commité). PV :
  `FHOLUnitStateFragment::CurrentHealth` (déjà là). `EHOLCombatMode` (Normal / AttackMove / Hold),
  `FHOLCombatFragment` (cible, dernier attaquant, point d'approche, cadence, attaquants, compteurs, `bOrderedTarget`,
  `bAttacking`, `bApproaching`). **`UHOLCombatProcessor`** (dernier du groupe Tasks, avant le suivi de chemin ; unités
  dans l'ordre de leur `UnitId` ; grille de 4 m refaite à chaque pas) : recherche tous les 4 pas, décalée par
  `(UnitId + pas) % 4`, et tout de suite à la mort de la cible ; candidats = ennemis dans le rayon + cible + dernier
  attaquant ; approche par marche de tâche (relancée si la cible a bougé de plus de 1 m) ; à portée : `StopWalk`,
  `StartAttack`, `TickAttack`, `GetDamage` ; mort par `RequestDestroyUnits` (D224, population libérée). Paysans sans
  agression : ne ripostent pas ; une unité en marche d'ordre, en récolte ou en chantier ne cherche pas. Cvars
  `hol.Combat.Enabled` (1), `hol.Combat.LogHits`, `hol.Combat.Report [Slot]`. `FSlotColors` en cache (0,5 s).
  `quick_check` **`combat`** : 3 Hommes d'armes contre 3 Lanciers à 6 m ; premier coup ~1 s ; Lanciers morts à 7,55 /
  9,7 / 15 s ; 30 coups de 8 = 10 par Lancier ; Hommes d'armes à 103 / 155 / 139 PV (riposte) ; population du slot 1
  −3. `quick_check` lancé avec `hol.Combat.Enabled 0`, combat réactivé au début de la partie (sinon les unités
  d'autres emplacements de `anims` se battaient). **Tests 78/78** ; **`quick_check -Parts
  tests,camera,move,gather,anims,formation,combat` PASS** (166 s). Ouverts : `net` pas lancé (serveur dédié sans
  `hol.Combat.Enabled 0`) ; un ordre du joueur n'interrompt pas un combat engagé ; pas d'activité « attaque » ; pas de
  places autour d'une cible.
- **2e boucle** lancée (sous-agent, worktree) : un ordre du joueur prime sur le combat, `quick_check` complet (`net`
  compris).
- **2e boucle faite** (sous-agent, rien de commité). **Un ordre du joueur prime** : `FHOLCombatFragment::Disengage()`
  (cible, dernier attaquant, `bOrderedTarget`, `bAttacking`, `bApproaching`) appelée par `ApplyQueuedOrders` pour tout
  ordre de déplacement hors tâche (simple, formation, bloc, Maj au démarrage) ; une unité en récolte ou en chantier est
  désengagée par le processeur de combat ; pas de recherche pendant une marche d'ordre ; ordre des processeurs vérifié.
  `combat` étendue : ordre de recul de 20 m en pleine mêlée → plus de cible en 1 s, aucun coup en 4 s, ≥ 8 m parcourus
  (6 coups avant et après ; x −6113 → −8109) ; les Lanciers poursuivent et les Hommes d'armes réengagent à l'arrivée.
  `net` : combat actif sur le serveur dédié, « aucun mort » vérifié. `combat` dans les parties par défaut.
  **`quick_check` complet PASS** (452 s), **tests 78/78**. Ouvert : une marche d'approche émise au pas précédent peut
  écraser un ordre Maj qui démarre (rare).
- **J7b.3** lancée (sous-agent, worktree) : ordres d'attaque, attaque-déplacement, arrêt, maintien de position.
- **J7b.3, 1re boucle faite** (sous-agent, ~25 min, rien de commité). **Touches** (choix sans l'utilisateur, à
  valider) : **Q** attaque-déplacement (soldats seuls), **S** arrêt, **D** tenir la position (toute sélection de ses
  unités). `EHOLOrderKind` (Move / Attack / AttackMove / Stop / Hold), `FHOLMoveOrder::TargetUnitId`, `CombatStamp` ;
  `IssueCombatOrder`, `IssueAttackMoveOrder`, `IssueCombatWalk`. Serveur (`ApplyCombatOrder`) : attaque acceptée si
  cible vivante et ennemie (D288) ; Attaque / Arrêt / Tenir arrêtent net, annulent récolte et chantier ; Maj met en
  file l'attaque et l'attaque-déplacement (Arrêt et Tenir immédiats) ; une attaque ordonnée bloque la file Maj.
  **Marche d'approche périmée** : `EngageSerial` (incrémenté par `Disengage`), marches tamponnées, les périmées
  rejetées (`TotalStaleCombatWalks`). Attaque-déplacement : recherche pendant la marche, reprise vers sa case après un
  détour (`bAttackMoveDetour`), Normal à 150 cm. Paysans sous ordre d'attaque : frappent. Client : clic droit sur un
  ennemi → `RequestAttackUnit` ; Q puis clic gauche (Maj = file ; clic droit ou Échap annule) ; repère rouge.
  Commandes `hol.Select.Attack`, `.AttackMove`, `.Stop`, `.Hold`. Grille : `EHOLGridCommand` AttackMove / Stop / Hold.
  `quick_check -Parts tests,menu,camera,move,formation,combat` PASS (144 s), **tests 78/78** ; partie « orders » :
  4 Paysans attaquent un Lancier (5 coups), S les arrête. Ouverts : localisation non compilée, scénarios
  attaque-déplacement et tenir, tests `CommandGrid`.
- **2e boucle** lancée (sous-agent, worktree) : ces trois points, `quick_check` complet.
- **J7b.3, 2e boucle faite** (sous-agent, ~25 min, rien de commité). Localisation refaite (GatherText, `fill_po.py` :
  en / ja 168 textes, 0 clé inconnue). Test **`CommandGrid.UnitOrders`** (Q / S / D aux cases 4 / 5 / 6 ; soldats : Q S D ;
  Paysans : A Z E R + S D ; mixte ; aucun bâtiment sur Q S D ; ennemi : rien ; Caserne : jamais Q S D). `quick_check`
  « orders » : **attaque-déplacement** (3 Hommes d'armes, Lancier à 4 m du chemin : engagé, tué en 10 coups, arrivée
  à moins de 4 m du but, écarts ≤ ~1,5 m) ; **tenir** (Paysan ennemi hors de portée : 0 cm, aucune cible ; Lancier au
  contact : 5 coups, 0 cm). `build hud` / `build barracks` attendent aussi S et D dans la grille des Paysans. **Délai
  du jeu de `quick_check` 480 → 660 s** (la séquence dure ~487 s). **`quick_check` complet PASS** (492 s, 3e
  lancement), **tests 79/79**.
- **J7b.4** lancée (sous-agent, worktree) : réseau du combat (J7-Q5).
- **J7b.4, 1re boucle faite** (sous-agent, ~35 min, rien de commité). **Bits** : 60 → **62** pour une unité non
  blessée hors combat (4e bit d'activité, bit « PV pleins ») ; +2 porteur, +4 palier si blessée, +12 cible si elle
  attaque, +6 front au repos (blessée qui attaque : 78). Activité : 8 attaque, 9 mourant, **10 « inactif face à son
  front »** (choix B : orientation sur 6 bits, ~5,6°) ; 11-15 libres. **Cible = numéro d'emplacement de réplication
  sur 12 bits** (4 095 = pas encore répliquée), pas l'identifiant. Palier = ⌈15 × PV / PV max⌉, 0 seulement à 0 PV.
  Envoi immédiat au changement de palier, d'activité, de cible ou de front. **Mourant** : 20 pas puis destruction ;
  population libérée à la destruction ; ni ciblé, ni ordres, ni sélection (collision gardée). Fichiers :
  `HOLUnitReplicationRules` (`SetCombat`, `GetFrontYaw`, `GetHealthLevel`, `IsSameAction`, `GetSendReason`),
  `HOLNetUnitStateNetSerializer`, `HOLUnitReplicationSubsystem`, `HOLUnitReceptionSubsystem` (`GetTargetUnitId`,
  `GetTargetEntity`), `HOLNetUnitDisplay` (tourné vers la cible ou le front), `HOLUnitView` (`bIncludeDying`),
  `DyingSteps = 20`. Test **`Net.CombatSendRule`**, tests `Net.*` étendus ; **tests 80/80**. `quick_check -Parts
  tests,combat,net` PASS (65 s) **mais `combat` n'a pas tourné** (dépend sans doute d'autres parties) et `net` ne
  vérifie aucun combat.
- **2e boucle** lancée (sous-agent, worktree) : combat vu par un client dans `net`, « Attaque » au bloc de sélection,
  `quick_check` complet.
- **J7b.4, 2e boucle faite** (sous-agent, rien de commité). `net` : **combat vu par le client 1** — le serveur
  (séquence `quick_net_server.seq`, `waitfile <Fichier> [S]` avec délai, `HOLTestSequence.cpp`) fait apparaître
  2 Hommes d'armes du slot 0 et 2 Lanciers du slot 1 sous la caméra du client 1 ; relevés toutes les 0,5 s : paliers
  qui ne font que baisser (#13 et #14 : 16 → … → 0), `Attacking` avec cible de l'autre slot, Lanciers `Dying` (≈ 6,5 s)
  puis retirés (≈ 14 s), Hommes d'armes restants, écart au serveur 5 cm ; capture `net_combat_c1` ; serveur : « seuls
  les 2 Lanciers tués ». **Bloc de sélection** : « Attaque » / « Attacking » / « 攻撃中 » pour une unité seule qui frappe
  (`ActivityAttacking`, localisation refaite, 169 textes). `combat` et `orders` inchangés avec le délai de mort.
  `quick_check` complet : tout PASS sauf `net` (ligne « Attaque » du HUD), corrigée puis `-Parts net` PASS (90 s) ;
  **tests 80/80**. `quick_check` complet relancé (session principale) pour confirmer.
- **`quick_check` complet de confirmation : PASS** (493 s), toutes les parties, `net combat` compris ; **tests 80/80**.
  J7b.4 faite (non commitée : J7b entière attend l'essai de l'utilisateur).

### 2026-10-19 — J7b.5 : présentation du combat

- **1re boucle** lancée (sous-agent, worktree) : barres de vie (toujours dans la sélection ; hors sélection si blessée,
  alliée ou ennemie — nuance J7-Q5) et PV au bloc de sélection. Le geste d'attaque et la chute suivront (aperçus à
  montrer à l'utilisateur, comme en J7a.3).
- **1re boucle faite** (sous-agent, rien de commité). **Barres de vie** : un composant instancié `HealthBars` dans
  `AHOLSelectionMarkers` (plan + `M_GroundFrame`, aucun asset nouveau ; fond + remplissage, face à la caméra, 230 cm
  au-dessus des pieds, 90 × 12 cm à 45 m, échelle bornée ×0,8 – ×1,8, hors du cône non tracées ; 1 draw call). Règle :
  toujours pour une unité sélectionnée ; hors sélection seulement si blessée ; jamais pour une mourante. Remplissage :
  PV exacts en solo et au serveur, palier / 15 chez un client. **Couleur** : vert pour les siennes, rouge pour les
  autres (pas d'alliés dans le code). Bloc : « PV x / y » pour une unité seule (client : `ceil(max × palier / 15)`),
  rien pour un groupe. `HOLUnitView::Gather` désormais à chaque image (partagé avec les anneaux) : à mesurer en fin
  de J7. Commande `hol.Units.Bars` ; `combat` et `net` vérifient la règle ; capture `combat_bars` (barres **petites** en
  vue haute). `quick_check -Parts tests,menu,camera,move,formation,combat,net` PASS (184 s), **tests 80/80**.
- Point à soumettre : les Paysans **frappent sur ordre** depuis J7b.3, alors que « Les Paysans combattent-ils ? »
  restait ouvert au catalogue de `AnimationsInProgress.md` (ligne « Attaque du Paysan »).
- **2e boucle** lancée (sous-agent, worktree) : coup de lance, coup d'épée, chute — poses et **aperçus seulement**
  (revue de l'utilisateur avant la fabrication dans Unreal, comme en J7a.3).
- **2e boucle faite** (sous-agent ; aperçus seulement, rien fabriqué dans Unreal, rien de commité). Dans
  `soldier_poses.py` : **`spear_attack`** (estoc à deux mains, boucle = cadence 1,875 s, 56 images à 30 i/s, pointe à
  179 cm devant à 124 cm de haut), **`maa_attack`** (taille en diagonale, bouclier levé, 1,375 s, 44 images à 32 i/s),
  frappe à **50 % de la boucle** (`ATTACK_HIT_FRACTION`, accordée à la préparation de 50 % du serveur) ;
  **`spear_fall`, `maa_fall`, `peasant_fall`** (1 s, 30 images, non bouclée ; bascule **en arrière**, arme **gardée
  en main** et posée au sol, bouclier au bras). Aperçus dans `HeroesOfLiberty-J5c\Saved\AnimPreview\` :
  `soldiers_attack.png`, `soldiers_attack_windup.png`, `troops_fall.png`, `troops_fall_mid.png` et les planches de
  chaque geste. **En attente de la revue de l'utilisateur** (sens de la chute, arme gardée ou lâchée, taille ou estoc
  pour l'Homme d'armes, amplitude de l'estoc), puis fabrication, fournisseurs, branchement sur Attack / Dying.
- **Arrêt des boucles autonomes** : la suite (fabrication des gestes, J7b.6 avec essai) attend l'utilisateur. Résumé
  dans `SummaryToReview.md` (racine du dépôt principal, non suivi).
- **Fin de séance** (demande de l'utilisateur) : prochaine étape écrite au point de reprise — l'utilisateur relit
  `SummaryToReview.md` et répond à ses questions, puis essai de J7b.

### 2026-10-19 — Revue de l'utilisateur (`SummaryToReview.md`)

- **Gestes validés tels quels** : chute **en arrière**, arme **gardée en main**, Homme d'armes en **taille
  diagonale**, estoc du Lancier **inchangé** (pointe à 179 cm). La fabrication dans Unreal peut suivre.
- **Paysans** : ils combattent **sur ordre, ou s'ils sont attaqués par une bête sauvage** (riposte réservée aux bêtes,
  pas aux soldats ennemis). Pas encore de bêtes sauvages dans le code : la riposte attendra qu'elles existent. Il faut
  un geste d'attaque pour le Paysan.
- Reste à revoir : choix faits sans l'utilisateur (Q / S / D, vert / rouge, population libérée 1 s après la mort,
  choix de J7b.3 et J7b.4), puis l'essai de J7b.
- L'utilisateur veut **tout tester** lui-même. Ajout de `hol.Units.SpawnHere [Count=5] [Type=Spearman] [Slot=1]
  [Radius=600]` (`HOLPlayerController.cpp`, développement seulement) : fait apparaître des unités autour du centre de
  la caméra, pour l'essai du combat. Compilé (20 s), jeu lancé depuis `HeroesOfLiberty-J5c` pour l'essai de J7b.

### 2026-10-19 — Essai de J7b par l'utilisateur

- **Tout est bien**, sauf trois retours :
  1. **Deux unités ennemies ne doivent pas pouvoir se repousser** : en combat, elles se poussent.
  2. **Attaque-déplacement sur A**, pas Q (AZERTY). **Sélection mixte Paysans + soldats** : la vue « Tous » montre les
     commandes des soldats (A = attaque-déplacement) ; la grille des Paysans reste dans leur sous-groupe (Tab). À noter
     pour plus tard : **touches attribuées selon la disposition du clavier** (AZERTY / QWERTY…).
  3. **Population libérée dès la mort**, pas à la disparition 1 s plus tard.
- Validés par l'essai : S / D, Maj, clic droit sur un ennemi, attaque-déplacement unité par unité, Arrêt et Tenir hors
  Maj, attaque ordonnée qui annule Tenir, barres de vie vert / rouge, bloc de sélection.
- Corrections lancées en sous-agents dans `HeroesOfLiberty-J5c` : d'abord touche A + population, puis la poussée.
- L'utilisateur : « le quick_check devrait être quick et pas durer 8 min ». **D327** : deux niveaux (rapide < 1 min par
  défaut, complet derrière `-Full` avant un commit d'étape et à une clôture). À faire après la poussée.
- **Touche A + population faites** (sous-agent, ~35 min, rien de commité). **A** : `AttackMoveCell` 4 → 0 ; la grille
  de construction n'apparaît plus s'il y a des soldats ; **sous-groupes d'unités (Tab)**, sur le modèle des bâtiments
  (`SplitSubgroups`) : seulement si plusieurs types, « Tous » puis soldats puis Paysans, sous-groupe retenu par son type ;
  Arrêt / Tenir / attaque-déplacement s'appliquent au sous-groupe actif ; panneau « Tous (N) » / « Paysan × N »
  (`HOLGameScreen.cpp`, textes existants). `CommandGrid.UnitOrders` réécrit. **Population** : `FHOLUnitRecord::bDying`
  (`MarkUnitDying`, au coup mortel ou au premier pas à 0 PV) ; `GetPopulationByOwner` ne compte plus les mourantes.
  Tests 80/80 ; `quick_check` complet PASS (510 s). Tab sur des unités et la ligne du sous-groupe : à l'essai. Note :
  avec `-Parts` partiel, `net 2 clients` échoue (dépendance entre parties, à régler avec D327).
- **Poussée entre ennemis corrigée** (sous-agent, rien de commité). Cause : `UHOLUnitSeparationProcessor` partageait
  tout recouvrement moitié-moitié, ennemis compris, et l'unité poussée, remise en file, poussait ses voisins (chaîne en
  mêlée). Règle pure `HOLCombat::GetOverlapShare(bEnemies, bWalkingA, bWalkingB)` : alliés moitié-moitié ; contre un
  ennemi à l'arrêt, l'unité qui marche (ou poussée par son camp) prend tout, l'ennemi ne bouge pas ; deux ennemis qui
  marchent : moitié chacun. Tableau `Owners` dans la passe (aucune recherche de voisins en plus). Test
  `Combat.Push` (**81/81**) ; `quick_check` partie `orders` : scénario « push » (12 Paysans ennemis à l'arrêt, 3 Hommes
  d'armes : le mur bouge de **0 cm**, avant jusqu'à 168 cm), « hold » resserré à 20 cm. Parties `tests, camera, move,
  formation, combat` puis `camera, combat` PASS. Effet : les Hommes d'armes contournent le mur au lieu de le traverser
  (5 coups en 10 s au lieu de 12). S2 à mesurer en fin de J7.
- Jeu relancé pour l'essai des trois corrections.
- **Essai des trois corrections : validé** par l'utilisateur (« ça a l'air bon »). Worktree avancé sur `main`
  (7c2c371, traces Insights d'une autre session, sans fichier commun), recompilé ; `quick_check` complet (la version
  complète, D327 ; encore le défaut du script) lancé avant le commit de J7b.2 à J7b.5.
- `quick_check` complet **PASS** (526 s, toutes les parties, **tests 81/81**). **J7b.2 à J7b.5 commitées : ce6a4d2**
  (dans le worktree, `main` avancé dessus). Suite : D327, puis les gestes, puis J7b.6.
- **D327 fait** (sous-agent, rien de commité ; seul `Tools/Tests/quick_check.ps1` changé). **Défaut = `tests` +
  `smoke`** : une partie solo à 2 joueurs (écrans titre et Nouvelle partie en français, lancement, caméra au-dessus du
  slot 0 et flèche haut, sélection du centre et clic dans le vide, 3 Hommes d'armes qui tuent un Paysan ennemi puis
  arrivent au point) : **PASS en 23 à 31 s**, tests 81/81. **`-Full`** = l'ancien défaut (sans `smoke`) : PASS en
  510 s. **`-Parts`** : liste découpée aux virgules (avec `-File`, `net,sources` arrivait en une seule chaîne : cause
  probable de l'« échec de `net` en partiel ») ; `-Parts net` seul PASS (92 s) ; les parties solo ajoutent `camera`
  d'elles-mêmes (non essayé seul). Délai maximum 660 s avec `camera`, sinon 120 s.
- D327 commitée (validée par l'utilisateur). Suite : fabrication des gestes de combat validés et du coup du Paysan.
- Fabrication des gestes lancée (sous-agent, worktree) : `spear_attack`, `maa_attack`, trois chutes, branchement sur
  Attack / Dying ; coup du Paysan en **aperçu seulement** (revue de l'utilisateur avant fabrication).
- **Gestes de combat fabriqués et branchés** (sous-agent, **19 min, 276k tokens** : au-delà de la limite de 15 min ;
  rien de commité). `A_Spearman_Attack` (56 images à 30 i/s, 1,867 s), `A_ManAtArms_Attack` (42 images, 1,4 s ; frappe
  sur l'image 21), `A_Spearman_Fall`, `A_ManAtArms_Fall`, `A_Peasant_Fall` (1 s) ; fournisseurs refaits (soldats :
  4 attaque, 5 chute ; Paysan : 12 chute) ; colonnes `AttackSequence` / `FallSequence` dans la table. **Choix** :
  30 i/s imposé par Unreal, donc l'attaque est jouée à la vitesse longueur / `AttackInterval` (une boucle = une
  cadence, frappe à 50 %) ; boucle calée sur le prochain coup du serveur en solo / hôte, et sur un coup qui commence
  chez un client (décalage possible si changement de cible en gardant le rythme, non essayé) ; **en solo, celui qui
  frappe se tourne vers sa cible** (ajout non validé) ; le Paysan qui frappe joue son repos en attendant son geste.
  Chute jouée une fois puis figée. Outils : `anim_rig.py` (clé `frames`), `make_peasant_animations.py` relit aux temps
  de la séquence. Test `Animation.Strike` (**82/82**), `quick_check` rapide PASS (27 s), `-Parts combat` PASS (109 s,
  « combat anims »). Captures `Saved_Tests\Screenshots\WindowsEditor\HOL\combat_melee.png`, `combat_fall.png`.
- **Bilan de la règle des 15 min** : la tâche mêlait trois livrables (fabrication, branchement, aperçu du Paysan).
  Amélioration : **un sous-agent = un seul livrable** ; l'aperçu du Paysan part dans une boucle à lui.
- Reste : aperçu du coup du Paysan (boucle suivante), mise à jour du catalogue de `AnimationsInProgress.md`, essai de
  l'utilisateur, commit.
- **Aperçu du coup du Paysan fait** (sous-agent, **5 min, 105k tokens** : dans les limites ; rien de fabriqué ni
  commité). `PEASANT_ATTACK` dans `soldier_poses.py`, rangé dans `PROPOSED` (hors `GESTURES`, donc hors fabrication
  tant que l'utilisateur n'a pas validé) ; `preview_animation.py` connaît `PROPOSED`. **Coup de hache à deux mains en
  diagonale** (la hache de la coupe du bois, aucune pièce nouvelle ; frappe au torse à ~1,1 m, garde et fin de geste,
  pieds fixes), boucle **2 s** (`AttackInterval` du Paysan), 60 images, frappe image 30 (50 %), armement à 40 %. Pour
  la fabrication : passer dans `GESTURES` et afficher la hache pendant l'attaque (colonne `AttackProp` sur le modèle de
  `BuildProp`). Aperçus : `peasant_attack_contact_sheet.png`, `peasant_attack_zoom.png`, `peasant_attack_hit.png`,
  `peasant_attack_windup.png` (`HeroesOfLiberty-J5c\Saved\AnimPreview\`). En attente de la revue de l'utilisateur, avec
  l'essai des gestes des soldats.

### 2026-10-19 — Essai des gestes de combat

- Retours de l'utilisateur :
  1. **Gel de 2-3 min en lançant un Homme d'armes** : diagnostic confié à un sous-agent (journal, piste : compression
     d'animation / DDC au premier chargement des séquences fabriquées aujourd'hui).
  2. **Les gestes marchent mais font « vraiment robot »** : question posée à l'utilisateur.
  3. **Demande** : le **point de ralliement** doit pouvoir se poser sur un bâtiment **fantôme** (fondation posée,
     personne n'a encore donné de coup de marteau). À faire.
- « Robot » vise **l'Homme d'armes et le Lancier** (gestes vus en jeu) ; le coup du Paysan n'est pas encore revu
  (aperçu seulement). Choix de l'utilisateur : **polir sur un geste d'essai** (attaque de l'Homme d'armes) :
  accélérations et ralentis, bassin qui mène et bras en retard, dépassement après le coup, puis désynchronisation par
  unité (décalage et petite variation de vitesse). Boucle 1 : le geste poli en **aperçu animé** (avant / après), sans
  fabrication.
- **Diagnostic du gel** (sous-agent, dans les limites ; rien modifié). Un seul gel, **42 s** dans le journal (image
  359, 17:34:00 → 17:34:42 UTC), à la naissance du premier Homme d'armes produit à la Caserne :
  `UHOLUnitSubsystem::FindTemplate` charge **de façon synchrone, à la première naissance d'un type**, le maillage et le
  fournisseur (`TryLoad`) ; en jeu non cuit, ça passe par le DDC (Zen). Or le **zenserver commun** (port 8558) venait
  d'être tué avec son parent, l'**éditeur sans fenêtre de ProjectShiva** (autre session, arrêt brutal entre 17:33:28 et
  17:34:00) : le jeu a relancé Zen et l'a attendu 39 s (relance lente : 826 sessions accumulées dans
  `Zen\Data\sessions`). Pas de compression d'animation ni de shader. **Version cuite** : pas de DDC, donc pas ce gel,
  mais l'à-coup du chargement synchrone reste (0,23 s pour le Lancier en non cuit). **Correctif jeu** : précharger les
  types d'unités au chargement de la carte. **Poste** : l'outil Shiva ne doit pas tuer les enfants de son éditeur, ou
  Zen lancé à part, ou un Zen propre à HeroesOfLiberty (clés à vérifier).
- **Attaque de l'Homme d'armes polie, en aperçu** (sous-agent, 8 min, 111k tokens ; rien fabriqué ni commité).
  **Cause de la raideur** (`anim_rig.Rig.sample`) : arrêt à vitesse nulle à chaque pose clé (main en ligne droite),
  tous les os partent ensemble, ni anticipation ni dépassement, bouclier et bassin passifs. **Outils** (`anim_rig.py`,
  inactifs par défaut, autres gestes inchangés) : `"curve": "hermite"` avec un `"flow"` par pose (0 arrêt, 1 traversée
  en arc, > 1 fouet, paire arrivée / départ) ; `"leads"` (avance / retard par os, `"loop": True`), valeurs exactes sur
  les poses clés (frappe inchangée, image 21). **`maa_attack`** : anticipation, épée qui monte plus loin et marque un
  temps, taille à pleine vitesse au coup (36 m/s), rebond et garde dépassée, poids pied arrière → genou avant ; bassin
  en avance de 6 %, bouclier en retard de 4 %. Aperçu `HeroesOfLiberty-J5c\Saved\AnimPreview\maa_attack_before_after.gif`
  (avant / après, côté + RTS). Reste : validation de l'utilisateur, puis Lancier, fabrication, essai.
- **Préchargement des types d'unités fait** (sous-agent, dans les limites ; rien de commité). `UHOLUnitSubsystem::
  OnWorldBeginPlay` → `PreloadTypes` : `FindTemplate` pour chaque ligne, **synchrone au chargement de la carte**
  (cartes à emplacements de départ seulement ; pas le menu ni le test de charge). Solo / serveur : gabarits simulés ;
  client : gabarits d'affichage ; serveur dédié : ni maillage animé ni statique (fournisseur gardé pour les durées).
  Journal « Unit types preloaded: 6 of 6 … in 631 ms » (clients ~570 ms, serveur 646 ms) ; avertissement « built after
  the preload ». `smoke` vérifie les deux lignes. Compilé ; `quick_check` rapide PASS (61 s, tests 82/82), `-Parts
  net` PASS (96 s).
- **Point de ralliement d'un fantôme fait** (sous-agent, dans les limites ; rien de commité). Blocage : le serveur
  (`UHOLProductionSubsystem`) ignorait production et ralliement sur tout bâtiment non fini (D312) ; le client envoyait
  déjà l'ordre. **Règle** : un bâtiment qui produira (`Produces` non vide) accepte le ralliement dès son fantôme (et en
  chantier) ; production toujours refusée avant la fin ; maison non finie : refusé. Point gardé jusqu'à la fin (même
  acteur), visible du seul propriétaire. Groupe de bâtiments (F) : le clic droit pose aussi le point des producteurs
  non finis. `quick_check` : séquence **`build barracks ghost`** (Caserne fantôme, point posé avant le premier coup,
  construite avec FastBuild, l'Homme d'armes produit arrive à < 4 m du point) PASS ; correction de `-Parts build` seul
  (attente « Selection: none » jamais écrite, remplacée par une marque `build none`). `quick_check` rapide PASS (22 s).
  `-Parts build` relancé par la session principale pour confirmer `camera 2p`.
- `-Parts build` relancé : **PASS** (261 s ; `camera 2p`, `camera 8p` et les 9 séquences `build`, `barracks ghost`
  compris). En attente : avis de l'utilisateur sur `maa_attack_before_after.gif`, puis essai (préchargement, ralliement
  sur fantôme) et commit.
- Avis de l'utilisateur sur le GIF : **plus de mouvement du bassin pour faire une vraie taille**. Boucle suivante :
  rotation du bassin (armement en arrière, rotation à travers la taille), nouveau GIF.
- **Bassin de la taille** (sous-agent, dans les limites ; rien fabriqué). `hip_turn(twist, bend, hip)` dans
  `soldier_poses.py` (bassin + trois os du dos, tête qui compense) sur les poses de l'attaque ; avance du bassin 7 %.
  Rotation **bassin 30°** (avant 13,6°), **épaules 69°** (avant 60,6°) ; bassin qui mène (image 18 : bassin −11°,
  épaules −1°) ; poids −7 cm à l'armement → +8 cm au coup. Pieds, poignée, boucle : 0 cm ; frappe inchangée (image 21,
  même point, 35,7 m/s). L'épée effleure le haut du bras droit (≤ 0,9 cm, images 19-20). GIF trois vues (côté, RTS,
  dessus avec lignes hanches / épaules) : `HeroesOfLiberty-J5c\Saved\AnimPreview\maa_attack_hips.gif`.
- Avis de l'utilisateur : **non** — « il faut que la garde de l'épée se trouve devant la position du bouclier, que le
  haut du corps se tourne en fait ». Compris : à la fin de la taille, la poignée de l'épée passe **devant le bouclier**
  (côté gauche), portée par une vraie rotation du haut du corps, pas seulement par le bras. Boucle suivante.
- **Haut du corps** (sous-agent, dans les limites ; rien fabriqué). `turned()`, `shield_turned()` (le bouclier tourne
  avec le tronc, bras gauche qui s'ouvre), `SHIELD_OPEN`, `SHIELD_BACK` ; poses d'armement, taille, suivi, rebond
  reprises. **Épaules 124°** (+54° à l'armement → −70° au suivi), **bassin 39°**. Fin du suivi (image 25) : **garde de
  l'épée devant le bouclier** (garde −36 / 45 / 104 cm, bouclier −37 / 12 / 127), bras droit croisé. Pieds, poignée,
  boucle : 0 cm ; frappe toujours image 21 (pointe −40 / 115 / 89) ; épée et bras jamais à travers le bouclier. Bouclier
  assez loin à gauche aux images 27-28 (réglable). GIF : `HeroesOfLiberty-J5c\Saved\AnimPreview\maa_attack_torso.gif`.
- **Attaque de l'Homme d'armes (haut du corps) validée** en aperçu (« ça m'a l'air bien »). Suite : polir l'estoc du
  Lancier de la même façon (aperçu GIF), puis fabriquer les deux, puis essai (avec préchargement et ralliement sur
  fantôme) et commit.
- **Estoc du Lancier poli** (sous-agent, dans les limites ; rien fabriqué). Hermite + flow + leads, `hip_turn()` ;
  poses GUARD, SET (anticipation), DRAWN, LOADED (armé, temps d'arrêt), LUNGE (frappe, 50 %), OVER (dépassement 4 cm),
  BACK, SETTLE. **Bassin 23°** (avant 6°), **épaules 54°** (avant 25°) ; le bassin pousse d'abord (11 cm sur le genou
  avant), puis le tronc, puis les bras. Frappe identique (image 28, pointe à 205 cm devant, 133 cm de haut ; les
  179 / 124 cm étaient la pointe en garde, `preview_animation` mesure l'image 0 pour une arme à `"edge"`). Pieds et
  boucle : 0 cm ; mains sur la hampe 4,8 cm max (5,0 avant), 0 à la frappe. GIF :
  `HeroesOfLiberty-J5c\Saved\AnimPreview\spear_attack_polished.gif`.
- Avis de l'utilisateur sur l'estoc : **la pointe plus au centre** (elle est à 17 cm à droite à la frappe) et **pas
  tout le corps en même temps** (enchaînement plus marqué : jambes → bassin → tronc → bras). Boucle suivante.
- **Estoc v3** (sous-agent, dans les limites ; rien fabriqué). Pointe **au centre** à la frappe (droite 0, avant
  205, haut 133 cm ; garde 0,4 cm ; −1 à +5 cm sur tout le cycle) ; lance à 5-6° de l'axe (droite sur l'axe, la hampe
  traversait le ventre). **Enchaînement** : leads augmentés (avance du bassin 0,04 → 0,12, rotation 0,08, dos
  0,065 / 0,05 / 0,038, clavicule 0,028, coudes 0,014) ; départs : poids image 19, bassin 21, tronc 22, bras et lance
  26 (écart 7 images au lieu de 1), retour dans le même ordre. Pieds et boucle 0 cm ; mains 4,1 cm max, 0 à la frappe ;
  hampe qui frôle le corps (3 points à 0,4 cm, images 32-34). GIF `HeroesOfLiberty-J5c\Saved\AnimPreview\spear_attack_v3.gif`.
- **Estoc v3 validé** (« très bien »). Fabrication des deux attaques polies lancée (sous-agent).
- **Attaques polies fabriquées** (sous-agent, dans les limites ; rien de commité). `A_ManAtArms_Attack` (42 images,
  frappe image 21, arme jusqu'à 37,3 m/s) et `A_Spearman_Attack` (56 images, frappe image 28) refaites en place
  (fournisseurs inchangés). Relecture compressée : os ≤ 0,024 cm, tête de l'arme ≤ 0,009 cm, racine et boucle 0 cm ;
  la compression ne déforme pas le fouet. Nouveau contrôle dans `make_peasant_animations.py verify()` (tête de l'arme,
  vitesse max, image de frappe). `quick_check` rapide PASS (28 s), `-Parts combat` PASS (110 s). Jeu lancé pour
  l'essai (attaques, préchargement, ralliement sur fantôme).
- **Essai validé** (« très bien ») : attaques polies, plus de gel, ralliement sur fantôme. `quick_check -Full` PASS
  (524 s, tests 82/82). **Commit 258c5d0** (worktree, `main` avancé dessus). Catalogue de `AnimationsInProgress.md` mis
  à jour. Reste avant J7b.6 : polir et faire revoir le coup du Paysan (puis fabrication, hache affichée), décalage des
  unités entre elles (optionnel).
- **Coup de hache du Paysan poli** (sous-agent, dans les limites ; rien fabriqué, reste dans `PROPOSED`). Hermite +
  flow + leads + `hip_turn()` ; poses garde, anticipation (12 %), montée (36 %), armé suspendu (43 %), mains tirées /
  hache qui traîne puis fouette (47 %), frappe image 30, suivi (58 %), rebond (66 %), garde dépassée (86 %). **Bassin
  41°**, **épaules ~100°** (avant 63°) ; départs : poids 21, bassin 22, tronc 23, main et hache 27 ; tête de hache à
  la frappe dans l'axe (−0,2 / 113,5 / 122 cm). Pieds et boucle 0 cm, mains 0,7 cm, manche jamais dans le corps.
  GIF `HeroesOfLiberty-J5c\Saved\AnimPreview\peasant_attack_polished.gif`, en attente de la revue de l'utilisateur.
- Avis de l'utilisateur : **le Paysan attaque au couteau**, pas à la hache. Geste choisi : **coup de pointe à une
  main** (épaule droite qui avance, couteau planté vers le ventre, main gauche en avant ; porté par le bassin et le
  tronc). Couteau visible seulement pendant l'attaque (comme le marteau). Boucle suivante : pièce couteau + geste, GIF.
- **Coup de couteau du Paysan en aperçu** (sous-agent, dans les limites ; rien fabriqué). Pièce `make_knife()`
  (`peasant_props.py`, id 8, os `ik_hand_gun`, masque 128 = **dernier bit libre**) : manche 10 cm, garde, lame 20 cm.
  Geste `PEASANT_ATTACK` (toujours dans `PROPOSED`) : garde poing gauche levé, anticipation, armé (épaule droite
  reculée, temps marqué), frappe image 30 (bras tendu, poing gauche devant le visage), suivi 4,5 cm, retrait, garde
  dépassée ; bassin 29°, épaules 69° ; départs poids 21, bassin 22, tronc 23, bras 27. Pointe à la frappe : 2,2 cm de
  l'axe, 77 cm devant, 105 cm de haut. Contrôles 0 cm. Couteau petit vu de la caméra RTS (épaississable). GIF
  `HeroesOfLiberty-J5c\Saved\AnimPreview\peasant_attack_knife.gif`. Fabrication : maillage refait, geste dans `GESTURES`,
  colonne `AttackProp` (8 pour le Paysan) sur le modèle de `BuildProp`.
- Avis de l'utilisateur : le Paysan doit avoir une **position de peur, ce n'est pas un combattant**. Boucle suivante :
  posture craintive (voûté, tête rentrée, poids en arrière, bras gauche en protection), coup hésitant puis recul vif.
- **Couteau en posture de peur** (sous-agent, dans les limites ; rien fabriqué, toujours dans `PROPOSED`). `afraid()` :
  dos voûté, épaules remontées, tête rentrée et détournée 8-14° ; bassin en arrière (−5 à −7 cm) et plus bas, couteau
  serré à la hanche, avant-bras gauche devant le visage. Rythme : garde qui tremble (~1 cm), faux départ (14-19), se
  ramasse (25), coup court surtout du bras (bassin 12°, poids +8 cm, sans fente) image 30, recul vif (retour plus
  rapide que l'aller), se recroqueville (43). Pointe 3,5 / 67 / 103 cm. Contrôles 0 cm. GIF
  `HeroesOfLiberty-J5c\Saved\AnimPreview\peasant_attack_fear.gif`.
- **Couteau en posture de peur validé** par l'utilisateur, commité en aperçu (f7b0b27). **Fin de séance** (demande de
  l'utilisateur) : reprise à « Prochaine étape exacte » (fabrication du coup du Paysan).

### 2026-10-20 — Fabrication du coup de couteau du Paysan

- Reprise. Worktree `HeroesOfLiberty-J5c` propre, au niveau de `main` (b9d91b8), aucun Unreal en cours. Fabrication lancée
  (sous-agent, un livrable, 15 min / 300k tokens) : maillage avec le couteau, geste dans `GESTURES` et fabriqué,
  colonne `AttackProp` sur le modèle de `BuildProp`, Paysan qui joue son attaque, `quick_check` rapide + `-Parts combat`.
- **Coup de couteau fabriqué et branché** (sous-agent, 14 min, 135k tokens : dans les limites ; rien de commité).
  `peasant_attack` dans `GESTURES` (`PROPOSED` vide), geste inchangé ; `A_Peasant_Attack` (60 images, 2,000 s, frappe
  image 30, tête du couteau 4,8 m/s max ; `verify()` : 0,003 cm à la frappe en compressé, boucle et racine 0 cm ; mains
  à l'axe du manche ≤ 0,48 cm). Séquence 13 de `SKM_Peasant` / `TP_Peasant` (14 séquences) ; couteau (pièce 8) à tous
  les LOD. Colonne `AttackProp` (`HOLUnitTypes.h`, contrôle dans `HOLUnitSubsystem.cpp`, `Choose` : `Prop = AttackProp`
  pendant `Attacking`, donc ni hache, ni pioche, ni charge en frappant) ; CSV : Paysan `AttackSequence` 13,
  `AttackProp` 8, les autres 0. Tests `Animation.Strike` et `Animation.Table` étendus. Compilation sans avertissement,
  tests 82/82, `quick_check` rapide PASS (34 s), `-Parts combat` PASS (119 s).
- Capture du Paysan au couteau **non obtenue** : l'ajout à `quick_check` (capture `combat_peasant`, contrôle
  `A_Peasant_Attack prop 8`) faisait partir le clic droit des Paysans sur le Lancier en ordre au sol (2 échecs sur 2) ;
  ajout retiré, patch gardé dans le scratchpad de la séance. Vérification par l'essai de l'utilisateur à la place.
- **Essai : coup de couteau validé** (« super le stab »). `quick_check -Full` : tout PASS (550 s, tests 82/82) sauf
  `net 2 clients` (`[KO] client 1 : health bars … none for an unhurt unselected (1)` : une barre de vie sur une unité
  intacte non sélectionnée, sans lien avec le couteau) ; `-Parts net` relancé seul : **PASS** (102 s) → aléa, à
  surveiller. **Commit 33f8d63** (worktree, `main` avancé dessus). Catalogue de `AnimationsInProgress.md` mis à jour.
- **Nouvelle demande de l'utilisateur** (à faire) : **lignes entre les actions en file** (Maj) : plusieurs ennemis à
  attaquer, plusieurs destinations, plusieurs bâtiments à construire, ou un mélange. Une ligne relie chaque action à la
  suivante, colorée selon l'action : **rouge** attaque, **blanc** construction, **jaune** déplacement.
- Ordre choisi par l'utilisateur : ces lignes, puis J7b.6. Lecture du code (sous-agent) : deux files sur le serveur seul,
  sans accesseur public (déplacements / attaques de groupe dans `HOLUnitOrderProcessor` : `QueuedMoves`,
  `MoveQueueOfUnit`, tête retirée quand elle démarre ; travail des Paysans dans `HOLBuildProcessor::Queues` : chantiers
  et sources) ; **rien n'est répliqué** (le client ne connaît ni la file ni la destination de l'ordre en cours) ; aucune
  ligne au sol aujourd'hui (repères en plans instanciés de `AHOLSelectionMarkers`, `M_GroundFrame`).
- **D328 — lignes de la file d'ordres** (questions une par une, recommandations retenues sauf la couleur de la récolte) :
  1. visibles pour les **unités sélectionnées** (comme AoE4), effacées à la désélection, chaque étape faite s'efface ;
  2. **une chaîne par file commune** : part du centre des unités qui la portent (suit le groupe en marche), puis relie
     l'action en cours et chaque action en file ; des Paysans aux files différentes ont chacun leur chaîne ;
  3. couleurs : **jaune** déplacement, **rouge** attaque d'un ennemi et attaque-déplacement (rouge supposé pour A,
     sans objection), **blanc** construction, **bleu** récolte (choix de l'utilisateur) ;
  4. réseau : **le serveur envoie au seul propriétaire** les files de ses unités, **seulement quand elles changent**
     (ajout, étape commencée ou faite, cible morte, chantier fini) ; rien en régime.
- Découpage (un sous-agent = un livrable, 15 min) : **J7b.5b-1** serveur : chaînes (action en cours + file, deux files
  réunies) en règle pure testée, envoi au propriétaire sur changement, commande de rapport ; **J7b.5b-2** dessin des
  chaînes chez le client (`AHOLSelectionMarkers`) ; **J7b.5b-3** contrôles solo et 2 clients dans `quick_check`,
  capture, essai de l'utilisateur.
- Avant J7b.5b-1 : les **3 correctifs confirmés de la revue de code de J7b** (ajoutés au journal par une autre séance),
  le premier bloquant la file Maj après une cible tuée (les lignes l'auraient montrée figée). Lancés en un sous-agent
  (un livrable : les trois correctifs et le test du processeur, `quick_check` rapide + `-Parts combat`).
- **Correctifs faits** (sous-agent, 8 min mesurées par l'outil — il s'en croyait à ~18 et s'est arrêté avant les
  tests ; compilation de 211 s ; 100k tokens ; rien de commité, aucun test lancé par lui). (1) `HOLCombatProcessor.cpp` : `bOrderedTarget = false` au coup mortel (autres
  chemins déjà justes) ; pas de test de processeur Mass possible simplement → scénario en jeu dans la partie combat
  de `quick_check` (« orders queue » : clic droit sur A, Maj + clic droit sur B, A puis B tués ; « orders queue seek » :
  Paysan C à 6 m pris à partie). (2) `HOLGameScreen.cpp` `RefreshUnitHealth` : `SetShown` avant la sortie anticipée.
  (3) `SetUnitSelection(Units, ActiveType)` : type actif posé avant la diffusion, `PurgeSelection` le passe. Risque
  signalé : après « push », les Hommes d'armes ne sont plus figés et peuvent attaquer le mur de Paysans, ce qui peut
  fausser l'attente de « combat fall ». Bilan des 15 min : le sous-agent surestime son temps ; lui donner l'heure de
  départ et lui faire lire l'horloge (`Get-Date`) avant de s'arrêter. Tests lancés par la session principale.
- Tests des correctifs : `quick_check` rapide **PASS** (43 s, tests 82/82) ; `-Parts combat` **FAIL** (183 s) : délai
  dépassé à l'étape 321 (`waitlog Selection: 3 units (own): ManAtArms x3`, le nouveau scénario de file), d'où
  `camera 8p` en échec et `[KO] queue: A ordered, B queued`. Hypothèse : les Hommes d'armes, plus figés après
  « push », quittent leur place. Mise au point du scénario confiée à un sous-agent (heure de départ, `Get-Date`).
- **Scénario de file fiable** (sous-agent, 12,5 min, 95k tokens ; seul `quick_check.ps1` changé ; correctif de
  combat **hors de cause**, il fait ce qu'on attend). Cause : les 3 Hommes d'armes frappaient encore le Paysan de
  « combat fall », hors du cadre de sélection ; et « combat fall » captait la mort d'un Paysan du mur (désormais tué
  par les Hommes d'armes du « push », plus figés). Changements : « combat fall » avant « push » ; 3 Hommes d'armes
  neufs pour la file, A / B / C sur terrain dégagé (`$QueueMenAtArms`, x −2000) ; `wait 0.3` avant le rapport de C ;
  Lancier d'« orders attack » né à 10 cm de son point (clic droit parfois au sol). `-Parts combat` **PASS** (152 s ;
  `queue: A #39, B #40 targeted in 7 reports, C #41`), rapide **PASS** (33 s, 82/82). Rien de commité : les correctifs
  attendent l'essai de l'utilisateur, avec les lignes (ligne des PV et grille à regarder aussi).
- **J7b.5b-1a lancée** (sous-agent) : règle pure des chaînes (`Orders.Chains`), collecte sur le serveur depuis les deux
  files (action en cours comprise), commande `hol.Orders.Chains`. **Précision de D328** (choix de la session, à
  confirmer à l'essai) : une unité n'a de chaîne que si elle a au moins une action en file, ou en a eu une depuis son
  dernier ordre sans Maj (la ligne reste jusqu'à la fin de la dernière étape) ; un simple ordre sans Maj ne fait
  aucune ligne. Coût nul sans file (S1 / S2 / S7). L'envoi au propriétaire est la boucle suivante (J7b.5b-1b).
- **J7b.5b-1a faite** (sous-agent, 11 min, 115k tokens ; rien de commité). `Simulation/HOLOrderChainRules.h/.cpp`
  (`HOLOrderChain::MakeChains` : étapes identiques → une chaîne, ordre stable, `MaxSteps = 16`), test
  `Orders.Chains` (**83/83**). Collecte : `UHOLUnitOrderProcessor::GetChainSteps` (file de groupe + action en cours
  notée à la première mise en file : cible ordonnée, ou ordre marché retrouvé par `OrderSerial` dans `OrderSteps`,
  table bornée à 256 → 128 ordres ; tête démarrée = action en cours) ; `UHOLBuildProcessor::GetChains` (drapeau
  `FBuilderQueue::bChain`, posé par Maj, effacé sans Maj ; la file de travail prime). Fin de chaîne : file vide, unité
  arrêtée et hors combat, 10 pas de grâce. Coût nul sans file (seules les tables sont lues). `hol.Orders.Chains
  [Slot]` ; `hol.Units.Move` 4e argument `q|a|qa`. Essai solo (combat coupé) : `Move -> Move -> AttackMove`, puis
  `AttackMove` seul, puis 0 après un ordre sans Maj. Rapide PASS (33 s). Pas encore essayé : chaînes des Paysans,
  attaque d'un ennemi en file. **J7b.5b-1b lancée** (envoi au seul propriétaire sur changement, rapport chez le client).
- **J7b.5b-1b faite** (sous-agent, 10,5 min, 110k tokens ; rien de commité). `AHOLPlayerState::OrderChains`
  (`COND_OwnerOnly`, `OnRep_OrderChains`, `GetOrderChains()`, `OnOrderChainsChanged`, `SetOrderChains()` qui ne
  réaffecte que sur changement, `LocateChainStep()` : attaque placée sur l'unité visée affichée, sinon repli) ;
  `FHOLNetOrderChain { UnitIds, Steps }`, `FHOLNetChainStep { Kind 8 bits, Location FVector_NetQuantize, TargetId
  (attaque seulement) }` : ~8-10 o par étape, ~40-50 o pour 3 unités et 3 étapes, par changement (estimé) ; une cible
  qui marche ne provoque pas de renvoi. `PublishChains()` après `StartQueuedMoves`, 0,25 s au plus, ne fait rien sans
  file (sauf vider une fois). `hol.Orders.Chains` : `Order chains (received) ...` chez le joueur local. Rapide PASS
  (36 s, 83/83). **Essai 2 clients** (serveur dédié, `-nullrhi`) : le client du slot 0 reçoit `Move -> Move ->
  AttackMove`, puis la chaîne raccourcie, puis 0 ; le client du slot 1 reçoit toujours 0. Non essayés : attaque en
  file (cible affichée chez le client), Build / Gather en réseau. **J7b.5b-2 lancée** (dessin, captures).
- **J7b.5b-2 faite** (sous-agent, 10 min, 108k tokens ; rien de commité). `AHOLSelectionMarkers::UpdateOrderLines`
  (composants `OrderLines`, `OrderPoints`) : chaînes dont une unité est sélectionnée ; départ = centre affiché des
  unités de la chaîne ; segments pleins (`M_GroundFrame`) de **25 cm** (`hol.Orders.LineWidth`) à 1,5 cm du sol,
  inclinés entre les hauteurs des extrémités (pas d'échantillon au milieu) ; disque de 30 cm à chaque étape (2,2 cm) ;
  anneaux et fanion au-dessus (3 cm). Couleurs : Move (0,95 0,8 0,05), Attack / AttackMove (0,9 0,08 0,05),
  Build (0,9 0,9 0,9), Gather (0,1 0,4 0,95). Coût nul sans chaîne ni sélection. Rapide PASS (39 s, 83/83).
  Captures `HeroesOfLiberty-J5c\Saved_Tests\Screenshots\WindowsEditor\HOL\ol_army.png` (jaune → rouge → rouge → jaune,
  conforme) et `ol_peasant.png` (blanc vers la maison en file, bleu vers le bois ; le premier déplacement sans Maj
  ne se voit pas : Paysan peut-être déjà arrivé, non vérifié). Jeu lancé pour l'essai (lignes + 3 correctifs).
- **Essai de l'utilisateur** (lignes) : (1) **la ligne doit s'afficher même pour un seul ordre** (sans Maj) →
  **D328 précisée**, point 1 : toute action ordonnée d'une unité sélectionnée a sa ligne (remplace la précision « au
  moins une action en file » de la session) ; (2) **défaut** : avec P1, P2, P3 en file, l'unité part vers le point
  le plus proche au moment du départ et saute ceux d'avant (P2 plus proche → P1 disparaît, P2 puis P3) ; attendu :
  P1 → P2 → P3 dans l'ordre. Défaut d'abord (sous-agent : régression du jour ou ancien, cause, correctif, contrôle),
  puis la ligne d'un seul ordre (coût à surveiller : les unités ordonnées sans file ont désormais une chaîne ; S2 à la
  fin de J7). Pas d'avis encore sur les 3 correctifs de la revue.
- **Points sautés corrigés** (sous-agent, 11,5 min, 100k tokens ; rien de commité). Défaut **ancien (J7a.4)**, pas du
  jour, aucune règle ne le voulait. Cause (`HOLUnitOrderProcessor.cpp`, `StartQueuedMoves` ~l. 1944-1990) : un ordre
  en file qui démarre passe par `StartedQueuedMoves`, appliqué au pas suivant ; pendant le reste du passage l'unité
  semble arrêtée, l'ordre suivant (plus loin dans la `TMap`) part dans la même trame et l'écrase (P2 sauté, chemin
  direct vers P3 : impression de « point le plus proche »). Correctif : `TSet<uint32> StartedNow` (une unité qui
  vient de démarrer un ordre n'est pas prête pour le suivant avant le pas d'après). File de travail des Paysans déjà
  dans l'ordre. Contrôle **« fifo »** dans la partie `move` de `quick_check` (4 Paysans, P2 plus proche que P1 : centre
  du groupe à < 3,5 m de P1, puis P2, puis P3) : `-Parts move` PASS (54 s, « P1, P2, P3 reached at reports 3, 7, 8 »),
  rapide PASS (40 s, 83/83). Avec `-nullrhi` les séquences s'arrêtent à `waitui` : essais en `-RenderOffScreen`.
- **Ligne d'un seul ordre lancée** (sous-agent) : toute action ordonnée par le joueur (déplacement, attaque,
  attaque-déplacement, construction, récolte) a sa chaîne ; rien pour l'automatique (riposte, retour à sa place,
  aller-retour de récolte) ; coût proportionnel aux unités ordonnées, stress tests à vérifier.
- **Ligne d'un seul ordre faite** (sous-agent, 14 min, 110k tokens ; rien de commité). Drapeau `bChain` sur les ordres
  (`FHOLMoveOrder`, `FHOLGatherOrder`, `FHOLBuildOrder`), posé seulement dans la portée `FPlayerOrderScope` des RPC du
  contrôleur (`ServerMoveUnits`, `…InLine`, `ServerAttackUnit`, `ServerAttackMove`, `ServerGatherUnits`,
  `ServerBuildUnits`) et par la pose d'une fondation ; **les stress tests n'en ont pas** (appels directs ; S7 passe un
  `PlayerIndex` : d'où le drapeau plutôt que l'index). Fin : déplacement / attaque-déplacement à l'arrivée (combats
  en route compris), attaque tant que la cible ordonnée est tenue, construction à la fin des chantiers de la file,
  **récolte jusqu'à la première arrivée à la source** (`UHOLGatherProcessor::ChainSources`) ; retirée aussi si
  l'unité se met à récolter / bâtir ou meurt. Sans chaîne : ralliement à la naissance (non trivial), recherche,
  riposte, retour à sa place, allers-retours, camp fini, chantier voisin. Priorité : bâtisseur, récolteur,
  déplacement. Rapide PASS (32 s, 83/83), `-Parts move` PASS (54 s). Essai scripté : `Move` d'une étape puis 0 à
  l'arrivée ; `Gather` puis 0 après l'arrivée. Restent : commentaires « Maj seulement » à reprendre (`GetChains`,
  `FBuilderQueue::bChain`, aide de `hol.Orders.Chains`). Jeu relancé pour l'essai.
- **Essai : lignes (un seul ordre compris), ordre de la file et 3 correctifs de la revue validés** (« sinon tout est
  bon »). `-Full` puis commit. Restent pour J7b.5b : **ligne du ralliement à la naissance** (choix de l'utilisateur :
  oui, **ligne blanche**, quel que soit le genre d'ordre du ralliement : D328 précisée), puis contrôles des lignes
  dans `quick_check` (solo et 2 clients).
- `-Full` **FAIL** (596 s, tests 83/83, `net 2 clients` PASS) : `gather` (`5 Peasants before, 12 + 4 made, one
  produced (9 + 17)`, comptes du HUD et inactifs faux, « . »), `build gap` (`two: 2 walker(s) seen in the way (0
  samples)`, `arrived (0 standing within 4 m)`), `orders` (`the target struck through them (0 hits)`). Tout passait
  ce matin (33f8d63) : relance de `-Parts gather,build,combat` pour savoir si c'est reproductible.
- Relance : `gather`, `build gap` **PASS** seuls → probablement les 4 Paysans du contrôle « fifo » (partie `move`,
  avant eux dans `-Full`) jamais retirés (« 12 + 4 made ») ; `orders` **FAIL** autrement (`the fight on the way
  (x -4291 at the kill)`) → instable. Sous-agent : retirer les Paysans de « fifo », cause de l'instabilité de
  `orders` (régression du jour ou scénario fragile), `-Parts combat` deux fois PASS.
- **Corrigé** (sous-agent, 14 min, 85k tokens ; rien de commité) : deux **scénarios qui laissaient des unités**, le jeu
  hors de cause. Nouvelle commande **`hol.Units.Remove X Y Radius [Slot=0]`** (`HOLUnitSubsystem.cpp` ~l. 95-120, par
  `RequestDestroyUnits`, journal `Remove: N units of slot S within R of (x, y)`). (1) « fifo » retire ses 4 Paysans
  (ils faussaient `gather` et traversaient `build gap` en allant au centre pour `footprints`). (2) `orders` : **aléa
  ancien** — le Paysan de « build wall » (à 8,5 m du Lancier d'attaque-déplacement, agressivité 12 m) attirait le
  Lancier et les Hommes d'armes hors de la route (x −4291 au lieu de < −4300) ; « build wall » retire maintenant son
  Paysan et les 3 de « gap » (nombre exact exigé). `-Parts move,gather,build,combat` **PASS** (444 s). `-Full` puis
  `-Parts combat` relancés par la session principale.
- `-Parts combat` (2e passe) **PASS** (156 s). `-Full` **FAIL** (414 s) : `timeout at step 1221: waitlog Remove: 3
  units of slot 0` (le nettoyage de « build wall » exige un compte exact ; en `-Full`, la partie `sources` en plus a
  sans doute laissé d'autres Paysans dans le cercle) → toute la suite en échec ; et `move : [KO] Escape clears the
  selection` (dernière sélection de la partie changée par « fifo »). Journal écrasé par la passe suivante. Leçon :
  **un nettoyage de test ne doit jamais bloquer la séquence** (attente sans compte, compte vérifié à l'analyse), et
  le reproduire dans l'ordre de `-Full`. Sous-agent lancé (`-Parts move,sources,gather,build`).
- **Nettoyages rendus non bloquants** (sous-agent, 15 min, 75k tokens ; seul `quick_check.ps1`). Comptes réels dans
  cet ordre : `Remove: 4` (fifo), `1` (mur), `3` (gap) : exacts. Deuxième cause trouvée : le Paysan du mur, encore
  sélectionné quand il est retiré, faisait écrire `Selection: none` avant la touche Échap, et l'attente qui suivait
  Échap bloquait (`timeout at step 964`) ; Échap passe maintenant **avant** les retraits. Attentes sur le rayon et le
  point (sans compte), comptes vérifiés à l'analyse (`fifo: its 4 Peasants taken away`, `walkers taken away: wall 1,
  way 3`) ; « Escape clears the selection » porte sur la touche (avant : 3 Paysans, après : none). `-Parts
  move,sources,gather,build` **PASS** (320 s). `-Full` relancé.
- Autre séance en parallèle dans la copie principale : **scénarios de test** (`ScenariosInProgress.md`, `hol.Scenario`,
  `Tools/Scenarios/`, premier scénario `order_lines` pour l'essai de ces lignes), code en fichiers nouveaux hors des
  fichiers de J7b.5b : fusion sans conflit attendue.
- `-Full` **FAIL** (646 s ; séquence allée au bout, tests 83/83) : `build gap` (`two: 2 walker(s) seen in the way`,
  `arrived (0 standing within 4 m)`, `walkers taken away: wall 1 of 1, way 1 of 3`) et `orders` (`the target struck
  through them (0 hits)`) : **2 sur 2 en `-Full`** depuis les changements du jour, alors que ces parties passent seules
  ou en petits groupes → interaction avec une partie propre à `-Full` (production, menu, anims, formation) ou
  régression qui ne se montre qu'avec plus d'unités. Sous-agent : cause prouvée par le journal (intact), reproduction
  minimale, correctif.
- **Cause** (sous-agent, 10,5 min, 135k tokens ; rien modifié) : **ni régression du jour, ni unités oubliées**. Un seul
  flux aléatoire pour toutes les naissances de la partie (`HOLUnitSubsystem.h` ~l. 164 `FRandomStream SpawnRandom{
  1066 }`, puisé à chaque naissance, `.cpp` ~l. 539-553) : toute unité créée avant un scénario change la disposition
  des suivantes. Les scénarios ajoutés aujourd'hui (« fifo », « combat fall » avant « push ») ont décalé le flux ; en
  `-Full` : (1) « build gap two » : les 2 Paysans naissent côte à côte (y 206 / 386), arrivent de front au passage
  et s'y coincent (4 cm en 5 s) ; (2) « push » : 3 Hommes d'armes bloqués contre le mur de 12 Paysans serrés, sans
  le contourner, 0 coup. **Faiblesse ancienne du jeu** : un marcheur bloqué par des corps redemande le même chemin
  toutes les 3 s (`HOLUnitOrderProcessor.cpp` ~l. 2335-2357, `PathStuckSteps` 60), les corps n'étant pas des
  obstacles du navmesh.
- **Choix de l'utilisateur** : tests stables maintenant (tirage fixé au début de chaque scénario), jeu plus tard.
  Ajouté aux retours d'essai : **5. Marcheurs bloqués par des corps** : deux de front devant un passage étroit, ou
  un mur d'unités immobiles → l'un cède le passage ou contourne (détour latéral) au lieu de redemander le même
  chemin ; à voir avec le point 1 (même domaine : la marche) et D292 / D293.
- **Graine par scénario** (sous-agent, 14 min, 85k tokens ; rien de commité) : `hol.Units.SpawnSeed <N>`
  (`HOLUnitSubsystem.cpp` ~l. 95-110, serveur / solo, journal `Spawn seed: N` ; `ResetSpawnSeed(int32)`), jeu
  inchangé sans elle ; dans `quick_check.ps1`, graines **101 à 130** en tête de chaque scénario qui fait naître des
  unités (après un éventuel `waitlog Selection: none` : avant, l'attente de « build camp » expirait). « build gap
  two » 113, « orders push » 128. `-Parts production,move,sources,gather,build` **PASS** (341 s). Non lancés par lui :
  combat, anims, formation, rapide → `-Full` lancé par la session principale.
- **`-Full` PASS** (570 s, tests 83/83, toutes les parties). **Commit c2b44e4** (lignes D328, ordre de la file,
  3 correctifs de la revue, contrôles), fusion de `main` (scénarios de test de l'autre séance, 9debb86) dans
  `j5c-anims` sans conflit (c9bb408), compilation sans avertissement, rapide PASS (26 s, 83/83), `main` avancé.
  **Reste pour J7b.5b** : (1) **ligne blanche du ralliement à la naissance** ; (2) contrôles des lignes dans
  `quick_check` (solo : `hol.Orders.Chains` et instances dessinées ; 2 clients : seul le propriétaire reçoit) ;
  (3) commentaires « Maj seulement » à reprendre (`GetChains`, `FBuilderQueue::bChain`, aide de `hol.Orders.Chains`).
- **Ligne du ralliement à la naissance faite** (sous-agent, 12 min, 140k tokens ; rien de commité). Genre d'étape
  **`Rally`** (`MakeRally(Point)`, point fixé à la construction de la chaîne), dessiné en blanc ; portée
  `FRallyOrderScope` (comme `FPlayerOrderScope`, drapeau `bRally` sur les ordres) ouverte dans le bloc du ralliement
  à la naissance (`HOLUnitSubsystem.cpp` ~l. 601 : sol, source, camp fini, chantier). Fin : sol à l'arrivée ; source
  (et camp fini) à la première arrivée ; **chantier quand l'unité commence à bâtir** (`StartBuilding`) ou à la fin du
  chantier. Coût nul sans ralliement et pour les stress tests. Test `Orders.Chains` étendu ; rapide PASS (23 s,
  83/83), `-Parts production` PASS (25 s), `-Parts build` PASS (263 s). Essai scripté (sol) : `Rally at (-13000,
  3000)` reçu, puis 0 à l'arrivée ; captures `HeroesOfLiberty-J5c\Saved_TestsRally\Screenshots\WindowsEditor\HOL\
  rally_line.png` / `rally_arrived.png`. Non couvert : ralliement sur un camp fini entre la naissance et le pas
  suivant (ordre de récolte hors portée, rare, déjà le cas pour les ordres du joueur).
- **Essai de l'utilisateur** : (1) **épaisseur de toutes les lignes divisée par deux** (25 → 12,5 cm) ; (2) **ligne de
  ralliement du bâtiment** : bâtiment sélectionné (forum, caserne, tout producteur) → ligne blanche jusqu'à son point
  de ralliement (fanion, source ou bâtiment visé), comme AoE4 (question posée : c'était bien cela) ; la ligne de
  l'unité née est gardée. D328 précisée. Lancé en sous-agent (affichage seul, aucun réseau en plus).
- **Fait** (sous-agent, 5 min, 40k tokens ; rien de commité) : `hol.Orders.LineWidth` 12,5 cm (disque d'étape gardé à
  30 cm) ; `AddLine` commun ; ligne blanche du **bord** de la forme visible du bâtiment (côté du point) au **fanion**
  (`GetRallyPoint` : sol, milieu de la source, centre du chantier), pour les bâtiments qui montrent un fanion
  (`GetCommandBuildings` + `HasRallyPoint` : seul, fantôme compris, ou sous-groupe actif d'un groupe F, comme les
  fanions) ; pas de disque au bout ; coût nul sans bâtiment sélectionné. Rapide PASS (23 s, 83/83). Captures
  `HeroesOfLiberty-J5c\Saved_Tests\Screenshots\WindowsEditor\HOL\rl_ground.png`, `rl_source.png`, `rl_army.png`
  (conformes). Jeu lancé pour l'essai.
- **Essai validé** (« sinon c'est bon ») avec une retouche : épaisseur encore divisée par deux → `hol.Orders.LineWidth`
  **6,25 cm** (disque d'étape inchangé, 30 cm). Compilé ; `-Full` lancé avant le commit.
- `-Full` **PASS** (574 s, 83/83). **Commit ed37f00** (lignes de ralliement, 6,25 cm), fusion de `main` (panneau des
  scénarios, 8906581) sans conflit (32631fa), compilation sans avertissement, rapide PASS (26 s), `main` avancé.
  **Reste pour J7b.5b** : contrôles des lignes dans `quick_check` (solo : `hol.Orders.Chains` et instances dessinées ;
  2 clients : seul le propriétaire reçoit ; ralliement), commentaires « Maj seulement » à reprendre. Puis J7b.6.
- **Contrôles des lignes faits** (sous-agent, 11,5 min, 125k tokens ; rien de commité). `AHOLSelectionMarkers::
  Describe()` (`hol.Select.Report`) donne `order lines N, order points N` (avant `frames dimmed`, motifs existants
  intacts) ; outils `Get-ChainReports` / `Format-ChainReport`. **move** : 3 Hommes d'armes (graine 102), Move puis
  Maj Move puis Maj AttackMove → chaîne `Move -> Move -> AttackMove` serveur et reçue, raccourcie jusqu'à `none`,
  3 segments et 3 points dessinés groupe sélectionné, 0 sans sélection, retirés à la fin. **Ralliement** (dans « move
  solo », avec `production`) : chaîne `Rally` vers (−13500, 0) pour le Paysan né, puis rien à l'arrivée. **net** :
  client 1 reçoit sa chaîne seule, client 2 n'en reçoit aucune pendant ce temps, puis la sienne seule. Délai : le
  serveur publie toutes les 0,25 s → `wait 0.5` avant chaque rapport. `-Parts production,move` PASS (77 s), `-Parts
  net` PASS (93 s), rapide PASS (21 s, 83/83). `-Full` lancé.
- `-Full` **PASS** (621 s, 83/83). **Commit 94eb04e**, fusion de `main` (GDD seul, 45ff0ab : pas de recompilation)
  en 0308eb4, `main` avancé. **J7b.5b close** (D328 au GDD à la clôture de J7b). Petit reste : commentaires « Maj
  seulement » (`GetChains`, `FBuilderQueue::bChain`, aide de `hol.Orders.Chains`) à reprendre à l'occasion.
  Suite : points 4 et 5 de la revue (à soumettre à l'utilisateur), puis **J7b.6** (vérification : `quick_check` solo
  et 2 clients du combat, essai de l'utilisateur ; S3 / S5 en combat à la fin de J7).
- **Points 4 et 5 de la revue soumis à l'utilisateur** :
  4. Demi-tour à la mort en solo / hôte → **l'unité garde l'orientation du moment de sa mort** (face à sa cible) pendant
     toute la chute, partout (solo, hôte, clients). Correctif d'affichage.
  5. PV d'une unité peu blessée affichés pleins chez un client → idée de l'utilisateur : « tableau des dégâts envoyé
     tous les X temps, dernière valeur gardée, servira aussi au soin ». Expliqué : c'est déjà le fonctionnement
     (état des unités changées envoyé par paquets, dernière valeur ; un changement de PV part tout de suite) ; une
     table séparée coûterait plus (identifiant répété) ; seule la précision diffère (16 paliers). **Choix de
     l'utilisateur : PV exacts tout de suite** (contre la recommandation « max − 1 maintenant, exact après mesure ») :
     **J7-Q5 révisée** (numéro D donné à la clôture de J7 ; D334 est le prochain de la review du GDD, autre séance) :
     bit « PV pleins » gardé, puis PV exacts d'une unité blessée (8 bits si le maximum de la table tient, sinon le
     nombre de bits du plus grand maximum), un envoi à chaque changement ; le soin passera par le même champ. Coût en
     grande bataille vérifié par S5 en combat à la fin de J7, retour aux paliers si le budget est dépassé.
  **Note de l'utilisateur** : « ça peut être gourmand, se pencher dessus niveau perf » → **point de vigilance des
  mesures de fin de J7** : octets par client en grande bataille (S5 en combat, comparé aux 16 paliers), nombre
  d'envois d'état provoqués par les coups, coût CPU de la sérialisation côté serveur ; pistes si trop cher :
  regrouper les changements de PV d'une unité sur un court délai (le dernier gagne) au lieu d'un envoi immédiat à
  chaque coup, ou moins de bits (paliers fins, p. ex. 64).
  Deux sous-agents à la suite (4 puis 5).
- **Point 4 fait** (sous-agent, 7 min, 78k tokens ; rien de commité). Cause (`HOLInterpolatedInstancesProcessor.cpp`
  ~l. 402-425) : au début de la chute, la séquence change, `bFacingSource` recalculé faux (la chute n'a ni source ni
  chantier), `bStrike` faux → lacet de la simulation (front de sa place). Correctif (~l. 427) : en `Dying`, lacet
  figé sur `FallYaw`, relevé une fois à l'entrée en chute sur le lacet dessiné à l'image précédente
  (`FHOLUnitAnimationStateFragment` : `bFalling`, `FallYaw`, `FallTurnDegrees` pour le contrôle). Client déjà juste
  (`HOLNetUnitDisplay.cpp` ~l. 66-87 garde `Display.Yaw`), même règle appliquée. Paysan : se tourne déjà vers sa cible
  depuis le couteau (`AttackSequence` 13). Contrôle : `hol.Units.AnimReport` écrit `Anim report falls: ... largest
  turn Y deg` ; « combat anims » : au moins une chute, écart < 3° (mesuré **0,0°**). `-Parts combat` PASS (165 s),
  rapide PASS (35 s, 83/83). À noter : « combat anims » laisse encore de côté le Paysan qui frappe (« Peasant has no
  strike yet ») : pourrait attendre `A_Peasant_Attack prop 8`. **Point 5 lancé** (PV exacts).
- **Point 5 fait** (sous-agent, 13 min, 150k tokens ; rien de commité). **8 bits** (plus grand maximum de la table :
  155, Homme d'armes ; une unité blessée a au plus max − 1 PV) ; `ensureMsgf` au chargement de la table si un maximum
  ne tient pas ; retour aux paliers : `HealthBits`, `GetWireHealth`, `HOLUnitView::GetReceivedHealth`. Coût **estimé** :
  +4 bits par envoi d'une unité blessée (70 au lieu de 66, +6 %) ; un envoi par coup (avant : un par palier franchi,
  ~2,5 fois moins pour l'Homme d'armes, presque autant pour le Lancier) ; ~0,5 envoi/s de plus par unité blessée en
  mêlée, ~35 bit/s ; ~7 kbit/s côté serveur pour 200 unités en mêlée. Tests `Net.*` étendus (aller-retour exact,
  255/256, chaque type de la table, 1 bit intacte, envoi à 1 PV près). `quick_check` : `net` (serveur `hol.Combat.LogHits
  1` ; chaque valeur affichée par le client est une valeur laissée par un coup du serveur, la dernière égale celle du
  serveur), `combat` (barres bornées par les relevés de combat). Recompilé sans avertissement ; rapide PASS (29 s,
  83/83), `-Parts net` PASS (103 s), `-Parts combat` PASS (150 s). Essai de l'utilisateur (points 4 et 5) lancé.
- **Essai validé** (« sinon tout est bon »). Repli si les PV exacts coûtent trop cher (choix de l'utilisateur) : **une
  minuterie sur le serveur à chaque coup reçu** : les coups reçus pendant ce court délai partent **en un seul envoi**
  (dernière valeur), au lieu d'un envoi par coup. Nouveau retour d'essai à faire plus tard (n° 6 ci-dessous).
  **Précisé par l'utilisateur** (repli seulement, en cas extrême si la mesure de fin de J7 montre un coût trop
  élevé) : (1) le serveur apprend qu'une unité a pris un coup → il lance **sa** minuterie (une par unité) ; (2) un
  deuxième coup pendant la minuterie → on remplace seulement la valeur à envoyer ; (3) un soin pendant la minuterie →
  idem ; (4) minuterie finie → on envoie la valeur, les PV se mettent à jour. **Délai le plus court possible** : aucune
  sensation de latence (de l'ordre d'un pas ou deux du serveur, à régler à la mesure).
  `-Full` puis commit.
- **Retour d'essai 6 (plus tard)** : **la formation a du mal contre les ressources et entre les bâtiments** ; test
  ultime : entre deux maisons, les unités se gênent pour passer, dans le meilleur des cas 4 sur 6 passent. À voir avec
  les retours 1 et 5 (la marche, D292 / D293, D324).
- Session des animations (mycrusader-6c) bloquée : « Unable to build while Live Coding is active » — le verrou de
  Live Coding est commun à tous les worktrees du même moteur. Fenêtre de 20 min promise après `-Full`. **À faire** :
  `-LiveCoding=0` sur les jeux d'essai lancés par la session principale et sur les instances de `quick_check.ps1`
  (comme `play.ps1`).
- `-Full` **PASS** (612 s, 83/83). **Commit 3f5a026** (PV exacts, lacet gardé à la mort), fusion de `main` (GDD,
  `.gitignore` seuls : pas de recompilation) en b6d3533, `main` avancé. Fenêtre laissée à mycrusader-6c (aucun Unreal
  de cette séance tant qu'elle n'a pas fini). Suite : `-LiveCoding=0`, puis **J7b.6** (vérification et essai du combat).
- `-LiveCoding=0` ajouté aux 4 lancements de `quick_check.ps1` (tests, serveur `net`, jeu, clients) et à la commande
  d'essai de « Commandes utiles » (worktree, non commité) ; à essayer (rapide) après la fenêtre de mycrusader-6c.
- Signalé par mycrusader-6c (non urgent) : avec `-Parts tests,camera,anims,combat`, « bars, nothing selected » échoue :
  la sélection laissée par la partie `anims` reste active (en `-Full`, `groups` finit par Échap). Défaut de liste
  partielle : faire finir `anims` par Échap (ou ouvrir `combat` par Échap) à l'occasion.
- Fenêtre de mycrusader-6c finie (son `-Full` PASS). `-LiveCoding=0` essayé : rapide PASS (29 s, 83/83) ; fin de `anims`
  par un clic dans le vide : `-Parts tests,camera,anims,combat` PASS (167 s). **Commit f73dbc7**, fusion de `main`
  (GDD seul) en 3e72c57, `main` avancé.
- **J7b.6 ouverte** (vérification). Plan annoncé à l'utilisateur : (1) contrôle du coup de couteau du Paysan dans
  « combat anims » (sous-agent lancé) ; (2) `-Full` ; (3) essai complet du combat par l'utilisateur (solo, et 2 clients
  s'il le veut) ; (4) clôture de J7b. S3 / S5 en combat (et coût des PV exacts) à la fin de J7.
- **(1) fait** (sous-agent, 7,5 min, 85k tokens ; seul `quick_check.ps1`) : « orders attack » prend `hol.Units.AnimReport
  0` et la capture `combat_peasant_strike` après l'ordre (clic et graine 123 inchangés) ; « combat anims » ne laisse
  plus le Paysan de côté : `Peasants striking play A_Peasant_Attack with the knife (prop 8) (4)` (rapport : `Peasant
  Attacking: A_Peasant_Attack prop 8 x4`). `-Parts combat` PASS ×2 (151 / 146 s), rapide PASS (28 s). Couteau peu
  visible au zoom de la capture. **Défaut vu** : le bloc d'un groupe de Paysans qui frappent affiche « 4 inactifs »
  (déjà avant ce changement) : à montrer à l'essai. `-Full` lancé.
- **(2) fait** : `-Full` **PASS** (609 s, 83/83). Commit f4ca921, fusion b21ee34, `main` avancé. **(3)** jeu lancé
  (`-LiveCoding=0`) pour l'essai complet du combat.
- **(3) Essai complet du combat : validé** (« ça a l'air bon »), sauf le **A clic suivi d'un déplacement en file**
  (retour d'essai n° 2) : choix de l'utilisateur, **plus tard avec les autres retours** (contre la recommandation « avant
  de clore J7b »). Le défaut « N inactifs » d'un groupe de Paysans qui frappent n'a pas été relevé par l'utilisateur :
  noté en retour n° 7 ci-dessous.
- **Retour 7 (plus tard)** : le bloc d'un groupe de Paysans qui frappent affiche « N inactifs » (vu sur la capture
  `combat_peasant_strike`) : une unité en combat ne doit pas compter comme inactive.

### 2026-10-20 — J7b.6 : clôture de J7b

- **J7b close.** Fait en J7b (journal ci-dessus) : processeur de combat, ordres (clic droit sur un ennemi,
  attaque-déplacement, S / D, Maj), barres de vie et bloc de sélection, réseau du combat, gestes polis des soldats,
  coup de couteau du Paysan (posture de peur), chutes, préchargement des types, ralliement sur un fantôme, D327
  (`quick_check` rapide / `-Full`), **D328** (lignes de la file d'ordres et du ralliement), file Maj qui ne saute plus
  d'étape, correctifs de la revue (1 à 5), **J7-Q5 révisée** (PV exacts en 8 bits, numéro à la clôture de J7),
  lacet gardé à la mort, `quick_check` stabilisé (`hol.Units.SpawnSeed`, `hol.Units.Remove`, `-LiveCoding=0`).
- **Report au GDD à la clôture de J7** (règle du plan de J7) : D327, D328 (réservés au § 22), J7-Q5 révisée et les autres
  décisions de J7 (numéros à la suite du § 22, aujourd'hui D340).
- **À la fin de J7** (règle des mesures) : S1 / S2 / S3 / S5 / S7 par `chapter_measures.ps1`, avec **S5 en combat**
  (coût des PV exacts : octets par client, envois par coup ; repli : minuterie par unité qui regroupe les coups) et
  S2 (au seuil depuis J6).
- **Retours d'essai à faire plus tard** (7, liste dans le journal du 2026-10-20) : groupe qui attend le retardataire,
  A clic validé trop tôt, pose sur une case occupée, pose unique sauf Maj, marcheurs bloqués par des corps, formation
  entre les bâtiments, « inactifs » en combat. Petits restes : commentaires « Maj seulement » (D328) ; points « plus
  tard » de la revue (`LastAttacker` sans limite, mort hors mêlée en J7c.2…).
- Suite : **J7c** (plan dans « Plan de J7 », questions déjà tranchées en partie par une autre séance).
- **Retours d'essai à faire plus tard** (demande de l'utilisateur : « ne fais pas tout de suite », à écrire) :
  1. **Groupe qui attend le retardataire** : 5 soldats groupés + 1 à 10 m, ordre de déplacement → les 5 attendent
     que le loin les rejoigne avant de partir. Attendu : ils partent tout de suite ; le retardataire prend sa place
     dans la formation quand il les a rejoints.
  2. **Attaque-déplacement (A clic) validée trop tôt** : avec A clic puis un déplacement en file, l'unité tue un
     ennemi sur cinq puis part vers la suite. Attendu : l'étape A clic n'est finie que quand la destination est
     atteinte **et** qu'il n'y a plus de menace à portée ; d'ici là, elle continue de combattre.
     → **corrigé, validé à l'essai et commité (2026-10-20, mycrusader-db, `main`)** : la file Maj (`StartQueuedMoves`)
     lançait l'étape suivante dès la marche finie, or une unité qui s'arrête pour frapper a fini sa marche ; elle attend
     maintenant la fin de l'attaque-déplacement (`Combat->Mode` revenu à `Normal`, posé par le combat à la case sans
     ennemi dans la recherche), sauf `hol.Combat.Enabled 0` ; la ligne rouge reste jusque-là. `quick_check` rapide
     PASS (31 s, 83/83, lancé par mycrusader-9b avec ce correctif). Scénario `attack_move_queue`.
  3. **Pose sur une case occupée** : en mode construction, curseur sur un chantier ou un bâtiment déjà construit →
     chercher dans les cases autour une position où la pose est possible (et y proposer le bâtiment).
  4. **Pose unique** : choisir un bâtiment = le poser une fois ; après la pose, le mode construction se ferme, sauf
     si **Maj** est enfoncée (poses en série).
- **Autre demande de l'utilisateur** (à faire, capture d'AoE4) : chez les **Paysans d'AoE4**, les **ordres** sont
  **à côté de la construction** : panneau d'ordres à gauche (attaque Q, arrêt S, construire W, déposer X, sortir F,
  garnison G, maintien V, supprimer Suppr, âge suivant T) et, à droite, le panneau de construction en **onglets par âge**
  (I, II, III, IV) ; **A Z E R choisissent l'onglet d'âge** avant de choisir la case de la grille. Donc
  **l'attaque-déplacement passe de A à Q** (« A clic » devient « Q clic ») pour ne pas entrer en conflit avec l'onglet I
  (revient sur le retour n° 2 de l'essai de J7b du 2026-10-19). À trancher au moment de le faire : ordre par rapport aux
  lignes colorées et à J7b.6, onglets d'âge dès maintenant (un seul âge en jeu ?) ou seulement la disposition.

### 2026-10-20 — J7c.1 : l'Archer

- J7c ouverte (mycrusader-37). Plan de J7c complet (Q1 à Q6, autre séance). Worktree `HeroesOfLiberty-Elrond` pris
  (`IN_USE.txt`), branche **`j7c-archer`** depuis `main` 81b5966 (`j7c-archer-preview` gardée, = `main`).
- Partage avec mycrusader-a6 (chantier animations) : elle fait les **corps gardés 30 s** dans `HeroesOfLiberty-J5c`
  (`HOLInterpolatedInstancesProcessor.cpp`, `HOLUnitReceptionSubsystem.cpp`, `HOLUnitSubsystem.cpp`, nouveau fichier
  de corps ; ni table des types ni séquences) ; l'Archer (questions, fabrication du tir et de la chute) revient à J7c.1.
  Chaque session prévient l'autre avant une compilation ou un lancement d'Unreal ; jeux en `-LiveCoding=0`.
- **Détails de l'Archer tranchés** (choix de l'utilisateur, A, la recommandation) : tout comme les aperçus validés —
  flèche cachée du lâcher à la reprise au carquois (échelle 0 sur `ik_hand_gun`, petit ajout au baker), corde à la
  mâchoire, carquois dans le dos, arc presque droit au tir et penché de 20° au repos, pas de flèche encochée au repos,
  corde droite à pleine allonge. Écartés : flèche encochée au repos ; revoir les GIF d'abord.
- Découpage de J7c.1 : **(a)** fabrication (maillage de l'Archer avec arc, carquois et flèche ; repos, marche, tir,
  chute passés de `PROPOSED` à `GESTURES`) ; **(b)** ligne de la table (vitesse 584 cm/s, maillage, séquences,
  coût, production) et production à la Caserne (grille, textes fr / en / ja), partie de `quick_check` ; **(c)** essai.
  Le tir lui-même (dégât différé, flèches) est J7c.2.
- En parallèle, mycrusader-9b (copie principale, `main`) fait passer l'attaque-déplacement de A à Q
  (`HOLCommandGrid::AttackMoveCell` 0 → 4) : grilles des bâtiments (Caserne A / Z / E) et de construction inchangées ;
  J7c.1b n'a donc pas à toucher `HOLCommandGrid` (refusionner `main` avant le commit de J7c.1). Validé et commité sur
  `main` par mycrusader-9b (**0f20427**) : à refusionner dans `j7c-archer`, avec **02a8963** (mycrusader-db :
  retour d'essai 2, `StartQueuedMoves` attend la fin de l'attaque-déplacement, `HOLUnitOrderProcessor.cpp`).

### 2026-10-20 — Attaque-déplacement de A à Q (mycrusader-9b)

- Demande de l'utilisateur (« Touche A → Q », la demande AoE4 du journal de J7b.6 : A Z E R gardées pour les onglets
  d'âge de la construction). Fait : `HOLCommandGrid::AttackMoveCell` 0 → 4 (Q) ; commentaires, aide de
  `hol.Select.AttackMove`, tests `CommandGrid.UnitOrders` (Q case 4 ; aucun bâtiment des Paysans sur Q S D). Le reste
  est inchangé : soldats dans la sélection = grille de construction cachée (« Tous ») ; Paysans seuls sans
  attaque-déplacement ; textes sans touche. Onglets d'âge et disposition AoE4 (ordres à gauche) : pas faits.
- Compilé (88 s) ; `quick_check` rapide **PASS** (31 s, tests 83/83), avec dedans le correctif du retour n° 2 de
  mycrusader-db (même copie, `HOLUnitOrderProcessor.cpp`, non commité). Scénario `attack_move_q` (6 Hommes d'armes,
  6 Lanciers ennemis, 5 Paysans). **Essai validé** (« tout est bon ») ; commité sur `main` par chemins explicites (le
  correctif de mycrusader-db et les ajouts des autres sessions au journal laissés de côté).
- **(a) faite** (sous-agent, ~10 min, ~120k tokens ; rien de commité ; détail dans `AnimationsInProgress.md`,
  « Fabrication de l'Archer ») : `SKM_Archer` (arc sur `hand_l`, flèche sur `ik_hand_gun`, carquois sur `spine_03`,
  bande à la couleur du joueur, pièces légères au LOD 3), `TP_Archer` (2 repos, 3 marche, 4 tir, 5 chute),
  `A_Archer_Idle` (11,3 s), `A_Archer_Walk` (1,0 s), `A_Archer_Attack` (48 images, lâcher image 24), `A_Archer_Fall`
  (1,1 s) ; nouvelle clé `"tool_hidden"` du baker : `ik_hand_gun` à l'échelle 0,001 (flèche cachée images 25-35 du
  tir, et toujours au repos, en marche, en chute) ; `--only` ajouté aux commandlets (autres soldats non refaits) ;
  relectures ≤ 0,071 cm, boucles fermées, os le plus bas de la chute à 0,48 cm. **Risque à vérifier en jeu** :
  l'échelle de l'os est-elle appliquée par le fournisseur de transformées sur le GPU ? Sinon flèche visible : autre
  moyen (C++ ou matériau). Valeurs pour la ligne `Archer` : `SKM_Archer`, `TP_Archer`, repos 2, marche 3, tir 4,
  chute 5, `AttackProp` 0 ; vitesse de lecture de marche à recalculer sur 584 cm/s.
- **(b) lancée** (sous-agent) : ligne `Archer` de la table, production à la Caserne (E), textes, contrôle en jeu de la
  flèche cachée, partie de `quick_check`.
- **(b) en partie faite** (sous-agent, ~11 min ; rien de commité ; worktree `HeroesOfLiberty-Elrond`) : ligne `Archer` (`SKM_Archer`,
  `TP_Archer`, repos 2, marche 3 à **3,47** (584 / 168,2), tir 4, chute 5, 584 cm/s, **70 PV**, **30 nourriture + 50 bois**,
  15 s ; combat gardé : 15 m, 1,625 s, 5 (+5 infanterie légère), armure 0/0) ; Caserne `Produces` + `Archer` (**E**) ;
  nom fr « Archer » / en « Archer » / ja « 弓兵 » (GatherText refait) ; tests mis à jour (`HOLAnimationTests` : l'Archer
  rejoint Lancier / Homme d'armes, la ligne `LightInfantry` sert de fournisseur du mannequin ; `HOLProductionTests`,
  `CommandGrid` Barracks E) ; tables refaites ; compilé sans avertissement (27 s) ; `-Parts tests` **PASS** (83/83, 24 s).
  Flèche cachée : lecture du moteur favorable (`AnimSequenceTransformProvider` garde une échelle **uniforme** par os,
  `FBoneTransformWithUniformScale`, en fp16 si demi-précision : 0,001 représentable) ; **capture en jeu pas faite**.
  Reste : capture d'un Archer au repos, production E dans la partie `build barracks` de `quick_check` (seule la ligne
  « A Lancier; Z Homme d'armes; E Archer » est changée, pas encore lancée), `AnimReport` de l'Archer.
- Vérifié par la session principale : les stress tests prennent `LightInfantry` (`hol.StressTest.UnitType`), pas
  l'Archer : S1 / S2 inchangés. `j7c-archer` avancée sur `main` 02a8963 (A → Q, retour d'essai 2), seul fichier
  commun (`HOLCommandGridTests.cpp`) réappliqué sans conflit. **(c) lancée** (sous-agent) : recompilation sur 02a8963,
  capture d'un Archer au repos (flèche cachée ?), `AnimReport`, Archer dans `build barracks`, `-Parts build` + rapide,
  puis scénario d'essai `archers`.

- **(c) faite** (sous-agent, ~13 min ; rien de commité ; `HeroesOfLiberty-Elrond`) : recompilé sur 02a8963 (38 s, sans
  avertissement) ; `-Parts tests` **PASS** (83/83). **Flèche cachée : OK en jeu** (captures `-RenderOffScreen` 2560×1440,
  `HeroesOfLiberty-Elrond\Saved_ArcherShot\Screenshots\WindowsEditor\HOL\archers_hr_*.png`) : au repos arc à la main gauche et
  carquois au dos, main droite vide ; `AnimReport` : `A_Archer_Idle` au repos, `A_Archer_Walk` en marche,
  `A_Archer_Attack` au tir. Aujourd'hui l'Archer tire avec le coup de mêlée à 15 m : dégât immédiat (10 au Lancier,
  1 à l'Homme d'armes), pas de flèche qui vole (J7c.2). Scénario **`archers`** (caserne finie, 6 Archers, 4 Lanciers +
  4 Hommes d'armes ennemis à 25 m, 5000 de tout) : `powershell -ExecutionPolicy Bypass -File Tools\Scenarios\play.ps1 archers -Root ..\HeroesOfLiberty-Elrond`.
  `quick_check` : `build barracks` produit A, Z puis **E** (3 nés, 3 au ralliement, 3 vers la veine) ; `build hud`
  attend « Produit : Lancier, Homme d'armes, Archer » dans l'infobulle de la Caserne.
- Capture regardée par la session principale (`HeroesOfLiberty-Elrond\Saved_ArcherShot\...\archers_hr_1_crop.png`) : arcs,
  carquois, bandes bleues, main droite vide au repos. **Essai de l'utilisateur lancé** : `play.ps1 archers -Root
  ..\HeroesOfLiberty-Elrond` (à juger : l'Archer et sa production sur E ; le tir a encore le dégât immédiat sans flèche, J7c.2).
- **Essai de l'utilisateur validé** (« tout est bon ») : allure, production sur E, geste de tir.
- **Retour d'essai 8 (plus tard, demande de l'utilisateur)** : **impossible de faire du hit and run avec l'Archer** :
  le geste de tir dure toute la cadence (1,6 s, lâcher à 50 %), l'Archer ne peut pas tirer puis repartir aussitôt.
  Pistes à trancher le moment venu : lâcher plus tôt dans le geste (bandé court, puis récupération interruptible par un
  ordre de marche), geste plus court que la cadence (attente entre deux tirs, interruptible), tir gardé si l'ordre de
  marche arrive après le lâcher. À voir avec J7c.2 (moment du lâcher partagé serveur / client, Q4).
- Suite : commit sur `j7c-archer`, refusion de `main`, `-Full`, avance de `main`.
- Commit **b739547** sur `j7c-archer`, fusion de `main` (d6fc962, doc seule) en 4dc01ad, **`-Full` PASS** (593 s, toutes
  les parties, `anims` : `Archer Idle: A_Archer_Idle`, tests 83/83), **`main` avancé à 4dc01ad**. `HeroesOfLiberty-Elrond` libéré.
  **J7c.1 close.** Suite : **J7c.2**.

### 2026-10-20 — J7c.2 : le tir

- `main` à c666a54 (corps gardés 30 s de mycrusader-a6 fusionnés). Hit and run gardé pour plus tard (retour 8, choix
  de l'utilisateur) : J7c.2 suit le plan (J7c-Q1 à Q6). `HeroesOfLiberty-Elrond` repris, branche **`j7c-shot`** depuis c666a54.
- Découpage (un sous-agent par livrable, 15 min / 300k tokens) : **(a)** serveur : dégât différé selon la distance
  (25 m/s, lâcher au même moment du geste que le coup de mêlée, liste courte cible / montant / pas d'arrivée, cible
  morte = flèche perdue), cible des tireurs (Q6 : la plus proche, gardée tant qu'à portée, sans `MaxAttackers`), mort
  par flèche sur le chemin « mourant » complet (revue de J7b : `StopWalk`, `Disengage`, activité `Dying`), tests,
  contrôle dans `quick_check` ; **(b)** client : flèches déduites (activité « attaque » + cible), maillage instancié,
  cloche légère, disparition à l'impact, rien sur le serveur dédié, capture ; **(c)** vérification et essai (J7c.3).
- **(a) lancée.**

### 2026-10-20 — Retour d'essai 1 : le groupe qui attend le retardataire (mycrusader-c9)

- Cause (lecture du code) : la marche en bloc (`StartBlockMarch` / `AdvanceBlocks`, `HOLUnitOrderProcessor.cpp`) part
  du centre de **tout** le groupe ; une unité à plus de `BlockJoinDistance` (4,5 m) de sa place de départ fait d'abord
  rassembler le groupe (jusqu'à `FormUpMaxSteps`, 3 s), puis le bloc s'arrête tant qu'une unité a plus de
  `BlockSoftHoldDistance` (1,6 m) de retard sur sa case, et ne la lâche qu'après `BlockWaitMaxSteps` (3 s) : ~6 s
  d'attente.
- **Choix de l'utilisateur (A, la recommandation)** : le bloc part tout de suite sans le retardataire (une unité à plus
  de 4,5 m du groupe, quand le reste fait au moins la moitié du groupe ; un groupe vraiment éparpillé se rassemble
  d'abord, comme aujourd'hui). Le retardataire marche **à sa vitesse**, sur son propre chemin, vers sa case du bloc, et y
  entre dès qu'il l'atteint : en chemin s'il est plus rapide que le bloc, sinon à l'arrivée, où il prend sa place.
  Aucune vitesse ne change. Écartés : bloc ralenti à 90 % tant qu'un retardataire n'est pas rentré ; retardataire 20 %
  plus rapide que sa vitesse de la table.
- Travail dans la copie principale, sur `main` (HeroesOfLiberty-Elrond pris par mycrusader-37 ; mycrusader-a6 a avancé `main` à
  c666a54 avant). Code lancé en sous-agent.
- **Boucle 1 faite** (sous-agent, ~8 min, rien de commité) : règle pure `HOLFormation::FindStragglers` (le corps = le plus
  grand amas reliés de proche en proche par des pas de 4,5 m au plus ; retardataires seulement si le corps fait
  **strictement** plus de la moitié : deux amas de 3 se rassemblent comme avant ; les coins d'un grand bloc ne sont pas
  des retardataires) ; `StartBlockMarch` : centre, chemin et rassemblement sur le corps ; les retardataires gardent leur
  case et leur place d'arrivée, partent sur leur propre chemin du navmesh vers leur place (comme une unité qui a quitté
  le bloc), ne retiennent jamais le bloc ; `AdvanceBlocks` : un retardataire qui passe à moins de 1,6 m de sa case entre
  dans le bloc (pas pendant le rassemblement ni la fusion d'arrivée) ; l'arrivée attend aussi les retardataires.
  Rapport : `stragglers N joined M`. Limite : l'allure du bloc reste celle du plus lent de tout le groupe, retardataire
  compris. Compilé sans avertissement (33 s), tests 84/84, `quick_check -Parts tests,formation` **PASS** (111 s ; le
  bloc de 9 de la partie n'a aucun retardataire, inchangé). Scénario d'essai `straggler` écrit (non vérifié).
- **Boucle 2** lancée : cas « retardataire » dans la partie `formation` de `quick_check`, scénario `straggler` vérifié
  sans fenêtre.
- **Boucle 2 faite** (sous-agent, ~10 min, rien de commité) : 2 cas ajoutés à la partie `formation` de `quick_check`
  (après `groups`) : 5 Hommes d'armes + 1 à 10 m derrière, puis 5 + 1 Archer ; vérifiés : `forming up on the move`,
  `1 stragglers left behind`, `formup 0`, `held` ≤ 4, `arrived 1`, les 6 à 60 cm de leur place. Mesuré :
  `units 6 formup 0 march 149 held 0 ... settle 34 ... stragglers 1 joined 0` et, Archer, `held 1 ... settle 15 ...
  joined 0`. Run FAIL sur la seule borne `settle` (8, celle des autres cas) : 34 pas = 1,7 s, les 10 m de retard ;
  borne relevée à 50 pour ces cas, pas encore relancé. L'Archer ne gagne que 61 cm/s : sur 35 m il ne rattrape pas
  le bloc (il lui faudrait ~85 m depuis 10 m) ; scénario changé : Archer à 7 m, ordre à 60 m ou plus.
- Scénario `straggler` : sans `.json`, la partie a voulu `BattleMap` (`Scenario straggler: /Game/Level/BattleMap...`)
  alors que `Plain` s'est chargée, puis `waitmap` a expiré (README : « sans .json : Plain » ; sans doute une config
  gardée dans `Saved_Tests` ; **petit défaut à regarder plus tard**) ; `straggler.json` ajouté (Plain, 2 joueurs). En
  `-nullrhi` les scénarios ne tournent pas (`waitui`) : les vérifier en `-RenderOffScreen`.
- **Boucle 3** lancée : `quick_check -Parts tests,formation` relancé, scénario vérifié en `-RenderOffScreen` avec les
  deux ordres.
- **Boucle 3 faite** (sous-agent, ~10 min) : `quick_check -Parts tests,formation` **PASS** (134 s, 84/84 ; cas
  `straggler` : `formup 0 ... held 0 ... settle 34 stragglers 1 joined 0` ; Archer : `held 1 ... settle 15 ... joined 0`).
  Scénario `straggler` vérifié en `-RenderOffScreen` (prêt, 12 unités) ; retardataires replacés **derrière** leur
  groupe (côté centre : devant, l'Archer gardait une place du premier rang et marchait devant le bloc). Ordres joués :
  gauche à 35 m `formup 0 march 137 held 0 ... settle 33 stragglers 1 joined 0` ; droite à 60 m `a straggler joined
  the block after 102 steps of march`, `formup 0 march 242 held 7 gap 129 speed 565 ... stragglers 1 joined 1`. À
  surveiller : l'Archer rentré marche à 565 cm/s (> pas du bloc 523) le temps de gagner sa case.
- **Essai de l'utilisateur lancé** : `play.ps1 straggler`.
- **Essai de l'utilisateur : raté.** (1) « de temps en temps ça marche, mais par chance » ; (2) quand le retardataire
  arrive près du groupe en mouvement, ils s'arrêtent, se groupent et repartent ; (3) il essaie de s'insérer dans sa
  case, casse la formation, et ils se bloquent ; (4) 2 groupes de 3 se superposent au moment de se rejoindre.
  Hypothèses (lecture du code) : (1) deux critères (`FindStragglers` par voisin le plus proche, « gathering first »
  par la distance à la place de départ) ; (2) entré à 1,6 m de sa case, il a ensuite la borne de marche (40 cm) et
  retient le bloc ; (3) sa case au milieu ou sur le côté du bloc ; (4) cas sans retardataire : le rassemblement
  d'avant, où les unités d'un ordre se traversent. Enquête lancée (sous-agent, sans changer le code).
- **Choix de l'utilisateur (A, la recommandation)** : le retardataire ne retient jamais le bloc ni ne le traverse ; au
  départ, on lui garde une **case vide du dernier rang**, du côté où il arrive ; le bloc marche avec ce trou, il y entre
  par l'arrière ; s'il est d'un type dont les rangs sont devant (groupe mêlé), il marche en queue et prend sa place à
  l'arrivée. Écartés : en queue jusqu'à l'arrivée dans tous les cas ; bloc refait à sa jonction.
- **Précision de l'utilisateur** : « tant qu'une unité n'est pas dans le bloc, elle garde sa vitesse » : l'allure du
  bloc est celle du plus lent **du bloc** (plus celle d'un retardataire) ; une unité qui entre prend le pas du bloc
  (le bloc ralentit à elle seulement si elle est plus lente).
- **Enquête faite** (sous-agent, ~6 min, aucun code changé ; 9 essais scriptés + 3 obliques, terrain plat) : (1)
  non reproduit (toujours 1 retardataire, « on the move ») ; (2) mécanisme confirmé (entré à ~130-160 cm, borne 40 cm
  ensuite : bloc retenu) ; (3) mécanisme établi : entré, il est en phase `Block` sans traversée (`GetPassThroughSerial`,
  `HOLSimulationFragments.h:171` : seules `FormUp` / `bReforming` se traversent) ; il bute sur les corps, ceux qu'il
  pousse dépassent 40 cm et retiennent le bloc à leur tour, et chacun **quitte le bloc** après 3 s ; (4) établi : 3
  contre 3, pas de retardataire, rassemblement en `FormUp` où l'évitement entre unités du même ordre est coupé
  (`HOLSimulationProcessors.cpp:614` et le regard en avant vers 250) : paires à moins de 60 cm dès 0,5 s.
- **Journal de l'essai de l'utilisateur** (`Saved/Logs/HeroesOfLiberty.log`) : sur un ordre de 106 m, `a straggler
  joined the block after 242 steps of march`, puis `arrived after 0 steps of form-up and 536 of march (126 held),
  slot gap 178 cm at most ... 5 units left it` : la formation cassée (symptôme 3). Aussi, groupe éparpillé
  (`gathering first`, 959 cm, 0 retardataire) : `72 held ... 5 units left it`. Beaucoup d'ordres relancés en marche.
- **Plan du correctif** (choix A et précision de l'utilisateur) : (a) à l'ordre, pour chaque retardataire, une case
  **réservée au dernier rang** de son type (la plus proche de son côté) ; les autres prennent les autres cases ; si les
  rangs de son type ne sont pas le dernier rang du bloc : **en queue** (point derrière le bloc), place prise à
  l'arrivée ; (b) le retardataire marche **à sa vitesse** : son chemin tant qu'il est loin, puis droit vers sa case
  (ou la queue) en traversant les unités de son ordre (comme `bReforming`), sans jamais retenir le bloc ; il est
  membre à 40 cm de sa case, prend alors le pas du bloc ; (c) **allure du bloc = le plus lent de ses membres** ; (d) un
  membre trop longtemps en retard (`BlockWaitMaxSteps`) redevient retardataire (rattrape sa case) au lieu de quitter le
  bloc pour de bon ; (e) rassemblement d'un groupe éparpillé **avec évitement** entre unités de l'ordre (plus de
  superposition), la traversée gardée pour la reformation de l'arrivée.
- **Boucle 4 faite** (sous-agent, ~12 min, rien de commité) : plan (a) à (c). Règle pure `ReserveRearSlots` (par
  retardataire, la case libre de son type la plus en arrière, puis la plus proche de lui en travers ; pas au dernier
  rang du bloc : **en queue** ; le corps prend les autres cases par `AssignPlaces`) ; `bStraggler` (fragment) : traverse
  les unités de son ordre en phase `Block` ; loin, son chemin vers sa place d'arrivée ; à 6 m de sa cible avec la ligne
  libre, droit vers sa case à sa vitesse ; repart sur un chemin au-delà de 9 m ou ligne coupée ; membre à 40 cm (pas
  du bloc, plus de traversée) ; jamais dans `bHeld` / `MaxGap` / `HeldSteps`. Allure = le plus lent du corps, le bloc
  ralentit à un membre plus lent qui entre. Rapport `queued N`. Compilé sans avertissement, tests 85/85,
  `quick_check -Parts tests,formation` **PASS** (129 s) : cas straggler `held 0 left 0 ... joined 0` (« its slot kept
  in the last row (150 cm across) ») ; Archer `held 1 left 0 settle 8 ... joined 0` (35 m, trop court pour rattraper).
- **Boucle 5** lancée : plan (d) et (e) (membre en retard redevient retardataire ; rassemblement avec évitement).
- **Boucle 5 arrêtée à sa limite** (15 min, ~160k tokens) : (d) et (e) écrits et compilés (DLL de 22:19 après le
  source de 22:18) : `GetPassThroughSerial` ne traverse plus en `FormUp` (seulement `bReforming` et un retardataire
  droit vers sa case), commentaires de la séparation et du regard en avant ; cas `scattered` ajouté à la partie
  `formation` de `quick_check`, champ `spacing` (distance minimale entre membres) au rapport ; `quick_check` coupé en
  cours (cas straggler : `held 0 left 0 ... settle 35 ... spacing 150`). **Bilan** : code + compilation + nouveau cas +
  `quick_check` ne tiennent pas dans une boucle (comme pour les poses de mycrusader-7c) ; **amélioration** : une boucle
  écrit et compile, la suivante ne fait que vérifier (relecture du diff, `-Parts tests,formation`, rapport).
- **Boucle 6** lancée : vérification seule.
- **Boucle 6 faite** (vérification, ~5 min) : `quick_check -Parts tests,formation` **FAIL** (149 s ; tests 85/85,
  `straggler`, `scattered`, `groups` PASS). `scattered` : `gathering first (512 cm)`, `formup 4 march 165 held 15 left
  0 ... spacing 120` (plus de superposition). KO : le **bloc de 9** (3 Paysans, 3 Lanciers, 3 Hommes d'armes côte à
  côte, `gathering first` 673 cm) : sans traversée en `FormUp`, deux membres restent à ~228 cm de leur case pendant 3 s,
  redeviennent retardataires (`left 2`, `held 63`, 204 cm de leur place, un plus rapide que le plus lent + 10 %) ; et le
  **glisser simple** (demi-tour) : 126 pas pour 120 au plus (16 retenus). Cause : les rangs de types différents doivent
  se croiser pour se mettre en place (la raison de la traversée en J7a.4).
- **Correctif retenu** (même règle que l'arrivée : « deux types se traversent, deux du même type se contournent ») :
  pendant le rassemblement, deux unités de l'ordre **de types différents** se traversent, **deux du même type**
  s'évitent (leurs places sont données sans croisement par type) ; reformation de l'arrivée et retardataire droit vers
  sa case inchangés. **Boucle 7** lancée (écriture et compilation seulement).
- **Boucle 7 faite** (~4 min, écriture et compilation, 27 s sans avertissement) : `RowKind` (type de rang + 1, posé
  au passage en `FormUp`, remis à zéro par un nouvel ordre), `GetPassThroughKind`, `CanPassThrough` (même ordre, sauf
  deux unités en `FormUp` du même type) dans le regard en avant et la séparation. Commentaires harmonisés (« playtest
  note 1 of J7b »). Vérification à suivre, après l'avance de `main` de mycrusader-7c (« N inactifs »).
- **Boucle 8 faite** (vérification, ~6 min) : compilé sur f7f00df (43 s, sans avertissement) ; `quick_check -Parts
  tests,formation` **PASS** (143 s, 85/85) : bloc de 9 `formup 11 march 151 held 14 left 0`, écart 40 cm, 565 cm/s au
  plus (≤ 575) ; glisser simple 116 pas (≤ 120) ; straggler `held 0 left 0 settle 35 stragglers 1` ; Archer `held 1
  left 0 settle 8` ; scattered `formup 4 march 165 held 15 left 0 ... spacing 120`. Aucun « fell behind ». Rapide PASS
  (24 s). **2e essai de l'utilisateur lancé** (`play.ps1 straggler`).
- **2e essai de l'utilisateur** : groupe de gauche (retardataire à 10 m) et groupe de droite (Archer qui rentre en
  chemin) : **0 problème**. À changer : (1) **2 paquets de 3** : « je ne veux plus ça » (le rassemblement) : ils vont
  à la destination, et s'ils se croisent en chemin ils peuvent fusionner, pas avant ; (2) **retardataire tout proche**
  (capture : 5 en colonne, 1 à ~3 m, donc dans le corps à moins de 4,5 m mais loin de sa case) : la formation attend
  d'être complète avant de partir (jonction en marche, bloc retenu à 160 cm) : « le même correctif que les 2 groupes
  de 3 ». Règle générale : **la formation ne s'arrête jamais pour attendre**.
- **Choix de l'utilisateur** (une question, deux points) : (1) **bloc qui balaie** (A, la recommandation) : le bloc
  part tout de suite, placé juste derrière l'unité la plus en arrière (assez loin pour que chaque unité ait le temps de
  gagner la ligne de sa case avant qu'elle n'arrive) ; les unités devant leur case se décalent sur le côté et la
  laissent venir ; plus de rassemblement ni de bloc retenu à la jonction. Écarté : retardataire dès 0,5 m derrière sa
  case. (2) **petits blocs** (B, contre la recommandation « en vrac ») : chaque paquet séparé marche en sa propre
  formation vers la destination ; les blocs fusionnent quand ils se rencontrent, sinon à l'arrivée. Écarté : l'autre
  paquet en retardataires.
- Découpage : (i) bloc qui balaie (écriture + compilation), (ii) vérification + cas « proche » dans `quick_check`,
  (iii) petits blocs (plusieurs blocs par ordre, places finales par paquet), (iv) fusion des blocs + vérification,
  (v) scénario et essai.
- **Séance arrêtée ici (demande de l'utilisateur : « marque tout pour continuer dans une prochaine session »).**
  **Reprise exacte** :
  - **État** : copie principale, `main` à f7f00df, **rien de commité** pour ce retour (validation partielle seulement) :
    `HOLUnitOrderProcessor.cpp/.h`, `HOLFormationRules.cpp/.h`, `HOLSimulationFragments.h`,
    `HOLSimulationProcessors.cpp/.h`, `HOLFormationTests.cpp`, `quick_check.ps1` (cas `straggler`, `straggler archer`,
    `scattered` de la partie `formation`), scénario `Tools/Scenarios/straggler.txt/.json` (non suivis). Compilé,
    `quick_check -Parts tests,formation` PASS et rapide PASS (boucle 8). Les autres sessions avancent `main` par
    `git merge --ff-only --autostash` après signal : le diff reste dans l'arbre de travail.
  - **Validé à l'essai** : retardataire à 10 m (case gardée au dernier rang, à sa vitesse, jamais de bloc retenu,
    membre en retard qui redevient retardataire) ; Archer plus rapide qui rentre en chemin.
  - **À faire, dans l'ordre** : (i) **bloc qui balaie** : dans `StartBlockMarch`, plus de « gathering first » ni de
    jonction retenue (`bJoining`, `GetBlockSoftPace`, `FormUpMaxSteps`) ; le bloc part tout de suite, son point de départ
    reculé sur le prolongement du chemin derrière l'unité la plus en arrière, assez pour que chaque unité ait le temps
    (écart en long ≥ écart en travers + marge) de gagner la ligne de sa case ; une unité devant sa case se décale sur le
    côté et attend que la case arrive (cible = la case avancée jusqu'à elle, pas de marche arrière) ; (ii) vérification
    + cas « retardataire proche » dans `quick_check` (5 en colonne + 1 à 3 m : `formup 0`, `held` ≈ 0, aucun membre
    retenu) ; (iii) **petits blocs** : un bloc par paquet séparé (`FindStragglers` devient « les paquets » ; paquet d'une
    seule unité = retardataire comme aujourd'hui), chacun avec sa part des places finales et son propre chemin ;
    `Blocks` aujourd'hui indexé par numéro d'ordre : passer à plusieurs blocs par ordre ; (iv) **fusion** : quand deux
    blocs d'un ordre se rencontrent (le petit près de l'arrière du grand), le petit se dissout et ses unités rejoignent
    leurs cases gardées au dernier rang du grand (mécanisme des retardataires) ; sinon chacun se reforme à l'arrivée ;
    vérification + cas `scattered` (2 × 3 : `held` ≈ 0, pas de rassemblement) ; (v) scénario `straggler` complété
    (5 + 1 proche, 2 × 3) et 3e essai de l'utilisateur ; puis `-Full`, commit sur `main` (avec `EtapeDefini.md`).
  - **À savoir** : mycrusader-cc a commité sur `j7c-shot` (2e3754b) une nouvelle règle des rangs d'un seul type
    (`MakeSquareRows`) et l'attribution des places par profondeur (`AssignPlaces` / `AssignPlacesByKind`), pas encore
    dans `main` : `ReserveRearSlots` appelle `AssignPlaces` sur les cases restantes (un rang troué) ; celle de nous deux
    qui fusionne en second refusionne `main` et vérifie ce point. Méthode : boucles de 15 min, une boucle écrit et
    compile, la suivante vérifie ; scénarios vérifiés en `-RenderOffScreen` (pas `-nullrhi`). Petit défaut noté : un
    scénario sans `.json` demande `BattleMap` au lieu de `Plain`.
- **Reprise (2026-10-21)** : copie principale, `main` toujours à f7f00df, `j7c-shot` pas encore fusionnée dans `main`.
  Copie du travail non commité dans le scratchpad (`backup1/retour1.patch` + scénario `straggler`). **Boucle 9** lancée :
  (i) bloc qui balaie (écriture et compilation seulement). mycrusader-56 a repris J7c.2 (b2) dans `HeroesOfLiberty-Elrond` :
  copie principale laissée à cette session, message avant toute avance de `main`.
- **Boucle 9 faite** (sous-agent, ~10 min, compilé en 26 s sans avertissement, rien de commité) : règle pure
  `HOLFormation::GetBlockSetBack` (repère du bloc au premier point du chemin ; par unité du corps, si elle est à plus de
  40 cm en travers de sa case : avance demandée = écart en travers + `BlockSweepMargin` 100 cm, sinon 0 ; recul = le
  plus grand manque, jamais négatif ; retardataires exclus) et 5 cas de test ; `StartBlockMarch` : plus de « gathering
  first », `Progress = -recul` (point du bloc sur le prolongement du chemin vers l'arrière), le corps en `FormUp` ;
  `AdvanceBlocks` : plus de `bJoining` ni d'allure réduite ni de délais de rassemblement (`FormUpMaxSteps` retirée) ;
  une unité en `FormUp` ne retient jamais le bloc ; membre à 40 cm de sa case ; devant sa case : cible = sa case
  avancée jusqu'à elle (décalage de côté puis attente, pas de marche arrière) ; derrière : droit vers sa case à sa
  vitesse. Journal « Block march: sweeping, start set back N cm » ; `formup` vaut toujours 0 ; `quick_check` : contrôles
  « forming up on the move » / « gathering first » devenus « sweeping in at once ». Risques : point reculé hors navmesh
  (mur derrière le groupe), un membre déjà entré qui ne laisse pas passer une unité en `FormUp`.
- **Boucle 10** lancée : vérification + cas « retardataire proche » dans `quick_check`.
- **Boucle 10 faite** (~15 min, à sa limite ; code C++ inchangé) : cas `straggler near` ajouté à la partie `formation`
  (5 Hommes d'armes en file + 1 à ~3,2 m derrière et à côté ; ordre à 38 m ; unités retirées à la fin) : **PASS**
  (`sweeping` recul 501 cm, `formup 0 march 173 held 0 left 0 ... spacing 137`). `-Parts tests,formation` **FAIL**
  (164 s, 85/85) : tous les cas d'un seul type passent (straggler, straggler archer, scattered `held 0 spacing 119`,
  straggler near), **tous les groupes mêlés échouent** (formation 6 mixtes `held 71`, `plain` `held 76`, bloc de 9
  `held 180`, pas arrivé dans la fenêtre). Cause (journal) : des unités devant leur case (Paysans, un Homme d'armes)
  restent en `FormUp` toute la marche, **poussées** par les membres d'un autre type des rangs de derrière : un membre
  (`Block`) ne traverse jamais (`GetPassThroughSerial` = 0) ; la cible « case avancée jusqu'à elle » se recale sur la
  position poussée, la case ne les atteint jamais ; les membres gênés retiennent le bloc un pas sur deux.
- **Correctif** (règle déjà tranchée : deux types se traversent, deux du même type se contournent) : un membre traverse
  les unités de son ordre d'un autre type de rang (`GetPassThroughSerial` / `GetPassThroughKind` aussi en `Block` hors
  retardataire et reformation). **Boucle 11** lancée : correctif, compilation, `-Parts tests,formation`, rapide.
- **Boucle 11 faite** (~12 min) : `IsBlockMemberWithKind` (membre en `Block`, ni retardataire ni reformation,
  `RowKind` ≠ 0) dans `GetPassThroughSerial` / `GetPassThroughKind` ; un retardataire qui entre dans le bloc reçoit son
  `RowKind` (il restait à 0). Compilé (55 s, sans avertissement). `-Parts tests,formation` **PASS** (181 s, 85/85),
  **`held 0` partout**, `formup 0`, `left 0` : 6 mêlés `sweeping 676 cm march 165 gap 10 speed 565` ; bloc de 9
  `855 cm march 166 gap 3` (avant : `held 180`) ; straggler `236 cm march 155 settle 28` ; Archer `285 cm march 149` ;
  scattered `738 cm march 175 spacing 119` ; straggler near `501 cm march 173 spacing 137`. Rapide PASS (29 s). À
  savoir : `spacing` des groupes mêlés à 1-4 cm (paire la plus proche tous types confondus, normal puisqu'ils se
  traversent ; à mesurer par type si besoin) ; cas « même type, devant sa case dans la file d'un membre » non montré
  par les cas.
- **Choix de l'utilisateur** : essai court du bloc qui balaie maintenant (A, la recommandation), avant (iii) et (iv).
  Scénario `straggler` complété : colonne de 5 Hommes d'armes + 1 à ~3 m (Y -4500), groupe mêlé de 9 (3 Paysans,
  3 Lanciers, 3 Hommes d'armes, Y 4300 / 4600 / 4900). Vérifié en `-RenderOffScreen` (sous-agent, ~4 min) : Plain,
  `ready`, 27 unités, rien hors navmesh ; colonne à 34 m `sweeping 538 cm ... 156 of march (0 held) ... settled after
  2 steps, 0 pushes` ; groupe mêlé d'abord à 6 m d'écart entre types : un type entier à ~9 m mis **en queue** (3
  retardataires, cas des petits blocs pas encore traité) : écart resserré à 3 m. **3e essai de l'utilisateur lancé**.
- **3e essai de l'utilisateur : bloc qui balaie refusé.** (1) groupe de gauche : « ils se sont rejoints et ont attendu
  avant de partir, se sont gênés pour partir et ont marché tout du long » : **échec** ; (2) groupe de l'Archer : mis
  en formation avant de partir, partis sans l'Archer qui s'est bien inséré plus tard : « bien, mais on enlève la
  formation au départ : si on doit être en formation, c'est **pendant le trajet ou après, pas avant** ». Référence :
  `C:\Users\baudo\Desktop\zza.gif` (une foule serrée d'Hommes d'armes et de Lanciers). Journal : 1er ordre 6 Hommes
  d'armes, `0 stragglers`, recul 562 cm, `346 of march (143 held)` (sans doute la colonne + 1 proche : deux du même
  type dans une file, celui de devant attend sa case, le membre derrière le contourne mal : le cas connu non traité) ;
  Archer : recul 312 cm, 0 retenu, entré après 97 pas ; ordre de 18 unités de trois groupes : **recul 8075 cm**,
  `settled after 36 steps ... 883 cm from its place` (le recul n'a pas de borne ; groupes séparés : cas des petits
  blocs). Le défaut de fond : avec le recul, toutes les unités sont devant leur case et l'attendent, ce qui se voit
  comme une mise en formation avant le départ.
- **Règle de l'utilisateur** (réponse à la question A rangs en marche / B bloc à plein pas / C en vrac) : « pas
  d'attente ni de ralentissement ; si le retardataire arrive à s'intégrer, tant mieux, sinon ce sera à l'arrivée ; le
  bloc, par contre, essaie de se mettre en rangs pendant sa marche, intelligemment ». Et : **tentative de formation en
  déplacement, que ce soit un ordre de marche ou une attaque-déplacement (Q clic) ; dès qu'on attaque ou engage un
  ennemi, plus de formation.**
- Lecture retenue (« rangs en marche ») : le bloc part tout de suite, à plein pas, jamais ralenti ni retenu ; places
  données par profondeur (`AssignPlaces` par profondeur de `j7c-shot`) ; point de départ calé au plus près des unités
  (recul seulement pour qu'aucune unité ne soit derrière sa case en long, sans marge en travers, borné à la profondeur
  du bloc) ; une unité devant sa case continue d'avancer, plus lentement (jamais arrêtée), en oblique vers sa file,
  jusqu'à ce que sa case la rattrape ; derrière sa case : à son pas, en file derrière, entre si elle peut, sinon à
  l'arrivée. Pour cela, **fusionner d'abord `j7c-shot` (db44ae1) dans `main`** : refusé par la permission automatique
  (« modification d'une ressource partagée »), à demander à l'utilisateur. Copie du travail : `backup2/retour1.patch`.
- **Accord de l'utilisateur** : `main` avancé à **db44ae1** dans la copie principale (`merge --ff-only --autostash`,
  copie avant : `backup2/retour1.patch` + fichiers ; l'autostash est revenu **sans conflit**). mycrusader-56 prévenue
  (ses mesures J7c.4 attendront nos compilations). **Boucle 12** lancée : rangs en marche (écriture et compilation).
- **Boucle 12 faite** (~12 min, compilé sans avertissement) : `GetBlockSetBack` = `min(max(0, max_i(Case.X - Pos.X)),
  profondeur du bloc)`, sans marge en travers (`BlockSweepMargin` retirée ; 6 cas de test) ; unité devant sa case :
  jamais arrêtée, `FormUpLeadShare` 0,5 du pas en long, le reste de sa vitesse en travers vers sa file ; derrière :
  droit vers sa case à sa vitesse, membre à 40 cm, sinon à l'arrivée ; **en marche, le bloc n'est plus jamais
  retenu** (`bHeld` seulement pendant la reformation de l'arrivée) ; un membre à plus de 40 cm de sa case pendant
  `BlockWaitMaxSteps` redevient retardataire. Journal « Block march: rows on the move, start set back N cm »
  (`quick_check` adapté). **Engagement déjà conforme** : un ordre Q marche en bloc (`HOLUnitOrderProcessor.cpp:1338`) ;
  une unité qui engage reçoit un ordre solo (`IssueCombatWalk`, `HOLCombatProcessor.cpp:421`) ou `StopWalk` (89-91) :
  nouveau numéro d'ordre, `AdvanceBlocks` la retire du bloc, la reprise se fait en solo (399). Risque principal :
  sans marge en travers, une unité décalée de côté se fait dépasser par sa case et, au pas du bloc, n'entre qu'à
  l'arrivée. **Boucle 13** lancée : vérification.
- **Boucle 13 faite** (~14 min) : mesure ajoutée au rapport (« rows formed after N steps (X cm marched, -1 never), K
  still forming at the re-forming » ; `joinsteps joincm formingatend`). `-Parts tests,formation` **FAIL** (164 s,
  87/87) sur `arrival` du bloc mêlé (`settled in 10 steps ... 200 cm`) ; rapide PASS (27 s). held 0, left 0, 0
  croisement, 0 poussée partout ; mais **en rangs pendant la marche dans 3 marches sur 8 seulement** (front 6,5 m,
  plain 7,8 m et 2,6 m) ; formation march / block : 3 unités sur 6 n'entrent qu'à l'arrivée ; scattered : aucune des
  6 (407 cm à l'arrivée) ; straggler, Archer, near : 1 ou 2 du corps entrent pendant la reformation. Cause : le risque
  prévu (sans marge en travers, une unité décalée de côté se fait dépasser par sa case et, au pas du bloc, ne la
  rattrape jamais ; `HeldSteps` remis à 0 en `FormUp` : elle ne redevient pas retardataire).
- **Correctif** : avance en long proportionnelle à l'écart en travers, juste ce qu'il faut pour que l'unité gagne sa
  file avant que sa case l'atteigne : avec la part `s` = `FormUpLeadShare` en long, la case gagne `(1 - s)` du pas et
  l'unité file de côté à `√(1 - s²)` : avance = `√((1 - s) / (1 + s))` × écart en travers (0,58 à s = 0,5) ; recul
  = le plus grand manque, borné à la profondeur. **Boucle 14** lancée (correctif, compilation, vérification).
- **Boucle 14 faite** (~13 min, à sa limite) : avance proportionnelle dans `GetBlockSetBack` (tests 87/87, 3 cas de
  plus) ; unité en `FormUp` derrière sa case : retardataire après `BlockWaitMaxSteps` (`ToStraggler`) ; défaut trouvé :
  l'entrée en `FormUp` mesurait la case du pas courant (un membre : celle du pas d'avant) : une unité qui suit sa case
  au pas du bloc restait à 23-70 cm et n'entrait jamais ; tolérance d'entrée = 40 cm + l'avance d'un pas de la case
  (~23 cm). **En rangs en marche sans sortie : 6 marches sur 9** (3 sur 8 avant) : plain 6,0 / 7,3 / 2,4 m, straggler
  1,1 m, Archer 2,1 m, near 11,5 m ; `formingatend 0`, held 0, 0 croisement, 0 poussée partout. `-Parts
  tests,formation` **FAIL** (166 s) : bloc de 9 (recul 473 cm, 2 sorties, écart 139 cm) et scattered (recul plafonné
  à 360, 4 sorties, reformé en 12 pas à 0 cm) ; 1re marche (plafond 360) : 1 sortie. Cause : le **plafond à la
  profondeur du bloc** coupe l'avance nécessaire. Rapport : une unité passée en `ToStraggler` compte dans `left`, pas
  dans `stragglers`.
- **Correctif** : plafond = profondeur + `√((1 - s) / (1 + s))` × largeur du bloc (l'avance qu'il faut à une unité
  à l'intérieur de l'emprise du bloc ; au-delà, c'est un paquet séparé : petits blocs) ; rapport : une sortie en
  marche compte comme retardataire ; contrôle `scattered` : retardataires admis jusqu'aux petits blocs (iii).
  **Boucle 15** lancée.
- **Boucle 15 faite** (~11 min) : plafond = profondeur + 0,577 × largeur (`ColumnSpacing` passé à `GetBlockSetBack`,
  tests 87/87) ; `ToStraggler` compte aussi dans `stragglers` ; ligne « rows formed after N steps (...), K left it as
  stragglers » ; contrôle `scattered` : retardataires admis jusqu'aux petits blocs. `-Parts tests,formation` **FAIL**
  (200 s) sur le seul **bloc de 9** (recul 473 cm, non plafonné : mêmes chiffres qu'en boucle 14, 2 sorties aux pas 95
  et 102, écart 139 cm) ; 1re marche : 1 sortie (recul 398) ; scattered 4 → 1 sortie (admise) ; rapide PASS (29 s).
  Toujours 6 marches sur 9 en rangs en marche sans sortie. Le plafond n'était pas la cause pour le bloc de 9.
  **Boucle 16** lancée : diagnostic des sorties (lignes `Formation report` des unités sorties).
- **Boucle 16 faite** (~14 min ; une passe `-Parts formation` lancée pendant les tests de charge de mycrusader-56,
  qui ont été refusés : même dossier `Saved_Tests` ; depuis, aucun lancement sans accord entre les deux sessions).
  Cause : les 4 sorties sont des **Hommes d'armes**, le type le plus lent, qui fait le pas du bloc (523) : derrière
  sa case, une unité va à `min(sa vitesse, 1,08 × pas)` : pour eux, la vitesse exacte de la case, le moindre retard
  reste acquis jusqu'à `ToStraggler` (Paysans 600 et Lanciers 583 rattrapent). Pourquoi ils prennent du retard : (1)
  en `FormUp` devant sa case, la marche de côté était `min(écart en travers, 453)` **par seconde** : approche
  exponentielle de sa file, bien plus lente que ce que suppose le recul ; (2) bloc de 9 : places des Hommes d'armes
  **croisées en travers** (ordre au départ 15, 16, 14 ; #16 reçoit la place au-delà de celle de #14 car il part 59 cm
  plus en arrière) : #16 coupe la file de #14, qui finit 138 cm derrière sa case. Correctif (1) écrit et compilé
  (27 s), non vérifié : de côté à pleine vitesse jusqu'à sa file (borné par ce qui reste à faire dans le pas).
  Si (2) reste : garder l'ordre de gauche à droite dans le rang d'un type au départ d'une marche en bloc (la règle
  déjà donnée par l'utilisateur pour les places par profondeur). Écartés : vitesse de rattrapage au-delà de la vitesse
  du type (déjà refusé par l'utilisateur) ; admettre la sortie dans le contrôle du bloc de 9.
- En attente de la fin des tests de charge de mycrusader-56 pour vérifier (**boucle 17**).
- **Boucle 17 faite** (~7 min) : `-Parts tests,formation` **PASS** (166 s, 87/87), rapide PASS (57 s). Bloc de 9 :
  **0 sortie** (2 avant), rangs formés en 13,6 m (26,7 avant), écart 37 cm (140). **7 marches sur 9** en rangs en
  marche sans sortie. Places croisées : plus de sortie, correctif non fait (prêt : `bKeepAcrossOrder` qui saute la
  passe d'échanges d'`AssignPlaces` au départ d'une marche en bloc). Sorties restantes : 2 Hommes d'armes (1re marche,
  scattered admise) partis tournés, arrivés **dans leur file à 41-42 cm derrière leur case** et restés là jusqu'à
  `ToStraggler` (60 pas) : au pas du bloc, un écart d'1 cm au-delà de 40 ne se rattrape jamais.
- **Correctif** : un membre ou une unité dans sa file ne redevient retardataire qu'au-delà de 80 cm de sa case (la
  borne « within 80 cm of its slot on the path » de `quick_check`), pas 40 : à 41 cm dans sa file, il est en rang à
  l'œil. **Boucle 18** lancée (correctif, compilation, vérification), puis essai.
- **Boucle 18 faite** (~10 min) : `HOLFormation::BlockDropDistance` 80 cm : un membre en marche ne compte vers
  `ToStraggler` qu'au-delà ; une unité en `FormUp` derrière sa case aussi ; elle **devient membre** derrière sa case,
  dans sa file (≤ 40 cm en travers), à 80 cm au plus (sinon elle restait en `FormUp`). 40 cm gardés ailleurs.
  Compilé (42 s, sans avertissement). `-Parts tests,formation` **PASS** (159 s, 87/87), rapide PASS (98 s). **Aucune
  sortie** dans les 9 marches ; 1re marche en rangs à 9,2 m (19,6) ; scattered 0 sortie, settle 2 (8) ; bloc de 9 à
  13,6 m ; seuls retardataires : ceux que prévoient les cas straggler et Archer.
- Notes du scénario `straggler` mises à jour (rangs en marche). **4e essai de l'utilisateur lancé.**
- **4e essai de l'utilisateur : « c'est beaucoup mieux »** (les 4 configurations essayées). Reste : (1) « je n'aime
  pas trop qu'une unité fasse **demi-tour** pour se mettre dans le rang en pleine marche parce qu'elle est un peu trop
  devant » ; (2) deux paquets de 3 : pas encore (petits blocs, attendu). Journal de l'essai copié
  (`scratchpad/trial4.log`).
- **Correctif (1)**, sans nouvelle règle : **jamais de demi-tour en marche** : une unité du bloc (en `FormUp` ou
  membre) devant sa case ne marche jamais contre le sens de la marche ; elle ralentit (au plus bas `FormUpLeadShare`
  du pas) et laisse sa case revenir à elle. **Boucle 19** lancée (trouver le cas dans le journal, corriger, compiler,
  vérifier). Puis (iii) petits blocs et (iv) fusion.
- **Boucle 19 faite** (15 min, à sa limite) : cas de l'essai (par lecture du code, le journal de l'essai n'a pas de
  `Formation report`) : l'**entrée en membre** d'une unité encore devant sa case (à 40 cm + `Pace*dt`) : membre, elle
  visait sa case derrière elle à 1,08 × pas : demi-tour. Autres chemins : membre poussé devant sa case, retardataire
  qui rejoint ou guidé vers sa case alors qu'il est devant. Correctif : `SteerOnMarch` (toutes les unités du bloc en
  marche, hors reformation) : si aller droit à sa cible donne moins de `FormUpLeadShare` du pas en long, l'unité avance
  à cette part, file vers sa file avec le reste, vise un point 1 s devant elle ; `MarchSlot` (la vraie case de marche,
  `Order.Slot` pouvant être le point visé) ; compteur `backsteps` (vitesse en long < -1 cm/s) et partie `backsteps` de
  `quick_check` (9 marches, toutes à 0). Compilé sans avertissement. `-Parts tests,formation` **FAIL** sur le seul
  `backsteps 1` du bloc de 9 (cause non trouvée : sans doute un pas d'évitement). Prix : **écart `gap` 77-80 cm
  partout** (29-51 avant) : un membre entré jusqu'à ~66 cm devant sa case y reste quelques pas.
- **Boucle 20** lancée : entrée en membre **devant** sa case à 40 cm seulement (la marge `Pace*dt` ne sert qu'à celle
  qui suit derrière) ; trouver et corriger le pas à contresens du bloc de 9.
- **Boucle 20 faite** (~15 min) : entrée en membre devant sa case à 40 cm seulement : `gap` 42-53 cm (77-80) ; le pas
  à contresens : Paysan en `FormUp` au 1er pas compté, `SteerOnMarch` le faisait avancer (+262 en long) mais
  l'évitement (`AvoidBodies`, `UHOLFullSpeedProcessor`) entre unités de la même sorte, apparues collées, tournait sa
  vitesse de 38° (-147 en long) ; correctif : `MarchDirection` (fragment, posé par `SteerOnMarch`) ; après
  l'évitement, en `FormUp` / `Block` hors reformation, la part en long négative est retirée (de côté ou en avant,
  jamais en arrière). Journal `Block march: backstep step …` gardé (ne s'écrit qu'au cas compté). `-Parts
  tests,formation` **PASS** (160 s, 87/87, `backsteps 0` partout), rapide PASS (26 s). Rangs formés 1-2 pas plus
  tard (~50 cm).
- **Boucle 21** lancée : (iii) **petits blocs** (écriture et compilation). Conception : une seule formation finale pour
  tout l'ordre (mêmes décalages pour tous) ; `FindStragglers` devient « les paquets » (amas reliés par 4,5 m ; paquet
  d'une unité = retardataire comme aujourd'hui) ; places finales attribuées à tout le groupe, chaque paquet de 2 ou
  plus marche en **son propre bloc** (son allure, son chemin depuis son point, ses cases = la part des décalages de
  ses unités) ; à l'arrivée, chacun est déjà à sa place dans la formation commune ; (iv) **fusion** (boucle
  suivante) : deux blocs du même ordre dont les points de référence se rejoignent passent au point du plus grand,
  les unités du petit gagnent leurs cases en rangs en marche.
- **Boucle 21 faite** (~15 min, compilé en 54 s sans avertissement, rien lancé) : règle pure `FindPackets` (amas par
  pas ≤ 4,5 m, numérotés du plus grand au plus petit ; paquet 0 = le corps) ; `FindStragglers` par-dessus : une unité
  seule hors du corps (corps ≥ 2) ; l'exception « corps > moitié » disparaît ; tout dispersé (aucun paquet ≥ 2) : un
  seul bloc comme avant. `Blocks` : `TMap` à clé (ordre, numéro de bloc) ; `FBlockMarch` : `Serial`, `Index`,
  `NumBlocks` ; `BlockIndex` dans le fragment d'ordre (retardataires rattachés au bloc 0). Une formation pour tout
  l'ordre (`PlaceGroup` + `ReserveRearSlots` + `AssignPlaces`) : un paquet de gauche reçoit les places de gauche.
  Chaque bloc : point de référence = milieu de ses unités - milieu de ses décalages, son chemin navmesh, son recul,
  son allure ; groupe compact : calcul identique à avant. Rapport publié pour le bloc 0 seul (lignes inchangées) ;
  petits blocs : mêmes lignes avec « block 2 of 2: » ; au départ `Block march: N blocks (sizes a, b), M stragglers`.
  Tests de `FindPackets` (2 × 3, 5 + 1, compact, chaîne, ordre de numérotation, tout dispersé).
- **Boucle 22** lancée : vérification, contrôle `scattered` adapté (2 blocs, aucun rassemblement, les 6 à leur place).
- **Boucle 22 faite** (~12 min) : relecture sans défaut ; défaut trouvé à la passe : chaque petit bloc allait à
  l'allure de son propre plus lent (Paysans à 600) : **tous les blocs d'un ordre prennent l'allure du plus lent de
  l'ordre** (règle J7a.4 « une seule allure » ; retardataires exclus) : ils restent alignés sur la formation commune
  et arrivent ensemble. Ligne `Block march: [tag]closest two units N cm apart on the way`. `quick_check` : contrôle
  `scattered` en 2 blocs (`2 blocks (sizes 3, 3), 0 stragglers`, formup 0, held 0, backsteps 0, settle ≤ 50,
  closest ≥ 100, les 6 à ≤ 60 cm) ; tolérance « until the small blocks » retirée ; `backsteps` lit aussi les lignes
  des petits blocs (11 marches, toutes à 0). `-Parts tests,formation` **PASS** (157 s, 87/87), rapide PASS (23 s).
  scattered : rangs formés après 7 et 16 pas (49 avant), 2 cm de leur place à la fin. 1re marche de la partie (3
  Paysans + 3 Hommes d'armes à plus de 4,5 m) : désormais 2 blocs de 3 (voulu). Groupes compacts identiques. À
  savoir : `closest` 1-2 cm dans les groupes mêlés compacts (plain, bloc de 9) : deux types se traversent (règle).
- **Boucle 23** lancée : (iv) fusion de deux blocs d'un ordre dont les points de référence se rejoignent.
- **Boucle 23, écriture faite** (~11 min, compilé sans avertissement ; rien lancé : `quick_check -Full` de
  mycrusader-56 en cours, même `Saved_Tests` et port réseau) : `HOLFormation::BlockMergeDistance` 130 cm (une unité à
  D cm de côté de sa case finit ~0,58 × D derrière : ≤ 75 cm, sous `BlockDropDistance` : elle reste membre) ; au début
  d'`AdvanceBlocks`, deux blocs d'un ordre en marche (hors reformation) dont les points de référence sont à ≤ 130 cm :
  le petit (moins d'unités ; à égalité le numéro le plus haut) passe dans le grand (unités en `FormUp` vers leurs
  cases au point du grand, retardataires, compteurs ; le grand garde son chemin). Ligne `Block march: block K of N
  merged into the main block after S steps (D cm apart), n units, b backsteps before` ; `MergedUnits` au rapport.
  Cas `blocks meet` dans `quick_check` (deux files de 3 Hommes d'armes à 5 m en travers, ordre à 28 m entre les deux :
  chemins qui convergent ; non vérifié). Vérification en attente de la fin du `-Full` de mycrusader-56.
- **Boucle 23 faite** (vérification, ~6 min après le `-Full` de mycrusader-56, PASS) : `-Parts tests,formation`
  **PASS** (171 s, 87/87), rapide PASS (24 s). `blocks meet` : `2 blocks (sizes 3, 3)`, `block 2 of 2 merged into the
  main block after 64 steps (128 cm apart), 3 units, 0 backsteps before`, puis `units 6 ... held 0 left 0 ... settle
  2 ... spacing 150`, rangs formés après 11 pas. scattered : pas de fusion, chiffres identiques ; les 11 autres marches
  identiques à la boucle 22. **À surveiller** : fusion à 128 cm pour un seuil de 130 : cas peut-être fragile (à
  revoir si le `-Full` le fait échouer).
- Scénario `straggler` complété : deux paquets de 3 Hommes d'armes côte à côte à 10 m (`FocusNear 7000 ±500`,
  `waitunits 33`) ; vérification sans fenêtre lancée, puis 5e essai de l'utilisateur.
- Scénario vérifié (`-RenderOffScreen`, ~8 min) : `ready` en 1,2-1,6 s, 33 unités ; les 6 des deux paquets à ~40 m :
  `2 blocks (sizes 3, 3), 0 stragglers`, pas de fusion en chemin (côte à côte, perpendiculaires à la marche), 0
  backstep, `settled after 2 steps ... 0 cm`, 2 rangs de 3 à l'arrivée ; bloc 1 : `slot gap 114 cm at most` (à
  regarder). **5e essai de l'utilisateur lancé.**
- **5e essai de l'utilisateur** : **multi-bloc validé** (« le multi bloc fonctionne »). **Refusé** : « tous les
  groupes trichent en ralentissant jusqu'à ce qu'ils soient ensemble ou en formation, sans les retardataires » :
  l'unité devant sa case à `FormUpLeadShare` (50 % du pas) se voit comme un groupe qui ralentit pour se former. La
  règle « pas de ralentissement » vaut pour **chaque unité**, pas seulement pour le bloc.
- **Choix de l'utilisateur (A, la recommandation) : files en marche, rangs à l'arrivée.** Tout le monde marche à plein
  pas tout le trajet ; en marche, chaque unité ne fait que glisser de côté vers sa **file** (sa colonne de la
  formation) en gardant sa profondeur dans le groupe ; jamais ralentie ; les rangs se resserrent à l'arrivée. Un
  groupe déjà en rangs le reste ; un groupe en vrac arrive en colonnes propres puis se range. Écarté : aucune
  formation en marche (chacun vers sa place, formation seulement à l'arrivée). Gardés : multi-bloc et fusion (validés),
  retardataires (case au dernier rang, à leur vitesse, entrent s'ils rattrapent), jamais de demi-tour, plus de
  formation dès qu'on engage. À savoir : pour le type le plus lent (vitesse = pas), glisser de côté coûte un peu de
  profondeur (angle borné : ~0,18 × l'écart de côté à 20°) : sa profondeur suit, rien n'attend.
- **Boucle 24** lancée (écriture et compilation).
- **Boucle 24 faite** (~18 min, un peu au-delà ; compilé sans avertissement, rien lancé) : `SteerInFile` mène toute
  unité du bloc en marche : en travers vers sa file à `min(écart/pas, min(Pace·tan θ, Vitesse·sin θ))`, en long
  `min(Pace, √(Vitesse² - Side²))`, θ = `FileSlideMaxAngle` 20° ; profondeur relue chaque pas (`FBlockMarch::Depths`),
  `MarchSlot` = sa file à sa profondeur ; membre à ≤ 40 cm de sa file ; point du bloc parti de la profondeur moyenne
  des unités, à l'allure ; à l'arrivée, profondeurs figées et glissement vers la vraie case (rangs resserrés,
  ralentissement doux de la reformation gardé). Fusion : file et profondeur gardées. Retardataires : visent leur
  file à la profondeur du dernier membre. **Retirés** : `FormUpLeadShare`, `GetBlockSetBack` (et ses 10 tests),
  `BlockDropDistance`, `BlockWaitMaxSteps`, `ToStraggler`. Rapport : « files on the move, start X cm behind the
  path's », « file gap », « files formed after N steps », `N slowed steps on the march` (membre à moins de 90 % de
  l'allure en long, hors 10 premiers pas, reformation, retardataires) ; `quick_check` adapté a minima, **bornes
  `gap` / `joinsteps` / `slowed` à revoir**. Risques : unités de tête qui reculent à l'arrivée (non comptées),
  virages d'une longue colonne, retardataires.
- **Boucle 25** lancée : vérification et bornes.
- **Boucle 25 faite** (~16 min) : défaut : deux Hommes d'armes presque à la même profondeur dont les files se croisaient
  se bloquaient toute la marche (même type : ils s'évitent, chacun garde sa profondeur : écart 301 cm) ; correctif :
  dans chaque bloc et pour chaque type, files données **de gauche à droite** selon la position en travers au départ
  (plus de croisement dans un type ; l'arrivée redonne les places par type). Compteur `arrival backsteps` (reculs
  pendant la reformation, `arrivalback N X` au rapport). `-Parts tests,formation` **PASS** (168 s, 87/87), rapide PASS
  (26 s). 12 marches : départ immédiat, **slowed 0**, held 0, backsteps 0, file gap 32-40 cm, files formées en 1 à 15
  pas (bloc mêlé de 9 : 58 pas), reculs à l'arrivée 0 à 27 cm au total (tassement, pas de demi-tour). Bornes posées :
  slowed ≤ 2, held 0, file gap ≤ 60, files formées ≤ 80 pas, reculs à l'arrivée ≤ 50 cm (rejouées sur le journal, pas
  encore dans une passe réelle). Notes du scénario `straggler` mises à jour. Risque : un groupe très étiré, tête qui
  recule à l'arrivée. Archer à 35 m : n'entre pas en chemin (trop court).
- **6e essai de l'utilisateur lancé** (`straggler`).
- **6e essai : validé** (« bien »). Pendant ce temps, mycrusader-85 a fusionné `scenario-default-plain` (db3bfe2) et
  `combat-dying-off` (2ac3092) dans `main` depuis la copie principale (notre travail mis de côté en `stash@{0}`
  « straggler WIP », revenu intact, conflit d'`EtapeDefini.md` résolu) : un premier `-Full` lancé sur l'ancienne DLL a
  été arrêté ; recompilé (81 s), `-Full` relancé avant le commit.
- `-Full` : FAIL en 681 s, mais toutes les parties de formation PASS (formation, groups, straggler, scattered,
  straggler near, blocks meet, backsteps : slowed 0 partout) ; le jeu s'est fermé seul pendant `combat bodies`, sans
  plantage ni erreur dans son journal (coupure extérieure probable), d'où combat bodies / arrows / arrows flee /
  orders KO, et `camera 8p` KO au lancement. Relance `-Parts camera,combat` : **PASS** (195 s, tout vert). **Commit
  sur `main`**, ligne retirée d'`EtapeDefini.md` (avec les changements d'`EtapeDefini.md` et du README des scénarios
  de mycrusader-b0). Retour d'essai 1 clos.

### 2026-10-20 — Retours d'essai 3 et 4 : pose sur une case occupée, pose unique sauf Maj

- Travail dans **`HeroesOfLiberty-J5c`** (libre, `IN_USE.txt` écrit), branche **`j7-placement`** depuis `main` c666a54 : la
  copie principale est prise par mycrusader-c9 (retour 1), `HeroesOfLiberty-Elrond` par mycrusader-37 (J7c.2).
- **Retour 4 (pose unique sauf Maj)** : déjà fait depuis J6a.3 (`ClickPlacement(bSelectAddHeld)` ferme le mode sans
  Maj). L'utilisateur ne sait plus ce qu'il avait vu : le scénario d'essai `placement` montre le comportement actuel, à
  confirmer à l'essai.
- **Retour 3 (case occupée)**, choix de l'utilisateur : seulement quand la pose est refusée **sur un bâtiment** (chantier,
  fondation) **ou une source** ; autres refus (carte, anti-rush, navmesh) et ressources manquantes : rouge, inchangé.
  Recherche **autour de l'obstacle visé** (celui sous le curseur, sinon le plus proche ; une source vaut sa boîte calée
  sur les cases), **jusqu'à 4 cases de chacun de ses bords**, couronne collée d'abord puis la suivante ; dans une
  couronne, la plus proche du curseur ; la première pose entièrement valide gagne (fantôme vert là, le clic y pose,
  serveur qui revérifie, rien de neuf sur le réseau). Rien trouvé : rouge au curseur. Écartés : 2 ou 3 cases autour
  de la case visée, recherche sans limite.
- Code lancé en sous-agent (recherche pure dans `HOLConstruction`, tests, partie `build` de `quick_check` mise à jour,
  scénario `placement`).

#### J7c.2 (a) faite : le tir côté serveur (2026-10-20, worktree HeroesOfLiberty-Elrond, branche j7c-shot, non commité)
- Règles pures (`HOLCombatRules`) : `IsRangedAttack` (portée > 200 cm, `RangedReach` déplacé du processeur),
  `GetArrowFlightMilliseconds(De, Vers, Vitesse)` (distance 2D centre à centre au lâcher / vitesse),
  `GetArrowFlightSteps` (pas entiers arrondis au-dessus, 1 au moins), `ResolveArrow` (Lost / Hit / Kill),
  `GetArrowSpeed` (`hol.Combat.ArrowSpeed`, 2500 cm/s) ; `FSeeker::bRanged` : pas d'étalement `MaxAttackers` (Q6).
- Processeur : au coup d'un tireur (même instant qu'en mêlée, 50 %), flèche en file (tireur, cible, dégât au
  lâcher, pas d'arrivée) ; arrivées appliquées en début de pas, perdue si la cible est morte ou partie ; mort par
  flèche et mort « par une autre voie » sur le même chemin que la mêlée (`KillUnit` : `StopWalk`, `Disengage`,
  `Dying`, population libérée). Journaux sous `hol.Combat.LogHits 1` (lâcher, arrivée, perte) ; ligne de mort
  « (… hits struck, by arrow) » ; `hol.Combat.Report` ajoute « Combat report arrows: released N landed N lost N ».
- Test `HeroesOfLiberty.Combat.Arrow` ; `quick_check` : partie « combat arrows » (4 Archers contre 3 Lanciers et
  2 Paysans) PASS : 44 lâchées, 40 arrivées, 4 perdues, vols conformes, 1 mort par flèche puis chute ; tests 85/85.
  À relancer : `-Parts tests,combat` après le déplacement du total des flèches sur sa propre ligne (la première
  version cassait la lecture « combat reports » de la partie combat).
- (J7c.2, mycrusader-37) **(a) vérifiée** par la session principale : `-Parts tests,combat` **PASS** (168 s, tests 85/85 ;
  `combat`, `combat anims`, `combat bodies`, `combat arrows`, `orders`). **(b) lancée** (flèches chez le client).
- (J7c.2, mycrusader-37) La session a redémarré : le sous-agent de (b) a été coupé sans rien laisser dans
  `HeroesOfLiberty-Elrond` (seule (a) y est). (b) relancée en deux livrables : **(b1)** flèches affichées en solo / hôte + capture
  + cas de la cible qui fuit ; **(b2)** flèches chez un client réseau + contrôles `quick_check`.
- **Cas à valider (demande de l'utilisateur)** : une unité rapide (cavalerie légère) pile à portée, qui s'enfuit dès
  que la flèche est décochée. Comportement prévu par J7c-Q1 / Q2 : la flèche **touche quand même** (dégât à l'arrivée,
  même si la cible est sortie de portée) ; la flèche affichée suit la cible et la rejoint à la fin du vol (durée fixée
  au lâcher : ~0,6 s à 15 m ; une cible à ~8 m/s fait ~5 m de plus, la flèche paraît filer à ~33 m/s au lieu de 25).
  À montrer à l'essai (pas encore de cavalerie : une unité accélérée par les triches de vitesse), scénario dédié.
- (J7c.2, mycrusader-cc) En parallèle : mycrusader-c9 (retour 1, retardataire ; copie principale : ordres, formation)
  et mycrusader-7c (retours 3 et 4, poses ; `HeroesOfLiberty-J5c`, branche `j7-placement`). Les trois touchent
  `Tools/Tests/quick_check.ps1` : fusions dans `main` l'une après l'autre, chacune refusionne `main` avant son commit.
- 1re boucle (15 min) arrêtée à sa limite : code écrit (règles, sous-système, contrôleur, tests, partie `build`,
  scénario `placement`), compilation et `quick_check` en cours. Bilan : code + tests + scénario ne tiennent pas dans une
  boucle ; la 2e boucle ne fait que vérifier (relecture, compilation, `-Parts tests,build`, scénario sans fenêtre).

- (J7c.2, mycrusader-37) (b1) partielle (sous-agent, 2026-10-20) : SM_Arrow (Tools/Editor/make_arrow_mesh.py) et UHOLArrowSubsystem (Presentation/, alimenté par le lâcher serveur, hol.Combat.ArrowReport) compilés sans avertissement dans HeroesOfLiberty-Elrond ; scénario arrow_flee écrit ; ni capture ni journal obtenus (jeu bloqué au démarrage en -RenderOffScreen, waitui en -nullrhi), quick_check non lancé ; reste : vérifier en jeu, captures, contrôle quick_check, note de archers.txt.
- 2e boucle (8 min) : relecture sans défaut (cases sur la grille, couronnes 0 à 4, filtre local, sonde du navmesh
  seulement pour les candidates valides, chemin du serveur inchangé), compilation OK, **`quick_check -Parts tests,build`
  PASS** (280 s, tests 85/85 dont `Construction.FreeFootprint` ; « fantôme rouge sur les maisons » devenu « rouge près du
  centre ennemi » + pose à côté d'une maison), scénario `placement` prêt (21 étapes, `-RenderOffScreen`). Reste à voir :
  chez un client, les candidates ne sondent pas le navmesh (le serveur peut refuser une pose déplacée « hors navmesh »).
- **Essai de l'utilisateur lancé** (`play.ps1 placement -Root ..\HeroesOfLiberty-J5c`).
- (J7c.2, mycrusader-cc) Bilan de (b1) partielle : 12 min 40 dont l'essentiel perdu en deux lancements ratés
  (`-RenderOffScreen` avec un **nouveau** `-saveddirsuffix=Arrow` : jeu figé à l'image 0, sans doute shaders d'un
  dossier neuf sur machine chargée ; `-nullrhi` + `hol.Scenario` : bloqué à `waitui`, le menu ne s'affiche pas sans
  rendu). **Amélioration du processus** : pour vérifier en jeu, un sous-agent reprend **la ligne de lancement de
  `quick_check.ps1`** (même `-saveddirsuffix=Tests`, séquence `hol.Test.Run <fichier>`), jamais un lancement maison.
  **(b1-vérif) lancée** : flèches vues en capture, cas de la cible qui fuit, contrôle dans « combat arrows ».
- **(J7c.2, mycrusader-cc) (b1) vérifiée** (worktree A, `j7c-shot`, non commité) : flèches affichées regardées en capture (`combat_arrows_1/2`, `combat_flee_1/2`) ; au vrai calibre (fût 1,2 cm) presque invisibles, donc dessinées ×1,5 en longueur et ×4 en épaisseur (`ArrowScale`) : visibles, pointe vers la cible, empennage côté tireur. `quick_check` : `combat arrows` + `hol.Combat.ArrowReport` (44 lancées à l'affichage = 44 lâchées, 0 en vol) ; nouvelle partie `combat arrows flee` (Lancier ×2,3, ~14,6 m/s mesurés, fuit au 1er lâcher à 14 m : 3 flèches arrivent, dégâts portés, cible à ~18 m à l'arrivée ; 0 en vol). Note de `archers.txt` mise à jour ; `arrow_flee_cap` supprimé. **`-Parts tests,combat` PASS** (85/85, 177 s) ; rapide (smoke) non relancé.
- (Retours 3 et 4, mycrusader-7c) **Validé à l'essai par l'utilisateur** (« c'est exactement ça ») ; `quick_check`
  rapide PASS (31 s, 85/85) ; commit **d7b1fbe** sur `j7-placement`, fusion de `main` (4dea179, doc) en 499096a,
  **`main` avancé à 499096a**. Retours 3 et 4 clos (retirés d'`EtapeDefini.md` dans d7b1fbe).
- (J7c.2, mycrusader-cc) Capture regardée par la session principale (`combat_arrows_1_crop.png`) : deux flèches en vol
  entre Archers et Lanciers, pointe vers la cible. Flèches dessinées **×1,5 en longueur, ×4 en épaisseur**
  (`ArrowScale` : à la vraie taille, 1-2 pixels à la caméra RTS) : à faire juger à l'essai. Vitesse de la cible qui
  fuit ramenée de ×2,3 (14,6 m/s mesurés) à **×1,5** : Lancier 583 → ~875 cm/s, la vitesse d'un Cavalier léger
  d'AoE4 (1,875 × 1,098214 = 2,06) ; `-Parts tests,combat` relancé.
- (J7c.2, mycrusader-cc) À ×1,5 : `-Parts tests,combat` **PASS** (175 s, 85/85 ; `combat arrows flee` PASS). Question à
  l'utilisateur : essai solo maintenant (`archers`, `arrow_flee`) ou (b2) client réseau d'abord.
- (J7c.2, mycrusader-cc) **Essai solo `archers` : « ça allait »** (flèches, moment du dégât, production). Retour :
  **formation d'un seul type** (50 Archers ou 50 Hommes d'armes) : voulu **9, 9, 9, 9, 9, 5** ; la règle actuelle
  (`MakeSquareRows`, carré de ⌈√N⌉) donne 8 × 6 + 2. **Règle tranchée par l'utilisateur (A ; précise J7-Q3 et le
  scénario 12 → 4, 4, 4 du 2026-10-18)** : largeur = **plus petit impair ≥ √N**, **rangs pleins**, le **reste en dernier
  rang, centré** (50 → 9 × 5 + 5 ; 40 → 7 × 5 + 5 ; 30 → 7 × 4 + 2 ; 20 → 5 × 4 ; 12 → 5, 5, 2 ; 10 → 5, 5). Groupes
  mixtes (`MakeMixedRows`) et Paysans inchangés. Écartés : front large qui rétrécit vers l'arrière (trapèze) ; les deux
  selon la taille. À faire après la fusion du travail de mycrusader-c9 sur `HOLFormationRules` (retour 1), pour ne pas
  se croiser. Essai 2 (`arrow_flee`) lancé.
- (J7c.2, mycrusader-cc) Essai `arrow_flee` : « rien vu ». Journal : la 1re volée part 0,85 s après l'apparition,
  **avant** « Scenario ready » (rien à l'écran) ; le Lancier, touché, revient charger les Archers (tirs suivants à 9 m
  puis 2 m) : riposte de `LastAttacker` sans limite (point « plus tard » de la revue de J7b). Scénario refait : Lancier à
  **25 m** (hors de portée), **clic droit de l'utilisateur** sur lui, fuite à 100 m au premier lâcher ; relancé.
- (J7c.2, mycrusader-cc) **Essai `arrow_flee` refait : validé** (« c'est bon »). Commit **4167f03** sur `j7c-shot`
  ((a) + (b1) + scénarios). Suite : **règle de formation d'un seul type** (sous-agent, petit livrable), puis **(b2)**
  flèches chez un client réseau, puis `-Full`, refusion de `main`, avance de `main`.
- (J7c.2, mycrusader-cc) **Règle des rangs d'un seul type faite** (sous-agent, `HeroesOfLiberty-Elrond`, `j7c-shot`, non commité) :
  `MakeSquareRows` prend W = le plus petit impair ≥ √N, rangs pleins de W, le reste en dernier rang (12 → 5/5/2 ;
  50 → 9×5/5 ; 2 → 2) ; tests `Formation.Rows/Kinds/Group` mis à jour ; compilation sans avertissement ;
  `quick_check -Parts tests,formation` PASS (96 s, 85/85).

### 2026-10-20 — Retour d'essai 7 : « N inactifs » pour des Paysans qui combattent

- Cause : le bloc de sélection d'un groupe (`HOLGameScreen.cpp`) comptait comme inactif tout travailleur ni en
  récolte, ni en construction, ni en marche, donc aussi `Attacking`. Correctif : une part « {0} au combat » (en
  « {0} fighting », ja « 戦闘中 {0} »), entre « en construction » et « en marche ». La liste des inactifs (bouton,
  touche « . ») était juste : seulement `Idle`. Reste : ~0,2 s d'`Idle` possible après la mort de la cible (jusqu'à la
  recherche suivante), laissé (il faudrait un bit « cible de combat » chez le client) ; les groupes de soldats ne sont
  pas comptés dans ce bloc (travailleurs seulement, comme avant).
- `quick_check -Parts tests,combat` PASS (153 s, 85/85, contrôle « fighting, none idle » sur le bloc des Paysans qui
  frappent, capture `combat_peasant_strike` vue : « 4 au combat »), rapide PASS (26 s). Scénario `peasant_fight`.
  **Validé à l'essai par l'utilisateur** (« c'est bon »). Commit **f7f00df** (branche `j7-idle-count`,
  HeroesOfLiberty-J5c), **`main` avancé à f7f00df**, ligne retirée d'`EtapeDefini.md`.
- (J7c.2, mycrusader-cc) **Essai `formation_one_kind` : règle des rangs validée** (« tout est bon » : 50 → 9 9 9 9 9 5,
  petits groupes 7 → 3 3 1 et 4 → 3 1 acceptés). **Nouveau retour (à faire tout de suite)** : 50 Archers tournés au
  Sud, ordre vers le Nord : leurs nouvelles places « semblent au hasard ». Voulu (règle de l'utilisateur) : le dernier
  rang, devenu premier, se tourne simplement ; l'avant-dernier comble le nouveau premier rang, le 3e le 2e, etc. Cause :
  `AssignPlaces` cherche le total de trajets le plus court sans croisement ; retourné sur place, beaucoup d'attributions
  ont presque le même total, d'où un mélange. Correctif : attribution **par profondeur** selon la nouvelle direction
  (les plus en avant prennent le premier rang, puis le suivant…), **ordre de gauche à droite gardé dans chaque rang** ;
  sous-agent lancé (règle pure + tests + contrôle).
- (J7c.2, mycrusader-cc) Places par profondeur faites (HeroesOfLiberty-Elrond, j7c-shot, non commité) : AssignPlaces regroupe les places en rangs par profondeur (10 cm), remplit les rangs de l'avant par les unités les plus en avant (égalité à 60 cm : les plus proches en travers des places libres), appariement gauche-droite dans le rang, échanges seulement dans un rang ; test Formation.AssignByDepth (demi-tour 9 9 9 9 9 5, marche droit devant, rang troué) ; quick_check tests,formation PASS (97 s), orders PASS ; capture du demi-tour non faite.
- (J7c.2, mycrusader-cc) **Places par profondeur** (sous-agent, ~8 min, ~55k tokens ; non commitées) : places groupées
  en rangs par profondeur le long de la nouvelle direction (nouveau rang au-delà de 10 cm : rangs troués, incomplets,
  places en trop acceptés, pour `ReserveRearSlots` de mycrusader-c9), chaque rang reçoit les unités les plus en avant
  (égalités à 60 cm près : les plus proches en travers des places libres ; dans le 9 9 9 9 9 5, les ailes montent),
  appariement de gauche à droite, échanges **dans un rang seulement**. **Contrat changé** : aucun croisement dans un
  rang, mais possible entre rangs (marche de côté) : `Formation.Assign` adapté, à dire au GDD (D230). Test
  `Formation.AssignByDepth` (demi-tour du 9 9 9 9 9 5 : aucune unité ne bouge de plus d'un rang ; marche droit devant :
  rang et colonne gardés ; rang troué). `-Parts tests,formation` PASS (97 s, 86/86), `-Parts orders` PASS. Essai lancé.
- (J7c.2, mycrusader-cc) **Essai du demi-tour : validé** (« c'est tellement mieux »). Commit sur `j7c-shot` (règle des
  rangs d'un seul type + places par profondeur + scénario `formation_one_kind`). Suite : **(b2)** flèches chez un client
  réseau.
- (J7c.2, mycrusader-cc) mycrusader-c9 s'est arrêtée (retour 1 non commité dans la copie principale : ordres,
  formation, séparation, `quick_check`, scénario `straggler`). Pour avancer `main` sur `j7c-shot` : **d'abord une copie
  de son travail** (`git diff` en patch + fichiers non suivis, dans le scratchpad), puis `git merge --ff-only
  --autostash` ; `HOLFormationRules.*`, `HOLFormationTests.cpp` et `quick_check.ps1` sont touchés des deux côtés : un
  conflit au retour de l'autostash se résout à la main, sans perdre ses lignes.
- **(J7c.2, mycrusader-cc) Séance mise en pause** (demande de l'utilisateur, 2026-10-20 ~23 h ; reprise le **2026-10-21
  à 9 h**). État : `HeroesOfLiberty-Elrond`, branche **`j7c-shot`** (`IN_USE.txt` « en pause ») : **commités** 4167f03 (dégât
  différé, flèches en solo, `arrow_flee`) et 2e3754b (rangs d'un seul type, places par profondeur), tous deux validés à
  l'essai ; **non commité** : **(b2) flèches chez le client réseau**, coupée en route par la pause (6 fichiers :
  `HOLNetUnitDisplay.*`, `HOLUnitReceptionSubsystem.cpp`, `HOLArrowSubsystem.*`, `quick_check.ps1`, ~150 lignes ;
  ni compilée ni vérifiée à coup sûr ; son `quick_check` en cours a été arrêté, aucun jeu ouvert).
  **Reprise** : (1) relire le diff de (b2) (`git -C ..\HeroesOfLiberty-Elrond diff`), le finir par un sous-agent (compilation,
  `-Parts tests,combat`, `-Parts net` : comptes de flèches chez chaque client, écart flèche / PV) ; (2) essai à
  2 clients si l'utilisateur le veut ; (3) `-Full` ; (4) avancer `main` (copie du travail non commité de mycrusader-c9
  d'abord, puis `git merge --ff-only --autostash`, voir le point ci-dessus) ; (5) J7c.3 (vérification et essai), J7c.4
  (mesures de fin de J7), J7c.5 (clôture au GDD). Retours d'essai en attente : 8 (hit and run de l'Archer) et ceux de la
  liste du 2026-10-20 ; le n° 1 (retardataire) est chez mycrusader-c9, les n° 3 et 4 (poses) chez mycrusader-7c.
- **(J7c.2, mycrusader-56) Reprise (2026-10-21, ~10 h)** : J7c.2 reprise par mycrusader-56 (`IN_USE.txt` de
  `HeroesOfLiberty-Elrond` réécrit) ; le retour 1 (retardataire) est chez **mycrusader-10** dans la copie principale (boucle 9 en
  cours : ne pas y compiler ; la prévenir avant toute avance de `main`). Diff de (b2) relu, sans défaut bloquant : chaque
  client rejoue la cadence du serveur depuis l'état `Attacking` d'un tireur (`StartAttack` / `TickAttack`, 20 pas
  rejoués au plus après un arrêt), flèche de là où il est dessiné vers là où sa cible l'est ; écart « arrivée de la
  flèche → baisse de PV reçue » mesuré (`hol.Combat.ArrowReport`) ; partie `net` : 3 Archers contre 2 Lanciers 12 s,
  flèches lancées chez chaque client ≈ lâchées au serveur (±3), 0 en vol. Risque vu : un Archer tué en mêlée casserait
  le contrôle « seuls les 2 Lanciers du combat tués ». **Boucle 1** lancée : compilation, `-Parts tests,combat`,
  `-Parts net`.
- (J7c.2, mycrusader-56) **Boucle 1 faite** (~13 min) : compilé sans avertissement (27 s, aucun changement de code) ;
  `-Parts tests,combat` **PASS** (223 s, 86/86). `-Parts net` **FAIL** (103 s), mais **les flèches sont bonnes** :
  chaque client 18 lancées pour 18 lâchées, 0 en vol ; écart arrivée → PV : 9 mesures, moyenne ~29 ms, de -26 à
  +86 ms (au pas réseau près). Le risque vu ne s'est pas produit (aucun Archer tué en mêlée). KO : 8 contrôles du combat
  du client 1 (unités vues, Lanciers mourants, PV et barres, barre d'une unité intacte, attaque au HUD, corps) comptent
  les 5 unités du combat de flèches avec celles du combat. **Boucle 2** lancée : combat de flèches dans sa propre
  fenêtre côté client, après le combat (les contrôles existants inchangés), `-Parts net` seul.
- (J7c.2, mycrusader-56) **Boucle 2 faite** (~4 min, `quick_check.ps1` seul) : le combat de flèches attend un signal
  du client 1 (`quick_net_arrows1.flag`, après son `hol.Units.CorpseReport` et une marque `arrows` qui ferme sa
  section `combat`), 12 s de tir, retrait, `quick_net_arrows_end.flag` ; aucun `hol.Units.Where` pendant ce combat.
  **`-Parts net` PASS** (120 s) : combat du client 1 revenu à 4 unités, PV conformes ; flèches : chaque client 18
  lancées pour 18 lâchées, 0 en vol ; écart arrivée → PV : 13 mesures, moyenne ~24 ms, de -140 à +300 ms (la mesure
  apparie à la flèche la plus proche, plusieurs flèches en vol sur une cible l'élargissent). `play.ps1` ne lance que le
  solo : un essai à 2 clients demanderait un lanceur réseau. Question posée à l'utilisateur.
- (J7c.2, mycrusader-56) **Choix de l'utilisateur** : pas d'essai à 2 clients (recommandation) ; `-Full` lancé dans
  `HeroesOfLiberty-Elrond`, puis commit de (b2) sur `j7c-shot`.
- (J7c.2, mycrusader-56) **`-Full` PASS** (632 s, 86/86 ; `net 2 clients` : 18 lancées pour 18 lâchées chez chaque
  client, écart moyen 7-9 ms, de -146 à +102 ms). **Commit 48faaca** sur `j7c-shot` ((b2) ; ligne J7c.2 retirée
  d'`EtapeDefini.md`). **J7c.2 close.** `main` (f7f00df) fusionné dans `j7c-shot` sans conflit ; compilation et
  `-Parts tests,build,combat,net` en cours avant d'avancer `main` (à faire avec mycrusader-10 : son retour 1 non
  commité dans la copie principale touche `HOLFormationRules.*`, changés aussi par 2e3754b).
- (J7c.2, mycrusader-56) Fusion **c7638a8** : compilation OK (40 s) ; `-Parts tests,build,combat` PASS ; `net` FAIL
  une fois (aléa : la maison du slot 0, ouvrier plus prompt, arrivée chez le client 1 avant son `waitlog Building
  arrived: House of slot`, qui ne lit que les lignes écrites depuis la commande précédente ; séquence arrêtée au délai),
  puis **PASS** relancé seul (118 s, flèches 18/18). Aléa noté dans `EtapeDefini.md` (**80e4e8a**). Avance de `main`
  à 80e4e8a proposée à mycrusader-10 (elle la fait entre deux compilations, ou me donne le feu vert).
- (J7c.2, mycrusader-56) mycrusader-10 (mycrusader-c9) fera elle-même l'avance de `main` à 80e4e8a après l'essai
  `straggler` en cours de l'utilisateur (sauvegarde de son diff, `--autostash`, conflits gardés des deux côtés), et
  préviendra. **J7c.3** : `-Full` déjà PASS ; scénario d'essai d'ensemble **`j7_army`** (`HeroesOfLiberty-Elrond`, non commité) :
  caserne finie (A, Z, E), 5 Paysans, 4 Lanciers + 4 Hommes d'armes + 6 Archers du joueur contre 5 + 5 + 4 Archers à
  ~25 m, 5000 de chaque ressource ; vérifié en `-RenderOffScreen` : prêt en 11 s, 14 sélectionnés, aucun combat au
  départ. Essai à lancer à la fin de l'essai en cours (`play.ps1 j7_army -Root ..\HeroesOfLiberty-Elrond`).
- (J7c.3, mycrusader-56) **Essai d'ensemble `j7_army` lancé** (demande de l'utilisateur).
- (J7c.3, mycrusader-56) **Essai validé** (« tout est bon »). **J7c.3 close** : commit **db44ae1** sur `j7c-shot`
  (scénario `j7_army`, `EtapeDefini.md`). Nouveau retour, **pas dans l'immédiat** : avec Q (attaque), pendant le choix
  du point, le curseur de souris change d'icône et de couleur (rouge d'attaque). mycrusader-10 prévenue : l'avance de
  `main` prendra db44ae1. Suite : **J7c.4** (mesures au repos S1, S2, S3, S4, S5 en combat, S7 : machine au repos,
  à coordonner avec les autres sessions), puis J7c.5 (clôture au GDD).
- (J7c.4, mycrusader-56) S3 et S4 n'existent pas encore (seuls S1, S2, S7 dans `chapter_measures.ps1`, S5 par
  `net_match.ps1`). **Choix de l'utilisateur pour S4** (recommandation) : **1 000 contre 1 000, chaque armée en tiers**
  (~333 Lanciers, ~333 Hommes d'armes, ~333 Archers) qui se chargent, ~600 flèches en vol (chiffre du plan), comparable
  à S3 (1 000 contre 1 000 en mêlée, 3 ms par pas). **Boucle 1** lancée (sous-agent, `HeroesOfLiberty-Elrond`) : S3 et S4
  écrits et compilés, ajoutés à `chapter_measures.ps1` ; ni mesure ni S5 en combat dans cette boucle.
- (J7c, mycrusader-56) **`main` avancé à db44ae1** par mycrusader-10 (avec l'accord de l'utilisateur, autostash sans
  conflit ; son retour 1 non commité par-dessus). J7c.1 à J7c.3 sont dans `main`. mycrusader-10 compile et lance des
  `quick_check` ~30-40 min : mesures au repos de J7c.4 seulement après accord avec elle.
- (J7c.4, mycrusader-56) **Boucle 1 faite** (~9 min, compilé sans avertissement, rien de lancé) :
  `HOLStressTestSubsystem` : `S3_Melee` (par camp 500 Lanciers + 500 Hommes d'armes, disques de 10 m côte à côte à
  Y = ±18 m, 16 m entre les fronts) et `S4_Mixed` (334 / 333 / 333, Archers en disque à Y = ±44 m, 6 m derrière leur
  mêlée) ; une attaque-déplacement par groupe (type et camp) par le chemin normal du serveur, 30 m au-delà de la ligne
  médiane ; combat forcé à 1 ; mesure dès 100 coups de mêlée depuis la charge (`hol.StressTest.BattleContactHits`,
  sinon après 40 s), puis la durée donnée ; lignes en plus : délai de contact, debout par camp au début et à la fin,
  coups, morts, flèches (lâchées, arrivées, perdues, en vol moyenne et max). `chapter_measures.ps1` : liste par défaut
  S1:10, S2:10, S7:20, S3:15, S4:15. **Boucle 2** : vérification (une passe de S3 et S4 hors repos : contact, survivants
  à la fin, flèches en vol), mycrusader-10 prévenue.
- (J7c.4, mycrusader-56) **S3, 1re passe** (hors repos, CPU ~37 % avant) : marche comme voulu (contact 4,8 s après la
  charge ; 1000 contre 1000 au début de la mesure, 892 contre 912 à la fin, 8 830 coups, 196 morts en 15 s) mais
  **10,88 ms par pas** (max 24,5 ; seuil 3) et 84 images/s (seuil 144). Détail : **`HOLCombatProcessor` 5,64 ms**,
  **`MassStandingAvoidance` 8,02 ms un pas sur 6** (~1,34 ms en moyenne), séparation 1,44, ordres 0,84, plein régime
  0,58, évitement en marche 0,47 (un pas sur 6). 5 188 paires superposées au début de la mesure, 3 218 à la fin. S4 pas
  encore lancé (mycrusader-10 lance un `quick_check`, rien de neuf avant son signal). Analyse des causes lancée en
  lecture seule (sous-agent).
- (J7c.4, mycrusader-56) **Analyse de S3** (lecture seule, estimations, pas de mesures) : dans le combat, la
  **recherche de cible** (`HOLCombatProcessor.cpp:309-365`) fait ~75-85 % des 5,6 ms : ~500 chercheurs par pas
  (un pas sur 4), grille hachée de 400 cm sur 1 320 cm de rayon (~56 cases, ~600 unités vues, alliés compris), et la
  recherche complète même quand la cible actuelle est vivante et à portée (`ChooseTarget` la garde alors toujours :
  ~la moitié des recherches pour rien). Puis `TMap` entité → indice (~14 000 accès par pas), tri, journaux de mort.
  Séparation : corrections en place, propagation dans la foule (ennemis ne se poussent pas : superpositions qui
  restent). Pistes **au résultat identique au bit près**, par gain : (1) pas de recherche si la cible est vivante et
  à portée (~2-2,5 ms) ; (2) tableau plat au lieu du `TMap` (~0,3) ; (3) grille dense par camp, distances au carré en
  préfiltre (~1) ; (4) listes de voisins en parallèle puis décision en ordre (~0,5-0,8) ; (5) grille dense pour la
  séparation (~0,3-0,5) ; (6) tri de clés (~0,1) ; (7) ordres de marche du combat sans allocation (~0,2-0,4) ;
  (8) évitement à l'arrêt en parallèle (sous-classe D302) **seulement si** le code 5.8 (`C:\UE\UE_5.8_AS_3`, que le
  sous-agent n'a pas lu) ne lit pas les fantômes des voisins pendant qu'il écrit (~1). Total estimé : **~5,5-6,5 ms**,
  toujours au-dessus de 3 ms. Question posée à l'utilisateur.
- (J7c.4, mycrusader-56) **Choix de l'utilisateur** (recommandation, comme D302) : **les optimisations identiques
  d'abord**, la plus rentable en premier, mesure après chacune, le reste décidé sur les chiffres réels. Écartés pour
  l'instant : changements équivalents mais pas identiques (recherche moins fréquente, pas d'évitement à l'arrêt pour
  les unités qui frappent) ; seuil revu pour S3 / S4 (D218). **Boucle 2** lancée (écriture et compilation) : pistes
  (1), (2), (6) derrière `hol.Combat.FastPaths` (0 = anciens chemins, pour comparer dans une même compilation) et
  ligne « Battle checksum » (empreinte des unités vivantes à la fin de la mesure) au rapport de S3 / S4.
- (J7c.4, mycrusader-56) **Boucle 2 faite** (~5 min, compilé sans avertissement, rien lancé) : `hol.Combat.FastPaths`
  (défaut 1) ; (6) rien si déjà trié, sinon tri de clés (UnitId << 32 | indice) ; (2) tableau plat par indice
  d'entité (numéro de série + tampon du pas, jamais vidé), `LastAttacker` cherché une fois par unité ; (1) raccourci si
  cible vivante, d'un autre propriétaire, `SearchRadius > 0` et à portée (`ChooseTarget` la rend alors toujours : retour
  immédiat `bInRange && bIsCurrentTarget`, aucun effet de bord dans le code sauté). « Battle checksum » : FNV-1a 64 des
  unités vivantes triées (UnitId, position au cm, PV, cible). **Boucle 3** (vérification) après le signal de
  mycrusader-10 : S3 deux fois en `FastPaths 0` (le test est-il déterministe ? fin de mesure au même pas ?), puis en
  `FastPaths 1`, empreintes comparées, gain mesuré ; puis S4.
- (J7c.4, mycrusader-56) Boucle 3 commencée : `s3_f0a` (`FastPaths 0`) : 12,39 ms (combat 6,48 ; machine chargée,
  un ProjectShiva « TestRunner » d'une session playable tourne), **mêmes chiffres exacts que la 1re passe** d'avant les
  optimisations (892 contre 912, 8 830 coups, 196 morts) : S3 paraît déterministe ; empreinte **6edfa48d73507399**
  (1 804 unités). Les 4 passes suivantes refusées (`stress_ab` : BUSY, jeu de la copie principale lancé par
  mycrusader-10 juste après son « go ») ; attente de son nouveau signal.
- (J7c.4, mycrusader-56) **Boucle 3 faite** (machine très chargée : CPU 37-73 % avant les passes, compilations
  d'autres sessions dans `HeroesOfLiberty-J5c` et la copie principale, ProjectShiva « TestRunner ») : **identité prouvée**,
  S3 : empreinte 6edfa48d73507399 sur les 4 passes (`FastPaths` 0, 1, 0, 1), mêmes coups et morts ; S4 : empreinte
  caef2615dcc530e2 dans les deux modes, mêmes morts au même pas. **Combat de S3 : 6,5-7,5 → 3,0 ms** (pas entier
  12,4 / 14,3 → 9,7 / 9,4 ms, hors repos). Reste dans S3 : évitement à l'arrêt 9-10 ms un pas sur 6 (~1,6 ms en
  moyenne), séparation ~1,8, ordres ~0,9. **S4 ne mesure pas encore ce qui est voulu** : 37 flèches lâchées en 15 s,
  1 en vol en moyenne (9 au plus) au lieu de ~600 : les Archers (à 34 m de la mêlée, rassemblement « gathering first »
  puis 61 m de marche vers 30 m au-delà de la ligne médiane) n'arrivent pas à portée dans la fenêtre ; ordres 2,5 ms
  dans S4 (marches en bloc de 333). À faire : S4 corrigé (Archers à portée vite, mesure quand les flèches volent),
  puis pistes (3), (4), (5), (7), (8) (code 5.8 de l'évitement à l'arrêt à lire dans `C:\UE\UE_5.8_AS_3`).
- (J7c.4, mycrusader-56) Boucle 4 lancée en deux sous-agents : correction de S4 (Archers à portée vite, mesure quand
  les flèches volent) et lecture du code 5.8 de l'évitement à l'arrêt.
- (J7c.4, mycrusader-56) **Évitement à l'arrêt lu** (`MassAvoidanceProcessors.cpp` 5.8, 1108-1410) : chaque unité
  écrit son propre fantôme et **lit le fantôme de ses 6 plus proches voisins** (1312, 1322, 1327), écrit dans la même
  passe (ordre Gauss-Seidel) : **pas de parallèle façon D302 au bit près** (course et dépendance à l'ordre) ; double
  tampon = Jacobi, pas identique. **Voie identique** : sous-classe en deux phases, (A) en parallèle, lecture seule :
  recherche des voisins dans la grille (`FindCloseObstacles` recopié tel quel, plafond 24, tri instable gardé),
  filtre, tri, les 6 plus proches et leurs données qui ne changent pas dans la passe ; (B) en série, dans le même
  ordre : le calcul qui lit les fantômes en direct (1219-1235, 1293-1406). Légal sans toucher au moteur (classe
  `MinimalAPI`, requête recopiée, `DefaultGame.ini:46`, l'étalement un pas sur 6 suit par `IsChildOf`, noms d'ordre de
  `HOLAvoidanceHoldProcessors.cpp:87, 122`). Gain estimé : 9-10 → ~1,5-2,3 ms sur le pas concerné (~1,2 ms en
  moyenne). Séparation : grille dense à l'ordre de visite identique possible (indices décroissants par case, ~0,3-0,6
  sur 1,8 ms), données chaudes regroupées par unité en plus ; pas de parallèle (corrections en place).
- (J7c.4, mycrusader-56) **S4 corrigé** (compilé sans avertissement, une passe) : Archers en 4 disques de 5 m, 3 m
  derrière leur mêlée, une attaque-déplacement par disque vers la ligne médiane ; mesure quand 100 coups de mêlée **et**
  100 flèches lâchées dans la dernière seconde (`hol.StressTest.BattleContactArrows`), repli à 40 s ; S3 inchangé par
  construction (chemins propres à S4), empreinte de S3 **à reconfirmer**. Passe : mesure 21,7 s après la charge
  (rassemblement puis marche des Archers, mêlée qui les gêne), 950 contre 945 au début, 850 contre 840 à la fin ;
  2 053 flèches lâchées (137/s), 1 414 arrivées, 588 perdues (29 %, cause à voir), **75 en vol en moyenne, 203 au
  plus** ; 7,62 ms par pas (machine chargée) : évitement à l'arrêt 5,46 (un pas sur 6), combat 1,93, ordres 1,41,
  séparation 1,34, évitement en marche 1,08 (un pas sur 6). **Plafond de la table** : 666 Archers / 1,625 s ≈ 410
  flèches/s × 0,6 s de vol ≈ **246 en vol au plus** : les « ~600 » du plan étaient une estimation, impossible sans
  changer cadence, portée ou nombre ; S4 garde la composition choisie. HeroesOfLiberty-Elrond libérée pour le renommage en
  `HeroesOfLiberty-Elrond` (mycrusader-b0, demande de l'utilisateur).
- (J7c.4, mycrusader-56) **Renommage fait** : la copie de travail est désormais **`C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`**
  (branche `j7c-shot` et diff non commité intacts, `IN_USE.txt` à mon nom) ; recompilée de zéro (195 s, OK).
  Attention à la fusion : la branche `combat-dying-off` (mycrusader-4c, `HeroesOfLiberty-J5c`) touche aussi
  `HOLCombatProcessor.*`. **Boucle 5** lancée (écriture et compilation) : évitement à l'arrêt en trois temps
  (collecte en ordre, voisins en parallèle en lecture seule, calcul en série dans le même ordre), derrière
  `hol.Simulation.ParallelStandingAvoidance`.
- (J7c.4, mycrusader-56) **Boucle 5 faite** (~10 min, compilé sans avertissement) : `UHOLParallelStandingAvoidanceProcessor`
  (`HOLParallelMassProcessors.*`) : (A) collecte en ordre (MAP:1149-1217), (B) `ParallelFor` en lecture seule
  (MAP:1237-1277, 1294-1316 : voisins, tri, les 6 plus proches, pointeurs de leurs fantômes), (C) en série dans le même
  ordre (MAP:1165-1169, 1219-1235, 1279, 1293-1406 : fantômes des voisins lus en direct, écriture du sien) ;
  `FindCloseObstacles` recopié tel quel ; `DefaultGame.ini` (processeur remplacé, étalement un pas sur 6 gardé par
  `IsChildOf`), `HOLAvoidanceHoldProcessors.cpp` (noms d'ordre). **Vérifié** (machine chargée) : S3 en
  `ParallelStandingAvoidance` 1 / 0 et `FastPaths` 1 / 0 : **empreinte 6edfa48d73507399 dans les 3 passes**, mêmes
  coups et morts ; évitement à l'arrêt **10,2 → 2,4 ms** sur son pas (~-1,3 ms en moyenne) ; pas de S3 **14,2 (rien
  d'optimisé) → 8,6 ms** (tout optimisé), même machine chargée. Restent : combat 3,15, séparation 1,78, ordres 0,95.
  **Boucle 6** lancée (écriture et compilation) : combat, pistes (3) grille dense par camp et (4) voisins en parallèle.
- (J7c.4, mycrusader-56) **Boucle 6 faite** (~10 min, compilé) : (3) `BuildDenseGrid` (mêmes cases de 400 cm, tri par
  comptage par camp et case, repli sur la grille hachée si la boîte est trop grande), `GatherNearby` (préfiltre au carré
  puis test exact), tampon « vu » au lieu de `Contains` ; (4) collecte en parallèle pour les chercheurs prévus
  (`AheadLists`), repli en série sinon. **Vérification** : S3 identique (6edfa48d73507399), **combat 7,89 → 2,15 ms**
  (pas 13,6 → 7,2 ms, machine chargée) ; **S4 diverge** : `FastPaths` 1 = c91c8e4b4aeb8e73 (6 911 coups, 220 morts)
  contre 0 = 235652bf6163a5c0 (6 985, 205, comme le 1er tour) : (3) ou (4) change un cas propre à S4 (tireurs : portée
  1 500 cm, rayon 52, pas de `MaxAttackers`). **Boucle 7** lancée : trouver la cause et corriger (S4 et S3 identiques).
- (J7c.4, mycrusader-56) **Boucle 7 faite** (~9 min, aucun changement gardé) : **pas de défaut dans le 2e tour** ;
  **S4 n'est pas déterministe** : avec `FastPaths 0` et les mêmes réglages, 4 passes = 4 empreintes (235652bf…,
  10930bfb…, 97675f36…, 02a31de9… ; deux passes côte à côte divergent au pas 306 ; contact 20,9 à 24,6 s après la
  charge). S3 a donné 6edfa48d73507399 sur 11 passes sous des charges très différentes : preuve valable pour S3.
  Relecture du 2e tour sans écart (mêmes cases, test exact, `MaxRadius` sur toutes les unités, listes d'avance
  dépendant du seul état de début de pas, `ChooseTarget` indépendant de l'ordre). **Piste de l'aléa** (non prouvée) :
  l'évitement en marche de Mass (`MassAvoidanceProcessors.cpp:328, 432`) dose le départ d'une marche par
  `World->GetTimeSeconds()` (temps réel des images, pas le pas fixe) contre l'heure de début d'action posée par
  `CreateNewAction(..., World)` (`StopWalk` du combat, ordres) ; S4 a bien plus de marches (Archers, reprises de
  marche du combat). Question posée à l'utilisateur.
- (J7c.4, mycrusader-56) **Choix de l'utilisateur** (recommandation) : **corriger l'aléa maintenant** (simulation qui
  dépend de la charge de la machine ; garder la preuve par empreinte pour S4 ; le mouvement peut changer un peu, à
  revoir à l'essai). Écarté : le noter pour plus tard. **Boucle 8** lancée : recenser les entrées de temps réel dans
  la simulation, les rendre dépendantes du pas fixe sans toucher au moteur, S4 deux fois (même empreinte attendue).
- (J7c.4, mycrusader-56) **Boucle 8 faite** (~12 min) : **la piste du temps réel était fausse** (pas fixe passé à
  tous les processeurs ; la ligne d'évitement qui lit l'heure du monde est sous `GetPreviousAction() != Move`, jamais
  vrai en 5.8 : `PreviousAction` n'est jamais écrit). Ajout d'une sonde `hol.Simulation.StepHashes` (1 : empreinte par
  pas, 2 : par processeur, 3 : vidage des pas 0 et 1). S4 : 3 passes sur 4 identiques (235652bf…), l'aléa est
  intermittent ; l'écart naît dans le pilotage (`HOLParallelSteerToMoveTargetProcessor`), sur un état non couvert par
  la sonde. **Cause trouvée** (lecture du moteur) : `MassSteeringProcessors.cpp:272, 304` tire le délai de rechoix de
  cible à l'arrêt par **`FMath::RandRange`, le générateur global** (partagé par tout le jeu, appelé dans un ordre qui
  dépend des fils depuis D302) ; le moteur a un tirage par unité trois lignes plus haut (`:259`,
  `UE::RandomSequence::FRand(Entity.Index)`). **Boucle 9** lancée : pilotage recopié dans la sous-classe D302 avec un
  tirage déterministe par unité et par pas (même plage), autres tirages globaux recherchés, S4 ×3 (dont 2 en
  parallèle) et S3 ×2.
- (J7c.4, mycrusader-56) **Boucle 9 faite** (~9 min, compilé sans avertissement) : `UHOLParallelSteerToMoveTargetProcessor`
  porte une copie du pilotage du moteur (`MassSteeringProcessors.cpp` 5.8, 21-52, 66-87, 93-433 ; journal visuel
  retiré), toujours en parallèle par morceau, les deux `FMath::RandRange` (272, 304) remplacés par
  `DeterministicRandRange(UnitId, pas, site d'appel)` (splitmix64 puis `UE::RandomSequence::FRandRange`, même plage) ;
  `hol.Simulation.DeterministicSteering` (0 = fonction du moteur). Aucun autre tirage global dans le pipeline ni dans
  le projet (seulement des `FRandomStream` à graine). **S4 ×3 (dont 2 en parallèle) : 235652bf6163a5c0 ; S3 ×2 (en
  parallèle) : 6edfa48d73507399** : stables, et **les mêmes qu'avant** (le tirage ne pesait que quand l'ordre des fils
  tombait mal) : mouvement inchangé. Les optimisations du combat (1er et 2e tours) sont donc aussi prouvées sur S4
  (`FastPaths` 0 = 1 = 235652bf…). Suite : `-Full` dans Elrond, puis commit sur `j7c-shot`, puis mesure au repos.
- (J7c.4, mycrusader-56) **`-Full` PASS** (630 s, 87/87, mycrusader-10 en pause pendant ce temps). **Commit a6bef71**
  sur `j7c-shot` (S3, S4, optimisations identiques, tirage déterministe du pilotage, sonde `StepHashes`). Pas encore
  dans `main`. Reste de J7c.4 : mesure au repos (S3 face aux 3 ms), puis décider du reste sur les chiffres réels.
- (J7c.4, mycrusader-56) Choix de l'utilisateur : S3 / S4 au repos tout de suite ; pause obtenue de mycrusader-e1, pas
  de réponse de mycrusader-10 ; deux ProjectShiva encore ouverts (éditeur `Playable-mercy`, banc `Playable-genji`
  relancé), CPU 43-64 % : machine pas au repos. **Séance arrêtée ici** (« on continuera tout ça dans une prochaine
  session »), pauses annulées.
  **Reprise exacte** :
  - **État** : `HeroesOfLiberty-Elrond` (ex-`MyCrusader-A`), branche **`j7c-shot` à a6bef71**, rien de non commité (sauf
    `IN_USE.txt`). `main` à db44ae1 (J7c.1 à J7c.3) : **a6bef71 pas encore dans `main`** ; à l'avance de `main`,
    conflit possible avec `combat-dying-off` (mycrusader-4c, `HOLCombatProcessor.*`) et avec le retour 1 non commité
    de mycrusader-10 dans la copie principale ; demander l'accord des sessions actives.
  - **Fait dans J7c.4** : S3 (1000 contre 1000 en mêlée) et S4 (tiers, Archers derrière leur mêlée, mesure quand les
    flèches volent ; plafond ~246 en vol avec la table) dans `chapter_measures.ps1` ; empreinte « Battle checksum » ;
    optimisations identiques au bit près (combat : `hol.Combat.FastPaths` ; évitement à l'arrêt en trois temps :
    `hol.Simulation.ParallelStandingAvoidance`) ; tirage déterministe du pilotage (`hol.Simulation.DeterministicSteering`) ;
    sonde `hol.Simulation.StepHashes`. Références : **S3 6edfa48d73507399, S4 235652bf6163a5c0**. Chiffres hors repos
    (machine chargée) : S3 ~7,2 ms par pas (combat 2,15, évitement à l'arrêt ~1,8 sur son pas, séparation 1,7,
    ordres 0,95), S4 ~5,8 ms.
  - **À faire, dans l'ordre** : (1) **mesure au repos de S3 et S4** (deux passes chacun, `stress_ab.ps1` ou
    `chapter_measures.ps1 -Tests S3:15,S4:15 -NoS5`), après avoir obtenu le calme : pause des sessions MyCrusader,
    **ProjectShiva fermés par l'utilisateur** (éditeurs et bancs `TestRunner` des sessions playable), CPU vérifié ;
    (2) décider du reste sur ces chiffres (pistes identiques restantes : séparation en grille dense ~0,3-0,6 ms,
    ordres de marche du combat sans allocation ; sinon question à l'utilisateur : écarts acceptés ou seuil de S3 / S4) ;
    (3) mesures complètes de fin de chapitre (S1, S2 au seuil, S3, S4, S5 en combat avec le coût des PV exacts, S7) ;
    (4) avance de `main` ; (5) J7c.5 (clôture de J7 au GDD : D327, D328, J7-Q5 révisée, J7-Q1 à Q6, et les décisions
    de J7c.4 : S4 en tiers, optimisations identiques, tirage déterministe).
- (J7c.4, mycrusader-56) **Reprise dans la même séance** (proposition de l'utilisateur) : la **mesure au repos de S3 /
  S4 se fera ce soir**, quand tout sera fermé (ligne ajoutée sous J7c.4 dans `EtapeDefini.md`) ; on continue d'ici
  là. Ordre : **S5 en combat** d'abord (manque aux mesures de ce soir : `net_match.ps1 -Units` ne lance que S2 sur le
  serveur ; S5 en combat = S3 sur le serveur, pire cas des PV avec 589 coups/s, les flèches de S4 étant déduites sans
  envoi), puis la grille dense de la séparation. mycrusader-10 lance `-Full` dans la copie principale (port du
  serveur) : rien de lancé avant son signal.
- (J7c.4, mycrusader-56) **S5 en combat écrit** (~6 min, scripts seuls, rien lancé) : `net_match.ps1 -Test S2|S3|S4`
  (défaut S2 : S5 inchangé ; S3/S4 : 20 s de mesure par défaut, 40 s de plus d'attente) ; `s5_report.py` : fenêtre de
  S3 / S4 dès « measure begins », lignes « Combat: » (coups, morts, envois d'activité par seconde et par coup ;
  `STATE_BITS` 60 → 61) ; `chapter_measures.ps1` : « S5 combat » (`-Test S3`) après le S5 habituel, sa ligne au
  tableau. Les envois de PV sont mêlés aux envois d'« activité » (majorant seulement). **Boucle** lancée (écriture et
  compilation) : compteur des envois dus aux seuls PV dans le rapport « Net (server) » (rien ne change dans ce qui est
  envoyé) et grille dense de la séparation au même ordre de visite (`hol.Simulation.FastSeparation`).
- (J7c.4, mycrusader-56) **Boucle faite** (~5 min, compilé sans avertissement, rien lancé) : (A) `HealthSends` dans
  `FHOLUnitReplicationStats` : un envoi d'« activité » qui n'aurait pas eu lieu avec les PV déjà envoyés (état refait
  avec les PV envoyés, `GetSendReason` = None) ; rapport « Net (server) » : `(birth N, activity N, error N, refresh N;
  health N)` ; `s5_report.py` : envois de PV par seconde, par coup, et leur débit par client ; `chapter_measures.ps1`
  les reprend. (B) séparation : grille dense par tri par comptage (chaque case lue en indices décroissants, comme les
  chaînes hachées), repli sur le hachage au-delà de 2^20 cases, corps de la paire inchangé (lambda commune).
  Vérification à faire (S3 6edfa48d…, S4 235652bf… inchangés ; gain de la séparation) après le signal de mycrusader-10.
- (J7c.4, mycrusader-56) Séparation **vérifiée** : empreintes inchangées (S3, S4), séparation 1,62 → 1,38 ms (S3),
  1,24 → 1,04 (S4). Commit **69e63d6**. `main` avait avancé (d3f1408 retardataire de mycrusader-10, terminée ;
  2ac3092 `combat-dying-off`) : **fusion 7c7174c** dans `j7c-shot`, conflits résolus (combat : fin des mourantes combat
  coupé de `main` avant le tri rapide ; séparation : corps commun `Undo` avec la règle `CanPassThrough` de `main` ;
  `EtapeDefini.md`). Tous anciens chemins contre tous nouveaux : **S3 6edfa48d73507399** dans les deux (pas 12,68 →
  5,79 ms, machine chargée) ; **S4 9598a723d88909af** dans les deux (**nouvelle référence** : les marches en blocs du
  retardataire font tirer bien plus les Archers, 5 309 flèches en 15 s, 354/s). **1er S5 en combat** lancé
  (`net_match -Stage -Units 2000 -Test S3`, clients en fenêtres sur l'écran 2 comme d'habitude).
- (J7c.4, mycrusader-56) **1er S5 en combat : FAIL** (machine chargée ; mesure de 20 s, 3 rapports, début 5,2 s après
  la charge) : **87 466 B/s par client** (seuil 32 000 ; S5 en marche ~24-25 000), **écart affiché 105,5 cm** (seuil
  25), corrections ~112 cm, ~1 200-1 400 sauts par client, 1 % bas 6 images/s ; serveur 8,14 envois par unité et par
  seconde, simulation 14,1 ms par pas sur le serveur dédié. Combat : 12 124 coups (606/s), 310 morts. **Envois
  d'« activité » : 5 543/s = 9,15 par coup** (~47 800 B/s par client) ; **envois dus aux seuls PV : 515/s = 0,85 par
  coup (~4 400 B/s)** : les PV ne sont pas le gros poste. Analyse lancée (lecture seule) : ce qui déclenche une
  « activité », pourquoi 9 par coup, cause de l'écart affiché, 14 ms du serveur, options.
- (J7c.4, mycrusader-56) **Analyse du S5 en combat** (lecture du code et des journaux) : (1) envois par raison dans la
  fenêtre : **« error » 9 825/s** (5,4 par unité et par seconde), « activity » 5 543/s (3,0), refresh 103, PV seuls
  515 ; le client lit ~8 000 états/s sur ~15 470 voulus. (2) **Vitesses bruitées** : `hol.Net.StepVelocity 1` (D294)
  envoie le déplacement du dernier pas, poussées de séparation comprises : un combattant debout dans la presse a
  40-100 cm/s au hasard, le client l'extrapole (jusqu'à 3 s), le serveur renvoie dès 35 cm. (3) **Activité** :
  surtout le **bord de portée** (`IsInRange` sans hystérésis, poussé de part et d'autre : Attacking → Idle → Moving →
  Attacking, 2-3 envois par aller-retour), marches de combat bloquées, changements de cible (mineurs). (4) **Écart de
  105 cm** : lien saturé (plafond du moteur `MaxClientRate` 100 000 B/s, ~3,3 Ko par tick à 30 Hz) + vitesses
  bruitées livrées en retard ; le lissage D271 n'est pas en cause. (5) **Serveur à 14 ms** : sur un serveur dédié,
  `FApp::ShouldUseThreadingForPerformance()` est faux, donc **tous les `ParallelFor` tournent sur un fil** (et 4
  ouvriers au plus) ; remède : **`-useperfthreads`** au lancement du serveur (jeu inchangé, à vérifier par
  l'empreinte). Options réseau seules (empreinte inchangée) : (a) vitesse nulle pour les unités qui ne marchent pas
  (~-35 à -45 Ko/s, mieux à l'œil) ; (b) tenue minimale de 0,2 s des changements d'activité (~-20 Ko/s, geste jusqu'à
  0,2 s en retard) ; (c) « engagé » = Attacking sur le fil tant qu'une cible est tenue (~-5 à -10) ; (d) PV sur
  minuterie (~-4) ; (e) codage « immobile » 1 bit ; (f) priorité par client. (a)+(b)+(d) ≈ 40-60 Ko/s : sous 32 il
  faut aussi (c) ou (e). Hors réseau (change le jeu) : hystérésis de portée. Question posée à l'utilisateur.
- (J7c.4, mycrusader-56) **Choix de l'utilisateur** (recommandation) : **vitesse nulle d'abord** (une unité qui ne
  marche pas est envoyée avec une vitesse nulle ; réseau seul, jeu identique) **+ `-useperfthreads`** au serveur
  dédié, puis S5 en combat remesuré et le reste décidé sur les chiffres. Écartés pour l'instant : tout le lot réseau
  d'un coup (tenue 0,2 s, « engagé », PV sur minuterie) ; seuil revu pour la mêlée (D274). Boucle lancée (écriture et
  compilation) ; mesure après le `-Full` de mycrusader-85 (port du serveur).
- (J7c.4, mycrusader-56) À savoir (mycrusader-85) : depuis d3f1408 (retardataire), la séquence solo de `-Full` prend
  ~660 s et `quick_check` tue le jeu à 660 s (`GameLimitSeconds`) : à passer à **780** dans `j7c-shot` aussi (même
  valeur que sa branche) avant le prochain `-Full`. Mieux (proposé par elle) : **refusionner `main`** dans `j7c-shot`
  quand elle y aura fusionné sa correction, au lieu de l'écrire soi-même.
- (J7c.4, mycrusader-56) **Vitesse nulle écrite** (~17 min, compilé sans avertissement) : `HOLUnitReplication::SendsVelocity`
  (= `IsWalkingActivity` : Moving et GatherTrip gardent la vitesse du pas, D294 ; Idle, IdleFacingFront, Gathering,
  Building, Attacking, Dying envoyés immobiles), `hol.Net.StillZeroVelocity` (0 = comme avant) ; une unité qui frappe
  ne marche jamais au serveur (`StopWalk` avant `bAttacking`) ; l'erreur se mesure contre l'état envoyé, donc cohérente
  sans autre code. Client : orientation et animations ne dépendent pas de la vitesse sauf Idle (toujours au repos
  désormais ; à voir à l'essai : unité inactive poussée vite). Test `Net.SendRule` étendu (combattant poussé 40-100 cm/s :
  moins d'envois, erreur sous le seuil). **`-useperfthreads`** dans `net_match.ps1` et le serveur de `quick_check.ps1`
  (GDD.md:1991, D275 GDD.md:2454, ImplementationInProgress.md:783 décrivent le lancement : à compléter à J7c.5).
- (J7c.4, mycrusader-56) Tests 87/87 PASS (67 s). **2e S5 en combat : FAIL mais mieux** : **70 333 B/s** par client
  (87 466), **écart affiché 71,8 cm** (105,5), sauts 420 / 255 (1 371 / 1 116) ; mais envois « error » toujours ~10 000/s
  (48-70 000 par rapport de 5 s), « activity » 6 989/s (en hausse), PV seuls 499/s ; serveur toujours 14,3 ms par pas
  malgré `-useperfthreads` (bien sur sa ligne de commande ; évitement à l'arrêt 14,9 ms sur son pas : 4 ouvriers au plus
  sur un serveur dédié, machine chargée). Commit **e7d0caf** (vitesse nulle, `-useperfthreads`). `main` refusionné
  (ea84b7b : limite 780 s de mycrusader-85, compte d'attaquants à la mort) : **d1f4f5f**, conflit du combat résolu en
  gardant les deux. Empreintes : S3 6edfa48d… identique ; **S4 de nouveau instable** (nouveaux chemins : 9598a723…,
  9598a723…, 3bcd2baa… ; anciens : 9598a723…, bb0706a6… ; `FastPaths 0` seul : cbaac7e6…) : une autre source d'aléa,
  propre à S4. Boucle lancée : sonde `StepHashes` étendue (combat, fantôme, pilotage, ordre, flèches), paires de S4
  jusqu'à divergence, processeur et champ fautifs.
- (J7c.4, mycrusader-56) **Sonde étendue** (non commitée : `HashUnits` par parties combat / fantôme / pilotage /
  ordre / identités d'entités, `HashArrows`) : une paire de S4 en parallèle diverge (f053c377… contre c6eb1144…) ;
  tout identique jusqu'au pas 307 ; **au pas 308, tout est identique en entrée de `HOLUnitSeparationProcessor` et les
  positions diffèrent à sa sortie**. Cause probable : la séparation **collecte les unités dans l'ordre des morceaux de
  Mass** (variable d'une passe à l'autre : changements d'archétype vidés dans un ordre qui dépend des fils) et tout
  ce qui suit (cases, `ToLook`, corrections en place) dépend de cet ordre ; le combat, lui, trie par `UnitId`.
  **Boucle lancée** : séparation triée par `UnitId` (et revue des autres processeurs dépendant de l'ordre :
  `HOLFootprintKeepOutProcessor`, ordres, maintien, plein régime…) ; nouvelles références d'empreinte attendues.
- (J7c.4, mycrusader-56) Boucle arrêtée à la demande de l'utilisateur (compile, **rien vérifié**) : séparation en ordre
  d'`UnitId` (clés triées, 9 tableaux permutés une fois, retour en écriture par `SortedIndexOf`) ; ordres : marcheurs
  coupés triés par (série d'ordre, indice d'entité) au lieu d'un tri stable sur la seule série, nouvelles demandes de
  chemin du pas triées par entité (budget de chemins servi dans l'ordre). Revue : `HOLFootprintKeepOut`, `HOLFullSpeed`,
  `HOLAvoidanceHold` indépendants de l'ordre ; **dépendants de l'ordre, non corrigés** : phase C de l'évitement à
  l'arrêt (fantômes des voisins lus en direct dans l'ordre des morceaux, voisins tirés de la grille d'obstacles du
  moteur remplie dans cet ordre et plafonnée : copie fidèle de Mass, la trier la séparerait de `ParallelMass 0`),
  `HOLBuildProcessor` et `HOLGatherProcessor` (places, stocks, dépôts pris dans l'ordre des morceaux ; pas dans S4).
  Commit **299958a** (avec la sonde étendue et `EtapeDefini.md` : S4 déterministe et S5 en combat encore ouverts).
- **Séance arrêtée ici (demande de l'utilisateur : « arrête-toi quand tu peux et commit ce que tu as, marque tout
  pour continuer plus tard dans une autre session »).**
  **Reprise exacte** :
  - **État** : `HeroesOfLiberty-Elrond` (ex-`MyCrusader-A`), branche **`j7c-shot` à 299958a**, rien de non commité (sauf
    `IN_USE.txt`, à mon nom, « en pause »). `main` à **ea84b7b** (fusionné dans `j7c-shot` en d1f4f5f) ; **aucun commit
    de J7c.4 dans `main`**. Dernier `-Full` PASS sur a6bef71 seulement (avant S5 en combat, vitesse nulle, fusions,
    tri) ; tests 87/87 PASS sur e7d0caf.
  - **Commits de J7c.4 sur `j7c-shot`** : a6bef71 (S3, S4, empreinte « Battle checksum », optimisations identiques
    du combat `hol.Combat.FastPaths` et de l'évitement à l'arrêt `hol.Simulation.ParallelStandingAvoidance`, tirage
    déterministe du pilotage `hol.Simulation.DeterministicSteering`, sonde `hol.Simulation.StepHashes`) ; 69e63d6
    (S5 en combat : `net_match.ps1 -Test S3`, `s5_report.py`, ligne de `chapter_measures.ps1` ; compteur des envois de
    PV seuls ; séparation en grille dense `hol.Simulation.FastSeparation`) ; 7c7174c et d1f4f5f (fusions de `main`) ;
    e7d0caf (vitesse nulle des unités qui ne marchent pas `hol.Net.StillZeroVelocity`, `-useperfthreads` au serveur
    dédié) ; **299958a (non vérifié)** (tri par `UnitId` de la séparation, deux ordres d'ordres, sonde étendue).
  - **Chiffres** (machine chargée) : S3 tous anciens chemins 12,7-14 ms par pas → tous nouveaux 5,8-7,4 ms (seuil 3) ;
    S4 ~5-6 ms. **S5 en combat** : 87 Ko/s → **70 Ko/s par client** (seuil 32), écart affiché 105 → **72 cm** (seuil
    25) ; envois « error » ~10 000/s inchangés par la vitesse nulle (à comprendre), « activity » ~7 000/s, PV seuls
    ~500/s (~4 Ko/s) ; serveur dédié 14 ms par pas même avec `-useperfthreads` (4 ouvriers au plus).
  - **À faire, dans l'ordre** :
    1. **Vérifier 299958a** : S4 ×4 (deux paires en parallèle) et S3 ×2 en parallèle → une seule empreinte chacun
       (nouvelles références ; S3 n'est plus 6edfa48d…) ; S3 avec `hol.Simulation.FastSeparation 0` = S3 par défaut ;
       et tous anciens chemins (`FastPaths 0`, `ParallelStandingAvoidance 0`, `FastSeparation 0`) = tous nouveaux. Si S4
       diverge encore : sonde `StepHashes 2` après l'évitement à l'arrêt et la grille d'obstacles (phase C dépendante
       de l'ordre des morceaux : la trier, quitte à ne plus coïncider avec `ParallelMass 0`, à demander à l'utilisateur
       si le résultat change).
    2. `quick_check -Full` (limite de 780 s déjà dans la branche), puis commit.
    3. **S5 en combat** : sonde des envois « error » par activité et par vitesse (pourquoi ~10 000/s restent), puis
       le reste du lot réseau (tenue minimale de 0,2 s des changements d'activité, « engagé » gardé tant qu'une cible est
       tenue, PV sur minuterie) ; sinon question à l'utilisateur (seuil de la mêlée, D274). Chaque S5 : `net_match.ps1
       -Stage -Units 2000 -Test S3` (serveur recuit si le code change ; clients en fenêtres sur l'écran 2 ; prévenir les
       sessions pour le port du serveur).
    4. **Mesures au repos, le soir** (ProjectShiva fermés par l'utilisateur, sessions en pause, CPU vérifié) :
       `chapter_measures.ps1` (S1, S2, S7, S3, S4, S5, S5 combat) ; décider du seuil de S3 / S4 sur ces chiffres.
    5. Avance de `main` (accord des sessions actives), puis **J7c.5** (clôture de J7 au GDD : D327, D328, J7-Q5 révisée,
       J7-Q1 à Q6, décisions de J7c.4 : S4 en tiers, optimisations identiques, tirage déterministe, vitesse nulle,
       `-useperfthreads`, ordre de visite par identifiant ; lancement du serveur dans GDD.md:1991, D275 GDD.md:2454,
       ImplementationInProgress.md:783).
  - **À savoir** : comparer deux passes = même empreinte « Battle checksum » à la fin de la mesure ; lancer les jeux
    directement (`stress_ab.ps1` refuse dès qu'une autre session compile) ; se coordonner avec les sessions actives
    (mycrusader-e1, mycrusader-85 dans `HeroesOfLiberty-Galadriel`) avant tout `-Full`, partie réseau ou mesure.
- (J7c.4, mycrusader-56) **Reprise** (2026-10-21) : point 1 lancé (sous-agent) : vérification de 299958a, S4 ×4 et
  S3 ×2 en parallèle, puis S3 `FastSeparation 0`, S3 et S4 tous anciens chemins ; une seule empreinte attendue par
  test. Sessions actives : mycrusader-4f (`HeroesOfLiberty-Galadriel`), mycrusader-01 (`HeroesOfLiberty-J5c`).
- (J7c.4, mycrusader-56) **299958a vérifié** (~10 min, binaire à jour, `-nullrhi`, 300 pas) : **S4 ×4 en parallèle :
  93587c1cc36b353c** (5 402 flèches lâchées, 3 393 arrivées) ; **S3 ×2 : 35c168b12173ffac** ; S3 `FastSeparation 0`
  = S3 ; tous anciens chemins (`FastPaths 0`, `ParallelStandingAvoidance 0`, `FastSeparation 0`) = tous nouveaux pour
  S3 et S4 (13,9-14,7 ms par pas contre 5,8-7,6, machine chargée, 4 jeux à la fois). **S4 enfin déterministe** ;
  **nouvelles références : S3 35c168b12173ffac, S4 93587c1cc36b353c** (le tri par `UnitId` change le mouvement de
  la séparation). Suite : `quick_check -Full` (sessions prévenues), puis commit.
- (J7c.4, mycrusader-56) **Analyse des envois « error » de S5 en combat** (lecture seule, ~10 min, `net_server.log`
  de 16:05 et ses NetSample, rejeu de la règle) : pas de défaut dans la règle (l'erreur repart de zéro à chaque envoi,
  la vitesse nulle est bien prédite, aucun budget ne recompte). Cause : **tremblement de la séparation dans la
  presse** : déplacement par pas p50 / p90 27 / 101 cm (pas 580-680), 17 / 66 cm (680-780), sens inversé d'un pas à
  l'autre dans 52-56 % des cas (unité 100 : +102 puis −75 cm) ; la séparation défait tout le chevauchement d'un coup
  sans plafond (`HOLSimulationProcessors.cpp:712-753`), un marcheur pressé contre un ennemi immobile le reprend chaque
  pas. Rejeu (vitesse nulle, 35 cm) : 9,1 → 3,7 envois par unité et par seconde selon la fenêtre, collé à la mesure
  (« activity » + « error » 9,7 → 5,9). Les « Moving » de la presse envoient 500-2 000 cm/s de bruit (10-12 envois
  par seconde). Biais : la règle tourne à 20 Hz, le réplicateur part à 10 Hz (envois comptés deux fois) ; « états sur
  le fil » (30 072/s pour 72 Ko/s) gonflé. Remèdes : (1) supprimer le tremblement (combattant bloqué au contact d'un
  ennemi immobile arrêté, ou correction plafonnée / amortie ; change le jeu) ; (2) vitesse nulle aussi pour la marche
  de combat ou vitesse lissée (réseau seul) ; (3) seuil 50 / 70 cm pour les immobiles (−27 / −55 % d'« error »,
  écart affiché en hausse) ; (4) position lissée pour les immobiles ; (5) tenue 0,2 s et « engagé » ; (6) PV sur
  minuterie (petit) ; (7) règle aux seuls pas du réplicateur. Sous 32 Ko/s : (1) ou (2), plus (3) ou (4). Sonde
  proposée : envois par activité et par raison, histogramme des pas depuis l'envoi et du déplacement du pas.
  Question posée à l'utilisateur.
- (J7c.4, mycrusader-56) **Choix de l'utilisateur** (recommandation) : **corriger le tremblement à la source**
  (combattant au contact d'un ennemi immobile arrêté au lieu de marcher dedans ; correction de la séparation
  plafonnée ou amortie par pas), précédé de la sonde des envois par activité pour mesurer le gain ; le mouvement
  change : nouvelles empreintes, S1 / S2 / S3 à revérifier. Après le `-Full` en cours.
- (J7c.4, mycrusader-56) **`-Full` FAIL** sur 299958a (799 s, machine saturée : processeur à 100 %, ProjectShiva
  compile, éditeurs ouverts) : séquence solo coupée par la limite de 780 s (lancement fini à 785 s), d'où les KO de
  ses dernières parties (flèches du combat, file d'ordres : pas de rapport) ; **caméra 8 joueurs FAIL** (« launched
  from the screen » KO, aucun rapport) ; tout le reste PASS (tests 87/87, menu, production, déplacement, récolte,
  construction, animations, formation, réseau 2 clients). Journaux gardés dans le scratchpad (`full1_logs`). Relance
  de `-Parts camera,combat` seules.
- (J7c.4, mycrusader-56) Relance `-Parts camera,combat` : **PASS** (208 s ; caméra 8 joueurs PASS : son échec venait
  de la machine saturée). Avec le reste du `-Full` (PASS), 299958a est vérifié. Commit **0fbafb4** sur `j7c-shot`
  (`EtapeDefini.md` : S4 déterministe retiré, tremblement de la séparation ajouté).
- (J7c.4, mycrusader-56) **Boucle 1 du tremblement** lancée : sonde réseau (envois par activité et par raison,
  histogrammes pas depuis l'envoi / déplacement du pas) et mesure solo du tremblement dans le rapport de S3
  (déplacement par pas des unités au combat, part des inversions de sens) ; empreinte S3 35c168b12173ffac inchangée
  attendue ; chiffres de départ.
- (J7c.4, mycrusader-56) **Boucle 1 faite** (~11 min, compilé sans avertissement, non commité) : sonde réseau dans
  `HOLUnitReplicationSubsystem` (unités et envois par activité et par raison, écart en pas et déplacement du pas des
  envois « error », en fin de ligne du rapport ; pas encore lancée en S5) ; sonde solo `SampleJitter` / `LogJitter`
  dans `HOLStressTestSubsystem` (ligne « Jitter », lecture seule ; classes attacking = `bAttacking`, approach =
  `bApproaching` ou une cible, walking). Empreintes inchangées (S3 35c168b…, S4 93587c1…). **S3** : déplacement par
  pas p50 / p90 / max 11,4 / 67,6 / 400 cm, inversions 49 % ; attacking 9,0 / 63 / 361, **inversions 54 %** ;
  approach 15,6 / 82 / 400, 42 %. **S4** : attacking 1,0 / 36 / 361, 52 % ; approach 12 / 51 / 394. Pointes de
  360-400 cm en un pas suspectes. Boucle 2 lancée : mécanisme exact, correction derrière une variable, S3 avec et sans.
- (J7c.4, mycrusader-56) **Boucle 2** (~12 min, diagnostic seul : correction + compilation + 4 jeux ne tenaient pas ;
  leçon : séparer le code et la passe de mesures). **Mécanisme** (à la lecture) : une unité qui frappe est à l'arrêt
  (action Stand) ; l'évitement à l'arrêt (`HOLParallelStandingAvoidanceProcessor`, `HOLParallelMassProcessors.cpp:768-784`)
  tire son fantôme, donc elle, vers `MoveTarget.Center` (point de `StopWalk`) ; la séparation (`HOLSimulationProcessors.cpp:742-747`)
  la déplace sans toucher au centre ni au fantôme (alliés qui marchent derrière : part 0,5 ; contre un ennemi immobile
  : part 1) ; au pas suivant le rappel la ramène dedans : **rappel élastique**, d'où 54 % d'inversions. Approche : même
  conflit pilotage / séparation. **Pointes de 360-400 cm** non établies : corrections enchaînées sans plafond (probable),
  retour sur la navmesh, `HOLFootprintKeepOut`. **Plan** : `hol.Simulation.SeparationSettle` (1 par défaut) : pour une
  unité qui frappe, `MoveTarget.Center` et fantôme décalés de la correction de la séparation (elle reste où elle est
  mise) ; ensuite, si besoin, plafond par pas (`SeparationMaxStep`) et arrêt d'un marcheur au contact d'un ennemi
  immobile. Boucle 3 lancée : code + compilation + S3 avec / sans.
- (J7c.4, mycrusader-56) **Boucle 3** (~14 min, arrêtée) : code écrit, **non compilé** (Build.bat bloqué derrière
  l'UnrealBuildTool de mycrusader-4f dans `HeroesOfLiberty-Galadriel`, même moteur ; leçon : vérifier le verrou d'UBT
  avant de lancer une boucle de code). `HOLSimulationProcessors.cpp` : `hol.Simulation.SeparationSettle` (1 par
  défaut) ; requête de la séparation : `FMassMoveTargetFragment` en écriture, `FMassGhostLocationFragment` (écriture)
  et `FHOLCombatFragment` (lecture) optionnels ; `Settles[i] = !bWalking && !bGhost && bAttacking`, permuté par
  `UnitId` ; à l'écriture, centre et fantôme décalés de la correction (X, Y). Sonde des pointes : plus grande
  correction du pas (toutes / unités qui frappent), lignes « Jitter sep max step » dans `HOLStressTestSubsystem`.
  À vérifier : `SeparationSettle 0` = 35c168b… (les accès changés peuvent changer l'ordre des processeurs de Mass).
  Compilation relancée par la session principale (attend le verrou).
- (J7c.4, mycrusader-56) Boucle 3 compilée par la session principale (117 s, sans avertissement). S3 ×2 avec
  `SeparationSettle 1`, S3 avec 0, S4 avec 1 (en parallèle) : **la correction n'a aucun effet** : empreintes
  identiques avec et sans (S3 35c168b12173ffac, S4 93587c1cc36b353c), Jitter identique au cm près (attacking S3
  9,0 / 63 / 361 cm, 54 %). La sonde des pointes tranche : **elles viennent de la séparation** (plus grande
  correction du pas, unités qui frappent : p50 / p90 / max 256 / 306 / 361 cm en S3 ; toutes : 280 / 338 / 426) ; au
  début de la mesure 5 260 paires se chevauchent (écart le plus petit −120 cm : deux unités presque l'une sur l'autre).
  Boucle 4 lancée : pourquoi `Settles` ne s'applique pas.
- (J7c.4, mycrusader-56) **Boucle 4** (non commitée) : **le rappel par l'évitement à l'arrêt n'existe pas** :
  `UHOLFullSpeedProcessor` (`HOLSimulationProcessors.cpp` ~280-290) met à zéro la vitesse de toute unité hors action
  Move ; le décalage du centre et du fantôme n'était lu par rien (d'où l'empreinte identique). **Vraie source : la
  séparation seule** : un ami qui avance pousse la frappeuse de moitié, elle passe en file, reprend tout son
  chevauchement contre l'ennemi (part 1), à chaque pas, dans des sens variables. Essai : décalage retiré (requête
  revenue à l'identique), `Share = Settles[Other] ? 1 : GetOverlapShare(...)` (une frappeuse n'est jamais poussée).
  S3 : off = 35c168b… ; **on 774d35e0f94a42d6 : Jitter attacking 0 / 0 / 0**, approach 19 / 79 / 417 (39,6 %) ; mais
  **chevauchements à la fin 9 536 paires (3 211 avant)**, **347 morts (182)**, mêlée 2,2 s après la charge (4,7) :
  deux frappeuses amies qui se chevauchent ne se séparent plus jamais. Pas acceptable tel quel. Question posée à
  l'utilisateur (amortir, plafonner par pas, ou frappeuse non poussable par un marcheur ami seulement).
- (J7c.4, mycrusader-56) **Choix de l'utilisateur** (recommandation) : **plafonner la correction de la séparation par
  pas** (~30 cm, un pas de marche ; règles de partage inchangées). Boucle 5 lancée : essai « jamais poussée » retiré,
  `hol.Simulation.SeparationMaxStep` (cm, 0 = comme avant), S3 à 0 / 20 / 30 / 45 cm, S4 ensuite.
- (J7c.4, mycrusader-56) **Boucle 5** (~6 min, non commitée) : essai « jamais poussée » retiré ; `hol.Simulation.SeparationMaxStep`
  (cm, défaut 30, 0 = 35c168b… vérifié) appliqué à la correction totale après les 2 tours ; sonde « sep max step »
  réduite à toutes les unités. **Le plafond ne guérit pas** : S3 à 20 / 30 / 45 cm : frappeuses à exactement le
  plafond à chaque pas (p50 = p90 = max), inversions 48-50 % ; chevauchements à la fin 9 727 / 6 362 / 4 259 (3 211
  sans) ; morts 361 / 246 / 133 (182) ; séparation 3,9 / 3,2 / 2,6 ms (1,9) ; mêlée 2,4-2,9 s après la charge (4,7).
  S4 à 30 : a8526c89…, 232 morts (198), chevauchements fin 3 608 (2 723). Lecture : la correction est étalée, pas
  supprimée ; le chevauchement est recréé à chaque pas (les marcheurs de l'approche entrent dans la presse, puis
  ping-pong entre voisins des corrections en place). Question posée à l'utilisateur.
- (J7c.4, mycrusader-56) **Choix de l'utilisateur** (recommandation) : plafond retiré ; **mesurer d'abord la source**
  (sonde seule, jeu inchangé) : part des corrections de la séparation due à un marcheur qui avance dans la presse,
  part due au renvoi entre unités debout ; remède ensuite (arrêt du marcheur bloqué, ou séparation relaxée).
- (J7c.4, mycrusader-56) **Boucle 6** (~10 min, sonde seule, non commitée ; plafond retiré, empreintes inchangées
  35c168b… / 93587c1…) : sonde « Separation source » dans `HOLSimulationProcessors` (classes de paires, tour, file ou
  premier passage, chevauchement au début / à la fin de la séparation, déplacement depuis la fin précédente ;
  `SumOverlaps` en O(N²) pendant la fenêtre seulement : temps faussés). **S3 par pas moyen** : chevauchement 95 510 cm
  (3 793 paires) au début de la séparation, 77 014 cm à la fin ; la séparation déplace ~256 000 cm pour n'en retirer
  que 18 500 (**7 % d'efficacité**) ; entre deux séparations seuls les marcheurs bougent (11 163 cm). Par classe :
  debout–debout ami 41 %, debout–debout ennemi 27 % (tout sur celle qui regarde), marcheur–debout ami 47 141 cm
  (moitié chacun), marcheur–marcheur ami 22 098, marcheur–debout ennemi 12 797 ; **toutes les paires debout–debout
  viennent de la file**, ouverte seulement par la poussée d'un marcheur ami ; tour 2 ≈ tour 1 (pas de convergence).
  S4 de même en plus petit. **Lecture** : l'énergie entre par les marcheurs qui poussent leurs amis debout ; la poussée
  se propage, la frappeuse reprend tout contre l'ennemi et revient dans ses amis. Question posée à l'utilisateur.
- (J7c.4, mycrusader-56) **Choix de l'utilisateur** : le marcheur pousse toujours (« un marcheur pousse toujours mais
  on peut affiner ») ; deux affinages à essayer, chacun derrière sa variable, puis choix sur les chiffres : **poussée
  plus douce** (l'ami debout prend 25 % du chevauchement au lieu de 50 %) et **poussée transmise à l'ennemi** (une
  paire debout–debout ennemie se partage à moitié au lieu de tout sur celle qui regarde). Écarté : un seul rang poussé.
  Boucle 7 lancée : S3 sans / doux / transmis / les deux, sonde allumée.
- (J7c.4, mycrusader-56) **Boucle 7** (~8 min, non commitée) : `hol.Simulation.SeparationFriendPushShare` (0,5 par
  défaut) et `hol.Simulation.SeparationPushToEnemy` (0 par défaut) dans `Undo` (`HOLSimulationProcessors.cpp:483-489,
  759-771`) ; défauts = 35c168b… vérifié. **Aucun gain net** : S3 doux 0,25 : attacking 7,5 / 58 / 408, 53,6 % ;
  transmis : 15,9 / 66 / 279, 47,5 % (p50 en hausse) ; les deux : 14,5 / 51 / 313, 53,7 % ; efficacité de la
  séparation 6,9-7,6 % partout (7,2) ; 7-20 % de coups en moins ; chevauchements fin 3 263 / 3 521 / 3 860 (3 211).
  S4 doux : attacking p90 36 → 25,5 cm, 188 morts (198) ; transmis : 3,2 / 42 / 265. Ces affinages ne touchent pas
  la cause (presse surcomprimée : ~77 000 cm de chevauchement résiduel, corrections en place qui se renvoient).
  Bilan après 7 boucles : question posée à l'utilisateur.
- (J7c.4, mycrusader-56) **Choix de l'utilisateur** (recommandation) : **regarder d'abord à l'écran** ; s'il ne se
  voit pas, débit par le réseau seul ; s'il se voit, séparation relaxée. Scénario **`melee_press`** écrit (non
  commité, dans Elrond) : 200 Lanciers et Hommes d'armes de chaque côté en quatre paquets, à ~28 m, attaque-déplacement
  des deux armées vers l'arrière de l'autre (`hol.Units.Move ... a`, coordonnées du monde de Plain à deux joueurs) ;
  vérifié hors écran (`-RenderOffScreen` : `-nullrhi` bloque `waitui`) : prêt, mêlée ~7 s après la charge, ~65
  images/s. Essai de l'utilisateur lancé (`play.ps1 melee_press -Root ...HeroesOfLiberty-Elrond`).
- (J7c.4, mycrusader-56) **Essai `melee_press` de l'utilisateur** : « ils tremblent énormément mais c'est parce qu'ils
  essaient d'accéder à une cible qui est déjà full entourée alors qu'il y a littéralement des ennemis autour qui ne
  sont pas totalement entourés et que donc s'ils essayaient de taper eux à la place, il y aurait moins ce souci ».
  **La cause est le choix de la cible**, pas la règle de la séparation : un combattant garde une cible pleine et
  pousse vers elle. Boucle 8 lancée : lecture du choix de cible (places autour d'une cible, `AttackerCount`), puis
  préférence pour un ennemi qui a encore de la place, derrière une variable ; S3 avec / sans.
- (J7c.4, mycrusader-56) **Boucle 8** (~9 min, non commitée, compilé sans avertissement) : **diagnostic** : cible
  rechoisie un pas sur 4 (`SearchSteps`) ou à la mort, gardée sans recherche si à portée ; `ChooseTarget`
  (`HOLCombatRules.cpp:116`) trie : unité, à portée, m'attaque, « encombré », plus proche, Id ; « encombré » = hors
  portée et `AttackerCount` ≥ `MaxAttackers` 6 (attaquants à toute distance) : un ennemi du front collé aux siens est
  entouré avec 2-3 attaquants et n'est jamais plein ; si tout est encombré, le plus proche. Pas de règle de places
  de combat au GDD. Le clic droit (`bOrderedTarget`) ne passe jamais par `ChooseTarget`. (Le « slot 4095 » est le
  joueur de la cible dans la réplication.) **Changement** `hol.Combat.PreferFreeTarget` (1 par défaut) : un ennemi
  hors portée est **plein** quand 6 corps vivants (tous camps) ont leur bord à 50 cm ou moins du sien
  (`CountBodiesAround` sur la grille dense, une fois par pas à la demande) ; plein passe après tous ceux qui ont une
  place ; si tous pleins, la cible actuelle d'abord (pas d'oscillation). 0 = 35c168b… / 93587c1… vérifiés.
  **S3** a2a470378f3c128b : attacking 5,3 / 52 / 384 (9,0 / 63 / 361), 50,7 % ; approach 18 / 54 / 411, **26,5 %**
  (41,5) ; chevauchements fin 2 988 (3 211) ; séparation −16 % de cm déplacés ; 201 morts (182) ; marcheurs 468 (361).
  **S4** 061585e3b17e519e : attacking 0,7 / 21 / 325 (1,0 / 36 / 361), 46,5 % ; approach 22,1 % (37,5) ; 222 morts
  (198). 2e essai `melee_press` lancé (note : `hol.Combat.PreferFreeTarget 0` pour comparer).
- (J7c.4, mycrusader-56) **2e essai `melee_press`** : « C'est vachement mieux » (cible pleine laissée : validé à l'œil).
  **Nouveau défaut** : « au début du scénario, la ligne rouge et la ligne bleue se sont "traversées" et se sont
  emmêlées ; normalement ce sont deux lignes qui se choquent et ce sont les morts qui vont faire cet emmêlement »
  (référence de l'utilisateur : https://www.youtube.com/shorts/zWR2KnqUMVI). Pistes : recherche de cible un pas sur 4
  (deux lignes à ~11,7 m/s de rapprochement : 2,3 m entre deux recherches), `CanPassThrough` de la séparation entre
  marcheurs ennemis. Boucle 9 lancée : diagnostic et mesure du croisement.
- (J7c.4, mycrusader-56) **Boucle 9** (~6 min, arrêtée : édition de liens LNK1104, DLL verrouillée par le jeu de
  l'essai encore ouvert ; compilé sans avertissement). Sonde temporaire `hol.Debug.CrossProbe N` (à retirer avant
  commit : unités passées au-delà de la médiane X adverse, paires ennemies à plus d'un demi-rayon, marcheurs au contact
  d'un ennemi). **Diagnostic** (lecture) : `CanPassThrough` hors de cause (même `OrderSerial` exigé, global : deux
  ordres différents) ; la séparation sépare bien deux marcheurs ennemis. **Cause probable** : le test « à portée »
  (`HOLCombatProcessor.cpp:796`, arrêt 815-819) ne vise que `Combat.Target` : une unité qui touche un AUTRE ennemi
  continue de marcher ; la séparation n'annule que la composante normale, la marche tangentielle continue : elle
  glisse autour du corps et se faufile dans la ligne. `PreferFreeTarget` peut aggraver (cible plus loin, derrière le
  front). **Plan** : `hol.Combat.StopAtContact` (1 par défaut) : un combattant de mêlée qui marche et dont la cible
  n'est pas à portée prend, à chaque pas, l'ennemi vivant à portée de plus petit `UnitId` (grille dense, `GatherNearby`)
  comme cible ; l'arrêt existant fait le reste. En attente : fermeture du jeu par l'utilisateur.
- (J7c.4, mycrusader-56) **Boucle 10** (~8 min, non commitée, sans avertissement) : `hol.Combat.StopAtContact` (1 par
  défaut ; `HOLCombatProcessor.cpp:55-58, 422, 773-801`) : un combattant de mêlée en marche d'ordre (`bMaySeek`, donc
  jamais une cible de clic droit) dont la cible n'est pas à portée prend, à chaque pas, l'ennemi vivant à portée de plus
  petit `UnitId` (grille dense seulement). 0 = a2a470378f3c128b vérifié. **Sonde de croisement** (`melee_press`,
  toutes les 0,5 s) : premier choc meilleur (paires ennemies profondes 5 → 0, marcheurs au contact 2 → 0 ; sans :
  17-23 et 9-13) ; mais **unités au-delà de la médiane adverse** 35-44 par camp à +8 s (20-31 sans) : sonde grossière
  (médiane à ~5 m du front), cause non tranchée (faufilement dans les trous entre ennemis debout, ou enveloppement par
  les ailes qui convergent vers un seul point). **S3** 18f713a534f17114 : attacking 0,0 / 37,5 / 410, 68 % sous 5 cm,
  48,2 % (5,3 / 52 / 384) ; approach 24,5 % ; 234 morts (201) ; combat 3,22 ms (3,09). 3e essai `melee_press` lancé
  (note : `hol.Combat.StopAtContact 0` pour comparer).
- (J7c.4, mycrusader-56) Demande de l'utilisateur (« il faut un bouton pour relancer le scénario ») : ligne ajoutée
  à `EtapeDefini.md` (Outils d'essai). 3e essai relancé à sa demande.
- (J7c.4, mycrusader-56) **3e essai `melee_press`** (avec `StopAtContact`) : « non » : capture de l'utilisateur, les
  deux camps sont **mélangés sur toute la profondeur** (rouges dans la ligne bleue et l'inverse). Proposition de
  l'utilisateur : **deux soldats ennemis ne peuvent pas se pousser**. Boucle 11 lancée : règle derrière une variable
  (paire ennemie : l'autre n'est jamais déplacé), recherche des autres poussées entre ennemis (évitement Mass,
  pilotage), sonde de croisement, captures hors écran avec / sans.
- (J7c.4, mycrusader-56) **Boucle 11** (~8 min, non commitée, sans avertissement) : `hol.Simulation.EnemyNoPush` (1 :
  paire ennemie, l'autre jamais déplacé ni mis en file ; `HOLSimulationProcessors.cpp:491-493, 782-787`) ;
  `hol.Simulation.AvoidEnemies` (0 : le regard en avant `AvoidBodies`, 231-330, ne se détourne plus d'un ennemi ;
  tableau `Owners`). Évitement Mass (`HOLParallelMassProcessors.cpp:69, 590`) sans notion de camp (non touché).
  Sonde affinée (unités avec un ennemi derrière elles à moins de 3 m, histogramme de X par 2 m) : **aucune différence
  nette** on / off / sans évitement des ennemis (pic « ennemi derrière » 96/93, 85/103, 92/108) ; histogrammes déjà
  recouverts sur 8 m au contact. Capture `cross11_off_3s.png` : deux armées en croissant, emmêlées en fermeture
  éclair (les autres captures vides : deux jeux hors écran en même temps). S3 EnemyNoPush 1 : 0db99bd7…, 195 morts.
  **Lecture** : le mélange ne vient ni de la poussée entre ennemis ni du regard en avant ; les armées arrivent en
  paquets lâches (4 paquets de 50 en rayon de 5 m), chaque unité s'arrête à sa portée et celles de derrière passent
  dans les trous. Question posée à l'utilisateur.
- (J7c.4, mycrusader-56) **Choix de l'utilisateur** (recommandation) : **zone de contrôle** (un soldat de mêlée qui
  passe à moins de ~1,5 m d'un ennemi s'arrête et l'engage ; un trou de moins de ~3 m entre deux ennemis devient un
  mur). L'utilisateur est absent : rien à essayer pour lui ; boucle 12 hors écran (captures une passe à la fois).
  `EnemyNoPush` remis à 0 par défaut (sans effet net).
- (J7c.4, mycrusader-56) **Boucle 12** (~8 min, non commitée, sans avertissement) : `EnemyNoPush` remis à 0 ;
  `hol.Combat.ZoneOfControl` (cm bord à bord, défaut 150 ; `HOLCombatProcessor.cpp:59-62, 783-823`, marche forcée
  vers l'ennemi pris : 655, 781, 822, 873). **Ne règle pas la fermeture éclair** : sonde dans le bruit (pic « ennemi
  derrière » 70-93 à 0, 82-88 à 150, 75-81 à 250) ; captures `z12_*` (Saved_Tests\Screenshots\WindowsEditor\HOL)
  emmêlées en peigne à 150 et 250. S3 ZoC 150 : 338cd979…, combat 2,05 ms (1,94), 225 morts (213). **Lecture de la
  session principale sur `z12_150_3s.png`** : les deux armées arrivent **en files régulières** (marche en bloc de
  J7a, attaque-déplacement de 200 unités), avec un écart entre les files plus large qu'un corps : les files de l'une
  passent entre celles de l'autre (dents de peigne). Boucle 13 lancée : vérifier cette piste (écart des files de la
  marche en bloc, bloc encore en marche au contact, zone de contrôle et arrêt au contact actifs pendant la marche en
  bloc ?), captures juste avant le contact, essai sans marche en bloc.
- (J7c.4, mycrusader-56) **Boucle 13** (~5 min, lecture et captures, rien de modifié) : **piste des files fausse** :
  `melee_press` ne déclenche aucune marche en bloc (marche de ~28 m plus courte que la taille du bloc,
  `BlockMarchMinDistanceInDepths` 1 ; 200 en FinalLeg, droit vers leur place) ; formation d'arrivée : rangs tous les
  180 cm, colonnes tous les 75 cm, 26 m de large, alors que les unités sortent des disques d'apparition sur 35 m :
  convergence en diagonale (10-30° pour 121 sur 200). Au premier mort : 120 arrêtées, 80 en marche de combat
  individuelle vers la position de leur cible (`IssueCombatWalk`, `HOLCombatProcessor.cpp:876`). `CanPassThrough`
  hors jeu. `hol.Units.BlockMarch 0` et `PreferFreeTarget 0` : même peigne. Interpénétration dès ~1,5 s après l'ordre
  (deux nuées à 5,2-5,8 m/s chacune : ~58 cm de rapprochement par pas). Les bleus apparaissent en treillis lâche (un
  corps entre voisins), les rouges en tas. **Lecture** : deux nuées lâches qui courent l'une dans l'autre, trajets
  individuels vers des cibles prises à 12 m qui se croisent. Boucle 14 lancée : variante du scénario où chaque armée se
  met d'abord en formation à l'arrêt, puis attaque (le cas d'une vraie partie) ; captures et sonde.
- (J7c.4, mycrusader-56) **Boucle 14** (~9 min, rien compilé) : scénario **`melee_formed`** (gardé, non commité :
  mêmes paquets, déplacement simple de chaque armée près de son camp, `wait 9`, puis la même attaque-déplacement) :
  **le défaut est réel** : peigne même entre deux armées formées (sonde « ennemi derrière » 90/85 à +3 s du premier
  mort ; 96/110 sans ZoC ni StopAtContact). Pas de verbe pour attendre une formation ; formations non vérifiées en
  chiffres (le bleu encore à moitié rangé). **Lecture des captures par la session principale** : `z14_def_pre.png`
  rouge en bloc net face au bleu ; `z14_def_c30.png` (premier contact) **le rouge s'est étiré en longues files
  séparées d'environ un corps**, qui entrent dans le bleu ; `z14_def_k3.png` mélange en bandes sur tout le front.
  Hypothèse : l'attaque-déplacement envoie chaque unité droit à sa place d'une formation d'arrivée derrière l'ennemi
  (FinalLeg, sans marche en bloc : marche plus courte que le bloc) : l'armée se réorganise en files pendant la charge,
  et les files passent dans les trous adverses. Bilan après 14 boucles : arrêt, question à l'utilisateur à son retour.
- (J7c.4, mycrusader-56) **Analyse de l'utilisateur** (sa référence, 1:12 à 1:25) : « il y a juste une bonne distance
  d'avoidance entre les unités et pour donner un coup, ils sont à la portée d'une épée et pas l'un contre l'autre ».
  Chiffres du jeu (`Tools/Data/UnitTypes.csv`) : rayon 60 cm, **portée de mêlée 50 cm bord à bord pour tous**
  (Lancier compris) ; la séparation garde les corps juste au contact. Boucle 15 lancée : portée de mêlée plus longue
  (variable d'essai) et marge entre corps amis, captures `melee_formed`, S3.
- (J7c.4, mycrusader-56) Boucle 15 arrêtée à la demande de l'utilisateur (« ne pars pas dans de la recherche, analyse
  la vidéo ») avant toute modification. **Analyse de la vidéo de référence** (AoE4, 100 contre 100, 1:12-1:25 ;
  téléchargée, images toutes les 0,5 s) : (1) les deux armées approchent **en files aérées** (environ un corps entre
  voisins, de côté comme en profondeur), comme chez nous ; (2) juste avant le choc, un vide de 2-3 m entre les fronts ;
  (3) au choc, **la frontière rouge / jaune reste une ligne continue** (bande mêlée d'un ou deux soldats au plus, aucun
  jaune au fond du rouge), les soldats du front face à face à une longueur d'arme, pas corps contre corps ; (4)
  derrière, les rangs restent aérés et **attendent derrière leurs amis** (ils ne se faufilent pas vers l'avant) ; (5)
  la ligne ondule ensuite là où un camp gagne, les corps tombent sur la ligne. Différence avec nous : chez nous les
  membres d'une file continuent vers leur cible et passent entre les ennemis arrêtés ; là-bas la tête de file s'arrête
  à portée d'arme et la file s'arrête derrière elle.
- (J7c.4, mycrusader-56) **Choix de l'utilisateur** : essayer les trois règles tirées de la vidéo, **avec « ils ne
  poussent plus en attaque »** : (1) **portée d'arme** (frappe à une longueur d'arme, la lance plus loin que l'épée ;
  aujourd'hui 50 cm bord à bord pour tous) ; (2) **espacement entre amis** (petit écart, armée aérée) ; (3) **en
  attaque, un soldat dont le chemin est barré par un ami qui combat attend derrière lui** au lieu de le pousser ou de
  le contourner (révise « un marcheur pousse toujours », limité à l'attaque). **Les trois règles sont validées par
  l'utilisateur, sous condition d'essai** (« je valide les 3 règles sous conditions de test »). **À faire plus tard** : l'utilisateur
  doit lancer des choses qui demandent qu'aucun agent ne tourne et que les worktrees soient propres. Travail en cours
  commité tel quel sur `j7c-shot` (WIP, compilé, non vérifié par `quick_check`).
- **Séance arrêtée ici (demande de l'utilisateur : « marque tout ça pour le faire plus tard », worktrees propres, aucun
  agent).**
  **Reprise exacte** :
  - **État** : `HeroesOfLiberty-Elrond`, branche `j7c-shot`, dernier commit = **ce1dcf5** (WIP de cette séance, au-dessus de 0fbafb4) ;
    `IN_USE.txt` « en pause ». `main` ne contient rien de J7c.4 (seulement le journal et `EtapeDefini.md`).
  - **Dans le WIP** (toutes derrière des variables ; défaut entre parenthèses) : `hol.Combat.PreferFreeTarget` (1,
    **validé à l'œil** : « vachement mieux ») ; `hol.Combat.StopAtContact` (1) et `hol.Combat.ZoneOfControl` (150) :
    sans effet net sur le mélange, à garder ou retirer selon les règles ci-dessous ; essais sans effet, **à retirer** :
    `hol.Simulation.SeparationFriendPushShare` (0,5), `SeparationPushToEnemy` (0), `EnemyNoPush` (0), `AvoidEnemies`
    (1) ; sondes : Jitter et « sep max step » (lecture seule), « Separation source » (`SumOverlaps` en O(N²) pendant
    la mesure : **fausse les temps de S3**, à couper par défaut ou retirer), `hol.Debug.CrossProbe` (temporaire, à
    retirer), sonde réseau par activité ; scénarios `melee_press` et `melee_formed` (Tools/Scenarios).
  - **Empreintes** : S3 / S4 de référence 35c168b12173ffac / 93587c1cc36b353c avec toutes les nouvelles règles à 0 ;
    avec les défauts du WIP elles changent (S3 PreferFreeTarget seul a2a470378f3c128b ; avec StopAtContact
    18f713a534f17114 ; avec ZoC 150 338cd979…).
  - **À faire, dans l'ordre** :
    1. Écrire les trois règles (portée d'arme par type, espacement entre amis, attente derrière un ami qui combat en
       attaque), chacune derrière une variable ; compiler ; **essai de l'utilisateur sur `melee_formed`** tout de suite
       (sans série de mesures avant ; « ne pars pas dans de la recherche »).
    2. Selon l'essai : nettoyage (variables sans effet, sondes), valeurs dans `UnitTypes.csv`, `quick_check -Parts`
       des parties touchées (combat, move, formation), S3 / S4, commit sur `j7c-shot`.
    3. Reprendre S5 en combat (70 Ko/s, seuil 32) : remesurer avec la mêlée corrigée, puis le lot réseau si besoin.
    4. Mesures au repos du soir (`chapter_measures.ps1`), avance de `main`, J7c.5.
  - **À savoir** : la vidéo de référence est https://www.youtube.com/shorts/zWR2KnqUMVI (1:12-1:25) ; l'analyse est
    au-dessus. Un jeu ouvert depuis le worktree verrouille la DLL (LNK1104) : fermer le jeu avant de compiler.
    Bouton « relancer le scénario » demandé (`EtapeDefini.md`, Outils d'essai).
- **Mesures au repos du 2026-10-21 au soir** (demande de l'utilisateur, Discord / Opera / Notepad fermés, processeur
  3-20 % avant chaque course ; `main` d72280a recompilé, serveur dédié recuit) :
  - `main`, `chapter_measures.ps1` (tag `rest_1021_main`) : **S1 sans rapport** (plantage, voir ci-dessous) ; **S2 FAIL**
    3,320 / 3,293 ms (J7b1 : 2,84) ; **S7 PASS** 2,947 / 2,950 ms (J7b1 : 2,58) ; rendu 214-216 i/s partout ;
    **S5 FAIL** 32 200 o/s (seuil 32 000, J7b1 : 30 897), erreur affichée 20,3 cm PASS.
  - **S1 plante** : les 8 joueurs naissent mêlés au même point ; depuis le combat (J7b.2, ffc9164, après les mesures
    J7b1) les unités immobiles attaquent l'ennemi au contact (460 morts en 10 s), puis `GetUnitLocations`
    (`HOLStressTestSubsystem.cpp:1059`) lit le fragment d'une entité morte → `check(CurrentArchetype)` de Mass.
  - Avec `hol.Combat.Enabled 0` (tag `rest_1021_nocombat`) : S1 2,439 / 2,560 ms PASS ; S2 3,139 / 3,114 ms
    **toujours FAIL** : le combat ne coûte que ~0,17 ms à S2, ~0,3 ms viennent d'ailleurs depuis J7b1 (candidats non
    vérifiés : lignes d'ordres côté serveur, déplacements de groupe d3f1408, formation).
  - `HeroesOfLiberty-Elrond` (WIP ce1dcf5, tag `rest_1021_elrond`) : **S3 FAIL** 18,24 / 18,29 ms, 134 i/s ; **S4 FAIL**
    17,42 / 17,32 ms, 131 i/s (avant le WIP : 5,8-7,6 ms) ; la sonde « Separation source » (O(N²), allumée par
    `IsBattle()`) fausse ces temps : chiffres sans valeur tant qu'elle reste allumée. S5 en combat non remesuré. S6
    n'existe pas encore (M8).
  - **S1 corrigé** (choix de l'utilisateur, conforme à la recommandation, contre « un disque par joueur » et « un seul
    joueur ») : S1 coupe `hol.Combat.Enabled` le temps de sa course et le rend à `StopTest` / `Deinitialize`
    (foule et cadrage inchangés ; le coût du combat se mesure dans S2, S7, S3, S4) ; toutes les lectures de fragments du
    stress test passent par `FragmentOf` (entité morte → aucun fragment). Vérifié : S1 deux fois sans plantage ni mort
    (`rest_1021_s1fix_1/2`), temps à reprendre au repos (processeur à 80 % : l'autre session compilait dans Elrond).
    Commité après l'accord de l'utilisateur (fc184fc).
  - **S2, A/B lancé** (accord de l'utilisateur) : côté A = `HeroesOfLiberty-Galadriel` détaché à **e163490** (code des
    mesures `j7b1_rest2`, S2 2,84 ms ; `archer-hit-and-run` fusionnée, rien perdu ; `IN_USE.txt`), compilé ; côté B =
    `main` fc184fc ; S2 de 10 s, A B A B A B (`s2ab_1021_*`).
  - **Résultat S2** : tours propres A 2,780 / 2,823 ms, B 3,006 / 3,018 (**+0,21 ms**) ; 3e tour faussé (processeur
    20-44 %, l'autre session). Traces (portées 623fab4 appliquées au côté A, non commitées) : A 2,716 / 2,810, B 3,178 /
    3,155 ; par processeur (moyenne de 2 traces) **`HOLCombatProcessor` 0 → 0,289 ms** par pas sans aucun combat,
    `HOLUnitOrderProcessor` 0,541 → 0,602 (+0,06), séparation 0,792 → 0,827 (+0,03) ; le reste à moins de 0,005.
    **Le dépassement de S2 est le coût fixe du combat** (2 000 unités rassemblées, triées, `IndexOfEntity` en `TMap`,
    grille, recherches des immobiles), pas une autre régression. Pas corrigé dans `main` : `j7c-shot` réécrit déjà
    `HOLCombatProcessor` (516 lignes, grille dense, collecte parallèle), le correctif irait dans Elrond ou après sa
    fusion. Pistes : table index d'entité → combattant en tableau au lieu du `TMap`, rien à refaire quand aucune unité
    ne peut chercher ni frapper. Côté A laissé en place (`HeroesOfLiberty-Galadriel` détaché à e163490, `IN_USE.txt`).
  - **Choix de l'utilisateur** (conforme à la recommandation) : le correctif du coût fixe du combat se fait **dans
    Elrond, sur `j7c-shot`, avec la mêlée** (pas dans `main` : conflit avec la réécriture de `HOLCombatProcessor`) ;
    confié à mycrusader-80, qui tient Elrond. À vérifier là : S3 / S4 identiques (empreintes), S2 sous 3 ms au repos.
  - **Mesures du matin préparées** (demande de l'utilisateur : « que je lance tout demain matin ») :
    `Tools/Tests/rest_measures.ps1` (non commité, à valider à son premier passage) : arrêt si un Unreal ou une compilation
    tourne, ou si le processeur dépasse 15 % (liste des applications à fermer) ; compile `main` et Elrond ; `main` S1, S2,
    S7 (x2) puis S5 ; Elrond S3, S4 (x2) puis S5 et S5 en combat ; bilan unique `Saved_Tests/Logs/rest_measures_<date>.txt`.
    Essai à blanc (`-DryRun`) passé.
- **Mesures au repos du 2026-10-22 matin** (`rest_measures.ps1`, lancé par l'utilisateur, 15,2 min ; processeur 12,7 %
  au départ, 3-5 % avant les courses ; `main` 05056c5, Elrond `j7c-shot` e7e0151 sans aucune sonde) :
  - `main` : **S1 PASS** 2,270 / 2,250 ms (combat coupé pendant S1, plus de plantage) ; **S2 PASS** 2,908 / 2,945 ms
    (marge 0,07 : le coût fixe du combat, 0,29 ms, reste à enlever) ; **S7 PASS** 2,604 / 2,776 ms ; rendu 252-272 i/s ;
    **S5 FAIL** 32 380 o/s (seuil 32 000), erreur affichée 20,7 cm PASS.
  - Elrond : **S3 FAIL** 5,710 / 5,694 ms, 208 i/s ; **S4 FAIL** 5,635 / 5,764 ms, 199 i/s (seuil de D218, 3 ms ; le
    seuil propre à S3 / S4 restait à décider sur ces chiffres) ; **S5 FAIL** 32 355 o/s ; **S5 en combat PASS**
    24 498 o/s (70 Ko/s avant la mêlée corrigée), 10,2 cm ; 169 envois d'activité et 87 de PV par seconde.
  - `rest_measures.ps1` a fait tout le parcours (compilation des deux copies comprise) : validé et commité (a0a922d).
- **Tranché — D342 : seuil propre aux batailles S3 et S4, ≤ 6 ms** (choix de l'utilisateur, conforme à la
  recommandation ; écarté : optimiser jusqu'à 3 ms). GDD : ligne D342, tableau des seuils de § 16.6, renvoi depuis D218,
  numéro suivant D343. Code sur `j7c-shot` (Elrond) : `MaxBattleSimulationMs` 6,0 dans le rapport du stress test
  (`IsBattle()`), `Get-StepThreshold` dans `chapter_measures.ps1` (S3, S4 = 6 ms ; les autres 3 ms). Vérifié : S3 deux
  fois au repos, **5,003 / 5,102 ms PASS** (seuil 6), 243-252 i/s. S1, S2, S5, S7 inchangés.
- **S5 hors combat, 32,4 Ko/s (seuil 32) : diagnostic commencé, arrêté là (fin de séance demandée par l'utilisateur).**
  - Comparaison au rapport de J6 (lignes « Client : états … le reste … ») : envois par unité inchangés (1,79 par seconde,
    1,76-1,78 en J6), paquets 43/s (42) ; **états** 25,6-25,7 Ko/s par client (25,2-25,5 : **+0,3**, bits par état :
    PV exacts sur 8 bits, 3f5a026 ?) ; **le reste** 6,76 Ko/s (5,9 : **+0,85 Ko/s**, la plus grosse part).
  - **Écarté** : les lignes d'ordres (D328, `OrderChains` de `AHOLPlayerState`) : seuls les ordres d'un joueur en ont,
    pas ceux du stress test (`HOLUnitOrderProcessor.cpp` ~1108, « the server's own … cost nothing here »).
  - **Reprise** : (1) trace réseau du serveur de S5 (Network Insights, `-NetTrace=1 -trace=net` par
    `net_match.ps1 -ServerSettings` ou ligne de commande du serveur) pour lire, propriété par propriété, ce qui fait
    « le reste » ; (2) si besoin, A/B de S5 contre le côté A (`HeroesOfLiberty-Galadriel` détaché à e163490 : serveur
    dédié à cuire là, ~5-8 min) ; candidats : réplication de l'état de combat ou des PV hors états, acteurs ajoutés
    depuis J7b (corps, flèches), état du joueur. Une boucle à part (cuisson + mesures).
  - (**mycrusader-80**, 2026-10-22 ~10:35) **Trace réseau prise** : `net_match.ps1 -ServerArgs` ajouté (arguments de
    ligne de commande du serveur) ; S5 tracée sur la copie principale (5dd09bc, serveur cuit à 9:10, à jour) :
    `-trace=net -NetTrace=3 -tracefile=Saved_Tests\Logs\s5_net.utrace` (2 Mo). Même mesure qu'au repos : **32,8 Ko/s**
    par client, le reste 6,8 Ko/s côté client, 44 paquets/s. Insights n'a pas d'export automatique du réseau : lecteur
    Python de la trace (`Tools/Tests/net_trace_report.py`) confié à un sous-agent (boucle 1).
  - **Boucle 1 finie (6 min, 112k)** : `net_trace_report.py` lit la trace en pur Python (LZ4 écrit à la main), ~2 s.
    Sur 40 s, par client, 31,2 Ko/s comptés par la trace (sans les en-têtes UDP/IP) : **états des unités 27,0 Ko/s**
    (`HOLUnitChunk_State > States`) ; emballage Iris des morceaux d'unités ~3,2 (`ChangeMasks` 2,15 : masque d'éléments
    de tableau de 64 bits par morceau envoyé ; `BatchSize` 0,46 ; en-têtes d'objet 0,6) ; en-tête de paquet 0,47 ;
    **tout le reste ~1 Ko/s** (`GameStateBase` temps du monde 0,06, `HOLUnitReplicator` 0,06, destructions en attente
    0,09, RPC `HugeObjectState` d'un morceau trop gros pour un paquet ~0,07). Ni corps, ni flèches, ni état du joueur.
  - **Cause trouvée : « le reste » n'a pas grossi, ce sont les états** : depuis J7b un état fait **62 bits** au lieu de
    60 (activité 3 → 4 bits pour Attaque / Mort / Face au front, + 1 bit « PV pleins », J7-Q5), mais `s5_report.py`
    comptait 60 bits fixes : les 2 bits × ~3 440 états/s = **~0,86 Ko/s** passaient dans « le reste ». `STATE_BITS`
    corrigé à 62 : le reste retombe à **5,95-5,97 Ko/s** côté client (5,9 en J6). Croissance de S5 depuis J6 = ~0,3 (un
    peu plus d'états) + ~0,86 (2 bits par état). **A/B contre Galadriel inutile**, pas de cuisson.
  - Reste à décider (question à l'utilisateur) : regagner ~0,4-0,8 Ko/s (2 bits dans l'état, ou l'emballage Iris), ou
    accepter ce coût du combat dans le seuil de S5.
  - **Choix de l'utilisateur (D343, 2026-10-22)** : regagner des bits. (1) **Pas sur 7 bits** au lieu de 8 (128 pas,
    lu juste jusqu'à ±64 pas = 3,2 s de l'horloge du client ; le rappel lent renvoie chaque unité au moins tous les
    40 pas). (2) **Positions sur 14 bits** au lieu de 15 : ±819 m autour de l'origine, **cartes jusqu'à 1,6 × 1,6 km**
    (Plaine 400 m) ; avertissement au démarrage de la réplication si les limites de la carte dépassent. Écartée : vitesse
    sur 8 bits (±6,4 m/s, le Paysan va déjà à 6 m/s). État 62 → **59 bits** ; S5 attendue vers ~30,7 Ko/s. Fait dans la
    copie principale (`main`, rien de J7c.4 touché ; j7c-shot change d'autres lignes des mêmes fichiers). Patch prêt,
    appliqué après l'A/B de S2 de mycrusader-43 (qui court sur les binaires de la copie principale).
  - D343 appliquée dans la copie principale (`HOLUnitReplicationRules.h` : `HorizontalBits` 14, `StepBits` 7 ;
    avertissement des limites de carte dans `HOLUnitReplicationSubsystem.cpp` ; `HOLNetTests.cpp` : 59 bits, pas sur 7
    bits, ±63 pas, cartes de 1,6 km ; `s5_report.py` `STATE_BITS` 59). Compile ; `quick_check -Parts tests,net` : net à
    2 clients PASS (combat, flèches), tests 89/90 (le test du pas attendait `% 256`, corrigé) puis **90/90 PASS**.
    Cuisson du serveur et S5 lancées.
  - **S5 PASS : 30 756 B/s par client** (avant 32 380-32 812), erreur affichée 21,8 cm (inchangée), 1,77 envoi par
    unité et par seconde, 42 paquets/s ; le reste 5,9 Ko/s côté client (comme J6). Cuisson + S5 en 202 s. Machine
    chargée par Elrond (images des clients basses, sans effet sur le débit). Aucun avertissement de limites (Plaine).
    **Validé par l'utilisateur, commité** (D343 au GDD). Question de l'utilisateur ensuite : une case de 4,2 m et des
    cartes de 220 × 220 cases au plus (924 m) feraient-elles gagner plus ? (réponse dans la conversation, voir plus bas).
  - Suite (choix de l'utilisateur) : **case de 4 m = grille des bâtiments** (D311 passe de 3 à 4 m), **cartes de 220 × 220
    cases (880 m)**, emprises gardées en cases (Centre 16 m, Caserne 12 m, Maison et camps 8 m) ; positions sur 13 bits
    au pas de 11 cm. **Chantier mené par mycrusader-4b** (`HeroesOfLiberty-J5c`, branche `tile-4m`, D344 et suivantes) ;
    mycrusader-80 lui a passé D343 (à remettre au niveau de `main`) et les outils de S5. Rien de plus ici.
- (J7c.4, **mycrusader-80**, reprise) `HeroesOfLiberty-Elrond` repris (`IN_USE.txt` « en cours », `j7c-shot` au-dessus
  de ce1dcf5). Boucle 16 lancée (sous-agent) : les trois règles derrière des variables — `hol.Combat.WeaponReach` (1)
  avec `hol.Combat.SpearReach` (130 cm bord à bord) et `hol.Combat.SwordReach` (70) ; `hol.Simulation.FriendGap` (25 cm
  entre soldats amis, pas les Paysans) ; `hol.Combat.WaitBehindFighting` (1) — compilation, notes de `melee_formed`,
  un lancement sans fenêtre ; puis essai de l'utilisateur sur `melee_formed`.
- (J7c.4, mycrusader-80) **Boucle 16** (~12 min, 136k tokens, non commitée, compilée sans avertissement) : portée par
  `HOLCombat::GetAttackRange` (`HOLCombatRules.cpp`, bornée à 199 cm, toutes les lectures du combat) ; `FriendGap`
  dans la séparation entre soldats du même joueur (`CarryCapacity` 0 : Paysans exclus ; cases de grille élargies) ;
  `WaitBehindFighting` (`HOLCombatProcessor.cpp` ~853-940 : ami de mêlée qui frappe ou attend à ≤ 50 cm dans un cône de
  ~50° → arrêt sur place, hystérésis 70 cm, profondeur d'attente plafonnée à 6, `WaitDepth` dans `FHOLCombatFragment`).
  Formation compatible (colonnes à 30 cm bord à bord, rangs à 60). Contrôle hors écran (`-RenderOffScreen`, `waitui`
  exige l'interface) : prêt en 10,5 s, 165 morts en ~80 s (63 / 102 : asymétrie à surveiller). À voir : `PlaceGap`
  (50 cm) et `ZoneOfControl` (150) pas alignés sur la lance, frappe à 130 cm peut-être « dans le vide ». 4e essai
  `melee_formed` lancé.
- (J7c.4, mycrusader-80) **4e essai `melee_formed`** : « pas du tout ce qui doit se passer » : la moitié de la première
  ligne est poussée / téléportée au milieu de la mêlée adverse au contact ; un soldat qui n'est pas au contact attend
  qu'une place se libère. **Voulu** : les fronts se chargent ; à portée d'engagement ils s'arrêtent et se frappent ;
  les rangs arrière cherchent une cible proche **non entourée**, restent en alerte, **quitte à se déplacer sur les
  bords**. Nouvelle vidéo de référence de l'utilisateur (GIF 48 s, 6 i/s ; feuilles d'images dans le dossier de
  travail de la session) : approche en files aérées ; au choc une **ligne de contact continue** qui ondule (bande mêlée
  d'un ou deux corps), personne ne la traverse ; derrière, les rangs se **tassent** derrière leurs amis (pas d'écart
  gardé) ; peu à peu les rangs arrière **débordent par les extrémités** et la ligne s'allonge et s'enroule autour du
  flanc adverse. Donc la règle 3 telle qu'écrite (attente) est **refusée** ; l'écart entre amis n'est pas ce qu'on
  voit au contact. Boucle 17 lancée (diagnostic seul) : sonde `hol.Debug.PassProbe` (qui fait passer une unité de
  l'autre côté d'un ennemi : marche, séparation, ZoC…), passes avec / sans `WaitBehindFighting`, `FriendGap`.
- (J7c.4, mycrusader-80) **Boucle 17** (~4 min, 80k tokens, sonde laissée : `HOLSimulationProcessors.cpp` ~511 et
  ~947) : **les traversées viennent de la séparation (`Undo`), pas de la marche** (pas ≤ 29 cm ; ZoC, StopAtContact,
  évitement Mass : 0). Traversées en 8 s après le contact : 126 (défauts, saut max 96 cm), 587 sans
  `WaitBehindFighting` (284 cm), 28 sans `FriendGap` (63 cm). **Mécanisme** : les rangs arrière marchent dans leurs
  amis, « moitié chacun » pousse l'ami de devant vers l'avant (cumulé sur plusieurs rangs dans le même pas) ; face à
  l'ennemi immobile l'unité du front prend tout le chevauchement, le long de l'axe centre à centre **du moment** :
  poussée au-delà du centre ennemi, l'axe s'inverse et `Undo` la sort du côté ennemi (la « téléportation »).
- (J7c.4, mycrusader-80) Boucle 18 lancée (correctif) : `hol.Simulation.EnemySideGuard` (paire ennemie résolue sur la
  normale du début du pas, jamais au-delà du centre) et `SeparationMaxStep` (plafond = rayon) ; `FrontHoldsFriends` (une
  unité au contact d'un ennemi n'est jamais poussée par un ami : les rangs arrière se tassent) ; attente passive
  retirée ; `FriendGap` 0 par défaut ; rangs arrière : cible libre (places comptées avec les portées d'arme), en
  alerte, contournement vers les bords ; passe hors écran avec sonde et captures.
- (J7c.4) **À faire avant de quitter ce soir** (demande via mycrusader-f6) : laisser Elrond **compilable** et la sonde
  « Separation source » **coupée par défaut** : l'utilisateur lancera demain matin `Tools\Tests\rest_measures.ps1`, qui
  compile Elrond et y mesure S3, S4 (×2), S5 et S5 en combat.
- (J7c.4, mycrusader-80) **Boucle 18** (~12 min, non commitée, sans avertissement) : `HOLSimulationProcessors.cpp`
  `EnemySideGuard` (1 ; paire ennemie sur la normale du début du pas, plus petit déplacement qui libère, jamais du
  côté ennemi), `SeparationMaxStep` (-1 = rayon, 0 = sans plafond), `FrontHoldsFriends` (1 ; « front » = attaque en
  cours ou bord à ≤ 10 cm d'un ennemi, passe 3×3 de la grille ; dans une paire amie, l'ami prend tout), `FriendGap` 0 ;
  `WaitBehindFighting` et `WaitDepth` **retirés** ; `CountBodiesAround` : un attaquant prend une place jusqu'à sa portée
  + 20 cm (≤ 200). Passe hors écran (`Saved__P4`) : **1 traversée** (126 avant), plus grand saut d'un traverseur 29 cm
  (96), 146 morts en ~48 s (52 / 94). Non vérifié en image : départ des rangs arrière vers les bords (la nouvelle marche
  peut attendre `bSearchStep`), coincement entre front et ami. 5e essai `melee_formed` lancé.
- (J7c.4, mycrusader-80) **5e essai `melee_formed`** : « la ligne de front fait parfaitement ce qu'il faut, il y a même
  les tentatives d'encerclement à la fin de la part des autres lignes » (**front validé** : `EnemySideGuard`,
  `SeparationMaxStep`, `FrontHoldsFriends`, portées d'arme). **Reste** : pendant les ~30 s après le choc, les rangs qui
  ne sont pas au front « font n'importe quoi, comme s'ils étaient perdus, et se poussent les uns les autres, ce qui les
  secoue » ; voulu : **une vague qui glisse pour combler les trous**. Boucle 19 lancée : mesure des secousses des rangs
  arrière (changements de direction, de cible, corrections de séparation), puis glissement derrière une variable.
  Précision de l'utilisateur : la vague glisse pour combler les trous **« ou commencer à prendre le flanc »** (pas de
  trou proche : glisser vers l'extrémité de la ligne, là où un ennemi n'est pas entouré).
- (J7c.4, mycrusader-80) **Boucle 19** (non commitée, sans avertissement) : `hol.Combat.RearSlide` (1 ;
  `SlideAlongFriends`, `HOLSimulationProcessors.cpp:367-505`, appelée par `UHOLFullSpeedProcessor` : soldat de mêlée
  qui marche pour combattre sans frapper, ami immobile au contact dans ±60° devant → glisse sur la tangente à 70 % de sa
  vitesse, côté de sa cible sinon côté le moins peuplé à 4 m ; deux côtés bloqués → arrêt doux ; vitesse lissée, ±30 %
  par pas) ; `hol.Combat.RearTargetKeep` (200 cm : garde la cible hors portée tant qu'elle est vivante et pas pleine, sauf
  nouvelle plus proche de 200 cm ; `HOLCombatProcessor.cpp:790-813`) ; sonde temporaire `hol.Debug.JitterProbe`.
  Hors écran, 15 s après le choc, unités hors du front (avant → après) : correction de séparation 22,2 / 46,2 cm →
  0,39 / 2,6 cm (moyenne / p95) ; demi-tours 14,8 % → 6,1 % des pas ; changements de cible 2,36 → 0,25 /unité/s ;
  marche contre un ami immobile 33 % → 0,6 % ; traversées 1 → 0. **Mais** morts en 60 s 147 → 92 (≈ 60 au front au lieu
  de ≈ 120 : les rangs arrière ne remplissent plus le front par la pression), choc 0,6 s plus tard (lissage). 6e essai
  `melee_formed` lancé.
- (J7c.4, mycrusader-80) **6e essai** : « c'est mieux, mais maintenant la ligne arrière fait plutôt des gauche-droite
  en boucle, ce qui ne les rend pas intelligents du tout ». Boucle 20 lancée : côté de glissade **engagé** (mémoire par
  unité, pas de retournement tant que le côté n'est pas bloqué un moment), côté choisi vers un ennemi libre ou le bout
  de la ligne plutôt que vers le côté le moins peuplé à 4 m (qui bascule quand les voisins glissent ensemble) ; mesure
  des retournements de côté.
- (J7c.4, mycrusader-80) **Boucle 20** (~10 min, non commitée, sans avertissement) : `hol.Combat.RearSlideKeep` (1 ;
  champs `SlideWay`, `SlideShutSeconds`, `SlideFreeSeconds` dans `FHOLCombatFragment` ; côté gardé, retourné seulement
  si la cible passe nettement de l'autre côté ou si le côté est bloqué 1 s (`RearSlideShutTime`) et l'autre libre ;
  nouveau côté = celui de la cible, sinon en s'éloignant du centre de son armée le long de la ligne (vers le bout le
  plus proche : les voisins prennent le même côté) ; oublié au front ou après 0,5 s libre) ; `RearSlideCreep` (0 :
  avance quand les deux côtés sont bloqués ; 0,15 remonte les morts 89 → 102 mais ramène les secousses ×12). Hors
  écran : retournements de côté 1,06 → 0,23 /unité/s, demi-tours 6,3 % → 1,3 %, traversées 0, morts 88 → 89 (147 sans
  glissade). 7e essai `melee_formed` lancé.
- (J7c.4, mycrusader-80) **7e essai** : « je ne vois plus aucun problème au niveau des lignes » (**glissade validée** :
  `RearSlide`, `RearSlideKeep`, `RearTargetKeep`) ; **reste** : la ligne de front est « extrêmement petite » par rapport
  aux soldats derrière qui ne font qu'attendre ; s'ils essayaient de **déborder par le flanc**, moins de soldats
  inactifs et une ligne de combat plus longue. Boucle 21 lancée : débordement actif des rangs arrière qui attendent.
- (J7c.4, mycrusader-80) **Boucle 21** (non commitée, sans avertissement) : `hol.Combat.RearFlank` (1) et
  `RearFlankDelay` (1,5 s) ; `FHOLFrontLine` (ligne de contact par joueur, calculée en O(N) dans la séparation :
  point moyen, axe, bouts) ; champs `FlankWaitSeconds`, `bFlanking`, `FlankSide`, `FlankWalkGoal` ; débordement
  (`HOLCombatProcessor.cpp:944-1045`) : soldat en attaque-déplacement tenu derrière sa ligne depuis 1,5 s → vraie marche
  vers le bout de la ligne de son côté (+2 m dehors, +1 m vers l'ennemi), arrêt sur un ennemi libre à ≤ 4 m ou au but,
  abandon après 8 s ; sonde `JitterProbe LINE`. Hors écran (30 s après le choc, moyenne ; RearFlank 0 → 1) : au front
  55,7 → 69,3 (92 à 30 s), en attente 296 → 269, longueur de ligne 36,6 → 43,6 m (53 m à 30 s, ne plafonne pas), morts
  89 → 98, traversées 0, demi-tours et retournements +6-7 %. Effet modeste : ~240 attendent encore (pistes : but
  intermédiaire en retrait, départs étalés). 8e essai `melee_formed` lancé.
- (J7c.4, mycrusader-80) **8e essai** : « ça marche super bien » (capture : ligne longue et enroulée des deux côtés,
  **débordement validé**) ; deux défauts : (1) la ligne **met beaucoup de temps à se mettre en place** ; (2) **il manque
  une limite** : un soldat bleu tout à droite peut décider d'aller tout à gauche (un paquet bleu traversait derrière la
  ligne pendant la capture). Boucle 22 lancée : côté = bout le plus proche du soldat, distance de débordement plafonnée
  (et cible libre cherchée près de soi), mise en place plus rapide (délai plus court, départs étalés, point en retrait
  pour ne pas retomber dans la foule).
- (J7c.4, mycrusader-80) **Boucle 22** (non commitée, sans avertissement) : `RearFlankDelay` 1,5 → **0,5 s**,
  `RearFlankStagger` (0,4 s étalés par Id % 8), `RearFlankNearest` (1 : bout le plus proche par projection),
  `RearFlankMaxShift` (1 200 cm : au-delà, pas de débordement), `RearFlankBack` (250 cm : point en retrait au niveau du
  bout avant le but ; champ `FlankBack`), `RearTargetMaxShift` (1 200 cm : ennemi plus loin le long de la ligne compté
  plein ; `RearTargetKeep` ne le garde plus). Hors écran (boucle 21 → 22) : au front à +10 / +20 / +30 s 68 / 84 / 91 →
  **104 / 130 / 133** ; en attente 269 → 220 ; ligne 43 → 63 m (80 au plus fort, 66 à +30 s : raccourcit, à
  regarder) ; morts 103 → **141** ; traversées 0 ; plus grand déplacement latéral **51 → 12 m** ; changements de cible
  0,23 → 0,45 /soldat/s, demi-tours par soldat ×2 (plus de soldats en mouvement). 9e essai `melee_formed` lancé.
- (J7c.4, mycrusader-80) **9e essai : validé** (« ok, nice, on garde ça pour l'instant, bien joué »). Mêlée retenue :
  portées d'arme (Lancier 130, Homme d'armes 70 cm bord à bord), `PreferFreeTarget`, `StopAtContact`, `ZoneOfControl`,
  `EnemySideGuard`, `SeparationMaxStep`, `FrontHoldsFriends`, `RearSlide` / `RearSlideKeep`, `RearTargetKeep`,
  `RearFlank` (boucle 22). Boucle 23 lancée : nettoyage (essais sans effet et sondes retirés, « Separation source »
  retirée), portées dans `UnitTypes.csv`, tests ; puis `quick_check -Parts` des parties touchées et commit sur
  `j7c-shot`.
- (J7c.4, mycrusader-80) **Boucle 23** (~15 min, sans avertissement) : `AttackRange` Spearman 130 / ManAtArms 70 dans
  `UnitTypes.csv`, table refaite ; `WeaponReach` / `SpearReach` / `SwordReach` / `GetAttackRange` retirés (lecture de
  `Constants.AttackRange`) ; retirés : `SeparationFriendPushShare`, `SeparationPushToEnemy`, `EnemyNoPush`,
  `AvoidEnemies`, `FriendGap` ; sondes retirées : CrossProbe, PassProbe, JitterProbe, « Separation source »
  (`SumOverlaps`), « sep max step », ligne « Jitter: » des rapports S3 / S4 et sonde réseau par activité
  (`HOLStressTestSubsystem`, `HOLUnitReplicationSubsystem` revenus à l'état d'avant ce1dcf5). Dossiers `Saved__P*`
  effacés. Restent : tests d'automatisation, passe `melee_formed`, relecture de `HOLUnitAnimation` (frappe à 130 / 70).
  Boucle 24 lancée : ces trois points et `quick_check -Parts` des parties touchées.
- (J7c.4, mycrusader-80) **Boucle 24** (~12 min) : tests 87/87 ; `quick_check -Parts tests,move,formation,combat`
  (347 s) tout PASS sauf `orders` : le contrôle « hold reach » faisait attaquer l'Homme d'armes qui tient la position
  par un Lancier, qui frappe désormais à ~110 cm bord à bord, hors des 70 cm de l'Homme d'armes : 0 coup rendu.
  Contrôle changé (l'attaquant est un Homme d'armes, même portée), la règle de jeu laissée telle quelle ; **question
  à poser à l'utilisateur** : une unité qui tient la position face à une allonge plus longue doit-elle riposter d'un
  pas ? `-Parts combat` relancé : **tout PASS** (168 s, « hold : 5 hits »). Passe `melee_formed` hors écran : 146 morts
  en 60 s (75 / 71), aucune erreur. Lecture de l'animation : le moment du coup ne dépend pas de la portée ; la lance
  (~3 m du centre) atteint le corps à 130 cm ; l'épée (~1,6 m) touche de peu à 70 cm, à regarder à l'œil.
- (J7c.4, mycrusader-80) **Commité sur `j7c-shot` : e7e0151** (mêlée validée, nettoyée, vérifiée).
- **Séance arrêtée ici (demande de l'utilisateur : PC mis en veille prolongée après le commit).**
  **Reprise exacte** :
  - **État** : `HeroesOfLiberty-Elrond`, branche `j7c-shot`, dernier commit **e7e0151** ; compile sans avertissement ;
    plus aucune sonde (S3 / S4 de nouveau mesurables) ; `IN_USE.txt` « en pause ». `main` ne contient rien de J7c.4.
  - **Mesures du matin** : l'utilisateur lance `Tools\Tests\rest_measures.ps1` (copie principale), qui compile Elrond et
    y mesure S3, S4 (×2), S5 et S5 en combat. Empreintes S3 / S4 changées par la mêlée (nouvelles références à
    prendre).
  - **À faire, dans l'ordre** :
    1. Lire les mesures du matin.
    2. **Coût fixe de `HOLCombatProcessor`** (S2 sans combat, 0,289 ms/pas ; choix de l'utilisateur : dans Elrond, sur
       `j7c-shot`) : tableau index d'entité → combattant au lieu du TMap `IndexOfEntity`, sortie rapide quand aucune
       unité ne peut chercher ni frapper ; S3 / S4 identiques (empreintes), S2 < 3 ms au repos ; A/B contre
       `HeroesOfLiberty-Galadriel` e163490 (`stress_ab.ps1 -Test S2 -Seconds 10 -Trace`, `trace_report.py`).
    3. ~~Question « tenir la position face à une allonge plus longue »~~ : tranchée, aucune riposte (plus bas).
    4. S5 en combat (70 Ko/s, seuil 32) : remesurer avec la mêlée corrigée, puis le lot réseau si besoin.
    5. Mesures de fin de chapitre, avance de `main` (accord des sessions actives), J7c.5.
- (J7c.4) **Choix de l'utilisateur (via mycrusader-f6)** : le coût fixe de `HOLCombatProcessor` dans S2 sans combat
  (0,289 ms/pas : rassemblement, tri, empreinte des 2 000 unités, `IndexOfEntity` en TMap) se corrige **dans Elrond,
  sur `j7c-shot`, avec la mêlée**. Pistes : tableau index d'entité → combattant, sortie rapide quand aucune unité ne
  peut chercher ni frapper ; S3 / S4 identiques (empreintes), S2 < 3 ms au repos. Côté A de l'A/B :
  `HeroesOfLiberty-Galadriel` détaché à e163490, compilé avec les portées de trace (`stress_ab.ps1 -Test S2 -Seconds 10
  -Trace`, `trace_report.py`).
- (J7c.4) **Tranché — Tenir la position face à une allonge plus longue : aucune riposte** (choix de l'utilisateur,
  contre la recommandation « riposte d'un pas ») : une unité qui tient la position (D) ne bouge jamais, même frappée
  par un ennemi hors de sa portée (Homme d'armes 70 cm contre Lancier 130 cm : 0 coup rendu). L'allonge est un avantage
  tactique ; au joueur de lâcher la tenue (S ou un autre ordre). Écartés : **riposte d'un pas** (avancer d'~1 m pour
  frapper puis revenir) ; **petite zone tenue** (~2-3 m). Rien à coder (règle actuelle, contrôle « hold reach » de
  `orders` déjà fait avec un Homme d'armes). Au GDD à la clôture de J7 (J7c.5).
- (J7c.4, **mycrusader-43**, reprise 2026-10-22 10:20) `HeroesOfLiberty-Elrond` repris (`IN_USE.txt` « en cours »,
  `j7c-shot` ea447f9). D327 précisé (d69b666). **Boucle 25 lancée** (sous-agent) : coût fixe de `HOLCombatProcessor`
  (tableau index d'entité → combattant, sortie rapide sans combat possible), empreintes S3 / S4 de référence prises
  avant le changement puis comparées, `-Parts tests,combat`, S2 tracé si le temps le permet.
- (J7c.4, mycrusader-43) **Boucle 25** (~11 min, non commitée, 0 avertissement) : la piste « tableau au lieu du TMap »
  était **déjà faite** (`FighterOfEntity` derrière `hol.Combat.FastPaths` ; `IndexOfEntity` ne sert qu'avec
  `FastPaths 0`). Dans S2, pas de combat parce que presque toutes les unités marchent sous un ordre (8 joueurs mêlés
  dans un disque). **Sortie rapide** `hol.Combat.FastIdle` (1 ; `HOLCombatProcessor.cpp`, +41 lignes, après le test
  `!bFighting`, avant le tri) : si aucune flèche en vol et aucune unité vivante n'a cible, dernier attaquant, approche,
  attaque, attaque-mouvement ni recherche à ce pas → `AttackerCount` 0, flanc remis à zéro (non occupées), `TickIdle`,
  `Deaths.Reset()`, retour. **Empreintes identiques** (nouvelles références après la mêlée, rapports de 5 s) : S3
  **8a9b34cde2035b53**, S4 **8b0790f36386d0ea**. `-Parts tests,combat` PASS (176 s, 87/87). S2 tracé (machine
  chargée, 17-34 %, chiffres indicatifs) : `HOLCombatProcessor` 0,478 → **0,434 ms**/pas seulement ; cause probable :
  une seule unité libre à son pas de recherche suffit à reprendre le chemin complet. Suite (boucle 26) : compter les pas
  qui prennent la sortie rapide, portées de trace dans `Execute` (rassemblement, tri, grille, passe en avance, boucle).
- (J7c.4, mycrusader-43) **Boucle 26** (~13 min, non commitée, 0 avertissement ; +121 / −3 lignes, compteurs
  `hol.Combat.CountIdle` 0 par défaut et portées de trace « loop 26 (temporary) ») : S2 de 10 s, pas 100-199 (marche
  sous ordre, aucun combat) : 16 en sortie rapide, **84 bloqués seulement par des chercheurs**, **2,0 chercheurs en
  moyenne par pas** sur 2 000 (tous avec une unité à portée : joueurs mêlés) ; fenêtre mesurée 36 / 200 en sortie
  rapide. Trace (machine chargée, 17 %, pas 4,54 ms ; combat 0,513 ms dont 0,052 de compteurs), par pas en chemin
  complet : boucle principale 0,211, grille 0,127, rassemblement 0,092, comptage des attaquants 0,031, passe en avance
  0,023 + 0,027, tri 0,008. `trace_report.py --scopes "HOLCombat*"` ne montre pas les portées (sommes faites sur
  `events.csv`). Proposé : (1) **sortie rapide par unité** dans la boucle principale (même critère que `FastIdle`,
  testé au tour de l'unité, pas à l'avance, pour rester bit-identique) ; (2) sauter le comptage des attaquants si
  aucune unité n'a de cible ; estimé 0,29 → ~0,18 ms au repos. Boucle 27 lancée : (1) et (2) derrière des variables,
  empreintes S3 / S4.
- (J7c.4, mycrusader-43) **Boucle 27** (~13 min, non commitée, 0 avertissement ; boucles 25-27 : +174 / −10 lignes dans
  `HOLCombatProcessor.cpp`) : **`hol.Combat.FastIdleUnit`** (1 ; boucle Main, juste après le test des mourants : unité
  sans cible, dernier attaquant, approche, attaque ni attaque-mouvement, et occupée ou pas en droit de chercher à ce
  pas, critère testé à son tour → flanc remis à zéro si non occupée, `TickIdle`, `continue` ; chemin complet relu : rien
  d'autre n'est touché) ; **`hol.Combat.SkipAttackerCount`** (1 ; `AttackerCount` mis à 0 au rassemblement, comptage
  sauté si aucune unité n'a de cible). **Empreintes identiques** S3 8a9b34cde2035b53, S4 8b0790f36386d0ea, variables
  à 1 comme à 0. `-Parts tests,combat` PASS (173 s, 87/87). Sous-agent arrêté par la session principale (une attente de
  fond restait). **A/B de S2 au calme lancé** (10:53, processeur 3-5 %, autres sessions prévenues) : même binaire
  Elrond, A = `FastIdle` / `FastIdleUnit` / `SkipAttackerCount` à 0, B = défauts, A B A B A B (`l27ab_*`).
- (J7c.4, mycrusader-43) **Résultats de l'A/B de S2** (machine pas au repos : Discord, VS Code, processeur 5-22 %
  selon les tours ; chiffres indicatifs) :
  - Non tracé (`l27ab_*`, 10 s) : A 3,243 / 3,140 / 4,012 ms, B 3,137 / 3,377 / 3,727 : trop bruité (charge montée à
    16-18 % aux tours 2-3).
  - Tracé (`l27tr_*`, processeur 3-10 %) : `HOLCombatProcessor` A 0,437 / 0,449 → B 0,408 / 0,389 ms : **gain ~0,045
    ms seulement** (estimé 0,1-0,15). Pas entiers A 3,620 / 3,648, B 3,698 / 3,558.
  - **`main` contre Elrond, tracé** (`l27mv_*`, main c446d99 pour le code ; 2e paire la plus propre) : pas 3,396 → 3,597
    ms (**+0,20**) ; séparation 0,864 → 0,930 (+0,07), combat 0,302 → 0,377 (+0,075, raccourcis compris), marche à
    pleine vitesse 0,562 → 0,597 (+0,035), ordres 0,640 → 0,660 (+0,02). 1re paire plus chargée (Elrond 4,110,
    22 % avant) : même sens.
  - **Constat** : `j7c-shot` coûte ~0,2 ms de plus que `main` dans S2 (tracé), réparti entre la mêlée (séparation :
    `EnemySideGuard` / `FrontHoldsFriends` ; `RearSlide` dans la marche ; combat réécrit) ; or `main` est à 2,91 ms au
    repos pour 3 : **fusionner `j7c-shot` ferait probablement passer S2 au-dessus du seuil**. Le coût fixe du combat
    seul n'explique pas tout. Question à l'utilisateur.
- (J7c.4) **Choix de l'utilisateur (conforme à la recommandation) : garder et alléger.** Les raccourcis du combat
  (`FastIdle`, `FastIdleUnit`, `SkipAttackerCount`) sont gardés et commités sur `j7c-shot` sans les compteurs de la
  boucle 26 ; puis les règles de mêlée (`EnemySideGuard`, `FrontHoldsFriends`, `RearSlide`…) ne s'appliquent qu'aux
  unités proches d'un ennemi ; enfin S2 au repos sur `main` et Elrond (l'utilisateur ferme ses applications). Boucle
  28 lancée : retrait des compteurs et portées de la boucle 26, compilation, empreintes, `-Parts tests,combat`.
- (J7c.4, mycrusader-43) **Boucle 28** (~10 min) : compteurs `CountIdle`, `FIdleCounts` et portées de trace retirés ;
  restent `FastIdle`, `FastIdleUnit`, `SkipAttackerCount` (+94 / −7 lignes). 0 avertissement ; empreintes S3
  8a9b34cde2035b53, S4 8b0790f36386d0ea identiques ; `-Parts tests,combat` PASS (176 s, 87/87). **Commité sur
  `j7c-shot` : 60f3005.** Boucle 29 lancée : prix de chaque règle de mêlée dans S2 (traces avec les variables coupées),
  sauts bit-identiques hors combat, propositions qui changeraient une règle sans les écrire.
- (J7c.4, mycrusader-43) **Boucle 29** (~13 min, non commitée, 0 avertissement) : prix des règles dans S2 (traces de
  10 s, machine chargée 16-34 %) : séparation 0,926 ms par défaut → 0,632 les trois coupées ; **`FrontHoldsFriends`
  ~0,24 ms** (0,682 seule coupée), `EnemySideGuard` ~0,07, `SeparationMaxStep` ~0,07 ; `RearSlide` ~0 (ne tourne pas
  dans S2). Cause : la passe « front » (3×3, ennemi à ≤ 10 cm) rend « front » presque toute la foule mêlée sans combat,
  et `Undo` traite les paires amies en « mur ». Écrit, bit-identique : `hol.Simulation.MeleeRuleSkips` (1 ; milieux
  d'armée de `BuildLookAhead` seulement si une unité peut glisser, `FrontLines` en tableaux par joueur au lieu de
  `TMap`), +111 / −3 dans `HOLSimulationProcessors.cpp` ; empreintes S3 / S4 identiques ; gain non mesuré (quelques
  dizaines de µs estimées). `quick_check` pas passé : le sous-agent a lancé celui de la copie principale (il ignore
  `-Project`), arrêté aussitôt, rien de touché. **Leçon** : dans les énoncés, donner le chemin complet du
  `quick_check.ps1` d'Elrond.
- (J7c.4) **Question « front = en combat »** posée (A : front seulement si l'unité ou l'ennemi touché est en combat,
  ~0,24 ms ; B : en plus, garde côté ennemi et plafond seulement en combat, ~0,38 ms) : l'utilisateur veut **voir un
  scénario pour décider**. Boucle 30 lancée : A (`hol.Simulation.FrontNeedsCombat`) et B
  (`hol.Simulation.EnemyGuardNeedsCombat`), défaut 0 ; scénarios `melee_front_a`, `melee_reinforce` (renforts par
  simple ordre de marche contre la ligne), `crowds_cross` (deux foules ennemies qui se croisent) ; empreintes, traces S2.
- (J7c.4, mycrusader-43) **Boucle 30** (~12 min, non commitée, 0 avertissement ; avec la boucle 29 : +164 / −9 dans
  `HOLSimulationProcessors.cpp` / `.h`) : marqueur **`Engaged`** (cible, attaque, approche, attaque-mouvement, détour
  d'attaque-mouvement ou flanc ; rempli au rassemblement, `bAnyEngaged`) ; **A `hol.Simulation.FrontNeedsCombat`** (0 ;
  passe « front » sautée si personne n'est engagé, sinon contact compté seulement si l'une des deux unités l'est) ;
  **B `hol.Simulation.EnemyGuardNeedsCombat`** (0 ; garde de côté et plafond seulement pour les paires où l'une est
  engagée, marquées `Guarded`). `FrontLines` avec A : presque inchangées en mêlée, vides sans combat (sans effet : seules
  les mêlées en attaque-mouvement, elles-mêmes engagées, les lisent) ; un renfort en simple marche qui touche un ennemi
  engagé reste « front ». Empreintes A / B à 0 identiques (S3 8a9b34cde2035b53, S4 8b0790f36386d0ea) ; A à 1 et A+B
  à 1 pas mesurées ; traces S2 pas prises ; `quick_check` pas passé (boucles 29 et 30). Scénarios dans Elrond
  (`Tools/Scenarios`) : `melee_front_a` (A à 1, vérifié), `crowds_cross` (B à 1, vérifié), `melee_reinforce` (A à 1,
  bascule à la console ; 50 renforts envoyés par clic au sol dans le dos de la ligne ; **pas encore vérifié** : la
  sélection au cadre ne prenait que 26 / 50, cadre élargi, non relancé). **Écart de processus** : `stress_ab` refusait
  (jeu de l'essai de l'utilisateur ouvert) et le sous-agent a lancé lui-même deux jeux avec les mêmes arguments pendant
  l'essai. **Leçon** : écrire dans les énoncés de ne jamais contourner le refus « BUSY ».
- (J7c.4, mycrusader-43) **Boucle 31** (~13 min, non commitée) : `melee_reinforce` corrigé et vérifié jusqu'au bout
  sans fenêtre (`SpawnHere 50 ManAtArms 0 400`, `Focus -13200 -300`, cadre `BoxGround -14300 -1400 -12100 800`,
  `waitlog Selection: 50 units (own)`, `waitlog Order: 50 units` ; renforts posés derrière leur ligne à ≤ 18 cm de leur
  place avec A à 1). `quick_check -Parts tests,combat,move,formation` d'Elrond (défauts) **PASS** (375 s, 87/87 ;
  couvre les boucles 29-30). Pas faits : empreintes A / A+B, traces S2. Écart : un premier lancement est parti pendant
  la fin d'une vérification de mycrusader-e9 (la garde affichait sans bloquer) ; corrigé (« BUSY » bloquant). Les
  scripts d'aide `scen.ps1` / `fpd.ps1` (dossier de travail de la session) ne vérifient pas la machine eux-mêmes.
  Commandes d'essai : `powershell -ExecutionPolicy Bypass -File
  C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 <melee_front_a | melee_reinforce |
  crowds_cross>`. Essai de l'utilisateur à lancer dès que le test réseau de mycrusader-4b (J5c) est fini.
- (J7c.4) 1er essai `melee_front_a` lancé puis fermé à la demande de l'utilisateur : **pas de bouton « Relancer »**
  (obligatoire ; c446d99 est dans `main`, pas dans `j7c-shot`). Avec l'accord de l'utilisateur : c446d99 repris sur
  `j7c-shot` (**ab2235f** ; conflit sur le texte d'aide de `hol.HUD.Click` seulement), Elrond recompilé (20 s), essai
  relancé. **Leçon** : avant un essai dans une copie de branche, vérifier que les outils d'essai de `main` y sont.
- (J7c.4) **Essai `melee_front_a` : « tout est bon »** (A à 1 : mêlée identique à l'œil à celle validée). Essai
  `melee_reinforce` lancé.
- (J7c.4) **Essai `melee_reinforce`** (capture de l'utilisateur) : « les unités respectent la ligne mais ne vont pas
  toutes, certaines restent en place » ; « hors scénario : les renforts ont buté plus d'une fois contre le mur, ce qui
  les a empêchés de bien se déployer, un agent doit s'en occuper » → transmis à mycrusader-e9 (chantier déplacement,
  cas 3 « mur à nous », noté dans `MovementInProgress.md`). **Tranché — A adopté** (choix de l'utilisateur, conforme à
  la recommandation) : `hol.Simulation.FrontNeedsCombat` passe à **1 par défaut** (une unité ne compte comme « front »
  que si elle ou l'ennemi touché est engagé dans un combat). Au GDD à la clôture de J7. Essai `crowds_cross` lancé
  pour décider de B.
- (J7c.4) **Essai `crowds_cross`** : avec B à 1, « ils ont tremblé dans tous les sens et là ils sont arrêtés »
  (capture : foules interpénétrées, bloquées) ; avec B à 0 (règle actuelle), « ils ont énormément de mal mais ils
  traversent ; je pense qu'on ne veut pas qu'ils se bousculent, **l'ennemi est un mur** ». **Tranché — B rejeté** :
  la garde côté ennemi et son plafond restent actifs pour toutes les paires ennemies ; variable B retirée du code.
  Le croisement de foules ennemies (l'ennemi doit faire mur, sans bousculade) est transmis à mycrusader-e9 (chantier
  déplacement). Boucle 32 : A à 1 par défaut, B retiré, nouvelles empreintes, S2, `quick_check`, puis commit.
- (J7c.4, mycrusader-43) **Boucle 32** (~13 min, non commitée, 0 avertissement) : `FrontNeedsCombat` **1 par défaut**
  (0 = ancienne règle) ; B retiré entièrement (variable, `Guarded`, conditions : garde de côté et plafond revenus au
  code d'avant la boucle 30) ; `Engaged` gardé pour A ; `MeleeRuleSkips` inchangé (+140 / −6 contre HEAD).
  `melee_front_a` supprimé ; `melee_reinforce` et `crowds_cross` gardés pour le chantier déplacement, réglages A / B
  retirés (notes de `crowds_cross` : voulu « l'ennemi est un mur », aujourd'hui traversée avec peine). **Empreintes
  inchangées avec A à 0 comme à 1** : S3 8a9b34cde2035b53, S4 8b0790f36386d0ea (dans S3 / S4 tout contact ennemi touche
  une unité engagée) ; les références restent. `quick_check -Parts tests,combat,move,formation` PASS (367 s, 87/87).
  Sous-agent arrêté (attente de fond). Contrôle des deux scénarios sans fenêtre lancé par la session principale.
- (J7c.4, mycrusader-43) `melee_reinforce` et `crowds_cross` sans fenêtre : « ready », 0 erreur, 0 avertissement HOL
  (après attente des essais de mycrusader-4b). **Commité sur `j7c-shot` : 03c9161** (A par défaut, `MeleeRuleSkips`,
  B retiré, deux scénarios). Reste : S2 au repos (`main` contre Elrond, et A à 0 / 1), puis S5 en combat et les
  mesures de fin de chapitre.
- (J7c.4) **L'utilisateur remet les mesures au repos à plus tard.** mycrusader-4b avancera `main` sur `tile-4m` (case
  de 4 m, positions sur 13 bits, `DefaultGame.ini` et deux `.uasset`) ; `j7c-shot` (03c9161) devra ensuite se poser
  sur `main` (fusion de `main` dans `j7c-shot` ; conflits attendus : `HOLUnitReplicationRules.h`, `HOLNetTests.cpp`,
  `HOLUIHUD.cpp` après le report du bouton « Relancer » ab2235f).
  **Reprise exacte** : (1) fusionner `main` (avec `tile-4m`) dans `j7c-shot`, compiler, `-Parts` des parties touchées,
  empreintes S3 / S4 (nouvelles références probables avec la case de 4 m) ; (2) mesures au repos (l'utilisateur ferme
  ses applications) : S2 `main` contre Elrond et `FrontNeedsCombat` 0 / 1, puis `rest_measures.ps1` (S1, S2, S7, S5,
  S3, S4, S5 en combat) ; (3) avance de `main` sur `j7c-shot` (accord des sessions), J7c.5.
- (J7c.4, mycrusader-43) `main` avancé par mycrusader-4b (2dd6a66 D344, e5815db GDD). **Boucle 33 lancée** (accord de
  l'utilisateur) : fusion de `main` e5815db dans `j7c-shot` (34 commits, 21 fichiers modifiés des deux côtés dont
  `UnitTypes.csv` / `.uasset`, `HOLCombatProcessor.cpp`, `Net/`, UI, scripts de test) ; table refaite depuis le CSV
  fusionné, `-Parts tests,combat,move,formation`, nouvelles empreintes S3 / S4 ; commit de fusion par la session
  principale ; partie `net` dans une boucle à part.
- (J7c.4, mycrusader-43) **Boucle 33** (~14 min) : fusion en cours, tous les conflits résolus et indexés (7 :
  `UnitTypes.csv` en-tête de `main` avec `AttackWindUpPercent` + portées de mêlée de `j7c-shot` ; `UnitTypes.uasset`
  refait depuis le CSV fusionné, 6 lignes, 0 erreur ; `HOLUIHUD.cpp` version de `main`, texte d'aide complet ;
  `net_match.ps1` les deux côtés ; `s5_report.py` 57 bits + `HEALTH_BITS` ; `stress_ab.ps1` chemins de `main` ;
  `EtapeDefini.md` version de `main`) ; fusion automatique vérifiée (élan par type de l'Archer présent dans tous les
  `StartAttack`, format 57 bits + `StillZeroVelocity`). Compilation 0 avertissement. `quick_check -Parts
  tests,camera,combat,move,formation` : tests 90/90, tout PASS **sauf `move`** (« lines: the same received » côté
  client : une étape `AttackMove (1 units)` en trop ; serveur PASS ; hasard d'échantillonnage ou effet de la fusion, à
  voir). Empreintes S3 / S4 pas prises. Relance `-Parts camera,move` puis empreintes par la session principale.
- (J7c.4, mycrusader-43) `-Parts camera,move` relancé : **PASS** (65 s ; l'échec de `move` était un raté
  d'échantillonnage). Empreintes (lancement direct, `-LiveCoding=0`, nouvelle règle de cohabitation) : **S3
  8a9b34cde2035b53 inchangée**, **S4 91a23fabb162056d** (avant 8b0790f36386d0ea ; seul S4 a des Archers : élan de 12 %
  de `main`). **Fusion commitée : 3654caf**, puis `main` 0886717 (`build.ps1`) fusionné sans conflit : **005eaee**.
  Boucle 34 lancée : serveur dédié d'Elrond avec le code fusionné, `quick_check -Parts net`, S5 indicatif.
- (J7c.4, mycrusader-43) **Boucle 34** (~13 min, rien de modifié) : **`quick_check -Parts net` PASS** (136 s ; serveur =
  éditeur en serveur dédié, aucune cuisson nécessaire) : `net 2 clients` PASS, mêlée (PV affichés = valeurs du
  serveur, écart du Man-at-Arms 5 cm), flèches 18 / 18 / 18, impact → PV 11 / 6 ms en moyenne. Le format 57 bits, la
  vitesse nulle des immobiles et `-useperfthreads` passent ensemble. **Serveur cuit d'Elrond** (pour `net_match` / S5)
  : `build.ps1 -Target HeroesOfLibertyServer` PASS (383 s) ; **reste à cuire** (`net_match.ps1 -Stage -Units 2000
  -Seconds 40`) avant S5, à faire avec les mesures au repos.
  **Reprise exacte (fusion faite)** : `j7c-shot` **005eaee** contient `main` jusqu'à 0886717. Reste, dans l'ordre :
  (1) mesures au repos quand l'utilisateur le décide (applications fermées) : cuire le serveur d'Elrond
  (`net_match.ps1 -Stage`), puis S2 `main` contre Elrond et `FrontNeedsCombat` 0 / 1, puis `rest_measures.ps1` ;
  (2) avance de `main` sur `j7c-shot` (accord des sessions) ; (3) J7c.5.
- (J7c.4, mycrusader-43) **L'utilisateur demande l'avance de `main` avant les mesures au repos** (« 2 du coup »).
  `main` avait reçu le Moulin (3f2053d, D346) et D345 (7f0e206) : fusionnés dans `j7c-shot` sans conflit (**9d92772**) ;
  compilation et `-Parts tests,build,combat` en cours. Obstacle à l'avance rapide dans la copie principale :
  `Tools/Data/UnitTypes.csv` y est modifié sans commit (copie restée là du chantier stats des unités, passé dans
  `HeroesOfLiberty-Haldir`, branche `unit-stats` ; à retirer, accord de l'utilisateur requis). **Conflit de règle à
  trancher** : le chantier stats fixe la portée de mêlée à **116 cm (0,29 case) pour tous les soldats de mêlée**,
  alors que la mêlée validée de J7c.4 utilise **Piquier 130 / Homme d'armes 70**.
- (J7c.4, mycrusader-43) Fusion 9d92772 vérifiée : `build.ps1` PASS (94 s), `-Parts tests,build,combat` **PASS** (460 s,
  90/90). **Choix de l'utilisateur** : (1) la copie restée dans la copie principale est retirée (`UnitTypes.csv`
  restauré, `UnitStats.md` et `make_unit_stats.py` effacés, identiques à ceux de `HeroesOfLiberty-Haldir`) ; (2)
  **Tranché — portée de mêlée par arme** (conforme à la recommandation) : Lancier 130, Homme d'armes 70 (la mêlée
  validée) ; le chantier stats reprend ces valeurs au lieu de 116 pour tous (à lui transmettre). Avance rapide refusée :
  `main` avait reçu MV.5 (e123afc, 5bd8b98, d368a3c ; recouvrement : `HOLUnitOrderProcessor.cpp`) → fusionné dans
  `j7c-shot` sans conflit (**48ffa00**) ; compilation et `-Parts tests,formation,move,combat` en cours.
  mycrusader-e0 (MV.2) prévenu : ne pas avancer `main` pendant l'atterrissage.
- (J7c.4, mycrusader-43) Fusion 48ffa00 vérifiée : `build.ps1` PASS (71 s), `-Parts tests,formation,move,combat`
  **PASS** (355 s, tests 92/92 ; move, formation, groupes, retardataires, blocs, combat, flèches, ordres). **`main`
  avancé sur `j7c-shot` : 48ffa00** (avance rapide dans la copie principale, recompilée par `build.ps1`, 52 s).
  mycrusader-e0 (MV.2) et mycrusader-c1 (stats des unités, `HeroesOfLiberty-Haldir` : portée par arme transmise, elle
  fusionnera `main` dans `unit-stats`) prévenus. Empreintes de référence sur `main` : S3 8a9b34cde2035b53, S4
  91a23fabb162056d. `HeroesOfLiberty-Elrond` libéré.
  **Reprise exacte (J7c.4)** : il ne reste que les **mesures au repos** quand l'utilisateur le décide (applications
  fermées), depuis la copie principale : cuire le serveur (`net_match.ps1 -Stage -Units 2000 -Seconds 40`), puis
  `rest_measures.ps1` (à adapter : tout se mesure maintenant sur `main`, S1, S2, S7, S5, S3, S4, S5 en combat) ; S2
  doit tenir sous 3 ms avec la mêlée (sinon `FrontNeedsCombat` / `EnemySideGuard` à revoir). Puis **J7c.5**.
- (J7c.4, mycrusader-f3, 2026-10-22) **Mesures au repos faites** sur `main` 233e31e (mêlée + D345.1 + D345.2a + MV.5),
  autres sessions mises en pause, processeur 4 % avant le départ (Discord resté ouvert, impossible à fermer
  cette fois ; Edge, IMF, ShivaGet fermés par l'utilisateur). `rest_measures.ps1` adapté : tout sur `main` (S1, S2, S7, S3, S4 x2, S5, S5 combat), Elrond retiré.
  Tag `rest_1022_2003_main`, résumé `Saved_Tests\Logs\rest_measures_1022_2003.txt`, 11,3 min :
  - **S1 PASS** 1,141 / 1,196 ms, 277 i/s ; **S7 PASS** 2,018 / 1,991 ms, 262 i/s.
  - **S2 PASS de justesse** 2,987 / 3,047 / 2,939 ms (moyenne 2,991, 3e passe nécessaire), 260 i/s.
  - **S3 FAIL** 6,207 / 6,156 ms (seuil 6 de D342), 243 i/s ; **S4 FAIL** 6,010 / 6,038 ms, 228 i/s. Au repos sur
    `j7c-shot` (2026-10-21) : S3 5,003 / 5,102 ms → **+1,1 ms**. Détail S3 : combat 2,33, pleine vitesse 1,68,
    évitement en marche 1,19 (1 pas sur 6), séparation 0,97, ordres 0,38.
  - **S5 PASS** 29 924 o/s (seuil 32 000), 23,0 cm ; **S5 en combat PASS** 22 823 o/s, 13,5 cm ; 234 envois
    d'activité et 112 de PV par seconde.
  - Piste : depuis la mesure de référence, `main` a reçu D345.1 (attaques typées, armures, bonus) et D345.2a (bâtiments
    attaquables) ; le combat pèse 2,33 ms au repos contre ~1,5-1,9 ms sous charge avant. À confirmer par un A/B au
    repos 48ffa00 contre 233e31e (S3) avant de toucher au code. **Suite : choix de l'utilisateur.**
- (J7c.4, mycrusader-f3) **A/B au repos de S3** (accord de l'utilisateur ; sessions 1e et 8c en pause ; Discord ouvert,
  processeur 3-18 % avant chaque passe). A = **48ffa00** (`HeroesOfLiberty-Galadriel` détaché ; ses portées de trace de
  623fab4 mises dans `git stash` de cette copie), B = **`main` 464c9f0** (233e31e + MV.4 mur ennemi). Alterné A B A B,
  15 s (script de séance `s3_ab.ps1`, logs `ab_s3_A1/A2` dans Galadriel, `ab_s3_B1/B2` dans la copie principale).
  Un premier essai faussé (compilation et tests de Haldir en plein milieu) a été jeté.
  - A : 5,690 / 5,036 ms (moyenne **5,36**) ; B : 6,168 / 6,262 ms (moyenne **6,21**) → **+0,85 ms**.
  - Par étape (moyennes A → B) : **combat 1,60 → 2,33 (+0,73)**, pleine vitesse 1,44 → 1,67 (+0,23), évitement en
    marche 1,25 → 1,21, séparation 1,06 → 0,99, ordres 0,37 → 0,39.
  - La bataille elle-même a changé : A 82 coups/s, 26 morts ; B **104 coups/s, 51 morts** (valeurs des unités de
    eb73ead : vitesses, portées, PV ; plus d'unités au contact). Le combat coûte plus, en partie parce qu'il y a plus de
    combat. Reste à séparer **valeurs (eb73ead, voulu)** et **code de D345.1 / D345.2a (armures, bonus, bâtiments
    comme cibles)** : A/B eb73ead contre `main`, ou trace du combat. Galadriel reste détaché sur 48ffa00 en attendant.
- (J7c.4, mycrusader-f3) **2e A/B au repos de S3** (dernier de la soirée, accord de l'utilisateur ; 1e et 8c en pause) :
  A = **275581a** (valeurs de eb73ead, sans MV.2, D345.1, D345.2a, MV.4), B = `main` 464c9f0. Logs `ab2_s3_*`.
  Machine un peu plus chargée qu'au 1er A/B (tout ~+0,7 ms), mais alternée :
  - A : 7,133 / 7,065 ms (7,10), combat 2,704 / 2,751 ; 97 coups/s, 45 morts.
  - B : 6,892 / 7,036 ms (6,96), combat 2,732 / 2,748 ; 104 coups/s, 51 morts.
  - **Conclusion : le code de D345.1 / D345.2a ne coûte rien de mesurable** (combat identique, malgré 7 % de coups en
    plus). **Le dépassement de S3 / S4 vient des valeurs des unités de eb73ead** (bataille plus dense : 82 → 97-104
    coups/s, 26 → 45-51 morts dans la mesure), pas d'une régression du code.
  - Galadriel remise dans son état d'avant (e163490 détaché + portées de trace de 623fab4 rendues par `git stash pop`,
    `IN_USE.txt` d'origine, mention de l'emprunt) ; ses binaires sont ceux de 275581a (recompiler avant usage).
  **Reprise exacte (J7c.4, mesures)** : mesures au repos faites (S1, S2 de justesse, S7, S5, S5 combat PASS ; S3 6,18
  et S4 6,02 ms FAIL au seuil 6 de D342). Cause : valeurs des unités (eb73ead), pas le code. **À trancher avec
  l'utilisateur (demain, pas de mesure avant)** : optimiser le combat (2,3 ms au repos, le plus gros poste de S3) pour
  repasser sous 6 ms, ou revoir le seuil des batailles (D342) avec les nouvelles valeurs. `rest_measures.ps1` adapté
  (tout sur `main`) et ce journal : non commités. Puis **J7c.5**.

### 2026-10-21 — Retour d'essai 8 : hit and run de l'Archer

- Séance **mycrusader-4f**, copie **`HeroesOfLiberty-Galadriel`** (`IN_USE.txt`), branche **`archer-hit-and-run`** depuis
  `main` ea84b7b. `EtapeDefini.md` : « Hit and run impossible avec l'Archer : le geste de tir dure toute la cadence ».
- **Constat (lecture du code)** : un ordre de marche (`Disengage`) garde la recharge (`CooldownMs` non remis à zéro,
  `TickIdle` pendant la marche) et une flèche lâchée part quand même (J7c-Q2). Le blocage : à chaque reprise de cible,
  `StartAttack` impose l'élan de **50 % de la cadence** (812 ms pour l'Archer, 1,625 s), partagé avec la mêlée
  (`HOLCombat::WindUpPercent`, `HOLUnitAnimation::StrikeHitShare`) ; un ordre pendant l'élan perd le tir. Chez le
  client, la cadence rejouée (`HOLNetUnitDisplay`) repart toujours de l'élan : faux si la recharge n'était pas finie.
- **Tranché — Lâcher au quart pour les tireurs** (choix de l'utilisateur, A, conforme à la recommandation) : élan par
  type dans la table (Archer ~25 %, soit ~0,4 s ; mêlée inchangée à 50 %) ; geste de tir refait avec le lâcher au quart
  (bandé plus vite, recharge plus longue), toujours une boucle à vitesse unique, coupée par la marche ; chez le client,
  la recharge rejouée continue pendant la marche (0 bit de plus). Écartés : **tir joué une fois puis pose « prêt »**
  (plus lisible, mais vitesses changeantes dans l'affichage, une pose de plus) ; **lâcher au quart pour tous** (refait
  les coups de mêlée validés en J7b). Au GDD à la clôture de J7 (J7c.5).
- (mycrusader-4f, boucle 1) **Code écrit** (pas encore compilé) : colonne **`AttackWindUpPercent`** (int, 50 par
  défaut, bornée 1..100) dans `FHOLUnitTypeRow` / `FHOLUnitTypeConstants` (`HOLUnitTypes.h`) et `UnitTypes.csv`
  (Archer 25, les autres 50) ; `HOLCombat::GetWindUpMilliseconds` / `StartAttack` prennent le pourcentage du type
  (`DefaultWindUpPercent` = 50 garde l'ancienne valeur) ; `HOLUnitAnimation::StrikeHitShare` devient
  `GetStrikeHitShare(Type)` et `GetStrikeStartPosition` prend la part du coup. Client : `FHOLNetUnitDisplayFragment`
  garde la recharge rejouée hors attaque (`StopShots` : tenue au dernier pas rejoué, `ShotIdleStep`) ; à la reprise
  (`StartShots`) : reste = recharge − pas d'arrêt × 50 ms (plancher 0), puis `StartAttack` moins le pas déjà compté,
  0 bit de plus. Le geste d'un tireur chez le client se cale sur cette cadence rejouée (`ShotSecondsToHit`, calculé
  par `UHOLNetUnitDisplayProcessor`, lu par `UHOLInterpolatedInstancesProcessor`) : lâcher dessiné sur la flèche
  dessinée. Tests : `Combat.Damage` (Archer 25 %, mêlée 50 %), `Combat.Cadence` (406 ms), nouveau
  **`Combat.HitAndRun`** (tir au pas 9, marche de 10 pas → tir suivant au pas 41 comme debout, 25 pas → élan après
  l'arrêt, jamais plus tôt que debout), `Animation.Strike` (part au quart), nouveau **`Net.ShotReplay`** (flèches du
  client sur les pas du serveur à travers arrêts et marches). Le geste de l'Archer garde son lâcher à 50 % : décalé
  d'un quart de boucle jusqu'à sa refonte (boucle suivante).
- (mycrusader-4f, boucle 1) **Compilation** `HeroesOfLibertyEditor Win64 Development` : **OK, 0 avertissement**
  (153 s ; un premier essai refusé, « Live Coding is active », à cause de deux bancs S4 de la copie Elrond : attendus).
  **Table refaite** depuis le CSV (`make_unit_types.py`, `UnrealEditor-Cmd -run=pythonscript ... -nullrhi -nosound`,
  18 s, 0 erreur) : `Content/Units/UnitTypes.uasset` modifié.
- (mycrusader-4f, boucle 1) `quick_check` rapide **PASS** (87 s, tests 89/89, smoke PASS). `-Parts tests,combat`
  **PASS** (208 s, 89/89) : combat (premier coup 0,75 s après l'apparition, morts à 9,4 / 14,15 / 19 s, 45 coups),
  combat anims, bodies, arrows, arrows flee, orders (attack-move : Lancier mort au rapport 8 sur 24, 10 coups ; push :
  0 cm, 13 coups) tous PASS. `-Parts net` **pas lancé** (une autre session occupait le serveur dédié, consigne du
  coordinateur) : à faire à la boucle suivante. Pas de référence chiffrée de `main` sous la main pour la mêlée ; la
  règle est la même au bit près pour la mêlée (50 % → mêmes 937 / 687 ms, test `Combat.Cadence` inchangé et PASS).
- (mycrusader-4f, boucle 2) **Geste de tir refait, lâcher au quart** (`soldier_poses.py`, `ARCHER_ATTACK`, nouvelle
  constante `ARCHER_RELEASE` = 0,25) : bandé deux fois plus vite (assise 0,05, levée et bandé 0,14, pleine allonge
  0,19, visée 0,23, **lâcher 0,25 = image 12 sur 48**), suivi au même rythme (main rejetée 0,31, tenue 0,40), recharge
  un peu plus lente (carquois 0,56, flèche sortie 0,68, ramenée sur l'arc 0,82, encochée 0,90) puis pose « prêt »
  tenue jusqu'à la fin de la boucle (dernière clé = première). Flèche cachée de 0,41 à 0,896 s (images 13 à 26, au
  lieu de 25 à 35) ; images zoomées de l'aperçu `[0, 2, 7, 9, 12, 15]` (prêt, assise, levée, pleine allonge, lâcher,
  suivi ; avant `[0, 5, 14, 18, 24, 27]`) ; clé de geste **`"hit_fraction"`** lue par la relecture du baker
  (`make_peasant_animations.py`, lâcher attendu à l'image 12 au lieu de `ATTACK_HIT_FRACTION`).
  `make_peasant_mesh.py --provider-only --soldiers` honore désormais `--only=SKM_Archer` (ne refait que `TP_Archer`).
- (mycrusader-4f, boucle 2) **Aperçus** (`preview_animation.py`, sans Unreal) dans `Saved_HitAndRun\` :
  `archer_attack_release.png` (images 9 à 16 : flèche encochée à pleine allonge jusqu'à l'image 12, partie à l'image
  13, main rejetée) et `archer_attack_loop.png` (une image sur 3). Contrôles : mains à 0,000 cm, pieds 0,000 cm,
  boucle 0,000 cm ; flèche dans le corps 0,5 cm au plus (0,13 s, levée).
- (mycrusader-4f, boucle 2) **Fabriqué** (PowerShell, `UnrealEditor-Cmd ... -run=pythonscript -unattended -nop4
  -nosplash -nullrhi -nosound`) : `make_peasant_animations.py --soldiers --only=archer_attack` (3 min 51, 0 erreur) →
  `A_Archer_Attack` 48 images / 1,6 s (longueur inchangée : rien à changer dans la table), relecture brute ≤ 0,118 cm
  (`foot_r` à 0,1 s), compressée ≤ 0,116 cm, boucle 0,000, `ik_hand_gun` ≤ 0,004 cm / 0,072°, mains à la flèche
  ≤ 0,025 cm, **lâcher lu à l'image 12 (attendue 12)**, **flèche cachée images 13 à 26** (14 sur 49, lues identiques).
  Puis `make_peasant_mesh.py --soldiers --provider-only --only=SKM_Archer` (34 s, 0 erreur) : `TP_Archer` refait,
  mêmes 6 séquences, fichier identique à l'octet (il ne garde que les références aux séquences).
- (mycrusader-4f, boucle 2) `quick_check -Parts tests,anims,combat` **PASS** (217 s, tests 89/89 ; combat : premier
  coup 0,75 s, morts à 10,85 / 14,4 / 19,55 s, 64 coups ; anims, combat anims, bodies, arrows, arrows flee, orders
  PASS). `-Parts net` **PASS** (129 s, aucun autre test réseau en cours ; flèches : 18 lâchées, 18 lancées chez
  chaque client, écart arrivée → perte de vie moyen −12 / −30 ms). Reste : essai en jeu par l'utilisateur
  (`Tools/Scenarios/archer_kite.txt`), puis commit après validation.
- (mycrusader-4f) Boucles 1 et 2 relues par la session principale (rejeu du client, planche du lâcher au quart). Écart
  des chiffres de la partie `combat` entre les deux boucles (morts à 9,4 / 14,15 / 19 s, 45 coups ; puis 10,85 / 14,4 /
  19,55 s, 64 coups) : mêlée seule (50 %, inchangée), partie jouée en temps réel dans la partie à 2 joueurs ; non
  élucidé, l'animation n'est pas lue par le serveur. **Essai de l'utilisateur lancé** : `play.ps1 archer_kite -Root
  ..\HeroesOfLiberty-Galadriel` (5 Archers contre 3 Hommes d'armes ennemis à 25 m ; tirer, reculer, retirer).
- (mycrusader-4f, boucle 3) **Retour d'essai de l'utilisateur** sur `archer_kite` : « divise par 2 le temps de lancer
  d'une flèche » → élan de l'Archer **12 %** (1625 × 12 / 100 = 195 ms, lâcher au 4e pas de 50 ms, 0,2 s ; avant 406 ms,
  9e pas), **cadence inchangée** (1,625 s) ; « très difficile de cibler un ennemi, il faut cliquer précisément dessus ;
  une marge autour d'une cible serait bien » → marge en pixels autour d'un ennemi pour le clic droit.
- (mycrusader-4f, boucle 3) **Élan 12 %** : `UnitTypes.csv` (Archer 25 → 12), commentaires de `HOLUnitTypes.h`,
  `HOLCombatRules.h`, `HOLUnitAnimation.h` (lâcher dessiné à 0,125 de la boucle, 0,005 de boucle ≈ 8 ms après la flèche
  du serveur, accepté). Tests mis à jour : `Combat.Damage` (12 %), `Combat.Cadence` (195 ms), `Combat.HitAndRun` (tir
  au pas 4, debout le suivant au pas 37 ; marche de 10 pas → 37, de 25 pas → 37 aussi (370 ms restants > 195), de 30
  pas → 4 + 30 + 4 = 38, de 60 pas → 68 ; jamais avant debout), `Animation.Strike` (part 0,12, départ de boucle 0,045 s),
  `Net.ShotReplay` (12 % ; tir au pas 103, après une marche de 0,5 s le suivant au pas 136 comme debout, pas 124).
- (mycrusader-4f, boucle 3) **Geste de tir refait, lâcher à 0,125** (`ARCHER_RELEASE` = 0,125, **image 6 sur 48**,
  0,2 s) : bandé deux fois plus vite encore — levée et bandé 0,06, pleine allonge 0,095, visée 0,113, lâcher 0,125 ;
  **assise supprimée** (0,025 : trop serrée ; `ARCHER_SET` retiré) ; suivi au même rythme 0,185 ; la suite étalée sur le
  reste de la boucle : tenue 0,29, carquois 0,48, flèche sortie 0,62, ramenée sur l'arc 0,78, encochée 0,88, puis
  « prêt » tenu jusqu'à la fin. Flèche cachée de 0,21 à 0,768 s ; images zoomées `[0, 2, 3, 5, 6, 9]` ;
  `"hit_fraction"` suit `ARCHER_RELEASE`. Aperçus (`preview_animation.py`) : `Saved_HitAndRun\archer_attack_release_12.png`
  (images 3 à 10 : flèche encochée à pleine allonge jusqu'à l'image 6, partie à l'image 7, main rejetée) et
  `archer_attack_loop_12.png` (une image sur 3) ; regardés. Mains 0,000 cm, pieds 0,000 cm, boucle 0,000 cm ; flèche
  dans le corps 0,5 cm au plus (0,13 s).
- (mycrusader-4f, boucle 3) **Marge de cible** : le choix d'une unité sous le curseur est `HOLUnitPicking::PickAt`
  (corps projeté à l'écran, D288), appelé par `FindUnitAt` (clic gauche `SelectAt`, clic droit `OrderAt`). Nouveaux
  `HOLUnitPicking::GetBodyDistance`, **`PickNear`** (un clic sur un corps gagne toujours ; sinon le corps le plus proche
  du curseur dans la marge, à égalité celui de devant), `GetScaledMargin` (marge donnée à 1080 px de haut, à l'échelle
  de la fenêtre) ; constante **`TargetMarginAt1080` = 14 px**, variable **`hol.Select.TargetMargin`** (0 = sur le corps
  seulement). `AHOLPlayerController::FindTargetAt` (unités des autres joueurs seulement, `GatherPickCandidates` prend
  `bOthersOnly`) remplace `FindUnitAt` dans `OrderAt` ; un ennemi pris par la marge seule cède à une ressource cliquée
  directement (le chantier à soi reste testé avant). **Sélection au clic gauche inchangée** (sur le corps seulement :
  cliquer à côté d'une unité pour désélectionner reste possible). Il n'existe **ni teinte de survol** ni choix d'unité
  par Q (l'attaque-marche Q vise un point du sol) : rien à aligner. Test `Movement.Picking` étendu (distance au corps,
  marge prise / refusée, coup direct gagnant, le plus proche plutôt que celui de devant, marge à l'échelle).
- (mycrusader-4f, boucle 3) **Compilation** `HeroesOfLibertyEditor Win64 Development` : **OK, 0 avertissement**
  (852 s, machine chargée par une compilation ProjectShiva). **Table refaite** (`make_unit_types.py`, 427 s, 0 erreur,
  6 lignes). **Fabriqué** `make_peasant_animations.py --soldiers --only=archer_attack` (14 s, 0 erreur) :
  `A_Archer_Attack` 48 images / 1,6 s (longueur inchangée), relecture brute ≤ 0,013 cm, compressée ≤ 0,012 cm, boucle
  0,000, `ik_hand_gun` ≤ 0,007 cm / 0,072°, **lâcher lu à l'image 6 (attendue 6)**, **flèche cachée images 7 à 23**
  (17 sur 49, lues identiques). `TP_Archer` pas refait (il ne garde que les références, longueur inchangée).
- (mycrusader-4f, boucle 3) `quick_check` rapide **PASS** (89 s, tests 89/89, smoke PASS). `-Parts tests,anims,combat`
  **PASS** (214 s, 89/89 ; combat : premier coup 0,7 s, morts à 9,15 / 14,35 / 19,65 s, 63 coups ; anims, combat
  anims, bodies, arrows, arrows flee PASS ; **orders PASS** : clic droit sur l'ennemi par `hol.Select.OrderGround` →
  attaque, file A/B/C, attaque-marche, tenue, poussée). `-Parts net` **PASS** (141 s, aucun autre test réseau ; flèches :
  18 lâchées, 18 lancées chez chaque client, écart arrivée → perte de vie moyen −18 / −18 ms). Reste : essai en jeu par
  l'utilisateur (`archer_kite` : tir ~0,2 s après l'arrêt ; clic droit à côté d'un ennemi), puis commit après validation.
- (mycrusader-4f) Boucle 3 relue par la session principale. Effet de bord de `FindTargetAt` (unités des autres joueurs
  seulement) : un clic droit sur une de ses propres unités placée devant un ennemi (dans la marge) attaque l'ennemi, là
  où il donnait un ordre de marche. Notes de `archer_kite` mises à jour (0,2 s, marge de 14 px). **2e essai de
  l'utilisateur lancé** : `play.ps1 archer_kite -Root ..\HeroesOfLiberty-Galadriel`.
- (mycrusader-4f) **2e essai de l'utilisateur validé** : « Tir, très bien » ; marge essayée à la console
  (`hol.Select.TargetMargin 25`) : « est bien » → **marge par défaut 25 px** (`TargetMarginAt1080`, notes de
  `archer_kite`). Recompilé ; ligne retirée d'`EtapeDefini.md`. Suite : commit, fusion de `main` (2 commits de doc de
  J7c.4), `-Parts tests,combat` sur la base à jour, avance de `main`.
- (J7c.4, 2026-10-24 soir) **Diagnostic de S5 sans mesure** (relecture des journaux du dernier S5, `-Stage`, 21:23,
  `s5_report.py`) : 32 073 o/s, **1,90 envoi par unité et par seconde** (1,77-1,79 du 2026-10-22 au J6) ; état 57 bits
  inchangé, « le reste » 5,98 Ko/s inchangé. **Les +7 % viennent du nombre d'envois** : la marche est moins prévisible
  (erreur affichée 23,0 → 24,2 cm aussi). S5 était à 29 924 o/s le 2026-10-22 au soir (`rest_measures_1022_2003`), à
  32 247 le 24 au matin (018c3a2) : la hausse vient d'un commit entre 3763623 et 018c3a2. Suspects (trajectoires des
  marcheurs) : 6ab675c (MV.4 détour sans contact), 7776dab (renforts, aide aux amis), 67cdb08 et bdd3677 (rochers),
  dd5a66e (détour vers une emprise), e333332 (keep-out à l'arrêt). Pas de bits à regagner : la cause n'est pas la taille.
  Question à l'utilisateur : trouver le commit (bissection de S5, ~4 min par cuisson + mesure) et voir si c'est une
  oscillation à corriger, ou accepter le coût des nouveaux détours et revoir D218.
- (J7c.4, 2026-10-24 nuit) **Bissection de S5 dans Elrond : pas de régression du code.** Chaque point recompilé et
  recuit (`net_match.ps1 -Stage -Units 2000`, script de séance `s5_point.ps1`) : **99ee126** 1,91 envoi/unité/s,
  32 185 o/s ; **3763623** (le code du 29 924 o/s du 22 au soir) 1,91, 32 224 o/s ; le même 3763623 **machine au
  calme** (b8 en pause, aucun autre Unreal) 1,93, 32 637 o/s. S2 marche toujours en LightInfantry (300 cm/s, inchangée
  par eb73ead). Le 29 924 du 2026-10-22 n'est pas reproductible (piste non prouvée : serveur cuit plus tôt et repris par
  `chapter_measures.ps1`, qui ne recuit que si une source est plus récente que l'exe). **S5 sur ce code vaut ~32,2-32,6
  Ko/s, comme `main` aujourd'hui** : le dépassement de 0,2-2 % est le niveau réel depuis au moins le 22. Elrond rendue
  (`allied-combat` 8fcb0f4, recompilée). Reste la décision de l'utilisateur : revoir D218 (≈ 33-34 Ko/s), ou gagner
  des bits sur les marcheurs ; le seuil d'envoi (35 cm) ne peut pas monter, l'erreur affichée est déjà à 24-25 cm.
