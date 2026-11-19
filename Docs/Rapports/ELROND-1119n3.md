# ELROND-1119n3 - HasResource par EHOLFaction

- Branche faction-resource-enum (worktree HeroesOfLiberty-Elrond, depuis 84f36fb). Pas de C++, pas de build, pas de quick_check.
- ROUGE 147591b : HasResource(EHOLFaction) en bouchon (vrai sauf BlackLegions) + tests. TEST_ROUGE: NON VU (rien lance). Rouges attendus : 2 (Test_FactionResourceOnlyDawnHasOne via la boucle sur l'enumeration, Test_FactionResourceTemplarsHaveNone).
- VERT ce85278 : regle = DawnOrder seule ; variante texte retiree ; HOLFactionSlot::Make(EHOLFaction, ...) ; HOLFactionSlot_Test passe "Legions"->BlackLegions, "Dawn"->DawnOrder. Seuls appelants de HasResource : HOLFactionSlot.as et ses tests ; seuls appelants de Make : HOLFactionSlot_Test.as.
- EHOLFaction : UENUM C++ (HOLMatchConfig.h:23), valeurs DawnOrder, BlackLegions, DragonChildren, ShadowCircle, FireHeirs, Templars ; deja utilisee en script.
- Perdu : le test « nom vide = Aube » (n'a plus de sens avec l'enumeration).
- A lancer par la session principale : quick_check des tests script (HOLFactionResourceRules_Test, HOLFactionSlot_Test), d'abord sur 147591b puis ce85278.
- BUILDS_DEMANDES: 0 ; TESTS_SCRIPT_AJOUTES: 1 (Test_FactionResourceTemplarsHaveNone) + 1 adapte ; TYPES_A_EXPOSER: aucun ; TESTS_TAUTOLOGIQUES: 0 ; EDITS_REFUSES: 0.
- NON VERIFIE : compilation du module script (syntaxe relue contre les usages existants de EHOLFaction : `EHOLFaction(Value)`, `int(EHOLFaction::Templars)`, f-string). Risque : surcharge supprimee, donc aucun appel texte restant (grep fait). Placeholders.txt : non concerne.
