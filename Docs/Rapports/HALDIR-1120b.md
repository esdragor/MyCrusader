# HALDIR-1120b (scenario specialization_cheat)

APPELS_DECLARES: ~27 (au-dessus des 25 : lectures en trop pour trouver la facon dont la case se dessine)
HEURE_DEBUT/HEURE_FIN: 09:07 / 09:11 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build, voulu)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scenario, pas de regle) ; assertit l'effet : oui (le scenario attend `chosen 1 -1 -1` du Spec.Report et les lignes `Cheat: ... started and ended at once, tier applied 1` / `Specialization: slot 0 tier 1 option 1 applied`)
COMMITS: 1 (32ddb97, branche wall-line), dernier a 09:09, merge_main_fait: non (non demande ; df895b0 est deja ancetre de HEAD)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 ; pas de test python de forme (aucun modele de scenario-forme en python dans Tools : seulement des *_test.ps1)
ESSAIS: specialization_cheat : preflight NON LANCE (consigne), aucun PNG vu : LOGIQUE SEULEMENT, jamais joue. Commande a lancer par la session principale (apres build du lot C++ qui contient DoCheatSpecialization, morceau 8 NOT COMPILED) : `Tools\Scenarios\preflight.ps1 specialization_cheat` puis `play.ps1 specialization_cheat -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
QUESTIONS: aucune (voir ANOMALIES : une question de design possible, non ouverte)
FAIT / RESTE:
- Fait : Tools\Scenarios\specialization_cheat.txt / .look / .json (copie du json de specialization_button, mapSize Small). 2 captures : specialization_cheat_before (D = Voie A, F = Voie B), specialization_cheat_after (POINT : apres `hol.Cheat.Specialization 1 1`). 1 waitkey, 1 notereset par capture, hol.Test.Sanity en fin.
- Lecture HOLSpecializationRules.as:198-226 (Advance) : `RunningMs[Tier] += ElapsedMs` puis `RunningMs >= DelayMs(Tier)` => Applied. Donc `AdvanceNow(Slot, 3600000, true)` APPLIQUE le palier : un depassement du delai n'est PAS sans effet (hypothese de CELEBORN-1119n3 fausse a la lecture ; le scenario le verifie par `tier applied 1`). Seul `bForumStanding=false` avec ForumDown==ForumDownCancels annule ; la triche passe true. Advance rend le plus petit palier atteint : un seul palier par appel.
- Lecture de la commande : HOLControllerCommandsC2.as:147-159 (palier 1..3 sinon usage), HOLRpcPlayer.as:1028-1040 (ServerCheatSpecialization, garde `CheatRefused`), HOLPlayerController.cpp:3508-3537 (DoCheatSpecialization : StartNow avec les regles du plateau, puis AdvanceNow ; un depart refuse est journalise `... refused by the board, answer N` et rien n'est force).
- Option 0 = Voie A (case D, GridCell06), option 1 = Voie B (case F, GridCell07) (HOLCommandGridRules.as:108). `hol.Cheat.Specialization 1 1` choisit donc la Voie B, pas la case D.
RESTE : lancer le scenario apres le build du lot C++ ; si une waitlog de log (texte exact C++) differe, corriger la ligne.
NON VERIFIE: tout le scenario (jamais lance) ; textes exacts des waitlog (pris au source C++, pas au journal) ; que `waitlog chosen -1 -1 -1` se trouve bien apres le Spec.Report (le compte `chosen` vient du source HOLControllerCommandLibrary.cpp:190) ; le code de triche C++ est NOT COMPILED, donc la commande peut ne pas exister dans le binaire du worktree.
ANOMALIES / TYPES_A_EXPOSER:
- PREMISSE A CORRIGER : il n'existe pas d'etat visuel « choisi » de la case D. HOLCommandGrid.cpp:68-82 (GetOpenSpecializationTier) rend 0 quand le palier 1 est choisi et que le palier 2 n'est pas atteint ; HOLCommandGridRules.as:167-176 ne dessine alors ni D ni F. Apres la triche les deux cases sont donc VIDES (le .look le dit). Si l'utilisateur veut voir « Voie B choisie » dans la case, c'est une decision de design (ligne a ajouter au rendu de la grille) : question possible Retours\Q-specialization-chosen-cell.md (A : cases vides ; B : la case choisie affiche le nom grise ; defaut A), non ouverte ici.
- TYPES_A_EXPOSER : aucun obligatoire. Utile mais absent : une ligne de journal du HUD pour une case vide (hol.HUD.Report ne dit pas qu'une cellule est vide, donc pas d'assertion de HUD apres la triche ; seul `Spec.Report` prouve l'effet). Les lignes `RpcCheat: specialization received by the server` n'apparaissent qu'en reseau (non utilisees ici).
- Aucun fichier de code touche hors `Tools\Scenarios\` ; Crown.umap ignore.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Haldir
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: docs seulement modifies (AlertsMinor/Aube/Auto_Journey_File/Duel/EtapeDefini/Palier3/Placeholders*/Signals/SpecializationInProgress.md + rapports non suivis) : oui, dont Auto_Journey_File.md et SpecializationInProgress.md par moi
