# SignalsInProgress.md : les signaux sur la carte (GDD § 15, D386 ; avec D369, D370, D375, D376, D357, D367)

Ouvert le 2026-11-12 (séance autonome, HALDIR-1112e, worktree Haldir, branche `maps`, chapitre A1 du relevé du jour). Méthode des
dossiers par feature : règles tirées du GDD, plan en étapes de 15 min, test rouge d'abord, au plus 2 questions
(`C:\Users\baudo\Documents\MyCrusader\Retours\Q-signals.md`). Règle D483 : règles pures en Angelscript ; le transport (RPC, serveur) en C++.

## Règles tirées du GDD (rien à trancher)

1. **Roue de signaux typés** (D386, § 15) : Alt maintenu + clic, **sur le monde ou la mini-carte**. Cinq types : **Attaque ici**,
   **Défends ici**, **Besoin d'aide**, **Regroupement**, **Éclaire ici**. Un **simple Alt + clic** pose un **signal neutre** (6e type).
2. Chacun a **son icône, son son et une ligne dans le fil** (traduite d'office, fr / en / ja ; fil de messages D445).
3. **Vus des alliés seulement** ; en *Diplomatie*, **des alliés mutuels** (D369 : alliés seulement si les deux se disent Allié ; la
   relation est la plus hostile des deux postures, D375 ; une rupture est immédiate, D370, donc les signaux disparaissent tout de suite).
   Ni un Neutre ni un Ennemi ne les voient. En mode à équipes : les coéquipiers (IA comprise).
4. **Nombre limité contre le spam** : indicatif **3 toutes les 5 s** (par joueur).
5. **Une IA alliée pourra plus tard y réagir** (pas dans ce chantier : seulement les recevoir).
6. Un éliminé en chacun pour soi et en *Diplomatie* ne parle qu'aux spectateurs (D376) : **aucun signal** vers ses anciens alliés.

Lectures faites sans décision (pas des questions) : l'émetteur voit son propre signal (comme AoE4) ; la limite compte par joueur et un
signal refusé ne compte pas ; fenêtre glissante (le 4e est refusé tant que le 1er a moins de 5 s).

Pas dans le GDD (questions, défauts codés) : la **durée de vie** d'un signal sur la carte (Q1) ; le **coéquipier éliminé** en mode à
équipes (Q2 ; D357 lui laisse la discussion avec ses coéquipiers).

## Ce qui existe (relevé du 2026-11-12)

- **Aucun code de signal** : Grep `Signal|Ping|Beacon|Marker` dans `Script/` et `Source/` ne trouve que `MassSignals` (moteur), le
  `PingSeconds` du cercle de clic (`Source/HeroesOfLiberty/Player/HOLSelectionMarkers.cpp:67`, retour de commande, pas un signal),
  le son `1kSineTonePing` du forum (`HOLForumMessages.h:256`) et `AHOLSelectionMarkers` (anneaux de sélection).
- **Alliés mutuels** : `HOLDiplomacy::IsAlly(Book, A, B)` de `Script/Diplomacy/HOLDiplomacyRules.as` (relation = la plus hostile, D375 ;
  IA = Ennemie en *Diplomatie*, D367 ; `NewTeamBook` pour les modes à équipes : coéquipiers alliés, IA comprise). **Réutilisée, pas
  recopiée** par `HOLSignal::Sees`.
- À réutiliser plus tard : le fil de messages (D445) pour la ligne du signal ; le cercle de la mini-carte (D669, `hol.Minimap.Alert*`)
  comme modèle d'icône animée ; `AHOLSelectionMarkers` pour un anneau au sol.

## Plan (étapes de 15 min)

- **SG0 relevé** : FAIT (ce fichier, 2026-11-12).
- **SG1 règles pures en script** : `Script/Signals/HOLSignalRules.as` (types, `Sees`, `CanSend`, `Post` avec la limite, `Expire`,
  `SeenBy` avec la durée de vie) et `Script/Tests/HOLSignalRules_Test.as` (6 tests, rouge d'abord 76f66b8). Valeurs au registre
  `Tools/Data/ValueSheet.csv` (`comm.signal.*`).
- **SG2 transport réseau (C++, agent C++)** : RPC serveur `ServerPostSignal(Kind, Location)` sur le contrôleur ; le serveur tient le
  `FHOLSignalBoard` (appelle `HOLSignal::Post` : la limite est appliquée au serveur, jamais au client seul) et envoie le signal aux
  seuls joueurs pour qui `Sees` est vrai (RPC client par contrôleur, pas de réplication à tous : un ennemi ne doit pas le recevoir) ;
  horloge = temps de partie du serveur en ms. Test : `net_match` à 3 joueurs (allié reçoit, ennemi ne reçoit pas, 4e refusé).
- **SG3 saisie et affichage** : le GDD dit « roue (Alt maintenu + clic) » et « un simple Alt + clic pose un signal neutre » ; la frontière
  entre les deux (clic maintenu qui ouvre la roue, clic bref = neutre ?) n'est pas écrite : à relire avec l'utilisateur à cette étape (pas
  posé maintenant, 2 questions au plus) ; icône sur la mini-carte (modèle
  D669) et marque au sol pendant `LifetimeMs` ; ligne dans le fil (D445). Icônes provisoires : ligne à ajouter à `Placeholders.txt`.
- **SG4 sons** : un son court par type (règle des sons non usants : graves, sourds, courts ; packs Kenney d'AudioInProgress) ; provisoire
  noté dans `Placeholders.txt`.
- **SG5 vérification** : scénario `signals` (Alt + clic, roue, allié qui voit, ennemi qui ne voit pas, 4e refusé), essai listé seulement
  VU À L'ÉCRAN, geste réel.
- **SG6 IA alliée** (après IA4) : lire `SeenBy` (Attaque ici / Défends ici) ; note seulement.

## Premier test rouge (SG1)

`Script/Tests/HOLSignalRules_Test.as` : 6 tests sur la vraie règle de *Diplomatie* (`HOLDiplomacy::SetStance`, `NewTeamBook`) : l'allié
mutuel voit le signal posé, l'allié d'un seul côté, le Neutre et l'Ennemi non ; la rupture le cache tout de suite ; le 4e signal en 5 s est
refusé et le plateau n'en montre que 3 ; un autre joueur garde son compte ; le signal s'efface après sa durée ; un éliminé de *Diplomatie*
ne signale pas. Commit du rouge 76f66b8 (namespace `HOLSignal` absent).

## Questions (`Retours\Q-signals.md`, défauts codés)

- Q1 : durée de vie d'un signal. Défaut A = 5 s (`LifetimeMs`), égale à la fenêtre de la limite.
- Q2 : le coéquipier éliminé signale-t-il ? Défaut B = oui en mode à équipes (`DefeatedTeammateSignals`), jamais en chacun pour soi ni
  en *Diplomatie* (D376).
- **Réponses de l'utilisateur (2026-11-12, D784)** : Q1 = **A, 5 s** (déjà codé) ; Q2 = **A, NON : un coéquipier éliminé voit les signaux de son équipe mais n'en pose plus** (À CHANGER : `HOLSignal::DefeatedTeammateSignals = false`, test à adapter).

## Suivi

- 2026-11-12 11:33-11:4x (HALDIR-1112e, `maps`, sans C++) : SG0 et test rouge SG1 (76f66b8 ; quick_check `tests` 11:36 : « Namespace
  'HOLSignal' doesn't exist », 0 test lancé). **SG1 VERTE** : quick_check `tests` 1186/1186 PASS (215 s), `values` PASS (3 lignes au
  registre `comm.signal.max_per_window`, `window_ms`, `lifetime_ms` ; Valeurs.md/.csv régénérés). Commit vert 9d30e2c, puis `git merge main` (arbre propre). **Prochaine étape pour un agent neuf : SG2** (agent C++ : RPC serveur, plateau tenu au
  serveur, envoi aux seuls `Sees`), puis SG3. Aucun test rouge en cours. Rien de visible en jeu : pas de ligne à `Placeholders.txt`.

## 2026-11-13 11:10 HALDIR-1113 (agent, maps)
- D784 applique : DefeatedTeammateSignals = false (il voit, ne pose plus) ; rouge dbc9699 (Post -> Silenced, le vivant ne voit rien de lui, le vaincu voit le signal de son equipe ; + Test_SignalStaysFiveSeconds a 4999/5000 ms), vert 8d028ff ; quick_check -Parts tests 1203/1203 PASS (11:10). Toujours aucun appelant en jeu (C++ SC2/SG2/SP2/N2 a faire). Reprise : etape C++ suivante du journal.

## 2026-11-13 12:35 HALDIR-1113b (agent, maps, sans C++)
- PROCHAINE ETAPE C++ (l. 72 de Auto_Journey_File.md), non codee : SG2. Lieu : `Source\HeroesOfLiberty\Player\HOLPlayerController.h` (45 RPC `UFUNCTION(Server, Reliable...)` deja la, meme modele) : `ServerPostSignal(Kind, Location)` ; plateau `FHOLSignalBoard` tenu au serveur (GameState ou sous-systeme) qui applique `HOLSignal::Post` (cadence, Silenced pour un vaincu, D784) ; envoi Client RPC aux seuls joueurs pour qui `HOLSignal::Sees` (equipe, vaincu voit sans poser). Test rouge : net_match (signal pose par un client, recu par l'allie, pas par l'adversaire ; vaincu refuse). Puis SG3 (Alt + clic, icone mini-carte, ligne du fil). Les regles pures AS (SG1) sont vertes et durcies (5 s a 4999/5000 ms).

- 2026-11-13 16:5x (HALDIR-1113c) : nouvelle question [A REPONDRE] posee (L73) : `C:\Users\baudo\Documents\MyCrusader\Retours\Q-signals-spectators.md` (elimine en chacun pour soi / Diplomatie et spectateurs ; defaut B de fait = aucun signal ; recommandation A = tous). A traiter dans SG2 (envoi aux spectateurs).

- 2026-11-13 17:10 REPONSES de l'utilisateur (via la session principale, ecrites en Reponse: dans les Q de Retours\) : Nomade Q1 = A (150 bois + 400 pierre) ; Nomade Q2 = B (bouton de specialisation ETEINT tant qu'aucun forum n'est acheve : A CHANGER, test rouge dans HOLSpecializationRules puis correction) ; Signaux = A (elimine chacun pour soi / Diplomatie et spectateur pur voient TOUS les signaux : A CHANGER, test rouge sur HOLSignal::Sees puis correction) ; Specialisation clic sur l'autre option = B (remplace directement, delai entier : A CHANGER, une ligne dans HOLSpecialization::Start, test rouge d'abord) ; Foret Q1/Q2 = a trancher (defauts inchanges, rien de plus a coder). NON CODE par HALDIR-1113c (plafond de 45 appels atteint) : prochain agent sans C++.

## 2026-11-13 18:08-18:20 HALDIR-1113d (agent, maps, sans C++)
- Reponse Q-signals-spectators = A APPLIQUEE : HOLSignal::SpectatorsSeeAll = true ; Sees/SeenBy prennent bViewerSpectator (entree de l'appelant comme HOLChat : spectateur pur, Viewer -1 permis, ou elimine en chacun pour soi / Diplomatie ; le coequipier elimine d'un match en equipe n'en est pas un, D784 inchange). Rouge 79d22a5 (Test_SignalSpectatorsSeeEverySignal, vu ROUGE 1224/1227), vert ee0d8cf ; quick_check -Parts tests 1227/1227 PASS (18:18). SG2 : le serveur envoie aussi aux spectateurs (bViewerSpectator = true).

## 2026-11-13 19:46-20:0x HALDIR-1113e (agent, maps, C++)
- SG2 FAIT (serveur) : UHOLSignalSubsystem (Source\HeroesOfLiberty\Match\HOLSignalSubsystem.h/.cpp, pont C++ abstrait, ReadBook lit les postures repliquees, IsSignalSpectator = sans siege ou elimine sans coequipier) + sous-classe script Script\Signals\HOLSignalBoardScript.as (HOLSignal::Post / Sees) ; AHOLPlayerController::RequestSignal -> ServerPostSignal (slot du joueur, temps du monde en ms, vaincu lu de AHOLGameState) -> ClientReceiveSignal aux seuls sieges qui voient (spectateurs compris, reponse A). Test C++ HeroesOfLiberty.Signals.ServerBoard : rouge 98d747f (vu ROUGE 1234/1235 a l'execution, pas de tableau script), vert e65b3b7 (1235/1235), merge main 5f2453f.
- NON VERIFIE : le chemin RPC (ServerPostSignal / ClientReceiveSignal) n'a aucun test reseau (aucun net_match a deux clients) ; rien de visible en jeu (SG3 a faire : Alt + clic, icone mini-carte, ligne du fil). Reprise : SG3 ; SP2 non commence (Specialization).

## 2026-11-13 22:45-23:1x Glorfindel (agent, net-playtest, C++ test seul)
- Test C++ `HeroesOfLiberty.Signals.DefeatedSpectator` (Source\HeroesOfLiberty\Tests\HOLSignalSpectatorTests.cpp) : vrai AHOLGameState (InitVictory, equipes 1,1,2,3, DefeatSlot), ReadBook (4 sieges, verrouille hors Diplomatie, 0-1 allies, 0-2 et 3-2 non) puis IsSignalSpectator + SeesNow sur le tableau script : elimine 1 avec allie vivant = PAS spectateur (voit le signal de l'allie, pas celui de l'ennemi, D784) ; elimine 3 sans allie = spectateur (voit l'ennemi, reponse A) ; sans siege (-1) voit tout ; vivant 0 ne voit pas l'ennemi 2. Rouge 924fb96 vu ROUGE a l'execution (1252/1253) avec une mutation temporaire non commitee de IsSignalSpectator (boucle d'allie coupee : les 2 assertions D784 rouges, les autres vertes), regle restauree (fichier identique a main), build PASS 22:54, `quick_check -Parts tests,net` PASS 23:00 (tests 1253/1253, net signals PASS, toutes les parties net PASS du premier coup). Aucun vrai bug trouve.
- 3e client reseau : NON FAIT. Le harnais `quick_check -Parts net` est code pour 2 clients (`foreach ($Client in 2, 1)`, `waitplayers 2`, config de partie a 2 sieges, journaux par client 1/2) : un 3e client = refonte de la partie net, pas une etape de 15 min.

## 2026-11-15 12:24-12:5x HALDIR-1115d (agent, maps, C++) : SG3 lot 1
- FAIT : `Source\HeroesOfLiberty\Player\HOLSignalInput.h/.cpp` (pur : `KindOfAltClick` = clic bref neutre, glisser > `hol.Signal.WheelPixels` 16 px = type par direction, sens horaire depuis le haut : Attaque, Defends, Aide, Regroupement, Eclaire ; `Expire` = `hol.Signal.IconSeconds` 5 s, D784 ; `ColorOf` = `hol.Signal.IconStyle` 0 couleur par type / 1 blanc). Controleur : Alt maintenu + clic gauche (HandleSelectStarted, apres les modes armes) -> a la relache, sol sous l'appui + type -> `RequestSignal` ; `ClientReceiveSignal` garde le signal (`ReceivedSignals`), `GetActiveSignals()` (UFUNCTION) ; mini-carte : `HOLGameScreenScript.as` ajoute un carre de 10 px colore par signal vivant aux marques.
- Test `HeroesOfLiberty.Signals.AltClick` (`Source\HeroesOfLiberty\Tests\HOLSignalInputTests.cpp`) : VU ROUGE a l'execution 12:40 (quick_check tests 1303/1304, stubs) ; le commit du rouge a ete REFUSE par le classifieur (enonce : commits verts seulement) : pas de SHA rouge, commit vert unique.
- Questions : `C:\Users\baudo\Documents\MyCrusader\Retours\Q-signals-ui.md` (Q1 geste de la roue, defaut A glisser ; Q2 icone, defaut A couleur par type).
- RESTE (agent neuf) : marque au SOL dans le monde (non faite) ; ligne du fil de messages D445 (non faite) ; dessin de la roue pendant le glisser (selon Q1) ; Alt + clic SUR LA MINI-CARTE (non fait : seul le monde) ; scenario `signals_altclick` a geste reel (hol.Mouse.Click avec Alt) + preflight ; ligne `Placeholders.txt` pour les icones provisoires (interdit a cet agent : a faire par la session principale) ; SG4 sons.
- NON VERIFIE : l'affichage (aucune image prise), la deprojection du clic (pas de vue au banc), le chemin Alt reel (IsInputKeyDown).

## 2026-11-15 13:04 HALDIR-1115e (agent, maps, C++) : SG3 lot 2

- FAIT : ligne du fil de messages d'un signal recu (HOLForumMessages::GetSignalMessage, table unique D445 : '{nom} : Attaque ici / Defends ici / Besoin d'aide / Regroupement / Eclaire ici', neutre '{nom} : Signal' ; ClientReceiveSignal -> AHOLGameState::NoteSignalReceived -> OnSignalMessage -> HUD HandleSignalMessage, viewer seul). Test HeroesOfLiberty.Signals.ThreadLine vu ROUGE (stub, 1304/1305) puis VERT ; tests 1305/1305, net PASS ; commit fc0798c.
- Placeholders.txt : ligne 'Icones de signaux' ajoutee (copie principale, non commitee).
- RESTE (agent neuf) : scenario signals_altclick (geste reel hol.Mouse.Click avec Alt a verifier, .look : carre 10 px couleur du type, journal 'Signal: received kind' + 'Signal message: Vous : Signal (slot 0)') + preflight + E-signals_altclick.md ; marque au SOL (provisoire) ; dessin de la roue et Alt + clic mini-carte apres Q-signals-ui ; SG4 sons. Spectateur : pas de ligne (HUD sans player state ne verifie pas INDEX_NONE ? : NON VERIFIE).

## 2026-11-15 12:17- GLORFINDEL-1115d (agent, net-playtest, sans C++ propre ; build du merge main seulement)
- Lecture (1) : le harnais net CABLE 2 clients en dur (grand match `foreach ($Client in 2, 1)`, `waitplayers 2`, `hol.Net.WhenClients 2`, 2 sieges Human, victoire Bob perd / Alice gagne, drapeaux done1/done2 ; idem netfog/netview/netspec `foreach ($Client in 1, 2)`). Ajouter Carol au grand match casserait victoire, Units.Where, sources (2 departs). Choix : sous-match leger `netsig3` (modele netspec), joue avec `-Parts net` (sauf `hero` seul) ou seul (`-Parts netsig3`).
- netsig3 : serveur Diplomatie, Alice 0 / Bob 1 / Carol 2 ; le serveur force les postures (hol.Diplomacy.Force) : Bob-Alice allies mutuels, Bob-Carol Neutre -> Bob poste kind 1 ; puis Bob-Carol Ennemi -> Bob poste kind 2 ; puis Carol poste kind 3 (temoin positif : sa reception marche). Rapport `net signals 3 clients` : Bob et Alice recoivent 1/1/0, Carol 0/0/1, reponses serveur `1:1:0 1:2:0 2:3:0`. Duree : ligne `netsig3 : N s (started at M s of the run)`.
- Test rouge : `$Sig3Launched = @(1, 2)` (le harnais tel qu'il etait, 2 clients) ; vert = `@(1, 2, 3)`.
- 12:24 ROUGE vu (cd8175a, `-Parts netsig3`, 144 s) : Bob et Alice recoivent 1 et 2, serveur `1:1:0 1:2:0`, Carol absente (0/0/0, sequence non faite), Bob/Alice en timeout sur posted3. 12:26 VERT (`$Sig3Launched = @(1, 2, 3)`) : `netsig3` PASS en 45 s ; Carol : seul `received kind 3 from slot 2`.
- 12:32 `quick_check -Parts net` PASS en 358 s (lancement 30 s) : grand match net + parties d'avant jusqu'a 315 s, netsig3 43 s (+~15 % : la partie net ne double pas). Commit vert f213c0c. Copie principale `Tools\Tests\quick_check.ps1` = base 339cebc (verifiee identique a HEAD de main apres avoir remis son BOM, que j'avais ote par erreur : `[IO.File]::WriteAllText` sur un chemin relatif resout dans le repertoire du processus = copie principale), puis reportee a l'identique (87 lignes ajoutees, empreintes egales), NON commitee.
- NON VERIFIE : spectateur dans netsig3 (couvert par le test C++ Signals.DefeatedSpectator, pas sur le reseau a 3) ; Alice-Carol non teste (posture par defaut Enemy).
- Appels : ||||||||||||||||||||||||||||||||||||||

## 2026-11-15 14:20-14:26 HALDIR-1115j (agent, maps, sans C++)
- CAUSE du carre non vu (E-signals_altclick) : AUCUN defaut du jeu. Le carre blanc pres du depart ETAIT le signal (neutre = blanc, defaut A ; marque (131.7, 202.6) px de 240). Le temoin 'wait 6' compte l'horloge FApp, plus rapide que le temps du monde en -RenderOffScreen (5 s de jeu = 7,3 s) : la 2e capture tombait 1 image avant l'expiration.
- FAIT (5a75423) : RefreshMinimap journalise 'Minimap signal mark at (x, y) px' et 'Minimap signals: N (world t s)' ; scenario : waitlog Minimap signals: 1 puis waitlog Minimap signals: 0 avant la 2e capture. preflight PASS 14:26, 2 PNG vus (carre a 1 s, absent apres) -> E [A ESSAYER].
- NON FAIT (arret de seance) : quick_check -Parts tests non lance apres le changement de script (log seul) ; aucun test unitaire ajoute (l'effet est asserti par les waitlog du scenario) ; .look non mis a jour (2e ligne dit encore '6 s plus tard').
- PROCHAINE ETAPE : quick_check -Parts tests ; marque au sol, roue et Alt + clic minicarte apres Q-signals-ui ; SG4 sons.
- 2026-11-15 (réponse de l'utilisateur, session principale, docs seulement) : D821 (Q-signals-ui) = Q1 A (Alt + clic glissé > 16 px) + Q2 A (carré de couleur par type) : défauts codés, rien à changer ; pictogrammes (B) possibles plus tard.
