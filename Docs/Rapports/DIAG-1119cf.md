# DIAG-1119cf : 404 de `lobby_match` sur integ-18 (2026-11-19, 17:18-17:31)

Agent en lecture seule, 17:34-17:40. Aucun fichier du projet modifié.

## Verdict en une ligne

(b) L'OUTIL, certitude assez forte (environ 80 %) : une course de 3 s dans le scénario `lobby_match.ps1`, aggravée par la charge (c, facteur secondaire). Aucune trace du JEU (a).

## Cause

Le salon L1 est supprimé par le gestionnaire quand le dernier joueur le quitte (`POST /lobbies/L1/leave` du second joueur). Dans le scénario, Alice quitte le jeu 3 s après son retour au salon et Bob 10 s après : le salon n'existe plus à 50,8 s. La lecture « les deux sont revenus » du test n'a une fenêtre valide que pendant ces 3 s. Passé ce délai, Alice a quitté, le salon n'a plus que Bob, la boucle ajoutée par 1ace000 (attente jusqu'à 20 s de « Alice ET Bob » nommés) tourne dans le vide jusqu'à ce que Bob quitte à son tour, et le GET suivant rend 404, que `Invoke-RestMethod` transforme en exception (`lobby_match.ps1:630-636`, pas de try/catch).

Les 3 s viennent du script : `$Back = ... "waitlog Menu: back in lobby", "wait 3", "quit"` (`lobby_match.ps1:157`), repris par Alice (queue `"wait 3","quit"`) ; Bob a `"signal $BobBack", "waitfile $AliceBack 240", "wait 10", "quit"` (bloc Bob, vers la ligne 588).

## Chronologie de la passe rouge (journal du jour du gestionnaire, heure locale)

Gestionnaire démarré 17:24:53 (un second à 17:28:55 : relance, voir « non vérifié »). Seul 404 de la journée sur `GET /lobbies/L1`.

| Heure | Appel | Réponse / sens |
|---|---|---|
| 17:25:25.5 | POST /lobbies (Alice) | crée L1 |
| 17:25:56.6 | POST /lobbies/L1/join (Bob) | ok |
| 17:26:01.9 | POST /lobbies/L1/start | ok |
| 17:26:31.4 | POST /matches/M20261119-162601-1/listening | 29,4 s après start (verte : 13,3 s) |
| 17:26:34.1 | .../started | |
| 17:27:26.4 | .../finished | fin de partie décidée et transmise |
| 17:27:28.5 | .../left puis POST /lobbies/L1/join (Bob) | « Bob back first : lobby, players (Bob), leader Alice » |
| 17:28:40.6 | .../left puis .../empty | Alice sort de la partie (« Alice stayed », 2 serveurs, avant) |
| 17:28:40.74 | POST /lobbies/L1/join (Alice) | salon à deux |
| 17:28:43.82 | POST /lobbies/L1/leave | **Alice quitte** (+3,08 s : « wait 3 », « quit ») ; salon = Bob seul |
| 17:28:50.80 | POST /lobbies/L1/leave | **Bob quitte** (+10 s : « wait 10 ») ; salon vide, supprimé |
| 17:28:50.892 | GET /lobbies/L1 | **404** (0,09 s après la suppression) |

241 s de test = 17:24:53 + 241 s = 17:28:54 : cohérent avec l'échec à 17:28:50.9.

## Chronologie de la passe verte (même fichier, gestionnaire démarré 15:44:02)

| Heure | Appel |
|---|---|
| 15:44:48.8 | start (15:45:02 listening : 13,3 s ; 15:45:04 started) |
| 15:45:47.4 | finished |
| 15:45:49.4 | join (Bob revenu le premier) |
| 15:47:01.3-.4 | left, empty, puis join (Alice revenue) |
| 15:47:04.49 | leave d'Alice (+3,07 s, identique à la rouge) |

Le test a lu L1 avec succès avant ce leave. Les passes de 12:18 (join 12:18:00.14, leave 12:18:04.13) et 13:18 (13:18:20.63 / 13:18:23.73) ont la même forme : la fenêtre valide est toujours d'environ 3 s. La verte et la rouge sont donc le même scénario ; seule la lecture du test tombe avant ou après le leave d'Alice. Le commit 1ace000 (« intermittent red : the lobby has 1 player(s) ») a vraisemblablement traité le même incident avec un mauvais diagnostic (« lu avant que le second revienne ») : c'était déjà « lu après qu'Alice a quitté ».

## Contre (a) le jeu

- Les deux `server.log` (M20261119-162601-1 et -162952-1) : aucune ligne `Match flow ... refused`, `ABSENT` ni `no script match flow board` (grep `refused|ABSENT|no script match flow board|Match flow`). Les seuls `Error:` sont du bruit de démarrage Angelscript / police (`F_HOL`, `float32 float`), présent dans les deux.
- `POST .../finished` à 17:27:26 : la fin de partie a été décidée ; Bob revient au salon 2 s après (verte : 2 s aussi).
- Le serveur sort sur `Server quits: empty after the match` (162952) et le gestionnaire reçoit `left`, `left`, `empty`.
- Les durées finished -> retour de Bob et leave d'Alice sont identiques à la verte.

## Pour (c) la charge, facteur secondaire

start -> listening : 29,4 s (rouge) contre 13,3 s (verte), soit x2,2 ; Bob rejoint 31 s après la création contre 23 s. La charge élargit la gigue de la boucle du test (Start-Sleep 1 s + Test-Log x2 + Start-Sleep 2 s), qui n'a que 3 s de marge. Elle explique pourquoi la rouge est tombée ce coup-ci, pas la cause.

## « 2 server(s) of this copy running »

Non élucidé par les journaux conservés. Le test exige seulement `>= 1` (`$ServersAfterStay -lt 1` échoue seulement à 0), donc 2 n'est pas bloquant. Pas comparé avec la verte (le compte n'est pas dans le journal du gestionnaire).

## Correctif proposé (non écrit)

1. `Tools\Tests\lobby_match.ps1:157` : ne plus quitter 3 s après le retour. Pour Alice, remplacer `"wait 3", "quit"` par `"waitfile $LobbyRead 120", "quit"` (nouveau fichier signal), et pour Bob remplacer `"wait 10"` par la même attente. Le test écrit `$LobbyRead` juste après la boucle de lecture (après le `break`, vers la ligne 637). Plus de course, plus besoin du poll de 20 s.
2. `Tools\Tests\lobby_match.ps1:630-636` : entourer le `Invoke-RestMethod` d'un try/catch qui distingue 404 (« L1 supprimé : Alice a quitté avant la lecture ») d'une erreur réseau, pour que l'échec dise la vraie cause.
3. Ne pas toucher au gestionnaire : la suppression du salon vide est le comportement attendu.

## Non vérifié

- Les `lobby_match_alice.log` et `_bob.log` sauvegardés sont ceux de la partie suivante (relance 17:29, 52 et 34 étapes) : les logs de la passe rouge sont écrasés. Je n'ai donc ni l'heure exacte de « Menu: back in lobby » ni celle du premier GET de la boucle du test. La preuve est indirecte : alignement des leave (43,82 et 50,80) avec le 404 (50,89) et la durée du test.
- Qui émet le 404 (test ou client) : deux cadences de GET d'environ 0,5 s coexistent sur L1 ; celle du 404 (50,889, après le leave de Bob) n'est pas celle de Bob, d'où mon attribution au test.
- Pourquoi le gestionnaire redémarre à 17:28:55 et ce qu'est la partie 17:29-17:30 (relance de `lobby_match` ou autre partie de `quick_check`) : non élucidé ; le journal copié `Saved_Tests__LobbyMatch__manager.log` couvre cette seconde instance et ne contient pas de 404.
- Corps des réponses GET L1 (non journalisé) : impossible de voir qui était nommé à chaque lecture.
- Code des portes de fin de partie (`HOLMatchFlowBoard.as`) non relu : hors sujet vu les journaux.
