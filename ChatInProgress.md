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
- 2026-11-15 14:26 GLORFINDEL-1115f : C1b VERT (chat sur le vrai reseau, partie netsig3 de `Tools\Tests\quick_check.ps1`, aucun C++ ecrit ; build 137 s seulement pour le C++ d'autres chantiers apres merge main 6a77f35). Couvert : prive Bob -> Alice (Alice + echo Bob, Carol non ; l'emetteur recoit toujours le sien, Reaches C++ = .as), 201 caracteres refuse (serveur `answer 2`), 6e envoi dans la fenetre refuse (`answer 3`), personne ne lit les refuses ; Carol vaincue (`hol.Victory.Defeat 2`), alliee d'Alice (alliance gardee apres la defaite, vu au Diplomacy report) : Allies d'Alice ne lui arrive pas (sent to 1), All d'Alice oui (3), son All a elle seule (sent to 1). Constat : vaincue, Carol envoie comme `slot -1` (son controleur devient spectateur : chemin « sans siege » de ServerPostChat). Rouge VU 14:18 (controles seuls : net FAIL 352 s, netsig3 43 s, 12 KO), vert 14:25 (net PASS 362 s, netsig3 49 s : +10 s). Question : `Retours\Q-chat-rules.md` (spectateur sans siege). Besoin C++ note : l'emetteur n'est pas prevenu d'un refus (pas de RPC de reponse) : avec C2. Suivant : C2 apres Q-chat-ui.
- 2026-11-15 (réponse de l'utilisateur, session principale, docs seulement) : D806 (Q-chat-rules) = A : spectateur sans siège = comme un vaincu (Tous et Alliés avec les spectateurs, jamais en privé, lit le Tous des vivants) ; reste : aligner `HOLChatRules.as` + test (ligne [ ] Glorfindel, sans C++).
- 2026-11-15 (réponse de l'utilisateur, session principale, docs seulement) : D807 (Q-chat-ui) = A : Entrée = champ sur Alliés, Tab bascule Alliés -> Tous -> privé, marque « (Alliés) » / « (Privé) » dans la ligne ; la saisie reste à coder (ligne [ ] Glorfindel, C++).
