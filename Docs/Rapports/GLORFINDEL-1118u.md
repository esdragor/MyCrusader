# GLORFINDEL-1118u : HUD H-2, message court + population bloquée, branche d'essai `net-playtest`

NON COMPILÉ. Aucun build, aucun quick_check, aucun lancement. Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel`.

Joueur : rien ne change à l'écran sauf la place du message (au-dessus de la file / grille, calé par le script sur `QueuePanel`, sinon `GridPanel`, au lieu de la colonne de commandes) ; mêmes textes, même durée 3 s, même fondu 0,5 s.

## Commits (anglais, sans Co-Authored-By, pas de push)
1. `88e3954` : C++ de liaison + correction des deux .as (règle sans clé, test neuf). Le HUD ne change pas (aucun appelant du nouvel événement, `bMessageLive` toujours faux).
2. `2c07165` : BASCULE : crochet dans `HOLGameScreenScript.as` (membres, `Build` dans `OnInitialized`, `RefreshShortMessage()` dans `Refresh`, overrides `ShortMessage` et `FrameTick`) + `ShowMessage` C++ passe la main au script, `CheckPopulationBlocked` / `UpdateMessage` ne sont plus appelés, la mise en page C++ `MessagePanel` n'est plus construite.
3. `8a5f28c` : RETRAIT : `UpdateMessage`, `CheckPopulationBlocked` (.cpp et .h), `MessagePanel`, `MessageText`, `MessageAge`, `bPopulationBlocked`, `MessageSeconds`, `MessageFadeSeconds`. Grep de contrôle de chaque symbole dans `Source\` et `Script\` : plus qu'en commentaires (`HOLGameScreen.h:400` doc de `ShowMessage` parle encore de MessageSeconds ; `HOLProductionPopulationEndTests.cpp:155` commentaire ; `HOLShortMessage.as` commentaires).

## (B) Coût par image : variante retenue
Variante « appel par image seulement pendant qu'un message avec clé est affiché ». `UHOLScriptScreen::bMessageLive` (privé) : vrai quand `SetShownMessageKey(clé non vide)` (appelé par l'override `ShortMessage`, par la note population et jamais autrement), faux quand `SetShownMessageKey("")` (fin du message, appelé par l'override `FrameTick` quand `HOLShortMessage::Tick` renvoie vrai, ou message sans clé). `NativeTick` n'appelle `FrameTick` que si `bMessageLive`. Pourquoi : la plus simple qui garde le fondu lisse (opacité calculée par le script à l'image, règle de durée et de priorité en script et testée) ; la variante « le C++ garde le minuteur et le fondu » aurait obligé le C++ à connaître le panneau du script ou à dupliquer la règle (jumeau interdit). Hors message : zéro appel script par image. Coût résiduel : un appel script par image pendant 3 s à chaque message.
Autre différence de rythme : la lecture « production retenue » se fait au `Refresh` du script (4 par seconde) au lieu de chaque image : le message population peut venir jusqu'à 0,25 s plus tard qu'avant (invisible ; les outils attendent par `waitlog`).

## (A) Fidélité : message sans clé, QUESTION POSSIBLE POUR L'UTILISATEUR (non tranchée)
Le C++ `UpdateMessage` sortait dès que `MessageKey` était vide : un message sans clé (cas `GetFeedbackKey` par défaut) n'était jamais masqué ni estompé. Le script fait pareil : `Advance` renvoie vrai sans vieillir pour une clé vide, `Tick` ne touche pas à l'opacité, `ShownKey` vide. Test `Test_ShortMessageWithoutKeyNeverEnds` (rouge à voir : retirer la ligne `State.Key.IsEmpty()` d'`Advance`). Ressemble à un défaut : à demander s'il existe réellement un `EHOLCommandFeedback` sans clé (à vérifier dans `GetFeedbackKey`, HOLGameScreen.cpp:162-236) ; sinon sans objet.
Autres écarts connus (voulus, signalés) : si `ScriptScreen` est nul (aucune classe script), le message n'est plus dessiné (seule la ligne `HUD message:` est écrite par le C++ ; il n'y a plus de repli visuel) ; la boucle de lecture `IsProductionHeldForRoom` est dans le C++ (const), identique.

## Table « ligne attendue par un outil -> où elle est écrite »
| Ligne / fait attendu | Lu par | Écrit par (après bascule) |
|---|---|---|
| `HUD message: NotEnoughWood "Pas assez de bois"` (3 fois, une par langue) | quick_check.ps1 1672, 1675, 4261, 4263 | script `HOLShortMessage::ShowText` -> `Log(n"LogHOLUI", MessageLogLine(Key, Text.ToString()))` ; le texte vient de `GetFeedbackText` C++ (LOCTEXT, langue active) |
| `HUD message: PopulationFull` + `queued Peasant` | quick_check 1710, 4284 | idem (commande refusée -> `HandleCommandFeedback` -> `ShowMessage` -> événement `ShortMessage`) |
| `HUD message: NoMilitaryBuilding "Aucun b.+timent militaire"`, `NoTechnologicalBuilding` | quick_check 1716, 1719, 4288, 4289 | idem |
| `HUD report: ..., message <clé>` (`message NotEnoughWood`, `message PopulationFull`, `selection none, grid hidden, orders none, queue hidden, message NoMilitaryBuilding`) | quick_check 4262, 4285, 4291 ; `hol.HUD.Report` | `UHOLGameScreen::Describe` (HOLGameScreen.cpp ~3109, inchangé) lit `MessageKey` : posé par `ShowMessage` (immédiat) et par `SetShownMessageKey` (note population, fin du message) |
| `HUD: production held at 100 % (population full), line to the thread` | `alert_population.txt:37` (`waitlog production held at 100 %`) | script `CheckPopulationHeld` -> `HeldLogLine()` ; ordre : message, ligne du fil, ligne de journal (comme le C++) |
| `HUD messages: ... Population pleine` (fil) | alert_population, quick_check 3243-3245 | inchangé : `AddThreadLine` (script, `HOLAlertRules::PopulationFullText`, même NSLOCTEXT `HOLGameScreen,PopulationFull`) |
| `queue note Population pleine`, `population full` (Describe de la file) | quick_check 4284 | inchangé (C++, `QueueNote`, `HeadState`) |

Réserves : (1) la verbosité du `Log` global du script pour `LogHOLUI` doit passer le même filtre que `UE_LOG(Display)` (NON VÉRIFIÉ) ; (2) la ligne du message est écrite à l'appel de l'événement, donc avant que le panneau soit visible, comme avant ; (3) `hol.HUD.Report` n'a pas de ligne script nouvelle (rien ajouté à `Describe` du script).

## Suspects de compilation (par ordre de probabilité)
1. Override script `ShortMessage(const FString& Key, const FText& Text)` : un `const FText&` en paramètre d'événement ; sinon `FText Text`.
2. `FHOLShortMessageParts` : struct de membres UObject passée en `&` (`FHOLShortMessageParts& Parts`), `FVector2D&` en sortie dans `FindAnchor`, `Log(n"LogHOLUI", ...)` global (déclaration non vue, lu dans HOLNewGameScreen.as:104).
3. `FrameTick(float32 DeltaSeconds)` passé à `Tick(..., float DeltaSeconds)` (conversion implicite float32 -> float).
4. `SetTextWrapWidth` devenue méthode `const` (la recette la voulait statique) : appelée par `Screen.SetTextWrapWidth(Parts.Text, WrapWidth)` dans `Build`.
5. C++ : `IsProductionHeldForRoom() const` reprend la boucle avec `FindUnitType`, `HOLProduction::IsWaitingForRoom`, `UHOLProductionSubsystem::GetDisplayStep(World)` ; si un de ces appels n'est pas const-correct, retirer `const`. `GetTypedOuter<UHOLGameScreen>()` dans `HOLScriptScreen.cpp` (déjà employé). Unity : pas de nouveau namespace anonyme. Aucune variable locale nommée Slot (membre de struct renommé `PanelSlot`).
6. `ShowMessage` : `UE_LOG` dans la branche sans script ; `LogHOLUI` déjà dans le fichier.
7. Retrait : `HOLProductionPopulationEndTests.cpp` et `HOLPopulationAlertThreadTests.cpp` (friend `FHOLPopulationAlertThreadTest`) : grep ne montre aucun usage des symboles retirés hors commentaires.

## Outils de test (rien à modifier)
`hol.HUD.Report` et `Describe*` donnent les mêmes textes (mirror `MessageKey`). `HUD rects` / `DescribeOverlaps` : le script crée un `MessagePanel` compté parmi ses panneaux visibles pendant 3 s ; `QueuePanel` / `GridPanel` ne sont pas dans la liste comparée ; `hud_rects_game_test` doit tolérer une entrée `MessagePanel=` (NON VÉRIFIÉ). La mise en page C++ ne crée plus de widget `MessagePanel`, donc pas de doublon de nom dans l'arbre.

## Commandes de validation (session principale, dans l'ordre)
1. `C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\build.ps1 -LiveCoding=0 -Wait` (un build : lot C++ + scripts).
2. `quick_check -Parts tests,smoke,production` (tests script : les 7 `Test_ShortMessage*` / `Test_PopulationHeld*` ; rouge à VOIR en appliquant une mutation de commentaire à la fois) ; puis la partie `build hud` (lignes ci-dessus).
3. `Tools\Scenarios\hud_rects_game_test.ps1 -Root <worktree>`.
4. `play.ps1 alert_population -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel` (message « Population maximale — construisez des maisons » UNE fois au-dessus de la file, fondu après 3 s, ligne « Population pleine » au fil ; regarder si le fil écrit la ligne deux fois : doublon connu entre `RefreshPopulationAlert` et `CheckPopulationHeld`).
5. Capture `build_hud_popfull` à comparer avec l'ancienne (message au-dessus de la file, hors colonne).

## Champs
BUILDS_DEMANDES: 1 ; TESTS_SCRIPT_AJOUTES: 1 (Test_ShortMessageWithoutKeyNeverEnds) ; TESTS_CPP_RETIRES: 0 ; LIGNES_CPP_SUPPRIMEES: ~110 (commit 3 et mise en page du commit 2) ; TEST_ROUGE: non vu (rien lancé) ; TYPES_A_EXPOSER: aucun nouveau type (5 fonctions de base : ShortMessage, FrameTick, IsProductionHeldForRoom, SetShownMessageKey, SetTextWrapWidth) ; merge main non fait.
