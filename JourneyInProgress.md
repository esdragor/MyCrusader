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
| M2.5.3 Serveur : acteur partagé, tableau rapide, marquage selon D269 | à faire |
| M2.5.4 Client : entités d'affichage, horloge du serveur, prolongement et lissage, branchement de l'affichage | à faire |
| M2.5.5 S5 : script, rapport (bande passante, envois, écart affiché) | à faire |
| M2.5.6 Partie à 2 clients dans `quick_check.ps1`, essai de l'utilisateur | à faire |
| M2.5.7 Clôture : D268 à D273 au GDD | à faire |

## Prochaine étape exacte

**J2 close** (2026-10-14). Suite, dans l'ordre de D252 :
1. **M2.5.1 et M2.5.2 faites** (`net_match.ps1`, règles pures ; voir le journal). Suite : **M2.5.3** serveur : acteur
   partagé toujours pertinent, tableau rapide (`FFastArraySerializer` sous Iris) dont l'élément s'écrit sur les
   largeurs de `FHOLNetUnitState` (71 bits), marquage par `HOLUnitReplication::GetSendReason` à chaque pas, numéro du
   pas répliqué, réglages de `FHOLUnitSendRules` dans les réglages du projet. Puis M2.5.4 à M2.5.7, puis **J3** :
   panneau du bâtiment (il montrera la sélection, qui n'a plus de retour visuel) et production d'un Paysan.
2. ~~S1 et S2 au repos, nouvelle référence sans ombres ni post-traitement~~ : **fait** (2026-10-14, nouvelle référence
   sur 5.8.3 dans `ImplementationInProgress.md`).

**Règle de cohabitation avec le stress test** : seule une carte qui a des emplacements de départ reçoit la caméra RTS ;
une carte sans emplacement (`StressTest`, `BattleMap`) garde le pion par défaut, donc le stress test et son fichier
(chantier M2) ne changent pas.

**Règle de tenue** (permanente) : mettre à jour ce fichier à **chaque étape** — tableau ci-dessus, prochaine étape
exacte, entrée de journal — avant de passer à la suivante.

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

