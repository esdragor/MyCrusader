# GALADRIEL-1118r : lot ST-2 (ecran Statistiques en script)

**NON COMPILE, AUCUN TEST LANCE.** Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`, branche `galadriel-tab`, commit `9dfed28` (3 fichiers neufs, aucun `Source\`), non fusionne, non pousse. "branche prete" apres le build du lot ST-1 deja compile.
Fin ~20:50, ~16 appels d'outils. Pas touche : EtapeDefini.md, Auto_Journey_File.md, *InProgress.md.

BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 3 (Test_StatsScriptClass, Test_StatsSummaryRow, Test_StatsTimelineSeries) ; LIGNES_CPP_SUPPRIMEES: 0 (ST-3) ; LIGNES_MIGREES_PAR_LOT: ~330 lignes de script ecrites (~ 270 lignes C++ reproduites : BuildLayout 62-112, Refresh 308-428, RefreshTimeline 242-302) ; TYPES_A_EXPOSER: aucun obligatoire (voir MANQUES).

## Fichiers (tous `Script\UI\`)
- `HOLStatsScreen.as` : `class UHOLStatsScreenScript : UHOLStatsScreen` ; surcharges `BP_BuildStats`, `BP_RefreshStats(bool)`, `BP_RefreshTimeline(int)` ; aide `CellText(FHOLStatsCell)`.
- `HOLStatsRules.as` : `struct FHOLStatsCell {Value, Key, bDash}` et `namespace HOLStatsRules` (TableCount, SeriesCount, SeriesName, ClampSeries, StatusKey, ResultKey, NumberCell, SummaryRow, MilitaryRow, EconomyRow, TableRow, FindRow, BuildCurves, BuildMarks). Fonctions pures : entrees = `FHOLStatsRowState` / `FHOLMatchStatsState` / `FHOLPlayerMatchStatsState`.
- `HOLStatsScreen_Test.as` : 3 tests, aides `STS_MakeRow`, `STS_MakePlayer`.

## Widgets crees (memes noms que le C++)
Heading (texte), SummaryTab, MilitaryTab, EconomyTab, TimelineTab (AddStatsButton dans une rangee), Status (texte corps, replie), TabPages (AddStatsPages), 3 grilles (AddStatsGridPage : Resume, Militaire, Economie), page Chronologie (AddStatsTimelinePage), rangee de series + 6 boutons `SeriesWorkers`, `SeriesMilitary`, `SeriesPopulation`, `SeriesBuildings`, `SeriesGathered`, `SeriesStock` (nom = "Series" + HOLStatsRules::SeriesName, label = TextOf du meme nom, ordre EHOLMatchStatsSeries), `Graph` (AddStatsGraph ; nom interne `StatsGraph` fixe par la base), BackButton (rangee propre). Colonnes : HeadersOf(0/1/2) (6 / 5 / 8 titres) ; cellule 0 = nom colore (AddStatsCell bColored), les autres = TableRow(...) : Resume equipe (tiret si 0) / resultat / score militaire / score eco / total ; Militaire tues / perdus / batiments perdus / armee max ; Economie nourriture / bois / pierre / or / produits / peons max / population max. Aucun texte localise en dur, sauf le tiret (voir MANQUES). Visibilites Status / TabPages et ordre Refresh = C++ (Refusal, puis attente, puis tableaux ; Refresh finit par la chronologie).

## MANQUES de la base C++ (section "A AJOUTER A LA BASE C++")
1. **Tiret "—"** : `FText::AsCultureInvariant(TEXT("—"))` du C++ ; en script `FText::FromString("—")` (chaine affichee en dur, commentaire `// MANQUE:` dans `CellText`). Souhait : cle `Dash` dans `UHOLStatsScreen::TextOf` (ou `FText UHOLStatsScreen::TeamText(int Team)`).
2. **Nom de joueur d'un slot ferme pour un repere** : le C++ nommait n'importe quel slot (`HOLForumMessages::GetPlayerName`) ; les lignes de GetWorldStatsRows excluent les slots fermes, donc un evenement d'un slot ferme aurait nom vide / blanc (commentaire `// MANQUE:` dans `BuildMarks`). Cas improbable (un slot ferme n'a pas de joueur). Souhait : `FText HOLMatchStatsRead::GetWorldSlotName(const UObject WorldContextObject, int SlotIndex)` + `FLinearColor GetWorldSlotColor(...)`.
3. **Journal par ligne de tableau** (`Stats screen: row <nom> (slot N) military .. economy .. , DescribePlayer`) : non reproduit, il faudrait `FString HOLMatchStatsRead::DescribePlayer(const FHOLPlayerMatchStatsState&)` ; verifier qu'aucun scenario / test ne le lit (non verifie). Le journal `Stats screen: timeline <serie>, N curves, M marks, <axes>` est reproduit (sans le nombre de points).
4. `FText::AsNumber` : voir suspects (l'equivalent exact du C++ ; si la liaison manque, une aide base `FText NumberText(int)`).

## Mutations pour voir chaque test rouge (une a la fois, dans le worktree, puis remettre)
- **Test_StatsScriptClass** : dans `HOLStatsScreen.as`, renommer `class UHOLStatsScreenScript` en `class UHOLStatsScreenScript2` (le nom assertit "HOLStatsScreenScript" : rouge ; ou supprimer la classe : `Found` invalide).
- **Test_StatsSummaryRow** : dans `HOLStatsRules.as::SummaryRow`, remplacer `Cells.Add(NumberCell(MilitaryScore + EconomyScore));` par `Cells.Add(NumberCell(MilitaryScore));` (Total 40 au lieu de 75, assert "the total adds the two scores" rouge) ; autre mutation : dans EconomyRow echanger `Player.Gathered.Wood` et `Player.Gathered.Stone` (Wood 30 au lieu de 50).
- **Test_StatsTimelineSeries** : dans `HOLStatsRules.as::BuildCurves`, remplacer `MakeCurve(Player, Chosen)` par `MakeCurve(Player, 0)` (la serie Militaire redonne 7 au lieu de 3 : rouge) ; autre : `ClampSeries` renvoyant `Series` tel quel (la serie 99 ne vaut plus la derniere).
Commande de la session principale (aucun build C++ n'est du lot ; recompilation script au lancement) : `quick_check -Parts tests` sur ce worktree (apres build ST-1 si la DLL n'est pas a jour : `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\build.ps1 -LiveCoding=0 -Wait`).

## NON VERIFIE / suspects de compilation script
1. **Tout** (aucune compilation script). Le test d'ecran (creation du widget) n'est pas ecrit : seuls les 3 tests demandes.
2. `FText::AsNumber(float(Value))` : liaison Angelscript non verifiee (aucun usage dans `Script\`, source du plugin non trouvee dans le worktree). Si absente : remplacer dans `CellText` (point unique) par `FText::FromString("" + Value)` (perd le groupement par culture) ou ajouter une aide base.
3. `FHOLResources` en script : champs `Food`, `Wood`, `Stone`, `Gold` supposes exposes (UPROPERTY non lus) ; sinon `EconomyRow` et les tests de Test_StatsSummaryRow (`Player.Gathered.Food = 100`) ne compilent pas.
4. Espace de noms `HOLMatchStatsRead::` (GetMilitaryScore, GetEconomyScore, MakeCurve, GetWorldStats, HasWorldStats, GetWorldStatsRows) : pris du rapport CELEBORN-1118p, aucun usage existant dans `Script\UI`. `UHOLStatsScreen::MakeMarkLabel`, `HeadersOf`, `TextOf` appeles par `UHOLStatsScreen::` (statiques, comme Settings).
5. Struct script `FHOLStatsCell` avec `FName Key;` sans initialiseur et `bool bDash = false;` ; `Key.IsNone()` suppose ; `NAME_None` suppose (non vu dans `Script\`).
6. `HOLStatsRules::BuildMarks` et `BuildCurves` renvoient `TArray<FHOLStatsGraphCurve/Mark>` (structs du module UI) : OK si exposes (CELEBORN les declare BlueprintType). `Marks[0].Color.R` est un float32 (assigne a `const float32`).
7. Hypothese des tests : `MakeCurve` donne >= 1 point pour un joueur avec 1 echantillon (X temps, Y valeur de la serie) ; si elle ajoute un point d'origine, le dernier point reste l'echantillon (le test lit le DERNIER point). A verifier au premier rouge/vert.
8. `Log(n"LogHOLUI", f"...")` avec une f-string contenant des appels de fonction namespace imbriques (`{HOLStatsRules::SeriesName(...)}`) : si le parseur refuse, mettre le resultat dans une variable avant.
9. `Status.SetVisibility(ESlateVisibility::HitTestInvisible)` / `SelfHitTestInvisible` : valeurs d'enum supposees nommees comme en C++.
10. Les appels `AddStatsGridPage()` ignorent leur retour (pose dans `Grids` par la base) ; `AddStatsSeriesButton` aussi (ajoute a `SeriesButtons`). `NativeOnInitialized` appelle `SelectSeries` donc `BP_RefreshTimeline` avant toute statistique : gere (Rows vides => retour).
11. `BP_RefreshStats` : le C++ journalisait chaque ligne ; ici non (voir MANQUE 3).

## Anomalies
Aucune. `Get-Date` PowerShell a avance de 2 min entre mes appels (clock rapide) : l'arret de 21:02 est loin. LF/CRLF : avertissement git normal, fichiers neufs en LF dans l'arbre.

## Reste (ST-3)
Bascule : `Open` pousse `FindScriptClass()`, passer la classe en Abstract, retrait de BuildLayout / Refresh / RefreshTimeline C++ et des LOCTEXT dupliques, scenario `ecran_statistiques.txt`.
