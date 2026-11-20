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
| intégrateur (commande) | — | Arwen | integrate.ps1 (main, avec 37ee2c0) integ-37 : rouge 15c46b4 ; verts integ-25, ability-columns, tool-known-reds | non | 00:01 | | |
| SAURON-1120se | — | Sauron | virtuous_knight : palier 1 pour la case E | non | 00:00 | 00:00 | 3bdad5f : `hol.Cheat.Tier 1` l.15 (modèle arquebusier.txt:6), .look _grid mis à jour |

Exception 23:11 (gate Merge 7 FAIL, passé outre) : 6 branches « sans les derniers commits de main » = seulement les commits de docs 1fcc200 / 0b3887c de la session principale, et integrate.ps1 teste le résultat exact (branche neuve depuis main + fusions) ; « copie principale modifiée » = `Docs/Archive/Retours_2026-11-20/E-neutrals_clairiere.md`, modifié avant la séance par une autre session, touché par aucune branche (le fast-forward ne le heurte pas).
| GALADRIEL-1120sa | a214bbbf | Galadriel | D833 Population pleine 15 s | non | 22:56 | 22:59 | 4692ec5/ae69298/fc20711 ; énoncé mal lu (délai de répétition 30->15 au lieu de durée de ligne 15 s) : erreur de l'énoncé de la session principale |
| GALADRIEL-1120sb | — | Galadriel | correction D833 : durée de ligne 15 s, délai 30 s, HOLShortMessage, son court | non | 23:00 | 23:03 | 8ce795d rouge, fd9a795, 4f9b255 ; son déjà joué par RefreshPopulationAlert |
| intégrateur (commande) | br8ncbjgj | Arwen | integrate.ps1 integ-31 passe 1 réelle (L20-6), vert duel-rewards | — | 22:56 | 23:08 | PASS 713 s : tests 1972/1972, net,build PASS ; L20-6 appliquée, l'outil marche du premier coup |
| CELEBORN-1120sa | a51c9896 | Celeborn + Haldir | fichiers non suivis (c28/c31, Crown.umap) | non | 22:54 | 22:58 | 7f4296c (c28_*) ; c31_f3/4/5/8 doublons et Crown.umap régénérable : SUPPRIMÉS à 23:4x avec l'accord de l'utilisateur (recoupé avant : .json/.look identiques aux c31_f1/6/7/9 suivis, .txt identiques ligne à ligne, seule la fin de ligne diffère) ; Celeborn et Haldir propres |

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
