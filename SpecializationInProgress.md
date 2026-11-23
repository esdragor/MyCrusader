# SpecializationInProgress.md : le choix de spécialisation de palier, mécanisme commun (GDD § 9.5 et § 13 ; D07, D332, D333, D683, D684, avec D131, D334, D690)

Ouvert le 2026-11-12 (séance autonome, HALDIR-1112f, worktree Haldir, branche `maps`, chapitre A2 du relevé du jour). Méthode des
dossiers par feature : règles tirées du GDD, plan en étapes de 15 min, test rouge d'abord, au plus 2 questions
(`C:\Users\baudo\Documents\MyCrusader\Retours\Q-specialization.md`). Règle D483 : règles pures en Angelscript ; l'état répliqué et le
serveur en C++. **D690** : ce mécanisme est commun ; l'Aube (A8 de `AubeInProgress.md`) et les Légions (L9 de `LegionsInProgress.md`)
le reprennent, chacune n'y ajoute que ses deux options et leurs effets.

## Règles tirées du GDD (rien à trancher)

1. **Un palier (1, 2, 3) ouvre un tronc commun et le choix d'une spécialisation parmi 2** (D07, § 9.5), **définitif pour la partie**.
   *Templiers* : une commanderie parmi **3** par palier (D90, D131) ; ce sont des technologies (D683) qui débloquent l'unité dans le
   bâtiment de son type (D334).
2. **Quand** (D332) : à l'ouverture du palier, **un bouton s'allume dans le HUD** (emplacement de faction, D201) ; le joueur choisit
   **quand il le décide**, **gratuitement** ; aucune échéance pour choisir.
3. **Délai** (D684, révise « instantané » de D332) : on clique, **le déblocage se lance au Forum** et le choix est appliqué **au bout de
   30 / 45 / 60 s (paliers 1 / 2 / 3)**, valeurs à régler en test ; **en parallèle** de la file du Forum (qui continue de produire), **une
   barre à part** dans le HUD de faction montre le compte à rebours ; écarté : une place dans la file (D283).
4. **Annulation** (D684) : **sans coût avant la fin** ; on peut annuler et **rouvrir le choix** tant qu'il n'est pas appliqué ; **définitif
   une fois appliqué** (D07). Rien n'est rendu parce que rien n'a été payé (choix gratuit, D332, D683).
5. **À l'échéance** : ce qui dépend du choix arrive (D332) : une *technologie* s'applique (rien à construire) ; un *bâtiment* est
   **débloqué** puis se construit en plusieurs exemplaires, payé comme tout bâtiment (D683) ; *Templiers* : l'unité et son contingent.
6. **Lisibilité** (D333) : aucune information publique ni sur la fiche du héros ; **un signe dans le monde**, posé **à l'application**
   (D684) sur ce que l'effet modifie ou sur le bâtiment qui forme les unités concernées (D683). Hors des règles pures (étape SP3).
7. **L'IA choisit selon ses profils de build** (D332, § 9.5).

Lectures faites sans décision (pas des questions) : le palier vient du niveau du héros et ne baisse jamais (D569, paliers P1-P3) ; après
une annulation, une nouvelle option repart d'un délai entier (« annuler et rouvrir le choix ») ; un déblocage en cours ne se relance pas
sur place (annuler d'abord).

Laissé au chantier par D684 (questions, défauts codés) : **le compte à rebours quand le Forum est à terre** (Q2). Pas écrit au GDD :
**deux paliers à choisir en même temps** (joueur arrivé au palier 2 sans avoir choisi le 1 : un déblocage à la fois ou les deux, Q1).

## Ce qui existe (relevé du 2026-11-12)

- **Aucun code de spécialisation** : Grep `Specializ|TierChoice` dans `Script/` et `Source/` : rien.
- **Palier du joueur** (à réutiliser, pas à recopier) : `HOLHeroXP::GetTier(Level)` (`Source/HeroesOfLiberty/Simulation/HOLHeroXPRules.h`,
  niveau ÷ 3 borné 0..3, P1) ; `AHOLGameState::GetTier(Slot)` (P3, lu par la construction `HOLConstructionSubsystem.cpp:437`, la
  production `HOLProductionSubsystem.cpp:398`, l'IA `HOLAISummary.cpp:550`, l'écran `HOLGameScreen.cpp:580`) ; commande de développement
  `hol.Cheat.Tier <0..3|off> [Slot|all]` (`HOLPlayerController.cpp:1806`, G0 de Palier3). Les règles pures reçoivent ce palier en
  argument (`PlayerTier`) : elles ne lisent pas le monde.
- Règles de palier voisines (même forme) : `Script/Units/HOLUnitTierRules.as` (`HOLUnitTier::Check`), `Script/Siege/HOLSiegeRules.as`
  (`TierOf`), `Script/Technologies/HOLTechnologyRules.as` (palier minimum d'une technologie, lieu de recherche D583).
- À réutiliser plus tard : `HOLAITalent.as` (`AnswerFamily`, profil par faction) pour l'IA ; `EHOLFaction` (`Match/HOLMatchConfig.h`) ;
  la grille de commandes et l'emplacement de faction du HUD (D201) pour le bouton ; `Technologies.csv` pour les effets *technologie*.

## Plan (étapes de 15 min)

- **SP0 relevé** : FAIT (ce fichier, 2026-11-12).
- **SP1 règles pures en script** : `Script/Specialization/HOLSpecializationRules.as` (`FHOLSpecializationState`, `NewState`, `IsOpen`,
  `DelayMs`, `Start`, `Cancel`, `Advance` qui rend le palier appliqué, `RemainingMs` pour la barre, `ChosenOption`) et
  `Script/Tests/HOLSpecializationRules_Test.as` (7 tests). Délais au registre `Tools/Data/ValueSheet.csv` (`tier.choice.delay_*`).
- **SP2 état répliqué (C++, agent C++)** : un `FHOLSpecializationState` par emplacement sur `AHOLPlayerState` (répliqué au seul
  propriétaire pour le déblocage en cours : D203, l'adversaire ne lit rien) ; RPC serveur `ServerStartSpecialization(Tier, Option)` et
  `ServerCancelSpecialization()` qui appellent `Start` / `Cancel` avec `AHOLGameState::GetTier(Slot)` ; `Advance` au pas de la
  simulation (temps de partie, pas fixe) avec l'état du Forum ; à l'application, un événement (le palier, l'option) que les chantiers de
  faction écoutent (Aube A8, Légions L9). Commande de développement `hol.Cheat.Specialization <Tier> <Option>` (applique sans délai,
  pour les scénarios). Test : `net_match` (choix par un client, appliqué au serveur après le délai, annulé, refusé deux fois).
- **SP3 panneau de choix** : bouton allumé à l'emplacement de faction (D201) tant que `IsOpen` ; panneau des 2 (ou 3) options avec les
  noms du § 13 (nom de l'option = nom du choix, D683) ; barre à part avec `RemainingMs` (D684) ; bouton Annuler ; signe dans le monde à
  l'application (D333 ; provisoire : ligne à `Placeholders.txt`). Scénario `specialization` (geste réel), essai VU À L'ÉCRAN seulement.
- **SP4 IA** : choisir selon le profil de build (D332), au palier ouvert, après un court délai ; modèle `HOLAITalent.as` (`AnswerFamily`).
- **SP5 reprise par les factions** : Aube A8-A10 et Légions L9 branchent leurs options (effets, bâtiments débloqués) sur l'événement de SP2.

## Premier test rouge (SP1)

`Script/Tests/HOLSpecializationRules_Test.as` : 7 tests : le choix s'ouvre avec le palier et attend ; appliqué à 30 s et pas à 29,999 s ;
annulé = rien de choisi, choix rouvert, délai entier pour l'autre option ; un seul choix par palier, définitif ; départs refusés (palier 0
ou 4, palier non atteint, option hors des 2, Templiers à 3) ; deux paliers (Q1) ; Forum à terre (Q2).

## Questions (`Retours\Q-specialization.md`, défauts codés)

- Q1 : deux paliers à choisir à la fois. Défaut A = un déblocage à la fois (`OneUnlockAtATime`), comme la barre unique de D684.
- Q2 : le compte à rebours quand le Forum est à terre. Défaut A = en pause, reprend quand il est relevé (`ForumDown = ForumDownPauses`).
- **Réponses de l'utilisateur (2026-11-12, D785)** : Q1 = **C, le palier 1 d'abord : le bouton du palier 2 ne s'allume qu'une fois le palier 1 appliqué** (À CHANGER : remplace `OneUnlockAtATime`, une ligne de plus dans `HOLSpecializationRules.as`, test à adapter) ; Q2 = **A, en pause quand le Forum est à terre** (déjà codé).

## Suivi

- 2026-11-12 11:45-11:56 (HALDIR-1112f, `maps`, sans C++) : SP0 relevé ; test rouge SP1 commité 567fd50 (quick_check `tests` 11:50 :
  « Identifier 'FHOLSpecializationState' is not a data type », 0 test lancé). **SP1 VERTE** : quick_check `tests,values` 1193/1193 PASS
  (219 s), `values` PASS (3 lignes au registre `tier.choice.delay_tier1_ms`, `_tier2_ms`, `_tier3_ms` ; Valeurs.md/.csv régénérés).
  Commit vert 1124a13, puis `git merge main` 3ff176d (docs seuls). Questions `Retours\Q-specialization.md` (Q1 deux paliers à la fois, défaut A ; Q2 Forum à terre, défaut A pause). **Prochaine étape
  pour un agent neuf : SP2** (agent C++ : état sur `AHOLPlayerState`, RPC serveur Start / Cancel avec `AHOLGameState::GetTier`, `Advance`
  au pas de la simulation, événement d'application, `hol.Cheat.Specialization`), puis SP3. Aucun test rouge en cours. Rien de visible en
  jeu : pas de ligne à `Placeholders.txt`.

## 2026-11-13 11:10 HALDIR-1113 (agent, maps)
- D785 applique : OneUnlockAtATime remplace par PreviousTierFirst (palier N ouvert seulement si N-1 applique, reponse PreviousTierNotApplied = 7) ; rouge bad840e (Test_SpecializationTierOneFirst, IsOpen 2/3 eteints avant le palier d'avant ; ligne 80 : 30/45/60 s assertis en litteral et par Start/Advance a 29999/44999/59999 + 1 ms), vert 8d028ff ; quick_check -Parts tests 1203/1203 PASS (11:10). Toujours aucun appelant en jeu (C++ SC2/SG2/SP2/N2 a faire). Reprise : etape C++ suivante du journal.

## 2026-11-13 12:33 HALDIR-1113b (agent, maps, sans C++)
- Ligne 80 durcie aux bornes exactes : 4 tests de plus dans HOLSpecializationRules_Test.as (pas de simulation 50 ms = 600/900/1200 pas pile a 30/45/60 s, pas de 33 ms = 910/1364/1819 pas, premier pas apres la borne ; un pas long de 45 s pose une fois, sans report sur le palier suivant ; pause Forum a terre 10 s + 50 s a terre + 19,999 s = pas encore, +1 ms = pose ; reprise apres annulation a 29,999 s = delai entier ; Templiers 3e commanderie a 30 s). Aucun trou dans les regles : verts d'emblee, pas de commit rouge. quick_check -Parts tests 1207/1207 PASS (12:33), commit 22dbb3b. Reprise : SP2 (C++), inchange.

- 2026-11-13 16:5x (HALDIR-1113c) : nouvelle question [A REPONDRE] posee (L73) : `C:\Users\baudo\Documents\MyCrusader\Retours\Q-specialization-switch.md` (changer d'option en cours ; defaut A = annuler d'abord, comportement actuel ; recommandation B). Le cas Nomade sans forum est pose dans Q-nomad-forum Q2.

- 2026-11-13 17:10 REPONSES de l'utilisateur (via la session principale, ecrites en Reponse: dans les Q de Retours\) : Nomade Q1 = A (150 bois + 400 pierre) ; Nomade Q2 = B (bouton de specialisation ETEINT tant qu'aucun forum n'est acheve : A CHANGER, test rouge dans HOLSpecializationRules puis correction) ; Signaux = A (elimine chacun pour soi / Diplomatie et spectateur pur voient TOUS les signaux : A CHANGER, test rouge sur HOLSignal::Sees puis correction) ; Specialisation clic sur l'autre option = B (remplace directement, delai entier : A CHANGER, une ligne dans HOLSpecialization::Start, test rouge d'abord) ; Foret Q1/Q2 = a trancher (defauts inchanges, rien de plus a coder). NON CODE par HALDIR-1113c (plafond de 45 appels atteint) : prochain agent sans C++.

## 2026-11-13 18:08-18:20 HALDIR-1113d (agent, maps, sans C++)
- Reponse Q-specialization-switch = B APPLIQUEE : Start sur l'autre option du palier en cours remplace (option annulee, l'autre au delai entier) ; clic sur l'option en cours elle-meme = Busy (rien ne repart, inchange). Rouge 9921b5f (Test_SpecializationOtherOptionReplacesTheRunningOne), vert 02507e1.
- Reponse Q-nomad-forum Q2 = B APPLIQUEE : IsOpen et Start prennent bForumComplete (= HOLNomad::HasCompleteForum, defaut true) ; sans forum acheve le bouton est eteint et Start rend NoForumComplete = 8 ; il s'allume a l'achevement (NoteForumComplete). Rouge 9921b5f (Test_SpecializationNomadButtonWaitsForTheFirstForum), vert e0097ff. quick_check -Parts tests 1227/1227 PASS (18:18), merge main ffddb3a. SP2 : passer HasCompleteForum de l'emplacement a IsOpen/Start.

- 2026-11-13 20:0x HALDIR-1113e : SP2 NON COMMENCE (plafond d'appels atteint apres SG2). Reprise : SP2 inchange (modele : UHOLSignalSubsystem + sous-classe script, etat tenu par le script au serveur).

## 2026-11-13 20:06-20:3x HALDIR-1113f (agent C++, maps)
- SP2 FAIT cote serveur (modele SG2) : pont C++ abstrait UHOLSpecializationSubsystem (UTickableWorldSubsystem, Match/HOLSpecializationSubsystem.h/.cpp) + sous-classe script Script/Specialization/HOLSpecializationBoardScript.as (un FHOLSpecializationState par siege ; Start/Cancel/IsOpen/Advance de HOLSpecializationRules.as). Vue FHOLSpecializationView (Match/HOLSpecializationView.h : PendingTier, PendingOption, RemainingMs, Chosen[1..3]) sur AHOLPlayerState, COND_OwnerOnly (ForceNetUpdate seulement sur depart/annulation/application ; le compte a rebours suit les envois ordinaires). AHOLPlayerController::RequestSpecialization/RequestCancelSpecialization -> ServerStartSpecialization(Tier, Option) / ServerCancelSpecialization (GetTier du GameState, IsForumComplete). Tick serveur/solo au temps du monde (GetTimeSeconds en ms entieres), Forum a terre = AHOLGameState::IsForumDown (pause, Q2 A). Evenement OnApplied(Slot, Tier, Option) pour A8/L9.
- Test C++ HeroesOfLiberty.Specialization.ServerBoard : rouge 1bce302 VU ROUGE (quick_check tests 1235/1236, 20:17), vert : build PASS, tests 1236/1236 + net PASS (20:28).
- RESTE : IsForumComplete rend toujours vrai (N2 doit y brancher HOLNomad::HasCompleteForum) ; hol.Cheat.Specialization ; test net_match a deux clients (choix par un client, applique au serveur, l autre client ne lit rien) ; Templiers 3 options (DefaultOptionCount pour tous) ; SP3 bouton/barre. NON VERIFIE en jeu ni en reseau. N2 non commence.

- 2026-11-13 20:5x HALDIR-1113g : IsForumComplete branche sur HOLNomad::HasCompleteForum (etat nomade par siege dans HOLSpecializationBoardScript.as), rouge c3b95d3, voir NomadInProgress.md. Personne n'appelle encore SetNomadNow (N2).
- 2026-11-15 13:18-13:38 ELROND-1115h (tiers, C++) : SP3 VERT : cases D/F de la grille du Forum principal (Voie A/B du palier ouvert ; en cours = N s + Annuler) -> RequestSpecialization / RequestCancelSpecialization ; test Specialization.ForumGrid vu ROUGE 13:30 (1312/1313), vert tests,net 13:38 ; Q-specialization-ui [A REPONDRE] (defaut A, cvar hol.UI.SpecializationCell=6) ; reste : libelles GDD 13 par faction (provisoires Voie A/B), Templiers 3 options, barre hors grille si Q=B, scenario specialization_button + preflight, net a 2 clients
- 2026-11-15 14:03-14:12 ELROND-1115i (tiers, sans C++) : scenario `specialization_button` (.txt/.json/.look) a geste reel (hol.Mouse.Click GridCell06 = case D) ; test rouge a392b65 (.look, preflight « no scenario ») ; scenario 76600b7 ; build PASS 14:08 (binaire perime apres merge main) ; preflight PASS 14:09 (37 s, 3 PNG vus) ; Retours\E-specialization_button.md [A ESSAYER] (VU A L ECRAN, GESTE reel, SON non entendu). DEFAUT VU : la case en cours (4 lignes « Voie A du palier 1 / En cours : 29 s / Annuler ») deborde sur la case C et se coupe ; l infobulle couvre la 3e rangee. Reprise pour un agent neuf : [ ] C++ texte de la case en cours (2 lignes « En cours 29 s » + Annuler implicite, ou police reduite), sans trancher Q-specialization-ui.
  - [ ] Besoin C++ (non fait, sans C++) : hol.HUD.Report ne liste pas les cases de specialisation (ShownGridTypes = TypeName vide pour Specialize/CancelSpecialization, HOLGameScreen.cpp ~2230) : l'etat « bouton visible » s'attend par hol.Spec.Report, pas par le HUD.
- 2026-11-15 14:26 ELROND-1115j (tiers, C++, ARRET demande par la session principale) : NON COMMITE, ROUGE dans l'arbre. Fait : textes de la case dans la table HOLGameNames (GetSpecializationName, GetSpecializationCellText ; HOLGameScreen les appelle, nom de la voie dans l'infobulle) ; HOLCommandGrid::GetReportName (stub NAME_None) branche dans ShownGridTypes (HOLGameScreen ~2231) pour Specialize/CancelSpecialization ; test HeroesOfLiberty.Specialization.ForumGrid deplace (git mv) dans Source\HeroesOfLibertyUI\Tests et etendu (2 lignes max, aucune ligne plus longue que le libelle de l'autre case, nom absent de la case en cours, secondes du tableau ; report CancelSpecializationT1B / SpecializeT1A, Paysan None). Build PASS 14:23 ; VU ROUGE quick_check tests 1319/1320 14:26 (ForumGrid seul). Prochaine etape (agent neuf) : [ ] HOLGameNames.cpp GetSpecializationCellText : LOCTEXT SpecializationRunningCell -> 'En cours {0} s\nAnnuler' (secondes seules, sans Name) ; [ ] HOLCommandGrid.cpp GetReportName : Specialize/CancelSpecialization -> FName 'SpecializeT<tier><A|B>' / 'CancelSpecializationT<tier><A|B>', sinon NAME_None ; [ ] build, quick_check tests,net, commit test+correctif ensemble ; [ ] scenario : waitlog sur '=CancelSpecializationT1A' du HUD report ; preflight + PNG ; fiche E.
- 2026-11-15 (réponse de l'utilisateur, session principale, docs seulement) : D823 (Q-specialization-ui) = B : bouton permanent à l'emplacement de faction + panneau des 2 options + barre du compte à rebours ; cases D / F gardées en raccourci ; PAS le défaut codé ; ligne [ ] Elrond (C++).
- 2026-11-16 (réponse de l'utilisateur, session principale, docs seulement) : essai specialization_button joué : OK (défaut connu du texte en cours qui déborde : non signalé par l'utilisateur ; la ligne [ ] du texte reste) ; fiche archivée dans Docs\Archive\Retours_2026-11-16\.
- 2026-11-16 15:03-15:13 ELROND-1116a (tiers, C++) : rouge ForumGrid VERT. HOLGameNames GetSpecializationCellText en cours = 'En cours N s' + 'Annuler' (2 lignes, nom de la voie dans l infobulle seulement) ; HOLCommandGrid::GetReportName = SpecializeT<palier><A|B> / CancelSpecializationT<palier><A|B> (hol.HUD.Report). Build PASS 15:04, quick_check tests,net PASS 15:12 (tests 1320/1320, rouge vu 2026-11-15 14:26 1319/1320 sur le meme stub). Suivant : merge main, puis scenario specialization_button (waitlog =CancelSpecializationT1A, .look 2 lignes), preflight, PNG, fiche E.
- 2026-11-16 15:15-16:35 ELROND-1116a : 3949fd5 + merge main 1ede5b5 (conflit du journal resolu). preflight 15:15 : PNG running VU ROUGE (DEFAUT persistant : 'En cours 29 s' se replie sur 2 lignes, 'Annuler' en 3e ligne coupe par l infobulle ; une ligne de case = 'Voie A du', 9 car.). Correctif : case en cours = '{0} s' + 'Annuler' (le mot 'En cours' retire, infobulle inchangee) ; test ForumGrid : chaque ligne <= 9 car. ; scenario : waitlog =SpecializeT1A enabled / =CancelSpecializationT1A enabled du HUD report, .look 2 lignes. Build en file derriere Haldir (15:16 -> 16:35+). Decision utilisateur 16:3x : arret du C++ aujourd hui, passage a Angelscript ; aucune ligne suivante prise.
- 2026-11-19 21:2x HALDIR-1119n1 (agent, branche templar-options, sans C++) : regle pure du nombre d options par faction (Templiers 3, autres 2) : OptionCountOf / NewStateFor / IsValidOptionFor (variantes, aucune signature changee). Rouge 5d2709f (stub 2 pour tous + Test_SpecializationOptionCountPerFaction, ...TemplarsCanPickTheThirdOption, ...DawnRefusesTheThirdOption ; 2 rouges attendus, le test Dawn passe sur le stub), vert voir git log templar-options. NON LANCE (pas de quick_check). Reste : brancher HOLSpecializationBoardScript (NewStateFor avec la faction du siege), 3e case de la grille, libelles des commanderies.
- 2026-11-19 22:00 NON COMPILE (CELEBORN-1119n3, morceau 8, branche cheat-specialization 37c8e78 rouge / 95b8288 vert) : hol.Cheat.Specialization <palier 1..3> <option> ecrit (plan C2 + test, RPC ServerCheatSpecialization, pont CheatSpecialization, HOLPlayerController::DoCheatSpecialization : StartNow puis AdvanceNow, refus du tableau rendu sans rien forcer) ; a compiler et tester par la session principale.
- 2026-11-19 22:08 GALADRIEL-1119n3 (agent, branche specialization-trial fc10863, sans C++) : essai specialization_button en echec = cause C (pas de regression : les cases D/F sont produites, le journal 20:56 les montre dans hol.HUD.Report) ; le 3e rapport etait pris 14 images apres l'annulation, vue de grille pas encore rafraichie (la 2e attente avait wait 2) : scenario corrige (wait 2 + frames 8) et mapSize Small ajoute ; a rejouer : preflight specialization_button.
- 2026-11-19 ~22:55, réponse de l'utilisateur : essai specialization_button OK (joué le 2026-11-19, sur main, scénario fc10863) ; fiche archivée dans Docs\Archive\Retours_2026-11-19\.
- 2026-11-20 09:1x HALDIR-1120b (agent, branche wall-line 32ddb97, sans C++) : scenario specialization_cheat (.txt/.look/.json) pour hol.Cheat.Specialization 1 1, JAMAIS LANCE (cheat C++ NOT COMPILED, pas de preflight). Lecture : Advance ajoute ElapsedMs a RunningMs puis teste >= DelayMs, donc AdvanceNow(Slot,3600000,true) APPLIQUE le palier (un depassement n'est pas sans effet) ; apres choix du palier 1 la case D/F n'offre plus rien (GetOpenSpecializationTier = 0), pas d'etat 'choisi' dessine. Details : Docs\Rapports\HALDIR-1120b.md
- 2026-11-20 10:4x ELROND-1120e (worktree Elrond, integ-25, beac365 test rouge + 5bb28d4 correctif, script seul, NON VERIFIE a l'execution) : morceau 7 du socle, moitie script faite : UHOLSpecializationBoardScript.SetSeatFaction/SeatFactionOf (GetSlotFaction sinon, Aube par defaut) dimensionne l'etat du siege par HOLSpecialization::OptionCountOf ; HOLCommandGridRules::GetCommandWithOptions(Context, Cell, OptionCount) montre la 3e case. Tests Test_SpecializationBoardSeatFactionOptions et Test_SpecializationThirdGridCellForThreeOptions (HOLSpecializationRules_Test.as). RESTE (C++, lot groupe) : porter le compte d'options jusqu'a la grille (FHOLSpecializationView.OptionCount + FHOLGridContext/ContextView.SpecializationOptions, GetCommand l'appelle encore avec 2), GetSpecializationName/GetReportName/ServerStartSpecialization pour l'option 2, appel de SetSeatFaction a l'initialisation des sieges. Q-specialization-troisieme-case (3e case masque la 1re recherche, defaut A). Rapport : Docs\Rapports\ELROND-1120e.md
- 2026-11-20 11:0x ELROND-1120f (worktree Elrond, integ-25, merge integ-27 fait, script seul, NON COMPILE) : Test_SpecializationBoardSeatFactionOptions REECRIT (7d681f4) sans evenement C++ protege : Board.SetSeatFaction/SeatFactionOf/StateOf (membres script) + regle pure HOLSpecialization::Start/Advance/ChosenOption ; cas : siege Aube refuse l option apres la derniere (BadOption), siege Templier accepte la 3e et elle atterrit, siege jamais regle = Aube, SetSeatFaction apres coup reconstruit l etat au compte Templier. Test_SpecializationThirdGridCellForThreeOptions : aucune fonction protegee. A voir a la session principale : rouge/vert a l execution. Rapport : Docs\Rapports\ELROND-1120f.md
- 2026-11-20 ~12:30 (DOCS-1120r, réponse de l'utilisateur, aucun code) : Q-specialization-troisieme-case = B (GDD D868) : cases 5, 6, 7 pour les Templiers, une ligne dans `GetSpecializationCell` (C++, lot C++ de la séance) ; libellé de la 3e voie (`GetSpecializationName`) à écrire dans le même lot.
- 2026-11-20 13:14 (DOCS-1120s, réponse de l'utilisateur, aucun code) : E-specialization_cheat = OK (la triche hol.Cheat.Specialization 1 1 choisit la voie du palier 1 sans attendre, vu en pré-vol sur main 991a8bc). Fiche archivée : Docs\Archive\Retours_2026-11-20\E-specialization_cheat.md.
- 2026-11-20 15:4x GLORFINDEL-1120pb (worktree Glorfindel, branche ability-columns, NON COMPILE, script seul) : D868 FAIT : test rouge 7b061b3 (Test_SpecializationThirdGridCellForThreeOptions reecrit : Templiers cases Base-1+k = 5 6 7, Aube 6 7, rien apres) puis correctif 4b771a1 dans HOLCommandGridRules.as (GetCommandWithOptions : FirstOption = GetSpecializationCell(0) - (OptionCount - 2), fin = FirstOption + OptionCount). GetSpecializationCell C++ INCHANGE : il ne connait que l'option, pas le nombre d'options (le compte est celui du script). Reste : libelle de la 3e voie (GetSpecializationName C++, lot C++), liaison de la faction du siege vers la grille (GetCommand garde 2 options par defaut), rouge a voir a l'execution (quick_check -Parts tests).
- 2026-11-20 16:0x GLORFINDEL-1120pd (worktree Glorfindel, ability-columns, c54698d test rouge + ec839e3 correctif, NON COMPILE, C++ autorise : liaison + libelle) : suite D868 : regle script HOLCommandGridRules::GetCommandForFaction (nombre de voies = OptionCountOf(faction), -1 = 2) ; la grille du jeu (HOLCommandGridBoard.as GetCommandAnswer) l'appelle avec Context.SpecializationFaction ; C++ : champ FHOLGridContext::SpecializationFaction (HOLCommandGrid.h) + FHOLGridContextView (ReadLibrary.h) + MakeContextView (Board.cpp) + HOLPlayerController.cpp (GetSlotFaction du siege) ; GetSpecializationName : Option>=2 = Voie C ; GetReportName : C ; commentaires HOLCommandGrid.h et Rules.as mis a jour. Test Test_SpecializationGridShowsThreeLanesForTheTemplars (HOLSpecializationRules_Test.as). A FAIRE : build (lot C++) puis tests script ; le script du board ne compile qu'apres le build (nouveau champ du USTRUCT). Hors portee : le test C++ HOLSpecializationGridTests.cpp reste 2 voies.
- 2026-11-23 09:08 (GALADRIEL-1123a) : releve des effets des voies Templiers dans Docs\Rapports\SPEC-VOIES-1123.md ; ChosenOption lu par le tableau seul, aucun effet en jeu ; question Retours\Q-spec-voies.md (defaut A, deblocage d'unite seul). Pas de code.
- 2026-11-23 09:15 (GALADRIEL-1123b, socle-ai, script seul, aucun code) : voie = unite (defaut A, reglage hol.Spec.UnlockUnit) NON CODE : les 9 unites de commanderie (Sergent du Temple ... Beffroi) sont absentes de Tools\Data\UnitTypes.csv (18 unites, aucune Templiers ; Dawn : Peasant, Monk, Arquebusier, Halberdier, VirtuousKnight). Production : refus serveur = HOLRpc::QueueProductionRefusal (HOLRpcPlayer.as:235, siege/batiment seulement) + EHOLProductionRefusal::NotProducible (HOLProductionQueue) ; aucune table par faction/voie lue. Prochaine etape : creer les unites du palier 1 (decision de la question Q-spec-voies), puis regle HOLSpecUnlock.
