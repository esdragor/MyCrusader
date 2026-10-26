# Net playtest in progress

Journal du chantier **M6 en parallèle** : jouer à plusieurs (2 à 8) sur des PC différents, sur des **serveurs dédiés
qui tournent en permanence sur le PC de l'utilisateur**, choisis dans une **liste de salons** depuis le jeu.
Ouvert le 2026-10-23 (`main` 3765102) comme « partie entre amis par un lanceur », élargi à M6 par R-Q1 = C1.
Décisions nouvelles : à la suite de la review du GDD (prochain numéro libre au moment du report, D421 au 2026-10-23).
Chantier voisin : `LauncherInProgress.md` (téléchargement et mises à jour).

Règles du travail : boucles de 15 min au plus (un sous-agent au plus 300k tokens), test rouge d'abord sur la vraie
chaîne, compilation par `Tools\build.ps1`, `-LiveCoding=0`, essais discrets (`-nullrhi` ou `-RenderOffScreen`),
chaque essai demandé vient avec ce qu'il faut pour le lancer, ce fichier mis à jour à chaque étape, rien de commité
avant la validation de l'utilisateur. Worktree prévu : `HeroesOfLiberty-Glorfindel` (libre au 2026-10-23).

## Ce qui existe déjà (état au 2026-10-23)

- **Serveur dédié** (cible `HeroesOfLibertyServer`, D231) cuit et mis en place par `net_match.ps1 -Stage`
  (`Saved/StagedBuilds/WindowsServer/`, ~3 Go avec les symboles). Il lit la partie dans un JSON (`-MatchConfig=`,
  D243) et écoute sur le port **7777**.
- **Les clients rejoignent par adresse** (premier argument de la ligne de commande, comme `open <adresse>`). Le mode
  de jeu donne à chaque arrivant le **premier emplacement humain libre** et fait apparaître les centres de tous les
  emplacements occupés au démarrage.
- **Partie à 2 clients vérifiée** par `quick_check -Parts net` : production, récolte, construction, combat, victoire.
  Mais les clients sont **l'éditeur en `-game`** (contenu non cuit), sur **une seule machine** (`127.0.0.1`).
- Écran « Nouvelle partie » du solo (D240, D244, D266) : carte, 8 lignes (état, faction, couleur, équipe), mode et
  options ; remplit `FHOLMatchConfig` portée par `UHOLMatchSubsystem` (D243).
- `Plain` a **8 emplacements de départ**. Le lanceur de séquences de test (`hol.Test.Run`) existe hors éditeur, sauf
  en Shipping. Victoire V1 (abandon, effondrement, bandeau de fin) et V2 (forum à vies) dans `main`.

## Ce qui manque

1. **Aucun jeu empaqueté pour les joueurs.** Aucun réglage d'empaquetage dans `DefaultGame.ini` : toutes les cartes
   seraient cuites (dont `BattleMap` et ses gros packs), et **seul l'anglais** serait livré. **26 ressources sont
   chargées par leur chemin** dans le code (`/Game/UI/Input`, `/Game/UI/Fonts`, `/Game/Rendering`, `/Game/Audio`,
   `/Game/Input`, `/Game/Units/Props`…) : le cuiseur ne les voit pas.
2. **Pas de salon** : le serveur démarre directement la partie de son JSON et la partie commence au lancement ; il ne
   revient pas au salon après une partie.
3. **Pas d'écran Multijoueur** ni de liste des salons ; pas de contrôle de version à la connexion.
4. **Déconnexion** : l'emplacement se libère ; en revenant, le joueur reprend un emplacement libre mais ses
   **ressources repartent à celles du départ** (`SetResources` à chaque arrivée, ressources dans le `PlayerState`) ;
   rien côté client en cas de perte de connexion.
5. **Triche ouverte** : en Development, `hol.Cheat.*` et `hol.Victory.Defeat` sont acceptées par le serveur, de
   n'importe quel client.
6. **Le combat ignore les équipes** (bug d'`EtapeDefini.md`) : en 2v2, les alliés se battent.
7. **Jamais essayé** entre deux machines, ni au-delà de 2 clients, ni avec plusieurs serveurs sur un PC.

## Décisions prises (à reporter au § 21 du GDD à la clôture)

| Q | Décision |
|---|---|
| R-Q1 | **C1** : pas de lanceur provisoire ; le salon se fait maintenant, chantier M6 en parallèle |
| M6-Q1 | **Serveurs dédiés sur le PC de l'utilisateur, indépendants de son jeu** : tous les joueurs sont des clients ; la partie continue sans lui ; pas de « Héberger » dans le jeu |
| M6-Q2 | **Démarré avec Windows** (raccourci dans le dossier Démarrage) ; *revue par M6-Q6* : le raccourci lance le gestionnaire |
| M6-Q3 | **Chef de salon** = premier arrivé, transmis ; il règle carte, options, emplacements, et lance quand tous sont « Prêt » ; chacun sa ligne ; conflit de couleur : premier arrivé |
| M6-Q4 | **Écran « Nouvelle partie » en version réseau** : noms, « Prêt », couronne du chef, droits par ligne, « Lancer », « Quitter le salon » ; pas de discussion |
| M6-Q5 | **Liste des salons du PC**, adresse cachée (`servers.json` à côté du jeu), nom du joueur gardé, messages d'échec |
| M6-Q6 | **Gestionnaire** toujours allumé : un serveur de partie lancé à la demande, éteint quand vide, maximum réglable, serveur planté signalé |
| M6-Q6b | Gestionnaire en **C# (.NET 8)**, un seul `.exe`, **HTTP**, protocole versionné |
| M6-Q7 | Fin : **« Retour au salon »** ou **« Quitter vers la liste »** ; mêmes réglages, « Prêt » à zéro, chef au premier revenu ; ~~retour d'office après ~60 s~~ **revu le 2026-10-25 : aucun retour d'office**, on reste dans la partie finie aussi longtemps qu'on veut (carte, statistiques) |
| M6-Q8 | **Spectateur** : « Regarder » une partie en cours ; vision complète (D376), sans discussion pour l'instant |
| M6-Q9 | **Le salon vit dans le gestionnaire** ; un serveur Unreal ne fait qu'**une partie**, lancé avec `-MatchConfig=` (chemin de M2.5), **emplacement donné par le nom**, nom absent = spectateur |
| R-Q2 | **Tailscale** (partage du PC) ; installation guidée avant l'essai à deux PC |
| R-Q4 | **Équipes respectées par le combat** avant le premier essai |
| R-Q5 | **Fenêtre de D71 (~3 min) puis défaite** ; bandeau à compte à rebours ; retour sous le même nom |
| R-Q7 | **Development**, triches refusées par le serveur sauf `-HOLCheats` |
| R-Q8 | **Zip par version**, version affichée, refus d'une version différente ; lanceur en chantier à part |
| M6-Q10 | **Spectateur** (2026-10-24) : rien de caché, caméra libre, accès aux **statistiques de la partie** pendant la partie ; « Regarder » refusé une fois la partie finie ; serveur éteint quand tout le monde est parti, spectateurs compris (déjà le cas) ; à la fin, même bandeau que les joueurs plus les statistiques, **sans retour d'office** (corrigé le 2026-10-25 : pour les joueurs non plus) |
| M6-Q11 | **Statistiques « comme AoE »** : à la fin pour tous, pendant la partie pour les spectateurs ; tableaux chiffrés par joueur (ressources, unités produites / tuées / perdues, bâtiments, paliers…) et **courbes dans le temps** (villageois, armée, passages de palier…) |
| M6-Q12 | **Noms des joueurs** dans le tableau des joueurs en partie (au lieu de « Joueur N ») |
| M6-Q13 | **Perte de connexion détectée en 20 s** (au lieu de 60 s) ; le retour sous 3 min reste R7 |
| M6-Q14 | (2026-10-25) Après la fin, un joueur resté dans la partie **n'est plus dans le salon** ; un non-chef ne bloque pas la relance ; **le chef reste chef** même absent (précision du 2026-10-25) : nouveau chef (premier revenu) seulement si le salon est plein sans lui, ou (ajout de la session) s'il a quitté la partie sans revenir ou si le serveur s'est arrêté ; « Retour au salon » rejoint ce salon sous le même nom si possible, sinon (place prise, plein, disparu, relancé) liste Multijoueur et message |

## Architecture retenue

```
 PC de l'utilisateur (toujours allumé, joignable par Tailscale)
 ┌──────────────────────────────────────────────────────────────┐
 │ HOLManager.exe (C#, HTTP, démarré avec Windows)              │
 │   salons : joueurs, chef, lignes, « Prêt », état             │
 │   « Lancer » → écrit match.json (noms par emplacement)       │
 │              → lance HeroesOfLibertyServer.exe -Port=77xx    │
 │                 -MatchConfig=match.json -ManagerUrl=…        │
 │   suit chaque serveur (écoute, partie finie, vide, planté)   │
 └──────────────────────────────────────────────────────────────┘
        ▲ HTTP (liste, salon, 2 lectures/s)      ▲ HTTP (état du serveur)
        │                                         │
 Jeu des joueurs ── réseau Unreal (Iris) ──► serveur de la partie
 (Multijoueur → liste → salon → partie ?Name=…?Version=…)
```

- Le **salon** (règles, chef, couleurs) est dans le gestionnaire ; le jeu n'en est que l'affichage.
- Un **serveur neuf par partie** : rien à remettre à zéro, rien ne tourne pendant qu'on est au salon.
- Le contrat de D243 tient : le salon remplit `FHOLMatchConfig` (qui gagne le **nom** de chaque emplacement,
  version 2), le serveur la lit.
- Même **version** obligatoire, contrôlée deux fois : par le gestionnaire à l'entrée du salon, par le serveur à la
  connexion.

## Essais de l'utilisateur (points de validation)

| Essai | Quand | Ce que l'utilisateur vérifie |
|---|---|---|
| **E1** Jeu empaqueté en solo | fin de R1 | double clic sur `HeroesOfLiberty.exe` → écran titre en français → Jouer → `Plain` ; récolte, construction, combat comme dans l'éditeur ; anglais et japonais |
| **E2** Salon sur un PC | fin de R5 | gestionnaire lancé ; deux clients du paquet → Multijoueur → « Créer un salon » → l'autre le voit et le rejoint → couleurs, équipes, « Prêt », le chef lance → partie → fin → « Retour au salon » |
| **E3** Plusieurs salons, spectateur | fin de R5 | deux salons dans la liste, une partie dans chacun ; un troisième client « Regarde » une partie en cours |
| **E4** Départ, retour, triche, équipes | fin de R7 | client fermé et relancé dans les 3 min : il retrouve tout ; au-delà, il a perdu ; `hol.Cheat.Resources` refusé ; deux alliés ne se battent pas |
| **E5** Deux PC | fin de R8 | Tailscale installé avec mon aide ; un ami voit la liste et rejoint ; partie de 10 min sans saccade |
| **E6** Partie à X | après E5 | X amis, partie complète jusqu'à la victoire, retour au salon |
| **E7** Clôture | après E6 | décisions reportées au GDD relues, retours d'essai dans `EtapeDefini.md`, commit |

## Plan d'implémentation (figé le 2026-10-23)

Chaque ligne est une boucle de **moins de 15 min de travail** (le temps de machine d'un empaquetage, ~20 à 40 min la
première fois, tourne à part). Test rouge d'abord : côté C#, tests xUnit (`dotnet test`) ; côté jeu, tests
d'automatisation `HeroesOfLiberty.*` et parties de `quick_check`.

**Ordre et parallélisme** : R1 (paquet), R2 (gestionnaire, C# seul) et R6 (équipes) ne dépendent de rien et peuvent
partir en même temps, dans trois worktrees ; R3 après R2.2 (protocole) ; R4 après R2 et R3 ; R5 relie tout ; puis R7,
R8, R9.

### R1 Jeu empaqueté — finit par E1

| Étape | Contenu | Contrôle |
|---|---|---|
| R1.1 Script d'empaquetage | `Tools/Package/package.ps1` : `RunUAT BuildCookRun -platform=Win64 -clientconfig=Development -server -serverconfig=Development -build -cook -stage -pak -prereqs -archive`, client et serveur dans `Saved\Packages\<version>\`, journal `Saved_Tests\Logs\package.log`, file d'attente de `build.ps1` | UAT démarre, journal écrit (tâche de fond) |
| R1.2 Contrôle de fumée (rouge) | `Tools/Tests/packaged_smoke.ps1` : exe empaqueté `-RenderOffScreen`, séquence `waitmap MainMenu`, Jouer, `waitmap Plain`, capture, Paysan produit ; échoue sur `Failed to load` / `Couldn't find file` | **rouge attendu** |
| R1.3 Cartes cuites | `[/Script/UnrealEd.ProjectPackagingSettings]` `+MapsToCook` : `MainMenu`, `Plain` | journal du cuiseur ; taille notée |
| R1.4 Dossiers toujours cuits | `+DirectoriesToAlwaysCook` pour les dossiers des 26 chemins du code | R1.2 : plus de ressource manquante |
| R1.5 Langues | `+CulturesToStage` `fr`, `en`, `ja` | R1.2 en `-culture=ja` et `en` : captures relues |
| R1.6 Boucle jusqu'au vert | un défaut du journal par boucle | R1.2 **vert** |
| R1.7 Version | le script écrit `ProjectVersion` (`<date>-<commit court>`) avant la cuisson ; écran titre et journal l'affichent | capture, ligne `Version:` |
| R1.8 Essai E1 | dossier prêt, consigne en 3 lignes | **essai** |

### R2 Gestionnaire (C#) — vers E2

| Étape | Contenu | Contrôle |
|---|---|---|
| R2.1 Projets | `Manager/HOLManager` (.NET 8, API minimale) et `Manager/HOLManager.Tests` (xUnit) ; `Tools/Manager/build_manager.ps1` (`dotnet publish` en un seul `.exe` autonome) | `dotnet test` vert sur un test vide ; `.exe` produit |
| R2.2 Protocole | `Manager/PROTOCOL.md` : routes, JSON, version du protocole, erreurs (version différente, salon plein, nom pris, pas le chef) | relu ; sert à R3 et R4 |
| R2.3 État du gestionnaire | `GET /status` : version du jeu servie, version du protocole, nombre de salons | test d'intégration (`WebApplicationFactory`) |
| R2.4 Règles : salons et entrée | créer un salon (nom), entrer avec un nom, chef = premier, noms uniques, 8 au plus, version refusée ; tests | **rouge puis vert** |
| R2.5 Règles : lignes | chacun sa couleur (unique) et son équipe ; un changement remet « Prêt » à zéro ; le chef seul change carte, options, ouvre / ferme un emplacement ; tests | **rouge puis vert** |
| R2.6 Règles : départ et silence | « Quitter », ou 5 s sans lecture → retiré ; chef transmis au suivant ; salon vide supprimé ; horloge injectée ; tests | **rouge puis vert** |
| R2.7 Routes du salon | liste, créer, entrer, lire l'état, ma ligne, « Prêt », réglages du chef, quitter | tests d'intégration, un par route |
| R2.8 Configuration de partie | « Lancer » (chef, tous prêts) → `match.json` au format de `FHOLMatchConfig` version 2 (noms par emplacement) ; fichier de référence partagé avec le test C++ de R3.1 | test : JSON identique à la référence |
| R2.9 Lancer un serveur | port libre (7777 et suivants), `HeroesOfLibertyServer.exe -Port= -MatchConfig= -ManagerUrl= -MatchId=` ; faux serveur (petit programme de test) dans les tests | test : processus lancé avec les bons arguments |
| R2.10 Suivre un serveur | routes que le serveur appelle (écoute, partie lancée, partie finie, vide) ; processus disparu en pleine partie → salon remis « au salon » avec « Le serveur de la partie s'est arrêté » ; maximum de serveurs → refus | tests avec le faux serveur |
| R2.11 Retour au salon | après « partie finie », le salon accepte les retours (« Prêt » à zéro, chef au premier revenu) ; le serveur est éteint quand il est vide | tests |
| R2.12 Démarrage avec Windows | `Tools/Manager/install_startup.ps1` (raccourci dans le dossier Démarrage) et `uninstall_startup.ps1` ; journal du gestionnaire dans un fichier | lancé à la main : raccourci créé, gestionnaire qui répond à `/status` |

### R3 Serveur de partie (C++) — vers E2

| Étape | Contenu | Contrôle |
|---|---|---|
| R3.1 Configuration version 2 | `FHOLMatchSlot::PlayerName` ; un fichier de version 1 se lit encore ; lecture du fichier de référence de R2.8 | tests `HOLMatchConfig` **rouge puis vert** |
| R3.2 Emplacement par le nom | `PostLogin` lit `?Name=` ; l'emplacement dont c'est le nom ; sans nom dans la configuration (solo, tests) : premier libre comme aujourd'hui ; nom inconnu → **spectateur** (M6-Q8, `IsSpectator` de V1.3b) | tests sur la vraie chaîne à 2 clients |
| R3.3 Contrôle de version | `PreLogin` lit `?Version=` ; différente → refus avec message | test |
| R3.4 Triches : inventaire et test rouge | appels de triche (`RequestCheat*`, `ServerCheatDefeat`, scénarios, stress test) listés ici ; test : client de serveur dédié qui demande 1 000 bois | **rouge** |
| R3.5 Triches gardées | refus sur serveur dédié sauf `-HOLCheats` ; `net_match.ps1` et `quick_check` le passent | **vert** ; `-Parts tests,net` vert |
| R3.6 Arguments du gestionnaire | `-ManagerUrl=`, `-MatchId=` lus ; « écoute » envoyé au gestionnaire (module HTTP) | journal ; gestionnaire de test qui reçoit l'appel |
| R3.7 État envoyé | « partie lancée », « partie finie » (au `match over` de V1), « vide » | journal ; tests du gestionnaire de R2.10 contre le vrai serveur |
| R3.8 Fin du serveur | quitte quand il est vide après la partie, ou ~2 min après la fin | journal |
| R3.9 Départ ensemble : règle | la partie attend tous les noms de la configuration, ~60 s au plus ; les absents entrent alors dans la fenêtre de R-Q5 ; tests | **rouge puis vert** |
| R3.10 Départ ensemble : gel | simulation, ordres et production gelés pendant l'attente (garde unique, comme `IsRefused` de V1.2) ; bandeau « En attente de Bob » | journal, capture |

### R4 Écrans du jeu — vers E2

| Étape | Contenu | Contrôle |
|---|---|---|
| R4.1 Client du gestionnaire | `UHOLManagerClient` (sous-système de l'instance de jeu) : lit `servers.json`, appelle `/status` et la liste ; client seulement (module d'interface ou module de jeu côté client, D247) | test contre un gestionnaire lancé par le test |
| R4.2 Écran Multijoueur | bouton de l'écran titre → liste (nom, joueurs, « au salon » / « en partie (12 min) »), « Actualiser », « Créer un salon », « Rejoindre » / « Regarder », champ « Votre nom » gardé | capture hors écran |
| R4.3 Messages d'échec | gestionnaire éteint, version différente, salon plein, nom pris | un cas par message (gestionnaire de test) |
| R4.4 Salon : lecture | `UHOLNewGameScreen` en mode réseau : état lu 2 fois par seconde ; lignes des autres grisées | capture à 2 clients |
| R4.5 Salon : écriture | ma ligne, « Prêt », réglages du chef, « Lancer », « Quitter le salon » envoyés au gestionnaire | test : un non-chef ne voit pas « Lancer » |
| R4.6 Salon : colonnes | nom, « Prêt », couronne du chef | capture |
| R4.7 Vers la partie | salon « en partie » → connexion au port donné avec `?Name=…?Version=…` ; « Regarder » → spectateur | 2 clients dans la partie, chacun à son emplacement |
| R4.8 Fin de partie | bandeau de D362 : « Retour au salon » et « Quitter vers la liste » ; retour d'office après ~60 s | capture ; retour au salon vu dans le gestionnaire |
| R4.9 Erreurs en partie | perte de connexion, serveur arrêté → liste et message | serveur tué pendant un test |
| R4.10 Traductions (1) | Multijoueur et messages d'échec fr / en / ja | `.po` relus |
| R4.11 Traductions (2) | salon, bandeau de fin, attente, erreurs fr / en / ja | `.po` relus |

### R5 Bout à bout — finit par E2, E3

| Étape | Contenu | Contrôle |
|---|---|---|
| R5.1 Script (rouge) | `Tools/Tests/lobby_match.ps1` : gestionnaire + 2 clients pilotés par séquence : liste → créer → entrer → « Prêt » → lancer → partie | **rouge**, puis vert |
| R5.2 Fin et retour | même script : abandon d'un joueur → fin → « Retour au salon » des deux → salon intact | **vert** |
| R5.3 Spectateur | troisième client « Regarde » la partie en cours | **vert** |
| R5.4 Partie `lobby` de `quick_check` | R5.1 à R5.3 dans `quick_check` | **vert** |
| R5.5 Mesure | mémoire d'un serveur en partie ; deux batailles en même temps (S3 sur deux serveurs) : pas de chaque serveur | rapport ; maximum de serveurs fixé avec l'utilisateur |
| R5.6 Essais E2 et E3 | paquet, gestionnaire, consigne | **essais** |

### R6 Équipes au combat — vers E4 (indépendant, peut partir tout de suite)

| Étape | Contenu | Contrôle |
|---|---|---|
| R6.1 Règle pure | `IsEnemy(emplacement A, B, configuration)` (équipes du lancement, D357) ; tests | **rouge puis vert** |
| R6.2 Cibles du combat | recherche de cible et riposte : seulement les ennemis | cas `sides` : deux alliés côte à côte ne se battent pas |
| R6.3 Ordres | clic droit sur un allié = déplacement, pas attaque | test sur la vraie chaîne |
| R6.4 Bug retiré | ligne « Le combat ignore les équipes » retirée d'`EtapeDefini.md` | — |

### R7 Déconnexion (R-Q5) — finit par E4

| Étape | Contenu | Contrôle |
|---|---|---|
| R7.1 Règle pure | fenêtre (~3 min, réglable) ; le nom parti retrouve son emplacement dans la fenêtre ; au-delà, défaite ; tests | **rouge puis vert** |
| R7.2 État gardé | ressources et compteurs de l'emplacement gardés dans le `GameState` au départ, rendus au retour au lieu de `StartingResources` | test : 300 bois → départ → retour → 300 bois |
| R7.3 Défaite à la fin de la fenêtre | `DefeatSlot` (raison « déconnecté ») → effondrement de V1 | test |
| R7.4 Propriétaire réseau | au retour, bâtiments rendus au nouveau contrôleur (`AssignBuildingOwners`) ; file de production visible | test à 2 clients |
| R7.5 Bandeau | « Bob s'est déconnecté, 2:41 » à tous ; « Bob est revenu » | capture |
| R7.6 Rejoindre depuis la liste | un joueur parti voit sa partie « en partie » avec « Revenir » | test |
| R7.7 Traductions | bandeau et « Revenir » fr / en / ja | `.po` relus |
| R7.8 Cas `net:rejoin` de `quick_check` | client fermé puis relancé : même emplacement, mêmes ressources ; fenêtre réduite à 10 s : défaite | **vert** |
| R7.9 Essai E4 | — | **essai** |

### R8 Mise en place sur le PC et chez les amis — finit par E5

| Étape | Contenu | Contrôle |
|---|---|---|
| R8.1 Tailscale guidé (votre PC) | compte, installation, adresse du PC notée dans `servers.json` | guidé pas à pas avec l'utilisateur |
| R8.2 Tailscale guidé (partage) | partager le PC aux amis ; fiche pour eux | fiche relue |
| R8.3 Zip | `package.ps1 -Zip` : `HeroesOfLiberty-<version>.zip` sans `.pdb`, sans serveur ni gestionnaire pour les joueurs | taille notée |
| R8.4 Mise à jour du PC hôte | script : arrête le gestionnaire et les serveurs, remplace le dossier, relance | essai à la main |
| R8.5 Fiche « Jouer à HeroesOfLiberty » | `LISEZMOI.txt` : Tailscale, dézipper, SmartScreen (« Exécuter quand même »), prérequis Visual C++, Multijoueur, nom | relue |
| R8.6 Mesure à 4 et 8 clients | partie ordinaire et S5 à 4 puis 8 clients : envoi du serveur (débit montant de la box) | rapport |
| R8.7 Essai E5 | deux PC, Tailscale | **essai** |

### R9 Partie à X et clôture — E6, E7

| Étape | Contenu | Contrôle |
|---|---|---|
| R9.1 Essai E6 | zip à jour | **essai** |
| R9.2 Retours d'essai | une ligne par défaut dans `EtapeDefini.md` | — |
| R9.3 Décisions au GDD | toutes les décisions ci-dessus au § 21 ; § 14.4 (prototype jouable à plusieurs), § 15 (Multijoueur, salon), § 16.4 (serveurs sur le PC, gestionnaire), § 16.6 (M6) ; D243 et D268 précisées | relu par l'utilisateur |
| R9.4 Commit | parties touchées de `quick_check`, `dotnet test`, `EtapeDefini.md` dans le même commit | après validation |

## Risques connus

- **Contenu manquant dans le jeu cuit** : tout ce qui est chargé par chemin ou par référence souple non suivie. R1.2
  le cherche dans le journal ; à surveiller sur les animations (tables de données à références souples, D221).
- **Taille** : `Content` fait 3,4 Go avec les packs d'origine ; ne cuire que les cartes utiles. Le serveur seul fait
  ~3 Go avec les symboles : ni serveur ni symboles dans le zip des joueurs.
- **Plusieurs parties sur un PC** : le processeur pendant les batailles (S3 ~6 ms par pas pour une seule partie).
  R5.5 mesure avant de fixer le maximum.
- **Votre PC joue aussi** : quand vous jouez sur le PC qui héberge, votre jeu et les serveurs se partagent la machine.
- **Deux langages** : le protocole (R2.2) et le fichier de référence de la configuration (R2.8, R3.1) tiennent les deux
  côtés ensemble ; toute modification passe par eux.
- **Matériel des amis** : squelettes instanciés et rendu Unreal 5.8 ; un PC ancien peut ne pas suivre.
- **Débit montant de la box** : ~32 Ko/s par client mesurés à 2 000 unités (S5), multiplié par le nombre de joueurs
  de toutes les parties. R8.6 mesure.
- **SmartScreen et antivirus** : les exe ne sont pas signés. Dans la fiche.
- **Bugs réseau déjà vus une fois** (`EtapeDefini.md`, « Aléas ») : barre de vie sur une unité intacte, maison de
  l'autre arrivée en avance.

## Reprise exacte (pour une nouvelle session)

- **État au 2026-10-23 soir** : **R1 et R2 finis et dans `main` (7cacb4c)**, E1 validé, choix de `PROTOCOL.md`
  acceptés. Worktree `HeroesOfLiberty-Glorfindel`, branche `net-playtest` = `main` 7cacb4c, gardé pour la suite
  (`IN_USE.txt`). **Prochaine étape : R3** (serveur de partie C++) : R3.1 lit
  `Manager/HOLManager.Tests/Data/match_v2_reference.json` (champ `playerName`, version 2) ; R3.6-R3.7 parlent aux
  routes `/matches/{id}/...` de `Manager/PROTOCOL.md`. **R6 est mené par une autre session** (EtapeDefini G, combat
  entre alliés, worktree Elrond, mycrusader-15) : ne pas le refaire.
- **Découpage de R3 en boucles** (une boucle = un sous-agent ≤ 15 min, minuteur lancé avec lui ; après chaque
  boucle, la session principale note ici et dans « Suivi » ce qui est fait et la boucle suivante, pour qu'une autre
  session reprenne) : **B1** R3.1 (config version 2) + R3.3 (contrôle de version à la connexion) ; **B2** R3.2
  (emplacement par le nom, spectateur, test à 2 clients) ; **B3** R3.4 + R3.5 (triches) ; **B4** R3.6 + R3.7 (appels
  HTTP au gestionnaire) ; **B5** R3.8 (fin du serveur) ; **B6** R3.9 + R3.10 (départ ensemble). Puis commit de R3
  (accord des sessions actives, `EtapeDefini.md` dans le même commit), puis R4.
- **Au 2026-10-24 : R1, R2, R3 dans `main` (174a0f4).** Glorfindel, branche `net-playtest` = `main` 174a0f4, gardé
  pour R4. **Découpage de R4 en boucles** (même règle que R3 : un sous-agent ≤ 15 min, minuteur, état noté dans
  « Suivi » après chaque boucle) : **B1** R4.1 client du gestionnaire (`UHOLManagerClient`, `servers.json`, `/status`
  et liste ; test contre le gestionnaire publié par `Tools\Manager\build_manager.ps1`) ; **B2** R4.2 écran
  Multijoueur (liste, Actualiser, Créer, Rejoindre / Regarder, nom gardé ; capture hors écran) ; **B3** R4.3 messages
  d'échec ; **B4** R4.4 salon en lecture (2 lectures/s) ; **B5** R4.5 + R4.6 salon en écriture et colonnes ; **B6**
  R4.7 vers la partie (`?Name=…`, port donné, « Regarder ») ; **B7** R4.8 fin de partie (Retour au salon / Quitter
  vers la liste, ~60 s) ; **B8** R4.9 erreurs en partie ; **B9** R4.10 + R4.11 traductions. Puis R5 (bout à bout,
  essais E2 et E3).
- **Au 2026-10-24 soir : R4 fini, non commité** (Glorfindel). R5 découpé : **B1** R5.1 + R5.2 (`lobby_match.ps1`, deux clients) ; **B2** R5.3 (spectateur) ; **B3** R5.4 (partie `lobby` de `quick_check`) ; **B4** R5.5 (mesure) ; puis E2 et E3 (R5.6).
- **Boucle en cours** : voir la dernière ligne de « Suivi ». Si elle dit « lancée » sans « rendue », la
  boucle a été coupée : regarder `git -C C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel status` et `diff`,
  relancer les tests de la boucle, et la reprendre.
- Outils : `Tools\Package\package.ps1`,
  `Tools\Tests\packaged_smoke.ps1 [-Culture]`, `Tools\Manager\build_manager.ps1 [-Test]`,
  `install_startup.ps1 / uninstall_startup.ps1 [-StartupDir]`. Un sous-agent n'attend jamais un empaquetage.
- État initial (2026-10-23 matin) : **toutes les questions tranchées, plan figé, rien codé.** Ce fichier, `LauncherInProgress.md`
  et la section « Réseau » d'`EtapeDefini.md` sont **dans la copie principale, non commités** : un worktree ne les
  voit qu'après un commit de doc (ou par leur chemin absolu `C:\Users\baudo\Documents\MyCrusader\...`).
- À démarrer en premier, en parallèle : **R1** (paquet) et **R2** (gestionnaire C#, sans compilation Unreal), par
  exemple dans `HeroesOfLiberty-Glorfindel` (libre au 2026-10-23, voir son `IN_USE.txt`) ; **R6** (équipes) dans un
  autre worktree libre.
- Avant de coder : écrire `IN_USE.txt` du worktree, fusionner `main`, première compilation par `Tools\build.ps1`.
- Outils : SDK .NET 8.0.314 et 9.0.310 installés ; moteur `UE 5.8 AS 3` avec la cible Server (D231).
- Chaque étape : test rouge d'abord, une boucle de 15 min au plus, ce fichier mis à jour, rien commité avant
  l'accord de l'utilisateur.

## Suivi

- 2026-10-23 : état des lieux fait, plan proposé à l'utilisateur (décisions R-Q1 à R-Q8 à trancher). Rien codé.
- 2026-10-23 : **R-Q1 = C, précisé C1** (choix de l'utilisateur) : pas de lanceur `.bat` provisoire, le **salon se
  fait maintenant**, dans un chantier **M6 mené en parallèle** (worktree Glorfindel, libre). R-Q3 et R-Q6 tombent
  dans le salon ; R-Q2, R-Q4, R-Q5, R-Q7, R-Q8 restent. Plan à refaire après les questions du salon (M6-Q1 posée :
  qui fait tourner le serveur).
- 2026-10-23 : **M6-Q1 = A, nuancée** (choix de l'utilisateur) : **serveur dédié sur le PC de l'utilisateur,
  indépendant de son jeu** (« A, B, C jouent, j'héberge ; B et C jouent sans moi, tant que mon PC est allumé, c'est
  bon »). Conséquences : le salon vit sur le serveur (pas d'hôte parmi les joueurs), le serveur revient au salon après
  une partie, pas de « Héberger » dans le jeu (seulement Multijoueur → Rejoindre), même version obligatoire pour le
  serveur et les joueurs (refus clair sinon). M6-Q2 posée : comment le serveur démarre et reste allumé.
- 2026-10-23 : **M6-Q2 = B d'abord, C si un plantage arrive pendant les essais** (choix de l'utilisateur) : raccourci
  « Serveur HeroesOfLiberty » dans le dossier Démarrage de Windows (le serveur démarre avec le PC) ; relance
  automatique (script de surveillance, relances limitées) seulement si un plantage survient aux essais. M6-Q3 posée :
  qui règle la partie dans le salon.
- 2026-10-23 : **M6-Q3 = B** (choix de l'utilisateur, conforme à la recommandation) : **chef de salon** = premier
  arrivé, transmis au suivant s'il part ; le chef choisit carte, options, emplacements ouverts / fermés et lance quand
  tous sont « Prêt » ; chacun choisit sa ligne (couleur, équipe, plus tard faction) ; une ligne changée remet son
  « Prêt » à zéro ; conflit (même couleur) tranché par le serveur, premier arrivé. R-Q2 posée : comment les amis
  atteignent le PC.
- 2026-10-23 : **R-Q2 = A** (choix de l'utilisateur, conforme à la recommandation) : **VPN de jeu, Tailscale avec
  partage du PC** ; rien d'ouvert sur Internet. L'utilisateur ne connaît pas l'outil : **étape d'installation guidée**
  à prévoir avant l'essai à deux PC (compte, installation, partage aux amis, fiche à leur envoyer). R-Q4 posée :
  équipes au premier essai.
- 2026-10-23 : **R-Q4 = B** (choix de l'utilisateur, conforme à la recommandation) : **le combat respecte les équipes
  avant le premier essai** (R7 fait avant E4) ; colonne « Équipe » du salon active d'emblée. R-Q5 posée : déconnexion
  en pleine partie.
- 2026-10-23 : **R-Q5 = B** (choix de l'utilisateur, conforme à la recommandation) : **fenêtre de D71 (~3 min
  indicatives), puis défaite** comme un abandon (effondrement de V1) ; bandeau à tous avec le compte à rebours
  (« Bob s'est déconnecté, 2:41 ») ; retour sous le même nom (saisi sur l'écran « Rejoindre », gardé) dans la fenêtre :
  il reprend emplacement, bâtiments, unités, ressources ; quand l'IA existera, elle prendra la place en équipe (D71).
  R-Q7 posée : version livrée et triches.
- 2026-10-23 : **R-Q7 = A** (choix de l'utilisateur, conforme à la recommandation) : **Development**, le **serveur
  refuse les triches** (`hol.Cheat.*`, `hol.Victory.Defeat`, scénarios, stress test) sauf lancé avec `-HOLCheats`
  (scripts de test) ; console laissée ouverte. R-Q8 posée : distribution et mises à jour.
- 2026-10-23 : **R-Q8 = A** (choix de l'utilisateur) : **un zip par version** sur un partage, version affichée sur
  l'écran titre et dans le journal, refus clair d'une version différente par le serveur ; script de mise à jour du
  serveur (remplace le dossier, relance). Nuance : un **lanceur** (téléchargement et mises à jour automatiques) est
  ouvert comme **chantier à part, en parallèle** : `LauncherInProgress.md`. Restent les questions du salon (M6-Q4 et
  suivantes), puis le plan refait.
- 2026-10-23 : **M6-Q4 = A** (choix de l'utilisateur, conforme à la recommandation) : le salon est **l'écran
  « Nouvelle partie » en version réseau** : nom du joueur sur sa ligne, colonne « Prêt », chef marqué d'une couronne ;
  chacun ne change que sa ligne, le chef la carte, les options et l'ouverture des emplacements ; « Lancer » au chef
  seul, actif quand tous sont prêts ; « Quitter le salon ». Pas de discussion de salon (avec les canaux de D385).
  M6-Q5 posée : écran « Rejoindre ».
- 2026-10-23 : **M6-Q5 = liste des parties** (choix de l'utilisateur : « je veux que mon PC puisse héberger autant de
  parties que possible, donc une liste de serveurs ; les infos de mon PC ne changent pas, elles sont cachées ») :
  Multijoueur → **liste des salons du PC de l'utilisateur** (nom, joueurs, « au salon » / « en partie »), **adresse
  jamais affichée**, lue dans un petit fichier à côté du jeu (que le lanceur pourra tenir à jour) ; nom du joueur
  demandé une fois et gardé ; messages d'échec (serveur éteint, version différente, salon plein, nom pris). Pas de
  secret : l'adresse se lit dans le dossier, Tailscale protège. **Conséquence** : un serveur Unreal = une partie, donc
  **plusieurs serveurs** sur le PC (un port chacun) ; revient sur M6-Q2 (le raccourci de démarrage lance ce qui gère
  les serveurs). M6-Q6 posée : salons fixes ou gestionnaire.
- 2026-10-23 : l'utilisateur est absent (« j'afk ») ; plan refait avec les décisions prises (R1 à R9, essais E1 à
  E7), questions restantes M6-Q6 (posée) à M6-Q9 listées. Rien codé, rien commité.
- 2026-10-23 : **M6-Q6 = B** (choix de l'utilisateur) : **gestionnaire** toujours allumé sur le PC, lancé par le
  raccourci du dossier Démarrage (revoit M6-Q2) : lance un serveur par salon à la demande (« Créer un salon »), port
  libre, éteint les serveurs vides, refuse au-delà d'un maximum (fixé après la mesure de R5.3), relance un serveur
  planté (couvre le C de M6-Q2) ; la liste du jeu l'interroge. R5 du plan à refaire (plus de salons fixes). M6-Q6b
  posée : en quoi écrire le gestionnaire (lien avec L-Q3 du lanceur).
- 2026-10-23 : **M6-Q6b = B** (choix de l'utilisateur, conforme à la recommandation) : gestionnaire en **C# (.NET)**,
  un seul `.exe` sans installation, répond au jeu en **HTTP** (liste, créer un salon ; module HTTP d'Unreal côté
  jeu), protocole versionné. Piste notée pour L-Q3 du lanceur (même outil). M6-Q7 posée : fin de partie.
- 2026-10-23 : **M6-Q7 = C** (choix de l'utilisateur, conforme à la recommandation) : bandeau de fin (D362) avec
  **« Retour au salon »** et **« Quitter vers la liste »** ; ceux qui reviennent retrouvent les mêmes réglages,
  « Prêt » remis à zéro ; chef au premier revenu si l'ancien est parti ; ceux qui n'ont rien choisi sont ramenés au
  salon après ~60 s (indicatif) ; « Continuer à regarder » reste pour un vaincu en pleine partie ; le serveur se remet
  à zéro entre deux parties. M6-Q8 posée : arrivée pendant une partie.
- 2026-10-23 : **M6-Q8 = B** (choix de l'utilisateur, contre la recommandation C) : **spectateur dès maintenant** :
  dans la liste, « Regarder » pour un salon en partie ; il entre sans emplacement, ne commande rien, joue la suivante
  depuis le salon (M6-Q7). Vision : complète, comme le spectateur de D376 en chacun pour soi (pas encore de
  brouillard ; à revoir avec M7) ; sa discussion attend les canaux de D385. M6-Q9 posée : où vit le salon.
- 2026-10-23 : **M6-Q9 = C** (choix de l'utilisateur, conforme à la recommandation) : **le salon vit dans le
  gestionnaire** (HTTP) ; un serveur Unreal ne fait qu'une partie, lancé neuf avec `-MatchConfig=` ; emplacement
  donné par le nom (`FHOLMatchConfig` version 2) ; nom absent = spectateur. Toutes les questions tranchées ; **plan
  figé** (architecture, R1 à R9, essais E1 à E7). SDK .NET 8 et 9 présents sur la machine. Rien codé, rien commité.
- 2026-10-23 : **R1 et R2 démarrés** dans `HeroesOfLiberty-Glorfindel`, branche `net-playtest` (depuis `main` f7269a8),
  `IN_USE.txt` écrit, première compilation de l'éditeur verte (44 s). Deux sous-agents en parallèle : R1 boucle 1
  (R1.1 `package.ps1`, R1.2 `packaged_smoke.ps1`, R1.3 cartes cuites, premier empaquetage lancé détaché ; R1.4 et
  R1.5 gardées pour après le rouge de R1.2) et R2 boucle 1 (R2.1 projets C#, R2.2 `PROTOCOL.md`, R2.3 si le temps).
- 2026-10-23 : **R1 boucle 1 rendue.** `Tools/Package/package.ps1` (R1.1 : file d'attente derrière tout UBT du moteur,
  BUSY si Unreal tourne dans la copie, journal `Saved_Tests\Logs\package.log`, archive
  `Saved\Packages\<aaaa-mm-jj>-<commit>`), `Tools/Tests/packaged_smoke.ps1` (R1.2 : exe hors écran, séquence titre →
  NewGame → `Plain` → Paysan produit, captures, échec sur ressource manquante / erreur fatale / étape manquée ; pas
  encore lancé), `DefaultGame.ini` : `+MapsToCook` `MainMenu` et `Plain` (R1.3). **Premier empaquetage lancé détaché**
  (`2026-10-23-f7269a8`). Pour R1.4 : 20 chemins `/Game/...` dans le code, dossiers `/Game/UI/Input`, `/Game/UI/Fonts`,
  `/Game/Rendering`, `/Game/Audio`, `/Game/Input` (tout le dossier : noms `IA_*` construits), `/Game/Units/Props` ; à
  surveiller : tables de `DefaultGame.ini` (`/Game/Units`, `/Game/Buildings`, `/Game/Resources`), références souples
  des animations (D221), `ServerDefaultMap=/Engine/Maps/Entry` non cuite. Doute : dossier `Saved` du jeu empaqueté,
  `-ExecCmds="hol.Test.Run"` jamais essayé hors éditeur.
- 2026-10-23 : **R2 boucle 1 rendue (~13 min) : R2.1, R2.2, R2.3 faits.** `Manager/` : `HOLManager` (.NET 8, API
  minimale, port 7700), `HOLManager.Tests` (xUnit, `WebApplicationFactory`), `global.json` (SDK 8.0.314),
  `Tools/Manager/build_manager.ps1` (un `.exe` autonome de 91,8 Mo dans `Saved\Manager\`, `-Test`). `PROTOCOL.md`
  (protocole 1, JSON camelCase, jeton `X-Player-Token`, 19 codes d'erreur, routes du salon, routes du serveur
  `/matches/{id}/listening|started|finished|empty` depuis 127.0.0.1 seulement, `match.json` version 2 avec exemple).
  R2.3 `GET /status` : **rouge** (2 échecs, 404) puis **vert** (3/3), vérifié aussi contre le `.exe`. Choix ouverts
  listés en tête de `PROTOCOL.md` (ports 7700 / 7777+, `version.txt`, emplacement fixe, « Prêt » remis à zéro, pas
  d'IA, noms réservés 2 min, noms 1-16 / 1-24, état `starting`, 2 joueurs au moins, `MaxServers` = 2 jusqu'à R5.5) :
  soumis à l'utilisateur. R2 boucle 2 lancée : R2.4 (salons et entrée), R2.5 si le temps.
- 2026-10-23 : **R2 boucle 2 rendue (~5 min) : R2.4 et R2.5 faits.** `Manager/HOLManager/Lobbies/` : `LobbyRules.cs`
  (toutes les valeurs en un endroit : 8 emplacements, 8 couleurs, équipes 0-4, noms 16/24, cartes), `Lobby.cs`
  (modèle, `IClock`, codes d'erreur), `LobbyStore.cs` (règles pures, un verrou, refus sans effet) ; `/status` compte
  les salons. Tests `LobbyEntryTests` et `LobbyLineTests` : **rouge 68 échecs / 1** (méthodes en
  `NotImplementedException`) puis **vert 69/69**. `PROTOCOL.md` complété : noms nettoyés (espaces, forme composée),
  ordre des contrôles d'entrée, codes manquants (`bad_request`, `lobby_not_found`, `not_in_lobby`, `lobby_in_game`,
  `slot_taken`), renvoyer la même valeur ne remet pas « Prêt » à zéro, carte plus petite → emplacements fermés.
  Chef calculé (le plus ancien arrivé encore là). R2 boucle 3 lancée : R2.6 (départ, silence), R2.7 (routes) si le
  temps.
- 2026-10-23 : **premier empaquetage PASS en 13,5 min** (`Saved\Packages\2026-10-23-f7269a8`) : client 3,2 Go dont
  **2,6 Go de `.pdb`** et 332 Mo d'exe ; contenu cuit ~150 Mo (`MainMenu` + `Plain`) ; serveur 3,0 Go. Contrôle de
  fumée R1.2 lancé (rouge attendu).
- 2026-10-23 : **R2 boucle 3 rendue : R2.6 et R2.7 faits.** R2.6 : quitter ou 5 s de silence → retiré (emplacement
  et couleur libérés), seulement au salon ; chef au plus ancien arrivé, avis `leader_changed` 10 s ; salon vide
  supprimé (gardé s'il est en partie) ; ménage au début de chaque appel du magasin, sous le verrou, sans minuteur ;
  tout appel avec jeton vaut présence. **Rouge 15 / vert 85/85.** R2.7 : `Api/LobbyRoutes.cs` (routes, table codes →
  statuts HTTP, `X-Player-Token`), `Api/LobbyViews.cs` (JSON construit sous le verrou) ; `start` et `watch` en
  `not_implemented` (501) jusqu'à R2.8-R2.10. **Rouge 19 / vert 104/104**, 0 avertissement. `PROTOCOL.md` : JSON
  illégal = `bad_request` d'abord, erreurs de `leave`, index d'emplacement. **À retenir pour R2.10-R2.11** : au retour
  au salon après une partie, remettre `LastSeen` à l'heure, sinon tous retirés au premier appel. R2 boucle 4 lancée :
  R2.8, R2.9.
- 2026-10-23 : **R1.2 rouge comme attendu** (40 s, 47 lignes de contenu manquant, 6 erreurs d'interface / partie) :
  `Failed to find object` sur `/Game/UI/Input/IMC_UI`, `IA_UI_*`, `/Game/Input/IA_Camera_*`, `IMC_Game`,
  `/Game/Rendering/M_GroundRing` ; `LogHOLUI` / `LogHOLMatch` Error « keys missing ». Faux positifs à exclure :
  `LogWindows: Failed to load 'aqProf.dll' / VtuneApi / WinPixGpuCapturer` (sondes de profileurs). R1 boucle 2
  lancée : R1.4, R1.5, filtre, nouvel empaquetage.
- 2026-10-23 : **R1 boucle 2 rendue (~4 min).** Le premier paquet allait **au bout de la séquence** (titre, NewGame,
  `Plain`, Paysan, sortie propre en 4,8 s) : le rouge ne venait que du contenu manquant. Déjà cuits : tables
  (`UnitTypes`, `BuildingTypes`, `SourceTypes`), 24 animations du Mannequin (références souples suivies), polices.
  `DefaultGame.ini` : `+DirectoriesToAlwaysCook` `/Game/Input`, `/Game/UI/Input`, `/Game/UI/Fonts`, `/Game/Rendering`,
  `/Game/Audio`, `/Game/Units/Props` (R1.4) ; `InternationalizationPreset=EFIGSCJK`, `+CulturesToStage` `fr`, `en`, `ja`
  (R1.5 ; les `.locres` sont déjà dans `Content\Localization\Game\`). Filtre : sondes `LogWindows: Failed to load
  '<profileur>.dll'` exclues, `LoadPackage: SkipPackage` ajouté. `package.ps1` vide l'archive de même version avant
  UAT. Deuxième empaquetage lancé. Doute : `DefaultCulture=en` du moteur ; sans `-culture`, le jeu prend la langue de
  Windows (à voir à E1).
- 2026-10-23 : **R2 boucle 4 rendue (~5 min) : R2.8 fait, R2.9 non commencé.** `Matches/MatchConfig.cs`
  (`FromLobby` : joueurs `Human` avec leur nom, vides ou fermés `Closed`), `Matches/IServerLauncher.cs` (interface
  seule), référence partagée `Manager/HOLManager.Tests/Data/match_v2_reference.json` (Bob 0, 1 fermé, Alice 2, Chloé 3 ;
  carte `/Game/Maps/Plain.Plain`), `POST /start` → 202 `{matchId}`, salon `starting`. Champ JSON **`playerName`**
  (`FJsonObjectConverter` met la première lettre en minuscule) : R3.1 ajoute `FString PlayerName` en dernier dans
  `FHOLMatchSlot`, version 2. **Rouge 11 / vert 114/114.** `PROTOCOL.md` : erreurs de `start` dans l'ordre, format du
  `matchId`, `-useperfthreads`, guillemets **après le `=`** (`FParse::Value`), port UDP libre dès 7777. R2 boucle 5
  lancée : R2.9.
- 2026-10-23 : **deuxième empaquetage PASS en 1 min** (cuisson incrémentale ; client 3 186 Mo, serveur 3 023 Mo).
  **R1.2 vert en `fr`, `en` et `ja`** (7 à 9 s, 4 captures chacun, `Saved_Tests\Screenshots\Smoke_<culture>`,
  journaux `Saved_Tests\Logs\packaged_smoke_<culture>.log`) : captures relues, textes dans la bonne langue (écran
  titre anglais, Nouvelle partie japonaise, HUD français avec Paysan produit). **R1.6 atteint.** Bouton « Triches »
  visible dans le jeu empaqueté (Development, attendu ; R3.4-R3.5 côté serveur). R1 boucle 3 lancée : R1.7 (version).
- 2026-10-23 : **R2 boucle 5 rendue : R2.9 fait.** `Matches/ProcessServerLauncher.cs` (port UDP libre dès
  `FirstServerPort`, 100 essais ; `match.json` dans `<MatchesDir>\<matchId>\` ; lancement sans console, journal
  `-abslog` ; `max_servers`, `server_start_failed` ; processus mort hors de `Running` ; `Dispose` arrête tout),
  `FakeServer/` (écrit sa ligne de commande, se ferme seul ≤ 60 s), réglages `FirstServerPort`, `MatchesDir`,
  `ManagerUrl` ; `/status.servers` = serveurs en marche. Ligne : `<exe> /Game/Maps/Plain -Port=7778
  -MatchConfig="C:\a b\...\match.json" -ManagerUrl=http://127.0.0.1:7700 -MatchId=M... -log -unattended
  -useperfthreads -abslog="...\server.log"`. **Rouge 14 / vert 124/124**, aucun faux serveur restant. R2 boucle 6
  lancée : R2.10.
- 2026-10-23 : **R2 boucle 6 rendue : R2.10 fait.** Routes du serveur `listening` (salon → `in_game`, `match` rendu
  avec port et nom), `started` (minutes de partie), `finished`, `empty` (depuis 127.0.0.1 seulement, `not_local`,
  `match_not_found`) ; `watch` rend le port sans nom. Surveillance dans `LobbyStore.FollowServers`, sous le verrou, au
  début de chaque appel et chaque seconde (`Matches/HousekeepingService.cs`) : serveur disparu → salon au salon, avis
  `server_stopped` ; 120 s sans `listening` → arrêté ; arrêté 10 s après `empty`. Retour au salon : « Prêt » à zéro,
  `LastSeen` à l'heure. **Rouge 23 / vert 147/147** (3 fois). Reste pour R2.11 : noms réservés 2 min, retour par
  `join` avec le même nom, chef au premier revenu. R2 boucle 7 lancée : R2.11, R2.12.
- 2026-10-23 : **R2 boucle 7 rendue (~10 min) : R2.11 et R2.12 faits, R2 fini.** Retour au salon (après `finished`
  ou `server_stopped`) : chacun garde sa ligne, réglages gardés, « Prêt » à zéro ; **2 min** sans retrait par le
  silence, nom réservé ; `join` avec un nom réservé reprend la ligne (nouveau jeton, même salon plein) ; chef = ancien
  chef s'il est revenu, sinon le premier revenu ; nouveaux venus sur un emplacement libre ; `leave` rend la place.
  **Rouge 7 / vert 158/158** (un test ancien réécrit : il codait l'ancienne règle des 5 s). R2.12 :
  `Tools/Manager/install_startup.ps1` et `uninstall_startup.ps1` (`-ManagerDir`, `-StartupDir`), raccourci réduit,
  dossier de travail = celui de l'exe ; journal `Logs\manager-<date>.log` (14 jours, `LogDir`) ; essayés sur un
  dossier temporaire seulement (cible relue, `/status` répond, journal écrit). Bug trouvé et corrigé : fermer la
  fenêtre du gestionnaire le tuait sans arrêter ses serveurs ; il s'arrête proprement maintenant. Reste : installer le
  raccourci pour de bon (l'utilisateur, plus tard) ; éventuellement journaliser les événements des salons.
- 2026-10-23 : **R1 boucle 3 arrêtée à 15 min** (minuteur). Laissé : R1.7 codé, compilé, empaqueté (UAT PASS en 7 min).
  Solution : `package.ps1` écrit `<archive>\<plateforme>\HeroesOfLiberty\Version.txt` **après** UAT (aucun fichier
  suivi touché, une version nouvelle ne recompile rien) ; `Source/HeroesOfLiberty/HOLVersion.h/.cpp`
  (`HOLVersion::Get()` : `dev` hors paquet, sinon la première ligne de `Version.txt`, `unknown` sans fichier) ; ligne
  `Version: <v>` au démarrage du module (jeu et serveur) ; version en bas à droite de l'écran titre ; test
  `Tests/HOLVersionTests.cpp` ; `packaged_smoke.ps1` vérifie la ligne. **Cause du dépassement** : `package.ps1`
  restait bloqué après le succès d'UAT, car `Start-Process -Wait` attend aussi les **démons lancés par la cuisson**
  (`zenserver`, `UnrealTraceServer`, restés allumés ; aux deux premiers paquets ils tournaient déjà), et l'agent
  l'attendait. Corrigé par la session principale (`WaitForExit()` sur UAT seul) ; empaquetage relancé.
  **Amélioration du processus** : un sous-agent ne doit jamais attendre un empaquetage ; il le lance détaché et rend
  la main, la session principale le surveille.
- 2026-10-23 : **R1.7 fait.** Empaquetage relancé avec le correctif : PASS en 1 min, rend la main, `Version.txt` =
  `2026-10-23-f7269a8` dans `Windows\` et `WindowsServer\`. Contrôle de fumée : **rouge** sans `Version.txt` (« the game
  says 'Version: unknown' »), **vert** avec (`LogHOLNet: Display: Version: 2026-10-23-f7269a8`) ; capture relue :
  version en petit en bas à droite de l'écran titre. `quick_check` par défaut PASS en 22 s (tests 133/133 dont
  `HeroesOfLiberty.Version`, smoke). **R1.8 : essai E1 proposé à l'utilisateur.**
- 2026-10-23 : **E1 validé par l'utilisateur** (« E1 fonctionne ») : **R1 fini.** **Choix ouverts de `PROTOCOL.md`
  acceptés tels quels** (« ok ») : ports 7700 / 7777+, `version.txt`, emplacement fixe dans le salon, « Prêt » remis à
  zéro, chef « Prêt » aussi, 2 joueurs au moins, pas d'IA, noms réservés 2 min, noms 1-16 / 1-24, état `starting`,
  `MaxServers` = 2 jusqu'à R5.5. Commit de R1 et R2 préparé : `main` (e26ede1) fusionné dans `net-playtest`.
- 2026-10-23 : avant le commit, **deux tests C# fragiles** (2 échecs sur 5 lancements :
  `ServerGoneShowsTheNoticeToThePlayers`, `LaunchStartsTheServerWithTheseArguments`). Cause : `commandline.txt` du
  faux serveur lu une seule fois dès qu'il existe, alors qu'un autre processus (antivirus ou indexeur) le tient un
  instant (`IOException`) ; et l'horloge avancée après la mort du faux serveur (le ménage réel de chaque seconde
  pouvait passer entre les deux). Corrigé dans les tests seulement (lectures et suppressions réessayées jusqu'à 10 s,
  horloge avancée avant la mort), rien d'affaibli : **20 lancements de suite à 158/158**. R6 (équipes au combat) est
  déjà mené par une autre session (EtapeDefini G, worktree Elrond) : ne pas le refaire ici. `main` f6703a9 fusionné ;
  compilation et `quick_check` PASS (133/133).
- 2026-10-23 : **R1 et R2 commités et dans `main` : 7cacb4c** « M6 R1 and R2: packaged game and lobby manager »
  (avance rapide de la copie principale après l'accord des sessions mycrusader-7d, -15, -c8 ; `EtapeDefini.md`,
  ce fichier et `LauncherInProgress.md` dans le même commit). Prochaine étape : R3.
- 2026-10-23 : **R3 B1 lancée** (R3.1 config version 2 + R3.3 contrôle de version), worktree Glorfindel, branche
  `net-playtest` = `main` 7cacb4c.
- 2026-10-23 : **R3 B1 rendue : R3.1 et R3.3 faits** (non commités). `FHOLMatchSlot::PlayerName` (dernier champ),
  `CurrentVersion` = 2 ; test `HeroesOfLiberty.Match.ManagerReference` lit le fichier de référence du gestionnaire
  (tous les champs, « Chloé » compris) ; `OlderJson` : version 1 → nom vide. `HOLVersion::IsAccepted` (sans
  `?Version=` : accepté seulement si le serveur est `dev` ; `unknown` toujours refusé), test
  `HeroesOfLiberty.VersionCheck` (8 cas) ; `PreLogin` du mode de jeu refuse avec « Version mismatch: server X, client
  Y » ; `Net/HOLLocalPlayer` (`GetGameLoginOptions` → `Version=<v>` sur toute connexion), `DefaultEngine.ini`
  `LocalPlayerClassName`. **Rouge 133/135** puis **vert 135/135**, `net` à 2 clients PASS (journal du serveur :
  `?Name=…?Version=dev`). Reste de R3.3 : aucun essai bout en bout d'un client refusé (règle pure seule). **Piège
  pour R3.2** : le moteur ajoute lui-même `Name=<pseudo du système>` (`LocalPlayer->GetNickname()`), et
  `FURL::AddOption` remplace une option de même clé : le `?Name=` du gestionnaire serait écrasé ; surcharger
  `GetNickname()` dans `UHOLLocalPlayer` (ou une autre clé), à tester. **R3 B2 lancée** (R3.2).
- 2026-10-23 : **R3 B2 arrêtée à 15 min** (minuteur), pendant son test rouge. Laissé (non commité) :
  `FHOLMatchConfig::HasPlayerNames()` et `FindSlotForName(Name, TakenSlots)` (sans noms : premier libre ; nom connu
  et libre : son emplacement ; inconnu, vide ou pris : `INDEX_NONE` = spectateur), tests dans
  `HOLMatchConfigTests.cpp`, branchement dans le mode de jeu, nom passé par `-PlayerName=` (client 1, Bob) ou
  `?Name=` (client 2, Alice) via `HOLLocalPlayer` / `HOLPlayerController` ; partie `net` de `quick_check` :
  configuration version 2 nommant Alice en 0 et Bob en 1 (ordre inverse de l'arrivée), contrôles « holds the slot
  of » et « logged in as ». La session principale compile et lance `-Parts tests,net` pour fixer l'état.
- 2026-10-23 : **R3.2 fait** (session principale). Premier `tests,net` rouge **pour une autre raison** : la partie
  `net` de `quick_check` lançait son serveur sur 7777 en dur ; le serveur de test d'**Arwen** tenait 7777, le nôtre
  s'est replié sur 7778 et les clients sont allés chez l'autre (même cause que l'alerte « Protocol mismatch
  HOLPlayerController » de mycrusader-15 sur `main`). Corrigé : `$NetPort` = premier port UDP libre dès 7777,
  `-Port=` au serveur, `127.0.0.1:${NetPort}` aux clients (attention : `$NetPort?Name` est lu par PowerShell comme une
  variable `NetPort?Name`). Puis **`tests,net` PASS** (136/136, `net 2 clients` PASS) : « Login request:
  ?Name=Bob?Version=dev » → « Bob holds slot 1 », « ?Name=Alice » → « Alice holds slot 0 » (ordre inverse de
  l'arrivée). Le piège du pseudo du système est réglé (`Name=` reçu = le nom passé). **Restes de R3.2** : rouge de la
  règle pure non relevé (boucle coupée pendant ce lancement) ; cas **spectateur** (nom inconnu) testé seulement en règle
  pure, pas sur la vraie chaîne (à couvrir en R4.7 / R5.3). **R3 B3 lancée** (R3.4 + R3.5 triches).
- 2026-10-23 : **R3 B3 rendue (~15 min) : R3.4 fait, R3.5 codé, vert à lancer.** Inventaire : 10 RPC
  `ServerCheat*` de `HOLPlayerController` (ressources, chantiers rapides, population, vitesses, dégâts, bâtiment,
  défaite, forum tombé, ruine sautée) et **trou du moteur** `APlayerController::ServerExecRPC` (`ServerExec <cmd>` :
  toute commande console sur le serveur hors Shipping, seul chemin des scénarios, du stress test et de
  `hol.Units.Spawn` vers le serveur) ; non gardés : `ServerSurrender`, les ordres ; **noté, non traité** :
  `ServerPause` (un client peut mettre le serveur en pause). `Net/HOLCheatRules.h/.cpp`
  (`AreCheatsAllowed(NetMode, -HOLCheats, hol.Cheat.Refuse)` : solo oui, client jamais, serveur seulement avec
  `-HOLCheats`), garde unique `IsCheatRefused` dans les 10 RPC et `ServerExecRPC_Implementation` (« Cheat refused:
  <quoi> from <nom> (slot N) ») ; test `HeroesOfLiberty.Net.CheatRule` (7 cas) ; `net_match.ps1` passe `-HOLCheats` ;
  partie `net` : serveur `-HOLCheats` + `hol.Cheat.Refuse 1`, client 1 tente `hol.Cheat.Wood 500` après sa capture de
  départ, puis le serveur remet `Refuse 0` (fichiers `quick_net_cheat_tried.flag` / `_back.flag`). **Rouge** : tests
  136/137, `[KO] server: client 1's cheat refused (0), the army's granted (3 of 2)`. Prochain : `quick_check -Parts
  tests,net` (attendu 137/137, « 1 refused, 2 of 2 »).
- 2026-10-23 : **R3.5 vert** (session principale) : `tests,net` PASS en 114 s, **137/137**, `net 2 clients` PASS ; une
  seule ligne « Cheat refused: resources from Bob (slot 1) » au serveur. **R3 B4 lancée** (R3.6 + R3.7 : appels HTTP
  du serveur au gestionnaire).
- 2026-10-23 : **R3 B4 rendue : R3.6 + R3.7 codés, vert sur `tests,net`.** `Net/HOLManagerLink.h/.cpp`
  (`FHOLManagerRules` : route `{url}/matches/{id}/{event}`, corps `{"port":N}`, `{"winners":[…]}`, `{}` ;
  `UHOLManagerLink` : lit `-ManagerUrl=` / `-MatchId=`, envoi HTTP asynchrone, chaque événement une fois) ; mode de
  jeu : `listening` en fin de `StartPlay` (port écouté), `started` au `PostLogin` qui complète les emplacements
  humains, `finished` sur `AHOLGameState::OnMatchOver`, `empty` au `Logout` du dernier après `started` ; test
  `HeroesOfLiberty.Net.ManagerRoutes` ; `quick_check` partie `net` : écouteur HTTP de test (`HttpListener`, fichier
  `Saved_Tests\quick_net_manager.txt`), serveur lancé avec `-ManagerUrl=… -MatchId=QC1`. `tests,net` PASS 138/138,
  appels reçus : `listening {"port":7777}`, `started {}`, `empty {}`. **Pas fait** : rouge (non montré) et `finished`
  (`-Parts tests,net,victory`, attendu `finished {"winners":[0]}`). La session principale les lance.
- 2026-10-23 : **R3.7 complet** : `-Parts tests,net,victory` PASS en 121 s (138/138) ; appels reçus dans l'ordre
  `listening {"port":7777}`, `started {}`, `finished {"winners":[0]}`, `empty {}`. **Rouge non montré** pour B4 (les
  contrôles comptent exactement un appel de chaque sorte : sans envoi ils échouent ; à montrer si une boucle repasse
  par là). **R3 B5 lancée** (R3.8 fin du serveur).
- 2026-10-24 : **R3 B5 rendue (~7 min) : R3.8 fait.** `FHOLManagerRules::ShouldQuit` (non géré ou avant `started` :
  reste ; 0 joueur : quitte ; sinon quitte `hol.Net.QuitAfterFinished` = 120 s après `finished`) ; `UHOLManagerLink`
  vérifie toutes les 0,25 s (serveur géré seulement), attend la réponse à `empty` (≤ `hol.Net.QuitEmptyWait` = 2 s),
  écrit « Server quits: … » puis `RequestEngineExit`. Test `HeroesOfLiberty.Net.ServerQuit` (8 cas) ; partie `net` :
  contrôle « server quits alone once empty » (arrêt de secours gardé). **Rouge** 138/139 + `[KO] server quits alone
  once empty`, **vert** 139/139, `net` PASS. Le cas « 120 s après la fin » n'est couvert que par le test unitaire.
  **Fin de R3 redécoupée** : **B6** = R3.9 (règle du départ ensemble) + gel de R3.10 (simulation, ordres, production) ;
  **B7** = bandeau « En attente de … » + traductions. **R3 B6 lancée.**
- 2026-10-24 : **R3 B6 rendue (~8 min) : R3.9 et gel de R3.10 faits.** `Match/HOLStartRules.h/.cpp`
  (`GetMissing` dans l'ordre de la configuration, casse ignorée ; `ShouldStart(manquants, écoulé, max)`), test
  `HeroesOfLiberty.Match.StartRules` ; mode de jeu : CVar `hol.Match.StartWaitSeconds` (60), attente posée avant
  `Super::StartPlay` si la configuration a des noms, vérifiée à `StartPlay`, à chaque `PostLogin` et chaque seconde ;
  journal « Match waits for: … » / « Match starts: all here (simulation step N) » / « … after 60 s without: … » ;
  accroche R7 commentée ; `started` envoyé au démarrage effectif (sans noms : comme avant). Garde unique dans
  `AHOLGameState` (`bWaitingForPlayers`, `MissingPlayers` répliqués) lue par `UHOLSimulationSubsystem` (aucun pas) et
  `UHOLCommandSubsystem::IsRefused` (ordres refusés). Partie `net` : contrôle attente puis « all here (simulation step
  0) », aucune production avant. **Rouge** 139/140 + `[KO] server: waits for the absent`, **vert** 140/140, `net` PASS,
  `smoke` PASS. Échéance des 60 s couverte par la règle pure seulement. **R3 B7 lancée** (bandeau + traductions).
- 2026-10-24 : **R3 B7 rendue à 15 min : bandeau et traductions faits, test encore rouge à cause du test.**
  `AHOLGameState::WaitEndSeconds` répliqué ; `HOLGameScreen` : `WaitPanel` / `WaitText` en haut au centre,
  « En attente de {Players} ({Time}) » (en « Waiting for … », ja 「…を待っています（…）」), rafraîchi 4 fois par
  seconde, journal « Wait banner: shown, … » / « hidden » ; chaîne de traduction refaite (GatherText, `fill_po.py`,
  GatherText : `.po`, `.archive`, `.locres`, manifeste à jour), tests 140/140. Partie `net` : capture
  `qcnet_N_wait` et contrôle « first client's wait banner ». **Rouge** puis **toujours rouge** : les deux clients
  arrivent à 0,9 s d'écart, la capture précède le bandeau (« shown, En attente de Bob (0:57) » 0,44 s après la
  capture). **R3 B7b lancée** : lancer Alice, attendre « Camera above the start » dans son journal, puis Bob ;
  capture d'Alice après « Wait banner: shown ».
- 2026-10-24 : **R3 B7b rendue : R3 fini (non commité).** `quick_check` partie `net` : Alice (client 2) lancée
  d'abord, Bob seulement quand elle est au-dessus de son départ (60 s au plus, contrôle « client 2 (Alice) above her
  start before client 1's launch ») ; capture d'Alice après « Wait banner: shown » ; contrôle « client 2's wait
  banner » (affiché, capture, caché, PNG). `tests,net` **PASS** en 124 s, 140/140. Capture
  `Saved_Tests_Net2\Screenshots\WindowsEditor\HOL\qcnet_2_wait.png` : « En attente de Bob (0:57) » en haut au centre,
  blanc sur cartouche gris foncé, lisible. **Prochaine étape : commit de R3** (après accord de l'utilisateur) :
  `main` a avancé (a74a5d1 et peut-être plus ; recoupements attendus : `HOLPlayerController`, `DefaultGame.ini`,
  `quick_check.ps1`) → dans Glorfindel, `git stash`, `git merge main`, `git stash pop`, résoudre, `build.ps1`,
  `quick_check -Parts tests,smoke,net,victory`, `dotnet test` du gestionnaire ; `EtapeDefini.md` (ligne M6 : R3
  fait) dans le même commit ; accord des sessions actives ; avance rapide de la copie principale. Puis R4 (écrans du
  jeu).
- 2026-10-24 : l'utilisateur dit de faire la suite (commit de R3 compris). `main` e8a0e36 fusionné dans
  `net-playtest` (travail de R3 mis de côté par `git stash` « r3 », **toujours gardé** dans la liste des stash par
  sécurité : `git stash drop` seulement après le commit). Recoupements de code fusionnés sans conflit ; conflits
  seulement sur les traductions : `fill_po.py` (les deux côtés gardés : textes V3 `ForumHold*` / `TableHold*` et
  `WaitingFor`), fichiers générés (`Content/Localization/Game/**`) repris de `main`, **à régénérer** (GatherText,
  `fill_po.py`, GatherText). Sous-agent lancé : régénération, compilation, `quick_check -Parts
  tests,smoke,net,victory`, `dotnet test`.
- 2026-10-24 : **mis en pause à la demande de l'utilisateur** (une autre session fait des mesures au repos) : le
  sous-agent a été arrêté pendant sa compilation, rien ne tourne dans Glorfindel. **À reprendre, une fois les mesures
  finies** : exactement l'étape précédente (compiler, régénérer les traductions, `quick_check -Parts
  tests,smoke,net,victory`, `dotnet test`), puis le commit de R3.
- 2026-10-24 17:59 : mesures au repos finies (aucun processus de mesure depuis 3 min) ; **vérification relancée**
  (même étape).
- 2026-10-24 : **vérification verte sur `main` e8a0e36** : compilation PASS ; traductions régénérées (GatherText,
  `fill_po.py` 200 textes en / ja, 0 clé inconnue, GatherText ; `WaitingFor` et `ForumHold*` dans les trois `.po`) ;
  `quick_check -Parts tests,smoke,net,victory` PASS en 137 s (145/145) ; `dotnet test` 158/158. `EtapeDefini.md` :
  ligne M6 (serveur de partie fait), ligne `ServerPause` ajoutée. **Commit de R3 en cours** (accord des sessions
  actives, puis avance rapide de la copie principale ; ensuite `git stash drop` du stash « r3 » dans Glorfindel).
- 2026-10-24 : **R3 commité sur `net-playtest` : 174a0f4** « M6 R3: dedicated match server for the lobby manager »
  (`EtapeDefini.md` et ce fichier dedans). Accord demandé à mycrusader-15 et -7d pour l'avance rapide de `main`
  (en attente). Reprise si coupé : vérifier que `main` est toujours e8a0e36, puis dans la copie principale :
  sauvegarder `EtapeDefini.md` et `NetPlaytestInProgress.md`, `git checkout -- EtapeDefini.md NetPlaytestInProgress.md`
  (contenu identique au commit, aux fins de ligne près : vérifier par `diff --strip-trailing-cr`), `git merge
  --ff-only net-playtest`, recopier la dernière version du journal si elle a bougé.
- 2026-10-24 : **R3 dans `main` : 174a0f4** (avance rapide après l'accord de mycrusader-15 et -7d ; journaux des
  autres sessions intacts ; stash « r3 » supprimé). R4 découpé (bloc « Reprise exacte »). **R4 B1 lancée** (R4.1
  client du gestionnaire).
- 2026-10-24 : **R4 B1 rendue : R4.1 fait** (non commité). `Net/HOLManagerClientRules.h/.cpp` (module de jeu :
  `FHOLManagerStatus`, `FHOLLobbySummary`, `FHOLManagerError` (Unreachable / Unreadable / Protocol, code gardé pour
  R4.3), `ParseStatus`, `ParseLobbies`, `ParseError`, `ParseServers`, `ReadFailure`) ; `HeroesOfLibertyUI/Net/
  HOLManagerClient.h/.cpp` (`UHOLManagerClient`, sous-système de l'instance de jeu, client seulement ; `GetStatus`,
  `GetLobbies` asynchrones avec délégués `(résultat, erreur)` ; `IsSameVersion` ; commandes `hol.Manager.Status`,
  `hol.Manager.Lobbies`) ; `servers.json` : `-ServersFile=`, sinon `ProjectDir` dans l'éditeur, **à côté de
  `HeroesOfLiberty.exe`** dans un paquet, sinon `http://127.0.0.1:7700` ; `Tools/Manager/servers.example.json`.
  Tests `HeroesOfLiberty.Net.ManagerClient.Parse` / `.Failure` (exemples de `PROTOCOL.md`) : **rouge** puis **vert
  147/147**. Vraie chaîne : `Tools/Tests/manager_client.ps1` (gestionnaire publié sur un port libre dès 7790, un
  salon créé et tenu vivant par lectures, jeu hors écran) **PASS** : « 1 lobbies (same version) », « L1 "Salon R4
  test" 1/8 lobby ». Gestionnaire injoignable : règle pure seulement. **R4 B2 lancée** (R4.2 écran Multijoueur).
- 2026-10-24 : **R4 B2 rendue : R4.2 fait** (non commité). `Screens/HOLMultiplayerScreen.h/.cpp` : bouton
  « Multijoueur » entre « Jouer » et « Réglages » ; titre, « Votre nom » (gardé dans `UHOLGameUserSettings::PlayerName`,
  par défaut le compte Windows filtré ; règle des noms de `PROTOCOL.md`), liste (nom, « 1/8 », « au salon » /
  « lancement » / « en partie (N min) », « Rejoindre » ou « Regarder »), « Actualiser », « Créer un salon », « Retour » ;
  Créer / Rejoindre / Regarder **journalisent seulement** (« Multiplayer: join L1 as baudo ») ; ligne d'état simple.
  `hol.UI.Open Multiplayer`. `manager_client.ps1 -Screen` : **rouge** (« unknown screen 'Multiplayer' ») puis **PASS**
  (deux fois) ; capture relue par l'agent (titre doré, nom lisible, une ligne de salon, boutons en bas) ;
  `tests,smoke` PASS (147/147). Restes : champ de nom au style par défaut (fond clair), manette non essayée,
  `manager_client.ps1` sans `-Screen` non relancé. **R4 B3 lancée** (R4.3 messages d'échec).
- 2026-10-24 : **R6 fait par mycrusader-15 et dans `main` : 8fcb0f4** (ligne G d'EtapeDefini : `FHOLCampTable`,
  `HOLVictoryRules::AreHostile`, combat et évitement par camp, attaque refusée sur un allié, clic droit sur un allié
  = déplacement ; tests `CampsByTeams`, cas « sides fight » / « sides avoid » ; essai validé par l'utilisateur).
  R6.1 à R6.4 faits. À fusionner dans `net-playtest` au prochain commit (recoupements probables :
  `HOLPlayerController`, `quick_check.ps1`).
- 2026-10-24 : **R4 B3 rendue (~8 min) : R4.3 fait** (non commité). `HeroesOfLibertyUI/Net/HOLManagerErrorText.h/.cpp`
  (`GetManagerErrorText` : un texte français par code de `PROTOCOL.md`, injoignable, illisible, version « le serveur
  a X, vous avez Y » lue dans le message du gestionnaire, cas général avec le message ; `MakeVersionMismatchError`) ;
  écran Multijoueur : `/status` d'abord, erreur en rouge (`HOLUIStyle::Missing`), liste vide ; journal « Multiplayer:
  error … ». Test `HeroesOfLiberty.UI.ManagerErrorText` (premier test du module d'interface) : **rouge 26/26** puis
  **vert**, `tests` 148/148. `manager_client.ps1 -Case Off|OtherVersion` (gestionnaire `--GameVersion=`) **PASS** ;
  sans `-Screen` et `-Screen` **PASS**. Capture `manager_client_version.png` : ligne rouge lisible « Version
  différente : le serveur a 2026-10-24-other, vous avez dev. ». `lobby_full` / `name_taken` : règle seule (B4).
  **R4 B4 lancée** (R4.4 : créer / rejoindre un salon et le lire).
- 2026-10-24 : **R4 B4 rendue à 15 min : R4.4 codé, vérification incomplète.** `HOLManagerClientRules` :
  `FHOLLobbySlot`, `FHOLLobby` (`GetNumPlayers`, `FindYourSlot`), `ParseLobby`, `ParseJoin` ; `UHOLManagerClient` :
  `CreateLobby`, `JoinLobby` (version envoyée), `ReadLobby`, `LeaveLobby`, jeton gardé (jeté sur `not_in_lobby` /
  `lobby_not_found`), `Send` générique ; Multijoueur : Créer (« Salon de <nom> ») / Rejoindre ouvrent
  `HOLNewGameScreen::EnterLobby` (lecture toutes les 0,5 s, titre = nom du salon, nom et « (chef) » par ligne, état
  Ouvert / Fermé / Pas prêt / Prêt, lignes des autres grisées, ma couleur et mon équipe actives mais rien envoyé,
  « Lancer » désactivé ; Retour / Échap = `leave`) ; `-PlayerName=` en ligne de commande pour les tests. Test
  `HeroesOfLiberty.Net.ManagerClient.Lobby` : **rouge** (25 attentes) puis **vert**. `manager_client.ps1 -Case Lobby`
  **PASS** (Bob entre, « 2 players (Alice, Bob) … leader Alice », sort ; vérifié côté gestionnaire), mais **capture
  prise trop tôt** (écran Multijoueur) : script corrigé (`waitui`, `wait 1`), **non relancé**. Non lancés : `-Case
  NameTaken`, `quick_check -Parts tests,smoke,menu`. Gestionnaire coupé pendant le salon : codé, non testé. La
  session principale lance les trois.
- 2026-10-24 : **R4.4 vérifié** (session principale) : `-Case Lobby` **PASS** et capture relue (« Salon R4 test »,
  « Alice (chef) » Pas prêt, Bob avec couleur Rouge et équipe actives, autres lignes « Ouvert » grisées, « Lancer »
  grisé) ; `-Case NameTaken` **PASS** (« Ce nom est déjà pris dans ce salon. » en rouge) ; `quick_check -Parts
  tests,smoke,menu` **PASS** (149/149, menu fr / en / ja). Remarque pour B5 : la note « Grisé : pas encore dans le
  jeu (IA, factions, équipes, mode et options) » du solo s'affiche aussi dans le salon, à adapter. **R4 B5 lancée**
  (R4.5 + R4.6 : salon en écriture, colonnes).
- 2026-10-24 19:10 : **R4 B5 coupée, reprise.** Trouvé dans Glorfindel (rien après 18:57) : R4.5 + R4.6 **codés et
  compilés** (`UpdateMe`, `UpdateSettings`, `SetSlot`, `StartMatch` du client ; salon : couleur, équipe, « Prêt » /
  « Plus prêt », carte et options du chef, emplacements ouverts / fermés par le chef, « Lancer » caché au non-chef et
  actif quand tous sont prêts, refus en rouge, « ♛ » devant le chef, note du salon adaptée) ; `manager_client.ps1
  -Case Ready|Leader` écrits, **non lancés**. Reprise : compilation, puis ces deux cas, puis `tests,smoke,menu`.
- 2026-10-24 19:20 : **R4 B5 rendue : R4.5 + R4.6 faits** (non commité). `manager_client.ps1 -Case Ready` **PASS**
  (Bob prend la couleur 5 et « Prêt », le gestionnaire a les deux, « Lancer » caché pour lui) ; `-Case Leader`
  **PASS** (Bob crée et mène, ferme l'emplacement 2, Alice prête par HTTP, Bob « Prêt », « Lancer » envoyé : 500
  `server_start_failed` faute de serveur dans le gestionnaire de test, salon intact) ; captures relues (« ♛ » devant le
  chef, « Prêt » / « Pas prêt », lignes des autres grisées, « Fermé » choisi, « Lancer » chez le chef seul) ;
  `quick_check -Parts tests,smoke,menu` **PASS** (150/150). Remarque : les lignes ouvertes montrent une couleur par
  défaut qui peut être celle d'un joueur (Violet deux fois dans `manager_client_ready.png`), sans effet sur la partie.
  **R4 B6 lancée** (R4.7 : salon « en partie » → connexion au port donné avec `?Name=…?Version=…`, « Regarder »).
- 2026-10-24 19:45 : **R4 B6 rendue (~13 min, plus le temps des essais) : R4.7 codé, vraie chaîne bloquée.**
  `HOLManagerClientRules` : `ParseWatch`, `MakeMatchAddress` (hôte de l'URL du gestionnaire, `:port?Name=…?Version=…`,
  sans nom pour un spectateur) ; `UHOLManagerClient::WatchLobby`, `OpenMatch` (`SetClientTravel` absolu, jeton du
  salon gardé pour R4.8) ; salon : « Lancement… » en `starting`, `JoinMatch` une fois en `in_game` (lecture arrêtée,
  pas de `leave` au voyage) ; Multijoueur : « Regarder » → `/watch` puis connexion sans nom, refus en rouge. Test
  `HeroesOfLiberty.Net.ManagerClient.Match` **rouge** (8 attentes) puis **vert** ; `-Case Leader` **PASS** ;
  `quick_check -Parts tests` **PASS** (151/151). `manager_client.ps1 -Case Match` (gestionnaire avec `--ServerExe=`
  sur le serveur cuit) **FAIL** : le serveur cuit date d'avant R3 (« match.json is invalid (version 2 is newer than
  this build (1)) »), il n'appelle jamais `listening`. **Séparateur corrigé** : les options d'une URL Unreal se séparent
  par `?` (`ParseOption` sur `Name=Bob&Version=x` rendrait « Bob&Version=x ») : `?Name=…?Version=…`, `PROTOCOL.md`
  corrigé (dans Glorfindel) et ce fichier aussi. **Serveur re-cuit** (`net_match.ps1 -Stage`, en cours), puis
  `-Case Match`. « Regarder » : pas d'essai de bout en bout (R5.3).
- 2026-10-24 19:35 : **R4.7 vérifié sur la vraie chaîne.** Serveur re-cuit (19:25) ; il lui manquait `Version.txt`
  (serveur cuit sans ce fichier = « unknown », refuse tout le monde, HOLVersion) : `-Case Match` pose « dev » le temps
  de l'essai et l'enlève (dans un paquet, `package.ps1` écrit le vrai). Script : attend `Plain` chez le client et
  « Bob holds slot 0 » dans le `server.log` de la partie ; filtre du journal corrigé (`LogHOLNet: Display: Manager`).
  `-Case Match` **PASS** (Lancer 202, `in_game`, « joining match … at 127.0.0.1:7778 as Bob », Bob à l'emplacement 0
  côté serveur) ; `-Case Lobby` **PASS** ; aucun serveur de Glorfindel laissé en marche (un serveur oublié par
  `net_match.ps1 -Stage` arrêté). À R8 : le serveur lancé par le vrai gestionnaire doit avoir le même `Version.txt` que
  le zip des joueurs. **R4 B7 lancée** (R4.8 : fin de partie, « Retour au salon » / « Quitter vers la liste », ~60 s).
- 2026-10-24 19:55 : **R4 B7 rendue (~13 min) : R4.8 fait** (non commité, serveur inchangé). Bandeau de fin d'une partie
  venue d'un salon : « Continuer à regarder », « Quitter vers la liste », « Retour au salon (n s) », retour d'office à 0
  (`HOLManagerClientRules::GetReturnCountdown`, 60 s) ; hors salon, bandeau inchangé. Le choix est noté et la partie
  quittée ; la carte du menu, une fois chargée, ouvre Multijoueur puis rejoint le salon sous le même nom
  (`HOLMenuPlayerController`) ; « Quitter vers la liste » oublie le salon sans appel (protocole). Fichiers :
  `HOLManagerClient`, `HOLEndBanner`, `HOLMenuPlayerController`, règles et tests. Test `ManagerClient.Return` **rouge**
  (5) puis **vert** ; `manager_client.ps1 -Case Return` **PASS** deux fois (Alice ne vient pas, 60 s de R3.9, Bob
  `hol.Victory.Surrender` ; serveur `finished {"winners":[1]}` ; jeu « Menu: back in lobby L1 as Bob » ; gestionnaire
  `lobby` avec Bob ; plus de serveur de Glorfindel) ; `tests` **PASS** (152/152). Capture relue
  (`manager_client_return.png` : « Défaite », trois boutons). Restes : « Quitter vers la liste » et retour d'office non
  essayés de bout en bout ; retour refusé (plus de 2 min) journalisé seulement, sans message (→ B8) ; le panneau
  « Triches » du client de l'éditeur s'affiche en partie réseau (le serveur les refuse ; à cacher ?) ; deux clients
  « Player » d'un `net_match` d'un autre worktree sont arrivés en spectateurs sur le port 7777 d'un serveur de test
  (ports partagés entre worktrees). **R4 B8 lancée** (R4.9 : erreurs en partie).
- 2026-10-24 20:10 : **R4 B8 rendue (~12 min) : R4.9 fait** (non commité, serveur inchangé). `HOLManagerErrorText` :
  `MakeMatchLossError` / `MakeMatchTravelError` (`match_lost`, `match_closed`, `match_version`, `match_full`,
  `match_refused`, `match_failed`, textes français) ; `UHOLManagerClient` écoute `OnNetworkFailure` / `OnTravelFailure`
  seulement pour une partie venue d'un salon (`bInMatch`, posé par `OpenMatch`) : salon oublié, perte notée, liste à
  l'arrivée au menu ; un choix du bandeau de fin déjà fait l'emporte. « Retour au salon » refusé : liste et message en
  rouge. Test `HeroesOfLiberty.UI.MatchLossText` **rouge** (29) puis **vert** ; `manager_client.ps1 -Case Killed`
  **PASS** (serveur de la partie tué par PID ; « La connexion à la partie est perdue. » en rouge sur la liste ; capture
  `manager_client_killed.png` relue ; le gestionnaire remet le salon « au salon ») ; `-Case Return` **PASS** ; `tests`
  **PASS** (153/153). **Délai mesuré : 60,2 s** entre la mort du serveur et le retour au menu (`ConnectionTimeout`
  d'Unreal, 60 s) : à décider avec R7 (15-20 s ?). Non essayés de bout en bout : retour refusé après 2 min, refus de
  connexion (version, pleine), serveur arrêté proprement. **R4 B9 lancée** (R4.10 + R4.11 : traductions).
- 2026-10-24 20:20 : **R4 B9 rendue (~5 min) : R4.10 + R4.11 faits** (non commité). 57 textes nouveaux traduits en / ja
  par la chaîne en place (GatherText, table de `Tools/Localization/fill_po.py`, .locres compilés ; `fill_po.py` : « 257
  texts filled, 0 unknown keys » en et ja). Vocabulaire : salon → lobby / ロビー, chef → lobby leader / リーダー, Prêt →
  Ready / 準備完了, Regarder → Watch / 観戦, serveur de parties → match server / 対戦サーバー. `quick_check -Parts
  tests,smoke,menu` **PASS** (153/153, menu fr / en / ja). Captures Multijoueur en / ja relues (aucun français, kana
  et kanji lisibles). Non vus à l'image en en / ja : salon, bandeau de fin, messages d'erreur. Japonais provisoire
  comme le reste. **R4 fini (non commité).** Avant le commit : fusionner `main` 8fcb0f4 (R6) dans `net-playtest`.
  **R5 B1 lancée** (R5.1 + R5.2 : `lobby_match.ps1`, deux clients du jeu, liste → salon → Prêt → partie → fin → retour).
- 2026-10-24 20:35 : **R5 B1 rendue : R5.1 + R5.2 faits** (non commité, ni code du jeu ni serveur changés).
  `Tools/Tests/lobby_match.ps1` : gestionnaire de test + serveur cuit + **deux clients du jeu** (Alice, Bob ; Saved et
  journaux séparés) : Alice crée, Bob rejoint, couleurs 2 et 5, deux « Prêt », Alice lance, les deux en partie
  (serveur : « Alice holds slot 0 (colour 2) », « Bob holds slot 1 (colour 5) »), Bob abandonne, victoire / défaite,
  les deux « Retour au salon », salon intact chez le gestionnaire (`lobby`, couleurs 2 et 5, « Prêt » à faux).
  **PASS du premier coup en 45 s** (pas de rouge : R4 complet ; noté). `-Case Return` **PASS**. Captures relues.
  Remarques : tableau des joueurs en partie « Joueur 1 / 2 » au lieu des noms ; capture de Bob prise 0,3 s après
  l'arrivée (centre au bord) ; au retour, le premier revenu (Bob) est chef 0,4 s avant qu'Alice, ancienne cheffe,
  le redevienne (règle du protocole ; « Lancer » visible un instant). **R5 B2 lancée** (R5.3 spectateur + R5.4
  partie `lobby` de `quick_check`).
- 2026-10-24 20:55 : **R5 B2 rendue (~12 min) : R5.3 + R5.4 faits** (non commité ; scripts seulement). `lobby_match.ps1` :
  troisième client « Carol » à chaque essai, lancé quand le salon passe `in_game` : liste « en partie (0 min) »,
  « Regarder », arrivée sans nom (serveur : « Carol joined as a spectator »), captures `lobby_match_watchlist`,
  `_watch`, `_watch_end` ; Bob n'abandonne qu'après l'arrivée de Carol (fichier signal). Rouge : 1er essai FAIL (bug du
  script : séquence de Carol tronquée), puis **PASS en 74 s** ; `quick_check -Parts lobby` **PASS en 87 s** (partie
  seulement sur demande, pas dans `-Full` ; FAIL clair sans serveur cuit, vérifié) ; `tests` **PASS** (153/153).
  **Constats sur le spectateur, à trancher avec l'utilisateur** : (1) pas de bandeau de fin ni de retour à la liste
  pour lui ; (2) **le serveur reste allumé tant qu'un spectateur reste** (« empty after the match » seulement à son
  départ) : correction côté serveur, puis re-cuisson ; (3) interface de joueur complète (ressources à 0, « Inactifs »,
  « Triches ») et caméra sur aucun départ ; (4) tableau des joueurs « Joueur 1 / 2 » au lieu des noms (joueurs aussi).
  **Arrêt des boucles ici** en attendant l'utilisateur : R5.5 (mesure de deux batailles, maximum de serveurs à fixer
  avec lui) et les points ci-dessus demandent son avis ; puis commit (fusion de `main` 8fcb0f4 d'abord), puis E2 / E3.
- 2026-10-24 21:15 : **Réponses de l'utilisateur** : M6-Q10 à M6-Q13 (tableau des décisions) ; R5.5 en fin de R5 ;
  fusion de `main` puis commit de R4 et R5 accordés une fois ces points faits et vérifiés ; le correctif de version qui
  casse S5 (`net_match.ps1`, serveur cuit sans `Version.txt`) est laissé à la session perf. **Boucles prévues** :
  **C1** noms des joueurs, délai de 20 s, « Regarder » refusé après la fin, bandeau du spectateur sans retour d'office
  (serveur touché : re-cuisson) ; **C2** statistiques côté serveur (compteurs et séries dans le temps, tests) ;
  **C3** envoi aux clients ; **C4** tableaux chiffrés ; **C5** courbes ; **C6** accès (bouton du spectateur, bandeau
  de fin pour tous) et captures ; **C7** traductions ; puis R5.5, fusion, commit, E2 / E3. **C1 lancée.**
- 2026-10-24 21:25 : **S5 cassé sur `main` par R3.3** (signalé par la session perf, mycrusader-f8, à la demande de
  l'utilisateur) : serveur de `net_match -Stage` sans `Version.txt` = « unknown », refuse tout le monde. Correctif :
  `net_match.ps1` écrit `Version.txt` = « dev » à côté du stage à chaque lancement (tests seulement ; paquets
  inchangés). Posé **non commité** dans la copie principale et dans Glorfindel ; vérification (S5 jusqu'au bout)
  demandée à mycrusader-f8.
- 2026-10-24 21:45 : **C1 rendue (~17 min) : codée, tests verts, pas vérifiée en partie** (re-cuisson bloquée :
  « A conflicting instance of AutomationTool is already running », empaquetage de la copie principale en même temps).
  M6-Q12 : `HOLForumRules` (nom du créneau venu de `LaunchConfig`, sinon « Joueur N » ; « Vous » pour soi), journal
  `Players table: …` (`HOLGameScreen`). M6-Q13 : `ConnectionTimeout=20.0` (`DefaultEngine.ini`, `IpNetDriver` ;
  vaut aussi pour le serveur ; `InitialConnectTimeout` 60 inchangé). M6-Q10 : `EHOLEndBanner::MatchOver` pour le
  spectateur (« Partie terminée », « Continuer à regarder », « Quitter vers la liste », sans compte à rebours,
  `CountsDownToLobby`), ligne `Detail` « Vainqueur : Alice » (place des statistiques) ; serveur : `PreLogin` refuse un
  client sans nom après la fin (« The match is over ») ; gestionnaire inchangé (`ServerFinished` remet `lobby`).
  Tests : **rouge** 151/153 puis **vert** 153/153. **C1b lancée** : contrôles ajoutés à `lobby_match.ps1`, puis, après
  la re-cuisson (lancée par la session principale à la fin de l'autre empaquetage), `lobby_match`, `-Case Killed`
  (≈ 20 s attendu), `tests`.
- 2026-10-24 22:00 : **Correctif S5 vérifié par mycrusader-f8** : S5 va au bout avec `net_match.ps1` corrigé (« Version:
  dev » des deux côtés ; avec `-Stage`, erreur affichée 24,2 cm PASS ; seule la bande passante échoue, 32 073 B/s pour
  32 000, question D218 hors de ce correctif). Commit sur `main` laissé à l'utilisateur.
- 2026-10-24 22:15 : **Correctif S5 commité sur `main` : 6167f5c** (par mycrusader-f8, accord de l'utilisateur ;
  `net_match.ps1` seul). Le même changement est dans Glorfindel : la fusion de `main` le reprendra sans conflit.
  Serveur de Glorfindel re-cuit (C1 dedans), rien laissé en marche ; C1b vérifie.
- 2026-10-24 22:35 : **C1b rendue.** `-Case Killed` **PASS, délai 20,1 s** (était 60,2 s) ; `tests` **PASS**
  (153/153) ; `lobby_match.ps1` : tous les contrôles C1 passent (tableau : « Vous, Bob » chez Alice, « Alice, Vous »
  chez Bob, « Alice, Bob » chez Carol ; Carol `MatchOver`, « Vainqueur : Alice », pas de retour au salon) **sauf Dave**
  (4e client, `open` direct après la fin) **admis** : le moteur envoie toujours le pseudo, le refus « sans nom » ne
  marchait jamais ; `PreLogin` corrigé (après la fin, refus si le nom ne tient aucune place, `FindSlotForName`),
  **à re-cuire**. Autre défaut trouvé et corrigé (client) : le bandeau des joueurs d'un salon n'avait plus le retour au
  salon (`CountsDownToLobby` évalué avant `SetResult`) → `RefreshWayBack()`. Reste : le spectateur venu de
  « Regarder » a « Quitter vers le menu » (il n'est pas membre du salon) au lieu de « Quitter vers la liste » (M6-Q10) :
  à corriger. **C1c lancée** (libellé et sortie du spectateur vers la liste, re-cuisson, `lobby_match`).
- 2026-10-24 22:55 : **C1c rendue : C1 fini et vérifié** (non commité). `EHOLMatchOrigin` (Local / Manager / Lobby,
  `HOLGameMenuRules`) : compte à rebours seulement pour Lobby, `QuitsToList` pour Manager et Lobby ; le spectateur venu
  de « Regarder » a « Quitter vers la liste », qui ouvre Multijoueur. Serveur re-cuit (1 min 11 s, `PreLogin` dedans).
  Tests **rouge** 152/153 puis **vert** 153/153 ; `lobby_match.ps1` **PASS** (79 s : noms, bandeau de Carol, retour
  de Carol à la liste, Dave refusé « The match is over (spectator) », serveur éteint au départ de Carol) ;
  `quick_check -Parts tests,lobby` **PASS** ; `-Case Return` **PASS**. Captures relues. Restes : Dave (`open` à la
  console) revient au titre sans message (sans gravité : « Regarder » disparaît de la liste après la fin) ; perte de
  connexion du spectateur pas essayée. **C2 lancée** (statistiques côté serveur).
- 2026-10-24 23:40 : **C2 rendue (au-delà de 15 min, coupure d'usage) : enregistreur fait, vérification partielle.**
  `Match/HOLMatchStats.h/.cpp` (structures prêtes pour C3, règles pures), `Match/HOLMatchStatsSubsystem` (fin de pas
  de la simulation, échantillon toutes les 10 s de jeu, `hol.Stats.Dump`) ; branchements : morts avec le tueur
  (`HOLUnitSubsystem::MarkUnitDying`, `HOLCombatProcessor`), production, bâtiments perdus (`DestroyBuilding`),
  récolte lue dans `UHOLSourceSubsystem::GetDeposited`. Retenu : tués / perdus (dont péons), bâtiments perdus,
  récolté par ressource, stock, produits, maxima ; courbes péons, armée, population, bâtiments, récolté, stock.
  **Manque dans le jeu : passages d'âge (pas encore de passage, seulement des onglets), héros, auteur d'un bâtiment
  rasé, bâtiments construits, technologies.** S4 : somme de contrôle identique avant / après (`dbf021190616ce7e`) ;
  temps 5,8 → 6,9 ms/pas (FAIL du seuil 6 ms, autre worktree en test en même temps : à refaire au calme). Test
  `Match.Stats.Rules` vert **sans rouge vu** ; `smoke` PASS, dump cohérent (3 hommes d'armes tuent un péon). **C2b
  lancée** : rouge, échantillon du pas 0, événement `Defeated`, essai avec récolte et production, S4 au calme.
- 2026-10-25 00:10 : **C2b rendue (~11 min) : C2 fini** (non commité). Rouge vu (`RecordUnitDeath` cassé : 1 attente en
  échec) puis vert 154/154 ; échantillon de t = 0 (la simulation diffuse `StepIndex` déjà incrémenté) ; événements
  `Defeated` (`DefeatSlot`) et `ForumFell` (`HandleForumFall`) branchés, **pas encore vus en partie** ; dump de la
  partie `gather` exact (3 produits = 3 « Production … done », récolté F 18 = somme des dépôts, 19 péons) ;
  `quick_check -Parts tests,smoke,production,gather` **PASS** (54 s). S4 au calme : somme identique partout, ~6,6
  ms/pas **avec comme sans l'enregistreur** (A/B) : il ne coûte rien de mesurable, la machine est plus lente qu'à 19:50
  (seuil 6 ms à revoir plus tard, hors C2). **C3 lancée** (envoi aux clients ; `Defeated` / `ForumFell` vus dans une
  partie `victory`).
- 2026-10-25 00:40 : **C3 rendue (~14 min) : statistiques envoyées aux clients** (non commité). Demande du client
  `ServerRequestMatchStats(FromSample, FromEvent)`, contrôle `HOLMatchStats::CanReceiveStats` (spectateur toujours,
  joueur seulement après la fin, vaincu = joueur), réponse fiable `ClientReceiveMatchStats` par morceaux de 20
  échantillons par joueur (~9 Ko ; 1 h ≈ 160 Ko en 18 morceaux), fusion côté client ; point d'accès
  `UHOLMatchStatsSubsystem::GetStats / GetOnStatsUpdated / RequestStats` (solo : lecture directe). Test
  `Match.Stats.Send` **rouge** puis **vert** ; serveur re-cuit ; `lobby_match.ps1` **PASS** (77 s : Carol reçoit pendant
  et après, Bob refusé pendant puis servi après, mêmes chiffres que le serveur, événement `Defeated` à 19 s vu par
  les trois) ; `quick_check -Parts tests,smoke,lobby` **PASS** (155/155, session principale). Restes : découpage en
  plusieurs morceaux vérifié seulement par le test pur ; horloge à 0 dans le dump d'un client. **C4 lancée** (écran des
  statistiques, tableaux chiffrés).
- 2026-10-25 01:05 : **C4 rendue à 15 min, incomplète** (non commité). `Screens/HOLStatsScreen` : onglets Résumé /
  Militaire / Économie / Chronologie (attente de C5), une ligne par place, résultat Vainqueur / Vaincu / En jeu,
  demande à l'ouverture, mise à jour sur `GetOnStatsUpdated`, refus « disponibles à la fin de la partie » ; scores
  `GetMilitaryScore` (10 par tué) et `GetEconomyScore` ((F+B+P+O)/10 + 5 par produit), test `Stats.Scores` **rouge**
  puis vert ; bouton « Statistiques » du bandeau ; `hol.UI.Open Stats` ; le HUD ne rouvre plus le bandeau sous l'écran.
  `quick_check` : tests 156/156, smoke, menu PASS ; **victory FAIL** (« Quitter vers le menu: MainMenu loaded » KO après
  le passage par les stats). Défauts : noms non colorés (style `UCommonTextBlock`), capture à chiffres nuls (victoire
  immédiate). Décision de la session : **le compte à rebours du retour au salon est gelé** pendant qu'on regarde les
  stats (il reprend au retour, sans repartir de 60). **C4b lancée.**
- 2026-10-25 01:30 : **C4b rendue (15 min) : étapes 1 à 3 faites** (non commité). KO de `victory` = vérification du
  script (section « victory win » coupée par la nouvelle marque « victory stats ») : corrigé, contrôle « stats screen
  opened, then closed » ajouté. Noms : pastille de couleur devant le nom (comme le salon). Compte à rebours gelé pendant
  les stats : `FHOLManagerClientRules::GetCountdownSecondsShown` + 4 cas dans `ManagerClient.Return` (**vert sans rouge
  vu**), bandeau (`bCountdownPaused`, « countdown frozen at N s », « (resumed) »), **pas encore vu en salon réel**.
  `quick_check -Parts tests,smoke,menu,victory` **PASS** (156/156). Restent : capture avec des chiffres non nuls,
  capture chez Carol, `lobby`. **C4c lancée.**
- 2026-10-25 01:45 : utilisateur absent ; consigne : **deux sessions tournent, la dernière à finir met le PC en
  veille**. Accord proposé à mycrusader-8d (chacun prévient l'autre quand il a fini). Plan de cette session jusque-là :
  C4c, C5 (courbes), C6 (bouton du spectateur), C7 (traductions) ; pas de commit (accord de l'utilisateur après C1-C4
  et vérification) ; R5.5 attend l'utilisateur (maximum de serveurs).
- 2026-10-25 02:05 : **C4c rendue (~9 min)** (non commité). Rouge du gel vu (`GetCountdownSecondsShown` vidé : 4
  attentes en échec) puis vert 156/156. Captures avec des chiffres en solo (2e match de `victory` : un Paysan produit
  et récolte, 3 Hommes d'armes tuent 3 Lanciers, défaite de l'IA) relues : Résumé 30 / 6 / 36, Militaire 3 tués, 3
  perdus et 1 bâtiment perdu chez l'adversaire, Économie nourriture 10, 1 produit = dump exact ; « right screen »
  vérifie l'écran du dessus (`menu=HOLStatsScreen`). `quick_check -Parts tests,smoke,menu,victory,gather` **PASS**.
  **Pas lancé** (mycrusader-8d mesure S5 sur le port 7777 ; serveurs dédiés suspendus jusqu'à son « serveurs
  libres ») : ajouts de `lobby_match.ps1` (stats chez Carol, gel puis reprise du compte à rebours chez Alice).
  **C5 lancée** (courbes, client seul, sans serveur dédié).
- 2026-10-25 02:30 : pause complète de ~8 min pour une mesure au calme de mycrusader-8d (C5 suspendue puis reprise).
  **8d a fini** : cette session est la dernière, elle met le PC en veille à la fin (C5, vérification en salon, C6, C7).
- 2026-10-25 02:50 : **C5 rendue : courbes faites** (non commité). Règles `EHOLMatchStatsSeries` (Péons, Armée,
  Population, Bâtiments, Ressources récoltées, Stock), `MakeCurve`, `GetNiceStep` (pas ronds, temps en m:ss),
  `GetAxisEnd`, `FormatMatchTime` ; widget `Widgets/HOLStatsGraph` (dessiné en code : grille, axes, une courbe par
  joueur à sa couleur, marques d'événements datées, légende) ; onglet Chronologie avec six boutons de série, mis à
  jour à chaque réception. Test `Stats.Curves` **rouge** puis **vert** (157/157) ; `quick_check -Parts
  tests,smoke,menu,victory` **PASS** (77 s) ; captures Péons et Armée relues (courbes, axes 0:00-0:40, marque
  « Joueur 2 vaincu », légende). Restes : courbes de même valeur superposées (la rouge cachée sous la bleue),
  étiquettes en italique, pas de détail par ressource. BOM de `quick_check.ps1` remis comme dans HEAD.
  **C6 lancée** (bouton « Statistiques » du spectateur pendant la partie, vérification en salon réel, courbes
  superposées).
- 2026-10-25 03:10 : **C6 rendue (~12 min) : accès du spectateur fait** (non commité, serveur inchangé). Règle
  `ShowsStatsButton` (spectateur avant la fin), test `GameMenu.StatsButton` **rouge** puis vert ; bouton
  « Statistiques » à côté de « Menu » (`HOLGameScreen`), écran redemandé toutes les 10 s tant qu'il est ouvert ;
  courbes décalées de 3 px par joueur (plus de courbe cachée) ; onglet et série revus rétablis à la réouverture
  (bogue du pool). `lobby_match.ps1` : stats de Carol pendant la partie (envois à 10 s d'écart côté serveur) et après ;
  **gel vu en salon réel** (« frozen at 59 s », « resumed at 59 s », puis retour au salon) ; 1er essai FAIL (Bob parti
  avant le retour d'Alice : attente ajoutée), puis **PASS** deux fois ; `quick_check -Parts
  tests,smoke,menu,victory,lobby` **PASS** (162 s, 158/158). Captures relues. Restes : pas de raccourci clavier
  (pas de convention), courbe au maximum collée au bord haut. **C7 lancée** (traductions, marge haute de l'axe).
- 2026-10-25 03:35 : **C7 rendue (~15 min) : traductions et marge de l'axe** (non commité). 44 textes traduits en / ja
  (`fill_po.py` : 301 textes, 0 clé inconnue ; Summary / Military / Economy / Timeline, 概要 / 軍事 / 経済 /
  タイムライン ; « Forum » → フォルム à valider) ; `GetAxisEnd(…, bHeadroom)` : au moins un dixième au-dessus du
  maximum, test `Stats.Curves` **rouge** (157/158) puis **vert** (158/158) ; `quick_check -Parts
  tests,smoke,menu,victory` **PASS** ; parties `victory` en `en` et `ja` PASS, captures relues (aucun français hors
  panneau de triches, kanji lisibles, courbe sous le bord haut). **C1 à C7 finis.**
  **Reprise** : tout est dans Glorfindel, **non commité**. Prochaine séance avec l'utilisateur : (1) relire les
  captures et décisions (« Forum » en japonais, absence de passages d'âge dans les stats : à ajouter quand le jeu en
  aura) ; (2) R5.5 (mesure de deux batailles, maximum de serveurs) ; (3) fusion de `main` (8fcb0f4, 6167f5c, a37995c…)
  dans `net-playtest`, `quick_check` complet + `lobby`, commit de R4, R5, C1-C7 (accord donné, sessions actives à
  prévenir avant l'avance de `main`) ; (4) essais E2 et E3. Session terminée, PC mis en veille (dernière session).
- 2026-10-25 : **Correction de l'utilisateur** : « pas de retour d'office après 60 s » valait pour tous, pas seulement
  le spectateur (mauvaise lecture de la session). M6-Q7 revu : à la fin, joueurs et spectateurs restent dans la
  partie aussi longtemps qu'ils veulent (carte, statistiques). Conséquence : la réservation de la ligne au salon (2 min
  après `finished`, PROTOCOL choix 6) doit durer **tant que le serveur de la partie tourne**, puis 2 min. **D1
  lancée** : compte à rebours retiré, sortie toujours possible depuis le menu de partie, réservation du gestionnaire.
- 2026-10-25 : accord donné à mycrusader-ea pour avancer `main` vers `victory-v4` (Victory V4, messages de la ruine).
  À la fusion de `main` dans `net-playtest`, recoupements attendus : `HOLGameState.*`, `HOLPlayerController.*`,
  `HOLGameScreen.*`, `quick_check.ps1`, `fill_po.py`, .po / .archive / .locres (refaire la collecte après fusion).
- 2026-10-25 : **D1 rendue (~15 min) : plus de retour d'office** (non commité, serveur inchangé). Compte à rebours
  retiré (règles `GetReturnCountdown`, `GetCountdownSecondsShown`, `CountsDownToLobby`, gel, test `ManagerClient.Return`
  supprimés) ; `OffersReturnToLobby` ; « Statistiques » du HUD pour tous après la fin (`ShowsStatsButton`, test
  **rouge** 156/157 puis vert) ; menu de partie après la fin : « Retour au salon », « Quitter vers la liste » (ou le
  menu), « Résultat » (rouvre le bandeau). `lobby_match` : Alice reste ~73 s dans la partie finie sans être renvoyée,
  ferme le bandeau, ouvre les stats, part par le menu ; `quick_check -Parts tests,smoke,menu,victory,lobby` **PASS**
  (239 s, 157/157) ; `-Case Return` **PASS** ; traductions à jour (0 clé inconnue) ; captures relues. Restes :
  « Résultat » et « Quitter vers la liste » depuis le menu non exercés de bout en bout. **D2 lancée** (réservation au
  salon tant que le joueur est dans la partie finie, puis 2 min).
- 2026-10-25 : **Réponses de l'utilisateur.** (1) **M6-Q14** : un joueur resté dans la partie finie **n'est plus dans
  le salon** (aucune réservation) ; le chef peut relancer sans lui ; « Retour au salon » = raccourci pour rejoindre ce
  salon sous le même nom ; place prise, salon plein, disparu ou relancé → renvoyé vers la liste Multijoueur avec un
  message. D2 réorientée en cours de boucle. (2) « Forum » en japonais : **町の中心** (machi no chūshin) ; fait pour
  la marque des stats (`fill_po.py`, .locres à régénérer) ; V4 sur `main` dit « フォーラム » (messages de chute) :
  à aligner à la fusion ; le bâtiment s'appelle « 本拠地 » (Main Center) : à confirmer avec l'utilisateur.
  (3) R5.5 : go (après D2, machine au calme). (4) Fusion de `main` et commit : go après D2 et R5.5.
- 2026-10-25 : **D2 rendue (gestionnaire seul) : M6-Q14 fait côté C#** (non commité). Après `finished` : salon `lobby`,
  réglages gardés, **aucun joueur ni réservation** (ancien jeton `not_in_lobby`) ; « Retour au salon » = `join`
  ordinaire (refus `name_taken`, `lobby_full`, `lobby_not_found`, `lobby_in_game`) ; premier revenu chef, « Lancer »
  sur les seuls revenus ; salon vide gardé tant que le serveur de la partie tourne. `server_stopped` (serveur arrêté sans
  `finished`, joueurs sortis malgré eux) garde la réservation de 2 min (choix de la session). xUnit **rouge** (7) puis
  **vert** 163/163 ; `PROTOCOL.md` à jour. Première version (route `left`) faite puis retirée ; **le serveur cuit
  contient encore `SendLeft`** (404 sans effet) : re-cuire. Restes → **D2b** : client (refus du retour → liste et
  message), `lobby_match`, `-Case Return`, re-cuisson, `quick_check`.
- 2026-10-25 : **Précision de l'utilisateur sur M6-Q14** : le chef reste chef du salon même resté dans la partie
  finie ; nouveau chef seulement si le salon est plein et qu'il n'est pas revenu. Ajout de la session (confirmé par l'utilisateur) :
  aussi s'il quitte la partie sans revenir ou si le serveur s'arrête (route `left` reprise). Consigne passée à D2b
  en cours.
- 2026-10-25 : **D2b rendue (15 min)** (non commité). Build et 156 tests verts au départ (rien de perdu). Client :
  refus du retour → liste et message, `MakeReturnToLobbyError` (`lobby_in_game` → « La partie a été relancée sans
  vous. »), test `UI.ManagerErrorText.ReturnToLobby` **rouge** puis vert (157/157), traductions (302 textes, 0 clé
  inconnue). Serveur re-cuit (10:18, sans `SendLeft`). `-Case Return` et `lobby_match` adaptés (chef absent gardé)
  **mais pas lancés** ; `dotnet test` 163/163. **Règle du chef pas faite côté gestionnaire** → **D3 lancée** (xUnit
  rouges, route `left`, `SendLeft`, re-cuisson, essais).
- 2026-10-25 : **D3 rendue : règle du chef faite** (non commité). Gestionnaire : `AwayLeader` / `AwayLeaderUntil` /
  `LeaderName`, route `POST /matches/{id}/left` (127.0.0.1), nouveau chef si plein, ou 30 s après le `left` du chef
  ou l'arrêt du serveur (`LeaderReturnGrace` : le client quitte le serveur avant son `join`, sans ce délai le chef qui
  revient perdrait la main) ; C++ `SendLeft` au `Logout` d'un joueur nommé. xUnit **rouge** (12) puis **vert**
  173/173 ; serveur re-cuit (10:26) ; `lobby_match` **PASS** (142 s : Bob revient seul, pas chef ; Alice revient par
  le menu et redevient cheffe) ; `-Case Return` **PASS** ; `quick_check -Parts tests,smoke,menu,victory,lobby`
  **PASS** (249 s, 157/157). Reste : cas « refus au retour » de bout en bout (couvert par xUnit et la règle client).
- 2026-10-25 : **R5.5 mesuré** (machine au calme, mycrusader-ea en pause ; S3 1000 contre 1000, 2 clients de l'éditeur
  par serveur ; `net_match.ps1` gagne `-Port` et `-Tag`, nouveau `Tools/Tests/sample_load.ps1`). Une bataille : 9,70
  ms/pas moyen (max 19,3), 23,7 Ko/s par client ; **deux batailles en même temps** : 10,19 et 10,37 ms/pas (max 31,9 et
  38,8), 20,0 et 23,1 Ko/s par client (seuil D450 ≤ 34) ; serveur ~330 Mo, stable ; PC 65 % (max 85 %) de processeur,
  34,5 Go libres au plus bas, les 4 clients de l'éditeur pèsent 9,4 Go et dominent la charge. Trois batailles non
  mesurées. **Recommandation `MaxServers = 2`** (défaut) ; décision à l'utilisateur. **Écart à signaler** : la
  référence d'une seule bataille fait 9,7 ms/pas contre ~6 ms connus (seuil 6 ms) : déjà vu en C2b (6,6 ms avec ou
  sans l'enregistreur) ; Glorfindel n'a pas encore les commits de perf de `main` (a37995c…) : à remesurer après la
  fusion, sinon à confier à la session perf. p95 non disponible (seulement moyenne et max).
- 2026-10-25 : **Commit sur `net-playtest` : 5fd9b87** « M6 R4-R5: lobby screens, lobby to match and back,
  spectators, match statistics » (tout le travail de R4, R5.1-R5.5, C1-C7, D1-D3). **Fusion de `main` en cours** :
  conflits de code (`HOLGameState.cpp` : événements des stats + `SendForumEvent` de V4 ; `HOLGameScreen.cpp` :
  `StatsButtonName` + `AlertPanelName`) résolus en gardant les deux côtés ; localisation à régénérer ; « フォーラム »
  de V4 remplacé par « 町の中心 » (mycrusader-ea prévenu). Boucle de fin de fusion lancée (localisation, build,
  re-cuisson, `quick_check -Full` + `lobby`, `dotnet test`). Ensuite : commit de la fusion, accord des sessions,
  avance rapide de `main`, journal et `EtapeDefini.md`.
- 2026-10-25 : mycrusader-ea d'accord pour la fusion ; la décision D du terme japonais « 町の中心 » (Forum) est à
  poser par ce chantier au report au GDD (clôture, R9 / E7), avec M6-Q10 à M6-Q14.
- 2026-10-25 : **Dans `main` : 3713f76** (avance rapide, accord de mycrusader-ea et -e7 ; 5fd9b87 = R4-R5 et C / D,
  c2bcfae et 3713f76 = fusions de `main`). Fin de fusion : localisation 311 textes (0 clé inconnue, 町の中心 partout),
  build, re-cuisson, `quick_check -Full` (FAIL `net 2 clients` dû à la fusion : motifs du bandeau et du tableau
  corrigés) puis `-Parts net,victory,lobby` **PASS**, `tests,smoke` **PASS** 165/165 après la 2e fusion, `dotnet test`
  173/173. **Prochaine étape : essais E2 et E3 de l'utilisateur** (consigne à lui donner) ; à décider : `MaxServers`
  (2 recommandé), « 本拠地 » → « 町の中心 » ? ; à remesurer : 9,7 ms/pas d'une seule bataille S3 (seuil 6).
- 2026-10-25 : **E2 / E3 prêts (R5.6).** Paquet `HeroesOfLiberty-Glorfindel\Saved\Packages\2026-10-25-3713f76\`
  (jeu et serveur à la même version, `package.ps1` fait les deux) ; `Manager\` copié dans le paquet (`Gestionnaire.bat`,
  `appsettings.json` : port 7700, `ServerExe` du paquet, `GameVersion` écrit en dur, **`MaxServers` 2**) ;
  `Windows\servers.json` ; lanceurs `Joueur_Alice/Bob/Carol/Dan/Eve.bat` (fenêtrés, nom et Saved propres).
  `packaged_smoke` **PASS** ; variante empaquetée de `lobby_match` (scratchpad) **PASS** en 129 s. Non essayés :
  deux salons à la fois, gestionnaire par double-clic. À corriger un jour : le gestionnaire cherche `version.txt` à
  côté de `ServerExe` (dans le paquet il est dans `WindowsServer\HeroesOfLiberty\`) ; `Manager\` et les `.bat` ne sont
  pas produits par `package.ps1` (option `-Playtest` ?). **Consigne donnée à l'utilisateur ; on attend ses retours.**
- 2026-10-25 : **Version du jeu trouvée dans le paquet.** Test rouge `StatusTests.VersionIsReadFromThePackagedServer`
  (« dev » au lieu de la version), puis `ManagerSettings.ResolveGameVersion` lit aussi
  `<dossier du serveur>\HeroesOfLiberty\Version.txt` ; `PROTOCOL.md` à jour ; `dotnet test` **174/174**. Non commité,
  paquet d'essai inchangé (son `GameVersion` écrit en dur reste valable). Reste : `-Playtest` de `package.ps1`.
- 2026-10-25 : accord donné à mycrusader-ea pour avancer `main` en ff vers `victory-v5` b9a7e06 (Victory V5, Régicide).
  Sa partie `lobby` de `quick_check` n'a pas été jouée (pas de serveur empaqueté dans Arwen) : **à jouer ici à la
  prochaine fusion de `main` dans `net-playtest`** (la partie `net` remplace l'abandon par la mort du Roi).
  Fait par ea : `main` = b9a7e06 puis doc b7b6350 (GDD D470-D474) ; **prochaine décision libre : D475** (pour la
  décision « Forum » = 町の中心 et celles de M6 à la clôture).
- 2026-10-25 : **Retours E2 de l'utilisateur** (paquet 2026-10-25-3713f76) :
  - **E2-1** : au « Retour au salon », on ne garde pas sa couleur de la partie ;
  - **E2-2** : il faut un choix de couleur « Aléatoire » ;
  - **E2-3** : dans le salon, « Quitter le salon » puis « Retour » (liste Multijoueur) → écran sans boutons au lieu de
    l'écran titre (Solo, Multijoueur, Options…). Piste : après un retour de partie, `HOLMenuPlayerController::BeginPlay`
    empile titre + Multijoueur + salon d'un coup.
  Boucles prévues : E2-3, puis E2-1, puis E2-2 ; puis nouveau paquet pour la suite des essais.
  À vérifier : `dotnet test` affiche « Now listening on: http://0.0.0.0:7700 » (un test démarre-t-il un vrai Kestrel ?
  conflit possible avec le gestionnaire de l'utilisateur pendant ses essais). → Non : `WebApplicationFactory` (mémoire).
  Commit **4b03cd1** sur `net-playtest` (« go » de l'utilisateur) : E2-1, E2-2, E2-3, version trouvée dans le paquet.
- 2026-10-25 : **Essais de l'utilisateur sur le paquet 2026-10-25-3713f76-e2 : « tout fonctionne ».** Demande **E2-4** :
  pour la lisibilité, les lignes sans personne (Fermé, Ouvert) de l'écran de nouvelle partie / salon affichent des
  champs vides (pas de faction, couleur, équipe), en solo et en réseau.
  → Fait (voir plus bas) ; commit **c3f2417** sur `net-playtest` (« commit » de l'utilisateur) : E2-4, `lobby_match`
  aligné sur E2-1, `package.ps1 -Playtest`. `main` pas encore avancée.
  Validé par l'utilisateur (« parfait ») : E2-1 = le gestionnaire rend au retour la couleur de la partie si elle est
  libre ; E2-2 = choix « Aléatoire » dans la liste des couleurs (solo et multijoueur), choisissable par plusieurs,
  tiré au lancement parmi les couleurs non prises ; un nouvel arrivant garde la couleur donnée aujourd'hui.
- 2026-10-25 : **E2-3 corrigé (titre perdu sous la liste Multijoueur)**, worktree Glorfindel, non commité.
  Cause (Common UI, `UCommonActivatableWidgetContainerBase::SetSwitcherIndex`) : quand on empile un écran pendant le
  fondu d'un autre empilement, l'écran « affiché » (déjà désactivé parce que recouvert) est pris pour un écran fermé
  par ailleurs et retiré de la pile à l'arrivée du fondu. `BeginPlay` après un « Retour au salon » empile titre puis
  Multijoueur dans la même image, puis le salon dès la réponse du gestionnaire, encore pendant le fondu : le titre
  sortait de la pile, « Retour » sur la liste laissait la couche vide (fond seul). Chemin normal (titre → Multijoueur →
  créer → Quitter → Retour) sain : chaque empilement y attend la fin du fondu (clics bloqués pendant le fondu).
  Correctif : `UHOLScreenStack` (`Widgets/HOLScreenStack.h/.cpp`), pile des deux couches de `UHOLRootLayout` ; à un
  empilement, si l'écran affiché est désactivé alors que le fondu monte encore dans la pile (index visé > index
  actif), il est oublié comme écran affiché et reste dans la pile (log « Screen stack: … kept under … »).
  Test : partie `menu` de `quick_check` (fr), cas « menu stack » : `hol.UI.Open Multiplayer`, 2 images,
  `hol.UI.Open NewGame`, Retour, Retour, capture `qc_fr_stackback` attendue `menu=HOLTitleScreen`. **Rouge** avant
  (`menu=none`), **vert** après (titre et ses quatre boutons sur la capture). Le lancement du `quick_check` passe
  désormais `-ServersFile` vers un gestionnaire absent (port 7799) pour ne jamais interroger celui de l'utilisateur.
  `quick_check -Parts menu` **PASS** ; `quick_check` par défaut **PASS** (tests 165/165, smoke). Non essayé :
  le vrai retour de partie empaqueté (à refaire au prochain paquet).
- 2026-10-25 : **E2-1 corrigé (couleur de la partie rendue au « Retour au salon »)**, worktree Glorfindel, non commité.
  Cause : depuis D2, `finished` vide le salon de ses joueurs (`LobbyStore.ServerFinished`) ; « Retour au salon » est un
  `join` ordinaire, qui donnait la première couleur libre (`LobbyStore.Enter`) : la couleur de la partie était perdue.
  Correctif (gestionnaire seul) : `Lobby.LastMatchColors` (nom → couleur, casse ignorée), rempli par
  `ServerFinished` avant de vider les emplacements ; `Enter` donne cette couleur si aucun joueur présent ne l'a, sinon
  la première libre comme avant (un nouvel arrivant ne change pas). Client C++ vérifié sans changement : il n'envoie
  `PUT /me` que sur un choix de l'utilisateur (`SetSelectedIndex` ne diffuse pas), rien n'écrase la couleur au retour.
  PROTOCOL.md (join et « Retour au salon ») mis à jour.
  Tests (`ReturnToLobbyTests`) : `ThoseBackTakeTheirMatchColorAgain` (retour dans un autre ordre, « alice » en
  minuscules : 2, 3, 0) **rouge** avant, vert après ; `AMatchColorTakenMeanwhileGivesTheFirstFreeOne` (couleur prise
  entre-temps → première libre). `dotnet test` **176/176**. Non essayé : le vrai retour de partie empaqueté.
- 2026-10-25 : **E2-2 fait (couleur « Aléatoire » à l'écran de nouvelle partie, solo et salon)**, worktree Glorfindel,
  non commité. Représentation : l'index de couleur juste après la palette, 8 = « Aléatoire »
  (`HOLPlayerColors::RandomColor` = `NumColors`, `LobbyRules.RandomColor` = `ColorCount`) ; dernière option de la liste
  (LOCTEXT `HOLNewGameScreen,RandomColour`, pastille grise `TextMuted`), jamais grisée ni « prise ».
  Gestionnaire : `PUT /me` accepte 0 à 8, 8 sans `color_taken` (plusieurs joueurs) ; le tirage fait foi au lancement,
  dans `MatchConfig.FromLobby` (graine de la partie) : par ordre d'emplacement, une couleur qu'aucun joueur n'a (choisie
  ou déjà tirée) ; match.json n'a que 0 à 7, le serveur de jeu ne change pas ; le salon garde « Aléatoire » (et
  `LastMatchColors` aussi : un joueur sur « Aléatoire » le retrouve au retour). Un nouvel arrivant garde la première
  couleur réelle libre. PROTOCOL.md (`PUT /me`, `slots[].colorIndex`) à jour.
  Solo : `FHOLMatchConfig::DrawRandomColors(FRandomStream&)`, appelé par `UHOLNewGameScreen::Launch` avec la graine,
  avant `Normalize` (qui borne toujours à 0-7). Écran : `IsColorTaken` faux pour « Aléatoire », le salon ne borne plus
  la couleur lue à 7.
  Tests : xUnit `SeveralPlayersCanChooseRandom` (LobbyLineTests), `RandomColorsAreDrawnAmongTheFreeOnesAtLaunch` et
  `RandomColorsVaryWithTheSeed` (MatchStartTests) **rouges** (3) avant, verts après ; borne hors limites passée de 8 à 9.
  `dotnet test` **179/179**. C++ `HeroesOfLiberty.Match.RandomColors` (écrit avant le code ; rouge = ne compilait pas,
  pas de construction séparée). `Toolsuild.ps1` **PASS** ; `quick_check -Parts tests,menu,lobby` **PASS**
  (tests 166/166, menu fr/en/ja, lobby).
  Reste : japonais — entrée ajoutée à `Tools/Localization/fill_po.py` (« Random », « ランダム ») mais pas de
  rassemblement (gather Game.ini) ni de .locres régénéré ; non vérifié à l'œil (liste de l'écran, pastille grise) ;
  pas de test xUnit du retour au salon sur « Aléatoire ».
- 2026-10-25 : **Paquet E2 corrigé prêt** : `HeroesOfLiberty-Glorfindel\Saved\Packages\2026-10-25-3713f76-e2\` (jeu et
  serveur à la même version, `package.ps1 -Version 2026-10-25-3713f76-e2` : le nom par défaut aurait repris celui du
  paquet précédent, HEAD toujours 3713f76 ; 3,9 min). Contient E2-1, E2-2, E2-3 et la version lue dans le paquet (non
  commités). Mise en place comme le précédent (laissé intact) : `Manager\` (`build_manager.ps1`, `Gestionnaire.bat`,
  `appsettings.json` port 7700, `ServerExe` du paquet, `MaxServers` 2, **`GameVersion` vide**), `Windows\servers.json`,
  `Joueur_Alice/Bob/Carol/Dan/Eve.bat`.
  Japonais : GatherText + `fill_po.py` (**312 textes, 0 clé inconnue**, en et ja) + GatherText, .locres recompilés
  (`HOLNewGameScreen,RandomColour` = « ランダム »). Attention : sous PowerShell 5.1, `-config=Config/Localization/Game.ini`
  doit être entre guillemets (sinon coupé au point : « Loading Config File ...Game failed »).
  xUnit : `ReturnToLobbyTests.ThoseOnRandomFindRandomAgain` (Alice et Chloé sur « Aléatoire » le retrouvent, Bob 0) ;
  `dotnet test` **180/180**. « Now listening on: http://0.0.0.0:7700 » pendant `dotnet test` : simple message, aucun
  vrai Kestrel (`WebApplicationFactory` = TestServer ; la journalisation reprend l'URL de `Program.cs`) ; vérifié en
  tenant le port 7700 pendant `dotnet test` : 180/180, pas de conflit possible avec le gestionnaire de l'utilisateur.
  Essais du paquet : `packaged_smoke` **PASS** ; variante empaquetée de `lobby_match` (scratchpad
  `packaged_lobby_match_e2.ps1`, contrôle « couleurs neuves » inversé : chacun retrouve la sienne) **PASS** en 113 s :
  `/status` gameVersion = `2026-10-25-3713f76-e2` avec `GameVersion` vide, Bob revenu le premier retrouve 5, puis
  Alice 2 ; journaux des clients : « Screen stack: HOLTitleScreen kept under ... (pushed while fading) » au retour
  (E2-3 vu dans le vrai retour de partie). Captures du jeu empaqueté, liste des couleurs ouverte (scratchpad
  `colour_list_fr.png` / `colour_list_ja.png`) : « Aléatoire » / « ランダム » en dernier, pastille gris clair, lisible.
  Reste : commit des correctifs E2 ; option `-Playtest` de `package.ps1` (Manager\ et .bat) ; non essayés : tirage
  « Aléatoire » dans une vraie partie en salon, deux salons à la fois, gestionnaire par double-clic.
  **Paquet prêt pour la suite des essais de l'utilisateur.**
- 2026-10-25 : **-Playtest de `package.ps1` fait (mise en place de l'essai réseau sur un PC)**, worktree Glorfindel
  (net-playtest, 4b03cd1), non commité. `Tools\Package\add_playtest.ps1 -PackageDir <paquet> [-Root] [-Players]
  [-NoBuild]` : publie le gestionnaire (`build_manager.ps1`), copie `HOLManager.exe` et
  `HOLManager.staticwebassets.endpoints.json` dans `Manager\`, écrit `Manager\Gestionnaire.bat`,
  `Manager\appsettings.json` (0.0.0.0:7700, `..\WindowsServer\HeroesOfLibertyServer.exe`, GameVersion vide, 2 serveurs,
  port 7777), `Windows\servers.json` (127.0.0.1:7700) et `Windows\Joueur_Alice/Bob/Carol/Dan/Eve.bat` ; modèles dans
  `Tools\Package\Playtest\` (`{Name}` dans `Joueur.bat`), .bat forcés en CRLF, .json en LF sans saut final.
  `package.ps1 -Playtest` l'appelle après Version.txt (échec → FAIL). Test `Tools\Tests\playtest_setup.ps1` (rouge
  d'abord : script absent) : faux paquet dans le scratchpad, contenu octet par octet, gestionnaire publié lancé sur
  7790 (`--urls`), `/status` gameVersion = Version.txt factice, arrêté par PID → **PASS** (avec et sans -NoBuild).
  Comparé au paquet E2 : identique octet par octet, sauf `Joueur_Dan/Eve.bat` du paquet fait main en LF (mêmes
  commandes). Pas essayé : un vrai `package.ps1 -Playtest` (UAT complet).
- 2026-10-25 : **E2-4 fait (lignes sans personne vides à l'écran de nouvelle partie, solo et salon)**, worktree
  Glorfindel (net-playtest, 4b03cd1), non commité. `UHOLNewGameScreen::Refresh` : une ligne non occupée (Fermé en solo,
  Ouvert / Fermé en salon : `Config.Slots[i].State == Closed`) met faction, pastille, couleur et équipe en `Hidden`
  (la place reste, les colonnes ne bougent pas ; hors de la navigation clavier, qui saute l'invisible) ; Visible dès
  qu'elle est prise. Valeurs gardées en mémoire ; `IsColorTaken`, couleur par défaut, « Aléatoire » inchangés (une ligne
  fermée ne comptait déjà pas). Nouvelle commande `hol.UI.Shown <Widget>...` (journal « Shown SlotTeam3: no »).
  quick_check menu fr : marques `menu emptyrow` (ligne 3 fermée vide, ligne 0 affichée, capture `qc_fr_emptyrow`) puis
  `menu takenrow` (`hol.UI.Choose SlotState3 1` → champs revenus, capture `qc_fr_takenrow`) : **rouge** avant (« closed
  line empty » KO), vert après ; captures regardées avant / après, et `lobby_match_lobby.png` (lignes Ouvert vides).
  `lobby_match.ps1` : le contrôle « colours kept: old lines » contredisait E2-1 (validé) et faisait échouer `lobby` ;
  inversé (chacun retrouve la couleur de la partie). `Tools\build.ps1` **PASS** ; `quick_check -Parts tests,menu,lobby`
  **PASS** (tests 166/166, menu fr/en/ja, lobby ; 168 s).
- 2026-10-25 : **Fusion de main (V5 Régicide) dans net-playtest : 298ea5e**, worktree Glorfindel (main a080862 : V5
  a03d4c8 / b9a7e06, GDD D470-D474, lanceur L0-L3 ; main non avancé). Code fusionné sans conflit (HOLGameState,
  HOLCombatProcessor, HOLUnitOrderProcessor, HOLPlayerController, HOLCommandGrid, mode de jeu, UnitTypes,
  `quick_check.ps1` — partie `net` de ea avec le Roi et 7 rapports + nos cas menu stack / emptyrow / takenrow /
  servers_none —, `fill_po.py`, PROTOCOL.md). Conflits seulement dans la localisation générée : en-têtes des .po
  (dates, HEAD gardé), .archive / .locres / manifeste refaits (GatherText, `fill_po.py` **315 textes, 0 clé inconnue**
  en et ja, GatherText) ; « ランダム », « 町の中心 » et « 王 » dans le .locres ja. `build.ps1` **PASS** ; serveur
  re-cuit (`net_match.ps1 -Stage` PASS, ses 3 processus arrêtés par PID) ; `quick_check -Parts tests,menu,victory,net,lobby`
  **PASS** en 283 s (tests 172/172, victory / forum / regicide, menu fr/en/ja, net 2 clients, **lobby**) ;
  `dotnet test` **206/206** (180 + 26 `VersionRoutesTests` de L2).
- 2026-10-25 : **`main` avancée en ff à 298ea5e** (aucune autre session active ; accord de l'utilisateur pour la
  fusion). `main` contient E2-1 à E2-4, `-Playtest`, la version trouvée dans le paquet. Copie principale à recompiler.
  Suite : **R7** (déconnexion, fenêtre de 3 min).
- 2026-10-25 : **R7 lancé** (« go » de l'utilisateur). Boucle 1 : R7.1 à R7.3 (règle, état gardé, défaite à la fin
  de la fenêtre), dans le worktree Glorfindel, branche `net-playtest` (= `main` 298ea5e + doc).
- 2026-10-25 : **R7 boucle 1 (R7.1 à R7.3, côté serveur) faite**, worktree Glorfindel (net-playtest, 298ea5e), non
  commité. Règle pure `FHOLDisconnectRules` (`Match/HOLDisconnectRules.h/.cpp`) : `KeepsSlot` (configuration nommée,
  emplacement humain nommé, partie lancée et pas finie, emplacement non vaincu), `IsWindowOver`, `GetSecondsLeft` (pour
  R7.5), `FindAbsent`, `GetExpired`, `GetStockOnTaking` ; fenêtre par défaut 180 s. `AHOLGameState` (serveur) :
  `AbsentSlots` (emplacement, nom, heure du départ, stock), `KeepAbsentSlot` (Logout), `KeepMissingSlot` (absents au
  départ de R3.9 : fenêtre ouverte au départ de la partie avec `StartingResources`), `GiveSlotStock` (PostLogin : stock
  gardé sinon celui du départ), `DefeatExpiredAbsences` → `DefeatSlot(Slot, Disconnected)` (nouvelle raison, même
  chemin qu'un abandon : effondrement de V1, règles des camps) ; une défaite pendant l'absence (Roi tué, forum) retire
  l'absence. Mode de jeu : `Logout` garde l'emplacement, minuterie 1 s `UpdateAbsences` tant qu'il y a des absents ;
  `PostLogin` passe par `GiveSlotStock` ; réglage `hol.Match.DisconnectWindowSeconds` (console) ou
  `-HOLDisconnectWindow=<s>` (ligne de commande). Le retour sous le même nom marchait déjà (`FindSlotForName` : la
  place se libère avec le PlayerState) mais repartait de `StartingResources` ; unités et bâtiments restent (rien ne
  les retirait). Journaux « Disconnect: slot N (Nom) kept / back / not back within W s ».
  Tests (`Tests/HOLDisconnectTests.cpp`) : `HeroesOfLiberty.Match.DisconnectRules` (vert d'emblée : la règle a été
  écrite avec le test) et `HeroesOfLiberty.Match.DisconnectGameState` (Bob 300 bois → départ → retour = 300 bois ;
  re-départ, fin de fenêtre → vaincu `Disconnected`, plus absent, partie pas finie grâce à l'IA de son camp ; solo :
  rien gardé) : **rouge** avec l'API en ébauche (173/174), vert après. `Toolsuild.ps1` **PASS** ;
  `quick_check -Parts tests` **PASS** 174/174.
  Gestionnaire : `POST /matches/{id}/left` au Logout ne retire personne du salon ni de la partie (`LeftBeforeEnd`,
  M6-Q14) ; le retour direct au serveur n'est pas gêné. Écart sans changement de protocole : un chef revenu dans la
  fenêtre reste dans `LeftBeforeEnd`, donc à `finished` il perd la direction comme s'il n'était pas revenu ; à régler
  par un message « revenu » (`POST /matches/{id}/back`, à décider) — noté, pas fait. Si tous partent, `empty` arrête
  toujours le serveur (fenêtre perdue) : inchangé.
  Reste : pas d'essai réseau (net:rejoin = R7.8) ; pendant l'absence les dépôts de ses Paysans et ses remboursements
  vont à un PlayerState absent (perdus) et sa production ne paie rien : à garder dans `AbsentSlots` ; R7.4 (propriétaire
  réseau au retour : `AssignBuildingOwners` est déjà appelé dans PostLogin, à vérifier à 2 clients), R7.5 (bandeau :
  répliquer `AbsentSlots`), R7.6, R7.7 (texte de la raison « déconnecté » si l'écran de fin l'affiche), R7.8, R7.9.
- 2026-10-25 : **Demande du chantier lanceur (L-Q7, choix de l'utilisateur : « B confié à la session réseau »)** :
  publier une version doit mettre à jour le **serveur de partie** sans couper les parties en cours. À faire ici, dans le
  gestionnaire : (1) un dossier serveur par version (`Servers\<v>\`, copié de `<archive>\WindowsServer\` par
  `Tools/Package/publish.ps1` du lanceur, à étendre) ; (2) la version du jeu relue **à chaque nouveau salon** (plus une
  fois au démarrage, `Program.cs` / `LobbyStore`), = la dernière publiée (`Versions\latest.txt`, L2) ; (3) chaque
  partie lancée depuis le dossier serveur de sa version (`ProcessServerLauncher`), celles en cours finissent sur la
  leur ; (4) **nettoyage** : le dossier serveur d'une ancienne version supprimé quand sa dernière partie se termine
  (et par `publish.ps1` en filet), un seul serveur de 3 Go en permanence. Détail et contexte : `LauncherInProgress.md`
  (L2 : dossier des versions, L4 : publication). En attendant : mise à jour du serveur à la main.
  → Commit **4a88040** sur `net-playtest` (« go » de l'utilisateur). `main` inchangée. Suite : R7.4 et la suite ; deux points à décider (chef revenu, gains des Paysans pendant l'absence).
- 2026-10-25 : (session lanceur, L10) **R8.1 commencée** : Tailscale installé sur le PC de l'utilisateur,
  `pc-belongfr-03` = **100.123.190.53** ; règles de pare-feu par port (TCP 7700, UDP 7777-7799, profil Privé,
  limitées à 100.64.0.0/10) demandées à l'utilisateur. Détail : `LauncherInProgress.md`.
- 2026-10-25 : (session lanceur) **L-Q8 = A, revient sur R-Q2 pour les amis** : l'utilisateur ne veut pas que ses amis
  installent Tailscale ; ils n'auront que le kit du lanceur et joindront le PC par **redirection de ports de la box**
  (TCP 7700, UDP 7777-7799), IP publique 90.55.168.58 (pas de CGNAT), avec un **mot de passe partagé** à concevoir
  (gestionnaire + client du jeu `HOLManagerClient` : à coordonner avec cette session). Détail : `LauncherInProgress.md`.
- 2026-10-25 : (session lanceur) **L11 dans `main`** : clé d'accès partagée (`X-HOL-Key`, `access_key.txt` du
  gestionnaire, `key` de `servers.json`, `-HOLAccessKey=` / option `HOLKey` vérifiée dans `PreLogin`), section
  « Clé d'accès (L11) » de `Manager/PROTOCOL.md`. Touche `HOLManagerClient`, `HOLLocalPlayer`, `GameModeBase::PreLogin`,
  `ProcessServerLauncher` : conflits possibles avec R7 (4a88040) à la fusion. À savoir : le test parent
  `HeroesOfLiberty.UI.ManagerErrorText` n'est jamais lancé (chemin parent de `…ReturnToLobby`), `quick_check` compris.
