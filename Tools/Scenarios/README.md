# Scénarios de test

Un scénario ouvre le jeu directement sur une situation toute prête (soldats, ennemis, bâtiments, ressources), au lieu
de la monter à la main avant chaque essai. Il lance une partie (comme l'écran Nouvelle partie), attend que la caméra du
joueur soit au-dessus de son départ, joue ses étapes, puis le panneau du scénario (en haut au centre, sous la barre des
ressources) passe de « Scénario <nom> en préparation… » à « Scénario <nom> prêt ». La partie reste ouverte : on joue.

**Règle : chaque essai demandé à l'utilisateur vient avec son scénario.**

## Lancer

Depuis PowerShell (pas Git Bash, qui réécrit les chemins `/Game/...`) :

```
powershell -ExecutionPolicy Bypass -File Tools\Scenarios\play.ps1 order_lines
powershell -ExecutionPolicy Bypass -File Tools\Scenarios\play.ps1              # la liste, avec leur description
powershell -ExecutionPolicy Bypass -File Tools\Scenarios\play.ps1 order_lines -Root ..\MyCrusader-A
```

Le jeu s'ouvre avec les réglages habituels de l'utilisateur (écran, fenêtre). Dans la console du jeu (²) :
`hol.Scenario` liste les scénarios, `hol.Scenario <nom>` en lance un, depuis le menu ou en pleine partie (la carte est
rechargée, rien ne reste de la partie d'avant).

## Format

- `<nom>.txt` : une étape par ligne ; lignes vides et lignes `#` ignorées ; la première ligne `#` sert de description.
  Les étapes sont celles de `hol.Test.Run` (Source/HeroesOfLiberty/Dev/HOLTestSequence.cpp) : commandes de console
  (`hol.Units.SpawnHere`, `hol.Build.PlaceNear`, `hol.Camera.FocusNear`, `hol.Cheat.Resources`…) et verbes d'attente
  (`frames`, `wait`, `waitlog`, `waitunits`, `waitui`…).
- `note <texte>` : ajoute le texte, une ligne par note, au panneau du scénario, en haut au centre de l'écran (et l'écrit
  au journal) ; le panneau reste jusqu'à ce qu'on le ferme (« × ») ou qu'un autre scénario le remplace. À la fin du
  scénario, deux ou trois notes disent quoi essayer.
- `<nom>.json` (facultatif) : la configuration de la partie, au format de `-MatchConfig=` :
  `{ "map": "/Game/Maps/Plain.Plain", "slots": [ { "state": "Human", "colorIndex": 0 }, { "state": "AI", "colorIndex": 1 } ] }`.
  Sans lui : la configuration par défaut (un joueur humain, Plain).
- Une étape qui n'arrive pas en 60 s met l'erreur en rouge dans le panneau et arrête le scénario ; le jeu reste ouvert.
