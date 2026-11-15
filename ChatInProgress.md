# Chantier : le transport du chat (A4 ; GDD § 15, D385, D376, D357, D792)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel`, branche `net-playtest`. Règle pure déjà dans main : `Script\Chat\HOLChatRules.as` (59e8f60).

## Plan

- **C1** (GLORFINDEL-1115e) transport serveur : `AHOLPlayerController::ServerPostChat(Channel, Text, TargetSlot)` (porte : `HOLChat::CheckPost`, longueur max et cadence par joueur, cvars `hol.Chat.MaxLength` / `hol.Chat.MaxPerWindow` / `hol.Chat.WindowMs`, registre `comm.chat.*`), destinataires par `HOLChat::Reaches` (miroir C++ de `HOLChatRules.as`, `Source\HeroesOfLiberty\Match\HOLChatRules.{h,cpp}` ; « même camp » = alliance mutuelle, relation Ally) ; `ClientReceiveChat` -> `AHOLGameState::NoteChatReceived` -> `HOLForumMessages::GetChatMessage` (nom par `GetPlayerName`, marque du canal) -> `OnSignalMessage` (le fil du HUD). Commande de dév `hol.Chat.Say <all|allies|private> [<slot>] <texte>`, `hol.Chat.Report`.
- **C2** saisie (Entrée, champ, bascule) et apparence : question `Retours\Q-chat-ui.md` (défaut codé : marque « (Alliés) » / « (Privé) »).
- **C3** discussion de salon (D505 : viendra avec D385), annonces de diplomatie dans le fil.

## Tests

- C++ `HeroesOfLiberty.Chat.Route` (destinataires calculés par HOLChat, cas netsig3 + D376 + D792), `HeroesOfLiberty.Chat.Gate` (longueur, vide, privé à soi, canal inconnu, cadence), `HeroesOfLiberty.Chat.ThreadLine` (ligne du fil).
- Harnais net, partie `netsig3` (1 serveur + 3 clients) : Bob envoie `hol.Chat.Say allies Hello allies` en phase 1 (Alice alliée mutuelle, Carol neutre) ; vérifie serveur « sent to 2 seats », Bob et Alice lisent, Carol non, ligne du fil d'Alice « Chat message: Bob : (Alliés) Hello allies ».

## Suivi

- 2026-11-15 13:18 GLORFINDEL-1115e : merge main (avance rapide a082990), binaire périmé -> build PASS 108 s. Code C1 écrit ; rouge vu avec un HOLChatRules.cpp bouchon (Reaches = l'émetteur seul, CheckPost = toujours Ok), puis la vraie règle.

## Appels (un trait par appel)

|||||||||||||||||||||||||

- 2026-11-15 13:48 GLORFINDEL-1115e : C1 VERT. Rouge VU a 13:31 (bouchon) : tests 1313/1315 (Chat.Gate, Chat.Route KO), netsig3 KO (serveur sent to 1 seat, Alice 0, pas de ligne du fil). Piege : la copie du vrai HOLChatRules.cpp gardait l'heure ancienne -> UBT ne recompile pas (build 4 s, rouge identique) ; touche du fichier puis build. Vert : quick_check -Parts tests,net PASS. Suivant : C2 (saisie Entree + champ) apres Q-chat-ui ; C3 discussion de salon (D505).
