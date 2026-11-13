# SpecializationInProgress.md : le choix de spécialisation de palier, mécanisme commun (GDD § 9.5 et § 13 ; D07, D332, D333, D683, D684, avec D131, D334, D690)

Ouvert le 2026-11-12 (séance autonome, HALDIR-1112f, worktree Haldir, branche `maps`, chapitre A2 du relevé du jour). Méthode des
dossiers par feature : règles tirées du GDD, plan en étapes de 15 min, test rouge d'abord, au plus 2 questions
(`C:\Users\baudo\Documents\MyCrusader\Retours\Q-specialization.md`). Règle D483 : règles pures en Angelscript ; l'état répliqué et le
serveur en C++. **D690** : ce mécanisme est commun ; l'Aube (A8 de `AubeInProgress.md`) et les Légions (L9 de `LegionsInProgress.md`)
le reprennent, chacune n'y ajoute que ses deux options et leurs effets.

## Règles tirées du GDD (rien à trancher)

1. **Un palier (1, 2, 3) ouvre un tronc commun et le choix d'une spécialisation parmi 2** (D07, § 9.5), **définitif pour la partie**.
   *Templiers* : une commanderie parmi **3** par palier (D90, D131) ; ce sont des technologies (D683) qui débloquent l'unité dans le
   bâtiment de son type (D334).
2. **Quand** (D332) : à l'ouverture du palier, **un bouton s'allume dans le HUD** (emplacement de faction, D201) ; le joueur choisit
   **quand il le décide**, **gratuitement** ; aucune échéance pour choisir.
3. **Délai** (D684, révise « instantané » de D332) : on clique, **le déblocage se lance au Forum** et le choix est appliqué **au bout de
   30 / 45 / 60 s (paliers 1 / 2 / 3)**, valeurs à régler en test ; **en parallèle** de la file du Forum (qui continue de produire), **une
   barre à part** dans le HUD de faction montre le compte à rebours ; écarté : une place dans la file (D283).
4. **Annulation** (D684) : **sans coût avant la fin** ; on peut annuler et **rouvrir le choix** tant qu'il n'est pas appliqué ; **définitif
   une fois appliqué** (D07). Rien n'est rendu parce que rien n'a été payé (choix gratuit, D332, D683).
5. **À l'échéance** : ce qui dépend du choix arrive (D332) : une *technologie* s'applique (rien à construire) ; un *bâtiment* est
   **débloqué** puis se construit en plusieurs exemplaires, payé comme tout bâtiment (D683) ; *Templiers* : l'unité et son contingent.
6. **Lisibilité** (D333) : aucune information publique ni sur la fiche du héros ; **un signe dans le monde**, posé **à l'application**
   (D684) sur ce que l'effet modifie ou sur le bâtiment qui forme les unités concernées (D683). Hors des règles pures (étape SP3).
7. **L'IA choisit selon ses profils de build** (D332, § 9.5).

Lectures faites sans décision (pas des questions) : le palier vient du niveau du héros et ne baisse jamais (D569, paliers P1-P3) ; après
une annulation, une nouvelle option repart d'un délai entier (« annuler et rouvrir le choix ») ; un déblocage en cours ne se relance pas
sur place (annuler d'abord).

Laissé au chantier par D684 (questions, défauts codés) : **le compte à rebours quand le Forum est à terre** (Q2). Pas écrit au GDD :
**deux paliers à choisir en même temps** (joueur arrivé au palier 2 sans avoir choisi le 1 : un déblocage à la fois ou les deux, Q1).

## Ce qui existe (relevé du 2026-11-12)

- **Aucun code de spécialisation** : Grep `Specializ|TierChoice` dans `Script/` et `Source/` : rien.
- **Palier du joueur** (à réutiliser, pas à recopier) : `HOLHeroXP::GetTier(Level)` (`Source/HeroesOfLiberty/Simulation/HOLHeroXPRules.h`,
  niveau ÷ 3 borné 0..3, P1) ; `AHOLGameState::GetTier(Slot)` (P3, lu par la construction `HOLConstructionSubsystem.cpp:437`, la
  production `HOLProductionSubsystem.cpp:398`, l'IA `HOLAISummary.cpp:550`, l'écran `HOLGameScreen.cpp:580`) ; commande de développement
  `hol.Cheat.Tier <0..3|off> [Slot|all]` (`HOLPlayerController.cpp:1806`, G0 de Palier3). Les règles pures reçoivent ce palier en
  argument (`PlayerTier`) : elles ne lisent pas le monde.
- Règles de palier voisines (même forme) : `Script/Units/HOLUnitTierRules.as` (`HOLUnitTier::Check`), `Script/Siege/HOLSiegeRules.as`
  (`TierOf`), `Script/Technologies/HOLTechnologyRules.as` (palier minimum d'une technologie, lieu de recherche D583).
- À réutiliser plus tard : `HOLAITalent.as` (`AnswerFamily`, profil par faction) pour l'IA ; `EHOLFaction` (`Match/HOLMatchConfig.h`) ;
  la grille de commandes et l'emplacement de faction du HUD (D201) pour le bouton ; `Technologies.csv` pour les effets *technologie*.

## Plan (étapes de 15 min)

- **SP0 relevé** : FAIT (ce fichier, 2026-11-12).
- **SP1 règles pures en script** : `Script/Specialization/HOLSpecializationRules.as` (`FHOLSpecializationState`, `NewState`, `IsOpen`,
  `DelayMs`, `Start`, `Cancel`, `Advance` qui rend le palier appliqué, `RemainingMs` pour la barre, `ChosenOption`) et
  `Script/Tests/HOLSpecializationRules_Test.as` (7 tests). Délais au registre `Tools/Data/ValueSheet.csv` (`tier.choice.delay_*`).
- **SP2 état répliqué (C++, agent C++)** : un `FHOLSpecializationState` par emplacement sur `AHOLPlayerState` (répliqué au seul
  propriétaire pour le déblocage en cours : D203, l'adversaire ne lit rien) ; RPC serveur `ServerStartSpecialization(Tier, Option)` et
  `ServerCancelSpecialization()` qui appellent `Start` / `Cancel` avec `AHOLGameState::GetTier(Slot)` ; `Advance` au pas de la
  simulation (temps de partie, pas fixe) avec l'état du Forum ; à l'application, un événement (le palier, l'option) que les chantiers de
  faction écoutent (Aube A8, Légions L9). Commande de développement `hol.Cheat.Specialization <Tier> <Option>` (applique sans délai,
  pour les scénarios). Test : `net_match` (choix par un client, appliqué au serveur après le délai, annulé, refusé deux fois).
- **SP3 panneau de choix** : bouton allumé à l'emplacement de faction (D201) tant que `IsOpen` ; panneau des 2 (ou 3) options avec les
  noms du § 13 (nom de l'option = nom du choix, D683) ; barre à part avec `RemainingMs` (D684) ; bouton Annuler ; signe dans le monde à
  l'application (D333 ; provisoire : ligne à `Placeholders.txt`). Scénario `specialization` (geste réel), essai VU À L'ÉCRAN seulement.
- **SP4 IA** : choisir selon le profil de build (D332), au palier ouvert, après un court délai ; modèle `HOLAITalent.as` (`AnswerFamily`).
- **SP5 reprise par les factions** : Aube A8-A10 et Légions L9 branchent leurs options (effets, bâtiments débloqués) sur l'événement de SP2.

## Premier test rouge (SP1)

`Script/Tests/HOLSpecializationRules_Test.as` : 7 tests : le choix s'ouvre avec le palier et attend ; appliqué à 30 s et pas à 29,999 s ;
annulé = rien de choisi, choix rouvert, délai entier pour l'autre option ; un seul choix par palier, définitif ; départs refusés (palier 0
ou 4, palier non atteint, option hors des 2, Templiers à 3) ; deux paliers (Q1) ; Forum à terre (Q2).

## Questions (`Retours\Q-specialization.md`, défauts codés)

- Q1 : deux paliers à choisir à la fois. Défaut A = un déblocage à la fois (`OneUnlockAtATime`), comme la barre unique de D684.
- Q2 : le compte à rebours quand le Forum est à terre. Défaut A = en pause, reprend quand il est relevé (`ForumDown = ForumDownPauses`).
- **Réponses de l'utilisateur (2026-11-12, D785)** : Q1 = **C, le palier 1 d'abord : le bouton du palier 2 ne s'allume qu'une fois le palier 1 appliqué** (À CHANGER : remplace `OneUnlockAtATime`, une ligne de plus dans `HOLSpecializationRules.as`, test à adapter) ; Q2 = **A, en pause quand le Forum est à terre** (déjà codé).

## Suivi

- 2026-11-12 11:45-11:56 (HALDIR-1112f, `maps`, sans C++) : SP0 relevé ; test rouge SP1 commité 567fd50 (quick_check `tests` 11:50 :
  « Identifier 'FHOLSpecializationState' is not a data type », 0 test lancé). **SP1 VERTE** : quick_check `tests,values` 1193/1193 PASS
  (219 s), `values` PASS (3 lignes au registre `tier.choice.delay_tier1_ms`, `_tier2_ms`, `_tier3_ms` ; Valeurs.md/.csv régénérés).
  Commit vert 1124a13, puis `git merge main` 3ff176d (docs seuls). Questions `Retours\Q-specialization.md` (Q1 deux paliers à la fois, défaut A ; Q2 Forum à terre, défaut A pause). **Prochaine étape
  pour un agent neuf : SP2** (agent C++ : état sur `AHOLPlayerState`, RPC serveur Start / Cancel avec `AHOLGameState::GetTier`, `Advance`
  au pas de la simulation, événement d'application, `hol.Cheat.Specialization`), puis SP3. Aucun test rouge en cours. Rien de visible en
  jeu : pas de ligne à `Placeholders.txt`.

## 2026-11-13 11:10 HALDIR-1113 (agent, maps)
- D785 applique : OneUnlockAtATime remplace par PreviousTierFirst (palier N ouvert seulement si N-1 applique, reponse PreviousTierNotApplied = 7) ; rouge bad840e (Test_SpecializationTierOneFirst, IsOpen 2/3 eteints avant le palier d'avant ; ligne 80 : 30/45/60 s assertis en litteral et par Start/Advance a 29999/44999/59999 + 1 ms), vert 8d028ff ; quick_check -Parts tests 1203/1203 PASS (11:10). Toujours aucun appelant en jeu (C++ SC2/SG2/SP2/N2 a faire). Reprise : etape C++ suivante du journal.
