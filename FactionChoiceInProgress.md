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
