# GALADRIEL-1119cf : outil de preuve du lot L4 (grille)

NON LANCE, aucun C++, aucun `.as`, aucun build. Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`, branche `grid-l3a`, commit 645d623 (sur fae3c2c ; anglais, sans Co-Authored-By, pas de push, pas de merge). Fichier touche : `Tools\Scenarios\tier_grid_client_run.ps1` seul.

## Commandes
- Passe verte : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\tier_grid_client_run.ps1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel -Expect enabled`
- Passe rouge (apres pose du bouchon `return FHOLGridCommandView();` en tete de `GetCommandAnswer`, `Script\Player\HOLCommandGridBoard.as:13`, par la session principale) : meme commande avec `-Expect empty`. Puis retrait du bouchon et nouvelle passe verte.
- Sortie : `tier_grid_client : PASS|FAIL`, puis une ligne `CHECK <nom> : ok|FAIL <pourquoi>` par controle ; code retour 0 / 1. Journaux dans `%TEMP%\tier_grid_client\` (`tier_grid_server.log`, `_bob.log`, `_alice.log`).

## Controles ajoutes (un seul script : le journal du serveur y est deja connu, `$sl`)
Pour CHACUN des 3 journaux (serveur dedie, Bob, Alice), fonction `Test-GridJournal` :
- NEGATIF : aucune ligne `CommandGrid: .* refused, no script grid board` (texte exact de `HOLCommandGrid.cpp:51`, `LogHOLCommandGridBridge`, Error). Une ligne = `FAIL grid-refused-<server|bob|alice>` avec la ligne citee.
- POSITIF : au moins une ligne `Board: CommandGrid script <classe>` (`Match\HOLBoardLog.cpp:51`, `Report<UHOLCommandGridBoard>(World, "CommandGrid")` ligne 79 ; appelee par `UHOLSimulationSubsystem::OnWorldBeginPlay` ligne 214 AVANT le retour anticipe des clients : serveur et client). Une ligne `Board: CommandGrid ABSENT (C++ fallback)` = `FAIL board-line-<qui>` cite ; ni l'une ni l'autre = `FAIL board-line-<qui>` (plateau non prouve present).
- Journal absent = `FAIL log-missing-<qui>`. Lecture avec partage (FileShare.ReadWrite), 2 s de pause apres l'arret des processus.
La planche de la grille est couverte par la ligne de presence existante (`CommandGrid`) : aucun journal a ajouter.

## Mode rouge `-Expect enabled|greyed|empty`
`empty` : PASS si Alice a bien un `HUD report ... selection MainCentre` ET aucune cellule `<touche>=Peasant ...` (le rapport HUD saute une cellule sans type, `HOLGameScreen.cpp:1690-1697` : une grille entierement vide s'ecrit `grid empty`) ET les 3 controles de journal restent verts (le plateau EST la, donc le vide vient de la reponse du bouchon, pas d'un plateau absent). `greyed` garde son sens d'origine (UnitTier). Le controle `UnitTier: no script board` reste applique en `enabled` et `empty`, pas en `greyed` (comme avant).

## Verifie sans lancer
- Parser PowerShell : 0 erreur.
- Les 3 fonctions (`Read-LogLines`, `Format-Cited`, `Test-GridJournal`) extraites par l'AST et executees sur 5 faux journaux (ok / ligne refused / ABSENT / sans ligne / absent) : 0, 1, 1, 1, 1 echec, textes et citations corrects.
- Relecture ligne a ligne : pas de `$e`/`$E`, `$w`/`$W`, `$Name` en collision ; variables de boucle `$ln`, `$item`, `$pair`, `$ck`, `$p` distinctes des parametres ; `$script:BoardLines` initialise avant l'appel ; resultats de fonction forces en tableau (`@(...)`, `.ToArray()`).

## NON VERIFIE
- Le corps principal du script (lancement des 3 editeurs, parse de la ligne `HUD report`, construction des CHECK) n'a pas tourne : seules les fonctions ont ete executees sur des faux journaux.
- Que les journaux des CLIENTS portent bien `Board: CommandGrid script` : lu dans le code (`LogWorldBoards` avant le retour anticipe client), jamais vu dans un journal client. Si Bob ou Alice rend `board-line-... no 'Board: CommandGrid script' line` alors que le jeu marche, c'est le controle positif qui est faux (ex. le monde de menu rapporte, pas le monde de partie), pas la grille : regarder `Board:` dans le journal cite.
- Que l'absence de cellule Peasant sous bouchon se traduit bien par `$cell -eq ''` (le HUD peut aussi ne jamais refresh) : sans bouchon pose, aucune preuve.

## Anomalies
- Un monde de MENU (client avant connexion) qui est un monde de jeu avec game instance rapporterait aussi ; si son plateau etait absent il ferait echouer le controle ABSENT (voulu : "aucun monde de jeu sans plateau"), a lire dans la citation.
- L'avertissement git LF/CRLF est normal (`eol=crlf`).

APPELS_VUS: 14 ; HEURE_DEBUT/HEURE_FIN: 17:33 / 17:37 ; BUILDS_DEMANDES: 0 ; PROCESSUS_DETACHES: aucun ; IN_USE.txt libre (sans accents).
