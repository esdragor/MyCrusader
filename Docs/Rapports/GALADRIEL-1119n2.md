# GALADRIEL-1119n2 : vue pure de l'emplacement de faction (A6a, D201, D823)

- Branche `faction-slot` (worktree HeroesOfLiberty-Galadriel), base c7067cf. ROUGE 317abf0 (stub qui compile + 4 tests), VERT 84f36fb (vraie vue).
- Fichiers neufs : `Script\UI\HOLFactionSlot.as` (structs `FHOLFactionPowerBox`, `FHOLFactionSlotView`, namespace `HOLFactionSlot` : `Make(Faction, Resource, Rows, CooldownLeftMs, Tier, bHasSpecializationButton, bTierOpen)`), `Script\Tests\HOLFactionSlot_Test.as`. Aucun fichier existant modifie.
- Regles reutilisees, pas recopiees : `HOLFactionResource::HasResource` (jauge ou non), `HOLPowerBook::StateOf` et `::Describe` (etat et ligne de chaque case).
- Vide = pas de ressource ET pas de bouton de specialisation. Les cases de pouvoir n'existent qu'avec une jauge (elles depensent la ressource). Bouton allume = bouton montre ET palier ouvert.
- Decision a confirmer (non tranchee ailleurs) : `bHasSpecializationButton` est une entree separee de `bTierOpen` (le brief ne disait pas quelle faction montre le bouton).
- Tests (4) : Test_FactionSlotEmptyForFactionWithoutResource, Test_FactionSlotShowsGaugeText, Test_FactionSlotPowerBoxesFollowBookStates, Test_FactionSlotSpecializationButtonSharesTheSlot. Sur 317abf0 les 4 doivent etre rouges (stub : visible a tort, textes et cases vides).
- NON VERIFIE : rien compile ni execute (pas de build, pas de quick_check). Syntaxe alignee sur HOLHeroBlock.as ; risques : `T.AssertEquals(int, ...)`, comparaison d'enum via AssertTrue (usage habituel), `float(int)`.
- Commande a lancer par la session principale : quick_check sur la partie script (Tests) apres sync de la DLL ; rouge a voir sur 317abf0, vert sur 84f36fb.
- Branchement a venir : hote du widget dans `UHOLScriptScreen` / ecran de jeu en script (emplacement au-dessus du bloc du heros, D201) qui appelle `Make` a chaque rafraichissement et applique la vue ; etat replique de la jauge (valeur, plafond, palier, recharges par pouvoir, palier de specialisation ouvert) en C++ (replication) a fournir ; table de textes du jeu pour les libelles ; position, taille, icones a trancher.
- BUILDS_DEMANDES: 0. TESTS_SCRIPT_AJOUTES: 4 (noms ci-dessus). TYPES_A_EXPOSER: aucun. EDITS_REFUSES: 0. PROCESSUS_DETACHES: aucun lance. Placeholders.txt : rien d'ajoute (aucun widget).
