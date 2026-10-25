# Mill in progress

**Chantier « le moulin » (D346)**, ouvert le 2026-10-22 par la session mycrusader-1d, dans la copie principale.

Décision (D346, au GDD § 21) : le moulin est le **camp de la nourriture**, façon AoE4 : dépôt de nourriture (baies pour
l'instant, fermes plus tard), 2 × 2 cases (8 m), 100 bois, 25 s, 1 000 PV, palier 0, économie, case **R** de la grille
du Paysan (`A` `R`) ; comme les camps (D312) : fini, ses bâtisseurs récoltent la source de nourriture la plus proche
à 25 m ; ralliement sur un moulin → récolte. Pas de ferme dans ce chantier.

## Étapes

| Étape | État |
|---|---|
| M.1 Décision D346 au GDD | **fait** (2026-10-22) |
| M.2 Code : ligne `Mill` de `BuildingTypes.csv`, nom et textes fr / en / ja, aide de `hol.Build.Place`, tests (camp → baies), partie `build camp` de `quick_check` | **fait** (2026-10-22, commité) |
| M.3 Scénario d'essai `Tools/Scenarios/mill.txt`, essai de l'utilisateur | **fait** (2026-10-22) : scénario `mill` ; essai de l'utilisateur **validé** (« works ») |
| M.4 Commit après validation (GDD, ce journal) | **fait** (2026-10-22). **Chantier clos.** |

## Journal

- 2026-10-22 : question « que doit être le moulin ? » → A, camp de nourriture (recommandation) ; D346 écrite au GDD
  (§ 13 palier 0, § 17, § 21, § 22 : numéro suivant D347, D345 restant réservé aux règles de combat).
- 2026-10-22 : M.2 et M.3 (scénario). Ligne `Mill` de `BuildingTypes.csv` (8 × 8 m, 400, 1 000 PV, `(Food)`, 100 bois,
  25 s, case 3 = R, palier 0, Economic), table refaite (`make_building_types.py`) ; `HOLBuildingTypeNames::Mill`, nom
  « Moulin » (`HOLGameNames.cpp`), en « Mill », ja « 製粉所 » (`fill_po.py`, GatherText → fill_po → GatherText, 179
  textes, 0 clé inconnue) ; aide de `hol.Build.Place`. **Rien à généraliser** : la logique des camps passait déjà par
  la colonne `Accepts` (`SendsBuildersToGather`, `FindCampSource`, dépôt le plus proche du `GatherProcessor`). Tests :
  `Construction.Types` (ligne Mill), `Construction.CampSource` (le moulin prend les baies, jamais un arbre),
  `CommandGrid.Tables` (cinq types, R = Mill), `CommandGrid.Categories` ; `quick_check` : nouvelle étape
  `build camp mill` (moulin Mi aux baies du slot 7, fermé, jamais épuisées : ses deux bâtisseurs cueillent, 36 dépôts
  de nourriture, tous au moulin), attentes du `build hud` (R=Mill, cellule « Moulin », infobulle « Dépôt : nourriture »).
  Résultats : tests 90/90 PASS ; `quick_check -Parts build` PASS en 309 s (premier passage : `build hud` KO sur
  l'infobulle attendue, corrigée). Scénario `Tools/Scenarios/mill.txt` (un joueur, Plain : fondation de moulin au
  bord des baies, 4 Paysans sélectionnés, trois notes). Essai de l'utilisateur à faire, puis commit (M.4).
- 2026-10-22 : essai **validé** (« works ») ; le jeu a mis ~8 min à s'ouvrir : au démarrage, Unreal lance
  `Build.bat -Mode=ValidatePlatforms`, qui attend derrière les compilations des autres copies (piste : l'éviter dans
  `play.ps1`). Commit sur `main` (M.4) ; ligne d'EtapeDefini retirée (fait). **Chantier clos.**
