# FARAMIR-1121sb

APPELS_DECLARES: 24 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 19:18 / 19:21 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build, sans C++)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: aucun (tache = scenario, pas une regle) ; assertit l'effet : sans objet
COMMITS: 1 (4146ec1, 19:20, branche zone-effect), merge_main_fait: non (aucun merge demande, brief : DLL non verifiee)
TESTS: preflight talents_heal : check.ps1 PASS (34 s, scenario atteint ready, aucun warning/erreur journal) mais preflight FAIL (2) = les deux WARN « pas de ligne HUD overlap » et « pas de ligne Sanity » (hol.HUD.Report / hol.Test.Sanity absents, ils existaient deja avant mon edit) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: talents_heal : preflight FAIL (2, voir plus haut), PNG NON ouverts, LOGIQUE SEULEMENT, aucune commande a donner a l'utilisateur
QUESTIONS: aucune
FAIT / RESTE:
- Ligne [ ] de la section Faramir = talents_heal_cell (« ++ » dore dessine par-dessus la fenetre Arbre de talents). Pas de C++ pris : la marque du monde est dessinee par du C++ (Presentation\HOLHealEffectSubsystem.cpp), le vrai correctif de couche reste a trancher par le proprietaire du rendu.
- Contournement sans C++ cote scenario (4146ec1) : Tools\Scenarios\talents_heal.txt, apres capture talents_heal_base : hol.Cheat.Wound 100 + waitlog + frames 40 avant d'ouvrir la fenetre (plus de soin donc plus de marque au moment de la capture talents_heal_cell) ; le wound 50 existant apres fermeture rejoue le « ++ ».
- Tentative de mise a jour du .look : integree dans le meme commit (chaine remplacee, 1 occurrence).
- Points « sans C++ » du releve GDD verifies deja faits ou bloques : B4 forêt/gue (Script\Terrain\HOLTerrainRules.as, F4 neutre D844, F5 gue en place), B2 XP d'exploration (C++ en place), N3 Nomade IA (HOLAINomadThink branche dans HOLAIBrain.as:200-205), B3 furtivite (Script\Fog interdit), Patrouille/Suivre (Q sans reponse).
- RESTE : relancer preflight talents_heal et corriger les 2 WARN, ouvrir les 3 PNG (talents_heal_cell en priorite : plus de « ++ » sur la fenetre), puis cocher/retirer la ligne de la file. EtapeDefini.md et Auto_Journey_File.md NON mis a jour (calls epuises).
NON VERIFIE: que hol.Cheat.Wound 100 remet bien les unites a 100 % et que la marque de soin disparait en 40 images ; aucun PNG ouvert.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Faramir
ANOMALIES_PROCESSUS: preflight a lance un editeur (cache via preflight, 34 s) sans anomalie ; DIAGNOSTICS_AS: sans objet (aucun .as touche)
Fichiers touches : HeroesOfLiberty-Faramir\Tools\Scenarios\talents_heal.txt, talents_heal.look ; ce rapport.
GIT_STATUS_PRINCIPALE: non verifie (aucune ecriture dans la copie principale hors ce rapport)
