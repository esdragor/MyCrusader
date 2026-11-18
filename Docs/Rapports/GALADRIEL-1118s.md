# GALADRIEL-1118s : ecran Statistiques en script, compilation et tests (ST-2 corrige)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`, branche `galadriel-tab`, commit `d5db451` (1 fichier : `Script\UI\HOLStatsScreen.as`), non fusionne, non pousse. Aucun `Source\`, aucun build. Pas touche : EtapeDefini.md, Auto_Journey_File.md, *InProgress.md. HOLStatsRules.as et HOLStatsScreen_Test.as inchanges (la mutation du rouge a ete remise, `git status` propre).
HOLNewGameScreen.as et son test supprimes du disque le temps des essais, puis remis par `git checkout` avant le commit.

BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 (les 3 de ST-2 passent) ; LIGNES_CPP_SUPPRIMEES: 0.

## Dernier compte quick_check -Parts tests
`tests 1534/1534 : PASS` (154 s, 21:03-21:06). Les 3 tests tournent (`Angelscript.UnitTests.UI.HOLStatsScreen_Test.Test_StatsScriptClass / Test_StatsSummaryRow / Test_StatsTimelineSeries`, vus dans le journal).

## Rouge vu
OUI. Mutation dans `HOLStatsRules.as::SummaryRow` : `NumberCell(MilitaryScore + EconomyScore)` -> `NumberCell(MilitaryScore)`. Resultat `tests 1533/1534 : FAIL`, echec `Test_StatsSummaryRow` seul. Remis (diff vide), non relance apres remise (temps), le vert 1534/1534 vient de l'essai d'avant.

## Chaque erreur -> correction (ce qui n'existe pas en Angelscript ici -> ce qu'on ecrit)
Les erreurs sont sorties une a la fois (4 essais de compilation, puis 2 essais de tests).
1. `FText::AsNumber(float)` et `(float32)` : n'existe PAS en script (seuls `FromString`, `AsCultureInvariant`, `IdenticalTo` sont lies dans le projet) -> `FText::FromString(f"{Cell.Value}")` (entier brut, perd le groupement par culture ; souhait base : `FText NumberText(int)`).
2. `HOLMatchStatsRead::HasWorldStats(this)` / `GetWorldStats(this)` / `GetWorldStatsRows(this)` : le parametre `WorldContextObject` est implicite en script (meta WorldContext) -> appeler sans argument : `HasWorldStats()`, `GetWorldStats()`, `GetWorldStatsRows()`.
3. `Grids[Tab].ClearChildren()` : `Grids` est `TArray<TObjectPtr<UGridPanel>>`, l'element est un `TObjectPtr`, sans methode de widget -> copier dans un pointeur local `UGridPanel Grid = Grids[Tab]; Grid.ClearChildren();` (l'affectation convertit) ; pour passer les grilles aux aides, `TArray<UGridPanel> Tables` remplie depuis `Grids`.
4. Surcharges `BP_BuildStats`, `BP_RefreshStats`, `BP_RefreshTimeline` : le prefixe `BP_` est retire en script (« Use name `BuildStats` instead ... ») -> `void BuildStats()`, `void RefreshStats(bool)`, `void RefreshTimeline(int)` avec `UFUNCTION(BlueprintOverride)`.
5. Appel `BP_RefreshTimeline(int)` (puis `RefreshTimeline(int)`) depuis le script : « Function RefreshTimeline is not BlueprintCallable or ScriptCallable » (piege 3 du brief : le nom resout vers la methode C++ privee `RefreshTimeline` de la base) -> corps dans une aide de nom distinct `DrawTimeline(int Series)`, appelee par la surcharge `RefreshTimeline` et par `RefreshStats`.

## Chaines en dur
Aucune sauf le tiret des cases vides (`CellText`, commentaire `// MANQUE:`, inchange).

## A AJOUTER A LA BASE C++
1. Tiret : cle `Dash` dans `UHOLStatsScreen::TextOf` (ou `static FText UHOLStatsScreen::TeamText(int Team)`).
2. Nom/couleur d'un slot ferme pour un repere : `FText HOLMatchStatsRead::GetWorldSlotName(int SlotIndex)` + `FLinearColor GetWorldSlotColor(int SlotIndex)` (contexte de monde implicite en script).
3. `FText UHOLStatsScreen::NumberText(int Value)` (= `FText::AsNumber`, absent du script).
4. Journal par ligne (`DescribePlayer`) : `FString HOLMatchStatsRead::DescribePlayer(const FHOLPlayerMatchStatsState&)`, non reproduit.

## NON VERIFIE
- Aucune creation d'ecran n'est testee (pas de 4e test : sans controleur de joueur dans le run, un test d'ecran serait soit vert a vide, soit rouge a tort ; non tranche, non ajoute). Les 3 tests ne touchent que les regles pures et FindScriptClass (ce dernier ECHOUE si la classe est absente : AssertTrue).
- Le rendu (BuildStats / RefreshStats executes sur un vrai ecran) : jamais joue ; la bascule `Open` (ST-3) n'est pas dans ce lot.
- Hypotheses des tests (MakeCurve, FHOLResources, enums de visibilite) confirmees par la compilation et le vert.

## Anomalies
Horloge Get-Date : 21:00 a 21:08 pendant ~12 appels ; l'heure des journaux UE est decalee d'1 h (20:0x). Avertissement git LF->CRLF normal.
