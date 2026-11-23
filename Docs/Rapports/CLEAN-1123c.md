# CLEAN-1123c (etape 0a, nettoyage de Retours\, 2026-11-23 08:49 -> ~08:55, arret dur 09:05)

APPELS_DECLARES: 9 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 08:49 / 08:52
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (docs seulement)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (docs seulement)   TEST_ROUGE_SEPARE: non (sans objet)
COMMITS: 1 (voir ci-dessous), merge_main_fait: sans objet (copie principale, main)
TESTS: aucun ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune creee ; 0 [A REPONDRE] dans Retours\ (Q-accord-fusion deja archivee le 2026-11-22 dans Docs\Archive\Retours_2026-11-22\, report = GDD D888)

## FAIT
Etat de depart : Retours\ contenait 8 fichiers (6 fiches, LISEZ-MOI.md, ESSAIS-A-JOUER.md), deja nettoye la veille (CLEAN-1122c) ; aucun Q-*.md.
Archive (git mv) vers Docs\Archive\Retours_2026-11-23\ :
- E-audio_panel.md : panneau d'ecoute des 96 sons, jamais joue, marque NE PAS JOUER (candidats jamais commites, decision B). Report verifie AVANT deplacement : Pierre/Metal elus par GDD D798 (Atelier de siege = Stone A, Forum = Metal B, Forge = Stone G) puis D826, D832, D834 (Caserne, Champ de tir, Ecurie) ; alertes attaque, volcan, diplomatie, fin d'entrainement validees telles quelles le 2026-11-10 (AudioInProgress.md l. 118) ; les 12 cues muettes le sont par decision de l'utilisateur (AudioInProgress.md l. 72) ; gain x2,5 d'ecretage : D743 (Bois F, Sacre H, 250 % code). Rien n'attendait plus de reponse.
- Copies des versions de LISEZ-MOI.md et ESSAIS-A-JOUER.md d'avant nettoyage : LISEZ-MOI_avant_nettoyage_1123c.md, ESSAIS-A-JOUER_avant_nettoyage_1123c.md (meme dossier).
Mis a jour : Retours\LISEZ-MOI.md (compte : 7 fichiers = 5 fiches + 2 index ; 1 [A ESSAYER] en titre = talents_heal LOGIQUE SEULEMENT ; 0 [A REPONDRE] ; liste des archives jusqu'a 2026-11-23), Retours\ESSAIS-A-JOUER.md (meme compte, ligne E-audio_panel retiree, renvoi vers l'archive), EtapeDefini.md (1 ligne CLEAN-1123c).

## RESTE dans Retours\ (5 fiches, tous gardes)
- E-talents_heal.md : [A ESSAYER] mais LOGIQUE SEULEMENT (GESTE interne, SON non entendu), recette 1121sd PASS ; attend l'utilisateur seulement s'il accepte un essai de logique.
- E-deploy.md : [PAS PRET, BUG EN FILE D877] ; la ligne [ ] correspondante est toujours dans Auto_Journey_File.md (l. 76, 77, 90) ; la recette 1122c du 2026-11-22 a ete PASS VU A L'ECRAN (geste interne) mais la fiche porte encore PAS PRET : decision de statut laissee a la session principale.
- E-playtest_hero_army.md, BUG-playtest_hero_army-spin.md, BUG-melee-spin-sanity.md : D641 / spins de seconde ligne, aucun correctif dans main (GDD D815 : « spins restants = boucles de seconde ligne de D641, inchangees » ; NeutralsInProgress.md l. 21). Gardes.

## DOUTES (laisses en place, a trancher par la session principale)
1. E-playtest_hero_army.md : l'entree EtapeDefini.md l. 789 dit « scenario modifie, non commite » (Celeborn-28, 2026-11-10 : combat A deplace a l'ouest du Forum, stall #18 = cause scenario). Je n'ai pas verifie si ce scenario modifie est dans main (hors perimetre : Tools\Scenarios) ; la fiche decrit l'ancien cadrage. Candidate a archivage si le scenario est devenu vert ou si D738 (PT7) ne le joue plus.
2. BUG-melee-spin-sanity.md : dernier jalon documente = 2026-11-14 (Auto_Journey_File.md l. 71, residu #2 non reproduit). Fiche ouverte « decision de priorite » ; pas de report equivalent a une fermeture, donc gardee.
3. E-deploy.md : statut PAS PRET peut-etre perime apres RECETTE-1122c (voir plus haut).
4. AudioInProgress.md l. 116 (« SUITE : l'utilisateur ecoute audio_panel... ») est maintenant perimee ; hors perimetre (fichier de chantier), a annoter par la session principale.

## NON VERIFIE
- Aucun fichier de code ni scenario lu ; l'etat de main pour le scenario playtest_hero_army n'est pas controle.
- Si l'utilisateur voulait ecouter le panneau pour verifier l'ecretage a 250 % : D743 dit « 250 % code, a ecouter » et AudioBuildingsInProgress.md l. 89-93 dit que l'ecretage est mesure sur fichiers, jamais entendu ; ce point n'a pas de fiche propre et est considere clos par les elections ulterieures (D798, D826, D832, D834).

## ANOMALIES
- Aucune. Auto_Journey_Retours.md non touche. Le LISEZ-MOI ne listait plus de Q-*.md (deja 0 [A REPONDRE] depuis le 2026-11-22 soir).
PROCESSUS_DETACHES: aucun (aucun processus lance)
ANOMALIES_PROCESSUS: aucune
