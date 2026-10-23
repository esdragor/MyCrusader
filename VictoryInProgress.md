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
- 2026-10-23 : **V2 lancée** (« go »). Worktree Arwen, branche `victory-v2` = `main` 2b53e83, `IN_USE.txt` écrit.
  Déjà fixé par le GDD : 2 vies au palier 0 (D349), ruine ~30 s puis reconstruction ~120 s seule, Paysans sans plafond,
  retour à tous ses PV (D352), forum à terre sans production / dépôt / tir / garnison, population gardée (D351), vies
  publiques dans le HUD (D353), 0 vie → défaite de V1. Code vu : chantier = `EHOLConstructionState` (Foundation,
  UnderConstruction, Finished), rendement des bâtisseurs D312 (`BuilderExponent=0.63`), `DestroyBuilding` rembourse la
  file. Questions de V2 posées une à une (V2-Q1…).
  - **V2-Q1 Aspect** : **décombres puis chantier** (recommandation) : à 0 PV, décombres à la taille du forum qui
    restent ~30 s, inertes ; puis il prend **l'aspect d'un chantier, quel qu'il soit** (barre de progression) ; fini =
    tous ses PV. Précision de l'utilisateur : la montée du sol est **provisoire** (comme les blocs de D261) ; la
    reconstruction reprend l'aspect du chantier, sans aspect à elle, et suivra donc le vrai chantier quand il viendra.
  - **V2-Q2 Paysans** : **comme un chantier** (recommandation) : clic droit sur le forum en reconstruction ; les
    Paysans s'ajoutent à la reconstruction seule par la courbe D312 (1 Paysan = autant que la reconstruction seule :
    0 → 120 s, 1 → 60 s, 5 → ~32 s, 10 → ~23 s, 40 → ~11 s) ; pendant les 30 s de décombres, ils attendent sur place.
  - **V2-Q3 File de production** : **remboursée** (recommandation) : à la chute, toute la file est annulée et remboursée
    en entier (comme `DestroyBuilding`) ; le point de ralliement est gardé pour après la reconstruction.
  - **V2-Q4 Vies dans le HUD** : **tableau des joueurs** (contre la recommandation « barre du haut ») : petit tableau
    façon score d'AoE2, une ligne par joueur (nom à sa couleur, vies, état du forum : debout / ruine / reconstruction).
    Par défaut : **en haut à droite, sous la barre** (le bas à droite est à la mini-carte) ; le bloc de sélection du
    forum montre aussi « Vies : N » et l'état (« En ruine 25 s », « Reconstruction 40 % »).

## Plan de V2 (tranché le 2026-10-23)

| Étape | Contenu | État |
|---|---|---|
| V2.1 Règles et état | Règles pures (vies = 1 + palier − chutes, palier 0 ; phases Debout / Ruine ~30 s / Reconstruction ; travail de reconstruction seule 120 s + Paysans D312 ; fini = tous ses PV) et tests rouges d'abord ; forum principal (celui du départ) : à 0 PV, chute au lieu de destruction (−1 vie, file remboursée, ralliement gardé), incassable en ruine et en reconstruction, ni production ni dépôt, population gardée ; à 0 vie → `DefeatSlot` (V1) ; vies et état publics, répliqués à tous ; triche `hol.Victory.FallForum <emplacement>` ; logs | **fait** (2026-10-23, non commité) : `Match/HOLForumRules` (`EHOLForumPhase` Standing / Ruin / Rebuilding, `FHOLForumState`, `FHOLForumTuning`, `BaseLives = 2`) ; `AHOLBuilding::bPrimaryForum` (répliqué, posé par le mode de jeu sur le centre de départ ; triche `SpawnBuilding` et stress test sans), `IsForumDown`, `TakesDeliveries`, `RestoreHealth` ; `AHOLGameState::ForumStates` répliqué à tous, `OnForumsChanged`, `GetLives`, `GetForumRebuildProgress`, `SetForumBuilders` (pour V2.2), `HandleForumFall` appelé par `DamageBuilding` à 0 PV ; défaite **toujours différée à la fin du pas** (`DamageBuilding` vient de `HOLCombatProcessor`), raison `EHOLDefeatReason::Forum` ; `UHOLConstructionSubsystem::RefundProductionQueue` sorti de `DestroyBuilding` ; production refusée sauf ralliement ; dépôts par `TakesDeliveries` ; triches `hol.Victory.FallForum <n>`, `hol.Victory.SkipRuin <n>` ; réglages `ForumLivesTier=0`, `ForumRuinTime=30`, `ForumRebuildTime=120` (`HOLBuildingSettings`) ; tests `Victory.Forum.Rules` / `.Fall` rouge 0/2 (41 échecs) puis **vert**, Victory **15/15**, voisins Buildings / Construction / Production / Gather / Combat / Match **50/50** |
| V2.2 Paysans | Clic droit sur le forum en ruine ou en reconstruction = ordre de bâtir ; attente sur place pendant la ruine, puis travail ajouté (D312) ; Paysans qui portaient une charge : autre dépôt | **fait** (2026-10-23, non commité) : `AHOLBuilding::NeedsBuilders()` (pas fini, ou forum principal à terre) ; `HOLBuildProcessor` : ordre de bâtir accepté sur son forum à terre (même ordre, mêmes places), refusé pour un autre joueur et sur un forum debout ; `PublishForumBuilders` → `SetForumBuilders` à chaque pas (0 pendant la ruine), log `Construction: slot N rebuilds its forum …, K builders at work` ; fin = suite de fin de chantier, `BuildersAtWork` à 0 ; client : clic droit sur son forum à terre = bâtir ; test `Victory.Forum.Builders` rouge (4 échecs) puis vert, Victory 16/16, Construction 19, Gather 7, Buildings 5 (47/47) ; 3 compilations, ~11 min, ~100k. Porteur dont le forum tombe : par lecture, rate le forum une fois puis autre dépôt (ou reste chargé), `MaxDepositMisses` = 6 empêche la boucle |
| V2.3 Aspect | Décombres à la taille du forum pendant la ruine, puis aspect du chantier ; bloc de sélection (vies, état, compte, progression) | **fait** (V2.3a aspect, V2.3b bloc de sélection, V2.3c traductions et captures, V2.3d retouches ; voir « Suivi de V2 ») |
| V2.4 Tableau des joueurs | HUD : tableau en haut à droite (nom, vies, état) ; textes fr / en / ja (boucle à part) | **fait** (voir « Suivi de V2 ») |
| V2.5 Vérification | Partie de `quick_check` (solo, 2 clients), scénario d'essai, essai de l'utilisateur | **fait** : `quick_check` partie `victory` (solo `victory forum`, 2 clients), scénario `forum_lives`, essai de l'utilisateur **validé** (« works ») |
| V2.6 Clôture | Décisions au GDD (V2-Q1 à Q4 ; **formule de D350 à corriger** : « 1 + palier − chutes » donne 1 vie au palier 0 alors que D349 en donne 2 → « 2 + palier − chutes », comme le code) , `EtapeDefini.md`, fusion dans `main` | **fait** (2026-10-23) : D417 à D420 au GDD, D350 corrigée, prochain D421 ; `EtapeDefini.md` à jour ; commits `4759b67` + `aa1ac36` (Arwen, `victory-v2`), `main` 8fef24f fusionnée ; `main` **avancée à aa1ac36** avec l'accord de mycrusader-7d, -cf, -db, -40. **V2 close** |

### Suivi de V2

- 2026-10-23 : V2.1 faite (sous-agent, **~23 min, ~185k : au-delà des 15 min**). **Bilan** : 5 compilations (~6 min) +
  règles + serveur + réplication + triches + 2 tests, c'était trop pour une boucle. **Changement** : une étape = un seul
  comportement et son test ; les agents notent l'heure de départ et s'arrêtent à 15 min avec un bilan, même au milieu.
  Risques notés pour la suite : le forum à terre reste `IsFinished()` (l'interface devra lire `IsForumDown()`) ; PV à 0
  pendant ruine et reconstruction, remis d'un coup à la fin ; `SetForumBuilders` à appeler à chaque pas ;
  `ForumStates` répliqué à chaque pas pendant la ruine (petit, allégeable) ; repli d'un porteur vers un autre dépôt pas
  vu en jeu. Logs : `Victory: forum of slot N fell, lives L`, `ruin -> rebuilding`, `rebuilt`, `slot N defeated (Forum)`.
  Tests rejoués par `scratchpad\run_tests.ps1 -Filter …` (session 3cb2f309).
- 2026-10-23 : V2.2 faite en 11 min (3 compilations) : le découpage plus fin tient. Restes : Paysans qui attendent
  pendant la ruine en activité « Building » (pas inactifs ; marteau sur les gravats ? → V2.3 : choix par défaut,
  **debout sans frapper**) ; ralliement impossible sur un forum à terre (client et `UHOLUnitSubsystem` lisent
  `IsFinished()`) → V2.3 par défaut l'autorise comme sur un chantier ; un Paysan qui finit une maison à < 20 m part
  reconstruire le forum à terre (cohérent avec « comme un chantier », à voir à l'essai) ; marche jusqu'aux places du
  forum en ruine vue seulement en jeu (V2.5). Progression : `AHOLGameState::GetForumRebuildProgress`, pas le
  `ConstructionWork` du bâtiment. V2.3 coupée en **V2.3a** aspect (décombres, chantier, Paysans debout pendant la
  ruine, grille grisée, ralliement sur le forum à terre), **V2.3b** bloc de sélection (vies, état, compte), **V2.3c**
  traductions et captures. V2.3a lancée.
- 2026-10-23 : accord donné à mycrusader-e9 pour avancer `main` à e28b00b (MV.4, formation sur place) : ne touche ni
  Arwen ni ce journal.
- 2026-10-23 : **V2.3a faite** (~12 min, ~130k, 4 compilations, non commitée) : ruine = pièces de `AHOLRubble`
  (`AHOLRubble::GetLookPieces` partagée) à la taille du forum, corps caché, sélectionnable, empreinte gardée ;
  reconstruction = aspect du chantier normal, hauteur par `HOLForumRules::GetLook` / `GetForumRebuildProgress` ; clients :
  `OnRep_Forums`, `OnRep_PrimaryForum`, aspect refait par phase ou cran visible de 2 % ; barre de vie suit `GetShownRise` ;
  Paysans debout sans frapper pendant la ruine, tournés vers le forum pendant la reconstruction ; grille **cachée** (pas
  grisée) sur le forum à terre (`bForumDown`), production refusée aussi par clic et touche ; ralliement sur son forum à
  terre (client et serveur) → ordre de reconstruire. Tests `Victory.Forum.Look` / `.Appearance` rouge (15 échecs) puis
  vert ; Victory, Buildings, Construction, Production, Animation **49/49**, CommandGrid 11/11. **À voir sur les captures
  (V2.3c)** : décombres à la taille du forum, passage décombres → dalle → montée → forum, 2 clients, Paysans sans marteau
  puis qui frappent, grille vide, drapeau sur le forum à terre. Restes : Paysans de la ruine pas tournés vers le forum ;
  parcours de tous les bâtiments à chaque cran (~50 par reconstruction, à alléger plus tard). V2.3b lancée.
- 2026-10-23 : **V2.3b faite** (~7 min, ~90k, 3 compilations, non commitée) : règle pure
  `HOLForumRules::GetSelectionBlock` → `FHOLForumBlock` (`GetRuinSecondsLeft` arrondi vers le haut, `GetRebuildPercent`
  en entiers), `FHOLForumTuning::StepSeconds`, `AHOLBuilding::GetForumBlock(bOwn)` ; `UHOLGameScreen::RefreshSelection` :
  debout = PV + « Vies : N » ; à terre = PV cachés, « En ruine — 25 s » ou « Reconstruction — 40 % » (+ « Bâtisseurs : N »
  au propriétaire seul) + « Vies : N » ; rafraîchi par `OnForumsChanged` (clients compris). Clés `ForumLives`,
  `ForumRuin`, `ForumRebuilding` (espace `HOLGameScreen`, déclarées dans `HOLForumRules.cpp`, désormais UTF-8 avec BOM).
  Tests `Victory.Forum.Block` + `Builders` / `Fall` étendus : rouge 3/6 puis vert ; Victory, CommandGrid, Construction,
  Localization 50/50, Buildings 5/5. Pas de test du HUD lui-même (aucun n'existe). Restes : traductions (le test
  Localization échouera après le gather), `Refresh()` complet à chaque pas de ruine tant qu'un forum est sélectionné.
  **V2.3c (traductions, captures) regroupée avec celles de V2.4.** V2.4 lancée (code, français seul).
- 2026-10-23 : **V2.4 faite** (~8 min, ~75k, 5 compilations dont 3 échecs C4458, non commitée) : règle pure
  `HOLForumRules::GetPlayersTable` → `FHOLPlayersTableRow` (emplacements occupés, groupés par équipe, `bNewGroup`,
  « Vaincu » grisé avant toute phase) ; `UHOLGameScreen::PlayersPanel` en haut à droite sous « Menu » (~59 px du haut),
  fond `HUDPanel` 60 %, 8 lignes 13 pt, nom à la couleur du joueur, écart de 6 px entre équipes, vaincu à 45 % ;
  rafraîchi par `OnForumsChanged` / `OnVictoryChanged` + relecture 4 fois par seconde dans `NativeTick` (pourcentage du
  serveur, config tardive chez un client) ; `Describe()` finit par `, players [slot 0 you colour 0 lives 2 standing | …]`.
  Clés `TableLives` « {0} vies », `TableLife` « {0} vie », `TableDefeated` « Vaincu », `TableRuin` « Ruine »,
  `TableRebuilding` « Reconstruction {0} % » (+ `You`, `PlayerN` réutilisées). Test `Victory.Forum.PlayersTable` rouge (6
  échecs) puis vert ; Victory + CommandGrid + Localization 32/32. **À voir sur les captures** : chevauchement avec la
  barre du haut sous 1500 px, alignement avec « Menu », lisibilité à 8 joueurs et des couleurs sombres sur le fond, grisé
  45 %, largeur pendant « Reconstruction 40 % », **couleur du nom peut-être écrasée par le style du `UCommonTextBlock`**
  (noms blancs ?). V2.3c lancée (traductions des 8 clés, captures du bloc et du tableau).
- 2026-10-23 : **V2.3c faite** (~9 min, ~75k, 1 compilation) : 8 clés en / ja dans `Tools/Localization/fill_po.py`
  (ForumLives « Lives: {0} » / « ライフ：{0} », ForumRuin « In ruins — {0} s » / « 廃墟 — 残り{0}秒 », ForumRebuilding
  « Rebuilding — {0}% » / « 再建中 — {0}% », TableLives « {0} lives » / « ライフ{0} », TableLife « {0} life »,
  TableDefeated « Defeated » / « 敗退 », TableRuin « Ruins » / « 廃墟 », TableRebuilding « Rebuilding {0}% » /
  « 再建中 {0}% ») ; 187 → 195 clés, aucune traduction changée. Captures `v23c_{a..f}_*_{fr,en,ja}.png` (Arwen,
  `Saved_Tests\Screenshots\WindowsEditor\HOL`, 1280×720) relues aussi par la session principale : noms à la couleur
  du joueur (le style ne l'écrase pas), pas de chevauchement, 8 lignes tiennent, décombres à la taille du forum, japonais
  complet. **Corrigé** : bloc d'un forum à terre figé (30 s, 0 %) → `Refresh()` 4 fois par seconde dans `NativeTick`
  si le forum sélectionné est à terre. **Défauts restants** : bleu de « Vous » (et vert 3, violet 6) peu lisible sur le
  fond ; vaincu affiche encore « 2 vies » ; « Vies : 2 » grand et blanc sous « Points de vie » petit et gris ;
  `TableLife` pour ≤ 1 (« 0 life », invisible). Boucle de retouches lancée, puis V2.5.
- 2026-10-23 : **V2.3d faite** (~8 min, ~95k, 3 compilations) : style `UHOLTextStyleHUDPlayerName` (gras 13, contour noir
  1 px, ombre ; couleur du joueur gardée), fond du tableau 80 % ; vaincu = « Vaincu » seul, sans vies ; rangée
  `SelectionLivesRow` « Vies » + valeur au style de « Points de vie » (`FHOLForumBlock::Lives`, `Text` = lignes de phase
  seules) ; `TableLife` pour exactement 1. Clé `ForumLivesLabel` « Vies » / « Lives » / « ライフ » ajoutée, `ForumLives`
  retirée, 195 entrées par langue, aucune autre traduction changée. `Block` / `PlayersTable` rouge 0/2 puis vert ;
  Victory + Localization + CommandGrid 32/32. Captures `v23d_*` relues (noms lisibles, vaincu sans vies, « Vies 2 » au
  style de « Points de vie »). **V2.3 close.** V2.5 lancée en deux agents parallèles sans compilation : partie
  `victory` de `quick_check` (solo + 2 clients) et scénario d'essai.
- 2026-10-23 : **scénario `forum_lives` prêt** (~9 min, ~100k) : `Tools/Scenarios/forum_lives.txt` / `.json` (Arwen,
  Plain, IA en 1). Temps : départ (6 Paysans, tableau) → 8 Hommes d'armes de l'IA frappent le forum
  (`hol.Cheat.DamageNear 2000` au premier coup), retirés 2 s après la chute (`hol.Units.Remove -14800 0 3000 1`) → ruine :
  l'utilisateur envoie ses Paysans (clic droit sur les décombres) → reconstruction (~30 s avec 6) → forum de l'IA tombe
  (`FallForum 1`) → 2e chute (`FallForum 0`) → Défaite. Répétition hors écran : PASS en 82,8 s (ruine 30,0 s,
  `6 builders at work`, `rebuilt` en 29,3 s, `defeated (Forum)`, `End banner: Defeat`), captures `v25s_*` relues
  (scratchpad). Notes : sans Paysans, reconstruction seule 120 s (pas de `waitlog` dans la version de l'utilisateur) ;
  des ennemis restés près des décombres tuent les bâtisseurs (normal sans V3) ; caméra qui glisse hors écran (curseur
  fictif, connu). Dossier `Saved_V25s` de la répétition à retirer d'Arwen avant le commit. **Essai demandé à
  l'utilisateur** : `play.ps1 forum_lives` depuis Arwen.
- 2026-10-23 : **partie `victory` de `quick_check` étendue** (~12 min, ~90k, `quick_check.ps1` seul, +66/−2) : 3e match
  solo `victory forum` (fall : `fell, lives 1`, production refusée `its forum is down`, HUD `lives 1 ruin`, bloc
  `in ruin 30 s` ; build : 2 Paysans, clic droit, `SkipRuin` → `2 builders at work`, `rebuilding 0%, builders 2` ;
  rebuilt : `lives 1 standing` ; defeat : `lives 0`, `defeated (Forum)`, `End banner: Defeat`, tableau `defeated
  greyed`) ; 2 clients : le client 1 abat son forum (`hol.Cheat.DamageNear 99999 0 0 own melee`), le client 2 lit
  `slot 1 colour 1 lives 1 ruin` (D353). Rouge sur vrais logs (pas d'unité au départ → 10/15 KO) puis vert.
  `-Parts victory` **PASS** 26 s ; `-Parts victory,net` **PASS** 130 s (+8 s). Reste : `hol.Victory.FallForum` /
  `SkipRuin` sans effet sur un serveur dédié (pas de contrôleur local, `HOLPlayerController.cpp` ~1345-1375).
- 2026-10-23 : **V2 validée par l'utilisateur** (« works », essai `forum_lives`). V2.6 : commit `4759b67` (Arwen,
  `victory-v2`), `main` 8fef24f fusionnée (ae9beea, sans conflit) ; GDD **D417 à D420** + formule de D350 corrigée
  (« 2 + palier − chutes »), prochain numéro D421 ; `EtapeDefini.md` : ligne « Fin de partie » sans « forum à vies »,
  ligne de la review du GDD à D421 (les deux non commités). `-Parts tests,menu,production,gather,build,anims,victory,net` :
  (1) FAIL en chaîne : l'ordre de bâtir de « victory forum » visait un point hors écran (caméra déplacée par les parties
  d'avant) → `hol.Camera.FocusSlot 0` avant ; (2) production et gather FAIL : `, lives N` et `, players [...]` ajoutés au
  rapport du HUD cassaient des vérifications qui lisent la ligne jusqu'au bout → sortis sur une ligne à part
  `HUD forum: selection, in ruin 27 s, lives 1, players [...]` (`UHOLGameScreen::DescribeForum`, `ShownForum`) ;
  à voir : gather « deux Paysans à moins de 90 cm » (69 cm) et anims « walking with Wood » (1 de 3), peut-être venus de
  `main` (MV) ou d'aléas. Relance en cours.
- 2026-10-23 : relance PASS sauf gather (90 cm) et anims (walking with Wood), **aussi rouges sur `main` 8fef24f** avec la même liste (Glorfindel emprunté et rendu), verts lancés seuls : noté dans `EtapeDefini.md`, pas de V2. Commit `aa1ac36` (ligne `HUD forum`, caméra) ; `main` avancée à aa1ac36 (autostash : fichiers des autres sessions intacts, fins de ligne seules réécrites). Arwen libérée. **Suite : V3 (tenir la ruine), puis V4 (messages)** ; plan de V3 à trancher question par question.

## V3 — Tenir la ruine (questions préparées pendant l'absence de l'utilisateur, 2026-10-23, à poser une à une)

Déjà fixé : D348 (reconstruction gelée tant que des ennemis sont près ; ~60 s tenus sans contestation → fondations
rasées, −1 vie, le cycle recommence ; une unité du défenseur gèle le compte, les Paysans qui reconstruisent contestent
aussi), D352 (compte dès la chute, ruine comprise ; rasé = reconstruction de zéro), D353 (anneau pour qui a la vision ;
sans brouillard pour l'instant : tout le monde), D401 (Neutre conteste comme un adversaire). Ouvert :

- **V3-Q1 Rayon de la zone** : A ~12 m autour du bord du forum (3 cases, anneau serré, combat au pied de la ruine) —
  recommandé ; B ~20 m (5 cases, archers qui tiennent de loin) ; C la zone anti-construction (bien plus large, une armée
  postée à distance « tient » sans se battre).
- **V3-Q2 Qui compte** : A par **équipe** (toute unité vivante d'un adversaire du défenseur = attaquant ; toute unité du
  défenseur ou de ses alliés = contestation ; Neutre = adversaire, D401 ; bâtiments non) — recommandé ; B le défenseur
  seul conteste (ses alliés ne gèlent rien) ; C les tours et bâtiments comptent aussi (une tour posée à côté tient).
- **V3-Q3 Le compte quand personne ne tient** (attaquants partis) : A **gardé** (ni remis à zéro ni rendu ; il reprend
  au retour des attaquants ; remis à zéro quand le forum est debout) — recommandé ; B remis à zéro dès que la zone est
  vide d'ennemis ; C qui redescend peu à peu (~2 s rendues par seconde).
- **V3-Q4 Après les fondations rasées** : A **nouvelle chute complète** (décombres ~30 s, compte reparti de 0, même
  message) — recommandé ; B reconstruction de zéro tout de suite, sans nouvelle ruine (compte reparti de 0) ; C le compte
  continue (une vie par minute tenue tant que l'attaquant reste).
- **V3-Q5 L'anneau** : A anneau au sol autour du forum, qui se remplit avec le compte, à la couleur de l'attaquant ; gris
  et figé quand c'est contesté — recommandé ; B une barre au-dessus du forum ; C seulement dans le bloc de sélection et le
  tableau (« Tenue 35 s »).

## À côté : échecs de `quick_check` enchaîné (pendant l'absence de l'utilisateur, 2026-10-23, Arwen branche `qc-chain`)

- Diagnostic (sous-agent, ~10 min, ~70k, rien corrigé) : **gather** « < 90 cm » reproduit par `-Parts production,gather`
  (73 s) : **vrai défaut du jeu** : des Paysans qui rentrent décharger ensemble au même point du dépôt n'ont que
  `StopSettleSteps = 10` pas pour s'écarter (`HOLGatherProcessor.cpp` l. 59, `Stop` l. 837) et restent superposés (69 cm
  pour des corps de 120 cm), contre D309 ; l'ordre d'arrivée change avec le temps pris par la partie production.
  **anims** « walking with Wood » : il faut `gather`/`production` **et** `build` avant ; hypothèse : 2 des 3 bûcherons
  coupent un arbre voisin hors de la boîte de ±700 cm de la sélection, le `waitlog Selection:` accepte n'importe quelle
  sélection. Logs : scratchpad `qcchain\`.
- Suite lancée : anims d'abord (confirmer par `-Parts gather,build,anims`, corriger la sélection du test), puis gather
  (se séparer jusqu'à ne plus se chevaucher, rouge `-Parts production,gather`). Rien de commité sans l'utilisateur.
- **anims corrigé** (test seul, ~13 min, ~60k, `quick_check.ps1`) : cause confirmée : l'arbre visé près de (-14221, -1900)
  abattu par gather et build, le groupe bois envoyé à 8,7 m (`source 4 at (-15093, -1810)`), la boîte de ±700 cm n'en
  sélectionnait qu'un (`Selection: Peasant #70`) ; pierre et or sélectionnaient 5 et 7 au lieu de 3. Correctif : chaque
  groupe mis à sa naissance dans un groupe de contrôle 6 à 9 (`hol.Select.Group Add`), resélectionné par
  `hol.Select.Group Select` en exigeant `Selection: 3 units (own): Peasant x3`, groupes vidés à la fin (la partie
  groups compare la barre exactement). `-Parts gather,build,anims` FAIL 334 s → **PASS** 332 s ; `anims` seul PASS 39 s.
  Gather (défaut du jeu) lancé.
- **gather corrigé** (jeu, ~13 min, ~80k, 4 compilations, `HOLGatherProcessor.cpp/.h` seuls) : cause vraie : l'écartement
  de fin de récolte poussait un Paysan dans le **mur visible** du centre ; après les 10 pas, la sortie du mur (EtapeDefini
  C.3, `EaseOutOfVisibleWalls`, 10 cm par pas) le ramenait tout droit sur son voisin (69 cm). Correctif : un Paysan
  arrêté qui s'écarte encore sort **d'un coup** du mur visible des dépôts (`EaseOutOfDeposits`, `FTransformFragment` en
  ReadWrite), et l'écartement continue **jusqu'à ne plus chevaucher** un corps au repos (`StopSettleMaxSteps` = 60 pas,
  3 s au plus ; `CollectRestingBodies`). Paysan seul : comme avant (sauf sortie du mur en un pas : petit saut possible).
  Rouge = `-Parts production,gather` (69 cm, 3 fois ; pas de test d'automatisation, pas de monde Mass dans `Gather.*`),
  vert PASS 46 s (119 / 121 cm), `gather` seul PASS, tests Gather + Construction + Movement 39/39. **À l'essai** : épuiser
  les baies avec 3-5 Paysans, retour au centre réparti le long du mur. `-Full` lancé.
- `-Full` sur Arwen : arrêt à « formation regroup » (`timeout … waitlog Selection: 6 units (own): Spearman x6`), cascade
  de FAIL derrière (combat, orders, net…), camera 8p FAIL ; **identique sur `main` f7269a8** (Glorfindel, rendu). Cause :
  la partie groups laisse 10 unités dans le groupe 2 hors de la zone nettoyée par regroup (`HUD groups: 2=10`) ;
  `hol.Select.Group Add 2` les garde (16). Correctif du test : chaque cas de regroup vide son groupe d'abord (`Group
  Select N` + `Group Remove N`). `-Full` relancé.
- `-Full` relancé : regroup **PASS** ; arrêt nouveau : « game launch ended by itself » (jeu coupé par quick_check à
  `$GameLimitSeconds` = 780 s : la séquence complète, débloquée, dure plus) → limite portée à **960 s**. Restent à voir :
  camera 8p (« launched from the screen ») et net « both gathered trees used up », aussi rouges sur `main`. `-Full` relancé.
- `-Full` (limite 960 s) : **42 PASS** (camera 8p comprise, débloquée), en 891 s ; seul FAIL : net « both gathered trees
  used up » (un seul arbre épuisé sur 2, chez les 2 clients ; aussi sur `main` en `-Full`, vert avec la liste de V2.6).
  Diagnostic lancé (sous-agent, test seul).
- **net corrigé** (test seul, ~10 min, ~60k) : la vérification « both gathered trees used up » ne tourne qu'avec la partie
  `sources` (d'où le vert de V2.6) ; le serveur entamait l'arbre le plus proche de (-15000, 0) / (15000, 0), anciens
  départs d'avant D344, alors que les clients visent depuis (-14800, 0) / (15200, 0) : pour le slot 1, arbre 43 entamé,
  arbre 45 récolté, jamais épuisé. Correctif : `TakeNear -14800 0 95` / `TakeNear 15200 0 95` ; sans `move`, `wait 4`
  avant le rapport des sources (le serveur n'a pas encore retiré / entamé les arbres). `-Parts net,sources,move` PASS
  160 s (sources 3 et 45 épuisées, 373 debout). Vu une fois : client 1 bloqué sur `Waiting on static mesh SM_Arrow`
  (`net,sources` seul), sans doute passager. `-Full` final lancé.
- **`-Full` PASS** en 895 s (44 parties, tests 132/132) sur Arwen `qc-chain` : premier `-Full` vert depuis les fusions
  du 2026-10-23. L'utilisateur est revenu ; il choisit l'essai du correctif de récolte d'abord (scénario `berries_return`
  en préparation), puis les questions de V3.
- `-Full` final **PASS** 895 s (44 parties). Scénario `berries_return` (~9 min, ~75k) : baies avec 5 Paysans, puis bois
  avec 3 ; répétition : 119,7 cm et 120,4 cm au plus près. **Essai de l'utilisateur validé** (« works »). Commit
  `f2acad7` (Arwen, `qc-chain`), `main` b4a8c8c (doc D421-D427) fusionnée (7cba45b) ; ligne « quick_check enchaîné »
  retirée d'`EtapeDefini.md`. `main` **avancée à 7cba45b** avec l'accord de mycrusader-05, -e5, -7d (autostash : fichiers des autres intacts).
- **Suite : V3 (tenir la ruine)**, questions préparées plus haut (V3-Q1 à Q5), à poser une à une.
