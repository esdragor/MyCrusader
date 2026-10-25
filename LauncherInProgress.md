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
| L-Q2 | **Liste des fichiers avec empreinte** (SHA-256), seuls les changés téléchargés ; *revue après L0 (L-Q2 bis = C)* : **pas de chunks pour l'instant**, fichiers **compressés au transfert**, et **correctifs binaires** des fichiers changés (l'exe surtout), le fichier complet en secours |
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
