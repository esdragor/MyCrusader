# Victory in progress

Journal du chantier **conditions de victoire** (ligne « Fin de partie » d'`EtapeDefini.md`), ouvert le 2026-10-23.
Règles : GDD § 14.1, décisions **D347 à D360**. Décisions nouvelles du chantier : à la suite de la review du GDD (prochain
numéro libre au moment de trancher, voir `EtapeDefini.md`).

## Ce qui existe déjà (état au 2026-10-23, `main` d239a1c)

- Bâtiments avec PV, attaquables (D345), détruits → décombres `AHOLRubble` (D345.2a) ; centre principal par joueur (J2).
- Configuration de partie : équipes (`FHOLMatchSlot::Team`, 0 à 4), options `bNomad` et `bRegicide` (menu seulement,
  sans effet en jeu).
- Rien de la fin de partie : pas d'état « vaincu », pas d'écran de fin, pas d'abandon.
- N'existent pas encore : héros et réapparition, paliers, garnison, tir du forum, Roi, départ Nomade, pack d'alerte
  (D202), mini-carte.

## Découpage proposé (à valider)

| Étape | Contenu | Dépend de |
|---|---|---|
| **V1** Défaite et fin de partie | État « vaincu » répliqué ; abandon (D356 : bouton + confirmation) comme premier déclencheur ; effondrement du vaincu (D358 : unités tuées, bâtiments en ruines inertes, ressources perdues) ; victoire quand tous les adversaires ont perdu (D357, équipes du lancement, dernier debout) ; écran Victoire / Défaite ; vaincu spectateur | rien |
| **V2** Forum à vies | Vies = 1 + palier − chutes (palier 0 pour l'instant → 2 vies) ; à 0 PV : −1 vie, ruine ~30 s, incassable, inerte (ni production ni dépôt) ; reconstruction seule ~120 s, Paysans qui aident sans plafond, retour à tous ses PV ; 0 vie → V1 ; vies de chaque forum dans le HUD pour tous (D353) | V1 |
| **V3** Tenir la ruine | D348 : ennemis près de la ruine → reconstruction gelée ; ~60 s sans contestation (compte dès la chute) → fondations rasées, −1 vie, reconstruction de zéro ; une unité du défenseur gèle le compte ; anneau du compte | V2 |
| **V4** Messages | D353 : message à tous (« tel joueur se fait pousser ») ; anneau seulement pour qui voit le forum ; pack d'alerte complet et ping plus tard (D202, mini-carte) | V2, V3 |
| Plus tard | Vies par palier (avec les paliers) ; réapparition du héros à la ruine (avec les héros) ; garnison éjectée (avec la garnison) ; Nomade D354 (avec le départ Nomade) ; Régicide D355 (avec le Roi) | chantiers à venir |

Ordre choisi par l'utilisateur (2026-10-23) : **V1 d'abord**. Travail dans un worktree neuf
`C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`, branche `victory-v1` (depuis `main` d239a1c), première compilation
PASS en 134 s.

## Plan de V1 (tranché le 2026-10-23)

Décisions à numéroter au report dans le GDD (§ 21) ; réponses de l'utilisateur :

- **V1-Q1 Abandon** : **Échap** désélectionne s'il y a une sélection, **sinon ouvre le menu en jeu** ; plus un
  **bouton « Menu »** dans la barre du haut (réponse « Échap et bouton »). Menu : Reprendre, Abandonner (→ confirmation
  « Abandonner la partie ? » Oui / Non) ; Réglages plus tard.
- **V1-Q2 Fin de partie** : **bandeau « Victoire » / « Défaite »** par-dessus la partie, boutons **« Continuer à
  regarder »** (spectateur, D357 : ne commande plus rien, la partie continue) et **« Quitter vers le menu »** ;
  statistiques plus tard.
- **V1-Q3 Effondrement (D358)** : unités du vaincu tuées sur place (corps 30 s, comme au combat) ; bâtiments détruits
  comme au combat (**décombres actuels**, 10 s) ; ressources perdues.
- Sans brouillard de guerre pour l'instant, le spectateur voit toute la carte.
- Pour essayer avant V2 : commande de triche `hol.Victory.Defeat <emplacement>` (fait perdre un emplacement, IA comprise,
  comme un abandon).

Étapes :

| Étape | Contenu | État |
|---|---|---|
| V1.1 Règles et état | Règles pures (équipes du lancement, dernier debout, qui a gagné) et tests ; état « vaincu » par emplacement répliqué à tous ; abandon demandé au serveur (vérifié) ; `hol.Victory.Defeat` ; fin de partie décidée par le serveur | **fait** (2026-10-23, non commité) : `Match/HOLVictoryRules`, `Match/HOLGameState` (`AHOLGameState` : `SlotDefeats`, `bMatchOver`, `WinnerMask` répliqués ; `DefeatSlot`, délégués `OnSlotDefeated` / `OnMatchOver`), `ServerSurrender` / `ServerCheatDefeat` du contrôleur, `hol.Victory.Surrender`, `hol.Victory.Defeat <n>` ; tests `HeroesOfLiberty.Victory` rouge 0/6 puis **vert 6/6** ; essai sans fenêtre sur `Plain` conforme (sous-agent, ~10 min, ~105k) |
| V1.2 Effondrement | D358 : unités tuées, bâtiments détruits, ressources à 0, ordres et production du vaincu refusés par le serveur | **fait** (2026-10-23, non commité) : `AHOLGameState::CollapseSlot` dans `DefeatSlot` ; unités à 0 PV et Dying (chemin du combat : `FinishDying` → corps 30 s), naissances en attente supprimées, `RequestUnits` refuse un vaincu (`HOLUnitSubsystem::KillUnitsOfOwner`) ; bâtiments par `DestroyBuilding` (décombres 10 s) ; stock à 0 après les remboursements ; garde unique `UHOLCommandSubsystem::IsRefused` + `DropOrdersOf` ; logs `Victory: slot N collapsed (U units, B buildings)`, `Victory: orders of defeated slot N refused (D358)` ; test `Victory.Collapse` rouge (20 points) puis **vert 7/7**, voisins Buildings / Production / Construction 25/25 (sous-agent, ~12 min, ~140k). À vérifier par script en V1.4 : chute et corps. Attention V2/Régicide : un `DefeatSlot` appelé dans un processeur Mass devra différer l'effondrement à la fin du pas |
| V1.3 Interface | Menu en jeu (Échap / bouton), confirmation, bandeau de fin et ses 2 boutons, spectateur ; textes fr / en / ja | **V1.3a fait** (2026-10-23, non commité) : `Player/HOLGameMenuRules` (Échap), `Screens/HOLGameMenu` (Reprendre, Abandonner → « Abandonner la partie ? », focus sur Non), bouton « Menu » en haut à droite ; 5 clés fr / en / ja (2 clés périmées `RefundAll` / `RefundHalf` retirées à la régénération, aucune traduction changée) ; tests `Victory.GameMenu.*` rouge puis **vert 9/9** ; contrôle hors écran : Échap → menu → Abandonner → Oui → `slot 0 defeated (Surrender)`, `collapsed`, captures `v13a_*.png` (Arwen, `Saved_Tests\Screenshots\WindowsEditor\HOL`). Défaut : fond de la confirmation transparent (le menu se lit dessous) → corrigé en V1.3b. **V1.3b codé** (2026-10-23, non commité) : `EHOLEndBanner`, `GetEndBanner` / `ShouldOpenEndBanner` (règles pures), `OnRep_Victory` + délégué `OnVictoryChanged` (diffusé à la main chez le serveur d'écoute / en solo), `Screens/HOLEndBanner` (Victoire / Défaite, « Continuer à regarder » ou « Continuer » pour le gagnant, « Quitter vers le menu »), spectateur `AHOLPlayerController::IsSpectator()` (ordres, grille, pose, annulations bloqués côté client ; grille cachée), « Abandonner » absent pour un vaincu ; fond de la confirmation : la cause était le fondu des couches, retiré par-dessus la partie ; tests `Victory.EndBanner.Which` / `.Closed` rouge puis **vert 11/11** ; contrôle hors écran **PASS** (A : abandon → Défaite → Continuer à regarder → menu du vaincu sans Abandonner ; B : IA vaincue par triche → Victoire → Quitter vers le menu → écran titre), captures `v13b_*.png` (fr / en / ja) ; 5 clés `HOLEndBanner` traduites (187 clés, aucune traduction changée) ; boutons de rangée à largeur minimale au lieu de fixe (`HOLScreen::AddButton`, libellés longs qui débordaient ; touche aussi l'écran Nouvelle partie) ; tests **11/11**. **V1.3 close** |
| V1.4 Vérification | Partie `victory` de `quick_check` (solo, 2 clients), scénario d'essai, essai de l'utilisateur | **fait** (2026-10-23) : essai de l'utilisateur **validé** (« victory_win fonctionne », puis « works » après « Quitter vers le menu » du spectateur). **V1.4a fait** (2026-10-23, non commité) : partie `victory` solo **PASS** (33-35 s ; abandon par Échap → menu → Oui, ordre refusé après la défaite, menu par-dessus le bandeau puis Échap → bandeau fermé, IA vaincue → Victoire → Quitter vers le menu), dans `-Full` ; `-Parts tests,menu` PASS (114/114, écran Nouvelle partie intact) ; correctif d'Échap préventif (`HOLGameMenuRules::KeepsMatchMenuInput`, `UHOLRootLayout::HasMenuOtherThan`, test `Victory.GameMenu.Input` rouge puis vert 12/12) : **défaut non reproduit hors écran** (le bandeau se réactive seul), à revoir à l'essai (souris ?). Reste : partie 2 clients, scénario, essai |
| V1.5 Clôture | Décisions au GDD, ligne d'`EtapeDefini.md` mise à jour, fusion dans `main` | **fait** (2026-10-23) : **D361** (menu en jeu, abandon), **D362** (bandeau de fin, « Quitter vers le menu » du spectateur), **D363** (effondrement par les chemins du combat, spectateur) au § 21 et paragraphe « À l'écran » au § 14.1 ; ligne « Fin de partie » d'`EtapeDefini.md` sans « abandon, vaincu effondré » ; commit `Victory V1` (9951964) sur `victory-v1`, `main` fusionnée trois fois sans conflit (lignes A / B / C, MV.4, doc MV.4) ; **`main` avancée à 4541266** avec l'accord de mycrusader-5c, -97, -3c, -d4 ; GDD (D361 à D363 avec les D364+ de la review) commité par mycrusader-d4. **V1 close** |

## Suivi

- 2026-10-23 : état des lieux fait, découpage proposé à l'utilisateur ; V1 d'abord ; worktree Arwen créé et compilé ;
  plan de V1 tranché (Q1 à Q3).
- 2026-10-23 : V1.1 faite (voir tableau). Logs pour les contrôles : `Victory: slot N defeated (Reason)`,
  `Victory: match over, winners a,b` / `no winner`, refus `Victory: surrender of slot N refused`. V1.2 lancée.
- 2026-10-23 : V1.2 faite (voir tableau) ; nouveaux fichiers remis en CRLF. V1.3 lancée.
- 2026-10-23 : V1.3 coupée en **V1.3a** (menu en jeu, abandon) et **V1.3b** (bandeau de fin, spectateur). Sous-agent
  de V1.3a arrêté au bout des 15 min, juste avant de compiler : code, règle d'Échap (`Player/HOLGameMenuRules`, test
  `HOLGameMenuTests.cpp`), `Screens/HOLGameMenu`, bouton de la barre, traductions écrits, non vérifiés. **Bilan** : code +
  traductions + captures, c'était trop pour une boucle ; à l'avenir, les traductions et les captures de contrôle
  font une boucle à part. Nouvel agent lancé pour finir : compiler, tests, clé « SurrenderConfirm » à compléter,
  diff des .po à contrôler, 2 captures.
- 2026-10-23 : V1.3a vérifiée (rien à corriger ; la clé « SurrenderQuestion » était bien là ; ~5 min, ~80k). V1.3b lancée
  (code seulement ; traductions et captures dans une boucle à part).
- 2026-10-23 : V1.3b codée (~14 min, ~155k). **Leçon** : un contrôle qui attend l'interface (`waitui`) ne marche pas
  sous `-nullrhi`, il faut `-RenderOffScreen`. Restes : cas limite (menu ouvert puis fermé par-dessus le bandeau →
  Échap coupé pour le bandeau), ressources / population du vaincu à vérifier à l'essai. Boucle de contrôle et
  traductions lancée.
- 2026-10-23 : contrôle de V1.3b et traductions faits (~10 min, ~90k) ; ressources et population du vaincu à 0 vus sur
  capture. V1.4 lancée : cas limite d'Échap, partie `victory` de `quick_check`.
- 2026-10-23 : V1.4a faite (~11 min, ~85k). Scénario d'essai lancé (sous-agent à part, règle « scénario neuf = son
  propre agent »), puis la partie 2 clients.
- 2026-10-23 : l'utilisateur est absent (« j'afk ») ; on continue seul, sans commit. Agent des scénarios arrêté à
  15 min, travail fait : `Tools/Scenarios/victory_win.txt` / `.json` (Entrée → l'IA perd par triche → Victoire) et
  `victory_surrender.txt` / `.json` (l'utilisateur abandonne → Défaite → spectateur, cas limite d'Échap à la souris),
  répétés hors écran (copies `_dry` rangées dans le scratchpad), captures `vw_*` / `vs_*`. **Défauts vus** : population
  du vaincu affichée 11/175 après l'effondrement (unités mortes) ; caméra de `victory_win` qui ne montre pas les 8
  Lanciers de l'IA. Agent de correction lancé.
- 2026-10-23 : **population du vaincu corrigée** (~15 min, ~105k) : `UHOLProductionSubsystem::AdvanceQueues` n'écrivait
  la population qu'aux joueurs ayant un bâtiment ; elle l'écrit maintenant à tous les `AHOLPlayerState` (sans
  bâtiment : 0 / 0) ; `Victory.Collapse` étendu (HUD du vaincu) rouge puis vert, filtre Victory 12/12. Caméra de
  `victory_win` : les « Lanciers » des captures étaient des buissons de baies (Lanciers nés sous le centre) ; scénario
  déjà corrigé, pas encore revu en jeu (lanceur : commandes de `-ExecCmds` à séparer par une **virgule**).
- 2026-10-23 : répétition par la session principale : `victory_surrender` → 11 unités et 2 bâtiments tombent, HUD du
  vaincu **0 / 0** ; `victory_win` → les 8 Lanciers de l'IA sont dans le champ avant l'effondrement, puis 8 unités et
  3 bâtiments tombent, Victoire (après coup, la caméra glisse hors écran : défilement au bord du curseur fictif, connu).
  Copies `_dry` retirées. `quick_check -Parts victory` **PASS** (20 s). Partie 2 clients lancée.
- 2026-10-23 : **V1.4c faite** (~9 min, ~120k) : avec `victory` + `net`, fin de la partie `net` : le client 1 abandonne
  (`hol.Victory.Surrender`), serveur `defeated (Surrender)`, `collapsed (7 units, 2 buildings)`, `match over, winners 1` ;
  client 1 `End banner: Defeat`, client 2 `End banner: Victory` ; le client 2 voit les unités du client 1 passer de 7
  à 0. `-Parts victory,net` **PASS en 125 s** (+5 s), contrôles `net` existants tous verts. Reste possible : bâtiments
  vus par le client 2 (un `Building report` de plus casserait un compte existant).
- **Prêt pour l'essai de l'utilisateur** : `Tools\Scenarios\play.ps1 victory_win` et `victory_surrender` (depuis Arwen).
  Ensuite : commit (après validation), `-Parts` des parties touchées, fusion dans `main` (accord des sessions de la copie
  principale), V1.5 (GDD, `EtapeDefini.md`).
- 2026-10-23 : essai de l'utilisateur : **`victory_win` validé** (« victory_win fonctionne ») ; `victory_surrender` lancé.
- 2026-10-23 : retour de l'essai de `victory_surrender` : « Quand je suis spectateur, il faut avoir "retour au menu" ».
  Fait : menu en jeu d'un vaincu ou d'une partie finie → **« Quitter vers le menu »** à la place d'« Abandonner »
  (même texte et même sortie que le bandeau : `UHOLEndBanner::LeaveMatch`, sans confirmation),
  `HOLGameMenuRules::CanQuitToMenu` ; test `Victory.GameMenu.QuitToMenu` rouge (3 échecs) puis vert, filtre Victory
  **13/13** ; `quick_check -Parts victory` PASS (23 s). `victory_surrender` relancé pour la suite de l'essai.
- 2026-10-23 : **V1 validée** par l'utilisateur (« works »). `-Parts tests,menu,production,victory,net` PASS (132 s,
  115/115) ; commit `9951964` ; `main` fusionnée (lignes A / B / C) : PASS (123 s, 119/119) ; MV.4 fusionnée
  (mycrusader-5c, accord donné) : PASS (233 s, 121/121) ; GDD D361 à D363 ; EtapeDefini à jour ; `main` → 4541266.
- **Suite du chantier** : V2 (forum à vies), puis V3 (tenir la ruine), V4 (messages). Plan de V2 à trancher question
  par question.
