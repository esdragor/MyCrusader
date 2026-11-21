# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir), `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons), `Docs\Archive\Auto_Journey_Jour_2026-11-12_archive_2026-11-13.md` (séance du 2026-11-12 : programme, tableau d'agents, bilan, leçons ; résumé du 2026-11-11), `Docs\Archive\Auto_Journey_Jour_2026-11-13_matin_archive_2026-11-13.md` (séance interactive du 2026-11-13 matin et reprise : tableau d'agents, programme, bilans, leçons) et `Docs\Archive\Auto_Journey_Jour_2026-11-13_soir_archive_2026-11-14.md` (séance autonome du 2026-11-13 soir : bilan, leçons, tableau d'agents ; résumés du 2026-11-13 matin et du 2026-11-12) et `Docs\Archive\Auto_Journey_Jour_2026-11-14_archive_2026-11-15.md` (séance autonome du 2026-11-14 : tableau d'agents, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-15_archive_2026-11-16.md` (séance autonome du 2026-11-15 : tableau d'agents, fusions n°1-7, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-16_apres-midi_archive_2026-11-16.md` (séance interactive du 2026-11-16 après-midi : tableau d'agents, lignes d'exception, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_matin_archive_2026-11-20.md` (séance de fonctions du 2026-11-20 matin : tableau d'agents, bilan, leçons, plan de lancement ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_apres-midi_archive_2026-11-20.md` (séance de fonctions du 2026-11-20 après-midi : tableau d'agents, plan de lancement, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-20_soir_archive_2026-11-21.md` (séance autonome du 2026-11-20 soir, passée minuit : tableaux d'agents du 2026-11-20 et du 2026-11-21, bilan, leçons, exceptions de gate ; résumé dans ce fichier) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Chapitres du GDD non commencés

**Relevé du 2026-11-15** : voir Docs\ReleveGDD_2026-11-15.md (état réel de A1-A4 et B1-B6, branchements C++ à faire, questions en attente). L'ancien relevé du 2026-11-12 est archivé dans Docs\Archive\Auto_Journey_Jour_Chapitres_GDD_releve_2026-11-12_archive_2026-11-15.md.

## Agents du 2026-11-21

SÉANCE DU 2026-11-21 (« lance une autojourney », 10:40 -> 14:40, autonome). Réponses de l'utilisateur (10:4x, interface de choix) : fin 14:40 (`Docs\SessionEnd.txt`) ; fusion dans main EN LOCAL si build PASS et tests verts sur le résultat exact, jamais de push ; mode AUTONOME (veilleur 30 min à :13/:43, alarme 14:25). main = 61a60e8 au lancement. Nettoyage 0a (4 agents) : 932b820, 4be2ce4, 7a46e54, bce2d69. check-list lue (L20) à 10:43, en entier, par la session principale, après 0a et avant tout lancement d'agent de travail. Intégrateur : Arwen (commandes `integrate.ps1` de la session principale, recette par agent).

| Agent | id | Worktree | Tâche | C++ | Lancé | Fini | Résumé |
|---|---|---|---|---|---|---|---|
| CLEAN-1121a | a8cd8c63 | principale | 0a : Jour + Retours | non | 10:41 | 10:42 | 932b820 |
| CLEAN-1121b | af5f7e8e | principale | 0a : file | non | 10:41 | 10:43 | 7a46e54, 48 [x] archivées, 6 [~] déjà dans main |
| CLEAN-1121c | ab288c8a | principale | 0a : Retours\ | non | 10:41 | 10:42 | 4be2ce4, 3 BUG-* gardés |
| CLEAN-1121d | a547ff5b | principale | 0a : procédure + check-list | non | 10:41 | 10:43 | bce2d69, 6 lignes retirées |
| sync (commande) | — | 8 worktrees | sync_worktrees.ps1 -WaitBuilds | non | 10:44 | 10:44 | 8 à bce2d69 (+4 docs), aucun build |
| ETAPE0-1121 | a04dd38f | lecture seule | plan de lancement | non | 10:45 | 10:49 | Docs\Rapports\ETAPE0-1121.md ; plafond C++ 3 (46 Go libres) |
| GLORFINDEL-1121a | ab263fa0 | Glorfindel | chat : Tab perdu (relais UHOLScriptScreen) | oui | 10:50 | 10:53 | 6f14e8e rouge (scénario chat_input), f804471 NativeOnPreviewKeyDown ; non compilé |
| GALADRIEL-1121a | a110c744 | Galadriel | chat : marque « (Alliés) » | non | 10:50 | 10:52 | f0b44d4 rouge, 92c4901 ; D833 déjà faite |
| HALDIR-1121a | a047860b | Haldir | test MainCentre 150 s | non | 10:50 | 10:53 | .cpp supprimé -> test script bd900a5 (vert attendu, table déjà 150), c0deaa8 |
| CELEBORN-1121a | a634bbbd | Celeborn | D866 moral sous-étape 3 | oui | 10:50 | 10:53 | 9792b9f rouge + bouchon, 61ee604 FHOLMoraleFragment ; reste : fragment sur archétypes |
| ELROND-1121a | a7033431 | Elrond | scénario deploy : capture trop tôt, FocusNear | non | 10:50 | 10:55 | 0acd77c rouge vu, 5375b7a, f48b70c ; preflight deploy PASS 68 s |
| GALADRIEL-1121b | ac4a094c | Galadriel | alertes mineures : son par type | non | 10:53 | 10:57 | D872 déjà dans main ; d5cd309 rouge, cce215c ; ~30 appels (hors règle 25) |
| HALDIR-1121b | ac9eed4e | Haldir | ligne suivante (SC3) | non | 10:53 | 10:55 | rien de codable sans C++/éditeur ; relevé f85b3c0 |
| intégrateur (commande) | — | Celeborn | integrate.ps1 integ-38 : rouges f0b44d4,9792b9f ; verts 92c4901,61ee604,f804471,c0deaa8 | oui | 10:57 | | (Arwen laissé aux essais de l'utilisateur) |
| RECORD-1121a | a6a1d4c7 | principale | enregistrer deploy BUG + virtuous_knight OK (GDD, file, archive) | non | 10:59 | 11:01 | D877 (GDD non commité), f8fd9d9 + 8dfc453 ; Retours : 0 essai, 1 BUG en file |
| GALADRIEL-1121c | aab58625 | Galadriel | ligne sans C++ ou lot C++ groupé prêt | non | 11:01 | | |
| GLORFINDEL-1121b | a2a270e4 | Glorfindel | ligne sans C++ | non | 11:01 | | |
| SAURON-1121a (fin) | ae6a49f6 | Sauron | — | — | — | 10:58 | 4387164 ; baisse de PV = redirection D864/D865 (journal chiffré), aucun coup direct ; C++ au lot : UE_LOG ApplyDamageRedirect |
| SAURON-1121b | a0ac5c4c | Sauron | ligne sans C++ | non | 11:02 | 11:01 | crête de spawn deploy : aea9d23 rouge vu, 1263954, 58b7a69 (mêmes lignes FocusNear qu'Elrond, mêmes valeurs) |
| GALADRIEL-1121c (fin) | aab58625 | Galadriel | — | — | — | 11:00 | infobulle de revenu = panneau HUD : 88c5b54 rouge, fef7e1f |
| GLORFINDEL-1121b (fin) | a2a270e4 | Glorfindel | — | — | — | 10:58 | règle Suivre : 77b30f8 rouge, e259262 |
| GLORFINDEL-1121c | a073b6b6 | Glorfindel | brancher Suivre | non | 11:03 | 11:06 | HOLFollowOrders.as : 2f88481 rouge, b5e9e29 ; branchement exige C++ (position d'unité par id) |
| ELROND-1121b (fin) | adcf6c0b | Elrond | — | — | — | 10:58 | file sans C++ vide (faite ou bloquée par TowerWood F8) |
| ELROND-1121c | aeefd341 | Elrond | TowerWood F8 données | non | 11:00 | 11:02 | 84add13 rouge, 482e26b CSV+uasset, ff7f583 ; Q-towerwood |
| HALDIR-1121c (fin) | afbfe916 | Haldir | — | — | — | 11:01 | SecondaryCentre : e75f069 rouge, 4599ab8 CSV+uasset, 8a51d85 ; Q-secondary-centre |
| FARAMIR-1121a (fin) | ac5e1184 | Faramir | — | — | — | 11:03 | gate Essay accepte un sha : 10420ec rouge vu, 6b382c8, gate_test 107/0 ; order_lines non fait |
| intégrateur integ-38 (fin) | — | Celeborn | — | — | — | 11:10 | PASS 957 s : 3 rouges vus nom par nom, 1979/1979, net,build PASS ; main -> 753b81b (ff, 14:09, après merge des docs 8dfc453), rien poussé |
ARRÊT 11:10 -> 14:09 : la session principale a posé Q-towerwood + Q-secondary-centre par AskUserQuestion en séance AUTONOME ; l'appel a bloqué l'orchestrateur ~3 h (aucun lancement). Remarque de l'utilisateur 14:1x : « tu n'aurais pas dû me poser les questions, tu étais en autonome ». Leçon L21-1. Réponses : A et A (enregistrées par RECORD-1121b).
| intégrateur integ-39 | — | Celeborn | rouges d5cd309,88c5b54,77b30f8,2f88481,84add13 ; verts fef7e1f,b5e9e29,ff7f583,58b7a69,6b382c8 (Haldir exclu : conflit uasset avec Elrond) | non | 14:11 | | |
| RECORD-1121b | a4bec6f2 | principale | réponses TowerWood A, centre secondaire A (D878, D879) | non | 14:11 | | |
| ELROND-1121b | adcf6c0b | Elrond | ligne suivante (hors BUG deploy, réservé prochaine séance) | non | 10:59 | | |
| HALDIR-1121c | afbfe916 | Haldir | SC2 centres secondaires (CSV + regen_tables + test) | non | 11:00 | | |
Réponses de l'utilisateur 10:57 (conversation) : deploy BUG (clic cible démonté = à portée + montage + tir ; montage auto en fin de déplacement, TOUS les engins, précisé 10:58), virtuous_knight OK. Mode : reste AUTONOME (choix de l'utilisateur 10:58, L9-9 proposé) : le BUG attend la prochaine séance.
| SAURON-1121a | ae6a49f6 | Sauron | scénario virtuous_knight (PV 155 -> 148) | non | 10:50 | | |
| FARAMIR-1121a | ac5e1184 | Faramir | gate Essay : sha dans `branches:` ; puis order_lines_blocks | non | 10:50 | | |
Arwen : pas d'agent (exception écrite) : worktree des essais donnés à l'utilisateur (deploy, virtuous_knight) et de l'intégration ; la tâche order_lines de l'étape 0 passe à Faramir (tâche 2).

Plan de lancement 10:5x : voir `Docs\Rapports\ETAPE0-1121.md`. Plafond C++ proposé 3 (vm 46 Go libres, aucun cl.exe, `BuildQueue.state` illisible : accès refusé) : Glorfindel (chat Tab), Haldir (test MainCentre 150 s), Celeborn (moral D866 3), dernier lancement C++ 13:40.
Sans C++ : Arwen (vérifie BUG-order_lines_multi_blocs à l'écran), Elrond (deploy.txt:83-85), Faramir (session_gate L19-3), Galadriel (marque « (Alliés) » HOLChatInputScreen.as:24), Sauron (virtuous_knight Hold). 6 lignes [~] de la file : 4 périmées ou quasi, 2 gardées (MatchConfig, garnison UI).

EXCEPTION AU GATE 10:4x : `-Phase Essay -Root Arwen` (deploy, virtuous_knight) = 2 FAIL « branche 'f46dc2d' inconnue » : faux positif, la ligne `branches:` des 2 fiches E cite un sha (ancêtre de main, vérifié par merge-base) que le gate lit comme une branche ; autres contrôles PASS (aucun écart de code avec main bce2d69, worktree propre, binaire 23:39 >= code 23:32). Commandes redonnées à l'utilisateur. À faire par un agent : fiches E ou gate (sha accepté).

FIN DE SÉANCE 14:53 (dépassement 13 min, règle de durée) : integ-40 FAIL Test_CGTables (Elrond retiré) ; integ-41 (Galadriel fef7e1f, Glorfindel b5e9e29, Sauron 58b7a69, Faramir 6b382c8) tests 1990/1990, net,build FAIL « valeurs vides » sous charge puis net rejoué seul PASS 361 s (L13-4) ; main 753b81b -> a987f58 (ff local, rien poussé). sync_worktrees -WaitBuilds 14:35 : 8 worktrees à jour (Elrond 78b533a, Haldir 2c5cadf, Galadriel c5ac151 gardent leurs commits non intégrés), 8 builds PASS (118-149 s). `-Phase Close` 14:53 : 1 FAIL = GDD.md non commité (D877-D879, accord de l'utilisateur nécessaire). Non intégrés : Elrond integ-25 (deploy scénario + TowerWood, Test_CGTables), Haldir wall-line (SecondaryCentre).

### Bilan 2026-11-21

- **Agents** : 23 agents Sonnet (4 nettoyage 0a, ETAPE0-1121, 3 Glorfindel, 3 Galadriel, 3 Haldir, 1 Celeborn, 3 Elrond, 2 Sauron, 1 Faramir, 2 RECORD) + commandes `integrate.ps1` (integ-38, 39, 40) et un sync ; séance autonome 10:40 -> 14:40, chiffres tirés du tableau ci-dessus (commits : un rouge + un correctif par tâche, voir la colonne Résumé).
- **main** : 61a60e8 -> 753b81b par integ-38 (PASS 957 s, 3 rouges vus nom par nom, tests 1979/1979, net,build PASS, ff local à 14:09 après merge des docs 8dfc453), rien poussé.
- **Passes suivantes** : integ-39 (14:11) rouges 11/11 vus ; integ-40 (verts seuls, Elrond 0eba386 compris) = FAIL 1992/1993 (14:21) : régression Angelscript.UnitTests.Tests.HOLCommandGrid_Test.Test_CGTables, probablement la case de grille 7 de TowerWood (risque signalé par ELROND-1121c) ; Elrond (integ-25 : scénario deploy + TowerWood) retiré de la passe, Haldir (SecondaryCentre 4599ab8) non intégré non plus (conflit .uasset avec Elrond) ; integ-41 = 4 verts (Galadriel fef7e1f, Glorfindel b5e9e29, Sauron 58b7a69, Faramir 6b382c8) en cours.
- **ARRÊT 11:10 -> 14:09** : `AskUserQuestion` en séance autonome a bloqué la session principale ~3 h, aucun lancement (L21-1). Réponses de l'utilisateur : deploy BUG D877 (clic sur cible démontée = à portée + montage + tir ; montage auto en fin de déplacement, tous les engins), virtuous_knight OK, TowerWood A (D878), centre secondaire A (D879).
- **Lot C++ groupé signalé par les agents** : `UE_LOG` dans ApplyDamageRedirect (Sauron) ; position d'unité par id pour Suivre (Glorfindel) ; accesseur des coups reçus + lieu de recherche pour les alertes (Galadriel) ; C++ « tout centre » du centre secondaire (Haldir) ; colonnes d'attaque de TowerWood (Elrond) ; `FHOLMoraleFragment` sur les archétypes (Celeborn).
- **Non appliquées** : L19-6 (mal appliquée), L20-8, L14-11, L15-4, L18-d, L20-5. Nouvelles leçons : L21-1 à L21-3 (Auto_Journey_Checklist.md).

### Lecons appliquees 2026-11-21
Séance du 2026-11-21 (10:40 -> 14:40), bilan écrit à 14:2x par CLOSE-1121a depuis le tableau d'agents ci-dessus ; les contrôles de `session_gate.ps1 -Phase Close` n'ont pas été relancés par cet agent, « non vérifiée » = absent du tableau.
T2-1 : appliquee : ETAPE0-1121 lancé 10:45, plan de lancement dans Docs\Rapports\ETAPE0-1121.md (plafond C++ 3, 46 Go libres)
T2-2 : appliquee : tableau une ligne par worktree libre (Glorfindel, Galadriel, Haldir, Celeborn, Elrond, Sauron, Faramir) ; Arwen : exception écrite (essais de l'utilisateur)
L3-12 : appliquee : ETAPE0-1121 a tiré les tâches des lignes de file (SC2, SC3, D866 3, TowerWood F8)
L5-7 : non vérifiée : 19 fichiers non suivis (Docs\Rapports\*-1121*, Docs\*) au git status de départ, question non relevée au tableau
L4-1 : appliquee : fin 14:40 dans Docs\SessionEnd.txt, veilleur 30 min, alarme 14:25
L4-3 : sans objet : aucune prolongation (extend_session.ps1) au tableau
L3-8 : appliquee : intégrateur Arwen nommé au lancement (commandes integrate.ps1 du tableau)
L4-9 : appliquee : intégrateur Arwen (integ-38 à 41)
Q5 : sans objet : aucune recette indépendante, aucun essai donné après 10:58 (deploy et virtuous_knight tranchés par l'utilisateur)
L1-14 : appliquee : intégrateur nommé (Arwen) ; rejeu des tests sur le résultat exact par integrate.ps1
L3-9 : non vérifiée : compte d'appels de la session principale non relevé (pas de point de reprise écrit)
T2-3 : non vérifiée : énoncés « lis agent_rules.md » non relus par cet agent
L1-1 : non vérifiée : seul GALADRIEL-1121b a un compte au tableau (~30 appels, hors règle 25) ; aucun dépassement de 45 noté
L3-6 : non vérifiée : aucun remplacement d'agent au tableau
L4-7 : non vérifiée : `tool_uses` absent du tableau
L5-3 : non vérifiée : même contrôle que L1-1
L5-6 : non vérifiée : message de la session principale non relevé
L3-11 : sans objet : aucune régénération de cartes (CSV + uasset TowerWood et SecondaryCentre : 482e26b, 4599ab8)
L5-5 : sans objet : aucune mesure de perf ni de budget
L3-7 : appliquee : veilleur 30 min à :13/:43, alarme 14:25
L1-4 : appliquee : veilleur silencieux si rien ne change (choix de l'utilisateur 10:58)
T2-4 : appliquee : 2 questions Q-*.md (Q-towerwood, Q-secondary-centre), mais posées par AskUserQuestion (voir L21-1)
Q5, L5-4 : sans objet : aucun correctif de recette à rejouer
Q8 : non vérifiée : défauts communs non relevés en tête de file
Q11 : sans objet : 0 essai donné (Retours : 0 essai, 1 BUG en file)
L4-11 : sans objet : aucun essai à plusieurs variantes
L3 / T2 : appliquee : fusion groupée par integrate.ps1 (integ-38 PASS 957 s, ff main 61a60e8 -> 753b81b)
L1-3 : appliquee : integrate.ps1 rejoue tests puis net,build (integ-38 : 1979/1979, net,build PASS)
L6-2 : appliquee : file lue par ETAPE0-1121 et nettoyée par CLEAN-1121b (48 [x] archivées), jamais en entier par la session principale
L6-4 : sans objet : le BUG deploy est réservé à la prochaine séance
L6-6 : sans objet : aucune mesure répétée
L7-7 : appliquee : première fusion integ-38 lancée 10:57 dès 4 verts (voir L14-11 pour la suite)
L8-4 : sans objet : aucun essai audio
L8-8 : appliquee : rouges C++ (9792b9f) et script passés avec les verts dans integ-38 (3 rouges vus nom par nom)
L9-1 : non vérifiée : aucune re-notification relevée au tableau
L9-6 : appliquee : listes de branches figées dans les commandes integrate.ps1 (Haldir exclu d'integ-39 : conflit uasset avec Elrond)
L9-8 : appliquee : 7 worktrees chargés de 10:50 à 11:03 ; Arwen exception écrite
L9-9 : appliquee : l'utilisateur répond à 10:57 ; mode interactif proposé, il choisit AUTONOME à 10:58
L9-10 : sans objet : aucun essai sonore
L10-2 : sans objet : aucun net rouge (net,build PASS à integ-38)
L10-3 : non vérifiée : lecture de `-Phase Report` non relevée au tableau
L11-3 : non vérifiée : BUG deploy rapporté à 10:57, verdict 5 min sur main non relevé (réservé à la prochaine séance)
L11-4 : non vérifiée : lignes de file cochées à la clôture, bilan en cours
L11-8 : appliquee : listes figées dans les commandes d'intégration (integ-38, 39), aucun message à l'intégrateur en cours
L11-9 : appliquee : intégrateur Arwen nommé ; « aucun essai donné » écrit (Retours : 0 essai)
L11-10 : sans objet : aucune mesure de mêlée
L10-5 : sans objet : aucun balayage A/B
L9-7 : sans objet : aucun tableau de mesure lu
L11-11 : sans objet : aucune fiche d'essai neuve
L11-12 : appliquee : Retours\ purgé par CLEAN-1121c (4be2ce4, 3 BUG-* gardés)
L11-13 : non vérifiée : heures annoncées par la session principale non relevées (agent de bilan : Get-Date lu)
L12-1 : appliquee : sync_worktrees.ps1 -WaitBuilds à 10:44 : 8 worktrees à bce2d69 (docs seulement, aucun build)
L12-1b : appliquee : plan ETAPE0-1121 : 46 Go libres, aucun cl.exe
L13-1 : sans objet : aucune erreur d'API 403 relevée
L13-3 : non vérifiée : docs commitées f8fd9d9 + 8dfc453 avant integ-38, mais D877 (GDD) non commité à 11:01
L13-4 : sans objet : aucun net rouge
L13-5 : non vérifiée : 19 non suivis au git status de départ, question « commiter ou exclure » non relevée
L13-6 : sans objet : aucun essai préparé (.look)
L13-7 : sans objet : aucun balayage de variantes
L14-2 : appliquee : plafond C++ 3 calculé à l'étape 0 (46 Go libres), 2 agents C++ (Glorfindel, Celeborn), dernier lancement C++ 13:40
L14-4 : sans objet : aucune réponse ambiguë (deploy « précisé 10:58 » enregistré tel quel par RECORD-1121a)
L14-5 : sans objet : aucun essai donné
L14-6 : appliquee : rouges VUS à l'exécution (integ-38 : 3 rouges nom par nom, 1979/1979 ; integ-39 : rouges 11/11)
L14-9 : sans objet : aucun défaut d'affichage commun
L14-10 : sans objet : aucun essai audio
L14-11 : non appliquee : integ-39 lancée à 14:11 alors que 5 verts (fef7e1f, b5e9e29, ff7f583, 58b7a69, 6b382c8) étaient prêts dès 11:06 ; arrêt L21-1
L15-3 : sans objet : aucun EDITS_REFUSES relevé au tableau
L15-4 : non appliquee : lignes en double ou « (fin) » hors ordre (SAURON-1121a, FARAMIR-1121a, ELROND-1121b, HALDIR-1121c, GALADRIEL-1121c, GLORFINDEL-1121b), ids « — » pour sync et intégrateurs
L15-6 : sans objet : builds par integrate.ps1 et sync (aucun Wait-Process d'agent)
L15-7 : non vérifiée : relevé GDD non refait par ETAPE0-1121 (relevé f85b3c0 de HALDIR-1121b seulement)
L15-8 : sans objet : aucun essai à rejouer
L15-9 : appliquee : une tâche = un rouge + un correctif (0acd77c/5375b7a, f0b44d4/92c4901, d5cd309/cce215c, 88c5b54/fef7e1f, 77b30f8/e259262, 84add13/482e26b, e75f069/4599ab8, 10420ec/6b382c8) ; GALADRIEL-1121b ~30 appels
L15-10 : appliquee : 16 rapports complets dans Docs\Rapports\*-1121*.md (git status de départ)
L16-1 : sans objet : aucune question tranchée par discussion
L16-2 : appliquee : gate Essay -Root Arwen 10:4x : 2 FAIL faux positifs (sha dans `branches:`) écrits en exception ; FARAMIR-1121a 6b382c8, gate_test 107/0
L16-3 : appliquee : main avancé après tests 1979/1979 sur le tip exact (integ-38, ff 753b81b)
L16-4 : appliquee : séance et questions en français, commits en anglais (b1df86b)
L16-5 : appliquee : chiffres de ce bilan tirés du tableau d'agents et des sorties d'integrate.ps1
L16-6 : appliquee : aucun build de confort (builds par integrate.ps1 et sync)
L16-7 : non vérifiée : aucune troncature de sortie PASS/FAIL relevée
L16-8 : non vérifiée : exception Essay 10:4x : sources C++ seules non revérifiées par cet agent (aucun écart avec main bce2d69 selon l'exception)
L16-9 : non vérifiée : sync 10:44 sans build, Get-StaleBinaryProblem par worktree non relevé
L17-a : appliquee : rouges d'integ-38 lus nom par nom avant de conclure
L17-b : non vérifiée : CLEAN-1121b a archivé 48 [x] et 6 [~] déjà dans main, accord de l'utilisateur non relevé
L17-c : non vérifiée : Docs\BuildQueue.state illisible (accès refusé, ETAPE0-1121)
L18-a : non vérifiée : CSV + uasset régénérés par ELROND-1121c et HALDIR-1121c (482e26b, 4599ab8), relecture git diff non relevée
L18-b : sans objet : aucune règle neuve de faction
L18-c : appliquee : dernier lancement C++ fixé 13:40, aucun C++ lancé après 11:03
L18-d : non appliquee : ligne neuve de BuildingTypes.csv sans entrée placeholders_map.csv (check_placeholders rouge à integ-39) et case de grille 7 de TowerWood sans Grep des tests de grille (HOLCommandGrid_Test.Test_CGTables rouge à integ-40, 1992/1993) (voir L21-3)
L18-e : appliquee : rouges annoncés puis vus (integ-38 : 3 ; integ-39 : 11/11)
L18-f : appliquee : ELROND-1121a : scénario deploy, capture trop tôt corrigée (5375b7a), preflight deploy PASS 68 s
L19-1 : appliquee : tout délégué (correctifs, diagnostics, pré-vols, fiches, nettoyage, enregistrement des réponses) ; exceptions : 2 Edit de Retours et le tableau d'agents par la session principale, permis
L19-2 : non vérifiée : champ DIAGNOSTICS_AS non relevé ; GLORFINDEL-1121a f804471 non compilé
L19-3 : appliquee : copie principale : documents seulement (git status de départ)
L19-4 : non vérifiée : tâches de données TowerWood (482e26b) et SecondaryCentre (4599ab8), recherche dans quick_check.ps1 et Tools\Scenarios non relevée ; voir L21-3
L19-5 : sans objet : aucun essai neuf, aucune fiche E
L19-6 : non appliquee : MAL appliquée : posée par AskUserQuestion en séance AUTONOME (Q-towerwood, Q-secondary-centre, 11:0x), blocage 11:10 -> 14:09 -> voir L21-1
L19-7 : appliquee : mémoire libre lue avant les lots C++ (46 Go libres, ETAPE0-1121)
L19-8 : appliquee : rouges séparés des verts (0acd77c/5375b7a, 6f14e8e/f804471, 9792b9f/61ee604, d5cd309/cce215c, f0b44d4/92c4901, ...)
L19-9 : appliquee : integrate.ps1 pour integ-38, 39, 40 et 41
L20-1 : non vérifiée : merge=union sur *InProgress.md non relu après fusion
L20-2 : appliquee : build integ-38 PASS, aucune erreur de compilation C++ relevée à l'intégration
L20-3 : non vérifiée : sortie de `-Phase Close` pas relancée
L20-4 : sans objet : aucun essai donné, aucun correctif de recette
L20-5 : non appliquee : GLORFINDEL-1121a : C++ non compilé par l'agent (6f14e8e, f804471) alors que la file de build était vide
L20-6 : appliquee : integrate.ps1 à chaque passe (integ-38, 39, 40, 41)
L20-7 : appliquee : integ-40 et integ-41 verts seuls (rouges déjà vus à integ-39)
L20-8 : non appliquee : premier Edit après le 8e appel chez GALADRIEL-1121b, CELEBORN-1121a, HALDIR-1121a, HALDIR-1121c, SAURON-1121a, ELROND-1121b
L20-9 : appliquee : tables régénérées en CSV + uasset (482e26b, 4599ab8)
L6-1 : non vérifiée : sortie de `Launch` non relevée au tableau
L8-7 : non vérifiée : même contrôle que L6-1
L6-3 : non vérifiée : `-Phase Report` non relevé
L7-8 : sans objet : Close -Exclude non utilisé
L8-3 : appliquee : heure de fin lue dans Docs\SessionEnd.txt (14:40)
L9-3 : appliquee : gate Essay sur un sha corrigé par FARAMIR-1121a (10420ec rouge vu, 6b382c8)
L11-1 : non vérifiée : Launch -Scenarios non relevé
L11-7 : non vérifiée : Launch -NoCpp non relevé
L6-5 : appliquee : build avant les tests dans integrate.ps1 (integ-38 : build PASS puis 1979/1979)
L7-4 : non vérifiée : stray_processes.ps1 en dernier geste des agents non relevé
L7-6 : sans objet : aucune fiche E écrite
L8-1 : appliquee : commandes `-Root Arwen` données à l'utilisateur (deploy, virtuous_knight), exception du gate écrite
L9-5 : appliquee : docs commitées par nom avant integ-38 (f8fd9d9, 8dfc453)
L10-1 : non vérifiée : aucun play.ps1 sans -Hidden relevé
L8-5 : non vérifiée : known_reds.txt non relevé
L8-6 : non vérifiée : Write/Edit sans heredoc non relevé
L7-1 : non vérifiée : même contrôle que L1-1
L9-2 : non vérifiée : même contrôle que L1-1
L7-3 : sans objet : aucun éditeur de scénario concurrent relevé (ELROND-1121a et SAURON-1121b ont modifié les mêmes lignes FocusNear : voir L21-2)
L11-5 : non vérifiée : commit_docs.ps1 non relevé
L11-6 : non vérifiée : play.ps1 -Hidden non relevé

## Résumé de la séance du 2026-11-20 soir

- Séance autonome « choses en suspens » 22:38 -> ~01:00 (fin prolongée une fois), session principale + ~25 agents Sonnet ; 7 passes d'`integrate.ps1` (integ-31 à 37, 36 et 37 vertes) ; main = f46dc2d (code) + docs, LOCAL, rien poussé ; tests 1977/1977, `net` et `build` PASS, gate Close 00:22 sans FAIL.
- Entré dans main, visible : D869b montage de l'engin (barre dès l'ordre, 2e cycle « Démonter »), chat (2e Entrée rouvre le champ, marque de canal lisible), `virtuous_knight` (redirection prouvée, case E active), « Population pleine » 15 s ; outils : `merge=union` des journaux (L20-1), gate Close L19-1 (L20-3), `regen_tables.ps1` (L20-9), `integrate.ps1` repart de main (37ee2c0).
- Trouvé : 2 erreurs de compilation script à l'intégration (accolade manquante, `WidgetBlueprint::` au lieu de `Widget::`), 1 test faux (SecondMountReturnsToPack), 1 énoncé qui lisait mal D833. Reste en tête de file : chat Tab perdu (C++ probable, reporté), « Allies » sans accent, capture deploy_move_unmounting trop tôt, `hol.Select.Hold` en scénario.
- Détail (tableaux d'agents, bilan, tableau « Lecons appliquees 2026-11-21 », exceptions de gate) : `Docs\Archive\Auto_Journey_Jour_2026-11-20_soir_archive_2026-11-21.md` ; 2 essais [À ESSAYER] (deploy, virtuous_knight) dans `Auto_Journey_Retours.md`.

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
