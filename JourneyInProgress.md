# Journey in progress

**Journal du chantier « parcours du joueur » (D252)**, ouvert le 2026-10-13. Après M2, le jeu avance dans l'ordre où
le joueur découvre une partie, chaque étape étant jouable : **J1** emplacement de départ et caméra RTS ; **J2** centre
principal par joueur, sélection ; **M2.5** réseau ; **J3** panneau du bâtiment et production d'un Paysan ; **J4** ordres
de déplacement et chemins ; **J5** récolte et ressources ; puis construction, armée et combat.

Cohabitation (mêmes règles que le chantier menu) : M2 tourne dans une autre session sur le module `HeroesOfLiberty` ;
décisions numérotées **D252 et suivantes**, tenues ici pendant le plan puis reportées au GDD (§ 21) ; pas de worktree,
commits sur `main` par chemins explicites ; ne compiler que si aucun éditeur ni compilation du projet ne tourne ; jamais
de code qui ne compile pas ; ne pas changer l'environnement du stress test (GameMode global, pion, caméra) sans le
vérifier sur S1 et S2.

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
   récolte), en commençant par **J5c.1** (voir « Plan de J5 »). En sous-agents (règle ci-dessous).
   Petit reste : le commentaire du test `Gather.Spots` décrit encore l'écart de 70 cm (le test passe l'écart
   explicitement : il reste juste, à reformuler à l'occasion). Hors de ce chantier, non commités : `GDD.md` (review du
   GDD, autre session) et les lignes du plan de J6 dans ce fichier (autre session).

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

## Commandes utiles

Compilation, traductions : voir `FrontEndInProgress.md` (« Commandes utiles »). Toujours depuis **PowerShell** (Git
Bash réécrit `/Game/...`), aucun éditeur du projet ouvert.

**Vérification de tous les jours (D267) : `quick_check.ps1`**, ~30 s : tests d'automatisation en parallèle d'un seul
lancement du jeu (menu fr / en / ja avec bascule de langue en direct, liste déroulante, partie à 2 puis 8 joueurs avec
les contrôles de caméra de D257). Chaque étape part dès que la précédente est finie (`hol.Test.Run`), chaque capture est
vérifiée (écran affiché au moment de la prendre). Une ligne PASS / FAIL par partie. **S1 et S2 seulement** quand on
touche aux unités, à la simulation ou au rendu, et à la clôture d'une étape.

```powershell
# Vérification rapide (D267) ; -Parts tests,menu,camera pour n'en lancer qu'une partie
powershell -ExecutionPolicy Bypass -File Tools\Tests\quick_check.ps1

# Lancer une session d'essai (deuxième écran, réglages de test) ; Jouer → Lancer
Start-Process "C:\UE\UE_5.8_AS_3\Engine\Binaries\Win64\UnrealEditor.exe" -ArgumentList '"C:\Users\baudo\Documents\MyCrusader\HeroesOfLiberty.uproject" -game -windowed -ResX=1600 -ResY=900 -WinX=3940 -WinY=100 -saveddirsuffix=Tests -nosplash'

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
   (comme D234) ; l'activité répliquée gagne « récolte » et le type de charge.
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
