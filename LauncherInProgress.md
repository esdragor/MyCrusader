# Launcher in progress

Journal du chantier **lanceur** : un petit programme que les amis installent une fois, qui **télécharge et met à jour**
le jeu tout seul, puis le lance. Ouvert le 2026-10-23, à la demande de l'utilisateur (réponse à R-Q8 de
`NetPlaytestInProgress.md` : « A, mais avoir un autre chantier en parallèle qui est de faire un launcher »).
Décisions nouvelles : à la suite de la review du GDD (prochain numéro libre au moment du report).

## Pourquoi

Le chantier réseau (`NetPlaytestInProgress.md`) livre le jeu **en zip par version** (R-Q8 = A). Le serveur et les
joueurs doivent avoir la **même version** (M6-Q1), sinon le joueur est refusé. Avec des mises à jour fréquentes, chaque
ami retéléchargerait tout le zip (1 à 3 Go, à mesurer). Le lanceur remplace ce geste.

## Ce dont il dépend

- Le **jeu empaqueté** et son **numéro de version** (R1 et R1.7 du chantier réseau).
- Le **refus d'une version différente** par le serveur (chantier réseau), que le lanceur évite en mettant à jour
  avant de lancer.
- Peut commencer **en parallèle** : la conception et le squelette ne dépendent de rien ; le téléchargement réel attend
  le premier paquet.

## Ce qu'il fait (premier jet, à trancher)

1. Affiche la version installée et la dernière version disponible.
2. Télécharge ce qui manque (de préférence **seulement les fichiers changés**).
3. Lance le jeu ; peut-être aussi « Rejoindre » le serveur connu (adresse, nom).
4. Sur le PC de l'utilisateur : met aussi à jour **le serveur** et le relance (M6-Q2 : serveur démarré avec Windows).

## Questions à trancher (une à la fois, options A / B / C, recommandation)

| Q | Question | Pistes |
|---|---|---|
| L-Q1 | Où sont rangées les versions ? | sur le PC de l'utilisateur, déjà allumé en permanence et joignable par Tailscale (R-Q2) ; un partage (Google Drive, OneDrive) ; GitHub Releases (dépôt privé) ; itch.io + `butler` (le lanceur n'aurait plus lieu d'être) |
| L-Q2 | Mise à jour complète ou par fichiers changés ? | liste des fichiers avec leur empreinte, seuls les changés téléchargés ; paquet `.pak` découpé par chunks pour que peu de fichiers changent ; zip complet |
| L-Q3 | Avec quoi l'écrire ? | piste : C#, comme le gestionnaire de salons (M6-Q6b du chantier réseau) ; C# (.NET, une fenêtre WPF ou WinForms, un seul `.exe`) ; Python empaqueté (PyInstaller) ; application Unreal (trop lourde) |
| L-Q4 | Que montre-t-il ? | version, barre de progression, « Jouer » ; nouveautés de la version ; état du serveur (allumé, joueurs dans le salon) |
| L-Q5 | Met-il à jour le serveur ? | oui sur le PC de l'utilisateur (mode « serveur ») ; non, script à part |
| L-Q6 | Se met-il à jour lui-même ? | oui (télécharge sa nouvelle version au démarrage) ; non (rare, à la main) |

Pistes venues du chantier réseau : le jeu lit l'adresse du PC de l'utilisateur dans un `servers.json` à côté de
l'exe (M6-Q5), que le lanceur peut tenir à jour ; le lanceur pourrait aussi montrer l'état des salons (L-Q4).

## Plan

Questions tranchées le 2026-10-25 (à reporter au GDD à la clôture, numéros D au moment du report) :

| Q | Décision |
|---|---|
| L-Q1 | Versions **sur le PC de l'utilisateur**, servies en HTTP par le gestionnaire de salons, par Tailscale |
| L-Q2 | **Liste des fichiers avec empreinte** (SHA-256), seuls les changés téléchargés ; *revue après L0 (L-Q2 bis = C)* : **pas de chunks pour l'instant**, fichiers **compressés au transfert**, et **correctifs binaires** des fichiers changés (l'exe surtout), le fichier complet en secours ; *outil après L3* : **HDiffPatch** (`hdiffz` à la publication, `hpatchz` livré avec le lanceur ; exe : 8,3 Mo au lieu de 124 Mo gzip) |
| L-Q3 | **C# (.NET 8), WinForms**, un seul `.exe` autonome, code commun avec le gestionnaire |
| L-Q4 | Versions installée / disponible, progression, **nouveautés**, « Jouer », **« Mettre à jour le jeu »** et **« Mettre à jour le lanceur »** ; pas d'état du serveur (il est dans le jeu) |
| L-Q5 | **Script de publication** dans `Tools/` (empaqueter, chunks, liste, nouveautés, dossier des versions du gestionnaire) ; le lanceur ne sert qu'aux joueurs |
| L-Q6 | Le lanceur **se met à jour lui-même, sur son bouton** |

Étapes de moins de 15 min, test rouge d'abord (xUnit, comme `Manager/HOLManager.Tests`). **Coordination** : le
gestionnaire (`Manager/`) est aussi travaillé par le chantier réseau (`net-playtest`, Glorfindel) ; L2 et L4 se font
après accord de cette session ou après la fusion de ses étapes en cours.

| Étape | Contenu | Fin |
|---|---|---|
| L0 Mesure | deux empaquetages successifs (`Tools/Package/package.ps1`), le second après un changement de code seul : quels fichiers changent, leur taille | tableau dans ce journal ; découpage décidé |
| L1 Bibliothèque commune | `Manager/HOLUpdate` : liste d'un dossier (chemin, taille, SHA-256), différence entre deux listes (à télécharger, à supprimer), lecture / écriture JSON | tests verts |
| L2 Routes du gestionnaire | `GET /versions/latest` (version, liste, nouveautés, version du lanceur, correctifs disponibles depuis les versions gardées), `GET /versions/{v}/files/{chemin}` (compressé, reprise par `Range`), `GET /versions/{v}/patches/{depuis}/{chemin}`, `GET /launcher/latest` ; `PROTOCOL.md` et sa version | tests verts |
| L3 Mesures (revue après L0) | (a) un changement de contenu (un asset, une table) : réécrit-il tout le `.ucas` ? (b) correctif binaire de l'exe entre deux versions de code : taille et durée avec `xdelta3`, `msdelta` (Windows, lisible depuis C#) et `HDiffPatch` ; outil choisi. Chunks seulement si (a) le demande | tableau ici, outil choisi |
| L4 Publication | `Tools/Package/publish.ps1` : `package.ps1`, liste par `HOLUpdate`, `Nouveautes.txt`, copie dans le dossier des versions, « dernière » marquée, N anciennes gardées, **correctifs des fichiers changés depuis chacune** (outil de L3), fichiers compressés ; les nouveaux serveurs de partie partent de la dernière, ceux en cours finissent sur la leur | deux versions publiées de suite, correctif présent |
| L5 Squelette du lanceur | `Manager/HOLLauncher` (WinForms, un seul exe autonome) : adresse lue dans le `servers.json` du jeu (M6-Q5), fenêtre (versions, nouveautés, « Jouer », deux boutons) | fenêtre qui lit `/versions/latest` |
| L6 Mise à jour du jeu | différence, téléchargement dans un dossier temporaire (**correctif si la version installée en a un, sinon fichier complet compressé**), correctif appliqué, empreinte vérifiée (échec : fichier complet), remplacement, fichiers retirés supprimés, barre de progression, reprise après coupure | tests verts avec `FakeServer` (correctif, secours) |
| L7 Mise à jour du lanceur | nouvel exe téléchargé à côté, exe en cours renommé, relance, ancien supprimé au démarrage suivant | test vert |
| L8 Jouer | lance le jeu ; jeu en retard : permis, avec l'avertissement « le serveur refusera cette version » | test vert |
| L9 Première installation | dossier vide : tout télécharger ; consigne en 3 lignes pour les amis | test vert |
| L10 Essai | deux versions publiées, lanceur dans un autre dossier (puis chez un ami par Tailscale) : mise à jour du jeu, du lanceur, « Jouer » | **essai de l'utilisateur** |

## Suivi

- 2026-10-23 : chantier ouvert, questions listées. Rien tranché, rien codé.
- 2026-10-25 : séance de questions ouverte (session EtapeDefini, en parallèle de H). **L-Q1 = A** (choix de
  l'utilisateur, conforme à la recommandation) : les versions sont rangées **sur le PC de l'utilisateur**, servies en
  HTTP par le gestionnaire de salons (M6-Q6b), joignable par Tailscale ; la version proposée est celle du serveur.
  Débit montant de l'utilisateur pas encore connu (pèse sur L-Q2). **L-Q2 = B** (conforme à la recommandation) :
  **liste des fichiers avec empreinte** (seuls les changés téléchargés) **et paquet du contenu découpé en chunks**
  (cartes, unités, interface, sons…) ; d'abord mesurer ce qui change entre deux empaquetages successifs (un changement
  de code seul) pour régler le découpage. **L-Q3 = A** (conforme) : **C# (.NET 8), WinForms, un seul `.exe`
  autonome** (~60 Mo, pas de .NET à installer), code commun avec le gestionnaire de salons (empreintes, protocole,
  HTTP). **L-Q4 = B**, avec la nuance de l'utilisateur : versions installée / disponible, barre de progression,
  « Jouer », **nouveautés** (texte court par version, rien si vide), **pas d'état du serveur** (précision de
  l'utilisateur : on lance le jeu, c'est dans le jeu qu'on le voit, écran « Rejoindre » M6-Q5), et
  **deux boutons : « Mettre à jour le jeu » et « Mettre à jour le lanceur »** (mise à jour à la demande, pas
  automatique). **L-Q6 tranchée par là** : le lanceur se met à jour lui-même, sur son bouton (technique : nouvel exe
  téléchargé à côté, l'exe en cours renommé, relance). Par défaut (non contesté) : chaque bouton signale qu'une mise à
  jour existe ; « Jouer » reste permis avec un jeu en retard (solo), en prévenant que le serveur le refusera (M6-Q1).
  **L-Q5 = A** (conforme) : script de publication dans `Tools/`. Toutes les questions tranchées ; plan écrit
  (L0-L10). Rien codé.
- 2026-10-25 : **L0 lancée** (feu vert de l'utilisateur), dans Elrond (`build-site-leftover` = `main` fb462ff,
  `IN_USE.txt` posé), en sous-agent. Trois empaquetages : `L0-a` (fb462ff), `L0-a2` (même code, rien changé : la
  cuisson et les `.pak` sont-ils identiques octet pour octet ?), `L0-b` (un changement de code seul, non commité). Pour
  chacun, liste du dossier joueur `Windows\` (chemin, taille, SHA-256, `.pdb` à part) ; comparaison a/a2 et a/b ici.
- 2026-10-25 : L0 : empaquetage `L0-a` (fb462ff tel quel) lancé dans Elrond, en tâche de fond (`package.ps1 -Version L0-a`).
- 2026-10-25 : L0 : `L0-a` PASS en 4,5 min (cuisson déjà faite dans Elrond) : `Windows\` 3188 Mo, `WindowsServer\` 3025 Mo.
  Liste de `L0-a` en cours, puis `L0-a2` (même code, rien changé).
- 2026-10-25 : L0 : `L0-a2` PASS en 1,4 min. Listes a et a2 faites (50 fichiers, 620,5 Mo hors `.pdb` ; un `.pdb` de
  2568 Mo). **a/a2 : 3 fichiers changés** (`Version.txt`, deux `Manifest_*.txt`), 0,25 Mo : `.pak/.ucas/.utoc` et exe
  identiques octet pour octet. Puis `L0-b` (texte d'un `UE_LOG` de `HeroesOfLibertyGameModeBase.cpp`).
- 2026-10-25 : L0 : `L0-b` PASS en 3,7 min (changement d'un texte de `UE_LOG`, fichier remis ensuite). Liste faite.
- 2026-10-25 : **L0 close.** Dossier joueur `Windows\` : 50 fichiers, 620,5 Mo hors `.pdb` (+ `HeroesOfLiberty.pdb`
  2568 Mo, jamais aux joueurs). Contenu cuit en **IoStore, un seul paquet, pas de chunks** (`bGenerateChunks` éteint) :
  `HeroesOfLiberty-Windows.ucas` 129,6 Mo, `.utoc` 0,1 Mo, `.pak` 24,7 Mo (fichiers hors IoStore), `global.ucas`
  3,3 Mo ; soit ~158 Mo de contenu. Le reste : exe du jeu **332 Mo** (Development, monolithique), DLL et redist ~120 Mo.

  | Comparaison | Changés (hors `.pdb`) | À télécharger / total hors `.pdb` |
  |---|---|---|
  | a / a2 (rien changé) | 3 : `Version.txt`, `Manifest_NonUFSFiles`, `Manifest_UFSFiles` (dates) | 0,25 / 620,5 Mo (0,04 %) |
  | a / b (code seul) | 5 : les mêmes + `Manifest_DebugFiles` + `HeroesOfLiberty.exe` | 332,2 / 620,5 Mo (54 %) |

  Cuisson **déterministe** : sans changement, `.pak/.ucas/.utoc` et exe identiques octet pour octet ; un changement de
  code seul ne touche pas le contenu cuit. Le poids d'une version de code est **l'exe** : il grandit de 1,5 Ko, tout
  se décale, 97 % de ses blocs de 64 Ko diffèrent (100 % en blocs de 1 Mo) ; compressé (gzip) : 124 Mo.
  **Proposition pour L3** : découper le contenu en chunks est **inutile pour l'instant** (158 Mo, intact d'une version
  de code à l'autre, déjà sauté par la liste d'empreintes). À la place : fichiers compressés au transfert (gzip,
  exe 332 → 124 Mo) dès L2/L6 ; L3 devient une mesure d'un changement de contenu, et les chunks seulement si un petit
  changement d'asset réécrit tout le `.ucas` (130 Mo). **Incertain** : changement de contenu (non mesuré) ; exe en
  Shipping (sûrement plus petit, mais tests et journaux à vérifier) ; un delta binaire de l'exe (msdelta, xdelta)
  serait bien plus petit, non mesuré ; serveur non comparé (exe 313 Mo, `.ucas` 102 Mo, même schéma attendu).
  Listes JSON et comparaisons dans le scratchpad de la session (`L0\`) ; archives `L0-*` supprimées d'Elrond.
- 2026-10-25 : Elrond libéré (`IN_USE.txt` retiré, `build-site-leftover` = `main` fb462ff, propre). Question posée à
  l'utilisateur : suivre la proposition de L0 (pas de chunks pour l'instant, compression au transfert, L3 = mesure
  d'un changement de contenu) ou garder les chunks de L-Q2.
- 2026-10-25 : **L-Q2 bis = C** (choix de l'utilisateur ; recommandation A) : pas de chunks pour l'instant,
  compression au transfert, **et correctifs binaires dès maintenant** (une version de code : quelques Mo au lieu de
  124 Mo compressés, à confirmer). Plan revu : L2 (routes des correctifs), L3 (mesures : changement de contenu,
  taille des correctifs de l'exe avec xdelta3 / msdelta / HDiffPatch, outil choisi), L4 (correctifs depuis les N
  versions gardées), L6 (appliquer, vérifier l'empreinte, fichier complet en secours). Prochaine étape : L1.
- 2026-10-25 : **L1 lancée** (« go »), dans Elrond, branche `launcher-l1` = `main` 520f6ad, `IN_USE.txt` posé, en
  sous-agent : `Manager/HOLUpdate` (bibliothèque .NET 8 : liste d'un dossier, différence, JSON) et
  `Manager/HOLUpdate.Tests` (xUnit, rouge d'abord), ajoutés à `HOLManager.sln` ; le gestionnaire n'y est pas encore
  branché (L2).
- 2026-10-25 : L1 : projets créés dans Elrond : `Manager/HOLUpdate` (bibliothèque net8.0) et `HOLUpdate.Tests` (xUnit,
  mêmes paquets que `HOLManager.Tests`), ajoutés à `HOLManager.sln`. Tests rouges en cours d'écriture.
- 2026-10-25 : L1 : **27 tests rouges** constatés (`dotnet test` sur `HOLUpdate.Tests`, types en `NotImplementedException`) :
  liste d'un vrai dossier temporaire, `.pdb` exclu, ordre ordinal, JSON aller-retour, différence (casse ignorée).
- 2026-10-25 : L1 : **vert**, 27/27 (`HOLUpdate.Tests`) et 173/173 (`HOLManager.Tests`), `dotnet test HOLManager.sln` 8 s.
  **L1 close** (non commitée, branche `launcher-l1` d'Elrond) : `FileEntry`, `VersionManifest`, `ManifestBuilder`, `ManifestDiff`.
- 2026-10-25 : L1 relue par la session principale : `dotnet test HOLManager.sln` repassé (27/27 + 173/173) ;
  `HOLManager.sln` ramené à l'ajout des deux projets (+12 lignes, en Any CPU seulement, au lieu des +59 / −3 de
  `dotnet sln add` qui ajoutait x64 / x86 : moins de conflits avec la session réseau), tests repassés. **L1 faite,
  non commitée** : attend l'accord de l'utilisateur (absent). Ensuite : L2, à coordonner avec la session réseau
  (`Manager/HOLManager`).
- 2026-10-25 : **L1 commitée** (accord de l'utilisateur) : `launcher-l1` rebasée sur `main`, `main` avancée à
  ad69a0c (avance rapide). L2 : accord demandé à la session réseau (elle travaille dans `Manager/HOLManager`).
- 2026-10-25 : **L2 lancée** (« go » de l'utilisateur). Session réseau fermée (pas d'accord possible) ; son travail non
  commité dans Glorfindel touche `ManagerSettings.cs`, `PROTOCOL.md`, `Lobbies/`, `Matches/MatchConfig.cs` : L2 se
  tient à l'écart (routes dans un fichier à part, une ligne dans `Program.cs`, section en fin de `PROTOCOL.md`,
  réglages à part si possible). Elrond, branche `launcher-l2` = `main` e247f12, `IN_USE.txt` posé, en sous-agent.
  Disposition du dossier des versions (fixée ici) : `<versions>/latest.txt` (nom de la dernière),
  `<versions>/<v>/manifest.json` (liste HOLUpdate), `<v>/notes.txt` (nouveautés), `<v>/files/<chemin>.gz` (fichier
  compressé à la publication, L4 : pas de compression à la volée, la reprise par `Range` porte sur le `.gz`),
  `<v>/patches/<depuis>/<chemin>.patch` (L3/L4), `<versions>/launcher/latest.txt` et `launcher/<lv>/HOLLauncher.exe`.
- 2026-10-25 : L2 : référence HOLUpdate ajoutée à HOLManager.csproj ; `VersionRoutesTests` écrit (25 cas, vrai dossier de
  versions, ManifestBuilder + GZipStream) : **rouge** 20/25 (les 5 « nom invalide » passent déjà : pas de route, 404).
- 2026-10-25 : L2 : **vert** : `Versions/VersionStore`, `VersionSettings` (VersionsDir), `Api/VersionRoutes` ; 3 lignes dans
  Program.cs. `dotnet test HOLManager.sln` : 27/27 + 199/199 (26 de L2). Reste : section en fin de PROTOCOL.md.
- 2026-10-25 : **L2 close** (non commitée, `launcher-l2` d'Elrond) : section « Versions du jeu (lanceur, L2) » en fin de
  PROTOCOL.md (protocolVersion inchangé, `format` 1 propre à /versions/latest) ; lanceur : en-têtes X-Launcher-Version/-Sha256.
- 2026-10-25 : L2 relue par la session principale : `dotnet test HOLManager.sln` repassé (27/27 + 199/199) ; garde
  des chemins vue (entrée exacte de la liste, déjà vérifiée par `FromJson`, puis fichier résolu sous le dossier de la
  version ; noms de version filtrés) ; existants touchés : `Program.cs` +5, `HOLManager.csproj` +4, `PROTOCOL.md` +69
  (fin). **L2 faite, non commitée** : attend l'accord de l'utilisateur. Fusion : conflits possibles avec la session
  réseau sur `Program.cs` / `PROTOCOL.md` (ajouts seulement, faciles). Suivant : L3 (mesures).
- 2026-10-25 : **L2 commitée** (accord de l'utilisateur), `main` avancée par avance rapide. **L3 lancée** (« go »),
  Elrond, branche `launcher-l3`, en sous-agent : (a) changement de contenu, (b) correctifs de l'exe (xdelta3, msdelta,
  HDiffPatch ; outils téléchargés dans un dossier temporaire à part).
- 2026-10-25 : L3 : outils prêts (xdelta3 3.2.2, HDiffPatch 5.1.3 des releases GitHub ; msdelta.dll par P/Invoke),
  banc d'essai vérifié sur petits fichiers (SHA-256 identique). `L3-a` (main tel quel) en cours d'empaquetage.
- 2026-10-25 : L3 : `L3-a` PASS en 4,4 min, liste faite (50 fichiers, 621,6 Mo). (a) : `SourceTypes.csv` Arbre 100 → 101
  bois, réimport par `Tools/Editor/make_source_types.py` (chemin officiel : CSV → DataTable `.uasset`) en cours.
- 2026-10-25 : L3 : `L3-b` PASS, liste faite. a/b : 5 fichiers changés : `.ucas` (130,4 Mo, **1 bloc de 64 Ko sur 2087**,
  1 octet), `.utoc`, `Version.txt`, 2 manifestes ; exe identique. CSV et `.uasset` remis (`git checkout`). `L3-c` lancé.
- 2026-10-25 : L3 : `L3-c` PASS (texte d'un `UE_LOG` de `HeroesOfLibertyGameModeBase.cpp`, remis ensuite), liste faite ;
  a/c : exe + 4 petits fichiers. Correctifs de l'exe a → c en cours (xdelta3, msdelta, HDiffPatch).
- 2026-10-25 : **L3 close.** Empaquetages : `L3-a` 4,4 min, `L3-b` 1,2 min, `L3-c` 4,6 min (tous PASS).
  **(a) Contenu** : `SourceTypes.csv` Arbre 100 → 101, réimport `make_source_types.py` (commandlet, 0,2 min) :

  | a / b (une valeur d'une table) | Résultat |
  |---|---|
  | Fichiers changés | `.ucas` 130,4 Mo, `.utoc` 0,14 Mo, `Version.txt`, 2 manifestes ; exe et `.pak` identiques |
  | `.ucas` | même taille, **1 octet** différent : 1 bloc de 64 Ko sur 2087 ; gzip inutile (130 → 129 Mo) |
  | Correctif du `.ucas` | xdelta3 709 octets, HDiffPatch 46 octets (SHA-256 identique) ; msdelta refuse (> 32 Mo) |

  **(b) Exe** a → c (332,3 Mo, un texte de `UE_LOG` ; plein : 332 Mo, gzip 124 Mo) :

  | Outil | Correctif | Création | Application | Mémoire créa. / appl. | SHA-256 |
  |---|---|---|---|---|---|
  | xdelta3 3.2.2 `-9 -B 512M` | 28,9 Mo | 39 s | 2,3 s | 1,3 Go / 0,5 Go | identique |
  | msdelta (Windows) | — : **refuse tout fichier > 32 Mo** (erreur 13, mesuré : 32 Mo OK, 33 Mo non) | | | | |
  | HDiffPatch 5.1.3 `-m-6 -SD -c-zstd-21-24` | **8,3 Mo** | 38 s | 0,3 s | 1,6 Go / ~30 Mo | identique |

  Sur les 32 premiers Mo de l'exe (seul cas possible pour msdelta) : HDiffPatch 0,26 Mo / 1 s, msdelta 0,89 Mo / 60 s,
  xdelta3 1,30 Mo / 4 s ; les trois identiques. **Outil recommandé pour L4/L6 : HDiffPatch** (le plus petit partout,
  application rapide et peu gourmande, vérifiée) : `hdiffz.exe` (1,5 Mo) dans le script de publication, `hpatchz.exe`
  (0,5 Mo) livré avec le lanceur et appelé en processus depuis C# (licence MIT) ; msdelta, bien qu'intégré à Windows,
  est exclu (32 Mo max : il faudrait découper l'exe, et il est 3 à 60 fois moins bon). **Chunks : non justifiés** :
  un changement de table ne touche que quelques octets du `.ucas`, que le correctif binaire réduit à presque rien.
  Le projet est remis (CSV, `.uasset`, `.cpp` par `git checkout`) ; archives `L3-*` supprimées ; listes, résultats et
  scripts dans le scratchpad de la session (`L3\lists`, `L3\results`, `L3\scripts`).
- 2026-10-25 : Elrond libéré (`IN_USE.txt` retiré ; `launcher-l3` = `main` 08cfa75, propre). Proposé à l'utilisateur :
  **HDiffPatch** comme outil des correctifs (L4 `hdiffz`, L6 `hpatchz` livré avec le lanceur), pas de chunks.
- 2026-10-25 : **HDiffPatch retenu** (accord de l'utilisateur, « fais les 3 ») ; pas de chunks (L3 : inutile). **L4
  lancée**, découpée : **L4a** = `Manager/HOLPublish` (outil C# : liste par HOLUpdate, `.gz`, correctifs `hdiffz`
  depuis les N versions gardées, dossier écrit à part puis renommé, `latest.txt` en dernier, élagage), tests sur de
  vrais dossiers ; **L4b** = `Tools/Package/publish.ps1` (empaquette ou prend une archive, appelle HOLPublish, dossier
  serveur du gestionnaire) et essai bout à bout sur deux vrais empaquetages.
- 2026-10-25 : L4a : HDiffPatch 5.1.3 (hdiffz, hpatchz, LICENSE MIT, README) dans Tools/ThirdParty/HDiffPatch d'Elrond, non exclus.
- 2026-10-25 : L4a : Manager/HOLPublish (console) + HOLPublish.Tests ajoutés à HOLManager.sln (Any CPU, à la main) ; 24 tests rouges constatés.
- 2026-10-25 : L4a : vert 24/24 (Publisher, VersionFolder, HDiffPatch, CommandLine) au premier essai ; reste dotnet test HOLManager.sln entier.
- 2026-10-25 : **L4a close** (non commitée, launcher-l4 d'Elrond) : dotnet test HOLManager.sln 27 + 24 + 199 verts ; essai à la main de l'exe : PASS / FAIL, élagage OK.
- 2026-10-25 : L4a : ajout à la disposition : <v>/published.txt (date UTC, ordre des versions, non servi) ; sortie en UTF-8 (publish.ps1 : [Console]::OutputEncoding UTF-8).
- 2026-10-25 : L4a relue par la session principale : `dotnet test` repassé (27 + 24 + 199) ; `hdiffz.exe` / `hpatchz.exe`
  de `Tools/ThirdParty/HDiffPatch/` identiques (SHA-256) à ceux de la release officielle. Non commitée : L4 sera commitée
  entière après L4b et l'accord de l'utilisateur. **L4b lancée** (même branche `launcher-l4`, en sous-agent).
- 2026-10-25 : L4b : Tools/Package/publish.ps1 écrit (Elrond) ; essai sur fausses archives : 2 PASS, correctif, refus -Version / doublon OK.
- 2026-10-25 : L4b : publish L4-a PASS (empaquetage 3,6 min, HOLPublish 16 s : 50 fichiers, 340,9 Mo gz). Puis L4-c (texte d'UE_LOG).
- 2026-10-25 : L4b : publish L4-c PASS (2,3 min + HOLPublish 72 s), correctif exe 8,3 Mo ; HOLManager le sert, hpatchz : SHA-256 identique.
- 2026-10-25 : **L4b close** (non commitée, launcher-l4) : archives L4-* et dossier d'essai supprimés ; serveur de partie : question posée.
- 2026-10-25 : L4b reçue par la session principale : **L4 faite** (a + b, non commitée, branche `launcher-l4`).
  Question posée à l'utilisateur (L-Q7) : mise à jour du **serveur de partie** à la publication (le gestionnaire lit
  la version du jeu une fois, au démarrage) : 1. `publish.ps1 -Server` arrête le gestionnaire, remplace le dossier du
  serveur, le relance, refuse si des parties tournent ; 2. un dossier serveur par version, les parties en cours
  finissent sur la leur (code du gestionnaire : session réseau) ; 3. à la main.
- 2026-10-25 : **L-Q7 = B, confié à la session réseau** (choix de l'utilisateur ; reco devenue B : le serveur d'une
  ancienne version se nettoie quand sa dernière partie finit, un seul serveur de 3 Go en permanence). Demande écrite
  dans `NetPlaytestInProgress.md` (fin de « Suivi »). En attendant : serveur mis à jour à la main.
- 2026-10-25 : **L4 commitée** (accord de l'utilisateur) : rebasée sur `main` 344f08a (la note L-Q7 y est déjà, commitée
  par la session réseau avec son journal), `dotnet test` 27 + 24 + 206 PASS, `package.ps1` (`-Playtest` ajouté) garde sa
  ligne PASS ; publication à blanc sur deux archives factices : deux PASS, correctif de l'exe 46 octets, `latest.txt`
  juste ; `main` avancée (ff). Elrond libéré. Suivant : **L5** (squelette du lanceur, `Manager/HOLLauncher`).
- 2026-10-25 : **L5 lancée** (« continuons le lanceur »), Elrond, branche `launcher-l5` = `main` d285ba3, `IN_USE.txt`
  posé, en sous-agent. Découpée : **L5a** = `Manager/HOLLauncher` (WinForms, net8.0-windows) et
  `HOLLauncher.Tests` : lecture du `servers.json` du jeu, client de `/versions/latest`, version installée lue dans
  le dossier du jeu, état affiché (à jour / en retard / serveur injoignable), tests rouges d'abord ; **L5b** = la
  fenêtre (versions, nouveautés, « Jouer », deux boutons inactifs) et la publication en un seul exe autonome.
- 2026-10-25 : L5a : commencée (sous-agent, Elrond) : lecture du plan, de PROTOCOL.md (L2), de ParseServers et de l'emplacement de servers.json en jeu empaqueté (FPaths::RootDir = dossier du jeu, à côté de l'exe d'amorce).
- 2026-10-25 : L5a : projets HOLLauncher (WinExe net8.0-windows) et HOLLauncher.Tests ajoutés à HOLManager.sln à la main (Any CPU seulement) ; 39 tests écrits, rouges : 38 FAIL (NotImplementedException), 1 PASS (délai court du client, déjà vrai).
- 2026-10-25 : L5a : verte : `dotnet test Manager/HOLManager.sln` 27 + 39 + 24 + 206 PASS, compilation 0 avertissement ; l'exe démarre (fenêtre vide « Heroes of Liberty - launcher 1 »). Décisions : version du lanceur = constante `LauncherInfo.Version = "1"` (montée à la main à chaque lanceur publié, même nom que `launcher/<lv>/`) ; servers.json illisible = erreur (InvalidDataException), absent ou sans ligne valide = gestionnaire local comme le jeu ; réponse refusée (erreur HTTP, corps refusé) affichée comme « serveur injoignable » avec son message ; Version.txt vide = pas installé ; délai du client 5 s. Rien commité. Suivant : L5b (la fenêtre).
- 2026-10-25 : L5a relue par la session principale : `dotnet test HOLManager.sln` repassé (27 + 39 + 24 + 206) ;
  `HOLManager.sln` +12 seul fichier existant touché. Décisions du sous-agent gardées (version du lanceur = constante
  `LauncherInfo.Version` « 1 », nom du dossier `launcher/<lv>/` ; « en retard » = installée ≠ dernière ; `servers.json`
  illisible = erreur affichée). **L5b lancée** (même branche, en sous-agent).
- 2026-10-25 : L5b : commencée (sous-agent, Elrond) : lecture du plan, de LauncherStatus, de la fenêtre vide de L5a.
- 2026-10-25 : L5b : 12 tests écrits (StatusView 9, StatusLoader 3), rouges : 12 FAIL, 39 PASS.
- 2026-10-25 : L5b : verte et essayée : StatusView (textes, boutons) + StatusLoader (servers.json → /versions/latest → Version.txt, InvalidDataException → vue « Erreur : … ») ; MainForm mince (TableLayoutPanel à la main, lecture en tâche de fond, boutons de mise à jour → « à venir (L6/L7) », « Jouer » inactif) ; csproj : publication un seul exe autonome compressé, sans trimming, si -r donné. `dotnet test HOLManager.sln` 27 + 51 + 24 + 206 PASS, 0 avertissement. Exe win-x64 71,7 Mo ; démarre : sans servers.json ni gestionnaire « Serveur injoignable : … refused … (127.0.0.1:7700) » ; contre un vrai HOLManager (port 7711, dossier de versions factice v2 + notes + lanceur 2) : installée v1, dernière v2, nouveautés montrées, les deux boutons de mise à jour actifs. Captures dans le scratchpad L5b. Rien commité.
- 2026-10-25 : L5b relue par la session principale : `dotnet test` repassé (27 + 51 + 24 + 206), capture vue (v1 / v2,
  nouveautés, lanceur 1 → 2 disponible, deux boutons actifs, « Jouer » grisé). **L5 faite, non commitée** (attend
  l'accord de l'utilisateur, avec L6 si elle avance d'ici là). Exe autonome 71,7 Mo (compressé, sans trimming).
  **L6 lancée** sur la même branche, découpée : **L6a** = plan et téléchargement (pour chaque fichier : correctif si
  `patchesFrom` contient la version installée, sinon `.gz` ; dossier temporaire ; reprise par `Range` ; gunzip ;
  SHA-256), **L6b** = correctif `hpatchz` appliqué et vérifié (échec : fichier complet), remplacement, fichiers retirés
  supprimés, `Version.txt` en dernier, **L6c** = fenêtre (bouton, barre de progression) et essai contre le gestionnaire.
- 2026-10-25 : L6a : commencée (sous-agent, Elrond) : lecture du plan, de PROTOCOL.md (routes des versions), de HOLUpdate et de VersionsClient. ManifestBuilder compte tout sous le dossier du jeu (seul *.pdb exclu) : le dossier temporaire `.holupdate/` y entrerait, le lanceur passera donc ses propres exclusions.
- 2026-10-25 : L6a : 22 tests écrits (UpdatePlan 9, FileDownloader 13 : complet gunzip + SHA-256/taille, coupure puis reprise par Range, serveur qui ignore Range, correctif gardé tel quel, 404 patch_not_found → complet, progression, annulation), rouges sur bouchons : 22 FAIL, 51 PASS.
- 2026-10-25 : L6a : **verte** : `UpdatePlan` (ManifestDiff réutilisée ; correctif si `patchesFrom` contient l'installée et fichier changé, sinon complet ; ajouté toujours complet ; FullBytes = tailles des complets seuls ; segments d'URL échappés ; ScanGame = ManifestBuilder avec `*.pdb` + `.holupdate/*` exclus, Version.txt absent → Empty) et `FileDownloader` (dossier `<jeu>/.holupdate/<v>/` : `files/<chemin>` vérifié, `files/<chemin>.gz.part`, `patches/<chemin>.patch(.part)` ; Range 206 → ajout, 200 → zéro, 416 → zéro ; gunzip puis taille + SHA-256, échec → tout supprimé ; complet déjà vérifié non retéléchargé ; 404 patch_not_found → complet ; coupure → UpdateDownloadException, .part gardé ; client sans délai global, ResponseHeadersRead). `dotnet test HOLManager.sln` 27 + 73 + 24 + 206 PASS, 0 avertissement. Aucun fichier existant touché. Rien commité. Suivant : L6b (hpatchz, remplacement, suppressions, Version.txt en dernier).
- 2026-10-25 : L6a relue par la session principale : `dotnet test` repassé (27 + 73 + 24 + 206). **Risque vu** : le
  lanceur est posé dans le dossier du jeu, et le plan supprime tout fichier local absent de la version publiée : il
  effacerait `HOLLauncher.exe`, `hpatchz.exe`, et tout fichier ajouté par le joueur. De plus le paquet publié porte un
  `servers.json` vers `127.0.0.1` (`add_playtest.ps1`) qui écraserait celui de l'ami. **Corrigé en L6b (par défaut,
  question posée à l'utilisateur)** : le lanceur garde la liste qu'il a installée (`.holupdate/installed.json`) et ne
  supprime que les fichiers de cette liste ; `HOLLauncher*.exe`, `hpatchz.exe` et `servers.json` sont hors liste
  (un `servers.json` local n'est jamais écrasé). Secours d'un correctif : URL du complet portée par le plan (au lieu
  de la relire dans l'URL du correctif). **L6b lancée** (en sous-agent).
- 2026-10-25 : **Accord de l'utilisateur** (« tout est bon et tu peux commit ») : règles de L6b gardées (suppression
  limitée à `installed.json` ; lanceur, `hpatchz.exe`, `servers.json` local jamais touchés) ; commit permis. L6b tourne
  dans la même branche : commit fait à son retour, après relecture et `dotnet test` (L5 + L6 jusque-là).
- 2026-10-25 : L6b : plan corrigé (UpdatePlan : installed.json précédent, seuls ses fichiers absents de la nouvelle version supprimés, sans lui rien ; constantes LauncherPattern `HOLLauncher*.exe` / HpatchzName / ServersName dans LauncherOwned, exclus du scan et de la cible ; servers.json téléchargé seulement s'il manque ; PlannedDownload.FullUrl, VersionOf supprimée ; RemovedFiles… passe désormais la liste) ; 15 tests nouveaux (UpdatePlan 4, GameUpdater 11 avec vrais correctifs hdiffz) : 11 FAIL sur bouchon GameUpdater, 77 PASS (les 4 du plan écrits avec le code, verts d'emblée).
- 2026-10-25 : L6b : **verte** (boucle 1 : points 1 à 4) : PatchApplier (hpatchz -f, chemin à côté de l'exe par défaut, injectable ; taille + SHA-256 ; échec → raison, sortie supprimée), UpdateApplier (vérif d'écriture exclusive de tout fichier à remplacer/supprimer + Version.txt avant tout changement → GameLockedException « … est utilisé : fermez le jeu, puis relancez la mise à jour. » ; déplacements en ordre ordinal, suppressions, Version.txt en dernier, installed.json (écrit en .tmp puis renommé), puis .holupdate/<v> supprimé), GameUpdater.UpdateAsync (plan → téléchargements → correctifs, échec → complet par FullUrl → remplacement ; UpdateResult, annulation/réseau/disque en message, jamais d'exception). `dotnet test HOLManager.sln` 27 + 88 + 24 + 206 PASS, 0 avertissement. Rien commité. Reste : L6c (fenêtre, barre de progression, essai contre le gestionnaire).
- 2026-10-25 : L6b relue par la session principale : `dotnet test` repassé (27 + 88 + 24 + 206). **L5 + L6a/b
  commitées** (accord de l'utilisateur) : fba688f sur `launcher-l5`, `main` avancée (ff). (Le message du commit dit
  « 61 xUnit tests » : c'est 88 pour HOLLauncher.Tests.) **L6c lancée** (fenêtre : bouton « Mettre à jour le jeu »,
  barre de progression, progression du complet de secours ; essai contre le vrai gestionnaire).
- 2026-10-25 : L6c : 10 tests nouveaux (ProgressView 6, StatusView.Updating 1, GameUpdater 1 : progression du fichier complet après un correctif raté, LauncherOptions 2 : `--update`) : 8 FAIL sur bouchons, 90 PASS (les 2 de `--update` écrits avec le code).
- 2026-10-25 : L6c : **points 1 et 2 verts** : ProgressView (« Fichier 3/5 : HeroesOfLiberty.exe — 12,4 Mo », barre = fichiers finis, « Annulation… ») ; StatusView.Updating (tout grisé) ; GameUpdater : le fichier complet après un correctif raté remonte sa progression (n/n, octets ajoutés au total) ; MainForm : « Mettre à jour le jeu » relit servers.json + /versions/latest puis GameUpdater en tâche de fond (client HTTP sans délai global), barre + « Annuler » (CancellationToken, aussi à la fermeture), message de fin puis Actualiser ; `--update` lance la mise à jour après la première lecture. HOLLauncher.Tests 98 PASS.
- 2026-10-25 : L6c : **essai réel vert** (scratchpad L6c) : v1/v2 factices (exe 6 Mo aléatoire, 3 × 64 octets changés ; Data/Old.pak retiré, Data/New.pak ajouté, Data/Common.txt identique), publiés par HOLPublish (Release) : v2 = 4 fichiers, exe 6 293 394 o en gzip, **correctif v1→v2 251 o**. HOLManager port 7711 (--VersionsDir), lanceur publié (71,7 Mo) + hpatchz.exe + servers.json local. Clic « Mettre à jour le jeu » par UI Automation (gameA, gameC : copies de v1 sans installed.json) et `--update` (gameB : avec installed.json = manifest v1 ; gameD) : message « Le jeu est à jour (version v2). » en ~1 s, puis Actualiser → « Version installée : v2 / Le jeu est à jour. ». Téléchargé 406 o (correctif 251 + New.pak.gz 133 + Version.txt.gz 22). Empreintes = v2 partout (exe 5e7b20e8f5d1) ; lanceur, hpatchz.exe, servers.json inchangés ; Old.pak **gardé** sans installed.json (A, C), **supprimé** avec (B). Barre de progression non vue (trop rapide en local). Gestionnaire arrêté. `dotnet test HOLManager.sln` 27 + 98 + 24 + 206 PASS, 0 avertissement. Rien commité.
- 2026-10-25 : L6c relue par la session principale : `dotnet test` repassé (27 + 98 + 24 + 206). **L6 faite et
  commitée** (accord donné pour L6) : `main` avancée (ff). Essai réel : 406 octets téléchargés (correctif de l'exe
  251 octets), jeu = v2, lanceur / `hpatchz.exe` / `servers.json` intacts, fichier retiré supprimé seulement avec
  `installed.json`. Non vu : la barre bouger (mise à jour locale < 1 s) — à voir en L10. **L7 lancée** (mise à jour du
  lanceur, en sous-agent).
- 2026-10-25 : L7 : 18 tests nouveaux (LauncherUpdater 13, ProgressView.FromLauncher 2, LauncherOptions `--update-launcher` 1, UpdatePlan 1 : scan sans HOLLauncher.new/old.exe ni .new.exe.part, GameUpdater 1 : ces fichiers jamais supprimés même listés dans installed.json) : 18 FAIL sur bouchons (dont le .part : supprimé, pas exclu), 98 PASS. FileDownloader : FetchRouteAsync public (route quelconque, reprise Range, en-têtes vus avant le corps) pour ne pas dupliquer.
- 2026-10-25 : L7 : **points 1 à 3 verts** : LauncherUpdater (GET /launcher/latest → HOLLauncher.new.exe.part à côté de l'exe en cours par FileDownloader.FetchRouteAsync, reprise Range ; en-têtes X-Launcher-Version/-Sha256 exigés avant le corps ; SHA-256 faux → .part supprimé, message ; .new.exe puis bascule SwapIn : exe → HOLLauncher.old.exe (ancien .old supprimé d'abord), .new → nom de l'exe, échec → ancien remis ; DeleteOld silencieux si verrouillé, DeleteOldAsync 20 × 250 ms en tâche de fond au démarrage) ; ProgressView.FromLauncher (« Lanceur version 2 : 12,4 / 71,7 Mo », barre en octets) ; motif LauncherPartPattern `HOLLauncher*.exe.part` ajouté à LauncherOwned ; MainForm : bouton → téléchargement (barre, Annuler), bascule, relance de l'exe avec les mêmes arguments (sans `--update-launcher`) et fermeture, sinon message puis Actualiser ; option `--update-launcher`. `dotnet test HOLManager.sln` 27 + 116 + 24 + 206 PASS, 0 avertissement. Rien commité.
- 2026-10-25 : L7 : **essai réel vert** (scratchpad L7) : lanceurs 1 et 2 publiés (Release win-x64, 71,8 Mo ; LauncherInfo.Version remis à "1"), dossier de versions = celui de L6c (v2, nécessaire pour que /versions/latest réponde) + launcher/latest.txt = 2 + launcher/2/HOLLauncher.exe ; HOLManager port 7712 ; lanceur 1 dans un dossier de jeu factice (servers.json → 7712) : « Lanceur : version 1 (version 2 disponible) », clic « Mettre à jour le lanceur » par UI Automation → ancien processus fermé à 2,4 s, nouveau relancé avec les mêmes arguments (`--game-dir …`) : « Lanceur : version 2 », exe = SHA-256 du lanceur 2 (550dbb3e…), HOLLauncher.old.exe déjà supprimé à ce démarrage (réessai en tâche de fond), ni .new.exe ni .part restants. Gestionnaire et lanceurs arrêtés. Barre non vue (local trop rapide). Rien commité.
- 2026-10-25 : L7 relue par la session principale : `dotnet test` repassé (27 + 116 + 24 + 206), bascule et retour
  arrière vus. **L7 faite, non commitée** (`launcher-l5` d'Elrond ; l'accord de commit portait sur L5/L6 : nouvel
  accord demandé). **L8 lancée** sur la même branche (en sous-agent).
- 2026-10-25 : L8 : PlayRules (pur : installé et à jour → actif ; en retard → actif + confirmation « Le jeu n'est pas à jour : le serveur refusera cette version. Jouer quand même (en solo) ? » ; injoignable + installé → actif, « Vous pouvez jouer en solo. » ajouté à l'état ; pas installé ou mise à jour en cours → inactif), GameLauncher (<dossier du jeu>\HeroesOfLiberty.exe, nom du paquet Windows\ ; absent → « Le jeu est introuvable : … n'existe pas. » ; Process.Start, dossier de travail = dossier du jeu, sans argument), StatusView.PlayEnabled/PlayConfirmation, MainForm : « Jouer » → confirmation éventuelle → jeu lancé → lanceur fermé (décision par défaut). Rouge (7 FAIL sur bouchons) puis vert : `dotnet test HOLManager.sln` 27 + 126 + 24 + 206 PASS. servers.json : le jeu empaqueté lit FPaths::RootDir() = dossier de HeroesOfLiberty.exe = celui que lit GameServers. Rien commité.
- 2026-10-25 : L8 : **essai vert** (scratchpad L8, lanceur de bin\Debug, pas l'exe publié) : dossier de jeu factice (Version.txt v1, servers.json → port fermé 7799, HeroesOfLiberty.exe = copie de cmd.exe) : « Serveur injoignable : … Vous pouvez jouer en solo. », « Jouer » actif, clic par UI Automation → faux jeu démarré depuis le dossier du jeu, lanceur fermé. Non vu à l'écran : la confirmation « jeu en retard » (couverte par les tests). 0 avertissement. Nom du joueur : pas géré par le lanceur (le jeu le garde dans ses réglages ; -PlayerName= des .bat ne sert qu'à deux joueurs sur un PC). Rien commité.
- 2026-10-25 : L8 relue par la session principale : `dotnet test` repassé. Doute du sous-agent sur le pseudo levé :
  sans `-PlayerName=`, `UHOLLocalPlayer::GetNickname` rend vide et le `?Name=` de l'URL ouverte par l'écran
  Multijoueur (nom sauvegardé, `HOLMultiplayerScreen.cpp`) est gardé ; le lanceur lance donc le jeu sans argument.
  **L8 faite, non commitée** (avec L7, accord demandé). **L9 lancée**. Défaut pris (question posée à l'utilisateur) :
  les amis reçoivent un **kit** (`HOLLauncher-kit.zip` : `HOLLauncher.exe`, `hpatchz.exe`, `servers.json` vers
  l'adresse Tailscale de l'utilisateur), fait par `publish.ps1 -Kit` ; dézippé dans un dossier vide, « Mettre à jour
  le jeu » installe tout.
- 2026-10-25 : L9 : points 1 verts : premier téléchargement sur un dossier « kit » (HOLLauncher.exe, hpatchz.exe, servers.json, sans Version.txt) déjà couvert par le moteur (test ajouté, vert d'emblée : tout en complet, aucun patch, kit intact, installed.json écrit) ; espace disque vérifié avant tout téléchargement (somme des tailles finales + 200 Mo, freeSpace injecté ; insuffisant → « Espace disque insuffisant : il faut … Go libres … », rien demandé) ; StatusView.UpdateGameText « Installer le jeu » quand rien n'est installé, MainForm le reprend. Rouge (3 FAIL sur bouchons) puis vert : HOLLauncher.Tests 129 PASS. Rien commité.
- 2026-10-25 : L9 : kit par un script à part `Tools/Package/make_kit.ps1 -ServerUrl http://<adresse>:7700 [-ServerName] [-VersionsDir] [-Out]` (publish.ps1 exige une archive du jeu ou un empaquetage Unreal ; le kit n'en dépend pas et ne se refait qu'au changement de lanceur ou d'adresse) : dotnet publish Release win-x64 → Saved\HOLLauncher\, pose launcher\<LauncherInfo.Version>\HOLLauncher.exe (.tmp puis renommé, alerte si un exe différent remplace la même version) puis launcher\latest.txt dans le dossier de versions (Saved\Manager\Versions par défaut), zip `Saved\Kit\HOLLauncher-kit.zip` (exe, hpatchz.exe, servers.json au format du jeu, LISEZMOI.txt en 3 lignes + SmartScreen) ; ligne PASS/FAIL, URL mal formée → FAIL vu. Script en UTF-8 avec BOM (PowerShell 5.1 lit sinon les accents en ANSI).
- 2026-10-25 : L9 : **essai vert** (scratchpad L9) : v2 factice de L6c publiée par HOLPublish dans L9\versions, kit fait avec `-ServerUrl http://127.0.0.1:7713` (**63,3 Mo** ; launcher\1\HOLLauncher.exe 71,7 Mo + latest.txt = 1), HOLManager sur 7713, kit dézippé dans un dossier vide : « Version installée : pas installé / Le jeu n'est pas installé ; la version v2 est disponible. », clic « Installer le jeu » par UI Automation → « Le jeu est à jour (version v2). » en 1 s, puis « Version installée : v2 ». Empreintes = v2 (4 fichiers), kit (exe, hpatchz, servers.json, LISEZMOI.txt) inchangé, installed.json écrit. Gestionnaire et lanceur arrêtés. `dotnet test HOLManager.sln` 27 + 129 + 24 + 206 PASS, 0 avertissement. Rien commité.
- 2026-10-25 : L9 relue par la session principale : `dotnet test` repassé (27 + 129 + 24 + 206). **L7, L8, L9 faites,
  non commitées** (`launcher-l5` d'Elrond, Elrond gardé : `IN_USE.txt` en place). Kit : `Tools/Package/make_kit.ps1
  -ServerUrl` (script à part de `publish.ps1` : le kit ne dépend pas du jeu), 63,3 Mo. **En attente de
  l'utilisateur** : accord de commit L7-L9 ; avis sur le kit ; pour L10, l'adresse Tailscale de son PC et le feu vert
  pour publier deux vraies versions dans le dossier du gestionnaire. Petit reste : message « Le jeu est à jour » après
  une première installation (« installé » serait plus juste).
- 2026-10-25 : **L7-L9 commitées** (accord de l'utilisateur) : 9b0a990, fusionnée dans `main` (d2ae192), `dotnet test`
  repassé sur `main` (27 + 129 + 24 + 206). Elrond libéré (`IN_USE.txt` retiré, `launcher-l5` = `main`). Suivant :
  **L10** (essai de l'utilisateur), qui attend son adresse Tailscale et le feu vert pour publier deux vraies versions.
- 2026-10-25 : L10 : l'utilisateur demande d'être guidé pour Tailscale (absent de ce PC). C'est aussi **R8.1** du
  chantier réseau (`NetPlaytestInProgress.md`) : guidage pas à pas, adresse notée ici et là-bas une fois connue.
  Ports à ouvrir ensuite sur le PC : gestionnaire 7700 (TCP), serveurs de partie dès 7777 (UDP).
- 2026-10-25 : L10 / R8.1 : **Tailscale installé** par l'utilisateur ; ce PC = `pc-belongfr-03`, adresse
  **100.123.190.53** (compte Tailscale de l'utilisateur). Pare-feu vu : les règles existantes (`holmanager.exe`,
  `heroeslibertyserver.exe`, créées par les invites Windows) ne valent que pour le profil **Public** et des chemins
  d'exe précis ; l'interface Tailscale est en profil **Privé** : il faut deux règles par port, limitées à
  `100.64.0.0/10` (TCP 7700, UDP 7777-7799), à créer par l'utilisateur en administrateur.
- 2026-10-25 : L10 / R8.1 : règles créées par l'utilisateur et vérifiées : « HOL gestionnaire (Tailscale) » TCP 7700
  et « HOL serveurs de partie (Tailscale) » UDP 7777-7799, entrée, `100.64.0.0/10`, tous profils. Suivant : partage du
  PC à un ami (R8.2) quand un ami est disponible ; en attendant, moitié locale de L10 (feu vert demandé).
- 2026-10-25 : **L10, moitié locale lancée** (« fais tout ce qui est possible sans ami »). **Défaut trouvé** : dans un
  vrai paquet, la version est dans `HeroesOfLiberty\Version.txt` (`package.ps1`, lue par le jeu), pas à la racine du
  dossier joueur ; le lanceur lit `Version.txt` à la racine (`InstalledVersion`, `UpdatePlan.ScanGame`,
  `UpdateApplier` « Version.txt en dernier ») : un vrai jeu serait vu « pas installé » et retéléchargé en entier à
  chaque fois. Les essais L6-L9 avaient un `Version.txt` factice à la racine. En parallèle : **L10a** correction
  (Arwen, branche `launcher-l10` = `main`), **L10b** empaquetage + publication de `L10-v1` (Elrond,
  `Saved\Manager\Versions`). Ensuite : kit (`http://100.123.190.53:7700`), installation, `L10-v2`, mise à jour, Jouer.
- 2026-10-25 : L10b : publish.ps1 -Version L10-v1 lancé (Elrond) ; HOLManager construit (build_manager.ps1 PASS, 8 s, 91,9 Mo, Saved\Manager\) : sans Server\HeroesOfLibertyServer.exe à côté, il annoncera la version de jeu « dev » (ResolveGameVersion) - à régler par GameVersion ou ServerExe avant l'essai.
- 2026-10-25 : **L-Q8 = A** (choix de l'utilisateur, conforme à la recommandation) : « je veux juste lui donner le
  launcher, pas qu'il ait à se loguer sur Tailscale ». Les amis n'installent **que le kit** ; ils joignent le PC par
  **redirection de ports de la box** (TCP 7700, UDP 7777-7799 → ce PC), le kit porte l'adresse publique (nom DynDNS
  plus tard si elle change) ; **mot de passe partagé** dans le kit pour que le gestionnaire ne réponde qu'aux
  lanceurs / jeux de l'utilisateur (à concevoir : touche aussi le client du jeu, chantier réseau). Tailscale reste
  installé sur le PC (accès de l'utilisateur), plus demandé aux amis. Mesuré : IP publique **90.55.168.58**, atteinte
  en 1 saut (vraie IPv4 de la box, pas de CGNAT) ; box `192.168.1.1` (probablement une Livebox, Orange) ; ce PC
  `192.168.1.19` en DHCP (à fixer dans la box).
- 2026-10-25 : L10a : défaut montré par les tests à la forme réelle (`HeroesOfLiberty/Version.txt`, `HeroesOfLiberty.exe` à la racine) : 10 rouges (jeu vu pas installé, tout retéléchargé sans correctif, version posée avant l'échec) ; corrigé par `GameFiles.VersionFile = "HeroesOfLiberty/Version.txt"` (Arwen, `launcher-l10`, non commité) lu par `InstalledVersion` (donc `ScanGame`, `StatusLoader`) et posé en dernier par `UpdateApplier` ; pas de repli sur un `Version.txt` racine (test `VersionAtTheRootIsNotRead`) ; HOLUpdate/HOLPublish inchangés ; `dotnet test Manager/HOLManager.sln` 27 + 130 + 24 + 206 verts, 0 avertissement.
- 2026-10-25 : L10a relue par la session principale : `dotnet test` repassé dans Arwen (27 + 130 + 24 + 206). Faite,
  non commitée (`launcher-l10` d'Arwen) ; le kit de l'essai sera fait depuis Arwen (lanceur corrigé). Attente : L10b
  (v1 publiée) et la redirection des ports par l'utilisateur.
- 2026-10-25 : L10b : publish L10-v1 **PASS** (5,3 min : empaquetage 5 min, HOLPublish 15 s) ; latest.txt = L10-v1, manifeste 50 fichiers / 621,6 Mo bruts, files\ 340,9 Mo gz, notes.txt en français ; Version.txt en `HeroesOfLiberty/Version.txt` (pas à la racine), pas de servers.json, `HeroesOfLiberty.exe` à la racine (amorce) ; seul le .pdb (2,5 Go) est exclu. Archive `Saved\Packages\L10-v1` laissée (6,07 Go).
- 2026-10-25 : L10b relue : `L10-v1` publiée (50 fichiers, 340,9 Mo gz), gestionnaire construit
  (`Elrond\Saved\Manager\HOLManager.exe`), à lancer avec `--GameVersion=L10-v1`. Suite en parallèle : **L10c**
  (gestionnaire sur 7700, kit depuis Arwen vers `127.0.0.1` pour l'essai local, installation de v1 par le lanceur,
  « Jouer ») et **L10d** (empaquetage seul de `L10-v2` dans Elrond, un texte d'`UE_LOG` changé puis remis, publié
  seulement après l'installation de v1).
- 2026-10-25 : L10d : log changé dans Elrond (InitGame, `Match on %s (L10-v2): ...`, émis à chaque chargement de carte) ; package.ps1 -Version L10-v2 lancé.
- 2026-10-25 : L10d : package L10-v2 **PASS** (3,1 min ; Saved\Packages\L10-v2 : Windows 3191 Mo, WindowsServer 3026 Mo), non publiée ; .cpp remis (git status propre hors IN_USE.txt) ; v1/v2 Windows : 51 fichiers des deux côtés, diffèrent seulement HeroesOfLiberty.exe (même taille, SHA différent), HeroesOfLiberty/Version.txt, les 3 Manifest_*_Win64.txt et le .pdb (exclu de la publication) ; .pak identiques.
- 2026-10-25 : L10c : gestionnaire d'Elrond lancé sur 7700 (`--GameVersion=L10-v1`, PID 70828, laissé tourner) ; `/versions/latest` = L10-v1, 50 fichiers ; `make_kit.ps1` depuis Arwen vers `http://127.0.0.1:7700` **PASS** (16 s, kit 63,3 Mo, `launcher\1\` posé dans les versions d'Elrond).
- 2026-10-25 : L10c : installation par `HOLLauncher.exe --update` dans un dossier vide : **14 s** jusqu'au message « Le jeu est à jour (version L10-v1). », puis « Version installée : L10-v1 » (défaut de L10a corrigé, vu au vrai) ; 50/50 empreintes conformes au manifeste (651,8 Mo bruts), `.holupdate\installed.json` en L10-v1, `HeroesOfLiberty\Version.txt` = L10-v1, les 4 fichiers du kit intacts.
- 2026-10-25 : L10c : relancé sans option : « Le jeu est à jour. », « Mettre à jour le jeu » grisé ; 2e `--update` : rien fait (par conception, pas de message quand à jour), aucun fichier modifié. (Le gestionnaire ne journalise pas les téléchargements : vérifié par les dates des fichiers.)
- 2026-10-25 : L10c : « Jouer » : amorce `HeroesOfLiberty.exe` puis `HeroesOfLiberty\Binaries\Win64\HeroesOfLiberty.exe`, lanceur fermé ; menu principal affiché en ~25 s ; journal du jeu : `Version: L10-v1`, `Manager client: 1 server(s), first Heroes of Liberty`, 0 erreur ; jeu fermé. Reste : l'essai depuis l'extérieur (IP publique, ports) et v1 -> v2 (L10d).- 2026-10-25 : L10 : redirection des ports faite par l'utilisateur (Livebox « Nautilus » : bail DHCP statique
  192.168.1.19, NAT/PAT TCP 7700 et UDP 7777-7799) ; règles du pare-feu ouvertes à toute adresse et renommées
  « HOL gestionnaire » / « HOL serveurs de partie » (vérifiées). **Port 7700 joignable depuis Internet** (portchecker.io :
  7700 ouvert, 7701 fermé en témoin ; check-host.net ne rend plus de résultat par API) ; la box renvoie aussi
  `http://90.55.168.58:7700` depuis le réseau local (boucle locale OK : un kit vers l'adresse publique s'essaie sur ce
  PC). UDP non vérifiable de l'extérieur sans ami. L10c relue (installation réelle 14,1 s, 50 fichiers justes,
  « Jouer » → menu). **Attention** : le gestionnaire est exposé sans mot de passe tant que L11 (clé partagée) n'est pas
  faite : à n'allumer que pendant les essais. **L10e lancée** : publication de `L10-v2`, passage v1 → v2 par le
  lanceur, kit public (`http://90.55.168.58:7700`) et installation par l'adresse publique.
- 2026-10-25 : L10e : publish.ps1 L10-v2 (archive, notes « Deuxième version d'essai : passage par correctif. ») **PASS** en 70 s (HOLPublish 66 s ; 50 fichiers, 340,9 Mo compressés ; 4 correctifs depuis L10-v1, 6,3 Mo dont `HeroesOfLiberty.exe.patch` 6 583 085 o pour un exe de 348 Mo, manifestes 42/66/858 o) ; versions gardées L10-v1, L10-v2 (757 Mo).
- 2026-10-25 : L10e : gestionnaire arrêté proprement (taskkill sans /F : « HOLManager stopping. ») et relancé `--GameVersion=L10-v2` (PID 53652) ; `/versions/latest` = L10-v2, `patchesFrom` = [L10-v1], notes justes.
- 2026-10-25 : L10e : lanceur sans option sur le jeu v1 : « Version installée : L10-v1 », « Dernière version : L10-v2 », « Une mise à jour du jeu est disponible (L10-v2). », nouveautés affichées, « Mettre à jour le jeu » actif.
- 2026-10-25 : L10e : `--update` v1 → v2 : **2,3 s** jusqu'à « Le jeu est à jour (version L10-v2). » (progression « Fichier 5/5 … 6,3 Mo ») ; seuls les correctifs ont transité (relevé de `.holupdate` toutes les 100 ms : `L10-v2\patches\` = les 4 .patch, `files\` = les 5 fichiers reconstruits, aucun .gz) ; seuls 5 fichiers modifiés (exe, Version.txt, 3 manifestes) ; 50/50 empreintes = manifeste v2 ; `installed.json` et Version.txt en L10-v2 ; zone de travail vidée ; « Version installée : L10-v2 », « Le jeu est à jour. », bouton grisé.
- 2026-10-25 : L10e : « Jouer » : lanceur fermé, jeu lancé ; journal `LogHOLNet: Display: Version: L10-v2`, `Manager client: 1 server(s)`, 0 erreur ; jeu fermé.
- 2026-10-25 : L10e : kit public `make_kit.ps1 -ServerUrl http://90.55.168.58:7700` depuis Arwen **PASS** (8 s, 63,3 Mo) ; dézippé dans un dossier vide, `--update` par l'adresse publique (boucle de la box) : **9,8 s** jusqu'à « Le jeu est à jour (version L10-v2). » (« Fichier 50/50 … 340,9 Mo », que des .gz, aucun correctif) ; 50/50 empreintes = manifeste v2 ; `installed.json` et Version.txt en L10-v2 ; « Version installée : L10-v2 », « Le jeu est à jour. » ; servers.json intact.
- 2026-10-25 : L10e : gestionnaire arrêté proprement (taskkill sans /F, « HOLManager stopping. ») ; plus aucun HOLManager ni serveur de partie, rien n'écoute sur 7700. **L10e faite, rien n'a cloché.**
- 2026-10-25 : L10e relue : **moitié locale de L10 réussie**. v1 → v2 par le lanceur en 2,3 s, 6,3 Mo de correctifs
  (exe 348 Mo → correctif 6,58 Mo), 50/50 empreintes, « Jouer » → jeu en `L10-v2` ; **kit public**
  (`http://90.55.168.58:7700`, 63,3 Mo, `scratchpad\L10\HOLLauncher-kit.zip`) : installation complète par l'adresse
  publique en 9,8 s. Gestionnaire arrêté (pas de mot de passe). Reste : commit de L10a (accord demandé) ; **L11** clé
  partagée (gestionnaire + lanceur + client du jeu, à coordonner avec le chantier réseau) avant de laisser le
  gestionnaire allumé ; essai avec un ami (UDP des parties non vérifié). Archives `Elrond\Saved\Packages\L10-v1/v2`
  (6 Go chacune) gardées pour l'instant.
- 2026-10-25 : **L10a commitée** (accord de l'utilisateur), branche `launcher-l10` d'Arwen fusionnée dans `main`,
  `dotnet test` repassé sur `main`. Suivant : L11 (clé partagée), en attente de la réponse de l'utilisateur.
- 2026-10-25 : **L11 lancée** (« fait le 11 après, oui ») : **clé partagée**, pour que le gestionnaire et les serveurs
  de partie exposés à Internet ne servent que les lanceurs / jeux du kit. Contrat (fixé ici) :
  - Clé : `access_key.txt` à côté de `HOLManager.exe` (réglage `AccessKeyFile`, défaut ce nom) ; fichier absent ou
    vide → pas de clé, tout ouvert comme aujourd'hui (développement). `make_kit.ps1` la crée si absente (32 octets
    aléatoires en base64url) et l'écrit dans le `servers.json` du kit.
  - `servers.json` : champ facultatif `"key"` par serveur, `{ "servers": [ { "name", "url", "key" } ] }`.
  - HTTP : en-tête **`X-HOL-Key`** sur toute requête ; clé fausse ou absente → **401** `{ "error": "bad_key" }`
    (comparaison à temps constant). **Exemptées** : les requêtes venant du bouclage (127.0.0.1 / ::1), c.-à-d. les
    serveurs de partie lancés par le gestionnaire et les essais locaux.
  - Serveur de partie : le gestionnaire lui passe **`-HOLAccessKey=<clé>`** ; le client ajoute l'option d'URL
    **`HOLKey=<clé>`** (encodée) à sa connexion (`UHOLLocalPlayer::GetGameLoginOptions`) ; `PreLogin` refuse sans
    elle (« Access key mismatch ») si le serveur en a une.
  Arwen, branche `launcher-l11` = `main` ; en parallèle **L11a** (C# : gestionnaire, lanceur, kit, `PROTOCOL.md`) et
  **L11b** (C++ : `ParseServers`, `HOLManagerClient`, option de connexion, `PreLogin`, refus affiché). Le chantier
  réseau a R7.1-R7.3 commité sur `net-playtest` (4a88040) non fusionné : conflits possibles, petits.
- 2026-10-25 : L11a : gestionnaire : réglage `AccessKeyFile` (défaut `access_key.txt`, relatif au dossier de l'exe), clé lue au démarrage (rognée, BOM ôté ; absente / vide → pas de clé) ; middleware `UseAccessKey` juste après `UseLobbyErrors`, avant toutes les routes (même inconnues) : hors bouclage (`AccessKey.IsExempt` : 127.0.0.0/8, ::1, ::ffff:127.x), `X-HOL-Key` comparé par `FixedTimeEquals`, sinon 401 `bad_key` (nouveau code de `LobbyErrors`, corps d'erreur habituel) ; `-HOLAccessKey=<clé>` en fin de ligne de commande du serveur ; journal « access key: on/off ». 22 tests xUnit (rouges d'abord), HOLManager.Tests 228.
- 2026-10-25 : L11a : lanceur : `ManagerServer(Name, Url, Key)` (`key` facultatif, vide = aucune) ; un seul `AccessKeyHandler` (DelegatingHandler) dans les deux `HttpClient` de la fenêtre, qui relit servers.json à chaque requête et n'envoie la clé qu'à l'adresse du serveur (schéma, hôte, port) ; 401 `bad_key` → « Clé d'accès refusée : le kit n'est pas le bon. Redemandez-le. » dans l'état (au lieu de « Serveur injoignable ») et dans les téléchargements (jeu, lanceur). 11 tests, HOLLauncher.Tests 141.
- 2026-10-25 : L11a : `make_kit.ps1 -ManagerDir` (défaut : parent de `-VersionsDir`) : `access_key.txt` créé s'il manque ou est vide (32 octets `RandomNumberGenerator`, base64url sans `=`, 43 caractères), repris sinon, écrit dans le `servers.json` du kit ; PASS « clé : créée / reprise » ; essai à blanc deux fois dans `scratchpad\L11` : créée puis reprise, même clé, servers.json du zip = la clé, clé absente du journal. `PROTOCOL.md` : section « Clé d'accès (L11) », ligne `bad_key` 401, `[-HOLAccessKey=<clé>]` ; `protocolVersion` inchangé.
- 2026-10-25 : L11a : `dotnet test` 420/420 (24+27+141+228), 0 avertissement. Essai réel (gestionnaire Arwen port 7714, versions d'Elrond, clé du scratchpad) : « access key: on », clé absente des journaux ; 127.0.0.1 → 200 ; `http://192.168.1.19:7714` sans en-tête → 401 bad_key, mauvaise → 401, bonne → 200 (`/status` et `/versions/latest`) ; lanceur, servers.json vers 192.168.1.19:7714 + bonne clé → « Le jeu n'est pas installé ; la version L10-v2 est disponible. », mauvaise clé → « Clé d'accès refusée : le kit n'est pas le bon. Redemandez-le. ». Tout arrêté, rien n'écoute sur 7714. Rien de commité.
- 2026-10-25 : L11b : côté jeu fait dans Arwen (`launcher-l11`, rien de commité) : `Net/HOLAccessKey.h/.cpp` (option `HOLKey`, en-tête `X-HOL-Key`, `-HOLAccessKey=`, `IsAccepted` : pas de clé attendue → tout passe, sinon égalité stricte, refus « Access key mismatch ») ; `ParseServers` lit `key` ; `UHOLManagerClient` met `X-HOL-Key` sur toute requête et pose la clé du 1er serveur par `HOLAccessKey::SetClientKey` à la lecture de servers.json ; `UHOLLocalPlayer::GetGameLoginOptions` ajoute `?HOLKey=<clé encodée>` ; `PreLogin` refuse après la vérification de version. Textes : `bad_key` (401) et `match_key` (refus du serveur) → « Clé d'accès refusée : réinstallez le jeu avec le kit reçu. » (+ en/ja dans `fill_po.py`, collecte des .po non relancée).
- 2026-10-25 : L11b : éditeur compilé (PASS) ; tests verts : `HeroesOfLiberty.AccessKey` (nouveau), `Version`, `VersionCheck`, `Net.ManagerClient.*` (Parse : clé lue), `UI.MatchLossText` (Access key mismatch → match_key ; 401 bad_key). Trouvé : `HeroesOfLiberty.UI.ManagerErrorText` n'est jamais lancé (chemin parent de `…ManagerErrorText.ReturnToLobby`, RunTests ne trouve que l'enfant) : à renommer. Cible serveur non compilée.
- 2026-10-25 : L11a relue par la session principale (rapport : 420/420 tests, essai réel 401 sans clé / 200 avec,
  bouclage exempté, lanceur « Clé d'accès refusée » sur mauvaise clé). `LISEZMOI.txt` du kit corrigé à la main (plus
  de Tailscale depuis L-Q8 ; « environ 350 Mo »). Attente de L11b (C++).
- 2026-10-25 : L11b relue (rapport : éditeur compilé, tests `AccessKey`, `Version*`, `Net.ManagerClient.*`,
  `UI.MatchLossText` verts ; cible serveur non compilée ; `.po` non regénérés). Notes : test parent
  `HeroesOfLiberty.UI.ManagerErrorText` jamais lancé (son chemin est le parent de `…ReturnToLobby` : RunTests ne
  trouve que l'enfant ; touche aussi `quick_check`) — pour le chantier réseau ; la clé apparaît dans le journal du
  serveur de partie (ligne moteur `LogNet: Login request`), sur le PC hôte seulement ; textes de refus différents
  lanceur / jeu, gardés (le lanceur dit de redemander le kit). **L11c lancée** : bout en bout dans Arwen (empaquetage
  `L11-v1` avec serveur, publication, gestionnaire reconstruit, kit avec clé vers l'adresse publique, installation).
- 2026-10-25 : L11c : `publish.ps1 -Root Arwen -Version L11-v1` **PASS** (package 5 min, jeu 3192 Mo + serveur 3026 Mo ; HOLPublish 15 s, 50 fichiers, 340,9 Mo) ; `build_manager.ps1` PASS (9 s) ; `make_kit.ps1 -ServerUrl http://90.55.168.58:7700` PASS (63,4 Mo, « clé : créée », `Saved\Manager\access_key.txt`, 43 caractères ; copie `scratchpad\L11\e2e\HOLLauncher-kit.zip`).
- 2026-10-25 : L11c : gestionnaire d'Arwen sur `0.0.0.0:7700` (`--GameVersion=L11-v1 --ServerExe=Saved\Packages\L11-v1\WindowsServer\HeroesOfLibertyServer.exe`) : « access key: on » ; `/status` par l'adresse publique sans en-tête → **401** `bad_key`, mauvaise clé → 401, bonne clé → 200 ; `127.0.0.1` sans en-tête → 200.
- 2026-10-25 : L11c : kit dézippé dans `scratchpad\L11\e2e\friend\` (vide), `HOLLauncher.exe --update` : installation complète par l'adresse publique avec la clé en **~8 s** (Version.txt écrit), « Le jeu est à jour (version L11-v1). » puis « Version installée : L11-v1 », « Le jeu est à jour. » (capture `e2e\launcher_installed.png`).
- 2026-10-25 : L11c : jeu du dossier `friend\` (paquet L11-v1, deux fenêtres Alice / Bob par `-saveddirsuffix`, conduites par `hol.Test.Run`, qui marche dans le paquet Development) : « Manager client: 1 server(s), first Heroes of Liberty (access key set) », liste lue (« 0 salon », aucun `bad_key`), Alice crée le salon, Bob le rejoint, prêts, lancement : le gestionnaire démarre le serveur de L11-v1 avec `-HOLAccessKey=<clé>` (= access_key.txt ; vu sur la ligne de commande du processus, le journal du gestionnaire ne l'écrit pas), les deux clients rejoignent **par 90.55.168.58:7777 (UDP par la box, en boucle)**, le serveur : « Login request … HOLKey=… », « Alice holds slot 0 », « Bob holds slot 1 », « Join succeeded » ×2 ; captures `e2e\l11c_list/lobby/alice/bob.png`.
- 2026-10-25 : L11c : contre-essai, copie `friend_nokey\` avec un servers.json sans `key` : lanceur « Dernière version : inconnue », « Clé d'accès refusée : le kit n'est pas le bon. Redemandez-le. Vous pouvez jouer en solo. » ; jeu « (no access key) », menu Multijoueur : « Clé d'accès refusée : réinstallez le jeu avec le kit reçu. » (HTTP 401 bad_key ; capture `e2e\l11c_refused.png` ; « Créer un salon » reste actif). **L11c PASS.**
- 2026-10-25 : L11c : tout arrêté (gestionnaire, 2 processus serveur, jeux, lanceurs) ; rien n'écoute sur 7700 / 7777. Vu dans netstat : 3 adresses inconnues d'Internet (robots de balayage) ont ouvert 7700 pendant les ~4 min d'allumage ; le gestionnaire ne journalise ni les 401 ni les requêtes, donc rien pour dire ce qu'elles ont demandé (401 attendu). Rien de commité.
- 2026-10-25 : L11c relue : **bout en bout PASS** avec la clé : `L11-v1` publiée depuis Arwen, kit avec clé vers
  `http://90.55.168.58:7700` (`Arwen\Saved\Kit\HOLLauncher-kit.zip`, clé dans `Arwen\Saved\Manager\access_key.txt`),
  401 sans clé par l'adresse publique, installation par le kit ~8 s, **partie à deux clients par 90.55.168.58:7777
  (UDP, via la box)** acceptée par le serveur de partie avec `HOLKey` ; contre-essai sans clé refusé par le lanceur et
  par le jeu. Vu : 3 robots d'Internet connectés sur 7700 en 4 min (la clé était nécessaire). `dotnet test` repassé
  dans Arwen. **L11 faite, non commitée** (accord demandé). Petits restes : le gestionnaire ne journalise ni requêtes
  ni 401 ; « Créer un salon » reste actif sur l'écran de refus ; `.po` à regénérer ; test parent
  `UI.ManagerErrorText` jamais lancé. Reste de L10 : l'essai avec un vrai ami (le nouveau kit, avec clé).
- 2026-10-25 : raccourci posé à la demande de l'utilisateur : `C:\Users\baudo\Desktop\Gestionnaire HOL.bat` (lance
  `Arwen\Saved\Manager\HOLManager.exe` sur 7700 avec la clé, `--GameVersion=L11-v1`, serveur de partie de l'archive
  `Arwen\Saved\Packages\L11-v1` ; fenêtre fermée = gestionnaire arrêté). Provisoire : chemins d'Arwen, à refaire quand
  le chantier réseau aura fixé l'emplacement durable du gestionnaire et du serveur par version (L-Q7).
- 2026-10-25 : **L11 commitée** (accord de l'utilisateur), `launcher-l11` d'Arwen fusionnée dans `main`, `dotnet
  test` repassé sur `main`. `IN_USE.txt` d'Arwen : `Saved\Manager`, `Saved\Packages\L11-v1`, `Saved\Kit` servent au
  raccourci du bureau et au kit des amis (ne pas supprimer) ; le code d'Arwen est libre. Reste : essai avec un ami ;
  petits restes de L11c.
- 2026-10-25 : **Essai avec un ami reporté** par l'utilisateur (« un autre jour »). À la reprise : il lance
  `Desktop\Gestionnaire HOL.bat`, envoie `Arwen\Saved\Kit\HOLLauncher-kit.zip` ; l'ami dézippe, « Installer le jeu »,
  « Jouer » ; vérifier la partie par UDP 7777 depuis l'extérieur (seul point non vérifié), puis fermer la fenêtre du
  gestionnaire. Chantier en pause jusque-là ; petits restes de L11c sans urgence.
- 2026-10-25 : Elrond libéré (`IN_USE.txt` retiré, propre, `launcher-l5` = d2ae192) ; ses `Saved\Packages\L10-v1/v2`
  (6 Go chacune) et `Saved\Manager\Versions` (L10) restent, inutiles, à supprimer sur accord. Arwen reste marqué pour
  ses seuls dossiers `Saved` (raccourci du bureau, kit) ; son code est libre.
- 2026-10-25 : Rangement (demande de l'utilisateur) : archives `L10-v1/v2` et `Saved\Manager\Versions` d'Elrond
  supprimées (~13 Go) ; le gestionnaire (avec `access_key.txt` et `Versions\L11-v1`), l'archive `L11-v1` et le kit
  **déplacés d'Arwen dans le `Saved` de `main`** : `MyCrusader\Saved\Manager`, `Saved\Packages\L11-v1`, `Saved\Kit`
  (dossiers par défaut de `publish.ps1` / `make_kit.ps1` lancés depuis `main`). `Desktop\Gestionnaire HOL.bat` pointe
  là ; vérifié en local (port 7798, bouclage : « access key: on », L11-v1, 50 fichiers, puis arrêté). **Arwen et
  Elrond libres** (sans `IN_USE.txt`).
- 2026-10-25 : Restes listés à l'utilisateur ; « fais-le si c'est rapide » : **L12** = journal du gestionnaire
  (requêtes, refus de clé), lancée en sous-agent (Arwen, branche `launcher-l12` = `main`) ; **DynDNS** (l'adresse
  publique d'une Livebox peut changer) guidé avec l'utilisateur : DuckDNS (gratuit, sans confirmation mensuelle, la
  Livebox ne le gère pas) tenu à jour par une tâche planifiée du PC (toujours allumé), nom mis dans le kit.
- 2026-10-25 : **DynDNS fait** : `heroesofliberty.duckdns.org` (compte DuckDNS de l'utilisateur) → 90.55.168.58 ;
  mise à jour par `MyCrusader\Saved\DuckDNS\update.ps1` (jeton dans `token.txt` à côté, `Saved\` ignoré par git,
  journal `update.log` 200 lignes) lancée par la tâche planifiée **« HOL DuckDNS »** (toutes les 5 min + à
  l'ouverture de session ; premier passage « OK »). **Kit refait** vers `http://heroesofliberty.duckdns.org:7700`
  (`MyCrusader\Saved\Kit\HOLLauncher-kit.zip`, 63,4 Mo, clé reprise). Vérifié, gestionnaire allumé ~1 min : par le nom,
  401 sans clé, L11-v1 avec ; portchecker.io sur le nom : 7700 ouvert (7701 fermé). Gestionnaire arrêté. Note :
  `make_kit` a prévenu que `launcher\1\HOLLauncher.exe` a été remplacé par un exe différent (même code, reconstruit) :
  sans effet tant qu'aucun ami n'a de lanceur installé.
- 2026-10-25 : L12 : journal du gestionnaire fait dans Arwen (`launcher-l12`, non commité) : nouvel `Api/RequestLog.cs` (refus `bad_key` en Warning sans clé ni en-tête, une ligne par adresse et par minute puis « N more refused from … in the last minute », accepté de l'extérieur en Information, sondages `/status`, `/lobbies`, `/lobbies/{id}`, `/watch` et bouclage en Debug, fichiers servis avec octets et 200/206), ligne de commande du serveur de partie journalisée avec `-HOLAccessKey=***` ; le fichier `Logs\manager-<date>.log` existait déjà (R2.12) ; PROTOCOL.md complété ; 444 tests verts, 0 avertissement ; essai sur 7715 depuis 192.168.1.19 : 1 ligne pour 5 refus, puis « 4 more refused » une minute après, clé absente du journal ; tout arrêté.
- 2026-10-25 : L12 relue par la session principale : `dotnet test` repassé dans Arwen (27 + 141 + 24 + 252). Refus
  journalisés (Warning, une ligne par adresse et par minute, puis un compte), requêtes extérieures acceptées en
  Information, sondages du jeu et bouclage en Debug, fichiers servis, ligne de commande du serveur avec
  `-HOLAccessKey=***` ; fichier `Logs\manager-AAAA-MM-JJ.log` déjà existant (R2.12). **L12 faite, non commitée**
  (accord demandé, puis `build_manager.ps1` depuis `main` pour le gestionnaire du raccourci).
- 2026-10-25 : **L12 commitée** (accord de l'utilisateur, « go ») : 0b9b14a, fusionnée dans `main`, `dotnet test`
  repassé (27 + 141 + 24 + 252). Gestionnaire du raccourci reconstruit depuis `main` (`build_manager.ps1` PASS ;
  `access_key.txt` et `Versions\` intacts ; démarrage vérifié sur le bouclage, journal dans `Saved\Manager\Logs\`).
  Arwen libre (`IN_USE.txt` retiré). Reste : essai avec un ami (reporté) ; petits restes sans urgence (traductions,
  « installé » après une première installation, « Créer un salon » sur l'écran de refus, version du lanceur à monter à
  la main, clôture au GDD).
