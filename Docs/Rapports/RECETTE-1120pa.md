# RECETTE-1120pa (2026-11-20, 16:44-16:52) : worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, integ-30 = main 9dc5e75

| Essai | Pré-vol | Fiche |
|---|---|---|
| deploy | FAIL (14) | non |
| chat_input | FAIL (5) | non |
| neutrals_clairiere | PASS | OUI : Retours\E-neutrals_clairiere.md (GESTE: interne, SON: non entendu) |
| virtuous_knight | FAIL (6) | non |

## deploy : FAIL, cause JEU
- `waitlog D=Pack` expire (60 s, étape 38). Journal : `Deploy: #13 phase 1 -> 2 (delay over)` a bien lieu 1,46 s après l'ordre, mais le HUD reste sur `D=Mounting0` et la case D dit `Montage… [--------]` encore 1,1 s après la fin du montage (activité Attacking). La case ne passe jamais à « Démonter ». Source de l'étiquette : Source\HeroesOfLibertyUI\HUD\HOLGameScreen.cpp:1029-1044 (ReadDeployFace, lit AHOLPlayerController::GetDeployProgress : phase restée « Mounting », permille 0).
- Deuxième défaut : `deploy_deploying.png` (0,3 s après D) montre `Montage… [--------]` : 0 dièse, alors que le .look exige au moins 1 (20 à 30 %). La barre ne progresse pas.
- PNG vus : deploy_packed et deploy_deploying (frais) ; les 10 autres captures non prises (étape bloquée), les anciennes (12:54) sont périmées.
- Conséquence : D869/D871 ne sont pas visibles à l'écran (barre figée, jamais Démonter).

## chat_input : FAIL, causes JEU et SCÉNARIO
- Scénario : chat_input.json sans mapSize, joue sur la carte de test grise (FAIL pré-vol).
- Jeu : le message affiché est en mojibake « (AlliÃ©s) salut les allies » (fil des messages, chat_input_shown.png ; journal `Chat message: Vous : (AlliÃ©s) ...`). Encodage UTF-8 mal décodé dans la marque de canal.
- Jeu : après l'envoi, la 2e Entrée (focus: none) ne rouvre pas le champ : `HUD chat: closed (last sent 'salut les allies')`, donc `waitlog chat: open` expire (étape 39) ; le « /w Joueur 2 salut » et le clic DipName_1 ne sont jamais joués ; chat_input_target non pris.
- PNG vus : chat_input_typed (champ blanc en bas à gauche, texte « salut les allies » gris très pâle sur blanc, peu lisible, pas de marque « (Allies) » visible comme le .look l'annonce ; panneau Diplomatie ouvert au milieu) ; chat_input_shown (message dans le fil, avec mojibake). La saisie réelle hol.UI.Type fonctionne.

## neutrals_clairiere : PASS
- 3 PNG ouverts et vus : avant (bosquet cache les Lanciers), large (bosquet disparu, Lanciers à découvert, un buisson isolé en bas à droite à la limite des 25 m), camp (gros plan sans arbre). Point du .look vu. Écarts mineurs : Hommes d'armes bleus en bas et non à gauche ; WARN pas de `HUD overlap`.
- GESTE: interne (commandes hol.*, aucune touche ni clic). Fiche écrite.

## virtuous_knight : FAIL, cause SCÉNARIO (+ à vérifier côté jeu)
- Pré-vol : `hol.Select.ClickUnit 0` vient 122 frames après le placement caméra (max 10) ; la sélection reste vide, `waitlog Selection: VirtuousKnight` expire (étape 37). before et hp non pris, l'anneau de 8 m et la baisse de PV du Chevalier non vus.
- PNG vu : virtuous_knight_grid : la case E dit « Chevalier Vertueux », mais A = Hallebardier, Z = Homme d'armes (le .look dit A Lancier, Z Hallebardier) et A/Z/E sont affichées grisées/peu contrastées. Le point E est donc vu, pas le reste.
- Le scénario saute aussi le geste réel de la touche E (spawn direct) et le waitkey est une attente d'utilisateur.

## Processus
stray_processes.ps1 -Stop lancé avant chaque pré-vol et à la fin : « aucun processus détaché ». Aucun jeu ni scénario modifié. Une seule fiche créée.
