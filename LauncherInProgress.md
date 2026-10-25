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
| L-Q2 | **Liste des fichiers avec empreinte** (SHA-256), seuls les changés téléchargés, **contenu découpé en chunks** |
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
| L2 Routes du gestionnaire | `GET /versions/latest` (version, liste, nouveautés, version du lanceur), `GET /versions/{v}/files/{chemin}` (reprise par `Range`), `GET /launcher/latest` ; `PROTOCOL.md` et sa version | tests verts |
| L3 Chunks | réglage d'empaquetage (`bGenerateChunks`, étiquettes d'assets primaires) selon L0 ; contrôle de fumée du jeu empaqueté | `packaged_smoke` vert, paquets séparés |
| L4 Publication | `Tools/Package/publish.ps1` : `package.ps1`, liste par `HOLUpdate`, `Nouveautes.txt`, copie dans le dossier des versions, « dernière » marquée, N anciennes gardées ; les nouveaux serveurs de partie partent de la dernière, ceux en cours finissent sur la leur | deux versions publiées de suite |
| L5 Squelette du lanceur | `Manager/HOLLauncher` (WinForms, un seul exe autonome) : adresse lue dans le `servers.json` du jeu (M6-Q5), fenêtre (versions, nouveautés, « Jouer », deux boutons) | fenêtre qui lit `/versions/latest` |
| L6 Mise à jour du jeu | différence, téléchargement dans un dossier temporaire, empreinte vérifiée, remplacement, fichiers retirés supprimés, barre de progression, reprise après coupure | tests verts avec `FakeServer` |
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
