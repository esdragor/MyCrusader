# ARWEN-1123c (SP3 regles du bouton de specialisation + routage FactionBox, integ-37, sans C++)

APPELS_DECLARES: ~25 (arret a 25 vus ; heure d'arret 09:40 non atteinte, 09:33 a la fin)
HEURE_DEBUT/HEURE_FIN: 09:24 / ~09:34 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (sans C++)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 331b87e, vu a 09:32 : quick_check tests 2099/2105 (6 tests de HOLSpecializationSlot_Test.as rouges, "Array index out of bounds" attendu sur le bouchon) ; assertit l'effet : oui (la regle serveur reelle HOLSpecialization::Start/Advance/Cancel sur un vrai etat, la vue en est calculee)   TEST_ROUGE_SEPARE: oui (331b87e bouchon+tests, puis dca9df1)
COMMITS: 2 (331b87e rouge, dca9df1 vert), dernier ~09:32 ; merge_main_fait: non
TESTS: quick_check -Root worktree -Parts tests : 2099/2105 au rouge ; VERT NON LANCE (budget d'appels) ; TESTS_TAUTOLOGIQUES: 0. NB : lancer quick_check avec le Tools\Tests\quick_check.ps1 de la copie principale et -Root du worktree echoue (scripts : "unknown super type UHOLConstructionBoard"), utiliser celui du worktree.
ESSAIS: aucun (pas de scenario specialization_slot, pas de preflight)
QUESTIONS: aucune
BUILDS_DEMANDES: 0   TESTS_SCRIPT_AJOUTES: 6 (FactionBoxNameGivesTheBoxIndex, SpecializationSlotDarkUntilATierIsOpen, ...TemplarsShowThreeOptions, ...BarCountsDownWhileTheUnlockRuns, ...MovesToTheNextTierOnceApplied, ...ClickStartsThenCancelsOnTheRealState)
TYPES_A_EXPOSER: AHOLPlayerState::GetSpecialization (retourne FHOLSpecializationView ; pas UFUNCTION, donc illisible du script client) ; palier de faction et Forum complet cote client a verifier (HOLNomad::HasCompleteForum)
FAIT / RESTE:
- TACHE 1 : HOLFactionSlot::BoxIndexOfName + ClickWidget route FactionBox<n> vers ClickFactionBox (HOLGameScreenScript.as, avant HeroPortrait) ; test rouge vu, vert NON LANCE.
- TACHE 2 (partielle) : Script\UI\HOLSpecializationSlot.as : FHOLSpecializationSlotView (allume = palier ouvert, 2 ou 3 options "Voie A/B/C du palier N" = table HOLGameNames.cpp, barre = 1 - reste/delai, texte "N s" arrondi au-dessus), ClickOption (Start / Cancel de l'option en cours / rien sinon). Regles pures testees.
- RESTE : widget (bouton + panneau + barre dans BuildFactionSlot / RefreshFactionSlot), liaison GetSpecialization, appels ServerStartSpecialization / ServerCancelSpecialization, scenario specialization_slot, preflight.
NON VERIFIE: le vert (compilation du script de dca9df1 jamais lancee, dont le routage ClickWidget) ; aucun widget ; libelle du bouton 'Specialisation' non verifie au GDD ; Placeholders.txt non touche (aucun provisoire ajoute encore).
PROCESSUS_DETACHES: voir la sortie du dernier geste dans la reponse (stray_processes -Stop lance)
ANOMALIES_PROCESSUS: aucune ; quick_check de la copie principale avec -Root worktree : echec de compilation des scripts (voir TESTS)
GIT_STATUS_PRINCIPALE: Auto_Journey_File.md, EtapeDefini.md modifies par moi (lignes propres) + docs deja presents : oui
