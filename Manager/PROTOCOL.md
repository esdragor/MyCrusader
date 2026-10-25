# Protocole du gestionnaire de salons (HOLManager)

Contrat entre le **gestionnaire** (`Manager/HOLManager`, C#, toujours allumé sur le PC hôte), le **jeu des joueurs**
(R4, `UHOLManagerClient`) et le **serveur de partie Unreal** (R3, `HeroesOfLibertyServer.exe`). Chantier M6, étape
R2.2 ; décisions M6-Q1 à M6-Q9 de `NetPlaytestInProgress.md`.

**Version du protocole : 1.** Toute modification incompatible (route retirée, champ renommé ou de sens changé) passe
à 2 ; un champ ajouté ne la change pas (les deux côtés ignorent les champs inconnus).

## Choix ouverts (à faire valider par l'utilisateur)

1. **Port du gestionnaire : 7700** (TCP, toutes les adresses, donc joignable par Tailscale) ; serveurs de partie
   **7777 et suivants**. Le pare-feu de Windows demandera l'autorisation au premier lancement.
2. **Version du jeu servie** : lue dans un fichier `version.txt` (écrit par `package.ps1`, R1.7) à côté du
   `HeroesOfLibertyServer.exe` que le gestionnaire lance. Pas de fichier → version `"dev"`, et seul un client `"dev"`
   entre.
3. **Le joueur garde l'emplacement reçu à l'entrée** (premier emplacement ouvert) : pas de route pour changer
   d'emplacement (= position de départ) dans cette version ; « ma ligne » = couleur, équipe (faction plus tard).
4. **« Prêt » remis à zéro** : un changement de **ma** ligne remet **mon** « Prêt » à zéro ; un changement du chef
   (carte, options, emplacement ouvert / fermé) remet **tous** les « Prêt » à zéro. Le chef doit lui aussi être
   « Prêt » avant « Lancer ».
5. **Pas d'IA dans les salons** pour l'instant : un emplacement est ouvert, fermé ou tenu par un joueur. Les
   emplacements ouverts restés vides au lancement sont **fermés** dans `match.json`.
6. **Retour au salon après la partie** (M6-Q7, R2.11, D2) : après `finished`, le salon repasse `lobby` avec ses
   réglages mais **sans les joueurs de la partie** : aucune ligne ni aucun nom ne leur est réservé, personne n'est
   attendu ; « Retour au salon » n'est qu'un raccourci pour `POST /join` sous le même nom (refusé comme tout `join` :
   `name_taken`, `lobby_full`, `lobby_not_found`, `lobby_in_game` ; le client revient alors à la liste avec le message
   en rouge, R4.9). **Chef** (M6-Q14) : le chef de la partie **reste chef** tant qu'il n'est pas revenu (`leader` donne
   son nom) ; pendant son absence, personne d'autre ne règle ni ne lance (`not_leader`) ; revenu par `join` (s'il y a
   une place), il reprend la main. Le premier revenu devient chef seulement si le salon est plein sans lui, s'il a
   quitté le serveur de la partie (`left`) ou si ce serveur s'est arrêté, sans être revenu dans les **30 s** (le temps
   de « Retour au salon », qui quitte le serveur avant le `join`). Le chef peut relancer sans attendre les autres. Le salon, même vide,
   reste tant que le serveur de sa partie finie tourne. **Serveur arrêté sans `finished`** (`server_stopped`) : chaque
   joueur de la partie **garde sa ligne** (emplacement, couleur, équipe) et les réglages restent ; pendant **2 min**, il
   n'est **pas retiré par le silence** et **son nom lui est réservé** : `POST /join` avec ce nom reprend sa ligne avec
   un **nouveau jeton** (l'ancien ne vaut plus). Il est « revenu » à son premier appel avec un jeton ou à ce `join`.
   **Chef** : l'ancien chef s'il est revenu, sinon le premier revenu. Passé 2 min, la règle des 5 s reprend : ceux qui
   ne sont pas revenus (ou se sont tus) sont retirés et libèrent leur place. Voir « Retour au salon » plus bas.
7. **Routes du serveur Unreal acceptées seulement depuis la machine elle-même** (`127.0.0.1`) : un client ne peut pas
   se faire passer pour un serveur.
8. **Noms** : 1 à 16 caractères, lettres (accents compris), chiffres, espace, `-`, `_` ; comparés sans tenir compte de
   la casse ; noms de salon : 1 à 24 caractères, mêmes règles plus `'`. Les espaces autour du nom sont retirés avant
   la vérification (un nom fait seulement d'espaces est refusé) ; les caractères sont comptés en forme composée
   (`é` = 1).
9. **État `starting`** entre « Lancer » et « le serveur écoute » (quelques secondes) : la liste l'affiche « en
   partie » sans minutes.

## Généralités

- Adresse : `http://<adresse du PC hôte>:7700`, lue par le jeu dans `servers.json` (M6-Q5). Pas de HTTPS : Tailscale
  chiffre déjà le trafic.
- Corps des requêtes et des réponses en **JSON UTF-8**, noms de champs en `camelCase`. Pas de WebSocket : les clients
  **lisent** l'état (2 fois par seconde dans un salon).
- Un joueur dans un salon est identifié par un **jeton** (`token`) rendu à l'entrée, renvoyé ensuite dans l'en-tête
  **`X-Player-Token`**. Pourquoi un jeton plutôt que le nom : c'est simple (une chaîne tirée au hasard, gardée par le
  client le temps du salon), cela ne dépend pas d'un nom qu'un autre client pourrait taper par erreur, et cela
  distingue deux entrées successives du même nom. Ce **n'est pas un secret** ni un mot de passe : pas de compte, pas
  de chiffrement ; c'est **Tailscale** qui décide qui peut joindre le PC.
- Identifiants : `lobbyId` (ex. `"L7"`), `matchId` (ex. `"M20261023-153012-3"` : `M`, date et heure UTC du
  lancement, numéro d'ordre), `token` (32 caractères
  hexadécimaux). Les jetons et identifiants ne sont valables que tant que le gestionnaire tourne.

### Erreurs

Toute erreur rend un code HTTP et ce corps :

```json
{ "error": "lobby_full", "message": "The lobby is full (8 players)." }
```

`error` est un code stable : **le jeu choisit le texte affiché (fr / en / ja) d'après ce code** ; `message` (anglais)
sert au journal. Un code inconnu du jeu → message générique avec le `message` brut.

| `error` | HTTP | Quand |
|---|---|---|
| `bad_request` | 400 | JSON illégal, champ manquant ou hors bornes (couleur, équipe, index d'emplacement, carte inconnue) |
| `invalid_name` | 400 | nom de joueur ou de salon hors des règles |
| `version_mismatch` | 409 | `gameVersion` du client ≠ version servie ; `message` donne les deux |
| `protocol_mismatch` | 409 | (réservé) le client demande une version du protocole que le gestionnaire ne parle pas |
| `lobby_not_found` | 404 | salon inconnu ou supprimé (vide) |
| `not_in_lobby` | 403 | jeton absent, inconnu, ou joueur retiré (5 s sans lecture, « Quitter ») |
| `lobby_full` | 409 | plus d'emplacement ouvert libre |
| `name_taken` | 409 | nom déjà tenu par un joueur du salon (sauf le retour d'un joueur de la partie, choix 6) |
| `lobby_in_game` | 409 | entrée refusée : partie en cours (« Regarder » reste possible) |
| `color_taken` | 409 | couleur déjà prise par un autre joueur du salon (premier arrivé, M6-Q3) |
| `not_leader` | 403 | réglage ou « Lancer » demandé par un autre que le chef |
| `slot_taken` | 409 | ouvrir ou fermer un emplacement tenu par un joueur ; carte de moins d'emplacements que ceux tenus |
| `not_all_ready` | 409 | « Lancer » alors qu'un joueur n'est pas « Prêt » |
| `not_enough_players` | 409 | « Lancer » seul dans le salon (2 joueurs au moins) |
| `max_servers` | 503 | « Lancer » alors que le maximum de serveurs de partie tourne déjà (M6-Q6) |
| `server_start_failed` | 500 | l'exécutable du serveur est introuvable ou n'a pas démarré |
| `no_match` | 409 | « Regarder » un salon qui n'est pas en partie |
| `not_local` | 403 | route du serveur Unreal appelée depuis une autre machine |
| `match_not_found` | 404 | `matchId` inconnu ou oublié (serveur arrêté) (route du serveur Unreal) |

Un corps JSON illégal (syntaxe, type de champ faux) est un `bad_request` **vérifié avant tout le reste** ; l'ordre des
vérifications donné pour chaque route vaut pour un JSON légal.

« **Le serveur de la partie s'est arrêté** » n'est pas une réponse à une requête : c'est un **avis** dans l'état du
salon (`notice: "server_stopped"`, voir plus bas).

## Routes du jeu (clients)

### `GET /status`

État du gestionnaire ; premier appel du jeu (écran Multijoueur), aussi utile pour vérifier à la main qu'il tourne.

```json
{ "protocolVersion": 1, "gameVersion": "2026-10-23-b4a8c8c", "lobbies": 2, "servers": 1, "maxServers": 2 }
```

### `GET /lobbies` — liste des salons

```json
{
  "lobbies": [
    { "id": "L7", "name": "Salon de Bob", "players": 3, "maxPlayers": 8, "state": "lobby",
      "minutes": null, "canJoin": true, "canWatch": false },
    { "id": "L8", "name": "2v2", "players": 4, "maxPlayers": 4, "state": "in_game",
      "minutes": 12, "canJoin": false, "canWatch": true }
  ]
}
```

- `state` : `"lobby"` (au salon), `"starting"` (serveur lancé, pas encore à l'écoute), `"in_game"` (en partie).
- `minutes` : minutes de partie écoulées depuis « partie lancée » (entier, arrondi vers le bas) ; `0` en partie avant
  « partie lancée » (`started`) ; `null` hors de `in_game` (`starting` compris).
- `maxPlayers` : emplacements **ouverts ou tenus** (le chef en ferme pour une partie à 4).
- `canJoin` : au salon et pas plein. `canWatch` : en partie (`"in_game"`).

### `POST /lobbies` — créer un salon

Crée le salon **et y entre** : le créateur est chef (pas de salon vide).

```json
{ "name": "Salon de Bob", "playerName": "Bob", "gameVersion": "2026-10-23-b4a8c8c" }
```

Réponse `201` : la même que l'entrée (ci-dessous). Réglages de départ : carte par défaut du gestionnaire (`Plain`), 8
emplacements ouverts, options désactivées. Erreurs : `invalid_name`, `version_mismatch`, `bad_request`.

### `POST /lobbies/{lobbyId}/join` — entrer avec un nom

```json
{ "playerName": "Alice", "gameVersion": "2026-10-23-b4a8c8c" }
```

Réponse `200` :

```json
{ "token": "9f2c4e0b7a3d4c1e8b6f5a2d0c9e7b14", "slot": 1, "lobby": { "…": "état du salon, voir GET /lobbies/{lobbyId}" } }
```

Le joueur reçoit le **premier emplacement ouvert libre**, la **première couleur libre** de la palette (0 à 7) et
l'équipe 0 (aucune). Le premier entré d'un salon est chef. Erreurs : `lobby_not_found`, `version_mismatch`,
`invalid_name`, `name_taken`, `lobby_full`, `lobby_in_game`, `bad_request`.

Ordre des vérifications (la première qui échoue donne l'erreur) : `lobby_not_found`, `bad_request` (champ manquant),
`version_mismatch`, `invalid_name` (nom du joueur, puis du salon pour `POST /lobbies`), `lobby_in_game`, puis
**retour d'un joueur de la partie** (nom réservé, choix 6 : réponse `200` avec sa ligne et un nouveau jeton, même salon
plein), `lobby_full`, `name_taken`. Une requête refusée ne change rien.

### `GET /lobbies/{lobbyId}` — état du salon (en-tête `X-Player-Token`)

Lu **2 fois par seconde** par chaque client au salon. **Cette lecture vaut présence** : un joueur dont le jeton n'a
pas lu l'état depuis **5 s** est retiré comme s'il avait quitté (pas pendant `starting` / `in_game` : il est alors
dans la partie ; pas non plus dans les 2 min qui suivent le retour au salon, choix 6). Sans jeton, la lecture est permise mais ne vaut pas présence (outil, débogage). **Tout appel avec un
jeton valable** (lecture, ma ligne, réglages, emplacement) vaut aussi présence. Le retrait est fait au début de chaque
appel au gestionnaire, et aussi une fois par seconde par le gestionnaire lui-même (ménage, R2.10) : un joueur muet
depuis 5 s ou plus est retiré avant que sa requête soit traitée (`not_in_lobby`). Un retrait (départ ou silence) qui change le chef pose `notice: "leader_changed"` pour 10 s.

```json
{
  "id": "L7",
  "name": "Salon de Bob",
  "state": "lobby",
  "minutes": null,
  "leader": "Bob",
  "you": "Alice",
  "settings": { "map": "Plain", "nomad": false, "regicide": false, "catastrophe": false },
  "maps": [ { "name": "Plain", "maxPlayers": 8 } ],
  "slots": [
    { "index": 0, "state": "player", "playerName": "Bob",   "colorIndex": 0, "team": 1, "faction": "DawnOrder", "ready": true },
    { "index": 1, "state": "player", "playerName": "Alice", "colorIndex": 3, "team": 2, "faction": "DawnOrder", "ready": false },
    { "index": 2, "state": "open" },
    { "index": 3, "state": "closed" }
  ],
  "match": null,
  "notice": null
}
```

- `slots` : toujours 8 ; `state` vaut `"open"`, `"closed"` ou `"player"`. `you` : le nom du jeton (`null` sans
  jeton).
- `faction` : rendue mais pas encore modifiable (D11 : deux factions au prototype).
- `match` : `null` hors de `in_game` ; en partie, ce que le client doit faire (voir « Démarrage de la partie ») ;
  lu sans jeton, `playerName` y vaut `null`.
- `notice` : avis à montrer une fois puis effacé au bout de 10 s : `"server_stopped"` (le serveur de la partie
  s'est arrêté), `"leader_changed"`, `null`.
- Erreurs : `lobby_not_found`, `not_in_lobby` (le client revient à la liste).

### `PUT /lobbies/{lobbyId}/me` — ma ligne (en-tête `X-Player-Token`)

Champs facultatifs ; seuls ceux présents changent.

```json
{ "colorIndex": 4, "team": 2, "ready": true }
```

- `colorIndex` : 0 à 7, unique dans le salon (`color_taken`). `team` : 0 (aucune) à 4.
- Un changement de couleur ou d'équipe remet **mon** « Prêt » à `false` (choix 4), sauf si `ready` est dans la même
  requête. Une valeur égale à l'actuelle n'est pas un changement.
- Plus tard : `faction` (`"DawnOrder"`, `"BlackLegions"`).
- Réponse `200` : l'état du salon. Erreurs : `lobby_not_found`, `not_in_lobby`, `lobby_in_game`, `bad_request`,
  `color_taken` (dans cet ordre).

### `PUT /lobbies/{lobbyId}/settings` — réglages du chef (en-tête `X-Player-Token`)

```json
{ "map": "Plain", "nomad": true, "regicide": false, "catastrophe": false }
```

Champs facultatifs. Un changement (valeur différente de l'actuelle) remet **tous** les « Prêt » à `false`. Une carte
de moins d'emplacements ferme ceux de trop (refus `slot_taken` s'ils sont tenus) ; une carte plus grande ne les rouvre
pas (le chef les rouvre). Réponse `200` : l'état du salon. Erreurs : `lobby_not_found`, `not_in_lobby`,
`lobby_in_game`, `not_leader`, `bad_request` (carte inconnue), `slot_taken` (dans cet ordre).

### `PUT /lobbies/{lobbyId}/slots/{index}` — ouvrir / fermer un emplacement (chef)

```json
{ "state": "closed" }
```

`state` : `"open"` ou `"closed"`. Un changement remet tous les « Prêt » à `false` (ouvrir un emplacement déjà ouvert
ne change rien). Erreurs : `lobby_not_found`, `not_in_lobby`, `lobby_in_game`, `not_leader`, `bad_request` (index
hors de 0 à 7 ou non entier, `state` absent ou inconnu, ouvrir un emplacement au-delà de la carte), `slot_taken` (emplacement tenu).

### `POST /lobbies/{lobbyId}/start` — « Lancer » (chef)

Conditions : chef, 2 joueurs au moins, **tous « Prêt »**, moins de `maxServers` serveurs en marche. Le gestionnaire
écrit `match.json` (format ci-dessous), lance le serveur et passe le salon à `starting`. Réponse `202` :
`{ "matchId": "M20261023-153012-3" }` ; l'état du salon dit alors `"starting"`, `match` reste `null` jusqu'à
`listening`. Erreurs, dans cet ordre : `lobby_not_found`, `not_in_lobby`, `lobby_in_game`, `not_leader`,
`not_enough_players`, `not_all_ready`, `max_servers`, `server_start_failed`. Une requête refusée ne change rien (le
salon reste au salon). « Lancer » vaut présence comme tout appel avec un jeton.

### `POST /lobbies/{lobbyId}/leave` — quitter (en-tête `X-Player-Token`)

Réponse `204`. Erreurs : `lobby_not_found`, `not_in_lobby`. Le chef qui part est remplacé par le joueur **entré le plus tôt** parmi ceux qui restent ; un salon vide
est supprimé (au salon seulement : un salon en partie vit jusqu'à la fin du serveur). Quitter rend aussi la place et
le nom réservés après une partie (choix 6).

### `GET /lobbies/{lobbyId}/watch` — « Regarder » (spectateur, M6-Q8)

Sans jeton. Réponse `200` : `{ "matchId": "…", "port": 7778, "minutes": 12 }` (salon `in_game` seulement :
`starting` → `no_match`). Le client se connecte avec
`?Version=…` et **sans `Name`** (ou un nom absent de la configuration) : le serveur en fait un spectateur (R3.2).
Erreurs : `lobby_not_found`, `no_match`.

## Démarrage de la partie (ce que reçoit le client)

Quand le serveur annonce qu'il écoute, l'état du salon passe à `"in_game"` et `match` devient :

```json
"match": { "matchId": "M20261023-153012-3", "port": 7778, "playerName": "Alice", "gameVersion": "2026-10-23-b4a8c8c" }
```

Le client ouvre alors `<adresse du PC hôte>:<port>?Name=<playerName>&Version=<gameVersion>` (même adresse que le
gestionnaire, celle de `servers.json`). `playerName` est **le nom à passer tel quel** (encodé pour l'URL) : c'est lui
qui donne l'emplacement (M6-Q9, R3.2). Le serveur vérifie `Version` (R3.3).

Pendant la partie, le client ne lit plus l'état du salon. À la fin, « Retour au salon » (bouton, ou seul ~60 s après
la fin, R4.8) = `POST /join` avec le même nom, quand il veut, comme un nouveau venu (choix 6, D2) ; « Quitter vers la
liste » = rien (il n'a plus de place dans le salon depuis `finished`).

### Retour au salon (choix 6, R2.11)

- Après `finished` (D2) : salon `lobby`, réglages gardés, **plus aucun joueur de la partie** (ancien jeton →
  `not_in_lobby`). Chaque joueur revient par un `POST /join` ordinaire (nouvel emplacement, couleur et équipe par
  défaut), refusé comme tout `join`. **Chef** (M6-Q14) : le chef de la partie le reste, absent (`leader` = son nom) ;
  ceux revenus avant lui ne règlent ni ne lancent (`not_leader`) ; à son `join`, il reprend la main (devant eux dans
  l'ordre d'arrivée). Le premier revenu (ordre d'arrivée) devient chef, avec `notice: "leader_changed"` s'il n'est pas
  seul, quand : le salon est **plein** sans lui (au `join` qui le remplit ; le chef revenu ensuite a `lobby_full`) ;
  le chef a **quitté le serveur** de la partie (`left` : « Quitter vers la liste », déconnexion), ou ce **serveur
  s'est arrêté**, sans `join` du chef dans les **30 s** qui suivent ; le chef parti du serveur **avant** `finished`
  ne l'est plus après. Personne revenu : pas de nouveau chef, `leader` garde le nom du chef absent ; le premier qui
  revient est alors chef si le chef ne l'est plus. « Lancer » ne compte que les joueurs
  revenus. Le salon, même vide, n'est pas supprimé tant que le serveur de la partie tourne (jusqu'à son arrêt, après
  `empty`) ; ensuite, vide, il est supprimé, et avec quelqu'un revenu, il suit les règles ordinaires.
- Les points suivants ne valent plus que pour le **serveur arrêté sans `finished`** (`server_stopped`).
- Au retour au salon (serveur arrêté avec `server_stopped`), chaque joueur encore au salon garde sa
  ligne, « Prêt » à `false`, et est **réservé 2 min** : pas retiré par le silence, nom à lui.
- `POST /join` avec le nom d'un joueur réservé (casse ignorée) : même ligne, **nouveau jeton**, réponse `200` ;
  l'ancien jeton donne ensuite `not_in_lobby`. Le gestionnaire ne distingue pas deux clients du même nom : c'est le
  dernier `join` qui tient la ligne (pas de compte, choix du jeton plus haut).
- Revenu = premier appel avec son jeton (lecture comprise) ou `join` par son nom. **Chef** = l'ancien chef s'il est
  revenu ; sinon le premier revenu ; personne revenu → l'ancien chef. (L'ancien chef garde son ordre d'arrivée, les
  autres arrivent de nouveau à leur retour.)
- Un nouveau venu prend un emplacement ouvert, jamais une ligne réservée. « Lancer » demande tous « Prêt » : un joueur
  pas revenu bloque donc « Lancer » jusqu'à ce qu'il revienne ou que ses 2 min passent.
- Après 2 min, la règle des 5 s reprend : les joueurs muets depuis 5 s ou plus (dont ceux pas revenus) sont retirés ;
  un salon où personne n'est revenu est supprimé.

## Routes du serveur Unreal

Le gestionnaire lance chaque serveur ainsi (un serveur neuf par partie, M6-Q9) :

```
HeroesOfLibertyServer.exe /Game/Maps/Plain -Port=7778 -MatchConfig="<dossier>\M20261023-153012-3\match.json"
    -ManagerUrl=http://127.0.0.1:7700 -MatchId=M20261023-153012-3 -log -unattended -useperfthreads
    -abslog="<dossier>\M20261023-153012-3\server.log"
```

- Carte en premier argument, en chemin de paquet (`/Game/Maps/<Nom>`) ; dans `match.json`, chemin d'objet
  (`/Game/Maps/<Nom>.<Nom>`).
- Les chemins sont entre guillemets **après le `=`** (`-MatchConfig="C: b\match.json"`), pas autour de toute
  l'option : `FParse::Value` d'Unreal lit la valeur après `=` et s'arrête à l'espace si elle ne commence pas par un
  guillemet.
- `-useperfthreads` : comme `Tools/Tests/net_match.ps1` (J7c.4) ; sans lui, un serveur dédié fait tourner chaque
  `ParallelFor` sur un seul fil.
- Port : le premier à partir de 7777 libre en **UDP** (Unreal écoute en UDP) et pas déjà donné à un serveur du
  gestionnaire encore en marche ; 100 ports essayés, puis `server_start_failed`.
- Le serveur est lancé sans fenêtre de console (son journal va dans `-abslog`) ; à l'arrêt du gestionnaire (Ctrl+C,
  fenêtre fermée, arrêt de Windows), les serveurs encore en marche sont arrêtés.
- Démarrage avec Windows (R2.12) : `Tools\Manager\install_startup.ps1` met le raccourci « Serveur HeroesOfLiberty »
  (fenêtre réduite, dossier de travail = celui de l'exe) dans le dossier Démarrage ; `uninstall_startup.ps1` l'enlève.
  Journal du gestionnaire : `Logs\manager-<aaaa-mm-jj>.log` à côté de l'exe, un fichier par jour, 14 gardés.

`<dossier>` = `Saved\Matches\` à côté du gestionnaire. Réglages (`appsettings.json` ou `--Clé=Valeur`) :
`ServerExe`, `MaxServers` (2), `FirstServerPort` (7777), `MatchesDir` (`Saved\Matches`), `ManagerUrl`
(`http://127.0.0.1:7700`), `LogDir` (`Logs` ; vide = pas de fichier). Le serveur appelle (corps JSON, réponse `204`, depuis
`127.0.0.1` seulement ; erreurs `not_local`, `match_not_found`) :

| Route | Corps | Effet dans le gestionnaire |
|---|---|---|
| `POST /matches/{matchId}/listening` | `{ "port": 7778 }` | salon `starting` → `in_game` ; `match` rempli pour les clients ; `port` absent ou 0 → port donné au lancement |
| `POST /matches/{matchId}/started` | `{}` | départ de l'horloge des `minutes` (tous arrivés ou ~60 s passées, R3.9) ; seul le premier appel compte |
| `POST /matches/{matchId}/finished` | `{ "winners": [0, 2] }` (emplacements) | salon remis `lobby`, sans avis, réglages gardés, **sans les joueurs de la partie** (aucune réservation ; retour = `join`) ; gardé même vide jusqu'à l'arrêt du serveur (choix 6, D2) |
| `POST /matches/{matchId}/left` | `{ "playerName": "Bob" }` | un joueur nommé a quitté le serveur (`Logout`, après `started`) ; pendant la partie : retenu pour `finished` ; après : si c'est le chef absent, il ne l'est plus que 30 s (M6-Q14) ; autre nom : rien ; `playerName` absent : `bad_request` |
| `POST /matches/{matchId}/empty` | `{}` | plus aucun client ; le serveur va quitter (R3.8) ; le gestionnaire l'arrête s'il tourne encore 10 s après |

- Ordre des vérifications : `not_local`, `bad_request` (JSON illégal, ou `playerName` absent pour `left` ; un corps vide est accepté ailleurs), `match_not_found`.
- Un appel répété, ou `listening` après `finished`, ne change rien (réponse `204`).
- Un `matchId` est connu de « Lancer » jusqu'à l'arrêt de son serveur (après `finished` aussi) ; ensuite
  `match_not_found`.

Le gestionnaire **suit le processus** (au début de chaque appel et une fois par seconde, sous le verrou des salons) :
s'il disparaît sans `finished` (avant `listening` comme en partie), le salon revient à `lobby` avec
`notice: "server_stopped"` ; un serveur qui n'a pas appelé `listening` dans les 120 s est arrêté, même avis ; un
serveur arrêté 10 s après `empty` sans `finished` donne le même avis. Au retour au salon par cet avis, les
joueurs gardent leur ligne et leur jeton, « Prêt » à zéro, et sont réservés 2 min (« Retour au salon » plus haut) ;
un salon que tous ont quitté pendant la partie est supprimé. Après `finished` (salon sans les joueurs de la partie,
gardé jusqu'à l'arrêt du serveur), le serveur reste suivi : il quitte seul
quand il est vide (`empty`, R3.8) et le gestionnaire l'arrête s'il tourne encore 10 s après.

## `match.json` (`FHOLMatchConfig` version 2)

Écrit par « Lancer », lu par le serveur avec `-MatchConfig=` (D243, `Source/HeroesOfLiberty/Match/HOLMatchConfig.h`).
C'est le JSON de `FJsonObjectConverter` (premières lettres en minuscule, énumérations par leur nom, noms de champs
lus sans tenir compte de la casse). **Version 2 = version 1 + `playerName` par emplacement** (R3.1,
`FHOLMatchSlot::PlayerName`) :

| Champ | Type | Sens |
|---|---|---|
| `version` | entier | `2` |
| `seed` | entier | graine de la partie (tirée par le gestionnaire) |
| `map` | chaîne | chemin d'objet de la carte, ex. `/Game/Maps/Plain.Plain` |
| `slots` | 8 objets | position de départ = index |
| `slots[].state` | `"Closed"`, `"Human"`, `"AI"` | `"Human"` pour chaque joueur du salon ; ouvert vide ou fermé → `"Closed"` (faction `DawnOrder`, couleur = index, équipe 0, comme `Normalize`) |
| `slots[].faction` | `"DawnOrder"`, `"BlackLegions"`, … | |
| `slots[].colorIndex` | 0 à 7 | palette D233 |
| `slots[].team` | 0 à 4 | 0 = aucune |
| `slots[].playerName` | chaîne | **nouveau en version 2** : nom du joueur attendu (tel qu'au salon : sans espaces autour, forme composée) ; `""` pour un emplacement fermé ou une IA |
| `mode` | `"Standard"` | |
| `bNomad`, `bRegicide`, `bCatastrophe` | booléens | options du mode Standard |

Le nom `playerName` est celui que `FJsonObjectConverter` écrit pour une propriété `PlayerName`
(`StandardizeCase` : première lettre en minuscule ; les booléens gardent leur `b`) ; il le relit sans tenir compte de
la casse. Un fichier de version 1 (sans `playerName`) se lit encore : noms vides, le serveur donne alors le premier
emplacement humain libre comme aujourd'hui (R3.2). Exemple complet (3 joueurs, 2 contre 1, emplacement 1 fermé par le
chef, Nomade) :

```json
{
  "version": 2,
  "seed": 184467,
  "map": "/Game/Maps/Plain.Plain",
  "slots": [
    { "state": "Human", "faction": "DawnOrder", "colorIndex": 0, "team": 1, "playerName": "Bob" },
    { "state": "Closed", "faction": "DawnOrder", "colorIndex": 1, "team": 0, "playerName": "" },
    { "state": "Human", "faction": "DawnOrder", "colorIndex": 3, "team": 2, "playerName": "Alice" },
    { "state": "Human", "faction": "DawnOrder", "colorIndex": 5, "team": 1, "playerName": "Chloé" },
    { "state": "Closed", "faction": "DawnOrder", "colorIndex": 4, "team": 0, "playerName": "" },
    { "state": "Closed", "faction": "DawnOrder", "colorIndex": 5, "team": 0, "playerName": "" },
    { "state": "Closed", "faction": "DawnOrder", "colorIndex": 6, "team": 0, "playerName": "" },
    { "state": "Closed", "faction": "DawnOrder", "colorIndex": 7, "team": 0, "playerName": "" }
  ],
  "mode": "Standard",
  "bNomad": true,
  "bRegicide": false,
  "bCatastrophe": false
}
```

Ce fichier sert aussi de **référence partagée** entre le test C# de R2.8 et le test C++ de R3.1
(`Manager/HOLManager.Tests/Data/match_v2_reference.json`, créé en R2.8 ; le test C# le compare au JSON produit).

## Versions du jeu (lanceur, L2)

Routes lues par le lanceur (`LauncherInProgress.md`). Elles **s'ajoutent** au protocole sans rien changer
(`protocolVersion` reste 1) ; la réponse de `/versions/latest` porte son propre `format` (1). Erreurs : même corps
`{ "error", "message" }` que plus haut.

Dossier des versions : réglage `VersionsDir` (défaut `Versions`, relatif au dossier du gestionnaire), rempli par le
script de publication (L4) :

```
<versions>/latest.txt                          nom de la dernière version
<versions>/<v>/manifest.json                   liste des fichiers (HOLUpdate : chemin, taille, SHA-256)
<versions>/<v>/notes.txt                       nouveautés (facultatif)
<versions>/<v>/files/<chemin>.gz               fichier compressé (gzip) à la publication
<versions>/<v>/patches/<depuis>/<chemin>.patch correctif binaire depuis la version <depuis> (L3/L4)
<versions>/launcher/latest.txt                 version du lanceur
<versions>/launcher/<lv>/HOLLauncher.exe
```

Noms de version (`{v}`, `{depuis}`, contenu des `latest.txt`) : `[A-Za-z0-9._-]+`, sauf `.` et `..`.

### `GET /versions/latest`

```json
{
  "format": 1,
  "version": "2026-10-25-b",
  "manifest": { "format": 1, "version": "2026-10-25-b",
                "files": [ { "path": "HeroesOfLiberty.exe", "size": 348127232, "sha256": "3f5a…" } ] },
  "notes": "Archers plus rapides.",
  "patchesFrom": [ "2026-10-24", "2026-10-25-a" ],
  "launcherVersion": "L3"
}
```

`version` est le nom du dossier (celui des routes ci-dessous) ; `manifest` est `manifest.json`, relu et vérifié
(chemins sûrs, empreintes) ; `notes` vide sans `notes.txt` ; `patchesFrom` : les dossiers de `<v>/patches/`, triés ;
`launcherVersion` vide sans `launcher/latest.txt`.

| `error` | HTTP | Quand |
|---|---|---|
| `no_version` | 404 | pas de `latest.txt`, nom illégal, ou dossier absent |
| `bad_manifest` | 500 | `manifest.json` illisible ou refusé ; `message` dit pourquoi |

### `GET /versions/{v}/files/{chemin}`

Envoie `<v>/files/<chemin>.gz` **tel quel** (`application/gzip`) : le lanceur décompresse puis vérifie l'empreinte de
la liste. **Reprise** par l'en-tête `Range` (réponse 206). `<chemin>` doit être **exactement** un `path` de la liste
de `v` (casse comprise) ; le fichier résolu doit en plus rester sous `<versions>/<v>/`.

| `error` | HTTP | Quand |
|---|---|---|
| `invalid_version` | 400 | `{v}` hors des règles de nom |
| `version_not_found` | 404 | pas de `<v>/manifest.json` |
| `file_not_found` | 404 | chemin hors de la liste, ou `.gz` absent |
| `bad_manifest` | 500 | liste de `v` illisible |

### `GET /versions/{v}/patches/{depuis}/{chemin}`

Envoie `<v>/patches/<depuis>/<chemin>.patch` (`application/octet-stream`), `Range` comme plus haut. Mêmes
vérifications (`{depuis}` compris) ; correctif absent → `patch_not_found` (404) : le lanceur prend alors le fichier
complet.

### `GET /launcher/latest`

Envoie `launcher/<lv>/HOLLauncher.exe` (`application/octet-stream`), `Range` comme plus haut, avec deux en-têtes,
présents aussi sur une réponse 206 : **`X-Launcher-Version`** (`<lv>`) et **`X-Launcher-Sha256`** (empreinte du
fichier entier, calculée une fois par fichier). Pas de lanceur publié → `launcher_not_found` (404).
