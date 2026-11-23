# DIAG-1123d : conflits integ-56 (Haldir)
Heure debut/fin : 11:12 / 11:14. Appels : ~6. Pas de build, pas de test, aucun push.

- Conflit 1 (merge 10db129, integ-25) : Script/AI/HOLAIBrain.as, hunk unique pres de la ligne 1086. Resolu en gardant les deux : ThinkRage, ThinkPowers (main), ThinkGarrison (integ-25), chacun appele une fois (lignes 1121-1123). Marqueurs retires par regex, aucun reste. Commit 7905f99.
- Merge 2943ff1 : auto-merge sans conflit (HOLAIBrain.as, HOLAIPowerCall.as, HOLFactionPowerServer.as, HOLRpcPlayer.as, test, scenario ai_sacred_light). Commit 77e7a66.
- Merge af8ff83 : auto-merge sans conflit (FortificationsInProgress.md, HOLCombatProcessor.cpp/.h, HOLBuildingShotTests.cpp, scenario tower_wood). Commit ffbb934.
- Tip integ-56 : ffbb934.

log --oneline -5 :
ffbb934 Merge commit 'af8ff83' into integ-56
77e7a66 Merge commit '2943ff1' into integ-56
7905f99 Merge commit '10db129' into integ-56
af8ff83 tower_wood: sweep of 6 arrow reports and captures every 3 frames around the in-flight moment
10db129 S4-b: the AI garrison call reads FHOLAIBuilding::SecondsSinceHit when no hit time is given (green)

NON VERIFIE : compilation des scripts (la variable Level doit etre en portee a l'appel de ThinkGarrison, ligne 1123) ; le C++ de HOLCombatProcessor et de la garnison (integ-25 et af8ff83) n'est pas compile. A lancer par la session principale : build puis quick_check du worktree.
ANOMALIES : aucune. IN_USE.txt de Haldir remis a libre.