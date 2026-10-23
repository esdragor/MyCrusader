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
| M6-Q7 | Fin : **« Retour au salon »** ou **« Quitter vers la liste »** ; mêmes réglages, « Prêt » à zéro, chef au premier revenu, retour d'office après ~60 s |
| M6-Q8 | **Spectateur** : « Regarder » une partie en cours ; vision complète (D376), sans discussion pour l'instant |
| M6-Q9 | **Le salon vit dans le gestionnaire** ; un serveur Unreal ne fait qu'**une partie**, lancé avec `-MatchConfig=` (chemin de M2.5), **emplacement donné par le nom**, nom absent = spectateur |
| R-Q2 | **Tailscale** (partage du PC) ; installation guidée avant l'essai à deux PC |
| R-Q4 | **Équipes respectées par le combat** avant le premier essai |
| R-Q5 | **Fenêtre de D71 (~3 min) puis défaite** ; bandeau à compte à rebours ; retour sous le même nom |
| R-Q7 | **Development**, triches refusées par le serveur sauf `-HOLCheats` |
| R-Q8 | **Zip par version**, version affichée, refus d'une version différente ; lanceur en chantier à part |

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
 (Multijoueur → liste → salon → partie ?Name=…&Version=…)
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
| R4.7 Vers la partie | salon « en partie » → connexion au port donné avec `?Name=…&Version=…` ; « Regarder » → spectateur | 2 clients dans la partie, chacun à son emplacement |
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

- **État au 2026-10-23 soir** : worktree `HeroesOfLiberty-Glorfindel`, branche `net-playtest` (depuis `main` f7269a8,
  **rien commité**, `main` a avancé depuis : à fusionner avant commit). **R1 fait jusqu'à R1.7**, essai **E1** en
  attente de l'utilisateur (paquet `Saved\Packages\2026-10-23-f7269a8`). **R2 fini** (158/158 `dotnet test`), choix
  ouverts de `Manager\PROTOCOL.md` soumis à l'utilisateur (surtout : emplacement fixe dans le salon). Prochaines
  étapes : E1 ; R6 (équipes) dans un autre worktree ; R3 (serveur C++ : R3.1 lit
  `Manager/HOLManager.Tests/Data/match_v2_reference.json`, champ `playerName`). Outils : `Tools\Package\package.ps1`,
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
