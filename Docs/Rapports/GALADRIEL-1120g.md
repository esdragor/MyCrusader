# GALADRIEL-1120g (D804 B alertes mineures, sans C++, worktree HeroesOfLiberty-Galadriel, branche specialization-trial)

APPELS_DECLARES: 17 (arret a 25 ; heure d'arret 11:45)
HEURE_DEBUT/HEURE_FIN: 11:24:59 / 11:27:52 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 4e7893e (test + stub qui compile en theorie : MinorAlert renvoie bShow = false) ; assertit l'effet : oui (bShow / LineSeconds / SoundKey / bPingMinimap calcules par MinorAlert). Rouge NON VU a l'execution : aucun build ni quick_check (consigne).
COMMITS: 2 (4e7893e test + stub, 673c05a correctif), dernier a 11:27 (seuil 30 min), merge_main_fait: oui (premier geste, main = ff668a9)
TESTS: aucun lance ; TESTS_SCRIPT_AJOUTES: 2 (Test_AlertMinorFirstGoes, Test_AlertMinorOncePer30s dans Script\UI\HOLAlertRules_Test.as) ; TESTS_TAUTOLOGIQUES: 0 (les valeurs assertees sont calculees par MinorAlert, aucune recopie de constante hors comparaison a 30 s)
ESSAIS: aucun
QUESTIONS: aucune (D804 = B decide)
BUILDS_DEMANDES: 1 (le lot C++ de la seance, voir TYPES_A_EXPOSER) ; une fois compile : quick_check -Parts tests sur ce worktree, rouge attendu : aucun pour le correctif seul (les deux commits sont distincts mais le rouge n'est vu que si on teste 4e7893e).

TYPES_A_EXPOSER:
- `UHOLScriptScreen::GetMinorEventsNow(int32& OutBuildingsFinished, int32& OutResearchesDone, FVector2D& OutLastBuildingLocation, FVector2D& OutLastResearchBuildingLocation) const` (BlueprintPure, modele GetPopulationNow) : compteurs CUMULES du joueur montre (batiments passes a Finished, recherches terminees) ; le script compare au dernier relu, appelle `HOLAlertRules::MinorAlert(Kind, Now, Last[Kind])` et, si bShow, `AddThreadLine(MinorAlertText(Kind), LineSeconds)` + `PlayAlertSound()`.
- Pourquoi : AHOLBuilding::GetConstructionState n'est pas UFUNCTION (HOLBuilding.h:81), FHOLMinimapEntity n'a pas d'etat de construction (HOLMinimapView.h:24-48), les technologies ne sont lisibles que par HOLGameStateTestLibrary::HasTechnology(Player, nom) (bibliotheque de test, sans enumeration ni evenement) ; le log « Construction: slot N finished ... » (HOLBuildProcessor.cpp:1005) et la recherche terminee (HOLProductionSubsystem.cpp:617) sont cote serveur, sans delegue lisible du HUD.
- Ping minimap : pas d'API pour poser un ping a un lieu hors detection de PV ; une fois le lieu connu (OutLast*Location), le script peut ajouter un `FHOLMinimapAlert` (X, Y, Age 0) a `MinimapAlerts` (HOLGameScreenScript.as:185), sans C++.
- Son : PlayAlertSound() = ping provisoire 1 kHz (/Engine/EngineSounds/1kSineTonePing), le meme pour les deux types ; `FHOLMinorAlert.SoundKey` (n"MinorResearchDone" / n"MinorBuildingDone") est reserve pour des sons propres. A ajouter a Placeholders.txt par la session principale (je n'ai pas edite ce fichier : ligne « son d'attente des alertes mineures = ping 1 kHz » deja couverte si la ligne alerte population existe).
- Localisation : cles NSLOCTEXT("HOLGameScreen", "MinorResearchDone" / "MinorBuildingDone") a traduire dans Tools/Localization/fill_po.py.
- Au passage : HOLGameScreenScript.as:1553 peut maintenant passer `AddThreadLine(HOLAlertRules::PopulationFullText(), HOLAlertRules::PopulationFullLineSeconds)` (signature a 2 arguments presente dans l'arbre depuis le merge main) ; non fait ici (non compile, hors tache).

FAIT / RESTE:
- FAIT : HOLAlertRules.as : enum EHOLMinorAlertKind {ResearchDone, BuildingDone}, struct FHOLMinorAlert, constantes MinorRepeatSeconds = 30 et MinorLineSeconds = 8, MinorAlert(Kind, NowSeconds, LastShownSeconds) (1 ligne / 30 s PAR TYPE : l'appelant garde un temps par type), MinorAlertText(Kind) (« Recherche terminée » / « Bâtiment achevé »).
- RESTE : liaison C++ ci-dessus puis branchement dans HOLGameScreenScript.as (je suis l'unique agent sur ce fichier ; non touche) ; « attaque ailleurs » = ligne + son (regle ShouldAlertAttackedElsewhere deja faite) ; scenario d'essai.

NON VERIFIE: rien n'a ete compile ni execute (aucun build, aucun quick_check) : syntaxe Angelscript (enum global, NAME_None, operateur ternaire sur FName, accents UTF-8 dans NSLOCTEXT, fichier a fins de ligne LF avec avertissement git CRLF) non confirmee ; le rouge du premier commit n'a pas ete vu.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
ANOMALIES_PROCESSUS: aucune. Journaux : AlertsMinorInProgress.md (ligne ajoutee), EtapeDefini.md et Auto_Journey_File.md (une ligne chacun, ajoutees apres les lignes de Galadriel) ; l'ecriture PowerShell a perdu le guillemet de n"..." dans le journal (interpolation), sans consequence. IN_USE.txt non mis a jour (non commite, sans C++).
GIT_STATUS_PRINCIPALE: ' M' AlertsMinorInProgress.md, Auto_Journey_File.md, EtapeDefini.md (mes docs voulus) + FortificationsInProgress.md, NomadInProgress.md et Docs/Rapports/*-1120g.md d'autres agents ; aucun fichier de code touche dans la copie principale : oui.
