# Alertes mineures (D202, C1 du relevé 2026-11-15) — journal de GALADRIEL-1115f

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`, branche `galadriel-tab` (avance rapide sur `main` a082990 à 13:18).
Question : `C:\Users\baudo\Documents\MyCrusader\Retours\Q-alerts-minor.md` [À RÉPONDRE] (défauts codés ci-dessous).

## 1. Inventaire PAR LECTURE du code (2026-11-15 13:20)

GDD § 15, D202 : « Mineures (unité ou bâtiment attaqué ailleurs, recherche terminée, population pleine…) : fil discret sur le côté, ping et son
court propre à chaque type. » D446 : le fil (5 lignes, ~8 s, sans son). D449 : les lignes du fil restent muettes. D397 : héros prêt = alerte mineure.

Le fil = `FHOLForumThread MessageThread` (HOLGameScreen.h:550) ; toutes les lignes passent par `MessageThread.Add` dans `HOLGameScreen.cpp` (5 appels,
l. 760, 772, 781, 798, 825) ; textes dans la table unique `HOLForumMessages.cpp`.

### Présentes dans le fil
| Alerte | Texte | Où | Son | Ping mini-carte |
|---|---|---|---|---|
| Tribut reçu (D372) | « Joueur 1 vous a offert 70 or (taxe 30) » | `GetTributeMessage`, `HandleTributeMessage` l. 754 | non | non |
| Signal reçu (SG3, HALDIR-1115e) | « Joueur 2 : Attaque ici » | `GetSignalMessage`, `HandleSignalMessage` l. 765 | non | (le signal a son marqueur, hors fil) |
| Héros prêt à revenir (D397) | « Votre héros est prêt à revenir » | `GetHeroReadyMessage`, `HandleHeroReadyMessage` l. 777 | non | non |
| Niveau du héros, palier atteint (J8b-Q4) | « Paladin-Commandant : niveau N », « Palier 1 atteint » | `GetHeroLevelMessages`, `HandleHeroLevelUp` l. 786 | oui (fanfare) | non |
| (Majeures, aussi au fil) forum tombé, ruine tenue, fondations rasées, Roi mort, vaincu, a quitté | `GetMessage`, `HandleForumEvent` l. 813 | bandeau + bip (`FHOLForumAlert`) | non |

### Manquantes (exemples de D202)
- **Population pleine** : PAS dans le fil. Existe seulement (a) la note « Population pleine » de la file de production (`QueueNote`,
  HOLGameScreen.cpp:3649, montrée quand la tête de file attend) et (b) le message de commande « Population maximale — construisez des maisons »
  (`EHOLCommandFeedback::PopulationFull`, zone des messages de commande, au clic seulement). → **codée ici en règle pure** (étape 3).
- **Recherche terminée** : rien (aucun événement de fin de recherche relayé au HUD ; Grep `Research` vide hors « Déjà recherchée »).
- **Bâtiment achevé** : rien dans le fil.
- **Unité / bâtiment attaqué ailleurs** : PAS de ligne au fil ni de son ; MAIS un **ping clignotant existe sur la mini-carte**
  (`Script\UI\HOLMinimapRules.as` `Detect` : PV en baisse d'un bien du joueur → point qui clignote 6 s, fusion à 25 m ; tests
  `Script\Tests\HOLMinimapRules_Test.as`). Le relevé (« aucun ping sur la mini-carte dans le code ») est donc **à corriger** pour ce cas.
- **Unité perdue**, **production terminée**, **ressource épuisée**, **villageois inactif** : rien (exemples AoE, pas dans D202 : à trancher).
- Aucun son propre aux mineures (D202 « son court propre à chaque type ») sauf la fanfare du héros ; aucun journal (historique) du fil.

## 2. Plan (étapes de 15 min)
1. [fait] Inventaire ci-dessus.
2. [fait] Question regroupée `Retours\Q-alerts-minor.md`.
3. [en cours] `Script\UI\HOLAlertRules.as` : `ShouldAlertPopulationFull(Watch, Used, Cap, Now, RepeatSeconds)` (front montant
   pas plein → plein ; anti-répétition : pas de nouvelle ligne avant `PopulationRepeatSeconds` = 30 s, défaut de la question) +
   texte (clé existante `HOLGameScreen,PopulationFull` « Population pleine », déjà traduite). Premier test rouge :
   `Test_AlertPopulationFullOnReaching` (le stub renvoie false : vu rouge à l'exécution), puis vert.
4. [suivant, C++] Branchement (voir § 3), puis recherche terminée et bâtiment achevé sur le même modèle.

## 3. Branchement C++ qui RESTE (non fait : sans C++)
- `AHOLPlayerState::GetPopulationUsed/GetPopulationCap` (HOLPlayerState.h:83-85) ne sont pas des `UFUNCTION` : les exposer
  (`UFUNCTION(BlueprintPure)`) pour que `HOLGameScreenScript.as` les lise à chaque rafraîchissement.
- Aucun moyen pour le script d'ajouter une ligne au fil : ajouter à `UHOLGameScreen` un `UFUNCTION(BlueprintCallable) AddThreadLine(FText)`
  qui fait `MessageThread.Add(Message, RealTime); RebuildMessages();` (modèle : `HandleSignalMessage` l. 765, machine du joueur seul,
  rien pour un spectateur).
- Puis dans `HOLGameScreenScript.as` : un `FHOLPopulationAlertWatch` membre, appel de la règle au tick du HUD, `AddThreadLine` si vrai.
- Son court / ping : selon la réponse à Q-alerts-minor (défaut : ni son ni ping, D449).

## 4. Appels d'outils (un trait par appel)
||||| ||||| ||||| ||||| |||||  (25 à l'écriture de ce journal)

## 5. Resultats (13:32)
- Avant : quick_check tests en echec de compilation de script (binaire perime apres merge main : FHOLReceivedSignal inconnu) -> build du C++ de main (PASS 103 s, 13:25, aucun C++ ecrit).
- Rouge VU a l'execution 13:28 : tests 1312/1314, seuls Test_AlertPopulationFullOnReaching et Test_AlertPopulationFullRepeat (stub false).
- Vert 13:32 : regle IsPopulationFull / ShouldAlertPopulationFull / PopulationFullText ecrite, quick_check tests PASS (test + correctif dans le meme commit).
- Etape 3 FAITE. Suivant : branchement C++ du section 3 (agent C++), puis recherche terminee et batiment acheve selon la reponse a Q-alerts-minor.
- 2026-11-15 (réponse de l'utilisateur, session principale, docs seulement) : D804 (Q-alerts-minor) = B : trois lignes + son court par type + ping mini-carte (pas pour population pleine), « attaqué ailleurs » = ligne + son, 1 fois / 30 s ; question archivée ; ligne [ ] Galadriel (C++) dans Auto_Journey_File.md. Le défaut codé (A pour population pleine seulement) est donc élargi.

## 6. Branchement au HUD (GALADRIEL-1115g, 2026-11-15 14:02-14:2x, C++)
- Plus petit changement retenu (AHOLPlayerState non touche) : `UHOLScriptScreen::GetPopulationNow(Used, Cap, RealSeconds)` (BlueprintPure, lit le
  joueur que montre l'ecran C++ : `UHOLGameScreen::GetBoundPlayerState`, rien pour un spectateur) et `UHOLScriptScreen::AddThreadLine(FText)`
  (BlueprintCallable) -> `UHOLGameScreen::AddThreadLine` (MessageThread.Add + RebuildMessages, modele HandleSignalMessage, ligne muette).
- Script : `HOLGameScreenScript.as` `RefreshPopulationAlert()` a chaque Refresh (4 fois/s), membre `FHOLPopulationAlertWatch PopulationWatch`.
- Test (vraie chaine) : `Source\HeroesOfLibertyUI\Tests\HOLPopulationAlertThreadTests.cpp` `HeroesOfLiberty.UI.Alerts.PopulationFullThread` :
  vrai UHOLGameScreen + sa partie script, AHOLPlayerState::SetPopulation, temps reel du monde, Refresh par DescribeScript, compte des lignes
  « Population pleine » de DescribeMessages (0 / 1 / 1 / 1 a 9 s / 2 a 34 s). Friend de UHOLGameScreen pour BindPlayerState.
- Build PASS 14:10 (129 s, file derriere Elrond). ROUGE VU 14:14 : tests 1320/1321, seul PopulationFullThread (script non branche).
  Vert : quick_check tests relance apres l'edition du script (resultat et commit : voir EtapeDefini / rapport).
- Localisation (3) verifiee : Config\Game.ini rassemble Script/*.as ET Source ; la cle HOLGameScreen,PopulationFull existe deja en C++
  (LOCTEXT de la note de file, meme texte source « Population pleine ») : meme cle + meme source = une seule entree, pas de doublon.
- NON FAIT : scenario `alert_population` + .look + preflight (budget d'appels) ; verbe hol.Cheat de population non cherche.
- Suivant : scenario alert_population (remplir la population : chercher un verbe hol.Cheat, sinon produire jusqu'au plafond), puis
  recherche terminee / batiment acheve selon la reponse a Q-alerts-minor.
- 1re relance 14:15 : script non compile (GetPopulationNow : float& C++ = float32 en script), corrige ; vert NON obtenu (tests 1320/1321), rien commite. Appels GALADRIEL-1115g : 44.
- GALADRIEL-1115h (14:24-14:3x, arret de seance demande, aucun build ni test lance, rien modifie dans le code, rien commite) :
  CAUSE LUE (Saved_Tests\Logs\quick_tests.log l. 19214) : « Expected 'its part in script' to be not null » (test l. 32). ScriptScreen
  est cree dans UHOLGameScreen::NativeOnInitialized, que UUserWidget::Initialize ne lance qu'avec un contexte de joueur ; le test cree
  l'ecran avec le World seul -> pas de partie script. Les hypotheses (a)(b)(c) ne sont pas en cause a ce stade (jamais atteintes).
  PROCHAINE ETAPE (test seul, C++ -> build) : modele HOLNewGameSitesBoxTests.cpp l. 36-61 : ULocalPlayer + APlayerController
  (Controller->Player, Player->PlayerController, World->AddController), puis `NewObject<UHOLGameScreen>(GetTransientPackage())`,
  `SetPlayerContext(FLocalPlayerContext(Player, World))`, `Initialize()` (NativeConstruct non lance : pas de Refresh ni de
  controleur requis), puis BindPlayerState(Player) comme aujourd'hui. Ensuite build, quick_check -Parts tests ; si encore rouge,
  lire l'assertion : piste (a) appeler RefreshPopulationAlert() en premier dans Refresh() du script.
- GALADRIEL-1116a (15:02-) : correctif du test (ULocalPlayer + APlayerController, SetPlayerContext, Initialize, modele HOLNewGameSitesBoxTests). Build PASS 15:22 (674 s, file + memoire). VERT 15:26 : quick_check tests 1321/1321 (rouge vu 2026-11-15 14:14 : 1320/1321, assertion l. 32). Commit branchement + test, puis merge main, puis G2 scenario alert_population.
- GALADRIEL-1116a suite : commit a11ed2a (branchement + test vert), merge main 825b841 (conflit de ce journal seul, deux cotes gardes ; main touche aussi le script l. 1384+ et BuildingTypes.csv/.uasset : fusion auto), LaunchCheck apres merge vide 15:28, quick_check tests relance sur la fusion 15:29. G2 NON COMMENCE (budget d'appels) : hol.Cheat.Population N n'ajoute que du plafond (AddCheatPopulation, N negatif non borne ici : a verifier) ; aucune ligne de journal quand le fil recoit une ligne (RebuildMessages ne logue rien) : le scenario lira 'hol.HUD.Report 1' (HUD messages) apres avoir rempli la population (produire des Paysans au centre par la vraie touche jusqu'au plafond, ou plafond reduit par cheat negatif), waitlog sur 'Population pleine' du rapport ; capture du fil de messages.
- GALADRIEL-1116b (15:34-, sans C++, LaunchCheck vide 15:37) : G2 FAIT : scenario alert_population (.txt/.look/.json, 15097fc + 329037a) : Plain seul, 9 Paysans offerts (9/10), vrai clic 960 540 sur le forum + vraie touche A : Paysan en file, population 10/10 des la mise en file (place reservee), 'HUD messages: [Population pleine]' ~1 s apres. Lecons : waitlog = sous-chaine LITTERALE (pas de regex : '.*' a fait 2 timeouts) ; la ligne vit ~8 s (D446), la capturer juste apres la mise en file, pas a la sortie du Paysan (20 s). preflight PASS 15:47 (71 s), PNG vu ; fiche Retours\E-alert_population.md [A ESSAYER] VU A L'ECRAN. Suivant : D804 B reste (recherche terminee, batiment acheve, son court, ping, attaque ailleurs) = C++ (aucun evenement relaye au HUD).
- GALADRIEL-1116b suite (sans C++) : D804 B 'attaque ailleurs 1 fois / 30 s' REGLE PURE : HOLAlertRules::ShouldAlertAttackedElsewhere(Watch, bHitInView, Now, RepeatSeconds) + FHOLAttackAlertWatch, AttackRepeatSeconds = 30 (un coup dans la vue ne dit rien et ne retarde pas la ligne suivante). ROUGE VU 15:52 (stub false : tests 1321/1323, seuls Test_AlertAttackedElsewhereOncePer30s / Test_AlertAttackedInViewQuiet), VERT 15:55 (1323/1323), un commit test + correctif. RESTE (C++) : detecter le coup hors de la vue (HOLMinimapRules.Detect sait deja les PV en baisse) et brancher la ligne + son ; recherche terminee, batiment acheve, sons courts, ping.
- SAURON-1116e (16:11-, worktree Sauron, branche visual, C++, D829 BUG utilisateur) : (a) LU : la file n etait PAS refusee a population pleine (TryEnqueue ne regarde pas la population ; CheckProduction dit PopulationFull sans bloquer) ; la place etait reservee au DEMARRAGE de la tete (HOLProduction::Advance : tete non demarree tant que FreePopulation < places, Used = unites + items demarres). A 10/10 la tete restait a 0 % ; la ligne du fil ne part qu au PASSAGE a plein (script RefreshPopulationAlert, transition, 30 s) : si la population etait deja pleine quand on met en file, aucune ligne (cause probable du constat ; le PNG de l essai la montrait car 9/10 -> 10/10 a la mise en file). (c) FAIT (non compile au moment de l ecriture) : Advance demarre la tete quelle que soit la population, verifie a la FIN (sans place : reste a 100 %, file derriere en attente) ; HOLProduction::IsWaitingForRoom ; subsystem : Used = unites seules (plus GetStartedPopulation) ; HUD UpdateHead : PopulationFull si 100 % et plein (barre pleine gardee + note) ; CheckPopulationBlocked : bloque = IsWaitingForRoom + plein, a la transition AddThreadLine('Population pleine') + log 'HUD: production held at 100 %'. (b) test vraie chaine Source/HeroesOfLiberty/Tests/HOLProductionPopulationEndTests.cpp HeroesOfLiberty.Production.PopulationCheckedAtEnd (9 Peasants servis, 2 en file par IssueProductionOrder, 9/10 en production, 2e a 100 % non sorti, 10/10, MarkUnitDying -> sort) ; Production.Advance reecrit en D829. Scenario alert_population : A deux fois, capture au blocage. Build lance 16:20 (un seul, file d attente). Rouge NON VU (un seul build, consigne).
- 2026-11-20 GALADRIEL-1120b (D833, sans C++, branche specialization-trial) : note de file Population pleine a cote de la barre (HOLQueuePanel.as : ligne de statut en UHorizontalBox, barre Fill, note Auto ; regle HOLQueuePanel::BarRightEdge/NoteLeftEdge/NoteGap ; test aa1eae8 rouge puis fix 3ba1ebe ; non compile, non vu a l'ecran). Duree 15 s de la ligne du fil = C++ (FHOLForumThread::LineSeconds 8.0 constexpr, HOLForumMessages.h:234) : a grouper (duree par FHOLForumMessage + AddThreadLine(Line, Seconds) expose). Son : HOLGameScreenScript.as:1537 appellera Screen.PlayAlertSound() une fois le son D804 B choisi.
- 2026-11-20 09:40 GALADRIEL-1120c (D833 part script, sans C++) : HOLAlertRules::PopulationFullLineSeconds = 15 (cd72b61 test + stub 0 rouge, 8fe8b75 correctif) ; HOLGameScreenScript.as RefreshPopulationAlert appelle PlayAlertSound() (ping 1 kHz provisoire /Engine/EngineSounds/1kSineTonePing, le meme que l alerte attaque ailleurs). L appel AddThreadLine(Line, 15.0) NON fait : la signature a 2 arguments n existe pas dans cet arbre (seul AddThreadLine(FText)) ; TODO commente a la ligne. Autre appel a mettre a jour avec le wrapper : HOLShortMessage.as:187.
- 2026-11-20 GLORFINDEL-1120c (ability-columns, C++ NON COMPILE, 2026-11-20 09:40) : D833 part C++ duree par ligne du fil : FHOLForumMessage::LineSeconds (defaut DefaultLineSeconds 8 s), FHOLForumThread::AddText(Text, Seconds, Now), Expire/GetOpacity lisent la duree de la ligne, UHOLScriptScreen::AddThreadLine(Line, Seconds = 0.0) (<=0 = 8 s), UHOLForumThreadHolder::AddLine ; test script Test_VictoryMessagesThreadLineSeconds (stub 5f488de, correctif 5eada0d) ; reste script : HOLGameScreenScript.as:1537 passe 15 s (autre agent).
- 2026-11-20 11:2x GALADRIEL-1120g (D804 B, sans C++, specialization-trial, non compile) : regle pure HOLAlertRules::MinorAlert(Kind, NowS, LastShownS) -> FHOLMinorAlert {bShow, LineSeconds 8, SoundKey nMinorResearchDone/nMinorBuildingDone, bPingMinimap} + MinorAlertText + EHOLMinorAlertKind ; 1 ligne / 30 s PAR TYPE ; tests Test_AlertMinorFirstGoes / Test_AlertMinorOncePer30s (4e7893e test + stub, 673c05a correctif ; rouge NON VU a l execution, aucun build). BRANCHEMENT NON FAIT : evenements illisibles du script (AHOLBuilding::GetConstructionState et FHOLMinimapEntity sans etat ; techs : seulement HasTechnology(Player, nom), aucune enumeration ni evenement). LIAISON A EXPOSER (lot C++) : UHOLScriptScreen::GetMinorEventsNow(int32& OutBuildingsFinished, int32& OutResearchesDone, FVector2D& OutLastBuildingLocation, FVector2D& OutLastResearchBuildingLocation) = compteurs cumules du joueur montre (le script compare au dernier relu, comme GetPopulationNow) ; son : PlayAlertSound() = ping provisoire 1 kHz pour les deux types (SoundKey reserve) ; ping minimap : aucune API script pour poser un ping a un lieu hors detection de PV (HOLMinimap::Detect) : exposer ou injecter un FHOLMinimapAlert(X, Y) dans MinimapAlerts du script (possible sans C++ une fois le lieu connu). Aussi : HOLGameScreenScript.as:1553 peut passer AddThreadLine(Text, PopulationFullLineSeconds) (la signature a 2 arguments est dans l arbre depuis le merge main).
- **GLORFINDEL-1120i (2026-11-20, NON COMPILE)** : accesseur UHOLScriptScreen::GetMinorEventsNow ECRIT (HOLScriptScreen.cpp/.h, e65331c stub + test Script\UI\HOLMinorEvents_Test.as, 09207f7 correctif) : batiments finis du joueur montre (FHOLBuildingIterator, IsFinished, GetOwnerPlayer) + FHOLTechnologyState.Done.Num(), cumul par FHOLMinorEventsWatch (1re lecture = base). OutLastResearchBuildingLocation reste (0,0) : aucun enregistrement replique du batiment qui recherche (plus gros : champ replique ou RPC). Reste : build, quick_check tests, brancher la regle MinorAlert dans HOLGameScreenScript.as.
- 2026-11-20 15:48 FARAMIR-1120pb (D804 B, partie script, sans C++, zone-effect, NON COMPILE, rouge NON VU a l'execution) : f311fd1 test rouge seul (Test_AlertMinorStepFromCounters, Test_AlertMinorStepNoReplay ; stub HOLAlertRules::MinorStep vide) puis 9173e34 correctif : MinorStep(FHOLMinorWatch, Buildings, Researches, Now) (1re lecture = base, un compteur qui monte = evenement, MinorAlert par type, 1 ligne / 30 s par type) ; HOLGameScreenScript.as RefreshMinorAlerts() a chaque Refresh (apres RefreshPopulationAlert) lit GetMinorEventsNow, ShowMinorAlert : AddThreadLine(texte, 8 s) + PlayAlertSound() (ping 1 kHz provisoire, SoundKey reserve) + FHOLMinimapAlert(X,Y) dans MinimapAlerts si le lieu est connu. RESTE : sons propres par type (assets), lieu de la recherche (OutLastResearchBuildingLocation reste (0,0) : champ replique ou RPC = C++), build + quick_check tests.
- 2026-11-20 23:00 GALADRIEL-1120sa (REMPLACE par 1120sb : delai 15 s etait une lecture fausse de D833) : voir ligne suivante.
- 2026-11-20 23:02 GALADRIEL-1120sb (specialization-trial, sans C++, NON COMPILE, rouge NON VU a l'execution) : D833 = la LIGNE reste 15 s (duree d'affichage), PAS un delai : HOLAlertRules::PopulationRepeatSeconds remis a 30 (defaut A de Q-alerts-minor) ; PopulationFullLineSeconds = 15 passe a AddThreadLine au RefreshPopulationAlert ET a HOLShortMessage.as CheckPopulationHeld ; test 8ce795d Test_AlertPopulationFullLineStays15s (duree 15, delai 30, regle : plein a 16 s = silence, a 31 s = ligne ; rouge attendu sur ae69298 : delai 15), correctif fd9a795 ; ancien test Test_AlertPopulationFullAtMostOncePer15s supprime. Son court D804 B : PlayAlertSound() deja appele au RefreshPopulationAlert (ping provisoire) ; CheckPopulationHeld ne joue pas de son (evite un double ping avec RefreshPopulationAlert, a trancher). Reste : quick_check -Parts tests.
- GALADRIEL-1121b 10:55 : sons propres par type (d5cd309 test rouge Test_AlertMinorStepSoundPerKind, cce215c fix HOLAlertRules.as AudioEvent + HOLGameScreenScript.as ShowMinorAlert), non compile. Ligne D872 deja dans main (0301e05, 8ab3d8a). Reste C++ : source des coups recus + vue pour 'attaque ailleurs' (ShouldAlertAttackedElsewhere non branche), lieu de la recherche.
