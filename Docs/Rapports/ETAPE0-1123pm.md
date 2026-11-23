# ETAPE0-1123pm : plan de lancement de l'apres-midi 2026-11-23 (lecture seule, 15:02, ~24 appels)

Impact joueur : cette vague vise le premier pouvoir de faction jouable de bout en bout (portee de Lumiere sacree D890, l'IA qui plante une banniere D891, Honneur a la mort S1-e) et la passe integ-58 qui met enfin en jeu la pose de palissade en ligne, la porte et le mur de pierre.

main = fc41498. Tips releves a 15:02 : ability-columns c44ff1f, integ-55 3ddc875, specialization-trial fefe033, integ-41 6985134, integ-25 3994a82, zone-effect 5e4d9fc, socle-ai b21e145. Toutes les branches des worktrees sont a +N de main (0 derriere) sauf specialization-trial (50 derriere, base 87434bd, pas de worktree).

## Bloc a coller dans Auto_Journey_Jour.md

### Plan de lancement 2026-11-23 apres-midi
- Arwen (integ-37 = main) : lancer D890 (file l.25) portee propre de Lumiere sacree 1200 cm, sans C++ ; pris : Script/Factions/HOLDawnPowers.as, HOLFactionPowerCast.as, Script/Tests/HOLDawnPowers_Test.as, AubeInProgress.md.
- Celeborn (integ-41) : lancer D893 (l.27) 3 unites du palier 1 Fer/Foi/Pierre, sans C++ ; pris : Tools/Data/UnitTypes.csv, Content/Units/UnitTypes.uasset (regen), Placeholders.txt, placeholders_map.csv, SpecializationInProgress.md.
- Elrond (integ-25) : lancer S4-b preuve en jeu (l.49) ai_garrison : faire frapper la tour seule hors de portee de la defense, sans C++ ; pris : Tools/Scenarios/ai_garrison.{txt,look,json}, FortificationsInProgress.md.
- Faramir (zone-effect) : lancer S1-e (l.33-34) Zone.KillHonor rouge -> vert, C++ OUI (simulation Mass, D839) ; pris : HOLCombatProcessor.cpp, HOLHealProcessor.cpp/.h, HOLZoneEffectLibrary.cpp/.h, HOLZoneEffectTests.cpp, toute la simulation.
- Galadriel (socle-ai) : ne pas lancer : S2-a (l.37) est sa seule ligne ouverte et exige HOLCombatProcessor.cpp + un build de la simulation, fichier et simulation pris par Faramir (un seul agent a la fois) ; enonce de vague 2 pret, a lancer au rapport de Faramir.
- Glorfindel (ability-columns) : lancer D891 (l.26) l'IA plante une banniere avant Lumiere sacree, sans C++ ; pris : Script/AI/HOLAIBrain.as, Script/AI/HOLAIPowerCall.as, Script/Factions/HOLFactionPowerServer.as, Script/Tests/HOLAIPowerCall_Test.as.
- Sauron (integ-55) : lancer S3-b (l.45) trace IsForegroundWindow / GetMousePosition en fin de hol.Mouse.Drag, C++ OUI (entree joueur, D839 : mesure) ; pris : Source/HeroesOfLiberty/Player/HOLPlayerController.cpp.
- Haldir (integ-56 = main) : ne pas lancer d'agent : racine de la passe integ-58 (integrate.ps1 de la session principale, build + 2 quick_check, ~15-25 min, un agent y casserait l'arbre) ; ensuite recette independante dans ce meme arbre deja compile.

## Passe integ-58 (liste figee)

Commande (une ligne) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\AutoJourney\integrate.ps1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir -Branch integ-58 -Reds 065a643,eeba85e,aeb8abc -Greens cdb1b3d,3ddc875,fefe033,6985134`

DryRun joue a 15:02 : 3 rouges, 4 verts, C++ rouges = non (pas de build avant les rouges), C++ verts = oui (un build). Les verts sont donnes par SHA (pas par nom) pour que les agents lances ci-dessus puissent committer sur leurs branches sans changer la liste. Chaine de merges simulee en lecture seule (git merge-tree --write-tree, main puis cdb1b3d, 3ddc875, fefe033, 6985134) : 0 conflit a chaque etape.

### -Reds (commits test seul, tous ABSENTS de main, verifies par status.ps1 -InMain)
- 065a643 (ability-columns) : Test_PlayerCastsSacredLightAtTheHeroWithoutBanner.
- eeba85e (integ-41) : F9 porte de palissade : Test_GatePassesOnlyTheOwnersUnits, Test_GateReplacesOnlyOwnPalisadeSection, Test_PalisadeGateIsATableRow.
- aeb8abc (integ-41) : F10 : Test_StoneWallIsATableRow, Test_WoodGateDoesNotReplaceStoneWall.
- Les « 3 autres » du bilan sont 3 TESTS rouges de plus dans eeba85e / aeb8abc, pas 3 commits : Integrate_integ-57.txt l.4 et l.10 montrent 3 cherry-pick et 6 noms de tests (2144/2150).
- integ-55 (68c86d0) et specialization-trial n'ont pas de commit rouge separe (scenario, UI) ; zone-effect et socle-ai (05f4465) hors liste.

### -Greens : etat branche par branche
- ability-columns : PRETE a cdb1b3d (pas c44ff1f). cdb1b3d = CastOriginOf, GLORFINDEL-1123n : quick_check tests 2106/2108, Test_PlayerCastsSacredLight... vert, les 2 rouges restants = Test_CGTables / uasset palissade (resolus par integ-41). Script seul (aucun .cpp/.h/.uasset dans le diff). NE PAS prendre le tip c44ff1f : il porte 265f895 + e269a1c (cellule du Moine, GLORFINDEL-1123o) dont le test Test_CGMonkSealButton n'a JAMAIS ete lance (rouge non vu, build en file) ; la cellule ne s'affiche pas de toute facon (HOLGameScreen.cpp : pas de case LightSeal).
- integ-55 3ddc875 : PRETE SOUS RESERVE. Contient EndWallDrag 68c86d0 (HOLPlayerController.cpp, journal « NOT COMPILED ») + 3 commits de scenario faction_click_cast (3e9ebce, 697bfe6, b68cc78). Le build d'integ-58 le compile ; si FAIL de build, retirer integ-55 et relancer. Bilan du matin : « 8 fondations vues » apres le correctif, mais le commit se dit non compile : NON VERIFIE.
- specialization-trial fefe033 : PRETE SOUS RESERVE. 23 commits propres, D889 vu sur PNG FR/EN ; C++ UI (HOLGameScreen.cpp/.h, HOLGridCell.cpp/.h, HOLGridCellStatusTests.cpp), dernier commit 15abcb8 « not compiled » : compile par le build d'integ-58. Fusionne sans conflit dans integ-57 ce matin.
- integ-41 6985134 : PRETE. F9 + F10, tests 2138/2138 PASS + check_placeholders PASS a 11:30 (CELEBORN-1123l), BuildingTypes.uasset regenere ; porte les rouges eeba85e et aeb8abc.
- integ-25 3994a82 : EXCLUE. Seulement 37040fe (commentaire) et 6d5eea3 (scenario ai_garrison dont le preflight est FAIL) ; ne vaut pas d'entrer dans main ; Elrond la corrige ci-dessus et elle sera integree a integ-59.
- zone-effect 5e4d9fc : PAS PRETE. Zone.KillHonor ROUGE (0 mort en 200 pas, FARAMIR-1123h), merge main a302195 non compile (C++ : HOLCombatProcessor.cpp, HOLHealProcessor.*, HOLZoneEffectLibrary.*, HOLZoneEffectTests.cpp). Manque : diagnostic du test, build, Zone.KillHonor + Zone.HealHonor verts.
- socle-ai b21e145 : PAS PRETE. S2-a NON COMPILE : 05f4465 (rouge jamais vu) + 9c515ca (HOLCombatProcessor.cpp, HOLMoraleCombatTests.cpp). Manque : build, rouge vu, vert ; reste Rage necrotique (RageMs), ordre DefaultGame.ini (Morale l.43 apres Combat l.41), tirs de tour sans Rate.

### Worktree d'integration et arbres sales
- Integration : Haldir (integ-56 = main exactement). Sauron reste libre pour du C++ (sa branche est dans les verts par SHA).
- `git status --short` : Arwen (13), Celeborn (9), Haldir (8), Sauron (26) : TOUT est non suivi (`??`), AUCUN fichier suivi modifie. Arwen / Haldir / Sauron = uniquement Docs/Integrate_integ-NN.{log,txt} ; Celeborn = Docs/Integrate_integ-38..41.{log,txt} + `CELEBORN-1123d_tests.ps1` (script de test a la racine, a ne pas committer, ni a lancer : script d'un agent, L25-1). Rien a perdre : aucun travail, journaux seulement.
- Haldir a deja Docs/Integrate_integ-57.{log,txt} (non suivis) : integ-58 ecrit Integrate_integ-58.txt a cote, sans collision.

### Conflit Placeholders.txt (integ-57)
- Origine : integ-57 a echoue au merge de a2b4208 (integ-41) : « Auto-merging Placeholders.txt / CONFLICT (content) » (Integrate_integ-57.log), apres 3 merges reussis (cdb1b3d, 68c86d0, fefe033). CELEBORN-1123m avait le meme conflit en fusionnant main dans integ-41 : les deux cotes ajoutent une ligne apres « Porte de palissade » (l.74), main ajoute « Mur de pierre ... [a venir] ».
- Etat aujourd'hui : NE SE REPRODUIT PLUS. Depuis, sync_worktrees a fusionne main dans integ-41 (4c64ffe, c21809f, 6985134, sans conflit) et `git diff main integ-41 -- Placeholders.txt` est vide. Chaine simulee main + 4 verts : 0 conflit. Aucune branche du lot ne touche Placeholders.txt par rapport a sa base.
- .gitattributes : `*InProgress.md merge=union` seulement ; AUCUN attribut pour Placeholders.txt. Resolution proposee : (1) ajouter `Placeholders.txt merge=union` dans .gitattributes (commit docs sur main AVANT integ-58 ; garde les lignes des deux cotes, c'est la regle du projet) ; (2) apres integ-58, ouvrir Placeholders.txt : l'arbre fusionne contient « Mur de pierre (ligne de table provisoire, CELEBORN-1123l ...) » ET la vieille « Mur de pierre [a venir] » : doublon probable a retirer a la main (CLAUDE.md : une ligne par chose) ; check_placeholders (dans `-Parts net,build`) dira s'il le refuse. D893 (Celeborn) ajoute des lignes dans ce fichier : meme risque a son integration, d'ou l'attribut.

### Recette independante
- Worktree : Haldir, apres PASS et avance de main (integ-58 deja compile la-bas : pas de build en plus). Si integ-58 echoue, Haldir rejoue la recette sur main a fc41498 (ca4220a pour le code, binaire d'integ-56 du matin).
- Jouables sur main AUJOURD'HUI (Tools/Scenarios dans main) : ai_sacred_light (anneau dore, 6a4b2c8 dans main), faction_click (sort « refuse soon » ? non : bEffectCoded est vrai dans main, HOLDawnPowers.as:15 : le tir doit partir, sans bouton de variante cast), tower_wood (logique ; la fleche en vol n'a jamais ete vue), wall_line_long (jouable mais montrera UNE seule fondation, defaut connu, correctif 68c86d0 hors main).
- NON jouable sur main : faction_click_cast (fichiers seulement sur integ-55 : Tools/Scenarios/faction_click_cast.{txt,look,json} absents de main) ; jouable apres integ-58. wall_line_long ne passera qu'apres integ-58 (EndWallDrag).
- Apres integ-58 : faction_click_cast, wall_line_long (8 fondations attendues), puis palissade / porte / mur de pierre en pose reelle, et ai_sacred_light. Hors du lot : Sceau du Moine (bouton), Honneur a la mort.

## Lignes de la file (Auto_Journey_File.md) periimees ou a corriger (non editees)
- l.30 S1-b : fait dans main (HOLDawnPowers.as:15 bEffectCoded = true, HOLFactionPowerCast.as Decide :65) : a cocher apres relecture du verdict -Parts net.
- l.35 S4-b / lot de liaisons : (a) et (c) dans main (HOLAIGarrisonOrders.cpp:12 IssueAIGarrisonOrder, HOLAISummary.h:137, HOLCommandGrid.h:53 et HOLCommandGridReadLibrary.h:30 LightSeal, HOLBindingsLotTests.cpp) ; reste seulement (b) palier / Forum complet client.
- l.48 S4-b : appelant branche dans main (HOLAIBrain.as:86-94 appelle HOLAIGarrisonCall::Issue ; HOLAIGarrisonCall.as:80 appelle IssueAIGarrisonOrder) : ligne perimee, la preuve en jeu est l.49.
- l.79 : « relancer integ-57 » devient integ-58 (liste ci-dessus ; tip 164bf27 caduc, specialization-trial a avance a fefe033 avec merge de main).
- l.85 Test_CGTables : deja marquee « perimee tres probablement » : a archiver.
- l.32 S1-d : le texte « RESTE C++ : bouton » est vrai mais GLORFINDEL-1123o (cellule) n'est pas dans la liste integ-58 ; HOLGameScreen.cpp n'a pas de case LightSeal.

---

# Enonces par worktree

Rappels communs : brief_commun.md (un test rouge + un correctif, 15 min, 25 appels, premier Edit avant le 10e appel, pas de build sauf si dit, merge main d'abord, ne pas tuer un build, ne pas lancer le script d'un autre agent). Chaque agent met a jour EtapeDefini.md (titres seuls) et son *InProgress.md, et lit Placeholders.txt avant de clore.

## Arwen (integ-37 = main) : D890, portee propre de Lumiere sacree

- Ligne : Auto_Journey_File.md l.25 « (sans C++, D890, 2026-11-23 12:05) portee de Lumiere sacree = valeur PROPRE de 1200 cm dans une ligne a part de la table des pouvoirs (plus celle du Sceau) ». GDD.md § 22 D890. Journal : AubeInProgress.md (section S1-b / Lumiere sacree). Fiche archivee : Docs\Archive\Retours_2026-11-23\Q-sacred-light-values.md.
- Fichiers (Grep dans Arwen) : Script/Factions/HOLFactionPowerCast.as:21-23 `SealRangeCm()` (lit la ligne LightSeal de HOLUnitAbilityTable : c'est la portee 800 cm du Sceau utilisee aussi pour la Lumiere), :65 `Decide(...)`, :89 `TargetRefusal(true, DistanceCm, SealRangeCm(), ...)` ; Script/Factions/HOLDawnPowers.as:8-18 (`Rows()` : ligne SacredLight, Cost 25, CooldownMs 20000, bEffectCoded) ; appels : Script/Factions/HOLFactionPowerServer.as:49 (Decide) et Script/AI/HOLAIBrain.as:165 (`HOLFactionPowerCast::SealRangeCm()` pour la zone de l'IA) a garder coherents.
- Test rouge : `Test_SacredLightRangeIs1200` dans Script/Tests/HOLDawnPowers_Test.as (fichier existant : Test_SacredLightFromFullBar l.5, Test_SacredLightCooldown l.36) : la ligne SacredLight de HOLDawnPowers::Rows() porte une portee de 1200 cm et Decide refuse a 1300 cm, accepte a 1100 cm (le Sceau reste a 800). Rouge d'abord : ajouter un champ de portee a la ligne (stub qui compile) puis correctif. Verifier ou est FHOLPowerRow (Grep `struct FHOLPowerRow`).
- C++ : NON (script).
- Pris : HOLDawnPowers.as, HOLFactionPowerCast.as, HOLDawnPowers_Test.as, AubeInProgress.md. Ne pas toucher HOLFactionPowerServer.as ni HOLAIBrain.as (Glorfindel, D891) : si Decide change de signature, le dire dans le rapport au lieu de modifier ces fichiers.

## Celeborn (integ-41) : D893, 3 unites du palier 1 des voies de commanderie

- Ligne : Auto_Journey_File.md l.27 « (D893, 2026-11-23 12:05) creer les 3 unites du palier 1 des voies de commanderie Fer / Foi / Pierre (PROVISOIRES : lignes UnitTypes.csv, mannequin, cout ; lignes dans Placeholders.txt), PUIS leur deblocage ». Cette vague : SEULEMENT les 3 lignes de donnees + placeholders (le deblocage ChosenOption sera la vague suivante). Journal : SpecializationInProgress.md ; constat de GALADRIEL-1123b dans Retours\Q-spec-voies.md (UnitTypes.csv = 18 unites, les 9 de commanderie absentes).
- Fichiers : Tools/Data/UnitTypes.csv (19 lignes dont l'en-tete ; colonnes Name,DisplayName,Radius,Speed,MaxHealth,AvoidanceDistance,Mesh... ; modele : la ligne LightInfantry, mannequin /Game/Units/Mannequin/SM_Mannequin) ; Content/Units/UnitTypes.uasset (regenere par Tools\AutoJourney\regen_tables.ps1 : a lancer par l'agent seulement si l'enonce du jour le dit, sinon laisser « regen a faire (integrate) » dans le rapport) ; Placeholders.txt (+ placeholders_map.csv) ; deblocage futur : Script/Specialization/HOLSpecializationRules.as:109 `ChosenOption`, HOLSpecializationBoardScript.as:162.
- Test rouge : `Test_SpecTier1UnitsExist` dans un fichier neuf Script/Tests/HOLSpecTier1Units_Test.as : les 3 noms de ligne choisis (a fixer par l'agent d'apres le GDD § Specialisation / Q-spec-voies ; une par voie Fer, Foi, Pierre) existent dans la table des unites avec cout et PV > 0. Trouver d'abord par Grep comment un test script lit UnitTypes (ex. Test_CGTables, HOLCommandGridRules_Test.as). Si aucune liaison de lecture n'existe : s'arreter et le rapporter (pas de C++ sur cette ligne).
- C++ : NON. Valeurs PROVISOIRES (cout, PV, vitesse copiees de LightInfantry / mannequin), chaque ligne dans Placeholders.txt.
- Pris : Tools/Data/UnitTypes.csv, Content/Units/UnitTypes.uasset, Placeholders.txt, placeholders_map.csv, SpecializationInProgress.md, Script/Tests/HOLSpecTier1Units_Test.as. SEUL agent autorise a editer Placeholders.txt dans cette vague. Commits sur integ-41 (tip 6985134 deja fige par SHA dans integ-58).

## Elrond (integ-25) : preuve en jeu de la garnison de l'IA (ai_garrison)

- Ligne : Auto_Journey_File.md l.49 « (S4-b preuve en partie, ELROND-1123j, integ-25, sans C++) scenario ai_garrison... preflight FAIL : AUCUN 'Garrison order: slot 1' en 96 s... RESTE : faire frapper la tour seule par un groupe hors de portee de la defense, ou lecteur de log du cerveau pour la garnison ». Journal : FortificationsInProgress.md (S4-b) ; rapport Docs\Rapports\ELROND-1123j.md.
- Fichiers : Tools/Scenarios/ai_garrison.txt, .look, .json (existent sur integ-25 seulement, 6d5eea3) ; l'appelant est dans main : Script/AI/HOLAIBrain.as:86-94 (HOLAIGarrisonCall::Issue), Script/AI/HOLAIGarrisonCall.as:68-80 (IssueAIGarrisonOrder), Source/HeroesOfLiberty/AI/HOLAIGarrisonOrders.cpp:12. NB : HOLAIBrain.as:94 passe `NoHits` (pas de lecteur de coups) ; HOLAISummary lit SecondsSinceHit (10db129, dans main).
- Test rouge : le scenario lui-meme (L20-8 : rouge = `waitlog "Garrison order: slot 1"` en preflight, deja rouge) ; correctif = un petit groupe (2-3 Archers) place hors de la portee de la defense de l'IA qui frappe la tour seule ; verifier au journal « Garrison order: slot 1 » puis PNG de la tour garnie. Si le journal montre que NoHits empeche tout appel, rapporter (le correctif serait un lot de script dans HOLAIBrain.as : a decider).
- C++ : NON.
- Pris : les 3 fichiers ai_garrison.*, FortificationsInProgress.md. Aucun fichier de Source.

## Faramir (zone-effect) : S1-e, Honneur a la mort (Zone.KillHonor)

- Ligne : Auto_Journey_File.md l.33-34 (S1-e, FARAMIR-1123g/h) « Honneur a la MORT : GDD D859 = ennemi tue a moins de 12 m d'un BATIMENT de l'Aube ; ... test Zone.KillHonor ROUGE (0 mort en 200 pas avec le centre de l'Aube a 10 m) ». Journal : AubeInProgress.md ; rapports FARAMIR-1123g.md / 1123h.md.
- Fichiers (Grep dans Faramir) : Source/HeroesOfLiberty/Simulation/HOLCombatProcessor.cpp:1536 (lambda `AwardKillHonor`), :1555 (appel `UHOLHealProcessor::CreditKillHonor(World, Slot)`), :1571 (`AwardKillHonor(Victim)`) ; Source/HeroesOfLiberty/Simulation/HOLHealProcessor.cpp:98 (`CreditKillHonor`) et HOLHealProcessor.h:108 ; Source/HeroesOfLiberty/Tests/HOLZoneEffectTests.cpp:405-407 (`HeroesOfLiberty.Zone.KillHonor`, FHOLZoneEffectKillHonorTest).
- Diagnostic attendu : pourquoi aucun mort en 200 pas (la victime est-elle assez fragile / a portee ? l'AwardKillHonor n'est-il appele que sur le chemin de mort par melee ?). Rouge deja la : Zone.KillHonor (vu 11:18 : 2099/2100). Correctif : le test (placer l'attaquant a portee, PV de la victime bas) puis, si le test est bon et reste rouge, le code. Un build du lot C++ de la seance, un seul, par `Tools\build.ps1 -LiveCoding=0 -Wait -Priority merge` ; pas de build sans que l'enonce du jour le dise (il le dit ici : un build, puis les tests Zone.*).
- C++ : OUI, raison D839 : processeur Mass (code appele par entite et par pas) et tests de la simulation (D838 : restent en C++).
- Pris : HOLCombatProcessor.cpp (EXCLUSIF dans toute la vague), HOLHealProcessor.cpp/.h, HOLZoneEffectLibrary.cpp/.h, HOLZoneEffectTests.cpp, et toute la simulation (aucun autre agent sur Source/HeroesOfLiberty/Simulation/). AubeInProgress.md.

## Galadriel (socle-ai) : NE PAS LANCER en vague 1 ; enonce de vague 2

- Raison : la seule ligne ouverte de socle-ai est S2-a (l.37) ; son diff touche Source/HeroesOfLiberty/Simulation/HOLCombatProcessor.cpp et Tests/HOLMoraleCombatTests.cpp, fichier et simulation donnes a Faramir pour toute la vague (un seul agent a la fois). Les autres lignes du moral (S2-b a S2-d) dependent de S2-a.
- Enonce vague 2 (a lancer au rapport de Faramir) : ligne l.37 (S2-a, C++ Mass), journal MoraleInProgress.md (« morale journal, S2-a step 5 », 4afd2cc) ; fichiers : HOLCombatProcessor.cpp (lectures Dealt / Rate du moral du frappeur, 9c515ca) et HOLMoraleCombatTests.cpp (05f4465) ; etapes : merge main, UN build (Tools\build.ps1 -LiveCoding=0 -Wait -Priority merge), voir le rouge 05f4465 sur le code d'avant 9c515ca puis le vert avec 9c515ca (ou le rouge via un bouchon), puis tests Morale* ; reste ensuite Rage necrotique (RageMs), ordre DefaultGame.ini (Morale l.43 apres Combat l.41, le plan dit avant), tirs de tour sans Rate. C++ OUI (Mass). Pris : HOLCombatProcessor.cpp et la simulation.

## Glorfindel (ability-columns) : D891, l'IA plante une banniere

- Ligne : Auto_Journey_File.md l.26 « (comportement IA neuf, D891, 2026-11-23 12:05) l'IA PLANTE UNE BANNIERE (origine de lancer de Lumiere sacree ; le joueur garde la banniere) ; le repli centre de l'armee (OriginOf) reste jusque-la ; test rouge d'abord ». Journal : AubeInProgress.md.
- Fichiers (Grep dans Glorfindel, branche ability-columns, cdb1b3d inclus) : Script/AI/HOLAIBrain.as:125 `ThinkPowers(...)`, :161-162 `CastOrigin = HOLFactionPowerServer::CastOriginOf(HOLHero::GetHeroBannerLocation(PlayerIndex), ArmyPoints)` (repli centre de l'armee quand la banniere vaut (0,0)), :165 portee ; Script/Factions/HOLFactionPowerServer.as:11-15 `CastOriginOf`, :25 commentaire, :37 appel du joueur ; Script/Tests/HOLAIPowerCall_Test.as:92 `Test_AIBrainCastsWithoutAPlantedBanner` (a RENOMMER ou conserver : il decrit l'ancien repli), :115 `Test_AIBrainCastsSacredLightWhenHonorAllows`. Trouver par Grep dans Script/Hero (ou HOLHero) la fonction qui plante la banniere du joueur (celle que lit `HOLHero::GetHeroBannerLocation`).
- Test rouge : `Test_AIPlantsABannerBeforeCasting` dans HOLAIPowerCall_Test.as : apres un ThinkPowers de l'IA (Honneur suffisant), GetHeroBannerLocation(slot IA) n'est plus (0,0) ; rouge d'abord (le code n'en plante pas), correctif dans HOLAIBrain.as (planter la banniere a l'arrivee de l'IA, ou au premier ThinkPowers, avant CastOriginOf). Valeur par defaut : position = centre des unites vivantes de l'IA (comme le repli) ; si l'API de plantation n'existe pas en script : rapporter le C++ manquant, ne pas en ecrire.
- C++ : NON.
- Pris : HOLAIBrain.as, HOLAIPowerCall.as, HOLFactionPowerServer.as, HOLAIPowerCall_Test.as, AubeInProgress.md. Ne pas toucher HOLFactionPowerCast.as ni HOLDawnPowers.as (Arwen, D890).

## Sauron (integ-55) : S3-b, cause du curseur sans sol en fin de glisser

- Ligne : Auto_Journey_File.md l.45 « S3-b (ARWEN-1123g) : cause du curseur sans sol en fin de glisser (GetCursorFraction exige Viewport->IsForegroundWindow, HOLPlayerController.cpp:1019 ; fenetre cachee / capture hors ecran ; le fantome gele a 8 cases) : tracer IsForegroundWindow / GetMousePosition a la fin de hol.Mouse.Drag, puis ligne de 16+ cases ». Journal : WallLineInProgress.md (ou le journal cite par la ligne : Grep `EndWallDrag` dans Docs\*InProgress.md).
- Fichiers (Grep dans Sauron) : Source/HeroesOfLiberty/Player/HOLPlayerController.cpp:1019 (`if (!Viewport || !Viewport->IsForegroundWindow() || SizeX <= 0 || SizeY <= 0 || !GetMouseP...`) ; EndWallDrag 68c86d0 est sur cette branche (meme fichier : un seul agent). Scenario : Tools/Scenarios/wall_line_long.txt.
- Test rouge : le scenario (pas de test C++ sans viewport, L20-8) : un `waitlog "Drag end: foreground="` ajoute a wall_line_long.txt, rouge tant que la trace n'existe pas ; correctif = UE_LOG de IsForegroundWindow, de la taille du viewport et de GetMousePosition a la fin de hol.Mouse.Drag. Un build (lot C++), puis preflight wall_line_long.
- C++ : OUI, raison D839 : mesure / diagnostic de l'entree joueur (controleur) ; le build d'integ-58 compile deja le meme fichier a Haldir : la file des builds peut retarder l'agent (L25-5 : 2 agents C++ au plus, Faramir est l'autre).
- Pris : HOLPlayerController.cpp (EXCLUSIF), Tools/Scenarios/wall_line_long.*. Aucun autre agent sur Source/HeroesOfLiberty/Player/.

## Haldir (integ-56 = main) : racine de integ-58, puis recette

- Pas d'agent de travail pendant integ-58 : l'arbre est a main, la passe fait `switch -c integ-58 main`, cherry-pick, reset --hard, 4 merges, un build, 2 quick_check. Le journal sort dans Haldir\Docs\Integrate_integ-58.txt (les 8 `??` existants n'y gênent pas).
- Apres PASS et avance de main (git -C ... merge integ-58 ou branche-sur-main selon le protocole), agent RECETTE independant (un autre que les auteurs) dans Haldir : scenarios dans l'ordre faction_click_cast (geste reel, tir au sol), wall_line_long (8 fondations), palissade / porte / mur de pierre en pose, ai_sacred_light, tower_wood ; fiches E-* LOGIQUE SEULEMENT tant que le geste est interne ; PNG vus obligatoires.
- Si integ-58 echoue (build integ-55 ou fefe033 non compiles) : retirer la branche fautive de -Greens, relancer, et Haldir joue la recette sur main courant (ai_sacred_light, faction_click, tower_wood).

## Questions ouvertes et suites (vague 2, prepares)

- D894 (l.28) colonne « faction » separee de FPartHashes : C++ simulation (Source/HeroesOfLiberty/Simulation/HOLSimulationSubsystem.cpp:45 `struct FPartHashes`, :384 `FPartHashes Parts;`) : apres Faramir et Galadriel (un seul agent sur la simulation).
- D895 (suivi mutuel : le dernier ordre donne gagne) et D892 (palissade / porte / mur : A pour les trois, valeurs codees) : GDD.md:3742, :3748 ; D895 : a chercher cote script (Follow) avant de proposer une ligne.
- Celeborn tower_wood (l.43) : journal de tir + ArrowScale = HOLCombatProcessor.cpp:953/:2751 + HOLArrowSubsystem.cpp:30 : apres Faramir (fichier).
- Bouton du Moine (l.32 S1-d) : C++ HOLPlayerController.cpp ClickGridCell (2799-2840) + HOLGameScreen.cpp RefreshGrid : apres Sauron (fichier).

## NON VERIFIE
- Les noms exacts de la fonction qui plante la banniere (D891), de FHOLPowerRow (D890), du lecteur de UnitTypes en script (D893) : l'agent les trouve par Grep (dit dans chaque enonce).
- EndWallDrag 68c86d0 et 15abcb8 compilent : ils ne le sont pas d'apres leurs commits, c'est le build d'integ-58 qui le dira.
- Les 24 appels de cette etape sont comptes de memoire ; la derniere heure lue est 15:02.
