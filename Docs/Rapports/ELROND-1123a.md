# ELROND-1123a : XP du duel = 150 (D859 n. 9), worktree HeroesOfLiberty-Elrond, branche integ-25

NON COMPILE (a la redaction) : le build de ce worktree etait en file derriere Faramir et Celeborn (build.ps1 -LiveCoding=0 -Wait lance a 09:00:41, "position 1 of 1 waiting" a 09:05). Voir BUILD plus bas.

APPELS_DECLARES: 17 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 08:58:23 / voir fin de reponse (le build en file n'a pas change l'heure d'arret)
PREMIER_BUILD: 09:00 (lance, en file)   BUILD: NON COMPILE (en file)   LAUNCHCHECK_DEBUT: vide a 08:59 (binaire a jour avant mes modifications)
TEST_ROUGE: 9d7f538 (HeroesOfLiberty.HeroXP.Duel150, bouchon poids Duel = 0 ; rouge NON VU a l'execution : aucun build termine) + assertit l'effet : non pour la partie regle (valeur calculee par GetXP puis GetLevel : 1 duel = niveau 1, 2 duels = niveau 2, pas d'effet match) ; l'effet en match est couvert par le test script Test_DuelXPGoesToTheWinnerOnly (qui verifie le gagnant, pas le +150 en feuille de heros)   TEST_ROUGE_SEPARE: oui (9d7f538 test + bouchon, da0dc68 correctif)
COMMITS: 3 (9d7f538 test rouge, da0dc68 correctif C++ + script, 8fb6e5e test script), dernier a 09:04, merge_main_fait: non (integ-25 = main en contenu, rien a fusionner)
TESTS: aucun lance (build en file). Ecrits : C++ HeroesOfLiberty.HeroXP.Duel150 (HOLHeroXPTests.cpp), script Test_DuelXPGoesToTheWinnerOnly (HOLDuelRewards_Test.as). TESTS_TAUTOLOGIQUES: 0 (valeurs assertees calculees par GetXP / GetLevel / XPWinnersOfEnded ; la constante 150 est une decision D859 n. 9, assertee a la main comme les autres tests de HOLHeroXPTests)
ESSAIS: aucun (pas de build)
QUESTIONS: aucune (defaut : XP fixe 150 a poids 100 %, un reglage = HOLHeroXP::DuelXP)
FAIT / RESTE :
 - FAIT : EHOLXPSource::Duel (HOLHeroXPRules.h), poids 100 (HOLHeroXPRules.cpp:28), constante HOLHeroXP::DuelXP = 150, borne de ToSource etendue a Duel (HOLSmallRulesReadLibrary.cpp:15, commentaire .h:12), nom "duel" dans le journal "Hero XP" (HOLHeroSubsystem.cpp).
 - FAIT : liaison minimale : AddXP n'etait PAS expose au script ; ajout de UFUNCTION(BlueprintCallable) UHOLHeroSubsystem::AwardDuelXP(int Slot) (HOLHeroSubsystem.h/.cpp, fichiers non listes dans "je prends" mais necessaires).
 - FAIT : HOLDuelRewards::XPWinnersOfEnded (bXpBonus + TriumphSlot de HOLDuel::Effects) et ApplyXP (HOLDuelRewards.as) ; appel d'une ligne dans HOLDuelLedger.as AdvanceDuels (fichier non liste dans "je prends").
 - Grep L18-d : aucun test ne compte les valeurs de EHOLXPSource (HOLHeroXPMatchTests.cpp ne cite que Combat / Command).
 - RESTE : build vert, lancer HeroesOfLiberty.HeroXP.* et le script Duel, voir le rouge de 9d7f538 (git stash / checkout du bouchon) ; essai en jeu : duel gagne -> ligne "Hero XP: slot N +150 (duel)" ; gagnant de HOLDuel::Effects pour un duel termine par double KO / temps ecoule : bXpBonus a verifier (HOLDuelRules.as:291). Commande build : Tools\build.ps1 -LiveCoding=0 -Wait (depuis le worktree), puis quick_check -Parts tests.
NON VERIFIE: tout ce qui est compilation C++ (UFUNCTION sur UWorldSubsystem, UHT), compilation Angelscript (UHOLHeroSubsystem::Get(), AwardDuelXP vu du script, struct FHOLDuelEnded), exactitude du nom de preset HOLDuel::Preset() dans le test script, rouge d'execution du test Duel150, que AdvanceDuels ne tourne que cote serveur / solo (dit dans l'en-tete du ledger, non retest).
PROCESSUS_DETACHES: voir la reponse finale (stray_processes.ps1 -Stop lance en dernier geste)
ANOMALIES_PROCESSUS: build en file derriere deux autres worktrees (Faramir, Celeborn) : l'heure d'arret est atteinte avant son tour.
GIT_STATUS_PRINCIPALE: docs seulement (EtapeDefini.md, Auto_Journey_File.md, ce rapport) : oui (non relu par git status)
DIAGNOSTICS_AS: non releves (pas d'editeur lance)
BUILDS_DEMANDES: 1 (celui de ce worktree, en file)   TESTS_SCRIPT_AJOUTES: 1 (Test_DuelXPGoesToTheWinnerOnly)   TYPES_A_EXPOSER: UHOLHeroSubsystem::AwardDuelXP (fait)
