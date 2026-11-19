# ELROND-1119p - correcteur de script, HUD H-5b (fil de messages)

Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, branche tiers, tip de départ a41238e.

## Erreurs de compilation Angelscript

Aucune. Les scripts écrits à l'aveugle (HOLMessageFeed.as, HOLMessageFeed_Test.as, crochet de HOLGameScreenScript.as, liaisons de UHOLScriptScreen) compilent tels quels. Les trois soupçons de l'auteur ne se sont pas réalisés : `TArray<bool>&` / `TArray<float32>&` en sortie, `UBorder.IsHovered()`, retour `UVerticalBox`. Aucune ligne de C++ à changer.

## Comptes successifs (quick_check -Parts tests)

| Heure | Étape | Compte |
|---|---|---|
| 09:24-09:26 | départ, 5 tests du fil | 1619/1619 PASS (1614 + 5) |
| 09:26-09:28 | après remplacement de la garde de recopie par un test de bornes | 1620/1620 PASS |
| 09:29-09:31 | ROUGE VU, deux valeurs faussées | 1618/1620 FAIL |
| 09:31-09:33 | valeurs remises | 1620/1620 PASS |

Rouge vu (fin de run 09:31:27) : `MaxLines()` attendu 6 au lieu de 5 dans Test_MessageFeedLinesAndListFollowTheThreadCount, `TopPadding(3)` attendu 3.0 au lieu de 2.0 dans Test_MessageFeedClicksAndPlaceAreTheCppOnes. Exactement ces deux tests sont tombés (N-2/N). Aucune valeur faussée ne reste dans le commit.

## Test tautologique

Test_MessageFeedClicksAndPlaceAreTheCppOnes finissait par trois assertions de recopie de constantes (`LineOpacity` 0.8, `PaddingX` 8, `PaddingY` 2). Elles sont retirées, avec un commentaire qui dit que la capture prouve ces valeurs. Le reste du test passe par de vraies règles (LineTakesClick, PieceIsName, TopPadding).

Test neuf Test_MessageFeedHitTestStaysInsideTheShortestList (HOLMessageFeed_Test.as) : HoveredAlertLine avec un compte de 4 sur des listes de 2 et 3 entrées (pas de hit, pas de lecture hors bornes), puis hit une fois l'entrée de survol ajoutée, puis compte 0.

## Commit

278ccdd (anglais, `git add` du seul HOLMessageFeed_Test.as, pas de push).

## Reste / anomalies

- Rien à corriger côté script. Aucun test hors fil devenu rouge.
- Anomalie : Get-Date a répondu 09:24 au lancement, alors que le brief annonce un lancement à 09:31 et un build à 09:29. Les heures ci-dessus sont celles de l'horloge de la machine.
- Une première tentative de relance en parallèle d'une édition a été refusée par le classifieur ; relancée seule, sans effet sur les résultats.
- Les tests ne vérifient que les règles pures ; le rendu réel du fil (liaisons C++ GetMessageState, GetMessageLine, clic) reste à voir en capture.
