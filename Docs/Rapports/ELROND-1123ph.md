# ELROND-1123ph (integ-25) : scenario specialization_slot, preuve a l'ecran du bouton permanent de specialisation
Pour le joueur : a cote de la jauge de faction, le bouton « Specialisation » s'allume au palier 1, ouvre les voies, lance le deblocage avec barre et « N s ».
- Commits : 92055e3 (specialization_slot.txt/.look/.json), ad6aafd (.look avec tailles mesurees). Branche integ-25 prete ; main NON fusionne (consigne). Aucun C++, aucun build (BUILDS_DEMANDES: 0).
- Scenario : Aube (DawnOrder, slot 0) vs Legions Noires gelees ; hol.Cheat.Tier 1, hol.Cheat.Honor 100 ; vrais clics hol.Mouse.Click SpecButton puis SpecOption0 ; waitlog sur "SpecializationSlot: start 1 0" (client, HOLGameScreenScript.as:2121) et "Specialization: slot 0 starts tier 1 option 0, answer 0" (serveur) ; fin par attente reelle 30 s, waitlog "Specialization: slot 0 tier 1 option 0 applied" (HOLSpecializationSubsystem.cpp:168) puis Spec.Report "chosen 0 -1 -1".
- Preflight : PASS 2 fois (16:24, 16:25 apres dernier commit), 60 s de rejeu, 37.8 s de scenario.
- PNG : before = bouton « Specialisation » gris ~143x20 px sous « 100 / 100 » et « Lumiere sacree : cout 25, pret » ; during = barre bleue ~140x3 px + « 28 s » + « Voie A du palier 1 (annuler) » + « Voie B du palier 1 » ; after = bouton, barre, voies disparus, voie choisie montree nulle part.
- Fiche : Retours\E-specialization_slot.md ecrite, VU A L'ECRAN, GESTE reel, SON non entendu.
- DEFAUTS DU JEU (lignes proposees) :
  [ ] SP3 : le bouton garde le libelle fixe « Specialisation » (HOLGameScreenScript.as:2052) ; il ne dit ni « a choisir » ni la voie.
  [ ] SP3 : une fois le palier 1 applique, le bouton disparait (OpenTierOf = 0, HOLSpecializationSlot.as:114-124) : la voie choisie n'est montree nulle part (la tache voulait « voie affichee »). Prevoir une ligne/infobulle « Voie A (palier 1) » lue de View.Chosen.
  [ ] SP3 : la barre SpecBar est tres fine (~3 px de haut a l'ecran, 6.0 demande) : peu lisible.
  [ ] Placeholders.txt : ligne bForumComplete provisoire (rapport 1123pd) a verifier ; libelle « Specialisation » sans accent.
- TESTS_SCRIPT_AJOUTES: 0 ; TYPES_A_EXPOSER: aucun ; TEST_ROUGE: sans objet (scenario seul) ; DIAGNOSTICS_AS: sans objet (aucun .as touche) ; EDITS_REFUSES: 0.
- NON VERIFIE : son ; Voie B ; annulation par second clic ; Templiers ; Nomade ; reseau reel.
- Anomalie : l'horloge de l'agent (16:22 au 14e appel) ; depasse 25 appels de peu (compte individuel des appels groupes).
