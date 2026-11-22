# [À ESSAYER] E-chat_input : Entrée ouvre le champ de chat sur Alliés, tu tapes un message aux alliés puis un privé ; après un clic sur la ligne du joueur 2 dans Diplomatie, Entrée puis Tab Tab met le champ sur le privé avec le nom « Joueur 2 »

STATUT : VU À L'ÉCRAN
GESTE: réel      (vraie chaîne Slate : `hol.UI.Key` envoie des événements clavier Slate Entrée / Tab, `hol.UI.Type ChatInputBox` tape dans le champ du jeu, `hol.Mouse.Click DipName_1` clique la ligne du joueur 2 ; pas d'appel interne `hol.Chat.*` ; pas de frappe OS physique, comme `neutrals_qground`)
SON: sans objet
branches: ability-columns (= main 6b226f8 ; contient f804471 UHOLScriptScreen::NativeOnPreviewKeyDown, relais de Tab vers le champ)
recette: RECETTE-1121sa PASS sur main 9e7a324 (Sauron) : preflight PASS (31 s), 4 PNG ouverts et vus ; Tab Tab met le champ sur le prive. Marque du prive = « Joueur 2 : » (nom du destinataire), conforme au design : D807 (GDD.md:3572) ne fixe « (Prive) » que pour la LIGNE RECUE, le code ChannelMark prend le nom du joueur (ChatInProgress.md:37) ; D833 (alerte population) et D871 (Mangonneau) ne concernent pas le chat. Le .look (« (Prive) » dans le champ) est a corriger, pas le jeu.
recette: RECETTE-1122b PASS VU A L ECRAN sur integ-50 7053828 : preflight PASS (34 s), 0 FAIL, 0 WARN, 0 erreur Angelscript, 0 WorldContext ; 4 PNG ouverts : champ (Allies) vide, ligne recue (Allies) salut les allies, champ Joueur 2 : apres Tab Tab (nom du destinataire, conforme D807, le .look dit encore (Prive) : a corriger), texte tape salut les allies dans la boite. Rapport Docs\Rapports\RECETTE-1122b.md

Règle (L7-6) : un essai de GESTE ou de SON reste LOGIQUE SEULEMENT tant que la ligne dit `interne` ou `non entendu`.

Pré-vol (2026-11-21 19:14, agent GLORFINDEL-1121sa, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, main 6b226f8) : `preflight.ps1 chat_input -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel` PASS (86 s, 4 PNG). Seul WARN : `frames 90` (ligne 14 du scénario, avant l'ouverture de Diplomatie, préexistant).
Journal (`Saved_Tests\Logs\scenario_chat_input.log`, 18.14.01 UTC-1) : `ChatInput: Tab (open true) ally true chosen 1 -> channel 0` puis `ChatInput: Tab (open true) ally true chosen 1 -> channel 2`, soit « ChatInput: Tab » 2 fois (avant f804471 : 0 fois, RECETTE-1120sb) ; messages `Chat message: Vous : (Alliés) salut les allies` et `Vous : (Privé) salut`.
PNG OUVERTS et vus :
- chat_input_typing : champ ouvert en bas à gauche, marque « (Alliés) : » + boîte vide avec curseur, Diplomatie ouvert à droite avec la ligne du joueur 2.
- chat_input_typed : « (Alliés) : » + « salut les allies » DANS la boîte, avant Entrée.
- chat_input_shown : champ fermé, fil en haut à droite : « Vous : (Alliés) salut les allies ».
- chat_input_target : après clic sur la ligne du joueur 2, Entrée, Tab Tab : le champ montre « Joueur 2 : » + boîte vide (la marque du privé est le NOM du destinataire, pas le mot « (Privé) ») ; le fil montre « (Alliés) salut les allies » et « (Privé) salut » ; Diplomatie reste ouvert.
NON VÉRIFIÉ : une vraie touche Tab physique du clavier (le relais passe par un événement Slate synthétique `hol.UI.Key`) ; le texte exact du `.look` annonce « (Privé) » alors que le champ affiche « Joueur 2 : » (à confirmer comme voulu) ; le son (sans objet).

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle.
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 chat_input -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
Durée : environ 2 min (86 s). Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
