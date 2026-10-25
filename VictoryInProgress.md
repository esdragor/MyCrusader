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

## Plan de V3 — Tenir la ruine (tranché le 2026-10-23)

Réponses de l'utilisateur :
- **V3-Q1 Rayon** : **~12 m du bord du forum** (recommandation).
- **V3-Q2 Qui compte** : **par équipe** (recommandation) : unité vivante d'un adversaire du défenseur = attaquant (Neutre
  compris, D401) ; unité du défenseur ou de ses alliés = contestation ; bâtiments et corps ne comptent pas.
- **V3-Q3 Compte sans personne** : **redescend peu à peu** (~2 s rendues par seconde) (contre la recommandation « gardé »).
- **V3-Q4 Après les fondations rasées** : **nouvelle chute complète** (recommandation) : −1 vie, décombres ~30 s,
  reconstruction de zéro, compte à 0 ; dernière vie → défaite.
- **V3-Q5 Affichage** : **anneau au sol** (recommandation) au rayon de la zone, qui se remplit à la couleur de
  l'attaquant, gris et figé si contesté, qui recule quand le compte redescend ; bloc et tableau : « Tenue — 35 s ».

Rappel D348 / D352 : reconstruction gelée tant qu'un ennemi est dans la zone ; le compte court dès la chute (ruine
comprise) ; il n'avance que si des attaquants sont là **sans** unité du camp défenseur ; contesté = figé.

| Étape | Contenu | État |
|---|---|---|
| V3.1 Règles et état | Règles pures (présence attaquants / défenseurs → compte qui avance, figé, redescend ; reconstruction gelée ; 60 s → nouvelle chute ou défaite), état répliqué, logs, triche ; test rouge d'abord avec la présence passée à la règle | **fait** (2026-10-23, non commité, ~12 min, ~90k, 2 compilations) : `HOLForumRules::Step(State, Tuning, Builders, FHOLForumPresence)`, `Raze`, `GetHoldSeconds` / `GetHoldProgress` / `GetHoldName` ; `FHOLForumState` + `HoldSteps`, `Hold` (`EHOLForumHold` None / Holding / Contested / Receding), `HolderSlot` (int8) ; attaquant de l'anneau = le plus nombreux, slot le plus bas à égalité (V3.2) ; `ForumHoldTime=60`, `ForumHoldDecay=2`, `ForumHoldRadius=1200` ; `AHOLGameState::AdvanceForums` → `GetForumPresence(Slot)` (triche d'abord, puis `SetForumPresenceProvider`, sinon personne) ; logs `held by slot A (H s)`, `contested`, `released`, `no longer held`, `razed, lives L` ; triche `hol.Victory.HoldPresence <Slot> <0|1> <0|1> [Attacker]` / `<Slot> off` ; tests `Victory.Forum.Hold.Rules` / `.Chain` rouge 0/2 puis vert, Victory + Construction + Buildings **46/46** ; pas = 0,05 s (60 s = 1200 pas) |
| V3.2 Présence réelle | Unités dans la zone (12 m du bord) par équipe, au serveur, chaque pas ; test sur la vraie chaîne (unités posées près de la ruine) | **fait** (2026-10-23, non commité, ~11 min, ~75k, 4 compilations) : `Match/HOLForumPresence` (`FHOLForumPresenceCounter`, `MakeProvider`, branché par `InitVictory` au serveur) : forums principaux à terre d'emplacements non vaincus, **un seul parcours des unités par pas** (aucun si tous debout), distance au rectangle de l'emprise (`HOLConstruction::GetDistance` sur `GetBodyBounds`) ≤ `HoldRadius` ; défenseur = propriétaire ou allié (`GetCamp`), tout autre = attaquant (Neutre = emplacement fermé / hors limites pour l'instant, en attendant les postures de D369) ; `UHOLUnitSubsystem::ForEachLivingUnit` ; `AHOLGameState::GetCurrentStep` ; tests `Victory.Forum.Hold.Presence` / `.PresenceTeams` rouge 0/2 puis vert, Victory + Construction + Buildings + Combat **57/57** ; `Forum.Builders` adapté (le Paysan ennemi au contact gelait la reconstruction : tué après le refus de son ordre). Non testé : égalité entre deux attaquants |
| V3.3 Affichage | Anneau au sol (remplissage, couleur, gris, recul), « Tenue — N s » au bloc et au tableau | **fait** (2026-10-23, non commité, ~15 min, ~85k, 3 compilations) : règles `ShowsHold`, `DescribeHold`, `GetRing` → `FHOLForumRing` (Fill, Tint Holder / Grey / Faded), `GetRingColor`, `GetZoneOutlinePoint` (emprise + `HoldRadius`, coins arrondis, départ au milieu du côté +X, sens horaire) ; anneau dans `AHOLSelectionMarkers::UpdateForumRings` (à l'image, bandes au sol `M_GroundFrame`, 128 morceaux, arc 60 cm, contour sombre 16 cm ; gris 0,5 contesté ; atténué = couleur mêlée de gris clair au recul), `AHOLGameState::FindPrimaryForum` ; textes `ForumHold` « Tenue — {0} s », `ForumHoldContested` « Contestée — {0} s », `TableHold` « {0} · tenue {1} s », `TableHoldContested` « {0} · contestée {1} s » ; `HUD forum` : `, hold N s holding|contested|receding by slot A` (avant `, lives`) et ` hold N s <état> by slot A` au tableau ; test `Victory.Forum.Hold.Display` rouge 0/1 puis vert, Victory + CommandGrid + Localization **37/37**. À voir : relief (anneau à la hauteur du pied du forum), lisibilité du trait, gris vs atténué, longueur du bloc à 3 lignes, largeur du tableau |
| V3.4 Traductions, captures | fr / en / ja, captures hors écran | **fait** (2026-10-23, non commité, ~15 min, ~70k) : `ForumHold` « Held — {0} s » / « 占拠中 — {0}秒 », `ForumHoldContested` « Contested — {0} s » / « 争奪中 — {0}秒 », `TableHold` « {0} · held {1} s » / « {0} · 占拠{1}秒 », `TableHoldContested` « {0} · contested {1} s » / « {0} · 争奪{1}秒 » ; 195 → 199 entrées, aucune traduction changée. Captures `v34_*` (Arwen, `Saved_Tests\Screenshots\WindowsEditor\HOL`, 1600×900) relues aussi par la session principale : arc rouge qui part de midi en sens horaire, coins arrondis propres, bloc à 3-4 lignes et tableau qui tiennent, japonais complet, 60 s → `razed, lives 0` → Défaite. **Défauts** : anneau fin (piste ~2 px, arc 3-4 px) ; arc gris « contestée » presque invisible sur le sol gris de `Plain`, recul rose pâle peu contrasté → retouche V3.4b (arc plus épais, gris foncé, recul plus sombre). Non capturé : rasé avec une vie restante (couvert par `Hold.Chain`) |
| V3.5 Vérification | `quick_check` (solo, 2 clients), scénario, essai | **fait** : partie `victory` (held, contested, released ; 2 clients), scénario `forum_hold`, essai de l'utilisateur **validé** (« fonctionne ») |
| V3.6 Clôture | GDD, `EtapeDefini.md`, fusion | **fait** (2026-10-23) : D434 à D438 au GDD (prochain D439), `EtapeDefini.md` sans « ruine à tenir » ; commit `28c1d3a`, `main` fusionnée deux fois (7cacb4c, ecc0190) ; `-Full` sans rocks PASS 913 s (44 parties, 138/138), puis tests,victory,sources,gather,net PASS ; `main` **avancée à a74a5d1** avec l'accord de mycrusader-7d, -e5, -15 et d'une session sans nom. **V3 close** |

### Suivi de V3

- 2026-10-23 : V3.4b (~10 min, ~40k) : arc 60 → 120 cm, piste 16 → 28 cm, contesté gris 0,5 → 0,18, recul = couleur mêlée
  de gris foncé 0,1 (rouge → 0,55 / 0,05 / 0,05) ; captures `v34b_*` : contesté et recul lisibles ; `Hold.Display`
  rouge puis vert, Victory + CommandGrid 36/36. Incident : un lancement de tests de V3.4 terminé par `,Quit` (au lieu
  de `;SoftQuit`) restait ouvert et bloquait la compilation : arrêté. V3.5 lancée (quick_check + scénario, sans
  compilation). `main` avancée entre-temps par e5 (M6) et 7d (rochers, f6703a9) avec mon accord : à fusionner avant le
  commit de V3 (recoupements : `DefaultGame.ini`, `quick_check.ps1`).
- 2026-10-23 : **scénario `forum_hold` prêt** (~8 min, ~70k) : départ (6 Paysans, 6 Hommes d'armes hors zone) → 4 Piquiers
  abattent le forum et restent (tenue) → l'utilisateur envoie ses Hommes d'armes (contestée) → Piquiers morts (recul) →
  Paysans à la reconstruction, Entrée avant la fin → unités retirées, 2e vague qui tient 60 s → rasé, 0 vie, Défaite.
  Répétition (`-saveddirsuffix=V35s`) : `held`, `contested (10 s)`, `released`, `ruin -> rebuilding`, `no longer held`,
  `6 builders at work`, `razed, lives 0`, `defeated (Forum)` ; captures `v35s_*` (scratchpad) relues. Points : rasé au
  temps 2 si l'utilisateur tarde plus de 60 s ; Entrée avant la fin de la reconstruction au temps 4 ; 2e vague au bord
  de l'anneau. **Essai demandé.**
- 2026-10-23 : **partie `victory` étendue pour V3** (~11 min, ~70k, `quick_check.ps1` seul, +30/−2) : sections `victory forum
  held` (3 Lanciers de l'emplacement 1 à 5 m du bord : `held by slot 1`, `HUD forum` `hold N s holding by slot 1`),
  `contested` (un Paysan à soi), `released` (Lanciers retirés : `released`, `receding` qui baisse, `no longer held`) ;
  2 clients : `HoldPresence` fixé par le client 1, le client 2 lit `slot 1 colour 1 lives 1 ruin hold 2 s holding by
  slot 0`. Rouge rejoué hors jeu sur 3 logs. `-Parts victory` PASS 32 s, `-Parts victory,net` PASS 126 s. Non couvert
  par quick_check : le rasage (60 s réelles ; `slomo` ignoré dans une séquence de test : `HOLTestSequence.cpp:336` passe
  par `GEngine->Exec`, pas le CheatManager) → couvert par `Hold.Chain`.
- 2026-10-23 : **V3 validée** (« fonctionne »). Commit `28c1d3a` (Arwen, `victory-v3`), `main` 7cacb4c (M6, rochers) fusionnée sans conflit ; GDD **D434 à D438** (prochain D439), `EtapeDefini.md` sans « ruine à tenir » ; `-Full` lancé avant l'avance de `main`.
- 2026-10-23 : `-Full` d'abord cassé par la partie `rocks` de main (ses `hol.Units.Remove` à 1800 cm emportaient 3 Paysans
  du centre, gather attendait 21 unités) : signalé à mycrusader-7d, corrigé par lui (ecc0190 : `Remove [Type]`, rocks hors
  de `-Full`). **Attention** : `-Full` dure ~913 s pour une limite de 960 s (`$GameLimitSeconds`). **V3 close.**
- **Suite : V4 (messages, D353)** : « tel joueur se fait pousser » à tous ; alerte complète et ping plus tard (D202,
  mini-carte). Plan à trancher question par question.

## V4 — Messages (questions préparées le 2026-10-24, à poser une à une)

Déjà fixé : D353 (information publique, spectacle suivant la vision : qui voit le forum → message + alerte majeure
complète D202 + anneau ; qui ne le voit pas → **message seul**, « tel joueur se fait pousser », sans ping ni anneau ;
l'IA reçoit la même chose) ; D202 (majeure = bandeau bref en haut, héraut de faction, ping, caméra au clic ; mineure =
fil discret sur le côté, ping, son court) ; D385 (fil de discussion où passent aussi les annonces). État du code
(`main` 6167f5c) : **aucun fil ni bandeau d'alerte**, pas de mini-carte, pas de brouillard (tout le monde voit tout) ;
`ForumStates` déjà répliqué à tous (`OnForumsChanged`), donc les changements de phase sont connus de chaque client.

- **V4-Q1 Quels événements** : A **chute, rasé, vaincu, et « on tient la ruine »** (début de la tenue par un adversaire)
  — recommandé ; B A + reconstruit et contesté (plus bavard) ; C chute et rasé seulement (le mot de D353).
- **V4-Q2 Où** : A **un fil de messages** à gauche, au-dessus de la barre du bas (futur fil des alertes mineures et de
  la discussion, D202 / D385), chaque ligne ~8 s puis s'efface — recommandé ; B un bandeau en haut pour tous ; C les deux :
  bandeau pour qui est concerné, fil pour les autres.
- **V4-Q3 Qui a le « spectacle »** (sans brouillard, tout le monde voit tout) : A **son camp** (soi et ses alliés)
  reçoit l'alerte majeure (bandeau + son, clic = caméra sur le forum), les autres le message seul — recommandé, reprend
  D353 en lisant « vision » comme « camp » jusqu'au brouillard ; B tout le monde reçoit l'alerte (pas de brouillard) ;
  C message seul pour tous, alerte plus tard avec la mini-carte.
- **V4-Q4 Les mots** : A **concrets et nommés** (« Le forum de Joueur 2 est tombé — 1 vie », « Joueur 1 tient la ruine
  de Joueur 2 », « Joueur 2 est vaincu ») avec « Votre forum est tombé » pour soi — recommandé ; B le ton de D353 (« Joueur
  2 se fait pousser ») ; C icônes et noms seuls.
- **V4-Q5 Son** : A **un son court** pour l'alerte (placeholder du moteur), héraut plus tard — recommandé ; B aucun son
  pour l'instant ; C un son pour chaque message.

## Plan de V4 — Messages (tranché le 2026-10-24)

Réponses de l'utilisateur :
- **V4-Q1 Événements** : **chute, rasé, vaincu, tenue** (recommandation) : forum tombé, fondations rasées, joueur
  vaincu, début de la tenue de la ruine par un adversaire.
- **V4-Q2 Où** : **un fil de messages**, chaque ligne ~8 s puis s'efface — **à droite, pas à gauche** (nuance de
  l'utilisateur) : sous le tableau des joueurs, au-dessus de la place de la mini-carte (bas à droite, D420).
- **V4-Q3 Alerte majeure** : **son camp seulement** (recommandation) : soi et ses alliés ont en plus le bandeau en haut,
  le son et le clic qui centre la caméra sur le forum ; les autres, la ligne du fil seule (« vision » lue comme « camp »
  jusqu'au brouillard).
- **V4-Q4 Mots** : **concrets et nommés** (recommandation) : « Le forum de Joueur 2 est tombé — 1 vie », « Joueur 1
  tient la ruine de Joueur 2 », « Fondations de Joueur 2 rasées — 0 vie », « Joueur 2 est vaincu » ; pour soi « Votre
  forum est tombé — 1 vie » ; noms à la couleur du joueur.
- **V4-Q5 Son** : **un son court à l'alerte** (recommandation), provisoire ; lignes du fil muettes ; héraut plus tard.

| Étape | Contenu | État |
|---|---|---|
| V4.1 Événements | Règles pures : événement (Fell, HoldStarted, Razed, Defeated ; emplacement, attaquant, vies) → texte et « alerte ou non » selon le camp du spectateur ; serveur qui émet les événements à tous (fiable, une fois chacun) ; logs `Message: …` ; test rouge d'abord | fait |
| V4.2 Fil à droite | Fil sous le tableau, ~8 s par ligne, noms à la couleur du joueur, `Describe` du HUD ; français seul | fait |
| V4.3 Alerte majeure | Bandeau bref en haut + son court + clic = caméra sur le forum, son camp seulement | fait |
| V4.4 Traductions, captures | fr / en / ja, captures hors écran (boucle à part) | fait |
| V4.5 Vérification | `quick_check` (solo, 2 clients), scénario, essai de l'utilisateur | **fait** : partie `victory` (solo : alerte, expiration, clic caméra, tenue annoncée une fois, chute adverse sans alerte, défaite sans alerte ; 2 clients : alerte chez le touché, ligne seule chez l'autre), scénario `forum_messages` (1+1 contre 1+1), correctifs V4.5c (pas d'alerte pour sa propre perte, alertes rapprochées en file, un seul son), essai de l'utilisateur **validé** (« works ») |
| V4.6 Clôture | GDD (V4-Q1 à Q5, prochain numéro libre), `EtapeDefini.md`, fusion | **fait** (2026-10-25) : commit `0389da9` (Arwen, `victory-v4`), `main` 48c8387 fusionnée sans conflit, `quick_check -Parts tests,menu,victory,net` **PASS** 142 s (153/153) ; GDD **D445 à D449** (prochain D450) ; `EtapeDefini.md` sans « messages (V4) » |

### Suivi de V4

- 2026-10-24 : questions V4-Q1 à Q5 tranchées. Worktree Arwen (libre), branche `victory-v4` depuis `main` 6167f5c, `IN_USE.txt` écrit, compilation PASS (37 s). V4.1 lancée (sous-agent, 15 min).
- 2026-10-24 : **V4.1 faite** (Arwen, 7 min, 3 compilations dont une ratée sur un alias PowerShell). Règles pures `Match/HOLForumMessages.h/.cpp` : `EHOLForumEventKind` (Fell, HoldStarted, Razed, Defeated), `FHOLForumEvent` (USTRUCT : Kind, Slot, OtherSlot, Lives), `HOLForumMessages::GetMessage(Event, Config, ViewerSlot)` → `FHOLForumMessage` (Text, bAlert = même camp que l'emplacement touché, jamais pour un spectateur ; NamedSlots dans l'ordre du texte pour les couleurs de V4.2 ; Shown `Message: … (alert)`). Clés (espace `HOLGameScreen`, français seul) : `MessageFell`, `MessageOwnFell`, `MessageHeld`, `MessageOwnHeld`, `MessageYouHold` (ajout : l'attaquant lit « Vous tenez la ruine de Joueur 1 »), `MessageRazed`, `MessageOwnRazed`, `MessageDefeated`, `MessageOwnDefeated` ; noms et vies réutilisent `You`, `PlayerN`, `TableLife`, `TableLives`. Serveur : `AHOLGameState::SendForumEvent` → `MulticastForumEvent` (NetMulticast, Reliable ; exécuté aussi en solo / écoute), log `Message: …` chez chaque récepteur (pas sur un serveur dédié ; spectateur = emplacement du PlayerState du premier contrôleur, INDEX_NONE sinon), puis délégué `OnForumEvent(const FHOLForumEvent&)`. Émis dans `HandleForumFall` (tout de suite, même depuis le combat : simple appel fiable mis en file), au rasage et à `DefeatSlot` (toute raison) ; HoldStarted seulement de None à Holding avec un attaquant connu (pas après contestation ni recul, pas au changement d'attaquant, pas si la tenue passe de None à Contested puis Holding). Tests `HeroesOfLiberty.Victory.Messages.*` (4) rouges 4/4 puis verts ; Victory + Localization 30/30 (Localization passe malgré les nouvelles clés).
- 2026-10-24 : **V4.2 faite** (Arwen, ~16 min en deux reprises après une coupure de quota, 5 compilations PASS). **Correctif de la tenue** : règle pure `HOLForumMessages::ShouldAnnounceHold(State, Announced)` (bit par attaquant annoncé, `AHOLGameState::ForumHoldAnnounced[MaxSlots]`, serveur seul, non répliqué) : HoldStarted quand la tenue est Holding par un attaquant pas encore annoncé depuis que le compte était à 0 (donc aussi None → Contested → Holding et un autre attaquant qui prend la tête ; rien pour le même après contestation ou recul) ; oublié à None / compte à 0, à la chute (`HandleForumFall`) et au rasage ; le message part après le log de la tenue. **Fil** : `FHOLForumThread` pur (5 lignes au plus, la plus récente en bas, 8 s par ligne, fondu sur les 0,5 dernières s), `HOLForumMessages::SplitNames` (texte coupé aux noms de `NamedSlots`). `UHOLGameScreen` : le tableau et le fil dans une colonne `RightColumn` alignée à droite (même place que le tableau avant : marge 16 px, sous « Menu »), le fil 6 px sous le tableau, lignes de 2 px d'écart, chacune son panneau `HUDPanel` 0,8 (comme le tableau), marge 8 × 2 px ; **noms à la couleur du joueur** par plusieurs blocs de texte sur la ligne (nom : style `HUDPlayerName` + couleur de son emplacement ; reste : `HUDLabel`) ; « Vous » non coloré (pas dans `NamedSlots`). Abonné à `OnForumEvent` dans `BindTableState` (tout poste : spectateur et vaincu compris), temps réel du monde. Muet. Rapport `hol.HUD.Report` : nouvelle ligne `HUD messages: [<texte> | <texte>]`. Tests `Victory.Messages.HoldAnnounced` et `Victory.Messages.Thread` rouges (3 échecs, puis plantage du bouchon) puis verts ; Victory + Localization + CommandGrid 43/43. Capture hors écran `Saved_Tests\Screenshots\WindowsEditor\HOL\v42_thread.png` (solo 1 contre IA, `hol.Victory.FallForum 1` puis `0`) : deux lignes sous le tableau, « Joueur 2 » en rouge ; un 1er essai montrait le nom en blanc (couleur posée avant l'ajout à la ligne, écrasée par le style à la construction du widget) : couleur posée après l'ajout. `HUD messages: […]` puis `[]` 9 s plus tard. Captures fr / en / ja et regard de l'utilisateur : V4.4.
- 2026-10-24 : **V4.3 faite** (Arwen, ~10 min, 3 compilations dont une ratée sur un paramètre `Slot` qui masquait `UUserWidget::Slot`). Règle pure `FHOLForumAlert` (`Match/HOLForumMessages.h/.cpp`) : `Show(Message, Slot, bMatchOverForViewer, Now)` seulement si `bAlert` et partie pas finie pour le joueur (bandeau Victoire / Défaite dû, `HOLGameMenuRules::GetEndBanner` ≠ None : la ligne du fil suffit), un nouveau remplace l'ancien (ses 4 s repartent), `ShowSeconds` 4 s, fondu sur les 0,5 dernières s, `Describe` `<texte> (slot N)` ou `none` ; `FHOLForumThreadLine::Slot` ajouté (emplacement touché). Son provisoire, constante unique `FHOLForumAlert::SoundPath` = `/Engine/EngineSounds/1kSineTonePing.1kSineTonePing` (bip court du moteur, chargé sans erreur), `UGameplayStatics::PlaySound2D`, alerte seule (fil muet). `UHOLGameScreen` : panneau `AlertPanel` en haut au centre, marge haute 16 + 72 = 88 px (place du bandeau d'attente R3.10, sous la barre des ressources), `HUDPanel` 0,85, marge 16 × 6 px, texte `HUDHeading` (18 gras, contre 13 pour le fil), noms colorés par `SplitNames` (aide commune `FillMessageRow` avec le fil). Clic gauche sur le bandeau, ou sur une ligne du fil qui porte une alerte (ces lignes seules deviennent cliquables), = `AHOLPlayerController::FocusCameraOnForum(Slot)` (nouveau : `FindPrimaryForum`, sinon le départ comme `hol.Camera.FocusSlot`) ; `hol.HUD.Click AlertPanel`. Log `Alert: <texte> (sound <chemin>)` ; rapport `hol.HUD.Report` : ligne `HUD alert: <texte> (slot N)` / `HUD alert: none`. Test `Victory.Messages.Alert` rouge (13 échecs, bouchons) puis vert ; Victory + Localization + CommandGrid 44/44. Captures hors écran (`Saved_Tests\v43_seq.txt`, config `v42_1v1.json`) `Saved_Tests\Screenshots\WindowsEditor\HOL\v43_noalert.png` (`FallForum 1` : fil seul, `HUD alert: none`) et `v43_alert.png` (`FallForum 0` : « Votre forum est tombé — 1 vie » centré sous la barre, lisible, sans recouvrir la barre, « Triches » ni le tableau) ; clic scripté → `Camera above the forum of slot 0`, `HUD alert: none` 5 s plus tard. Restes : le son n'est pas cuit tant qu'il n'est référencé que par chemin (à vérifier au paquet, ou le héraut le remplacera) ; traductions V4.4.
- 2026-10-24 : **V4.4 faite** (Arwen, ~7 min, aucune compilation). 9 clés en / ja dans `Tools/Localization/fill_po.py` (ordre des arguments vérifié dans `HOLForumMessages.cpp`) : `MessageFell` « {0}'s forum has fallen — {1} » / « {0}のフォーラムが陥落 — {1} », `MessageOwnFell` « Your forum has fallen — {0} » / « あなたのフォーラムが陥落 — {0} », `MessageHeld` « {0} holds {1}'s ruins » / « {0}が{1}の廃墟を占拠 », `MessageOwnHeld` « {0} holds your ruins » / « {0}があなたの廃墟を占拠 », `MessageYouHold` « You hold {0}'s ruins » / « {0}の廃墟を占拠中 », `MessageRazed` « {0}'s foundations razed — {1} » / « {0}の基礎が破壊された — {1} », `MessageOwnRazed` « Your foundations are razed — {0} » / « あなたの基礎が破壊された — {0} », `MessageDefeated` « {0} is defeated » / « {0}が敗退 », `MessageOwnDefeated` « You are defeated » / « あなたは敗退しました ». GatherText, `fill_po.py` (209 textes, 0 clé inconnue), GatherText : 200 → 209 entrées en / ja / fr, aucune traduction ni source existante changée (comparaison entrée par entrée des .po). Captures hors écran 1280×720 `v44_{a..d}_{fr,en,ja}.png` (dossier `Saved_Tests/Screenshots/WindowsEditor/HOL`, séquences `Saved_Tests/v44_seq_{fr,en,ja}.txt`, config `v42_1v1.json`, `-culture=`) : (a) `FallForum 1`, fil seul, nom adverse en rouge ; (b) `FallForum 0`, alerte « Your forum has fallen — 1 life » ; (c) `HoldPresence 0 1 0 1`, alerte et fil « Player 2 holds your ruins », anneau rouge ; (d) `Defeat 0`, « You are defeated » dans le fil, `HUD alert: none` sous le bandeau Défaite. Relues : japonais complet (pas de carrés), lignes anglaises courtes (la plus longue, « Player 2's forum has fallen — 1 life », dépasse un peu à gauche la largeur du tableau, sans gêne), aucun recouvrement (alerte entre la barre des ressources et le décor, fil sous le tableau). Défauts vus, hors V4.4 : « Triches + » non traduit (panneau de développement) ; « Vous » / « あなた » bleu foncé peu lisible sur le panneau sombre du tableau (déjà là avant V4). Victory + Localization 33/33.
- 2026-10-24 : **V4.5a : partie `victory` de `quick_check` étendue pour V4** (Arwen, ~8 min, `Tools/Tests/quick_check.ps1` seul, aucune compilation). Solo : nouvelle section `victory forum alert` après `fall` (clic `hol.HUD.Click AlertPanel` puis rapports à 4,5 s et 9 s) et `victory forum other` avant `defeat` (`hol.Victory.FallForum 1`). Contrôles : `fall` `Message: Votre forum est tombé — 1 vie (alert)` et `Alert: … (sound /Engine/…)` ; `alert` `HUD alert` `… (slot 0)` → `none` → `none`, `HUD messages` `[ligne]` → `[ligne]` → `[]`, `Camera above the forum of slot 0` ; `held` / `contested` / `released` un seul `Message: Joueur 2 tient votre ruine (alert)` ; `other` `Le forum de Joueur 2 est tombé — 1 vie` sans `(alert)`, aucun `Alert:`, `HUD alert: none` ; `defeat` `Message: Vous êtes vaincu`, ni `Alert: Vous êtes vaincu` ni autre `HUD alert` que `none`. 2 clients (fin de `net`) : client 1 exactement `Votre forum est tombé — 1 vie (alert)`, `Joueur N tient votre ruine (alert)`, `Vous êtes vaincu (alert)` ; client 2 exactement `Le forum de Joueur N est tombé — 1 vie`, `Vous tenez la ruine de Joueur N`, `Joueur N est vaincu` (noms `Joueur <emplacement+1>`, pas les noms de connexion) ; `HUD alert` du client 1 sur son emplacement, `none` chez le client 2 ; aucun `Alert: Vous êtes vaincu` chez le client 1. Rouge rejoué hors jeu (script à part, scratchpad) sur les logs d'avant V4 (2026-10-23) : les 14 nouveaux contrôles KO, les anciens OK. Jeu : `-Parts victory` PASS 44 s, `-Parts victory,net` PASS 142 s avant les deux derniers contrôles (avant V4 : 32 s et 126 s ; +12 s pour les 9 s d'attente de `alert`). **Deux défauts du jeu vus, laissés rouges** (rejoués sur les logs frais : `victory forum` FAIL et `net 2 clients` FAIL sur ces seuls contrôles) : (1) solo, la chute à la dernière vie montre l'alerte « Votre forum est tombé — 0 vies » (et le bip) sous le bandeau Défaite : l'événement Fell part avant `DefeatSlot`, `FHOLForumAlert::Show` ne sait pas encore la partie finie ; (2) 2 clients, l'abandon montre `Alert: Vous êtes vaincu` chez le client 1 : le multicast arrive avant la réplication de l'état de fin (l'OnRep du bandeau). Pistes : pas d'alerte pour Fell à 0 vie ni pour Defeated du spectateur lui-même, ou décider « fini pour le joueur » d'après l'événement. Non couvert : un spectateur sans emplacement (aucun dans `net`, ajout d'un 3e client trop long : règle couverte par le test `Victory.Messages`), le rasage (60 s réelles, `Hold.Chain`).

- 2026-10-24 : **scénario `forum_messages` prêt** (V4.5b, ~8 min, Arwen, aucune compilation) : `Tools/Scenarios/forum_messages.txt` / `.json` (Plain, 4 joueurs : moi slot 0 + Joueur 3 IA alliée slot 2 en équipe 1 ; Joueur 2 slot 1 et Joueur 4 slot 3, IA adverses, en équipe 2). Le panneau du scénario recouvre la place du bandeau d'alerte (tous deux en haut au centre, panneau dessiné par-dessus, vu en capture) : tout le déroulé est donné en notes au départ, puis l'utilisateur ferme le panneau (×) ; la touche Entrée marche encore (6 temps). Temps : (1) `FallForum 1` → « Le forum de Joueur 2 est tombé — 1 vie », fil seul ; (2) 4 Piquiers de Joueur 2 frappent le forum, `FallForum 0` → bandeau + son « Votre forum est tombé — 1 vie », puis « Joueur 2 tient votre ruine » (bandeau + son), Piquiers retirés 4 s après (`hol.Units.Remove 0 0 1000000 1`, sinon rasé à 60 s) ; (3) caméra éloignée par l'utilisateur, `HoldPresence 0 1 0 3` 6 s → « Joueur 4 tient votre ruine », clic sur le bandeau → caméra sur son forum ; (4) caméra éloignée, `FallForum 2` → « Le forum de Joueur 3 est tombé — 1 vie » + bandeau + son (allié), clic → caméra sur le forum de l'allié ; (5) `Defeat 1` → « Joueur 2 est vaincu », fil seul ; (6) `Defeat 0` → « Vous êtes vaincu » dans le fil, bandeau Défaite, pas d'alerte (`Message: Vous êtes vaincu (alert)` mais aucun `Alert:`). Répétition hors écran (copie `_dry` avec captures et clics `hol.HUD.Click ScenarioClose` / `AlertPanel`, `-saveddirsuffix=V45m`, avec son, `hol.Camera.TestCursor 0.5 0.5` contre la caméra qui glisse) : 7 `Message:` dans l'ordre, 4 `Alert:` (soi ×3, allié ×1 ; aucun pour Joueur 2), `Camera above the forum of slot 0` puis `of slot 2` après les clics, `End banner: Defeat`, scénario `ready`. Captures `v45n_*` relues (scratchpad `v45m_captures`) : bandeau lisible, noms colorés (Joueur 2 rouge, Joueur 3 vert, Joueur 4 jaune), fil sous le tableau, caméra sur la ruine verte de l'allié après le clic. Copies `_dry` et `Saved_V45m` retirées. **Essai demandé à l'utilisateur** : `play.ps1 forum_messages` depuis Arwen ; regarder : le son à chaque bandeau (et jamais pour un adversaire), le clic sur le bandeau dans ses 4 s (ou sur sa ligne du fil, 8 s), la lisibilité du fil et des couleurs. Points : panneau du scénario sur la place de l'alerte (à corriger côté interface si gênant : déplacer l'un des deux) ; deux bandeaux au temps 2 à quelques ms d'écart (le second remplace le premier aussitôt : « Votre forum est tombé » ne se voit presque pas, il reste dans le fil) ; « Vous » bleu foncé peu lisible au tableau (déjà noté).
- 2026-10-24 : **V4.5c : deux défauts des alertes corrigés** (Arwen, ~7 min, 2 compilations PASS 33 s et 19 s, la première sur une bascule temporaire pour le rouge). (1) **Pas d'alerte pour sa propre perte** : dans `HOLForumMessages::GetMessage` (`Source/HeroesOfLiberty/Match/HOLForumMessages.cpp`), `bAlert` faux quand le spectateur est l'emplacement touché et que l'événement le fait perdre (Defeated de soi ; Fell ou Razed de soi à 0 vie) : ligne du fil seule, log `Message:` sans `(alert)` ; pour un allié l'alerte reste. Règle pure, indépendante de l'ordre multicast / réplication de l'état de fin. (2) **Alertes rapprochées** : `FHOLForumAlert` (pur) garde un bandeau au moins `MinSeconds` 1,5 s ; une alerte arrivée avant attend dans `Waiting` (au plus `MaxWaiting` 2, la plus ancienne en trop jetée, son son éventuel reporté sur la suivante), puis s'affiche ses 4 s (`Advance`) ; `Expire` ne cache pas tant qu'une attend ; un seul son pour des alertes arrivées à moins de `SoundGapSeconds` 1 s l'une de l'autre (`bSound`, joué à l'affichage) ; la file vidée une fois la partie finie pour le joueur. HUD (`Source/HeroesOfLibertyUI/HUD/HOLGameScreen.cpp`) : `ShowAlert` → `Alert.Show`, `TickAlert` → `Alert.Advance`, `PresentAlert` remplit le bandeau et joue le son si `bSound` (sinon log `Alert: … (no sound, its burst's played)`) ; `HUD alert:` inchangé. Tests `Source/HeroesOfLiberty/Tests/HOLForumMessagesTests.cpp` : Fell / Razed / Defeated de soi à 0 vie sans alerte (allié à 0 vie : alerte), rafale chute + tenue à 40 ms, rafale de 4 (2 en attente, plus ancienne jetée, un son), alerte plus tardive avec son, file vidée à la fin. Rouge 40 OK / 4 KO (`Messages.Alert`, `.Defeated`, `.Fell`, `.Razed`), vert 44 / 44 (`Victory` + `Localization` + `CommandGrid`). `Tools/Tests/quick_check.ps1` : `victory forum defeat` exige `Message: Vous êtes vaincu` et `Message: Votre forum est tombé — 0 vies` sans `(alert)` et aucun `Alert: ` (resserré : ni la défaite ni la dernière chute) ; `net` client 1 attend `Vous êtes vaincu` sans `(alert)`, toujours aucun `Alert: Vous êtes vaincu`. `-Parts victory,net` **PASS en 139 s** (140 s mur ; avant 142 s). Scénario `Tools/Scenarios/forum_messages.txt` : textes seuls ajustés (temps 2 : la tenue 1,5 s après la chute, sans second bip ; temps 6 : sa propre perte n'est jamais une alerte), pas de répétition. Reste : l'essai du scénario par l'utilisateur (temps 2 à revoir à l'œil et à l'oreille) ; panneau du scénario sur le bandeau laissé tel quel ; aucun commit.
- 2026-10-24 : **prêt pour l'essai de l'utilisateur** : `Tools\Scenarios\play.ps1 forum_messages` depuis Arwen. Ensuite : commit après validation, fusion de `main`, V4.6.
- 2026-10-25 : **V4 validée** par l'utilisateur (« works »). V4.6 : commit `0389da9`, `main` 48c8387 fusionnée, `-Parts tests,menu,victory,net` PASS 142 s ; GDD D445 à D449 ; `EtapeDefini.md` à jour. Accord de mycrusader-b8 pour avancer `main`.

## V5 — Régicide (questions préparées le 2026-10-25, à poser une à une)

Choix de l'utilisateur (2026-10-25) : **V5 Régicide** avant Nomade. Déjà fixé : D206 (Roi façon AoE2 : né près du
centre principal, hors population, attaque 1, sa mort fait perdre ; ce n'est pas le héros ; jamais converti, retourné
ni relevé), D355 (Roi mort **ou** forum à 0 vie = défaite), D365 (garnison dans tout bâtiment à garnison : **plus tard,
avec la garnison**), D207 (en Nomade, le Roi naît avec un travailleur : avec Nomade). Code : option `bRegicide` au menu
seulement ; unités par `Tools/Data/UnitTypes.csv` (Paysan 50 PV vitesse 800, Homme d'armes 155 PV vitesse 697) ;
défaite par `AHOLGameState::DefeatSlot` (raison à ajouter) ; messages de V4 (fil, alerte).

- **V5-Q1 Le Roi en chiffres** : A **fragile et vif** (~75 PV, vitesse d'un Paysan, armure légère : un raid peut le
  tuer, il doit fuir) — recommandé, comme AoE2 ; B robuste et lent (~200 PV) ; C comme un Homme d'armes (155 PV).
  → **Réponse de l'utilisateur (2026-10-25)** : ni A, ni B, ni C : **vitesse d'un Lancier** (777), **~250 PV**.
- **V5-Q2 Ses ordres** : A **déplacement, arrêt, attaque sur ordre** (attaque 1), **ne riposte pas et n'attaque pas
  d'office**, ni récolte ni construction — recommandé ; B comme un soldat (riposte, attaque d'office) ; C déplacement seul.
  → **Réponse (2026-10-25)** : **A** (recommandation) : déplacer, arrêter, attaquer sur ordre ; ni riposte ni attaque d'office ; ni récolte ni construction.
- **V5-Q3 Son aspect** : A **mannequin + couronne** (pièce provisoire à sa couleur, comme les accessoires de récolte)
  — recommandé ; B mannequin plus grand ; C même mannequin, seul le nom change.
  → **Réponse (2026-10-25)** : **A** (recommandation) : mannequin + couronne provisoire à sa couleur.
  → **Nuance de l'utilisateur (révise D206)** : le Roi a **50 d'attaque** (au lieu de 1), « donc peut se défendre », mais **ne peut pas être soigné** (aucun soin ne le vise : Moine, auras, etc., quand ils existeront). Conséquence sur V5-Q2 à reposer : riposte ou non.
  → **V5-Q2 révisée (2026-10-25)** : **A** (recommandation) : il **riposte quand on le frappe**, ne prend jamais d'office une cible (pas de rayon d'agressivité), ni récolte ni construction ; cadence et portée d'un Homme d'armes par défaut (1,375 s, mêlée).
- **V5-Q4 Sa mort à l'écran** : A **message « Le Roi de Joueur 2 est mort »** (fil pour tous, alerte pour son camp sauf
  soi, D447), puis la défaite habituelle — recommandé ; B défaite seule (message « vaincu » de V4) ; C A + son Roi
  marqué au tableau des joueurs (couronne près du nom).
  → **V5-Q4 (2026-10-25)** : **A** (recommandation) : message « Le Roi de Joueur 2 est mort » (fil pour tous, alerte
  pour son camp sauf soi, D447), puis la défaite habituelle.
  → **Nuance de l'utilisateur** : le Roi **ne peut pas attaquer de bâtiment** (pas d'attaque contre bâtiment, D345).

## Plan de V5 — Régicide (tranché le 2026-10-25)

Le Roi : **250 PV**, **vitesse d'un Lancier** (777), **50 d'attaque** (cadence et portée d'un Homme d'armes, mêlée),
armure légère (0), **hors population**, **jamais soigné**, **n'attaque pas les bâtiments** ; **riposte** quand on le
frappe, ne prend **jamais** d'office une cible ; ni récolte ni construction ; ordres : déplacer, arrêter, attaquer
(unités seulement). Aspect : mannequin + **couronne** provisoire à sa couleur. Né près du forum principal au départ, pour
chaque emplacement (IA comprise), seulement avec l'option Régicide. Mort = message puis défaite (D355 : en plus du
forum à 0 vie). Révise D206 (attaque 1 → 50, soins, bâtiments). Garnison (D365) et Nomade (D207) plus tard.

| Étape | Contenu | État |
|---|---|---|
| V5.1 Le Roi | Type d'unité (table, chiffres ci-dessus, hors population, drapeau « jamais soigné », pas d'attaque contre bâtiment, riposte sans agressivité, ni récolte ni construction) ; naissance au départ avec `bRegicide` ; test rouge d'abord | fait (2026-10-25, Arwen, sans commit) |
| V5.2 Mort = défaite | Raison `King` de `DefeatSlot` (différée à la fin du pas : la mort vient du combat), événement `KingDied` et ses textes (fil, alerte), test | fait |
| V5.3 Aspect | Couronne provisoire à sa couleur, nom « Roi » dans la sélection, grille de commandes réduite | fait (2026-10-25, Arwen, sans commit) |
| V5.4 Traductions, captures | fr / en / ja, captures hors écran | fait (2026-10-25, Arwen, sans commit) |
| V5.5 Vérification | `quick_check` (solo, 2 clients), scénario, essai | **fait** : partie `victory regicide` (solo) et mort du Roi à 2 clients (à la place de l'abandon), scénario `regicide`, essai de l'utilisateur **validé** (« tout fonctionne ») |
| V5.6 Clôture | GDD (D450+), `EtapeDefini.md`, `UnitStats.md`, fusion | **fait** (2026-10-25) : commit `a03d4c8`, `main` 86bf2c5 fusionnée (`b9a7e06` : conflit `HOLCombatProcessor.cpp` réglé en gardant les deux changements, traductions reprises de `main` puis régénérées : 311 → 314, aucune changée) ; `-Parts tests,menu,victory,net,lobby` : tout PASS (171/171) sauf `lobby` (pas de serveur empaqueté dans Arwen, hors V5) ; GDD **D470 à D474** (prochain D475) ; `EtapeDefini.md` à jour |

### Suivi de V5

- 2026-10-25 : questions V5-Q1 à Q4 tranchées (avec trois nuances de l'utilisateur : 250 PV / vitesse de Lancier, 50
  d'attaque sans soins, pas d'attaque contre bâtiment).
- 2026-10-25 : **V5.1 Le Roi** fait dans Arwen (`victory-v5`, sans commit). Type `King` dans `Tools/Data/UnitTypes.csv`
  (250 PV, 777, 50 d'attaque mêlée, 1,375 s, portée 70, armure 0, population 0, coût 0, rayon d'agressivité 0, attaque
  contre bâtiment 0, mannequin de l'Homme d'armes) et table `/Game/Units/UnitTypes` réimportée ; nouvelle colonne `Ripostes`
  (vrai pour les soldats, faux pour le Paysan, lue seulement sans rayon d'agressivité) ; catégorie d'armure `King` = « jamais
  soigné » (`HOLCanBeHealed`, à lire par les futurs soins). Riposte sans agressivité : `FSeeker::bRipostes`, `ChooseTarget`
  ne garde alors qu'un ennemi qui l'attaque (jamais un bâtiment), le processeur de combat lui donne sa cible et son dernier
  agresseur comme seuls candidats. Bâtiments : recherche de bâtiment nulle (attaque 0) et ordre sur un bâtiment refusé
  pour une unité sans attaque contre bâtiment (`HOLUnitOrderProcessor`). Naissance : `FHOLStartRules::ShouldSpawnKing` /
  `GetKingLocation`, dans `SpawnMainCentres` (log `Regicide: king of slot N spawned at (x, y)`), triche
  `hol.Victory.SpawnKing <slot>` ; nom « Roi » (`HOLGameNames`, français seul). Tests `HeroesOfLiberty.Victory.Regicide.*`
  (KingType, Riposte, Birth) : rouges (pas de King dans la table), puis 106 tests verts (Victory, Combat, Production, Match,
  Buildings, Construction, Animation, Localization, Orders, Gather, CommandGrid). Reste : `UnitStats.md` à régénérer dans
  `main` à la clôture (V5.6), essai en partie (V5.5).
- 2026-10-25 : **V5.2 Mort du Roi = défaite** fait dans Arwen (`victory-v5`, sans commit). Détection : au début de la
  mort d'une unité (`UHOLCombatProcessor::KillUnit`, chemin commun aux coups, aux flèches et aux unités mises à 0 PV
  autrement, via `FinishDying`), si sa catégorie d'armure porte `King` (`HOLIsKing`), `AHOLGameState::HandleKingDeath(Slot)` :
  refusé pour un emplacement déjà vaincu, déjà compté, hors partie ou partie finie (l'effondrement de D358 tue le Roi d'un
  vaincu sans rien redonner) ; sinon log `Regicide: king of slot N died`, événement `KingDied` envoyé à tous tout de suite,
  et défaite `EHOLDefeatReason::King` à la fin du pas (`PendingKingDefeats`, appliquée par `ApplyPendingDefeats` après
  celles du forum), d'où `Victory: slot N defeated (King)` puis le « … est vaincu » de V4. Un emplacement sans Roi ne perd
  jamais par cette voie. Textes (`Match/HOLForumMessages.cpp`, français seul) : « Le Roi de {0} est mort » / « Votre Roi est
  mort » ; alerte pour son camp, jamais pour soi (perte propre, comme `Defeated`). Triche `hol.Victory.KillKing <slot>` :
  `UHOLUnitSubsystem::KillUnitsOfOwner(Slot, TypeName)` (nouveau filtre de type) met le Roi à 0 PV, puis le chemin normal.
  Tests `HeroesOfLiberty.Victory.Regicide.Death` (défaite à la fin du pas, deux Rois de deux emplacements, rien après
  l'effondrement, fin de partie) et `.Message` (soi, allié, adversaire, spectateur) : rouges (ne compilaient pas : ni
  `KingDied`, ni `King`, ni `HandleKingDeath`), puis 47 tests verts (Victory, Combat, Localization). Reste : la triche et le
  chemin du combat non essayés en partie (`quick_check`, scénario de V5.5).
- 2026-10-25 : **V5.3 Aspect et commandes du Roi** fait dans Arwen (`victory-v5`, sans commit). Couronne : pièce `crown`
  (id 7) de `Tools/Editor/soldier_props.py`, sur l'os `head` (repère « body », bas du bandeau à 177 cm, sur le front) :
  bandeau cylindrique fermé dessus, rayon 13 cm, 8 cm de haut, à la couleur du joueur, entre deux filets dorés (laiton),
  et cinq créneaux de 4,5 x 4,5 x 6 cm à sa couleur ; 28 x 28 x 14 cm dans le maillage, environ 42 cm en jeu (échelle 1,5) ;
  120 triangles (légère : 8 côtés). Nouveau maillage `/Game/Units/Mannequin/SKM_King` (mannequin + épée + bouclier +
  couronne) et fournisseur `TP_King` (les gestes de l'Homme d'armes), faits par `Tools/Editor/make_peasant_mesh.py
  --soldiers --only=SKM_King` ; ligne `King` de `Tools/Data/UnitTypes.csv` dessus, table réimportée
  (`make_unit_types.py`). Visible chez tous : c'est le maillage du type, pas un état. Commandes : `FHOLUnitView::bKing`
  (catégorie `King`, serveur et client), `FHOLGridContext::bOnlyKings` (`GetUnitsContext`) : des Rois seuls n'ont que
  l'arrêt (S) ; ni attaque-déplacement (Q), ni position tenue (V, qui prendrait les ennemis à portée) ; le déplacement et
  l'attaque d'une unité restent au clic droit. Sélection mixte : les commandes des autres restent ; côté serveur, une
  attaque-déplacement donne au Roi un simple déplacement (`HOLUnitOrderProcessor`, mode `Normal` pour une unité qui ne
  fait que riposter) et la règle pure `HOLCombat::GetSearchRadius` ne lui donne plus `AttackMoveRadius` ; en position
  tenue (groupe mixte), `ChooseTarget` ne lui garde qu'un ennemi qui l'attaque, à portée. Clic droit sur un bâtiment
  ennemi : refusé pour lui depuis V5.1 (pas d'ordre, il ne bouge pas, rien ne le plante). Test
  `HeroesOfLiberty.Victory.Regicide.Commands` : rouge (5 échecs : Q et V offerts, rayon 1600 en attaque-déplacement,
  ennemi pris en passant et en position tenue), puis vert ; 61 tests verts (Victory, Combat, CommandGrid, Orders,
  Localization). Capture hors écran (Plain, `hol.Victory.SpawnKing 0` et `1`, `Saved_Tests/v53_king.seq`) :
  `Saved_Tests/Screenshots/WindowsEditor/HOL/v53_king.png` (hauteur de départ), `v53_king_close.png` (15 m, bleu),
  `v53_king_enemy.png` (rouge) ; rapport `selection King (own), hp 250/250, grid hidden, orders S=Stop`, bloc « Roi »,
  « Vous », « Points de vie 250 / 250 », rien de récolte. Reste : à la hauteur de départ la couronne se lit mal (une
  tache) ; à 15 m, filets dorés et créneaux lisibles : à grossir ou à dorer entièrement si l'utilisateur le veut ; le
  bloc de sélection n'a de ligne d'attaque pour aucune unité (seulement l'activité « Attaque ») ; traductions en / ja en
  V5.4.
- 2026-10-25 : **V5.3, couronne refaite** sur le retour de l'utilisateur (« elle est grosse et flotte au dessus du roi »).
  Couronne flottante : la même pièce `crown` (id 7, `Tools/Editor/soldier_props.py`), trois fois plus grande (bandeau
  rayon 39 cm, 24 cm de haut, fermé dessus ; deux filets plus sombres ; cinq créneaux de 13,5 x 13,5 x 18 cm), 84 x 85 x 44 cm
  dans le maillage, environ 1,3 m en jeu (échelle 1,5) ; bas du bandeau à 245 cm du sol dans le maillage (63 cm au-dessus
  de la tête, environ 95 cm en jeu) ; **entièrement dorée** (or 222/178/52, filets laiton), plus rien à la couleur du
  joueur (le corps la porte). Os : `pelvis` (repère « body ») : elle suit la marche et le combat, et **tombe avec lui** à la
  mort (elle bascule près du corps puis disparaît avec lui) ; la racine aurait été plus stable mais l'aurait laissée
  flotter 30 s au-dessus du corps (`HOLCorpse::HoldSeconds`) ; une pièce de présentation à part était plus lourde pour le
  même effet. `SKM_King` / `TP_King` refaits (`make_peasant_mesh.py --soldiers --only=SKM_King`). Captures hors écran
  (`Saved_Tests/v53_king.seq`, `Saved_Tests/v53_king_death.seq`) dans `Saved_Tests/Screenshots/WindowsEditor/HOL/` :
  `v53_king.png` (hauteur de départ : une couronne d'or nette au-dessus du Roi), `v53_king_close.png` (15 m),
  `v53_king_walk.png` (en marche : elle suit, penchée avec son bassin), `v53_king_falling.png` et `v53_king_dead.png`
  (elle bascule et retombe près du corps). Tests Victory + Animation : 42 verts, aucun échec. Reste : en marche elle se penche
  avec le bassin (ballant d'environ 20 à 30 cm visible) ; la barre de vie, au-dessus de la tête, est en partie cachée par
  la couronne de près ; l'écran « Défaite » cache le corps sur les captures de mort.
- 2026-10-25 : retour de l'utilisateur sur la couronne (captures v53) : « elle est grosse et flotte au dessus du roi » → couronne **flottante au-dessus de la tête**, grosse, lisible à la hauteur de jeu ; par défaut dorée (corps déjà à la couleur du joueur), ~3× la taille actuelle, ~1 m au-dessus de la tête ; remplace la couronne posée. À faire après la mesure de charge de mycrusader-b8 (pause demandée, plus rien ne tourne).
- 2026-10-25 : info de mycrusader-b8 : l'utilisateur a choisi « 町の中心 » pour « Forum » en japonais ; b8 a changé `MessageFell` / `MessageOwnFell` (ja) dans `fill_po.py`, à venir dans `main` avec sa fusion de `net-playtest`. **V5.4** : tout texte japonais qui parle du forum prend « 町の中心 » ; recoupement attendu sur `fill_po.py` à la fusion de V5.
- 2026-10-25 : **V5.4 Traductions et captures** faite (Arwen, `victory-v5`, sans commit ; ~7 min avant une coupure de quota, puis ~5 min ; 1 compilation PASS pour un commentaire de `HOLGameNames.cpp`). 3 clés en / ja dans `Tools/Localization/fill_po.py` (ordre des arguments vérifié dans `Match/HOLForumMessages.cpp`, un seul argument : le nom) : `HOLGameNames,King` « King » / « 王 », `HOLGameScreen,MessageKingDied` « {0}'s King is dead » / « {0}の王が死亡 », `HOLGameScreen,MessageOwnKingDied` « Your King is dead » / « あなたの王が死亡 » ; aucune ne parle du forum. La raison de défaite `King` n'est pas affichée (`GetReasonName`, `TEXT` de journal) : rien à traduire. GatherText, `fill_po.py` (212 textes, 0 clé inconnue), GatherText : 209 → 212 entrées en / ja / fr, aucune traduction ni source existante changée, aucune retirée (comparaison entrée par entrée des .po ; seul l'ordre des entrées bouge). Captures hors écran 1280×720 dans `Saved_Tests/Screenshots/WindowsEditor/HOL/` (séquences `Saved_Tests/v54_run1_{fr,en,ja}.seq` et `v54_run2_{fr,en,ja}.seq`, config `v42_1v1.json`, `-culture=`) : `v54_a_{fr,en,ja}.png` (Roi sélectionné à la hauteur de départ : bloc « Roi » / « King » / « 王 », grille « Arrêt » / « Stop » / « 停止 » seule, couronne d'or), `v54_enemy.png` (Roi rouge sous le forum adverse, couronne d'or), `v54_b_{fr,en,ja}.png` (`KillKing 1` : fil « Le Roi de Joueur 2 est mort » puis « Joueur 2 est vaincu », nom en rouge, `HUD alert: none`, bandeau Victoire), `v54_c_{fr,en,ja}.png` (`KillKing 0` : fil « Votre Roi est mort » puis « Vous êtes vaincu », `HUD alert: none`, bandeau Défaite, corps et couronne tombée visibles). Relues : japonais complet (pas de carrés), lignes courtes (la plus longue « Player 2's King is dead »), aucun recouvrement. Défauts vus : à la première passe anglaise, la caméra n'était pas sur sa base (`FocusSlot own` sans effet visible, Roi hors champ) ; relancée, elle l'était : aléa non expliqué des captures ; « Triches + » non traduit et « Vous » bleu foncé peu lisible (déjà vus en V4.4) ; sur le Roi rouge, la sélection affichée reste celle de son propre Roi (normal). Victory + Localization : 39/39 verts, aucun échec. Note : la traduction ja de « Forum » (« 町の中心 », mycrusader-b8) n'est pas encore dans Arwen : elle viendra avec `main`.
- 2026-10-25 : **V5.5a `quick_check` du Roi** fait dans Arwen (`victory-v5`, sans commit, `Tools/Tests/quick_check.ps1` seul). Solo : 4e partie de la partie `victory` (marques `victory regicide`, `… pass`, `… riposte`, `… death`), lancée **avec l'option** (`"bRegicide": true` dans `Saved_Tests/victory_regicide.json`, donc par le vrai chemin de naissance, pas `SpawnKing`) : les deux Rois nés (log `spawned at (-15935, 0)` et `(14065, 0)`) ; Ctrl + A puis `hol.HUD.Report` : `selection King (own), hp 250/250, grid hidden, orders S=Stop`, `population 0/10` (hors population) ; combat allumé : un Paysan ennemi posé à 5,5 m pendant 4 s, aucun coup du Roi ; un Homme d'armes ennemi au même endroit : il frappe (12, 238 PV), le Roi riposte (3 coups de 46 en 5 s) ; `hol.Victory.KillKing 1` : `Regicide: king of slot 1 died`, messages exactement `Le Roi de Joueur 2 est mort|Joueur 2 est vaincu` sans `(alert)`, `HUD alert: none`, `Victory: slot 1 defeated (King)`, `match over, winners 0`, `End banner: Victory`, mon Roi vivant. 2 clients (fin de la partie `net`) : **l'abandon de V1.4c remplacé par la mort du Roi** (une seconde partie à 2 clients coûtait trop ; le Roi avant l'abandon impossible : sa mort finit la partie à 1 contre 1) ; les contrôles de V2-V4 gardés (chute du forum, prise, messages, alerte, effondrement, bandeaux, unités qui partent, gestionnaire) ; l'abandon reste couvert en solo (Échap, menu, « Abandonner »). Après la prise du forum, client 1 le demande, le serveur fait `SpawnKing 0` et `SpawnKing 1` (`quick_net_server.seq`), chaque client montre `King of slot 0 x1, King of slot 1 x1` (`hol.Units.Report`), puis le serveur fait `KillKing 1` (client 1, Bob, emplacement 1) : serveur `defeated (King)` (et plus `Surrender`), client 1 `…|Votre Roi est mort|Vous êtes vaincu` et `Defeat`, client 2 `…|Le Roi de Joueur 2 est mort|Joueur 2 est vaincu` et `Victory` ; 7 rapports d'unités chez client 2 (au lieu de 6). Méthode : rouge rejoué hors jeu sur le `quick_game.log` d'avant (13 contrôles KO sur 14), puis vert : `-Parts victory` PASS en 116 s (la partie Régicide ~12 s ; lancement lent, machine chargée par l'autre agent), `-Parts victory,net` PASS en 140 s. Défauts vus : aucun nouveau ; le rapport du HUD écrit l'activité en `EHOLUnitActivity::Attacking` (format brut, déjà dans la partie `net`) et le Roi reste « Attacking » juste après le retrait de son agresseur et sous le bandeau (activité non remise à zéro, à surveiller) ; chez client 2, le Roi mort est encore montré au premier rapport après le bandeau (comme les autres unités, avant l'effondrement). Reste : V5.5 scénario et essai en partie, l'abandon en réseau (`ServerSurrender`) n'est plus joué par `quick_check`.
- 2026-10-25 : **V5.5b scénario `regicide` prêt** (Arwen, ~10 min, aucune compilation) : `Tools/Scenarios/regicide.txt` / `.json` (Plain, 1 contre 1 IA, `"bRegicide": true`, lu par `FJsonObjectConverter` : chaque camp n'a que son Roi, le mien né à (-15935, 0), derrière le forum). Déroulé (~2 min, Entrée entre les temps) : (1) Roi sélectionné → bloc « Roi », grille « Arrêt » seule, l'utilisateur le déplace ; (2) **simplifié** : le Roi ramené à sa place (`hol.Units.Move -15935 0 0`), 3 Paysans ennemis nés à 3-5 m de lui, 8 s, il ne bouge pas ni ne frappe, puis retirés (les faire passer était fragile : `hol.Units.Move` d'un emplacement emmène aussi son Roi, à 300 m) ; (3) un Lancier ennemi (rayon d'agressivité 16 m) le frappe : riposte, `Spearman of slot 1 killed by #1 King of slot 0 (2 hits struck)`, Roi à 242 puis 234 / 250 ; (4) 4 Hommes d'armes à moi à côté du Roi, 4 Lanciers ennemis ~32 m plus loin, une Maison ennemie à gauche du forum (`hol.Cheat.SpawnBuilding House 1 -1800 -1600`) : l'utilisateur fait Q avec le groupe (cadre) au-delà des Lanciers, puis clic droit du Roi seul sur la Maison ; (5) `hol.Victory.KillKing 1` → fil « Le Roi de Joueur 2 est mort », « Joueur 2 est vaincu », Victoire. Répétition hors écran deux fois (copie `_dry` avec captures et ordres scriptés : `hol.Select.ClickUnit 0`, `hol.Select.Box`, `hol.Select.AttackMove -15935 5500`, `hol.Select.OrderNear -1800 -1600` ; `-saveddirsuffix=V55r`, `hol.Camera.TestCursor 0.5 0.5`, `hol.Scenario.AutoContinue 1`) : `Regicide: king of slot 0/1 spawned`, aucune frappe au temps 2, riposte au temps 3, `Order: 5 units attack-move` puis les seuls Hommes d'armes tuent les 4 Lanciers (le Roi traverse la mêlée et arrive à (-16001, 5325) sans aucun coup porté), `Order: 1 units attack House of slot 1 (other)` → `Move order of slot 0: 1 units ignored, not its own` (refusé, il ne bouge pas), puis `Regicide: king of slot 1 died`, `Message: Le Roi de Joueur 2 est mort`, `Victory: slot 1 defeated (King)`, `Message: Joueur 2 est vaincu`, `End banner: Victory`, `HUD alert: none`, scénario `ready` (46 s). Captures `v55_*` relues (scratchpad `v55r_captures`, 2e passe dans `run2`) ; la caméra avancée de 8 m à la 2e passe pour que le panneau du scénario, qui grandit, laisse voir le Roi et la mêlée. Copies `_dry` et `Saved_V55r` retirées. **Essai demandé à l'utilisateur** : `Tools\Scenarios\play.ps1 regicide` depuis Arwen ; regarder : la couronne et la grille « Arrêt » seule (temps 1), le Roi immobile à côté des Paysans (2), la riposte sans poursuite (3), le Roi qui marche sans frapper pendant que ses soldats combattent, et le refus sur la Maison (4), le fil et la Victoire (5). Points : le log du refus dit « not its own » (texte commun aux refus, trompeur pour le Roi) ; au temps 4 un Lancier ennemi part seul vers la gauche avec sa ligne d'ordre puis revient mourir (vu aux deux passes, sans gêne) ; si les Lanciers frappent le Roi en passant, il riposte (attendu) ; au temps 5 la mort du Roi adverse est par triche (il est à 300 m).
- 2026-10-25 : **V5 validée** par l'utilisateur (« tout fonctionne », essai `regicide`). V5.6 : voir le tableau. **V5 close.** Suite du chantier : vies par palier, Nomade, garnison du Roi (chantiers à venir).
