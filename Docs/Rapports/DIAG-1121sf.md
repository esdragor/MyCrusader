# DIAG-1121sf (integ-44, C:\Users\baudo\Documents\HeroesOfLiberty-Sauron)

APPELS_DECLARES: 11
HEURE_DEBUT/HEURE_FIN: 20:36 / 20:38
PREMIER_BUILD: sans C++ (RELECTURE_CPP, NON COMPILE)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (correction de tests deja rouges : DIAG-1121se) ; assertit l'effet : oui (les tests existants)
COMMITS: 2 (7d5bf9d D818 follow tests, a4f3531 ShieldsUp morale), dernier 20:38, merge_main_fait: non (non demande)
TESTS: aucun lance (consigne : ni build ni quick_check). TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune

FAIT:
- D818 (GDD.md:3594) dit « clic droit sur un allie = Suivre », sans cas cible-dans-la-selection ; la regle ecrite de GLORFINDEL-1121sh (allie hors selection = Suivre) est conforme. Corrige les 2 TESTS, pas la regle.
  - Script/Orders/HOLFollowOrders_Test.as Test_ApplyRightClickOnAllyStartsFollowForTheOthers : selection {3,4}, cible 9 (hors selection) ; 2 suiveurs, 9 absent du livre.
  - Script/Player/HOLControllerCommandsC10a_Test.as Test_ControllerCommandsC10aRightClickOnAllyStartsFollow : selection {1,2}, cible 9 ; 2 suiveurs.
- Source/HeroesOfLiberty/Tests/HOLHeroTalentEffectTests.cpp (ShieldsUp seul) : include HOLMoraleProcessor.h, UHOLMoraleProcessor cree + CallInitialize, lambda RunStep l'execute avant le combat (modele HOLHeroAbilityMatchTests.cpp:73-90).
AUTRES TESTS C++ : 17 fichiers executent le combat seul (Run(*Combat,...)) ; Grep « aura|Dawn Aura|IsInAura|Morale » : 0 hit dans tous sauf HOLHeroAbilityMatchTests (deja corrige, 129fa9a) et HOLMoraleRulesTests (tests du processeur). Dans HOLHeroTalentEffectTests.cpp : DawnStep (cri Close Ranks, vitesse, pas d'aura de degats) et HealersHands (HOLHealProcessor lit ses propres sources d'aura, HOLHealProcessor.cpp:183-207) laisses tels quels ; seul ShieldsUp lit IsInAura via le combat (HOLCombatProcessor.cpp:788) et le levier de moral.
RESTE: la session principale doit recompiler (Tools\build.ps1 -LiveCoding=0) puis lancer quick_check -Parts tests (HeroesOfLiberty.HeroTalent.ShieldsUp, Orders Follow, ControllerCommandsC10a).
NON VERIFIE: tout (rien compile ni lance). Doute : la valeur 10.2 attendue (-15 % aura) vient du levier de moral du processeur ; si le fragment de moral n'est pas pose par CallInitialize seul, ShieldsUp reste rouge.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: inchange hors docs (voir git status de la session)