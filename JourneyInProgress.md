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
| J2.3 Sélection : canal, clic, Échap, cercle, survol | à faire |
| J2.4 Vérification (D264) | à faire |
| J2.5 Clôture : D259 à D264 au GDD | à faire |

## Prochaine étape exacte

**J2.2 faite.** Suite :
1. **J2.3, sélection** (D262) : canal de collision propre à la sélection (pièces du bâtiment), clic gauche (rayon sous
   le curseur) dans le contrôleur, clic dans le vide et Échap pour désélectionner (actions dans `IMC_Game` par
   `make_game_input.py`), cercle au sol à la couleur du propriétaire sous le bâtiment sélectionné, survol : le bâtiment
   s'éclaircit (paramètre des matériaux) ; commandes de jeu pour les essais par script (clic simulé à une position).
2. J2.4 et J2.5 dans l'ordre du tableau.
3. **En attente** : S1 et S2 au repos, nouvelle référence sans ombres ni post-traitement (D258, D265).

**Règle de cohabitation avec le stress test** : seule une carte qui a des emplacements de départ reçoit la caméra RTS ;
une carte sans emplacement (`StressTest`, `BattleMap`) garde le pion par défaut, donc le stress test et son fichier
(chantier M2) ne changent pas.

**Règle de tenue** (permanente) : mettre à jour ce fichier à **chaque étape** — tableau ci-dessus, prochaine étape
exacte, entrée de journal — avant de passer à la suivante.

## Commandes utiles

Compilation, tests d'automatisation, traductions, `capture_screens.ps1` : voir `FrontEndInProgress.md` (« Commandes
utiles »). Toujours depuis **PowerShell** (Git Bash réécrit `/Game/...`), aucun éditeur du projet ouvert.

```powershell
# Lancer une session d'essai (deuxième écran, réglages de test) ; Jouer → Lancer
Start-Process "C:\UE\UE_5.8_AS_1\Engine\Binaries\Win64\UnrealEditor.exe" -ArgumentList '"C:\Users\baudo\Documents\MyCrusader\HeroesOfLiberty.uproject" -game -windowed -ResX=1600 -ResY=900 -WinX=3940 -WinY=100 -saveddirsuffix=Tests -nosplash'

# Bout en bout de la caméra (D257) : 2 et 8 joueurs, PASS / FAIL par cas, captures cam_*.png
powershell -ExecutionPolicy Bypass -File Tools\Tests\check_camera.ps1

# Stress test court sur le deuxième écran (S1 puis S2), rapport dans Saved_Tests\Logs\HeroesOfLiberty.log
& "C:\UE\UE_5.8_AS_1\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\baudo\Documents\MyCrusader\HeroesOfLiberty.uproject" /Game/Maps/StressTest `
  -game -windowed -ResX=1920 -ResY=1080 -WinX=3840 -WinY=0 -saveddirsuffix=Tests -nosplash `
  -ExecCmds="r.VSync 0,t.MaxFPS 0,scalability 2,hol.Units.SpawnBudget -1,hol.StressTest S1 2000 3 quit"

# Refaire les ressources (éditeur complet pour la carte, commandlet pour les touches)
#   UnrealEditor.exe <uproject> -nosplash -unattended -ExecCmds="py <projet>/Tools/Editor/create_match_map.py"   (supprimer Content/Maps/Plain.umap avant)
#   UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script=<projet>\Tools\Editor\make_game_input.py -unattended -nop4 -nosplash
```

Commandes de jeu pour les essais par script : `hol.Camera.Report [délai]`, `hol.Camera.TestCursor X Y` (curseur simulé
du défilement au bord, `off` = vrai curseur), `hol.Delay <secondes> <commande>`, `hol.UI.Key <touche> [délai] [durée]`
(`MouseScrollUp` / `MouseScrollDown` = un cran de molette), `hol.UI.NewGame.Launch <fichier JSON|-> [délai]`.

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

## Décisions du chantier (à reporter au § 21 du GDD)

| # | Date | Sujet | Décision |
|---|---|---|---|
| D253 | 2026-10-13 | Technique — Périmètre de J1 | **Caméra, emplacement de départ et réglages de la caméra** (choix de l'utilisateur, conforme à la recommandation) : le joueur arrive dans la partie à son emplacement de départ et déplace une caméra RTS ; dans Réglages > Commandes, **vitesse de la caméra** et **défilement au bord de l'écran** sont dégrisés et agissent ; les touches de la caméra passent par Enhanced Input (remappables plus tard). « Changer les touches » reste grisé : le remappage viendra quand il y aura assez d'actions, chaque étape déclarant les siennes (sélection en J2, groupes de contrôle…). Écartés : **caméra et emplacement seuls** (onglet entièrement grisé, valeurs fixes) et **remappage complet de D30 dès maintenant** (actions sans effet, à redessiner quand leurs systèmes arriveront). |
| D254 | 2026-10-13 | Technique — Comportement de la caméra | **Façon AoE4** (choix de l'utilisateur, conforme à la recommandation) : **pas de rotation**, plongée fixe (~55°), hauteur de jeu par défaut 30-40 m (la vue de jeu de D236), **zoom à la molette** entre ~15 et ~70 m ; déplacement aux **flèches** (ZQSD est pris par la grille de commandes de D30), au **bord de l'écran** et en **glissant au clic molette** ; caméra **bornée à la carte** ; au début de la partie, au-dessus de son **centre principal** (emplacement de départ en attendant J2). Valeurs indicatives, à régler en test. Motif : lisible et identique pour tous (compétitif, cartes symétriques, mini-carte toujours orientée pareil) ; le dézoom maximal reste sous la vue large mesurée (~100 m, D237), donc dans les performances déjà tenues. Écartés : **rotation façon BFME** (mini-carte qui suit l'orientation, formations à lire sous tous les angles) et **caméra cinématique** qui s'aplatit en zoomant (horizon visible, coût de rendu hors des mesures, lisibilité moindre). |
| D255 | 2026-10-13 | Technique — Fabrication de la caméra | **Caméra locale, sans pion** (choix de l'utilisateur, conforme à la recommandation) : le serveur ne crée aucun pion ; le contrôleur de partie `AHOLPlayerController` (C++, curseur visible) fait apparaître **chez le joueur seul** un acteur caméra (bras et caméra) qu'il pilote ; rien n'est envoyé au serveur quand la caméra bouge (ce que chacun reçoit dépendra du brouillard de guerre, M7, pas de sa caméra). Actions Enhanced Input de la partie (déplacement, zoom, glisser) dans un contexte `IMC_Game` créé par script, comme `IMC_UI` ; vitesse et défilement au bord enregistrés dans `UHOLGameUserSettings` ; le stress test place la caméra pour ses vues de mesure (D236, D237) par une fonction dédiée, hors des limites de jeu. Écartés : **pion caméra répliqué** (un pion par joueur sur le serveur, mouvements envoyés pour rien, à-coups possibles avec la latence) et **gestionnaire de caméra seul** (moins courant, sans composants réutilisables, plus dur à régler dans l'éditeur). |
| D256 | 2026-10-13 | Technique — Emplacements de départ et limites de la carte | **Acteurs posés dans la carte, sur une nouvelle carte** (choix de l'utilisateur, conforme à la recommandation ; l'utilisateur autorise l'assistant à créer des cartes) : un acteur **emplacement de départ** dérivé du `PlayerStart` d'Unreal porte son numéro (0 à 7, D244 : l'emplacement est la position de départ) ; un acteur **limites de la carte** borne la caméra (puis la mini-carte et la construction). Une **carte de partie neuve**, créée par script comme `MainMenu` : sol plat d'environ 400 m × 400 m couvert par le navmesh, lumière et ciel, 8 emplacements symétriques sur un cercle dans un ordre qui écarte les premiers (en 1 contre 1, face à face), les limites ; elle devient la carte proposée par « Nouvelle partie ». `BattleMap` (carte d'ouverture de l'éditeur) et `StressTest` ne changent pas. Les vraies cartes, faites à la main, placeront ces acteurs de même. Écartés : **emplacements calculés par le code** (ne tient pas pour des cartes dessinées avec leur terrain et leurs ressources), **`PlayerStart` étiquetés** (numéro tapé sans contrôle, pas de place pour des données propres à l'emplacement) et **modifier `BattleMap`**. |
| D257 | 2026-10-13 | Technique — Vérification de J1 | **Tests, bout en bout et stress tests** (choix de l'utilisateur, conforme à la recommandation) : tests d'automatisation sur la logique (caméra bornée aux limites, zoom entre ses bornes, emplacements symétriques et dans l'ordre, réglages vitesse et bord enregistrés puis relus) ; bout en bout par script sur le deuxième écran (partie lancée depuis le menu à 2 puis 8 joueurs, caméra au-dessus du bon emplacement, flèches et molette envoyées comme en D246, captures ; défilement au bord essayé avec une position de souris simulée dans le jeu, jamais en pilotant le bureau) ; S1 et S2 courts pour vérifier que les vues de mesure n'ont pas bougé ; puis une session d'essai de l'utilisateur. Écarté : essai manuel seul (rien de rejouable). |
| D259 | 2026-10-13 | Technique — Périmètre de J2 | **Centre principal et sélection au clic** (choix de l'utilisateur, conforme à la recommandation) : au début de la partie, un **centre principal** sur chaque emplacement **occupé** (joueur humain ou IA ; rien sur un emplacement fermé), à la **couleur de son joueur** (D244) ; il remplace le repère de l'emplacement, et les repères d'échelle de J1 partent. **Clic gauche** sur un bâtiment : il est sélectionné, le sien comme celui d'un adversaire (pour le regarder) ; **clic dans le vide** ou **Échap** : plus rien de sélectionné. Retour visuel : **cercle au sol** à la couleur du propriétaire sous le bâtiment sélectionné, **surlignage léger au survol**. Pas de panneau ni de bloc de sélection (J3, HUD de D201). Écartés : **bloc de sélection provisoire du HUD** (anticipe un HUD redessiné en J3) et **sélection par cadre et multiple** (sans objet sans unités, prévue en M4). |
| D260 | 2026-10-13 | Technique — Fabrication des bâtiments | **Acteur répliqué, valeurs en table, GAS plus tard** (choix de l'utilisateur, conforme à la recommandation ; les bâtiments sont des acteurs classiques, § 16.4) : une **classe C++ commune** à tous les bâtiments, **répliquée**, que le **serveur** fait apparaître au début de la partie sur chaque emplacement occupé (D259) ; son **propriétaire** est le numéro du joueur (un octet, comme `OwnerPlayer` des unités, D233) ; ses valeurs (nom, emprise, points de vie, maillage en référence souple) viennent d'une **table des types de bâtiment** fabriquée depuis un **CSV** par script, comme les unités (D221, D227). Le composant **GAS** et ses attributs viendront quand un bâtiment aura une capacité ou prendra des dégâts (M5). Écartés : **GAS dès J2** (code et réplication pour des valeurs que rien ne lit avant M5) et **un Blueprint par bâtiment** (valeurs dispersées au lieu d'une table à équilibrer). |
| D261 | 2026-10-13 | Technique — Apparence provisoire du centre principal | **Bloc simple coloré** (choix de l'utilisateur, conforme à la recommandation) : volumes de base du moteur, un bloc et une tour centrale, d'environ **15 × 15 m au sol et 10 m de haut** (valeurs de la table, D260), avec un **toit ou bandeau à la couleur du joueur**, tirée de la même palette que les unités (`HOLPlayerColors`, D233, D244). *Précisé à l'essai (remarques de l'utilisateur sur les captures)* : les pièces à la couleur du joueur sont **non éclairées** (la même couleur sur toutes les faces, sans côté sombre ; la pierre reste éclairée) ; la tour porte un **toit plat** un peu plus large qu'elle (un cône éclairé montrait un côté sombre, non éclairé il devenait un disque) ; le dessus du bloc est d'un **ton plus sombre** de la couleur du joueur, pour que le toit s'en détache. Lisible de loin, rapide, évidemment provisoire ; le vrai modèle se branchera par la table. Écartés : **assemblage des pièces de château de FantasyVillage** (plus long, pièces qui s'emboîtent mal, toujours provisoire) et **modèle à télécharger sur Fab** (dépend d'un choix et d'une licence). |
| D262 | 2026-10-13 | Technique — Sélection | **Chez le client seul, survol par teinte** (choix de l'utilisateur, conforme à la recommandation) : l'état de sélection vit dans le contrôleur du joueur ; le serveur ne le connaît pas, les futurs ordres lui porteront la liste visée par la file de commandes (comme AoE). On vise par un **rayon sous le curseur** sur un **canal de collision propre à la sélection**. Sélectionné : **cercle au sol** à la couleur du propriétaire ; survolé : le bâtiment **s'éclaircit** légèrement (paramètre du matériau, sans coût de rendu). Écartés : **contour en post-traitement** (passe de rendu en plus, à mesurer quand il s'étendra aux unités) et **sélection connue du serveur** (aller-retour réseau à chaque clic, sans usage tant que les alliés ne voient pas la sélection des autres). |
| D263 | 2026-10-13 | Technique — Angelscript | **Pas d'Angelscript : tout en C++** (choix de l'utilisateur, après avoir d'abord retenu un hybride puis demandé ce qu'il permettait) : le moteur du projet est celui d'Angelscript (Hazelight), mais le projet n'en écrit pas ; le gameplay reste en **C++** comme le cœur (D242, D255, D260), les valeurs en tables et les outils d'éditeur en Python. Le moteur du **serveur dédié** (D231) reste le build **Epic 5.8.3** en cours. Motif : un seul langage, aucun moteur supplémentaire à compiler pour le serveur, aucune dépendance au suivi des versions par Hazelight. Écartés : **hybride** (cœur C++, gameplay en script : rechargement à chaud, mais moteur du serveur à recompiler depuis les sources de Hazelight et deux langages) et **décider à M2.5**. |
| D264 | 2026-10-13 | Technique — Vérification de J2 | **Comme J1** (choix de l'utilisateur, conforme à la recommandation ; l'essai Angelscript prévu d'abord tombe avec D263) : **tests d'automatisation** (centres sur les seuls emplacements occupés, couleur de la palette du joueur, valeurs lues dans la table, rayon de sélection qui touche le centre) ; **bout en bout par script** à 2 et 8 joueurs (captures ; clic simulé sur son centre, sur un centre adverse, dans le vide, puis Échap) ; **essai de l'utilisateur**. S1 et S2 ne sont pas concernés (cartes sans emplacements, aucun centre). Écartés : **sans bout en bout** (sélection au clic non rejouable) et **essai de l'utilisateur seul**. Clôt le plan de J2 (D259 à D264). |

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
