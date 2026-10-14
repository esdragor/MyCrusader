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
| J4 — plan (Q1 à Q5, décisions à partir de D285) | **en cours** (2026-10-14) : Q1 tranchée (D285) ; plan validé (D286 à D289) |
| J4.1 Navmesh dynamique et obstacles | **en cours** (2026-10-14) |

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
