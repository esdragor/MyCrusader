# ARWEN-1126m : D920 Chapelle Dawn;Templars (spec-voie-1125m, sans C++)

APPELS_DECLARES: ~50 (au-dela du seuil de 45 : les attentes du quick_check ont coute des appels)
HEURE_DEBUT/HEURE_FIN: 11:15:30 / voir la fin du fichier (ligne FIN)
PREMIER_BUILD: sans C++   BUILD: sans C++ (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 7bfa015 (Test_ChapelIsBuiltByDawnAndTemplarsOnly, HOLTemplarsBrother_Test.as) + assertit l'effet : oui (colonne Faction du VRAI BuildingTypes compile : la liste que le filtre de construction lit)   TEST_ROUGE_SEPARE: oui (commit 7bfa015 seul, puis le correctif)   TEST_ROUGE vu a 11:22 : `Error: Test Completed. Result={Fail} Name={Test_ChapelIsBuiltByDawnAndTemplarsOnly}`, seul Fail du journal quick_tests.log a ce moment
COMMITS: 2 (7bfa015 test rouge, correctif ensuite), merge_main_fait: oui (HEAD contient main 5dc2318, fait par la sync de 11:04 ff51b25 ; merge-tree propre)
TESTS: quick_check -Parts tests (1er run coupe a 11:24 apres le rouge vu pour liberer l'editeur) ; 2e run apres regen : 2332/2332 PASS ; TESTS_TAUTOLOGIQUES: 0 (la valeur lue vient de la table compilee, pas d'une constante du test)
ESSAIS: aucun (spec_voie_queue : preflight non rejoue faute de temps)
QUESTIONS: aucune
FAIT / RESTE:
- FAIT : `Tools\Data\BuildingTypes.csv` ligne Chapel Faction `Dawn` -> `Dawn;Templars` (FTable::Allows decoupe sur `;`, HOLFactionRules.cpp:238) ; `Content\Buildings\BuildingTypes.uasset` regenere par `regen_tables.ps1 -Root <Arwen>` (exit 0, 0 erreur ; Unit/Building/Source ; seul BuildingTypes.uasset change).
- PREPARATION FUSION : la branche contient deja main (merge-base = main = 5dc2318) : fusion dans main = avance rapide, fc3b85f, 743e78f, 6cbbdbe, 736f246, scenario spec_voie_queue (.txt/.json/.look) et les deux tests inclus. Le scenario spec_voie_queue attendait la ligne Templars du binaire ; il faut maintenant rejouer son preflight sur un binaire a jour (le .uasset regenere + Factions.csv) : `Tools\Scenarios\preflight.ps1 spec_voie_queue` depuis le worktree Arwen.
- RESTE : (1) preflight spec_voie_queue (74 a 180 s) puis fiche E si PNG vus ; (3) passe d'integration (Haldir) ; (4) publication 0.4.4 [B] debloquee cote Chapelle, reste japonais « Voie A/B du palier 1 ».
NON VÉRIFIÉ: effet en jeu (case Frere infirmier dans la Chapelle d'un siege Templier) non vu : spec_voie_queue non rejoue ; 1er run de tests coupe, donc pas de compte complet avant correctif ; Placeholders.txt non concerne.
PROCESSUS_DETACHES: aucun (stray_processes.ps1 11:24) ; relance en dernier geste voir ligne FIN
ANOMALIES_PROCESSUS: le 1er quick_check a ete coupe par TaskStop (UnrealEditor-Cmd 42240 termine, rien de detache) ; UnrealEditor 13436 et ceux de ShivaProject ne sont pas a moi.
GIT_STATUS_PRINCIPALE: docs seuls (EtapeDefini.md, ce rapport) : oui

RESULTAT 2e quick_check tests (11:26-11:30, apres regen) : tests 2332/2332 PASS, quick_check PASS en 260 s ; Test_ChapelIsBuiltByDawnAndTemplarsOnly, Test_TemplarsSeatQueuesTheInfirmaryBrother, Test_TemplarsBrotherStaysClosedToOtherSeats verts. Correctif = 3e268ca.
FIN: 11:3x (arret avant l'heure dure 11:44 : au-dela de 45 appels vus ; la 2e tache de la file n'est pas prise).
COMMITS (rectif) : 2, 7bfa015 (11:22) et 3e268ca (11:28).
