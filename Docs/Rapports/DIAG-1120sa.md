# DIAG-1120sa : integ-32, « tests 0/0 » (lecture seule, 2026-11-20, ~23:15)

## Cause : UNE erreur de compilation Angelscript, et c'est la seule

- Fichier:ligne : `Script/UI/HOLAlertRules_Test.as` (134:2), message `Unexpected end of file` puis `Hot reload failed due to script compile errors. Keeping all old script code.` (fin de `Docs\Integrate_integ-32.log` : `[KO] scripts compiled`, `[KO] tests ran (0)`).
- Cause exacte : une accolade fermante manquante. Dans `Test_AlertPopulationFullLineStays15s` (lignes 45-56) la ligne 57 est vide puis la ligne 58 ouvre `void Test_AlertAttackedElsewhereOncePer30s` : le `}` final de la fonction a disparu. Au total 12 `{` pour 11 `}` dans le fichier (profondeur finale 1, relevée ligne à ligne : la profondeur reste à 1 à partir de la ligne 58).
- Commit fautif : `8ce795d` « Test: population full line stays 15 s, repeat delay stays 30 s » (appliqué sur integ-32 sous le sha `ec87025`). Son diff supprime `-}` (fin de `Test_AlertPopulationFullAtMostOncePer15s` de 4692ec5) et ne rajoute aucun `}` après la dernière assertion (`+ ... 31.0 ... "a new line");` puis rien). Preuve : `git show 8ce795d:Script/UI/HOLAlertRules_Test.as` = 12 `{` / 11 `}` ; `git show 4692ec5:...` = 12 / 12 (sain). Le même défaut est dans le commit d'origine de la branche Galadriel, pas dans le picorage : on ne peut pas le résoudre en re-picorant.
- `1a00e0f` (Glorfindel, intégré sous `f0e7912`) : aucune erreur trouvée. `4692ec5` (intégré sous `e1e8fd3`) : sain (12/12).

## Correctif proposé (une ligne)

Ajouter un `}` seul sur une ligne, juste après la ligne 56 (`T.AssertTrue(... 31.0, Delay), "full again 31 s after the first line: a new line");`), avant la ligne vide. Dans un commit de correction sur integ-32 (ou à la source, dans la branche Galadriel, avant fusion).

## Par branche (ordre de fusion à venir)

- **specialization-trial** (`ae69298`, `fd9a795`) : `ae69298` ne touche qu'une ligne de `HOLAlertRules_Test.as` (`Test_AlertPopulationFullLineDuration` : `<` devient `<=`) et NE répare PAS l'accolade ; relevé sur la pointe de la branche : 12 `{` / 11 `}`. Donc la pointe de `specialization-trial` ne compile pas non plus : ajouter le `}` manquant (même ligne 56) dans cette branche ou dans un commit d'intégration. `fd9a795` ne touche pas ce test (HOLAlertRules.as, HOLShortMessage.as). Résultat net des deux commits sur `PopulationRepeatSeconds` : 15 puis 30, donc 30.0 (le test `Stays15s` attend 30 : cohérent).
- **ability-columns** (`3579205`, correctif de chat) : le correctif remplace le bouchon `ReturnFocusToGame` par `++FocusReturns; if (Screen != nullptr) { WidgetBlueprint::SetFocusToGameViewport(); }` et appelle `ReturnFocusToGame()` en fin de `Close()`. Le test de Glorfindel passe alors (`Screen` est nul dans le test : seul le compteur monte, aucun appel moteur). `UWidgetBlueprintLibrary::SetFocusToGameViewport()` existe bien (`WidgetBlueprintLibrary.h:67-68`, `BlueprintCallable, BlueprintCosmetic`, statique sans paramètre). Le nom script `WidgetBlueprint::SetFocusToGameViewport()` n'est PAS vérifié : aucun `.as` du projet n'emploie `WidgetBlueprint::`, et le plugin Angelscript n'a pas été trouvé dans la recherche. Si la compilation refuse le nom : essayer `SetFocusToGameViewport` sans espace de noms n'est pas possible ; solution de repli : `WidgetBlueprint::SetInputMode_GameOnly(PC)` (liée par le même espace de noms) ou un accesseur C++ à exposer, à signaler au lot C++.

## Tests rouges : compilent-ils avec leur bouchon sur main ?

- `Test_ChatInputScreenCloseAfterSendReturnsFocusToGame` (HOLChatInputScreen_Test.as, 1a00e0f) : tous les symboles existent dans HEAD : `UHOLChatInputScreen.State` (l.112), `FocusReturns` (l.120), `LastCommitTime`, `Close()`, `ReturnFocusToGame()` (bouchon l.180), `HOLChatInput::PressEnter(FHOLChatInputState&, const FString&, bool, bool)`, `HOLChatInput::Opened` / `Send` (int), `HOLChatInputScreen::EnterOpensField(bool, float, float)`. `AssertEquals` porte sur des `int` des deux côtés, pas de piège double/float. Compile et doit être rouge (`FocusReturns` reste 0 : le bouchon et `Close()` ne l'incrémentent pas).
- `Test_AlertPopulationFullLineStays15s` (8ce795d) : symboles présents dans main : `HOLAlertRules::PopulationFullLineSeconds` (HOLAlertRules.as:39, = 15.0), `PopulationRepeatSeconds` (:17, = 30.0), `ShouldAlertPopulationFull`, `FHOLPopulationAlertWatch`. Une fois la syntaxe réparée il COMPILE. MAIS il ne sera pas rouge : sur main les deux constantes valent déjà 15 et 30 (le « rouge » du commit supposait un délai de 15 s, état de la branche Galadriel avant `fd9a795`). Il sera vert d'emblée : prévoir « rouge non vu » pour cette ligne dans la recette, pas une régression.
- `Test_AlertPopulationFullAtMostOncePer15s` (4692ec5) a été renommé par 8ce795d : sur integ-32 il n'en reste qu'une fonction, pas de doublon de nom.
- C++ Elrond (`c7affed`, `HOLDeployRules.cpp`) : non analysé ici (hors tâche) ; le log montre un conflit de fusion integ-25 sur `Source/HeroesOfLiberty/Siege/HOLDeployRules.cpp`, laissé tel quel comme demandé.

## NON VÉRIFIÉ

- Le nom de liaison script `WidgetBlueprint::SetFocusToGameViewport` (voir ci-dessus).
- Aucune relance de compilation : le correctif de l'accolade est prédit par comptage d'accolades et lecture, pas par un `quick_check`.
- Le journal de l'éditeur de ce quick_check n'est pas apparu sous `Saved\Logs` (seuls `build.log` 23:11, `HeroesOfLiberty.log` 17:27) ; l'erreur vient du log d'integrate (qui reproduit la sortie de `quick_check`), pas du journal de l'éditeur.
- Des erreurs secondaires derrière l'erreur de syntaxe (le compilateur s'arrête au premier fichier en échec) restent possibles dans les autres `.as` : une fois le `}` ajouté, relire la sortie du compilateur.
