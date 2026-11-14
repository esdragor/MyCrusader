# Choix de faction et cheat des Templiers (FactionChoiceInProgress.md)

Worktree Arwen (branche `d751-root`). Agent JEU (C++) ; l'agent MANAGER fait la commande `faction` côté serveur (`Manager\`, `PROTOCOL.md`).

## Décisions de l'utilisateur (2026-11-13, en conversation)

- Liste des factions de « Nouvelle partie » **dégrisée** (lève le gris de D495 pour la faction), en solo et au salon.
- Choisissables : Ordre de l'Aube, Légions Noires. **Enfants du Dragon, Cercle de l'Ombre, Héritiers du Feu : visibles mais grisés.**
- **Templiers : absents de la liste** (pas grisés : le cheat reste secret) tant que le cheat n'est pas activé.
- **Cheat = triple clic sur le titre « Heroes of Liberty » de l'écran-titre** ; un triple clic de plus le désactive, sans retour visuel.
- Valable pour **la session de jeu seulement** (rien de sauvegardé).
- Au salon : un joueur sans cheat voit « Templiers » sur la ligne d'un autre (ou d'une IA du chef) mais ne peut pas le choisir pour lui.
- Les Templiers n'ont aucune règle propre : ils se jouent comme l'Aube (`HOLFactionRules::ResolveKey`), voulu pour l'instant.

## Contrat avec le Manager

Le champ optionnel `faction` (nom de `EHOLFaction`) de `PUT /lobbies/{id}/me` et de `PUT /lobbies/{id}/slots/{n}` (IA). Le Manager n'accepte que
`DawnOrder`, `BlackLegions`, `Templars` (sinon 400 `bad_request`) ; il fait confiance au client pour le cheat. La valeur se relit dans `slots[].faction`.

## Pièces

- `Match/HOLFactionChoice.*` : règles pures (listée / choisissable / Sanitize / noms / drapeau de session / compteur de clics).
- `Tests/HOLFactionChoiceTests.cpp` : `HeroesOfLiberty.Match.FactionChoice.*`, `HeroesOfLiberty.Net.FactionChoice.Bodies`.
- `HOLTitleScreen` : triple clic sur `Title` ; `HOLNewGameScreen` : liste, `HandleFactionChosen`, salon.

## Journal

- 2026-11-13 19:2x : premier test rouge écrit (stubs de `HOLFactionChoice`, corps de requête avec `faction` déjà codés), commit 7371e9f, build lancé.
- 2026-11-13 19:5x : rouge constaté (3 tests FactionChoice KO, 1223/1226), puis vert : build PASS, tests 1238/1238, `quick_check -Parts smoke` PASS, `-Parts lobby` PASS (après `net_match.ps1 -StageOnly` : le serveur compilé était plus vieux que la dll).
- Solo : liste de 5 entrées (6 avec le cheat), Dragon / Ombre / Feu grisés, `HandleFactionChosen`, lancement avec `Sanitize` ; captures vues (`Tools\Tests\faction_shots.ps1`, `faction_A_without` : 5 entrées dont 3 grisées, `faction_B_with` : 6 entrées avec « Templiers » en dernier) ; log : Dragon refusé, Légions prises, Templiers pris avec le cheat, « Templiers » toujours affiché après le retrait du cheat.
- Salon : client prêt (champ `faction` de `PUT /me` et de `PUT /slots/{i}`, lecture générale de `Row->Faction`, liste active sur ma ligne et sur les IA du chef, grisée sinon, ligne Templiers affichée sans le cheat) ; le Manager a livré sa commande (4043c18). NON FAIT : test de bout en bout du choix de faction au salon (la partie `lobby` de quick_check n'a pas d'étape faction) ; à ajouter.
- Triple clic réel sur le titre : NON vérifié à l'écran (la console n'injecte pas de clic) ; son compteur est testé (`FactionChoice.TripleClick`), le branchement `NativeOnMouseButtonDown` + `Title` rendu hit-testable est à essayer à la main.
- Correctif au passage : `HOLSelectionMarkers.cpp` incluait `HOLRageVisual.h` avant son propre en-tête (build FAIL « Expected ... first header »), ordre inversé (5f0266e).
- Décisions posées sans trancher : aucune ; à noter : le Manager remet « Prêt » à faux quand la faction change (son choix 4, comme couleur et équipe).
- 2026-11-13 19:1x (suite, sans l'utilisateur) : **triple clic réel vérifié** : commande `hol.UI.Click <Widget> [N]` (`Dev/HOLUIClick.cpp`, route la pression et le relâchement par le test de survol de Slate jusqu'à `NativeOnMouseButtonDown`), `faction_shots.ps1` : 2 clics = rien, 3 = « Title screen: faction cheat on », 3 de plus = off, 3 de plus = on, puis Nouvelle partie : liste à 6 entrées (capture `faction_B_with`), cheat coupé : liste à 5 (`faction_A_without`), ligne Templiers affichée « Templiers » ; Dragon refusé, Légions prises. Un clic sur « Jouer » a été essayé et NON gardé (après lui `hol.UI.Text` / `Choose` ne trouvaient pas les widgets du nouvel écran, cause non cherchée) : l'écran est ouvert par `hol.UI.Open NewGame`.
- **Salon de bout en bout** : `lobby_match.ps1 -Faction` (et `quick_check -Parts lobby`, qui le lance après la partie existante). Alice (cheat actif) crée un salon Moyenne, prend Légions puis Templiers, met une IA en slot 2 et la passe en Légions ; Bob (sans cheat) voit « Templiers » sur la ligne d'Alice, sa liste n'a pas les Templiers, le Dragon est grisé, `hol.UI.Choose` forcé sur la ligne d'Alice n'envoie rien (garde `HandleFactionChosen`) ; Launch ; `match.json` = Human/Templars, Human/DawnOrder, AI/BlackLegions ; le serveur de jeu logue `Match factions: slot 0 Templars, slot 1 Dawn, slot 2 Legions` (`GetSlotFactionKey`). Rouge d'abord (`Lobby: factions` absent, Bob en timeout), puis PASS. Piège rencontré : `Saved\Manager\HOLManager.exe` était antérieur à la commande `faction` (la réponse gardait `DawnOrder`) : `Tools\Manager\build_manager.ps1` à relancer avant la partie `lobby`.
- Le refus du Dragon en 400 est testé côté Manager (`LobbyFactionTests`, 21 tests verts, dont `DragonChildren`) ; la liste du jeu le grise avant toute requête.
- 2026-11-14 09:2x-09:4x (essai à la main de l'utilisateur, 3 défauts trouvés, tous corrigés, **MARCHE** selon l'utilisateur) : (1) fenêtre du triple clic portée de 0,8 s à 2 s (`FClickCounter::Window`, tests du compteur adaptés) ; (2) la liste des factions de Nouvelle partie restait celle d'avant le cheat : la pile d'écrans réutilise l'écran fermé, `UpdateFactionLists` n'était appelée que par `Refresh` ; ajout de `NativeOnActivated` qui la rappelle ; (3) clics rapides perdus : Slate envoie le 2e clic d'une paire rapide en double-clic, pas en appui ; `NativeOnMouseButtonDoubleClick` compte aussi (`CountTitleClick`). Les clics simulés de `hol.UI.Click` ne font jamais de double-clic : leur test ne pouvait pas voir (3). Build PASS ; `quick_check -Parts tests` 1238/1238 après (1) et (2) ; NON relancé après (3). Pas de test automatique pour (2) ni (3).
- 2026-11-14 : **DANS main** (fast-forward 18a7a5d). `main` fusionné d'abord dans `d751-root` (sans conflit), build PASS, `quick_check -Parts tests` 1257/1257. Le dossier principal avait des modifications non commitées d'autres sessions (renommages de `Retours\`, journaux) : elles sont restées intactes, `git merge d751-root` direct dans `main` refusait de les écraser, d'où la fusion par `d751-root` puis `--ff-only`.
