# ETAPE0-1119n-B : plan de lancement (points 3, 4, 5 + Aube A2), lecture seule, 2026-11-19 ~21:20

Tout relu dans main (le relevé du 2026-11-16 est périmé sur plusieurs points). NON VÉRIFIÉ en jeu ni par exécution : lecture du code seulement.

## Partie 1 : worktrees
- **Celeborn** (`test-world-11`, HEAD d02ae9f) : `git diff --stat` VIDE. Les « 16 fichiers » sont 16 fichiers NON SUIVIS `Tools/Scenarios/c28_b20|c28_w20|c31_f3|f4|f5|f8` (.json/.txt/.look), datés du 2026-11-10 : scénarios d'essai PT7 (« essai PT7, questions 1 et 7 de GDD 17.1 », PlaytestInProgress.md) jamais ajoutés. Aucun fichier modifié. Pas de travail perdu à commiter d'urgence ; à ajouter ou à jeter par l'utilisateur (ne pas les toucher). Peut recevoir un agent : des fichiers non suivis ne gênent ni édition ni commit (ne pas faire `git add -A`).
- **Haldir** (`forge-lines`, HEAD d02ae9f) : `Content/Maps/Crown.umap` NON SUIVI (1,1 Mo, 2026-11-07), pas de diff. C'est une carte générée par l'éditeur (Crown), pas un reste de code. Peut recevoir un agent (script) ; ne pas faire `git add -A`.
- Les deux HEAD (d02ae9f) sont plus récents que le 2ff2dbf du début de séance : un commit docs « step 0a cleanup » de 21:16 est passé ; vérifier que main est bien à d02ae9f avant de lancer.

## Partie 2 : état réel par point

### Point 3, moral (M3 à M5)
- Fait en script : M1 (`Script/Morale/HOLMoraleRules.as`, 17 tests `Test_Morale*` dans `Script/Tests/HOLMoraleRules_Test.as`, y compris `Test_MoraleAttenuationSources`, `Test_MoraleImmunityBlocksMalusOnly`, `Test_MoraleRemovalLeavesBonus` = la règle de M8 est DÉJÀ écrite et testée), M2 (`Tools/Data/Effects.csv`, 7 lignes dont NecroticRage ; `Script/Morale/HOLMoraleTable.as` généré par `Tools/Data/make_effects.py` : NE PAS éditer à la main), M6 (`Script/UI/HOLMoraleBlock.as`, 8 tests). Aucun code moral dans `Source/` (aucun fichier `*Moral*`).
- Reste = uniquement C++ : M3 (copie de la règle + `FHOLMoraleFragment`, test d'accord script/C++), M4 (processeur Mass : aura, cri, bannière, `HOLCombatProcessor.cpp`), M5 (masque réseau `FHOLNetUnitState`), M7/M8 (pont `HOLMoraleLibrary`, points d'entrée Aube), M9 (`hol.Morale.*`).
- Part de règle pure à écrire en script : AUCUNE de réelle (le masque M5 est un champ réseau C++ ; le recopier en script serait un jumeau interdit par D839). Seule pièce utile : la traduction « fin de duel -> effets à poser » (tâche 1 ci-dessous), qui lit la table de moral.
- Lignes `[ ]` d'`Auto_Journey_File.md` : aucune ligne moral dans Elrond / Celeborn / Galadriel.

### Point 4, spécialisation
- Commandes de triche : en script, `Script/Player/HOLControllerCommands.as` (`PlanOf` l.161, `hol.Cheat.Tier` l.199-207, `Send` l.~300-315, console l.380-401) et `Script/Player/HOLControllerCommandsC2.as` (`hol.Spec.Start` l.139-147, `Send` l.~188). Chaîne : console -> `PlanOf` (pur) -> RPC script `UFUNCTION(Server)` de `Script/Net/HOLRpcPlayer.as` (`ServerCheatTier` l.1014, `ServerStartSpecialization` l.862) -> pont C++ `HOLRpcBridge::*` (`HOLScriptRpcPlayerActor.cpp:145`) -> `AHOLPlayerController::DoStartSpecialization` (`HOLPlayerController.cpp:4342`) -> `UHOLSpecializationSubsystem::StartNow/AdvanceNow` + `RefreshView` (C++, non exposés au script).
- Modèle d'une triche récente + test : `hol.Cheat.Tier` ; test de plan `Script/Player/HOLControllerCommands_Test.as:89-98`, test de l'effet `Script/Tests/HOLTierCheat_Test.as` ; modèle de test pour `hol.Spec.*` : `Test_ControllerCommandsC2Specialization` (`HOLControllerCommandsC2_Test.as:~118-128`).
- `hol.Cheat.Specialization` : plan pur faisable en script ; l'EFFET manque de liaison C++ : `HOLRpcBridge::CheatSpecialization(Tier, Option)` (appelle `Board->StartNow` puis `AdvanceNow(DelayMs(Tier))`, puis `RefreshView`) + le RPC `ServerCheatSpecialization` qui l'appelle (le RPC est du script, mais non compilable sans la liaison).
- Templiers à 3 options : règle `DefaultOptionCount = 2` partout (`HOLSpecializationRules.as:28`, `NewState(OptionCount)` l.58 accepte déjà 3 ; `Board StateOf` l.10 passe toujours le défaut). Règle pure `OptionCountOf(faction)` faisable ; le branchement exige la faction du siège côté tableau (liaison C++ absente) + la 3e case de la grille (`HOLGameScreen`, C++). `EHOLFaction::Templars` existe (vu dans `HOLAITalent.as`, `HOLFactionChoice_Test.as`).
- Texte de la case qui déborde : lui-même en C++ (`HOLGameNames::GetSpecializationCellText`) ; le journal 2026-11-16 15:15-16:35 dit corrigé (« {0} s » + Annuler, build en file) : à relire dans `HOLGameNames` avant toute reprise. La ligne `[ ]` l.42 de la file signale que `specialization_button` ÉCHOUE sur un main du 2026-11-16 (waitlog `=SpecializeT1A enabled`, cause non lue) : à recontrôler par le pré-vol de la session principale avant d'y envoyer quelqu'un.
- Lignes `[ ]` : l.42, l.44 (SP3 bouton permanent, C++ puis D857 : script), l.46 (texte, net à 2 clients). Le « net_match à deux clients » est un test réseau : lot C++/session principale.

### Point 5, duel D7
- `bRefusalCost` (`HOLDuelRules.as:116`, posé l.284) et `bFactionEffect` (l.123, posé l.292) : lus UNIQUEMENT par `HOLDuelRules_Test.as` ; aucun code de jeu. Idem `bXpBonus`, `TriumphSlot`, `DemoralisedSlot`, `HesitationSlot`. Le seul consommateur de `HOLDuel::Tick` est `HOLDuelLedger.as:194-199`, qui ne fait que journaliser (« Duel ended: slots ... »).
- Règle pure possible en script : la traduction `FHOLDuelEffects` -> liste d'ordres de moral (slot, effet de `HOLMoraleTable`, durée, rayon). Les durées sont déjà alignées (`Test_MoraleTableDuelEffects`, `HOLMoraleTable_Test.as:80` : Triumph et Demoralised = `S.TriumphMs`, Hesitation = `S.HesitationMs`). Le rayon du duel n'est PAS dans `Effects.csv` (RadiusCm 0 pour ces 3 lignes) ni dans `FHOLDuelSettings` : la valeur de départ écrite au journal est 12 m (M2).
- Branchement : `HOLDuelLedger.as:195` (boucle `Ended`) -> ordres -> application aux unités = M7 (C++ : `HOLMoraleLibrary` + `HOLDuelSubsystem`). XP : il n'existe pas de source `Duel` dans `EHOLXPSource` (`HOLHeroXPRules.h:10`) -> C++ ; montant « bonus modéré » (GDD 10.5, D35) sans nombre. Faction (Honneur, recharge Bannière) : A4/A5c, l'Honneur n'existe pas encore.

### Aube A2
- C++ de simulation, pas une règle script : redirection de dégâts dans la chaîne de combat à côté de `HOLCombatProcessor.cpp:638-730`, champ de table lu par la simulation, empreinte (hash), anneau au sol (`HOLSelectionMarkers`). Seule la ligne `VirtuousKnight` de `UnitTypes.csv` est de la donnée. A attendre un lot C++ groupé. (Plus : le cumul de deux Chevaliers est à demander à l'utilisateur.)

## PLAN DE LANCEMENT (3 tâches, fichiers disjoints)

Règle commune : deux commits (test rouge d'abord, puis correctif) ; l'agent ne lance rien, la session principale lance les tests ; le rouge « vu » = erreur de compilation de l'espace de noms absent (précédent M1) puis échec, à constater par la session principale sur le commit du test seul.

### Tâche 1 : récompenses du duel, règle pure (D7, partie script)
- (b) lire : `Script/Duel/HOLDuelRules.as:111-135` (FHOLDuelEffects), `:270-312` (Effects), `:415-420` (FHOLDuelEnded), `Script/Duel/HOLDuelLedger.as:188-199`, `Script/Morale/HOLMoraleTable.as:6-13` et `:168` (Effect), `Tools/Data/Effects.csv` (lignes Triumph/Demoralised/Hesitation), `Script/Tests/HOLMoraleTable_Test.as:80-97`.
- (c) créer `Script/Duel/HOLDuelRewards.as` (espace de noms `HOLDuelRewards`, struct `FHOLDuelMoraleOrder { int Slot; int EffectId; int DurationMs; int RadiusCm; }`, fonction `Orders(const FHOLDuelSettings& S, const FHOLDuelEffects& E)`) ; ne PAS toucher `HOLDuelRules.as`, `HOLDuelLedger.as`, `Effects.csv`, `HOLMoraleTable.as` (généré).
- (d) créer `Script/Duel/HOLDuelRewards_Test.as` ; modèle : `HOLDuelRules_Test.as:~135` `Test_DuelVictoryEffects` (enchaîne la vraie chaîne `Advance` -> `Effects`) et `HOLMoraleTable_Test.as:80` (valeurs lues dans la vraie table, aucun chiffre à la main). Tests : `Test_DuelRewardsWinnerTriumphLoserDemoralised` (victoire : 2 ordres, Triumph au gagnant, Demoralised au perdant, durée = `S.TriumphMs`) ; `Test_DuelRewardsRefusalHesitationOnRefuserOnly` (refus ou ignoré : 1 ordre Hesitation, au `Target`, durée `S.HesitationMs`, jamais Triumph) ; `Test_DuelRewardsDoubleKnockOutGivesNothing` (D590 : aucun ordre) ; `Test_DuelRewardsNothingWhileNotOver` (Active : liste vide) ; `Test_DuelRewardsRadiusIsTheDuelsOwn` (rayon identique pour les trois, > 0).
- (e) aucun C++ : fonction pure sur deux structs. Liaison C++ manquante pour la suite (hors tâche) : pont `HOLMoraleLibrary` + `HOLDuelSubsystem` pour appliquer les ordres aux unités (M7), source XP `Duel` dans `EHOLXPSource`.
- (f) NE PAS trancher : rayon d'effet du duel (valeur de départ du journal M2 : 12 m = 1200 cm, à poser en constante nommée `DefaultRadiusCm`, marquée provisoire) ; montant du bonus d'XP (aucune valeur au GDD, ne pas coder `bXpBonus`) ; effet de faction (A4/A5c). Rien d'autre.
- (g) à lancer : fichier `HOLDuelRewards_Test.as` (5 tests ci-dessus) + non-régression `HOLDuelRules_Test.as`, `HOLMoraleTable_Test.as` ; `quick_check.ps1 -Parts tests`.

### Tâche 2 : plan de `hol.Cheat.Specialization` (partie script)
- (b) lire : `Script/Player/HOLControllerCommandsC2.as` en entier (liste des noms, `PlanOf` l.139-147, `Send` l.~160-195, enregistrement console en bas), `Script/Player/HOLControllerCommands.as:199-207` (modèle Tier), `Script/Specialization/HOLSpecializationRules.as:71-76` (IsValidTier) et `:48-56` (codes).
- (c) modifier `Script/Player/HOLControllerCommandsC2.as` : ajouter `hol.Cheat.Specialization <Tier> <Option>` au `PlanOf` (route `ServerCheatSpecialization`, Ints = Tier, Option ; usage `Usage: hol.Cheat.Specialization <Tier> <Option>` ; palier hors 1..3 ou option < 0 refusés) + déclaration console ; NE PAS ajouter l'appel RPC tant que la liaison C++ n'existe pas (laisser `Send` inchangé ou y journaliser « route not sent » comme `LogNotSent`).
- (d) modifier `Script/Player/HOLControllerCommandsC2_Test.as` : `Test_ControllerCommandsC2CheatSpecialization` (route, Ints, usage sur 0 et 1 argument, palier 0 et 4 refusés), calqué sur `Test_ControllerCommandsC2Specialization` (l.118-128) et `HOLControllerCommands_Test.as:89-98`.
- (e) plan pur : aucun C++. MANQUE pour l'effet (lot C++ groupé) : `HOLRpcBridge::CheatSpecialization(Tier, Option)` dans `HOLScriptRpcPlayerActor.cpp` (modèle `DoStartSpecialization`, `HOLPlayerController.cpp:4342` : `StartNow` puis `AdvanceNow(HOLSpecialization::DelayMs)` puis `RefreshView`, tricherie interdite sans le drapeau `CheatRefused`) puis le RPC script `ServerCheatSpecialization` dans `HOLRpcPlayer.as` (modèle `ServerCheatTier` l.1014 avec `HOLRpc::CheatTierRefusal`).
- (f) NE PAS trancher : la triche applique-t-elle SANS respecter le palier atteint / l'ordre des paliers (le journal dit « applique sans délai » seulement, GDD `SpecializationInProgress.md:57`) : défaut = respecter `PreviousApplied` et le palier du joueur, sans délai ; à confirmer.
- (g) à lancer : `Test_ControllerCommandsC2CheatSpecialization` + toute `HOLControllerCommandsC2_Test.as` et `HOLControllerCommands_Test.as`.

### Tâche 3 : Templiers à 3 options, règle pure
- (b) lire : `Script/Specialization/HOLSpecializationRules.as:12-70` (struct, `DefaultOptionCount`, `NewState`) et `:131-153` (Start), `Script/Tests/HOLSpecializationRules_Test.as` (tests de Start / Advance existants, 21 Ko), `Script/Specialization/HOLSpecializationBoardScript.as:10-16` (`StateOf`, lecture seule).
- (c) modifier `Script/Specialization/HOLSpecializationRules.as` : `int OptionCountOf(EHOLFaction Faction)` (3 pour `Templars`, `DefaultOptionCount` sinon) ; NE PAS toucher `HOLSpecializationBoardScript.as` ni la grille.
- (d) modifier `Script/Tests/HOLSpecializationRules_Test.as` : `Test_SpecializationOptionCountPerFaction` (Templars 3, Dawn et Legions 2) ; `Test_SpecializationTemplarsCanPickTheThirdOption` (`NewState(OptionCountOf(Templars))`, `Start(.., 1, 2, ..)` = Ok, `Advance` jusqu'à `Tier1DelayMs`, `ChosenOption` = 2) ; `Test_SpecializationDawnRefusesTheThirdOption` (Option 2 -> `BadOption`). Modèle : les tests Start/Advance déjà dans ce fichier.
- (e) règle pure : aucun C++. Branchement qui attend le lot C++ : le tableau script ne connaît pas la faction du siège (`StateOf` utilise toujours le défaut) -> liaison `SetSeatFaction(Slot, Faction)` (sur le modèle de `SetSeatNomad`, `HOLSpecializationSubsystem.h`) + 3e case de la grille du Forum (`HOLGameScreen`/`HOLCommandGrid`, `GetSpecializationCellText`).
- (f) NE PAS trancher : la 3e voie est-elle une 3e case D/F/? de la grille (disposition) et les noms des 3 commanderies (libellés GDD 13 provisoires) ; défaut du journal : D90/D131, 3 options Templiers, rien d'autre posé.
- (g) à lancer : les 3 tests ci-dessus + tout `HOLSpecializationRules_Test.as`.

## Ce qui exige du C++ (lot groupé d'une autre séance)
- Moral M3 à M5, M7 (application des ordres du duel), M8 (points d'entrée Aube), M9 : tout le moteur Mass et le réseau.
- XP de duel (source `Duel` dans `EHOLXPSource`), Honneur (A4).
- Aube A2 (redirection de dégâts), anneau de sélection.
- Effet de `hol.Cheat.Specialization` (`HOLRpcBridge::CheatSpecialization`), `SetSeatFaction` du tableau, 3e case de la grille, texte de case (si encore à corriger), test net à 2 clients.

## Écartés, non vérifiés
- Aucune 4e tâche : le point 3 n'a plus de règle pure à écrire. Candidat de remplacement, non prêt : diagnostic de `specialization_button` (ligne `[ ]` l.42) : à faire APRÈS un pré-vol de la session principale sur main courant.
- Aucun test exécuté, aucun build ; les numéros de ligne sont ceux lus à 21:17-21:20.
