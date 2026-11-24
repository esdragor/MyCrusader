# FARAMIR-1124e (F1 : banniere plantee par l'IA de l'Aube)

Joueur : la banniere de l'IA est bien plantee sur le heros, a 2 m des blesses ; la Lumiere sacree tombe sur eux. Aucun correctif necessaire.

APPELS_DECLARES: 17 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 11:28:31 / 11:36 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: sans objet   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: aucun (la banniere est correcte, pas de rouge a produire) + assertit l'effet : non applicable   TEST_ROUGE_SEPARE: non (rien de rouge ; commit unique, scenario durci)
COMMITS: 1 (6b567f1 a 11:35 ; ai_sacred_light.txt : `waitlog banner at (` + `waitlog planted` avant `waitlog Sacred Light cast`), merge_main_fait: non (consigne : ne pas fusionner main)
TESTS: play.ps1 ai_sacred_light -Hidden -Root <worktree> deux fois (1re : scenario d'origine, journal lu ; 2e : avec les deux waitlog, atteint « ready », banniere (14908, -19) plantee, cast step 569) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: ai_sacred_light : captures ouvertes (after) ; le .look attend « zone de lumiere sur les blesses » : VU (cercle dore sur la melee rouge/bleu, ring interieur + exterieur). Pas de nouvel E-*.md. Commande : play.ps1 ai_sacred_light -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir
QUESTIONS: aucune

## Journal lu (hol.AI.LogPowers 1), step 159
- `Hero ability: slot 1 plants his banner at (14992, 28)` (sim)
- `AI slot 1 step 159: banner at (14991, 27) planted` (le point d'ordre PlanBanner, tronque en int : ecart 1 cm, pas une vue interpolee notable)
- `powers think ... decision cast=true row 0, banner (14992, 28), nearest wounded 200.29 cm, range 1200 cm` : banniere != (0, 0), plus proche blesse 200 cm <= 1200 cm.
- `Zone effect: slot 1 puts a Sacred Light at (14992, 28)`, placeholder 600 cm 5.0 s.
- Hero sheet slot 1 : level 5, banner 14992 28, until 1059. Centre principal de l'IA (16400, 0) : la banniere est a ~1400 cm devant son forum, dans la melee (le heros y combat : « #25 ManAtArms killed by #18 Hero »).
- Le 2e cast (step 559) vient sans nouvelle plantation (banniere encore active ou recharge) : non regarde en detail.

## FAIT / RESTE
FAIT : F1 verifie, banniere correcte ; scenario durci (assertion « planted » avant le cast).
RESTE : aucune assertion numerique (≠ (0, 0), <= 1200 cm) : `waitlog` ne teste que la presence d'une ligne ; une assertion de valeur demande une regle pure script (ex. test sur PlanBannerOf, deja couvert par GLORFINDEL-1123pb) ou une commande scenario `assertlog <regex>` (outil, hors tache).
F2 deja fait (04b5db0) ; F3 attend le C++ de Glorfindel : non lances.

## NON VERIFIE
- Position reelle du heros comparee a la banniere : la ligne de plantation est la place du heros lue par HOLFactionPowerServer::HeroPlaceOf (HOLHero::GetHeroLocation) ; je n'ai pas lu une position du heros independante, donc je ne prouve pas que GetHeroLocation n'est pas une vue interpolee en reseau (mode net non joue, net mode 0). TYPES_A_EXPOSER: aucun.
- Sans combat (aucun blesse), PlanBanner ne plante pas : les « powers think wounded 0 » montrent la banniere de repli (centre d'armee), cas non teste ici.

## Divers
BUILDS_DEMANDES: 0   TESTS_SCRIPT_AJOUTES: 0 (assertions de scenario seulement)   DIAGNOSTICS_AS: 0 erreur (aucun .as touche)   EDITS_REFUSES: 1 (Edit sans Read prealable, rejoue)
ANOMALIES_PROCESSUS: le 1er play.ps1 a depasse 180 s (passe en arriere-plan, termine normalement).
PROCESSUS_DETACHES: 1 UnrealEditor.exe (PID 55480, 2e play.ps1) arrete par stray_processes -Stop, aucun reste
GIT_STATUS_PRINCIPALE: inchange hors docs voulus : oui (seul ce rapport ajoute)
