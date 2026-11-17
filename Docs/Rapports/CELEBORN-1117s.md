# CELEBORN-1117s : liaison HOLForumRules au script + port des tests purs Forum.*

NON COMPILÉ : aucun build, aucun quick_check (consigne). Le C++ et le .as n'ont jamais été vus compiler.

APPELS_DECLARES: ~14 (arrêt à 45)
HEURE_DEBUT/HEURE_FIN: 23:54 / 23:58
PREMIER_BUILD: sans build   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: sans build (rien lancé)
TEST_ROUGE: aucun (rouge non vu) ; assertit l'effet : oui (états et valeurs calculés par la règle)
COMMITS: 1, f299edb à 23:57 sur hero-j8-close ; merge_main_fait: oui (premier geste, sans build)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0 (chaque valeur vient de HOLForumRules via la bibliothèque ; aucune constante recopiée)
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 0 (la session principale lance le build groupé)
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 9 (Test_ForumRulesLives, Test_ForumRulesGameValues, Test_ForumRulesPhases, Test_ForumHoldRules, Test_ForumHoldProgress, Test_ForumBlock, Test_ForumHoldDisplay, Test_ForumLook, plus ForumNear helper non test : 8 tests + 1 helper)
LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: ~330 lignes de test C++ couvertes
TYPES_A_EXPOSER: HOLVictoryRules et FHOLMatchConfig (Galadriel) ; reste pour Forum : GetPlayersTable (FHOLMatchConfig), GetRingColor, GetZoneOutlinePoint, FHOLForumBlock::Text complet (accents)

## Fichiers (worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, branche hero-j8-close, commit f299edb)
- Source\HeroesOfLiberty\Match\HOLForumRulesReadLibrary.h / .cpp (neufs)
- Script\Tests\HOLForumRules_Test.as (neuf)

## Fonctions C++ ajoutées (UHOLForumRulesReadLibrary, namespace script `HOLForumRulesRead`)
Raison du C++ : liaison pour exposer une règle pure au script (D836 : la règle reste en C++, FHOLForumState est répliqué).
Quatre petits USTRUCT BlueprintType (le FHOLForumState réel est un USTRUCT non BlueprintType, on le copie) :
FHOLForumStateRead (Falls, Phase, RuinStepsLeft, RebuildWork int64, HoldSteps, Hold, HolderSlot ; Phase/Hold = valeur de l'enum en int32), FHOLForumTuningRead (8 champs), FHOLForumStepResultRead, FHOLForumBlockRead.
Fonctions : GetBaseLives, GetGameTuning (FromSettings), GetLivesByTier, GetLives, GetRebuildTotalWork, GetRebuildWorkPerStep, GetStepsToRebuild, GetRebuildProgress, GetRebuildPercent, GetRuinSecondsLeft, Fall, Advance, Step (présence passée en bAttackers/bDefenders/AttackerSlot), Raze, Skip (ces 5 ont l'état en ref, copié puis réécrit), GetHoldSeconds, GetHoldProgress, ShowsHold, DescribeHold, IsRingShown, GetRingFill, GetRingHolderSlot, GetRingShown, GetSelectionBlock (bIsMainForum=false = état nul), GetLook, GetLookProgress, BuildersWait.
Ne touche ni HOLVictoryRules ni FHOLMatchConfig.

## Suspects de compilation (à regarder en premier si le build ou le script casse)
1. `int64` en UPROPERTY BlueprintType (FHOLForumStateRead::RebuildWork) et en retour UFUNCTION (GetRebuildTotalWork) : normalement accepté par UHT UE5 et par le plugin Angelscript ; si refus, passer en double/int32 (RebuildWork reste petit dans les tests) et adapter ToRule/FromRule.
2. `UPARAM(ref)` sur un USTRUCT en UFUNCTION(BlueprintCallable) static : même motif que HOLFactionChoiceReadLibrary::Click (FHOLClickCounter), donc attendu OK.
3. `static_cast<int8>(Read.HolderSlot)` et `FromRule` (uint8 Falls -> int32) : implicites, OK ; vérifier l'avertissement de conversion éventuel (/WX).
4. Fonctions anonymes de namespace `ToRule` surchargées (State / Tuning) dans le .cpp : OK en C++, aucun conflit UHT.
5. Script : `FHOLForumStateRead State;` construit à la valeur par défaut (HolderSlot -1) : si le plugin n'applique pas les défauts C++ aux champs, les assertions « -1 » de Test_ForumHoldRules et le défaut `BuilderExponent 0.63` / `HoldRadius 1200` seraient faux (rouge probable, pas un bug de règle) ; vérifier d'abord ce point si un rouge inattendu sort.
6. Script : le nom de namespace `HOLForumRulesRead` est déduit de `UHOLForumRulesReadLibrary` comme `HOLFactionChoiceRead` l'est pour `UHOLFactionChoiceReadLibrary` ; la fonction globale `ForumNear` est dans un fichier seul (pas partagée).
7. `FString::Contains` en script (Test_ForumBlock) : méthode attendue ; sinon remplacer par `Find(...) >= 0`.

## Tests C++ retirables (par la session principale, après rouge vu puis vert)
- Victory.Forum.Rules (FHOLVictoryForumRulesTest) : entièrement couvert (Lives, valeurs du jeu, phases, Skip) -> retirable.
- Victory.Forum.Hold.Rules (FHOLVictoryForumHoldRulesTest) : entièrement couvert (tenue, contestation, recul, rasage, dernière vie, secondes, progression, reconstruit remet 0) -> retirable.
- Victory.Forum.Block, Victory.Forum.Hold.Display, Victory.Forum.Look : PARTIELS, à garder (non portés : lignes Text avec tiret cadratin et accents, GetRingColor, GetZoneOutlinePoint, GetPlayersTable avec FHOLMatchConfig ; Look : la fin du test utilise FHOLUnitTypeConstants/animation). Ils deviennent retirables si on accepte de perdre ces lignes ou après liaison des types manquants.
- Non portables (monde/Mass, D838) : Forum.Fall, Hold.Chain, Hold.Presence, Hold.PresenceTeams, Builders, Appearance. Forum.PlayersTable attend l'exposition de FHOLMatchConfig (Galadriel).

## À lancer par la session principale
Build groupé (`Tools\build.ps1 -LiveCoding=0 -Wait` dans le worktree Celeborn), puis `quick_check -Parts Script` ; rouge à voir en faussant une valeur attendue (par ex. `GetLivesByTier(0, 0)` attendu 3) avant de retirer les tests C++.

## État git
Worktree : 16 fichiers non suivis dans Tools\Scenarios (c28_*, c31_*) hérités d'une séance précédente, sans lien avec la migration, NON commités ni touchés. Aucun fichier suivi modifié.

NON VÉRIFIÉ: compilation C++ et script, noms exacts des tests découverts (Test_ + nom), tout comportement à l'exécution.
PROCESSUS_DETACHES: aucun (rien lancé)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: non relevée ; aucun fichier de la copie principale touché hors ce rapport.
