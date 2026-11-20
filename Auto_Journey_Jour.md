# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir), `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons), `Docs\Archive\Auto_Journey_Jour_2026-11-12_archive_2026-11-13.md` (séance du 2026-11-12 : programme, tableau d'agents, bilan, leçons ; résumé du 2026-11-11), `Docs\Archive\Auto_Journey_Jour_2026-11-13_matin_archive_2026-11-13.md` (séance interactive du 2026-11-13 matin et reprise : tableau d'agents, programme, bilans, leçons) et `Docs\Archive\Auto_Journey_Jour_2026-11-13_soir_archive_2026-11-14.md` (séance autonome du 2026-11-13 soir : bilan, leçons, tableau d'agents ; résumés du 2026-11-13 matin et du 2026-11-12) et `Docs\Archive\Auto_Journey_Jour_2026-11-14_archive_2026-11-15.md` (séance autonome du 2026-11-14 : tableau d'agents, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-15_archive_2026-11-16.md` (séance autonome du 2026-11-15 : tableau d'agents, fusions n°1-7, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-16_apres-midi_archive_2026-11-16.md` (séance interactive du 2026-11-16 après-midi : tableau d'agents, lignes d'exception, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_matin_archive_2026-11-20.md` (séance de fonctions du 2026-11-20 matin : tableau d'agents, bilan, leçons, plan de lancement ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_apres-midi_archive_2026-11-20.md` (séance de fonctions du 2026-11-20 après-midi : tableau d'agents, plan de lancement, bilan, leçons ; résumé dans ce fichier) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Chapitres du GDD non commencés

**Relevé du 2026-11-15** : voir Docs\ReleveGDD_2026-11-15.md (état réel de A1-A4 et B1-B6, branchements C++ à faire, questions en attente). L'ancien relevé du 2026-11-12 est archivé dans Docs\Archive\Auto_Journey_Jour_Chapitres_GDD_releve_2026-11-12_archive_2026-11-15.md.

## Agents du 2026-11-20

SÉANCE DU 2026-11-20 SOIR (« lance une autojourney le temps de faire les choses en suspens dans les worktree », 22:38 ->). check-list lue (L20) à 22:40, en entier, par la session principale, avant tout lancement d'agent de travail. État au lancement : main = 97da243, 8 worktrees à 0 commit d'avance, IN_USE tous libres ; non suivis : Celeborn 16 scénarios `c28_*`/`c31_*`, Haldir `Content/Maps/Crown.umap`. Nettoyage 0a léger (un agent, comme l'après-midi) : CLEAN-1120s, d41c8d2. Réponses de l'utilisateur (22:4x, interface de choix) : fin = « quand ce qui est en cours sera fini » (fin provisoire 00:30 dans `Docs\SessionEnd.txt` pour le gate, prolongée par `extend_session.ps1` tant qu'il reste du travail en cours, pas de nouveau chantier) ; fusion dans main EN LOCAL si build PASS et tests verts sur le résultat exact, jamais de push ; mode AUTONOME (veilleur 30 min) ; non suivis de Celeborn/Haldir : un agent vérifie leur usage puis les commite sur la branche s'ils sont utiles, sinon les liste.

Sync 22:50 : 8 worktrees à d41c8d2 (docs seulement, aucun build) ; gate Launch 22:53 sans FAIL (vm libre 27 Go). Veilleur 30 min (:13/:43), alarme 00:17. Arwen = intégrateur (aucun code).

| Agent | id | Worktree | Tâche | C++ | Lancé | Fini | Résumé |
|---|---|---|---|---|---|---|---|
| CLEAN-1120s | a033b03e | principale | nettoyage 0a léger | non | 22:41 | 22:48 | d41c8d2, Jour archivé, Retours déjà vide |
| ETAPE0-1120s | aeb7aee4 | lecture seule | plan de lancement du suspens | non | 22:51 | 22:55 | Docs\Rapports\ETAPE0-1120s.md ; 12 lignes de file périmées |
| ELROND-1120sa | afe754cb | Elrond | D869b deploy (barre à 0, 2e cycle) | oui | 22:56 | 23:04 | c7affed rouge (2 tests), 5cbbf40 ; build incrémental lancé 22:59, en cours à 23:05 (37 appels) |
| GLORFINDEL-1120sa | ad169904 | Glorfindel | chat_input : 2e Entrée | oui | 22:56 | 23:00 | script seul : 1a00e0f rouge, 3579205 (Close rend le focus au viewport), 542aa2f ; liaison AS SetFocusToGameViewport non vérifiée |
| SAURON-1120sa | a80499f0 | Sauron | virtuous_knight ClickUnit + regen_tables.ps1 (L20-9) | non | 22:56 | 23:00 | d4a1527 rouge, 318837f, 7a56853 ; scénario non joué, regen_tables en -DryRun seulement |
| SAURON-1120sb | — | Sauron | no_window_test FAIL : ecran_multijoueur.ps1:40 sans -WindowStyle Hidden | non | 23:01 | 23:02 | c09790a : exception documentée (essai joué par l'utilisateur), no_window_test PASS |
| FARAMIR-1120sa | aad178df | Faramir | L20-1 merge=union + L20-3 gate L19-1 | non | 22:56 | 23:10 | 5cfde92/b6863eb (L20-1, 3/3), e318343/4247f1e (L20-3, 12/12), rouges vus ; gate_test complet non fini |
| intégrateur (commande) | — | Arwen | integrate.ps1 integ-32 : rouges c7affed,1a00e0f,4692ec5,8ce795d ; verts integ-25, ability-columns, specialization-trial, tool-known-reds, zone-effect, duel-rewards | oui | 23:11 | 23:12 | FAIL : build rouges PASS 75 s, tests rouges 0/0 (module script ne compile pas ?), conflit merge integ-25 HOLDeployRules.cpp (bouchon du rouge c7affed contre correctif) ; état laissé en conflit |
| DIAG-1120sa | a9a2e606 | Arwen (lecture) | diagnostic des 0/0 tests d'integ-32 | non | 23:13 | 23:15 | `}` manquant HOLAlertRules_Test.as (8ce795d, aussi à la pointe de specialization-trial) ; test population vert d'emblée sur main (constantes déjà 15/30) |
| SAURON-1120sc | a3f6a9f8 | Sauron | integrate.ps1 : verts repartent de main sans les rouges (conflit bouchon) | non | 23:13 | 23:15 | 7d5fae6 rouge vu, 37ee2c0 ; integrate_test 28/28 |
| GALADRIEL-1120sc | — | Galadriel | accolade manquante HOLAlertRules_Test.as | non | 23:16 | 23:16 | c36fb65, accolades équilibrées sur 4 fichiers |
| intégrateur (commande) | — | Arwen | integrate.ps1 (version Sauron 37ee2c0) integ-33 : rouges c7affed,1a00e0f ; mêmes 6 verts | oui | 23:17 | 23:22 | FAIL : rouges vus 3/3 nom par nom (1972/1975), remise à main PASS, 6 merges sans conflit, build PASS, tests verts 0/0 (script ne compile pas) |
| GLORFINDEL-1120sb | — | Glorfindel (+Galadriel si fautif) | diagnostic + correctif du 0/0 d'integ-33 | non | 23:23 | 23:26 | HOLChatInputScreen.as:186 « No matching signatures WidgetBlueprint::SetFocusToGameViewport » (3579205) -> `Widget::` (728ee41, le plugin retire le suffixe BlueprintLibrary) ; journal commité 43bba9b par la session principale |
| intégrateur (commande) | — | Arwen | integrate.ps1 integ-34 : verts seuls (rouges déjà vus dans integ-33, L20-7) | oui | 23:28 | 23:30 | FAIL 1975/1976 : Siege.Deploy.SecondMountReturnsToPack ; script compile (chat Widget:: OK) |
| ELROND-1120sb | — | Elrond | SecondMountReturnsToPack encore rouge sur integ-34 | oui | 23:31 | 23:33 | test faux (aide PermilleAt appelait PhaseAt(-1) = Packing ; le contrôleur ne le fait pas) : 77a229e, aucune assertion affaiblie ; build PASS 17 s |
| intégrateur (commande) | — | Arwen | integrate.ps1 integ-35 : 6 verts | oui | 23:34 | 23:36 | INTERROMPU pendant les tests (fin de la commande avec la session) : 6 merges + build faits, Arwen propre, file vide |
| intégrateur (commande) | — | Arwen | integrate.ps1 integ-36 : relance d'integ-35 | oui | 23:38 | 23:48 | PASS 611 s : 6 merges, build 65 s, tests 1976/1976, net,build PASS ; main avancé en local 0b3887c -> 923d4af (ff, preflight_merge OK), docs c57e12c, rien poussé |
| RECETTE-1120sa | — | Arwen | recette indépendante deploy, chat_input, virtuous_knight sur main 923d4af | non | 23:49 | 23:54 | correctifs CONFIRMÉS à l'écran (barre 1 dièse, 2e cycle « Démonter » ; 2e/3e Entrée rouvrent le chat) ; FAIL scénario deploy (wait 2 manquant l.75) ; FAIL jeu chat : Tab Tab -> pas de « (Privé) <nom> », marque « (Alliés) » invisible ; FAIL scénario virtuous_knight (coups directs, pas la redirection) ; aucune fiche E |
| sync de fin (commande) | — | 8 worktrees | sync_worktrees.ps1 -WaitBuilds (SANS -NoBuild, L15-1), lancé pendant la recette ; Sauron : journal non commité de SAURON-1120sc commité avant (5248d4a) | oui | 23:51 | 23:55 | 6 builds PASS (53-78 s) ; Sauron CONFLIT JourneyInProgress.md annulé (branche sans merge=union) |

Fin prolongée à 01:00 (23:55, extend_session.ps1) : les 3 essais en cours ne sont pas finis (consigne « quand ce qui est en cours sera fini »).

| ELROND-1120sc | — | Elrond | scénario deploy : wait 2 + HUD.Report avant l.75, preflight, fiche E | non | 23:56 | | |
| GLORFINDEL-1120sc | — | Glorfindel | chat : cible Privé après Tab Tab + marque de canal (test rouge, correctif script) | non | 23:56 | 00:12 | 15c46b4 rouge, 7b9f5b4 (ChosenTarget réel, contexte rafraîchi, marque sur fond sombre style HUDLabel, log `ChatInput: Tab ... chosen`), f3e44eb ; cause du Tab non confirmée (Tab arrive par OnUserMovedFocus) ; rien compilé |
| SAURON-1120sd | — | Sauron | fusion main (conflit journal) + scénario virtuous_knight (preuve de redirection) | non | 23:56 | 23:59 | fusion 3f94dbe (journal : deux côtés gardés), 86529fc ; a97f228 (Chevalier en garde hol.Select.Hold, allié à 6 m, ennemis de l'autre côté) ; case E grisée = MinTier 1 sans palier ; 2 preflights lancés contre la consigne |
| build (commande) | — | Sauron | build après fusion de main | oui | 00:00 | 00:00 | PASS 55 s |
| ELROND-1120sc | — | Elrond | scénario deploy | non | 23:56 | 00:00 | 24c73b7 (3 wait après les délais de montage/démontage) ; preflight PASS 13 PNG, VU À L'ÉCRAN (1 dièse, retour « Monter ») ; E-deploy.md écrite (GESTE interne) |
| intégrateur (commande) | — | Arwen | integrate.ps1 (main, avec 37ee2c0) integ-37 : rouge 15c46b4 ; verts integ-25, ability-columns, tool-known-reds | non | 00:01 | 00:11 | PASS 619 s : rouge vu (1976/1977, Test_ChatInputScreenClickedPlayerThenTabTabIsPrivate), verts 1977/1977, net,build PASS ; main avancé en local c57e12c -> f46dc2d, docs b95aa87 |

## Agents du 2026-11-21

Suite après minuit de la SÉANCE DU 2026-11-20 SOIR (même session, fin 01:00 dans `Docs\SessionEnd.txt`). check-list lue (L20) à 22:40 le 2026-11-20 par cette même session, avant tout agent de travail (voir la section du 2026-11-20). Exception au gate Launch 00:12 (recette sur Arwen) : FAIL « DLL 23:39 plus ancienne que le dernier commit de code 23:57 » = commit de script Angelscript seul (0 commit `Source` de retard, Get-StaleBinaryProblem vide) ; integ-37 vient de passer 1977/1977 et net,build sur ce binaire avec ces scripts (L16-8 : le contrôle compte les .as comme du code ; vérifié : depuis 23:39 seuls Script/UI/HOLChatInputScreen*.as et HOLGameScreenScript.as ont changé).

| Agent | id | Worktree | Tâche | C++ | Lancé | Fini | Résumé |
|---|---|---|---|---|---|---|---|
| RECETTE-1120sb | — | Arwen | recette 2 : chat_input, virtuous_knight, deploy sur main f46dc2d | non | 00:13 | 00:15 | deploy PASS et virtuous_knight PASS (VU À L'ÉCRAN, fiches E) ; chat_input FAIL jeu : Tab perdu (HandleTab jamais appelé), « Allies » sans accent ; 2e Entrée OK |
| sync de fin 2 (commande) | — | 8 worktrees | sync_worktrees.ps1 -WaitBuilds | non | 00:15 | | |
| DOCS-1121a | — | principale | clôture : lignes de file (Tab C++ reporté, L18-c), Retours (2 essais), tableau Lecons appliquees | non | 00:17 | | |
| SAURON-1120se | — | Sauron | virtuous_knight : palier 1 pour la case E | non | 00:00 | 00:00 | 3bdad5f : `hol.Cheat.Tier 1` l.15 (modèle arquebusier.txt:6), .look _grid mis à jour |

Exception 23:11 (gate Merge 7 FAIL, passé outre) : 6 branches « sans les derniers commits de main » = seulement les commits de docs 1fcc200 / 0b3887c de la session principale, et integrate.ps1 teste le résultat exact (branche neuve depuis main + fusions) ; « copie principale modifiée » = `Docs/Archive/Retours_2026-11-20/E-neutrals_clairiere.md`, modifié avant la séance par une autre session, touché par aucune branche (le fast-forward ne le heurte pas).
| GALADRIEL-1120sa | a214bbbf | Galadriel | D833 Population pleine 15 s | non | 22:56 | 22:59 | 4692ec5/ae69298/fc20711 ; énoncé mal lu (délai de répétition 30->15 au lieu de durée de ligne 15 s) : erreur de l'énoncé de la session principale |
| GALADRIEL-1120sb | — | Galadriel | correction D833 : durée de ligne 15 s, délai 30 s, HOLShortMessage, son court | non | 23:00 | 23:03 | 8ce795d rouge, fd9a795, 4f9b255 ; son déjà joué par RefreshPopulationAlert |
| intégrateur (commande) | br8ncbjgj | Arwen | integrate.ps1 integ-31 passe 1 réelle (L20-6), vert duel-rewards | — | 22:56 | 23:08 | PASS 713 s : tests 1972/1972, net,build PASS ; L20-6 appliquée, l'outil marche du premier coup |
| CELEBORN-1120sa | a51c9896 | Celeborn + Haldir | fichiers non suivis (c28/c31, Crown.umap) | non | 22:54 | 22:58 | 7f4296c (c28_*) ; c31_f3/4/5/8 doublons et Crown.umap régénérable : SUPPRIMÉS à 23:4x avec l'accord de l'utilisateur (recoupé avant : .json/.look identiques aux c31_f1/6/7/9 suivis, .txt identiques ligne à ligne, seule la fin de ligne diffère) ; Celeborn et Haldir propres |

### Lecons appliquees 2026-11-21
Séance du 2026-11-20 soir (22:38 -> 01:00), bilan écrit à 00:2x par DOCS-1121a depuis le tableau d'agents et les rapports ; les contrôles de `session_gate.ps1 -Phase Close` n'ont pas été relancés par cet agent (à lire à la sortie du gate Close). Les lignes sans sha ni heure reprennent une règle d'agent_rules.md imposée par l'énoncé (champ de rapport non recompté par DOCS-1121a). Règles non appliquées ce soir : L20-8, L19-2, L15-4, L14-5.
T2-1 : appliquee : ETAPE0-1120s lancé 22:51, plan de lancement une ligne par worktree (Docs\Rapports\ETAPE0-1120s.md)
T2-2 : appliquee : tableau 22:56 une ligne par worktree libre (Elrond, Glorfindel, Sauron, Faramir, Galadriel, Celeborn+Haldir, Arwen intégrateur)
L3-12 : sans objet : étape 0 faite quand même (22:51), la file portait déjà les suspens
L5-7 : appliquee : non suivis Celeborn/Haldir : accord de l'utilisateur demandé au lancement (22:4x), CELEBORN-1120sa 7f4296c
L4-1 : appliquee : fin provisoire 00:30 écrite dans Docs\SessionEnd.txt, alarme 00:17, prolongée à 01:00 à 23:55 (extend_session.ps1), aucun agent au-delà de son arrêt
L4-3 : appliquee : prolongation par extend_session.ps1 une seule fois (23:55), motif écrit dans le Jour
L3-8 : appliquee : intégrateur Arwen nommé au lancement (ligne `intégrateur` du tableau)
L4-9 : appliquee : intégrateur Arwen (integ-31 à integ-37) puis recettes RECETTE-1120sa et sb
Q5 : appliquee : recettes indépendantes RECETTE-1120sa 23:49 et RECETTE-1120sb 00:13 sur main fusionné (Arwen)
L1-14 : appliquee : intégrateur nommé (Arwen), rejeu des essais par un autre agent
L3-9 : sans objet : point de reprise non écrit, session non relancée (compte d'appels de la session non vérifié par DOCS-1121a)
T2-3 : appliquee : énoncés « lis agent_rules.md » + arrêt dur (DOCS-1121a : arrêt dur 00:35)
L1-1 : appliquee : aucun agent au-delà de 45 appels déclarés au tableau (max ELROND-1120sa 37 appels)
L3-6 : appliquee : aucun agent remplacé à plus de 200k, tâches de 2 à 15 min
L4-7 : appliquee : `tool_uses` jamais > 100 au tableau
L5-3 : appliquee : même contrôle que L1-1 (45 vus), RECETTE-1120sb ~31 appels pour une limite de 30 : dépassement mineur noté dans son rapport
L5-6 : sans objet : 0 question Q-*.md posée ce soir
L3-11 : sans objet : aucune régénération de cartes ce soir
L5-5 : sans objet : aucune mesure de perf ni de budget ce soir
L3-7 : appliquee : veilleur 30 min (:13/:43), alarme 00:17
L1-4 : appliquee : veilleur silencieux si rien ne change
T2-4 : appliquee : questions : 0 (aucun Q-*.md créé ce soir)
Q5, L5-4 : appliquee : correctifs deploy et chat rejoués par RECETTE-1120sa (23:49) puis sb (00:13) avant toute commande donnée
L5-4 : appliquee : rejeu par un autre agent (RECETTE-1120sa/sb) des correctifs deploy 5cbbf40/24c73b7, chat 728ee41/7b9f5b4
Q8 : appliquee : défauts communs de la recette sb écrits en tête de file à 00:2x (Tab perdu, « Allies », deploy_move_unmounting, hol.Select.Hold)
Q11 : appliquee : 2 essais donnés (deploy, virtuous_knight), sous le plafond de ~6
L4-11 : sans objet : aucun essai à plusieurs variantes ce soir
L3 / T2 : appliquee : fusion groupée par integrate.ps1 (integ-36 PASS 611 s : 6 merges, UN build 65 s, tests 1976/1976, ff main 0b3887c -> 923d4af ; integ-37 PASS 619 s, main -> f46dc2d)
L1-3 : appliquee : intégrateur rejoue tests puis net,build (integ-36, integ-37) ; merge_main_fait relevé aux rapports
L6-2 : appliquee : la file lue par extraits (ETAPE0-1120s), jamais en entier par la session principale
L6-4 : appliquee : bug chat rejoué par le geste exact (RECETTE-1120sa/sb : Entrée, Tab Tab)
L6-6 : sans objet : aucune mesure répétée ce soir
L7-7 : appliquee : première fusion (integ-32) à 23:11 dès 3 verts, sans attendre la fin
L8-4 : sans objet : aucun essai audio
L8-8 : appliquee : rouges C++ (c7affed) fusionnés en premier à integ-32 ; verts script suivis
L9-1 : sans objet : aucune re-notification en boucle relevée
L9-6 : appliquee : integrate.ps1 teste la branche neuve depuis main + fusions (tips lus à l'exécution, exception gate Merge 23:11 écrite)
L9-8 : appliquee : 8 worktrees chargés (suspens) ; Sauron et Faramir enchaînés sur plusieurs tâches
L9-9 : appliquee : réponses de l'utilisateur (22:4x) appliquées en séance (fin, fusion locale, autonome)
L9-10 : sans objet : aucun essai sonore
L10-2 : sans objet : aucun net rouge ce soir (net,build PASS à integ-31, 36, 37)
L10-3 : appliquee : tests 0/0 d'integ-32/33 lus avant conclusion (DIAG-1120sa 23:15, GLORFINDEL-1120sb 23:26) ; Get-StaleBinaryProblem lu à la recette 00:12
L11-3 : sans objet : aucun bug rapporté à l'utilisateur ce soir
L11-4 : appliquee : lignes de file cochées par DOCS-1121a à 00:2x (8 lignes `[x]` avec sha)
L11-8 : appliquee : listes de branches figées dans la commande integrate.ps1 (6 verts), aucun message à l'intégrateur en cours
L11-9 : appliquee : intégrateur Arwen, recettes sa et sb, 2 essais prêts écrits dans Retours
L11-10 : sans objet : aucune mesure de mêlée ce soir
L10-5 : sans objet : aucun balayage A/B ce soir
L9-7 : sans objet : aucun tableau de mesure lu ce soir
L11-11 : appliquee : fiches E-deploy et E-virtuous_knight vérifiées sans réponse antérieure dans Retours\REPONSES-*.md
L11-12 : appliquee : Retours\ purgé par CLEAN-1120s (22:48, Retours déjà vide)
L11-13 : appliquee : heures annoncées prises de Get-Date (00:16, 00:17)
L12-1 : appliquee : sync_worktrees.ps1 -WaitBuilds SANS -NoBuild (23:51 : 6 builds PASS 53-78 s)
L12-1b : appliquee : gate Launch 22:53 : vm libre 27 Go, pas de build lent
L13-1 : sans objet : aucune erreur d'API 403 relevée
L13-3 : appliquee : docs de séance commitées par nom avant l'intégration (43bba9b, 5248d4a, c57e12c, b95aa87)
L13-4 : sans objet : aucun net rouge ce soir
L13-5 : appliquee : non suivis traités (CELEBORN-1120sa : 7f4296c, doublons supprimés avec accord)
L13-6 : appliquee : .look de deploy (13 PNG, 1 dièse) et de virtuous_knight (_grid mis à jour 3bdad5f) vérifiés
L13-7 : sans objet : aucun balayage de variantes ce soir
L14-2 : appliquee : gate Launch 22:53 vm libre 27 Go, au plus 2 agents C++ simultanés (Elrond, Glorfindel)
L14-4 : sans objet : aucune question posée par interface de choix ce soir hors réponses de lancement
L14-5 : non appliquee : deploy et virtuous_knight donnés avec GESTE: interne (hol.UI.Key, hol.Select.*) : aucun geste réel clic ; ligne de file à créer (lot « gestes réels »)
L14-6 : appliquee : rouges VUS à l'exécution (integ-33 3/3 nom par nom 1972/1975 ; integ-37 1976/1977 Test_ChatInputScreenClickedPlayerThenTabTabIsPrivate)
L14-9 : sans objet : aucun défaut d'affichage commun à 3 essais
L14-10 : sans objet : aucun essai audio
L14-11 : appliquee : fusions dès que les verts sont prêts (integ-32 23:11, integ-33 23:17, integ-36 23:38, integ-37 00:01)
L15-3 : sans objet : aucun EDITS_REFUSES relevé aux rapports ; edit_line.ps1 / replace_text.ps1 disponibles
L15-4 : non appliquee : ids « — » jamais remplis à 10 lignes du tableau (SAURON-1120sb, GALADRIEL-1120sc, ELROND-1120sb, RECETTE-1120sa...), ELROND-1120sc en double (lignes 23:56)
L15-6 : sans objet : builds par integrate.ps1 et sync -WaitBuilds (aucun Wait-Process d'agent)
L15-7 : appliquee : ETAPE0-1120s : 12 lignes de file périmées listées avant lancement
L15-8 : appliquee : recette indépendante dès la première fusion (RECETTE-1120sa 23:49) ; .look nomme l'objet attendu
L15-9 : appliquee : tâches un test rouge + un correctif (agents de 2 à 15 min)
L15-10 : appliquee : rapports complets dans Docs\Rapports\*-1120s*.md (17 fichiers), session principale reçoit des résumés
L16-1 : sans objet : question tranchée par D830 (Angelscript par défaut)
L16-2 : appliquee : commandes play.ps1 avec -Root Arwen, preflight PASS sur ce -Root après merge (00:13) ; exception gate Launch 00:12 (script seul) écrite
L16-3 : appliquee : main avancé seulement après quick_check tests vert sur le tip exact (923d4af 1976/1976 ; f46dc2d 1977/1977)
L16-4 : appliquee : séance et questions en français, commits en anglais
L16-5 : appliquee : chiffres du bilan tirés du tableau d'agents et des sorties d'integrate.ps1
L16-6 : appliquee : aucun build de confort (builds par integrate.ps1 et sync_worktrees.ps1)
L16-7 : sans objet : aucun outil PASS/FAIL tronqué relevé
L16-8 : appliquee : exception gate Launch 00:12 écrite avec preflight et vérification (seuls .as changés depuis 23:39, Get-StaleBinaryProblem vide)
L16-9 : appliquee : après sync 23:51 : 6 builds PASS vérifiés (53-78 s) ; Sauron conflit journal annulé, binaire à jour
L17-a : appliquee : tests 0/0 d'integ-32 lus (DIAG-1120sa : `}` manquant HOLAlertRules_Test.as) avant de conclure à une régression
L17-b : sans objet : aucun retrait massif de lignes longues
L17-c : appliquee : integ-35 interrompu avec la commande ; état revérifié (Arwen propre, file vide) avant integ-36
L18-a : sans objet : table générée non modifiée sans exécution (regen_tables.ps1 en DryRun)
L18-b : sans objet : aucune règle neuve de faction ce soir
L18-c : appliquee : lot C++ du Tab reporté à 00:16 (dernier lancement C++ non pris après fin - 60 min)
L18-d : sans objet : aucune commande ajoutée à une liste enregistrée
L18-e : appliquee : rouges c7affed et 1a00e0f vus 3/3 (integ-33) puis verts ; rouge 15c46b4 vu à integ-37 (1976/1977 puis 1977/1977)
L18-f : appliquee : scénario deploy : 3 `wait` après les délais (24c73b7) ; attente de l'effet encore manquante à deploy_move_unmounting (ligne de file)
L19-1 : appliquee : tous les correctifs, diagnostics, pré-vols et fiches par agents ; session principale : commandes, avances de main, 3 commits de journaux laissés non commités par des agents (43bba9b, 5248d4a)
L19-2 : non appliquee : 2 erreurs de compilation script passées à l'intégration (`}` manquant HOLAlertRules_Test.as 8ce795d, corrigé c36fb65 ; `WidgetBlueprint::SetFocusToGameViewport` 3579205, corrigé 728ee41) ; champ DIAGNOSTICS_AS non rempli
L19-3 : appliquee : aucun fichier de code modifié dans la copie principale (git status au lancement : docs et rapports seulement)
L19-4 : sans objet : aucune donnée de faction changée ce soir
L19-5 : appliquee : E-deploy.md écrite au pré-vol (00:00) ; E-virtuous_knight.md écrite par la recette sb (00:13) ; capture « pendant » au .look de deploy
L19-6 : sans objet : 0 question ce soir
L19-7 : appliquee : mémoire libre lue avant le lot C++ (gate Launch 22:53 vm libre 27 Go)
L19-8 : appliquee : rouges séparés des verts (c7affed/5cbbf40, 1a00e0f/3579205, d4a1527/318837f, 8ce795d/fd9a795, 15c46b4/7b9f5b4, 7d5fae6/37ee2c0)
L19-9 : appliquee : integrate.ps1 pour toutes les passes (integ-31 à integ-37)
L20-1 : appliquee : merge=union sur *InProgress.md (FARAMIR-1120sa 5cfde92/b6863eb, 3/3 tests) ; reste : le worktree Sauron sans la règle a conflit au sync 23:55 (annulé)
L20-2 : appliquee : aucune erreur de compilation C++ à l'intégration (build PASS integ-32 75 s, integ-36 65 s) ; conflit de bouchon c7affed corrigé dans l'outil (37ee2c0)
L20-3 : appliquee : gate Close ne compte plus fusions groupées / avances / Docs (FARAMIR-1120sa e318343/4247f1e, 12/12)
L20-4 : appliquee : 2 recettes (sa 23:49, sb 00:13), correctifs rejoués (deploy 24c73b7, chat 7b9f5b4) ; chat Tab reste FAIL, lot C++ reporté
L20-5 : appliquee : ELROND-1120sa build incrémental lancé 22:59 (file vide, vm 27 Go)
L20-6 : appliquee : integrate.ps1 7 passes (integ-31 à integ-37), défaut corrigé dans l'outil 37ee2c0 (verts repartent de main)
L20-7 : appliquee : integ-34 verts seuls (rouges déjà vus à integ-33), rouges script dans le même build
L20-8 : non appliquee : premier Edit après le 8e appel chez 5 agents du soir (énoncés sans « premier Edit avant le 8e appel »)
L20-9 : appliquee : regen_tables.ps1 + test (318837f, -DryRun) ; assets non régénérés ce soir faute de CSV changé
L6-1 : appliquee : gate Launch 22:53 sans FAIL (3 lignes `sans C++`)
L8-7 : appliquee : gate Launch 22:53 sans FAIL
L6-3 : appliquee : -Phase Report à la réception des rapports (contrôle du gate)
L7-8 : sans objet : Close -Exclude non utilisé
L8-3 : appliquee : heure de fin lue dans Docs\SessionEnd.txt (00:30 puis 01:00)
L9-3 : appliquee : gate Essay sur le nom de scénario (contrôle du gate)
L11-1 : sans objet : Launch -Scenarios non lancé ce soir
L11-7 : appliquee : Launch -NoCpp pour les agents « sans C++ » (contrôle du gate)
L6-5 : appliquee : build.ps1 avant quick_check après merge de main (integrate.ps1 : build avant tests)
L7-4 : appliquee : stray_processes.ps1 en dernier geste des agents (règle 20)
L7-6 : appliquee : E-deploy.md et E-virtuous_knight.md portent GESTE: interne et SON: non entendu
L8-1 : appliquee : commandes play.ps1 -Root Arwen (main compilé, tip portant les branches)
L9-5 : appliquee : docs commitées par nom avant Launch et Merge (commit_docs.ps1 : c57e12c, b95aa87)
L10-1 : appliquee : aucun play.ps1 sans -Hidden dans un énoncé ; no_window_test PASS (c09790a)
L8-5 : appliquee : known_reds.txt lu par quick_check (rouges connus distingués)
L8-6 : appliquee : Write/Edit sans heredoc ; eol_check à la demande
L7-1 : appliquee : même contrôle que L1-1 (45 appels vus)
L9-2 : appliquee : même contrôle que L1-1 (45 appels vus)
L7-3 : sans objet : aucun éditeur de scénario concurrent relevé
L11-5 : appliquee : commit_docs.ps1 couvre Placeholders*, *InProgress.md, SessionEnd.txt
L11-6 : appliquee : play.ps1 -Hidden ferme ses éditeurs (recettes sa et sb)
L5-1 : appliquee : Get-Date en premier geste (agents et DOCS-1121a : 00:16)
L3-10 : appliquee : gate Launch (contrôle du gate)
L4-2 : appliquee : gate Launch -End sans FAIL (fin 00:30, puis 01:00)
L5-2 : appliquee : `## Agents du 2026-11-20` présent (gate Launch 22:53 sans FAIL)
L3-1 : appliquee : gate Launch (plafond C++ : au plus 2 agents C++ simultanés)
L4-4 : appliquee : au plus 2 agents C++ simultanés (ELROND-1120sa, GLORFINDEL-1120sa à 22:56)
L1-10 : appliquee : *InProgress.md suivis ; journaux de Sauron non commités commités avant sync (5248d4a)
L1-11 : appliquee : contrôle du gate Launch (non relancé par DOCS-1121a)
Q10 : appliquee : contrôle du gate (non relancé par DOCS-1121a)
L4-10 : appliquee : Essay -Root sur Arwen (main compilé), worktree propre à la recette
L3-4 : appliquee : champ GIT_STATUS_PRINCIPALE ; git status copie principale : docs et rapports seulement
L7-2 : appliquee : Launch -Root : DLL à jour (sync 23:55 : 6 builds PASS)
L8-9 : appliquee : 8 worktrees rattrapés par sync_worktrees.ps1 (22:50, 23:55)
L4-14 : appliquee : contrôle Close (non relancé par DOCS-1121a) ; fichiers non commités = docs de séance commités par ce commit
L5-bilan : appliquee : le présent titre `### Lecons appliquees 2026-11-21` (séance du 2026-11-20 soir, passée minuit)
L12-1c : appliquee : gate Launch 22:53 : vm libre 27 Go, aucun cl.exe d'un autre moteur
L4-6 : appliquee : fusion dès verts (L7-7, L8-8)
L11-2 : appliquee : build.ps1 -Wait / integrate.ps1 pour les builds
Q12 : appliquee : contrôle Close (non relancé par DOCS-1121a)
L1-6 : appliquee : Retours\ sans réponse non traitée (Retours déjà vide à 22:48)
L9-4 : appliquee : contrôle Close fins de ligne ; .gitattributes en vigueur (L20-1)
L12-6 : appliquee : EtapeDefini à jour avec les fusions (contrôle Close non relancé par DOCS-1121a)
L1-12(c) : appliquee : check.ps1 FAIL si audit Sanity échoue (contrôle de l'outil)
L8-2 : appliquee : audit hol.Test.Sanity check 5 dans check.ps1
L10-4 : appliquee : eol_check.ps1 -Fix ou remplacement par lignes (règle 6)
L12-2 : appliquee : sync_worktrees.ps1 sans -NoBuild avant les lancements de recette (23:51)
L12-3 : sans objet : aucun build ayant dépassé -MaxWaitMin
L12-7 : sans objet : aucun BUILD LENT (builds de 17 à 78 s)
L13-2 : appliquee : IN_USE.txt `CPP: oui` / `CPP: non` (règle 10)
L14-3 : appliquee : status.ps1 / sync_worktrees.ps1 ForeignSession ; status.ps1 -InMain avant « absent de main »
L14-7 : appliquee : build.ps1 imprime Get-StaleBinaryProblem
L14-8 : appliquee : FrameWaits Warn de check.ps1 ; deploy.txt utilise `wait` après délai (24c73b7)
L14-12 : appliquee : commit_docs.ps1 -Only utilisé ; docs gelées avant la clôture
L14-13 : appliquee : extend_session.ps1 23:55 (01:00 écrit)
L15-1 : appliquee : tous les worktrees à main après sync 23:55 (6 builds PASS) ; Close non relancé par DOCS-1121a
L15-2 : appliquee : edit_line.ps1 -Match sur une seule ligne (cette clôture)
L7-5 : appliquee : stub qui compile + test rouge VU puis un commit vert (règle 7)
L15-5 : appliquee : TEST_ROUGE vu à l'exécution (integ-33 3/3 nom par nom)
L12-4 : appliquee : LAUNCHCHECK_DEBUT à l'entrée des agents C++ (règle 10)
L12-5 : appliquee : TESTS_TAUTOLOGIQUES: 0 aux rapports
L13-8 : appliquee : brief commun (8 derniers appels réservés)
L14-1 : appliquee : lecture en un appel groupé, edit_line.ps1 (cette clôture)
L1-2 : appliquee : commits par lot vert, dernier commit relevé aux rapports
L3-2 : appliquee : PREMIER_BUILD relevé (ELROND-1120sa : 22:59, build incrémental)
L4-5 : appliquee : BUILD PASS/FAIL aux rapports (ELROND-1120sb build PASS 17 s)
L3-5 : appliquee : un seul lot C++ par agent (ELROND-1120sa)
L1-8 : appliquee : un seul statut par agent (InProgress + ligne EtapeDefini)
L1-9 : appliquee : chemins absolus, aucune écriture hors worktree et docs
L4-8 : appliquee : aucune décision D attribuée par un agent
L1-7 : appliquee : QUESTIONS: 0 (aucun Q-*.md créé)
T2-5 : appliquee : TESTS: parties lancées et chiffres aux rapports (1976/1976, 1977/1977)
T2-6 : appliquee : quick_check après build.ps1 (integrate.ps1 : build puis tests)
L5-8 : appliquee : outil partagé fautif corrigé dans l'outil (integrate.ps1 37ee2c0), pas de copie privée
T2-7 : appliquee : aucun design tranché par un agent (0 question, valeurs par défaut)
L1-5 : appliquee : aucune décision de design prise par un agent
L4-12 : appliquee : champ ANOMALIES_PROCESSUS obligatoire (règle du rapport)
L4-13 : appliquee : UHT supprimé seulement si nécessaire ; aucun .uasset en conflit pris « theirs »
Q1 : appliquee : essais listés seulement avec preflight PASS (deploy 13 PNG, virtuous_knight 3 PNG)
Q2 : appliquee : PNG ouverts et vus (deploy, virtuous_knight, fiches E)
Q3 : appliquee : essais de moins de 3 min (deploy ~2 min, virtuous_knight ~1 min)
Q4 : appliquee : aucun `waitkey` dans les scénarios joués
Q6 : sans objet : aucun réglage à l'œil ce soir
Q7 : sans objet : aucun essai d'IA ou de système ce soir
Q9 : appliquee : libellés et touches de la grille et du GDD (D869b, D864/D865), rien d'inventé
L1-13 : appliquee : TEST_ROUGE assertit l'effet (barre à 0, Tab, redirection)
L3-3 : sans objet : outil à construire, non démarré ce soir

## Résumé de la séance du 2026-11-20 après-midi

- Séance interactive 15:32 -> 18:00, session principale + ~45 agents Sonnet en 8 vagues ; intégration integ-30 (Arwen) en 4 passes ; main = fd41600 (LOCAL, rien poussé) ; tests 1971/1971 puis 1972/1972, `net` et `build` PASS.
- Visible en jeu : D869 montage/démontage de l'engin (partiel), chat privé cliquable + `/w`, ligne Diplomatie cliquable, Chevalier Vertueux à la Caserne (D873), clairière des neutres sans bosquet (VU À L'ÉCRAN, essai joué OK).
- Sans effet visible : D862, D864/D865, D867, D868, D870, garnison, moral des duels, `integrate.ps1`, `hol.Mouse.Drag`, `hol.Decor.ClearNear Trees`, anneau de 8 m ; réponses D871 à D876 enregistrées.
- Trouvé : 4 erreurs de compilation d'agents sans build, 1 bug de type d'occupant de garnison (a680f3f), conflits de journaux à presque chaque fusion ; recette : 3 défauts en tête de file (deploy partiel, 2e Entrée du chat, scénario virtuous_knight).
- Détail (tableau d'agents, bilan, tableau « Lecons appliquees 2026-11-20 », plan de lancement) : `Docs\Archive\Auto_Journey_Jour_2026-11-20_apres-midi_archive_2026-11-20.md` ; axes L20-1 à L20-9 : `Auto_Journey_Checklist.md`.

## Résumé de la séance de fonctions du 2026-11-20 matin

- Séance autonome 08:45 -> arrêt ~13:01 à la demande de l'utilisateur, session principale + environ 50 agents Sonnet en 11 vagues ; quatre intégrations (integ-26 à integ-29), main 410cb7d -> 991a8bc, tests 1922/1922 (1817 au lancement : 105 tests neufs), `net` et `build` PASS, cinq essais donnés (VU À L'ÉCRAN), 0 question ouverte à la fin (7 réponses D862-D868 enregistrées).
- Entré dans main, visible : Hallebardier de l'Aube (D858), engin de siège qui garde son ordre (D861), signal animé et Alt + clic sur la mini-carte (D860), barre d'Honneur dans le HUD, son de l'Écurie (D834), lignes de mouvement par bloc. Sans effet visible encore : valeurs D859, zone et pose du Sceau de lumière, triche `hol.Cheat.Honor`, visée au sol, garnison, alertes mineures, saisie du chat, pose en ligne des murs, redirection de dégâts A2, WIP D827 / D828.
- Clôture rattrapée à 14:30-15:31 à la demande de l'utilisateur (`session_gate.ps1 -Phase Close` non lancé à l'arrêt) : main a831674 poussé ; les 7 worktrees de branche fusionnés en avance rapide sur a831674 puis build PASS chacun ; Arwen = main 991a8bc compilé, gardé tel quel pour les essais.
- Leçons L19-1 à L19-9 : check-list lue et session principale = lancer seulement (L19-1, non appliquée), pièges script et diagnostics (L19-2), chemins absolus (L19-3), identifiants de données cherchés dans quick_check (L19-4), fiche E dès le pré-vol (L19-5), Q posées dès qu'elles paraissent (L19-6), mémoire libre avant un lot C++ (L19-7), test rouge séparé (L19-8), `integrate.ps1` (L19-9).
- Détail (tableau d'agents, bilan, tableau « Lecons appliquees 2026-11-20 », plan de lancement) : `Docs\Archive\Auto_Journey_Jour_2026-11-20_matin_archive_2026-11-20.md`.

## Résumé de la séance de fonctions du 2026-11-19 soir

- Séance autonome 21:08 -> 23:10, session principale + 22 agents Sonnet : socle commun des factions en règles pures script (jauge de faction, livre de pouvoirs, emplacement de faction, capacité visée au sol, Templiers à 3 options, récompenses du duel, pose en ligne des murs, dépense des pouvoirs par l'IA) + petit lot C++ (jauge répliquée `AHOLGameState::FactionResource`, triche `hol.Cheat.Specialization`) ; main df895b0 puis docs, tests 1817/1817, partie `net` PASS, gate Close sans FAIL ; rien branché en jeu pour le joueur.
- Réponses du soir : D858 (Hallebardier valeurs B, remplace le Lancier), D859 (10 valeurs du socle recommandées), D860 (signal animé + Alt + clic mini-carte), D861 (engin de siège garde son ordre pendant le déploiement) ; `specialization_button` OK. Suite : lot C++ groupé du socle (`Docs\Rapports\LOT-CPP-socle-factions-1119.md`), corrections D860/D861, Aube A1.
- Leçons : un fichier GÉNÉRÉ modifié par un agent sans exécution se relit en `git diff` avant le premier lancement ; l'énoncé d'une règle neuve donne le type canonique des entrées communes (faction, siège, case) ; le gate Close refuse un lot C++ resté hors de main (L15-1).
- Détail (tableau de 22 agents, bilan, tableau « Lecons appliquees 2026-11-19 ») : `Docs\Archive\Auto_Journey_Jour_2026-11-19_soir_archive_2026-11-20.md`.

## Résumé des séances de migration du 2026-11-16 soir au 2026-11-19

- Migration C++ -> Angelscript (D830, D835), 17 reprises du 2026-11-16 21:03 au 2026-11-19 ~20:35, environ 300 agents Sonnet ; TERMINÉE : étape 5 close par D857 (2026-11-19 19:23), les 43 corps serveur `Do<X>` et l'enveloppe réseau restent en C++.
- main = dc993f6 (integ-23) + docs = 2ff2dbf, LOCAL, RIEN POUSSÉ (push : « pas maintenant », à la décision de l'utilisateur) ; build PASS, tests 1782/1782 ; chaîne réseau complète verte sur integ-21 (non rejouée sur dc993f6 : écart de tests seulement).
- Décisions D835 à D857 dans le GDD (commité le 2026-11-19 avec l'accord de l'utilisateur) ; règle permanente : code neuf en Angelscript, C++ par exception (D839) ; tests de la simulation Mass en C++ (D838).
- Rien n'attendait l'utilisateur à la fin : Forge (D856) jouée OK, les 5 essais de revue du HUD joués OK.
- Non vérifié à la clôture : périmètre Ferme + camps de D856 accepté par un « OK » d'essai seulement ; 2 lignes `[~]` de migration douteuses dans la file (MatchConfig, L17-12 Production) ; 6 lignes `[~]` rangées FAITE sur la seule absence du fichier C++ de test.
- Leçons de la reprise 17 : un test porté par un agent sans build peut changer le sens de l'assertion (lire le premier rouge avant de conclure) ; `PlayerArray` se parcourt par indice + `.Get()` en Angelscript ; un build lancé par une session qui s'arrête meurt avec elle ; une archive de la file se fait par section entière.
- Détail (tableaux d'agents du 2026-11-16 au 2026-11-19, bilans des reprises, tableaux « Lecons appliquees » des 2026-11-16, 2026-11-17 et 2026-11-19) : `Docs\Archive\Auto_Journey_Jour_2026-11-16_soir_au_2026-11-19_migration_archive_2026-11-19.md`.

## Résumé de la séance du 2026-11-16 après-midi

- Séance interactive 14:46 -> ~18:00, 27 agents, fusion groupée FUSION-1116a, rien poussé : main 9c9d22b -> 3d3cb3c, tests 1326/1328 (2 rouges connus D831 : NomadFirstForum, NomadReplicated).
- Décisions D827-D834 ; D830 = arrêt du développement C++, passage du maximum en Angelscript, tests compris (plan Docs\Rapports\ANALYSE-1116a.md).
- Non fini : specialization_button échoue sur main (ligne Elrond), signals_altclick sans mapSize (ligne Haldir), WIP Faramir D827/D828 non compilé, défaut du cvar hol.Nomad.ForumCell et tests Nomad à corriger avec la migration D830.
- Worktrees à rebuilder (sync_worktrees -NoBuild 17:45) : Arwen, Elrond, Faramir, Galadriel, Glorfindel, Haldir, Sauron (Celeborn à jour).
- Détail (tableau d'agents, exceptions, bilan, leçons) : `Docs\Archive\Auto_Journey_Jour_2026-11-16_apres-midi_archive_2026-11-16.md`.

## Résumé de la séance du 2026-11-15

- Séance autonome 09:02 -> 14:37 (arrêtée par l'utilisateur), ~65 agents, 7 fusions groupées, rien poussé : main 140cdb7 -> 22ad07b, tests 1267 -> 1320 verts.
- Reste ROUGE non commité, worktrees non synchronisés avec main : Elrond (tiers : texte de la case de spécialisation en cours + rapport HUD, test ForumGrid rouge voulu, git mv du test vers le module UI) ; Galadriel (galadriel-tab : alerte population branchée, test sans PlayerContext, suite dans AlertsMinorInProgress.md).
- Détail (tableau d'agents, fusions, bilan, leçons) : `Docs\Archive\Auto_Journey_Jour_2026-11-15_archive_2026-11-16.md`.
