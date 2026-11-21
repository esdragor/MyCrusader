# FARAMIR-1121sf

NON FAIT (aucun commit, aucun correctif) : la premisse de la tache (ZOrder / couche de la marque inferieure a celui des fenetres) ne se tient pas par lecture du code ; une question est posee.

APPELS_DECLARES: 18 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 19:37 / ~19:42 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: aucun (pas de commit). Assertit l'effet : sans objet.
COMMITS: 0, merge_main_fait: non
TESTS: aucun ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: Retours\Q-heal-mark-couche.md (defaut code : C, rien modifie)
BUILDS_DEMANDES: 0   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun
FAIT / RESTE:
- Lu HOLHealEffectSubsystem.cpp : la marque = DrawDebugLine(..., SDPG_Foreground) (l.40, scene) + DrawDebugString (l.41, canvas AHUD). Aucun widget, aucun ZOrder a baisser.
- Les fenetres sont du UMG : Screen->AddToViewport() ZOrder 0 (HOLGameHUD.cpp:79). Scene et canvas du HUD sont dessines AVANT Slate/UMG : une marque de scene ne peut pas etre « par-dessus » une fenetre UMG. Si le « ++ » se voyait sur la fenetre, c'est de la transparence de la fenetre (la marque transparait) ou un autre dessin : hypothese de la ligne [ ] C++ d'Auto_Journey_File.md (l.93) fausse en l'etat.
- Contournement `hol.Cheat.Wound 100` de talents_heal.txt (l.33-35) LAISSE en place (rien ne le rend inutile ; pas de build).
- RESTE : un PNG de talents_heal_cell SANS le contournement pour trancher (fenetre translucide ? => option A de la Q, script sans build) ; ligne [ ] C++ a reformuler par la session principale.
NON VERIFIE: tout ; aucun lancement ; la cause (translucidite) est une hypothese, non vue a l'ecran. Pas de lecture du frame de la fenetre Arbre de talents (hors HOLTalentPanel.as, qui ne porte aucun style).
PROCESSUS_DETACHES: aucun (rien lance)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: inchange par moi hors Retours\Q-heal-mark-couche.md et ce rapport : oui
