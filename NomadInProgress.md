# NomadInProgress.md : l'option Nomade (GDD § 14.1 et § 14.3 ; D207, D354, D402, avec D347, D536, D702)

Ouvert le 2026-11-12 (séance autonome, HALDIR-1112g, worktree Haldir, branche `maps`, chapitre A3 du relevé du jour). Méthode des
dossiers par feature : règles tirées du GDD, plan en étapes de 15 min, test rouge d'abord, au plus 2 questions
(`C:\Users\baudo\Documents\MyCrusader\Retours\Q-nomad.md`). Règle D483 : règles pures en Angelscript ; le départ, l'état répliqué et la
défaite côté serveur en C++.

## Règles tirées du GDD (rien à trancher)

1. **Départ** (D207, façon AoE2) : **sans centre principal** ; les Paysans sont **dispersés en petits groupes à des endroits aléatoires
   de la carte** ; **le héros apparaît avec l'un des groupes** ; avec Régicide, **le Roi apparaît avec un travailleur**, pas forcément
   avec le héros. Vigilance : **distance minimale entre les groupes de joueurs**. Écarté : le Nomade d'AoE4 (position habituelle).
2. **Stock** (D402) : **le prix d'un forum est ajouté au stock de départ** ; pas de forum offert ; le joueur le pose où et quand il veut
   et le construit normalement, ou dépense autrement à ses risques (D354).
3. **Défaite** (D354) : **tant qu'aucun forum n'est achevé, le joueur perd s'il n'a plus aucun Paysan.** Écartés : délai pour bâtir,
   vies dès le départ.
4. **Forum en chantier détruit** (D354, D402) : **pas de vie perdue** ; ses ressources sont perdues.
5. **Dès le premier forum achevé** : les règles normales (le forum à vies, D347 ; vies selon le palier).
6. **Héros avant le premier forum** (D354) : il réapparaît **auprès du groupe de Paysans le plus nombreux**.
7. **Sites sacrés** (D702) : aucun groupe de départ dans la zone d'un site.
8. La graine de partie (`FHOLMatchConfig`, D244) sert au placement (reproductible).

Lectures faites sans décision : le premier forum achevé est noté une fois pour toutes (s'il tombe ensuite, c'est le forum à vies, pas
D354) ; à égalité de taille, le premier groupe (même réponse sur toutes les machines).

## Ce qui existe (relevé du 2026-11-12)

- **La case du menu et du salon** : `FHOLMatchConfig::bNomad` (`Source/HeroesOfLiberty/Match/HOLMatchConfig.h:170`), liste déroulante
  `NomadDropdown` (`Source/HeroesOfLibertyUI/Screens/HOLNewGameScreen.cpp:312`, lue l. 955), champ `nomad` du salon
  (`Net/HOLManagerClientRules.cpp:253`, `:359`), testée par `HOLMatchConfigTests.cpp:111`. **Aucune règle de jeu ne la lit.**
- **Défaite à réutiliser** (pas à recopier) : `AHOLGameState::DefeatSlot(Slot, EHOLDefeatReason)` (`Match/HOLGameState.h:94`, serveur,
  décide la fin de partie D357, journal « Victory: slot N defeated ») ; `EHOLDefeatReason` (Forum, King, Disconnected, SacredSites... :
  N2 y ajoute `Nomad`) ; `IsSlotDefeated` ; vies du forum `HOLForumRules::GetLives` (`Match/HOLForumRules.h:238`), règles de victoire
  `HOLVictoryRules` (`Match/HOLVictoryRules.h:73`).
- `VictoryInProgress.md` range Nomade « plus tard, avec le départ Nomade » (l. 24, l. 560).

## Plan (étapes de 15 min)

- **N0 relevé** : FAIT (ce fichier, 2026-11-12).
- **N1 règles pures en script** : `Script/Nomad/HOLNomadRules.as` (`FHOLNomadState`, `NewState`, `NoteForumComplete`,
  `IsBeforeFirstForum`, `IsDefeated`, `FallCostsLife`, `StartingAmount`, `HeroBackAtLargestGroup`, `LargestGroup`, `SplitIntoGroups`,
  `FarEnough`) et `Script/Tests/HOLNomadRules_Test.as` (7 tests). Valeurs au registre (`nomad.group_size`, `nomad.min_enemy_distance_cm`).
- **N2 départ sans forum (C++, agent C++)** : avec `bNomad`, pas de forum principal au départ ; Paysans en groupes (`SplitIntoGroups`)
  placés par la graine, `FarEnough` des autres camps et hors des zones de sites (D702) ; héros avec un groupe, Roi avec un travailleur ;
  stock = `StartingAmount` par ressource avec le prix du forum lu dans les données ; état `FHOLNomadState` par emplacement, noté à
  l'achèvement du premier forum ; mort d'un Paysan : `IsDefeated` puis `DefeatSlot(Slot, EHOLDefeatReason::Nomad)` ; chute d'un forum en
  chantier : `FallCostsLife(false)`, aucune vie ; réapparition du héros au `LargestGroup`. Tests C++ ou `net_match` sur l'effet (slot
  vaincu au dernier Paysan tué, pas avec un forum achevé).
- **N3 IA** : l'IA en Nomade regroupe ses Paysans et pose son forum tôt (modèle `HOLAITalent.as`, économie IA1).
- **N4 scénario** : `Tools/Scenarios/nomad.txt` (geste réel, départ dispersé, dernier Paysan tué) ; essai VU À L'ÉCRAN seulement.

## Premier test rouge (N1)

`Script/Tests/HOLNomadRules_Test.as` : 7 tests : dernier Paysan mort sans forum = défaite ; avec un forum achevé = non, pour toujours ;
sans l'option jamais ; forum en chantier détruit sans vie ; prix du forum ajouté au stock ; héros au plus gros groupe ; petits groupes
et distance minimale.

## Questions (`Retours\Q-nomad.md`, défauts codés)

- Q1 : taille des groupes de départ. Défaut A = 2 au plus (`GroupSize = 2`, 5 Paysans = 2, 2, 1).
- Q2 : distance minimale à un groupe ennemi. Défaut A = 40 cases (`MinEnemyDistanceCm = 4000`).
- **Réponses de l'utilisateur (2026-11-12, D786)** : Q1 = **B, 1 Paysan par groupe (cinq Paysans seuls)** (À CHANGER : `GroupSize = 1` dans `HOLNomadRules.as` et ligne `nomad.group_size` de `ValueSheet.csv`, test à adapter) ; Q2 = **A, 40 cases** (déjà codé).

## Suivi

- 2026-11-12 11:59-12:08 (HALDIR-1112g, `maps`, sans C++) : N0 relevé ; test rouge N1 commité 938087b (quick_check `tests` 12:02 :
  « Identifier 'FHOLNomadState' is not a data type », 0 test lancé). **N1 VERTE** : quick_check `tests,values` 1200/1200 PASS (242 s),
  `values` PASS (2 lignes au registre `nomad.group_size`, `nomad.min_enemy_distance_cm` ; Valeurs.md/.csv régénérés). Questions
  `Retours\Q-nomad.md` (Q1 taille des groupes, défaut A 2 ; Q2 distance aux ennemis, défaut A 40 cases). **Prochaine étape pour un agent
  neuf : N2** (agent C++ : départ sans forum, `EHOLDefeatReason::Nomad` par `DefeatSlot`, forum en chantier sans vie), puis N3, N4. Aucun
  test rouge en cours. Rien de visible en jeu : pas de ligne à `Placeholders.txt`.

## 2026-11-13 11:10 HALDIR-1113 (agent, maps)
- D786 applique : GroupSize = 1 (5 Paysans = 5 groupes de 1), 40 cases inchangees, ValueSheet nomad.group_size mis a jour ; rouge 4f83b15, vert 8d028ff ; quick_check -Parts tests 1203/1203 PASS (11:10). Toujours aucun appelant en jeu (C++ SC2/SG2/SP2/N2 a faire). Reprise : etape C++ suivante du journal.

- 2026-11-13 16:5x (HALDIR-1113c) : nouvelle question [A REPONDRE] posee (L73) : `C:\Users\baudo\Documents\MyCrusader\Retours\Q-nomad-forum.md` (Q1 prix du forum D402, defaut A = cout du centre secondaire ; Q2 palier sans forum, defaut A = pause comme Forum a terre).

- 2026-11-13 17:10 REPONSES de l'utilisateur (via la session principale, ecrites en Reponse: dans les Q de Retours\) : Nomade Q1 = A (150 bois + 400 pierre) ; Nomade Q2 = B (bouton de specialisation ETEINT tant qu'aucun forum n'est acheve : A CHANGER, test rouge dans HOLSpecializationRules puis correction) ; Signaux = A (elimine chacun pour soi / Diplomatie et spectateur pur voient TOUS les signaux : A CHANGER, test rouge sur HOLSignal::Sees puis correction) ; Specialisation clic sur l'autre option = B (remplace directement, delai entier : A CHANGER, une ligne dans HOLSpecialization::Start, test rouge d'abord) ; Foret Q1/Q2 = a trancher (defauts inchanges, rien de plus a coder). NON CODE par HALDIR-1113c (plafond de 45 appels atteint) : prochain agent sans C++.

## 2026-11-13 18:08-18:20 HALDIR-1113d (agent, maps, sans C++)
- Q2 = B applique (voir SpecializationInProgress.md) : HOLNomad::HasCompleteForum ajoute, entree de HOLSpecialization::IsOpen/Start ; rouge 9921b5f, vert e0097ff ; 1227/1227 PASS.
- Q1 = A VERIFIE, rien a coder ici : HOLNomad::StartingAmount(StandardAmount, ForumPrice, bNomad) recoit le prix en entree ; aucun appelant ne lit encore le cout du centre secondaire (D783, 150 bois + 400 pierre). NOTE POUR N2 : passer ce cout lu dans les donnees (type du centre secondaire de BuildingTypes.csv), pas MainCentre (0).

## 2026-11-13 20:30-20:5x HALDIR-1113g (agent C++, maps)
- N2 etape 1 FAITE : UHOLSpecializationSubsystem::IsForumComplete lit HOLNomad::HasCompleteForum de l'etat du siege tenu par la sous-classe script (HOLSpecializationBoardScript.as : TArray<FHOLNomadState>, SetSeatNomad / NoteSeatForumComplete / SeatHasCompleteForum ; cote C++ SetNomadNow / NoteForumCompleteNow / HasCompleteForumNow). Siege standard = vrai (rien ne change sans bNomad : personne n'appelle encore SetNomadNow). Test C++ HeroesOfLiberty.Specialization.NomadForum : rouge c3b95d3 VU ROUGE (quick_check tests 1236/1237, 20:37 : bouton du Nomade allume, depart accepte), vert (script seul, pas de rebuild) : voir commit suivant.
- N2 depart nomade NON COMMENCE (heure : aucun C++ apres 21:05). Plan pour l'agent suivant : dans AHeroesOfLibertyGameModeBase::SpawnMainCentres, si MatchConfig.bNomad : pas de centre principal ; Paysans en groupes (HOLNomad::SplitIntoGroups) places loin des ennemis (FarEnough) et hors zones de sites (HOLSiteOption::NomadGroupClear) - regles en Angelscript, donc passer par le pont script (comme le tableau de specialisation) ; Board->SetNomadNow(Slot, true) au depart ; NoteForumCompleteNow a l'achevement du premier forum (point d'accroche de fin de chantier a trouver : aucun OnConstructionComplete dans Source) ; stock : HOLNomad::StartingAmount(standard, prix, true) aux 3 endroits qui donnent StartingResources (GameModeBase.cpp ~329, ~343, 616). ATTENTION : le prix du centre secondaire (D783, 150 bois + 400 pierre) est dans Script/SecondaryCentres/HOLSecondaryCentreRules.as (CostWood, CostStone ; ValueSheet eco.secondary_centre.cost_*), PAS dans BuildingTypes.csv (aucune ligne a ce cout) ; defaite a 0 Paysan (HOLNomad::IsDefeated) par AHOLGameState::DefeatSlot avec une nouvelle EHOLDefeatReason.
- Test reseau a deux clients des signaux (SG2) : NON FAIT (pas faisable en 15 min ce soir).

## 2026-11-13 20:47-21:0x HALDIR-1113h (agent C++, maps)
- N2 etape 2a FAITE (stock nomade, reponse A de Q-nomad-forum Q1) : UHOLSpecializationSubsystem::StartingStock(World, bNomad, Standard) ; sans bNomad renvoie Standard tel quel (rien d'autre lu) ; avec bNomad, NomadStartingAmount(Resource 0-3, Standard) par le pont script (HOLSpecializationBoardScript.as : HOLNomad::StartingAmount(standard, HOLSecondaryCentre::Cost(), true) = +150 bois +400 pierre). Branche aux 4 endroits qui donnent StartingResources (GameModeBase : GiveSlotStock a la connexion, SetResources sans game state, KeepMissingSlot, SpawnAIPlayerStates). Test C++ HeroesOfLiberty.Specialization.NomadStock (sans bNomad = identique ; Nomade = +150/+400 calcules par le script) : rouge b85afbb/6627655 VU ROUGE (1237/1238, 20:52), vert 6c59e9a (1238/1238, 20:54). Au passage : HOLSelectionMarkers.cpp, son en-tete d'abord (controle UHT qui bloquait le build).
- RESTE N2 (ordre conseille, plus petite d'abord) : (b) EHOLDefeatReason::Nomad + regle pure IsDefeated -> DefeatSlot quand le dernier Paysan meurt sans forum acheve ; (c) SetNomadNow(Slot, true) au depart dans SpawnMainCentres SEULEMENT en meme temps que (d) pas de centre principal + Paysans en groupes (SplitIntoGroups, FarEnough, NomadGroupClear 3200 cm via le pont script ; Roi/heros places pres d'un groupe : SpawnKing/SpawnStartingPeasants/SpawnStartingHero prennent un AHOLBuilding& aujourd'hui, il faut une variante a un point) ; (e) NoteForumCompleteNow a l'achevement du premier forum (point d'accroche a trouver : pas d'OnConstructionComplete dans Source). Sans (c)+(d) ensemble, un Nomade garderait son centre et verrait sa specialisation eteinte : ne pas brancher (c) seul.
- Scenario nomad_start : NON FAIT (le depart nomade n'existe pas encore en jeu).
