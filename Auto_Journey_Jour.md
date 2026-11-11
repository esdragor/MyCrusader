# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin) et `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Agents du 2026-11-10 (seance AUTONOME 10:02 -> fin 10:55, arret dur des agents 10:55, alarme 10:40 cloture + fusion groupee, 3 agents C++ max = Galadriel, Arwen, Sauron, cartes SANS regeneration, integrateur/fusion = Elrond (agent neuf vers 10:35), main depart = 644b196, accord utilisateur : commits + fusions dans main, jamais de push ; veilleur 43cc6a94 toutes les 23 min)
| nom | id | worktree | branche | tache (ligne de Auto_Journey_File.md) | lancement |
|---|---|---|---|---|---|
| Galadriel-1 (C++) | a2ff5a5c | HeroesOfLiberty-Galadriel | galadriel-tab | L86 survol des stats (BUG utilisateur) | 10:05 |
| Arwen-1 (C++) | a50bdc9a | HeroesOfLiberty-Arwen | d751-root | L90 G14 champ Faction de FHOLAISummary | 10:05 |
| Sauron-1 (C++) | a8a8ad9d | HeroesOfLiberty-Sauron | visual | L88 hol.Decor.ClearNear | 10:05 |
| Celeborn-1 (sans C++) | aa7703a3 | HeroesOfLiberty-Celeborn | hero-j8-close | L89 mesure melee_stuck_on_bf 8 fois | 10:05 |
| Elrond-1 (sans C++) | ae078e21 | HeroesOfLiberty-Elrond | tiers | L87 note 5 scenario arquebusier | 10:05 |
| Faramir-1 (sans C++) FINI 10:09 (35 tool_uses ok) | ad2fe270 | HeroesOfLiberty-Faramir | fog | L26, L98, L24 | 10:05 |
| Glorfindel-1 (sans C++) | a36171a6 | HeroesOfLiberty-Glorfindel | net-playtest | L91 stray_processes python | 10:05 |
| Haldir-1 (sans C++) | a37c3759 | HeroesOfLiberty-Haldir | maps | L79 ligne N sites du journal make_maps | 10:05 || Faramir-2 (sans C++) | a44715cc | HeroesOfLiberty-Faramir | fog | L52, L53, L54 (note+look fog_orders_flicker, zz_edge_walk, fog_trees) | 10:12 |
| Glorfindel-2 (sans C++) | ae4199b3 | HeroesOfLiberty-Glorfindel | net-playtest | L70 fiches Deconnexion | 10:22 |
| (FINIS) Haldir-1 10:09 maps 0be29cf, Elrond-1 10:14 tiers 06ccaab (arquebusier note 5), Glorfindel-1 10:19 net-playtest 7701e5f (stray_processes python), Faramir-1 10:09 | | | | | |
| Elrond-2 INTEGRATEUR (fusion groupee) | a32a38a9 | HeroesOfLiberty-Elrond | tiers | fusion maps 0be29cf, net-playtest 906e70f, fog 934e078, tiers 06ccaab, hero-j8-close e160313 | 10:35 |
| Galadriel-1 FINI 10:33 NON COMPILE (50 tool_uses, 7bd1332 : hol.HUD.Hover, erreur de header corrigee, a recompiler par un agent neuf) ; Faramir-2 FINI 10:25 (44 tool_uses) ; Glorfindel-2 FINI 10:24 (15) | | | | | |
| Galadriel-2 (C++ compiler+tester) | ada56630 | HeroesOfLiberty-Galadriel | galadriel-tab | compiler 7bd1332, preflight unit_stats_attack, tests | 10:41 |
| Arwen-1 FINI 10:39 (85 tool_uses : HORS REGLE >80, G14 d751-root 3b4bd14 tests 1141/1141) ; Sauron-1 FINI 10:38 (55 tool_uses, visual 22e1f57 tests 1138) ; Celeborn-1 FINI 10:34 (50) | | | | | |
| Elrond-2 FINI 10:44 (17 tool_uses) : FUSION 1 FAITE main=tiers=004969d (maps, net-playtest, fog, hero-j8-close ; build 166 s, tests 1137/1137, aiserver+aiduel PASS) | | | | | |
| Elrond-3 INTEGRATEUR tour 2 | ab1f57d9 | HeroesOfLiberty-Elrond | tiers | fusion d751-root 3b4bd14 + visual 22e1f57 | 10:45 |
| Elrond-3 FINI 10:49 (16 tool_uses) : FUSION 2 FAITE main=2162a89 (d751-root 3b4bd14 + visual 22e1f57 ; build 73 s, tests 1142/1142, aiserver+aiduel PASS) | | | | | |
| Elrond-4 INTEGRATEUR tour 3 | ac3df5b9 | HeroesOfLiberty-Elrond | tiers | fusion galadriel-tab ec28851 | 10:50 |

## Bilan de la seance autonome du 2026-11-10 (09:57 -> fin 10:55, fin lue dans Docs\SessionEnd.txt, 1 h demandee par l'utilisateur, cartes SANS regeneration, 15 agents dont l'etape 0, 3 fusions)
- Fusions dans main (avance rapide, rien de pousse) : FUSION 1 main=004969d (maps 0be29cf make_maps.ps1 controle ", N sites" ; net-playtest 906e70f stray_processes type Python ; fog 934e078 scenarios fog ; hero-j8-close 474259b mesures melee ; build 166 s, tests 1137/1137, aiserver+aiduel PASS) ; FUSION 2 main=2162a89 (d751-root 3b4bd14 G14 champ Faction de FHOLAISummary + correctif des chiffres de l'Arquebusier pour l'IA, visual 22e1f57 hol.Decor.ClearNear ; build 73 s, tests 1142/1142, aiserver+aiduel PASS) ; FUSION 3 main=1bb7422 (galadriel-tab ec28851 hol.HUD.Hover, build 40 s, tests 1142/1142, aiserver+aiduel PASS). Docs main final = 4ef3a87.
- Fait : bug « pas d'infobulle pour les autres stats » : le code actuel est bon (hover 5/5 vert au hit-test Slate), l'essai du 09-11 etait sur un binaire d'avant d813698 ; G14 (l'IA Legions connait sa faction) ; Arquebusier : l'IA ne le commandait jamais faute de chiffres, corrige ; mesure melee : melee_stuck 5/8, melee_stuck_on_bf (RearKeepBothFull 1) 7/8, ecart non significatif a N=8, cvar laisse a 0, A/B N=16 a faire ; stray_processes arrete aussi les python/pythonw du Root et session_gate Launch les signale ; make_maps.ps1 refuse un journal sans 2 ou 3 sites ; note 5 du scenario arquebusier (preflight PASS, PNG vus) ; fog_orders_flicker_off et fog_trees (note, .look, curseur virtuel) ; Technologies T9 (GDD § 11 contre Technologies.csv) ; Placeholders sons mis a jour.
- Ouvert pour l'utilisateur : Retours\Q-technologies-legions.md [A REPONDRE] (lieu de recherche des 4 technologies Legions, defaut A = Caserne + Necropole) ; Retours\ESSAIS-A-JOUER.md date du 09-11 (build 60013e8, a rejouer sur main avant de le donner : -Phase Essay) ; arquebusier (E-arquebusier.md) a jouer ; Q-arquebusier-valeurs, Q-forge-armure, etc. inchanges.
- Non fait / a refaire : sync_worktrees SAUTE Glorfindel (Content/Rendering/MI_Ground_Plain.uasset modifie non commite, artefact de regeneration, sauvegarde dans Temp\MI_Ground_Plain.glorfindel.uasset) ; binaires des 6 autres worktrees perimes apres sync -NoBuild (build a lancer au prochain Launch) ; neutrals dans une clairiere (hol.Decor.ClearNear jamais lance en jeu : ajouter la commande a neutrals.txt, preflight, E-neutrals) ; Content/Maps/Crown.umap non suivi (Haldir) et Content/Audio/Candidates/ (Arwen) non suivis, laisses ; regeneration des 7 cartes (bSitesLaidByMaps = true) non planifiee ; D753-D766 du GDD non ecrits.
- Hors regle : Arwen-1 85 tool_uses (>80, <=100). Autres : Galadriel-1 50 (NON COMPILE, repris par Galadriel-2 21), Sauron-1 55, Celeborn-1 50, Glorfindel-1 49, Haldir-1 44, Faramir-2 44, Faramir-1 35, Elrond-1 33, Elrond-2 17, Elrond-3 16, Elrond-4 13, Glorfindel-2 15.

### Lecons appliquees 2026-11-10
- T2-1 : appliquee : etape 0 (agent adff08e7, 41 tool_uses) a rendu le plan par worktree, relu contre la procedure.
- T2-2 : appliquee : 8 worktrees lances a 10:05 (tableau ## Agents du 2026-11-10 ecrit avant le lancement).
- L5-1 : appliquee : session_gate Launch lu a 10:02 (53 min restantes) ; Get-Date lu avant chaque lancement ; 3 lancements tardifs (Galadriel-2 10:40, Elrond-3 10:43, Elrond-4 10:50) pour des fusions et une compilation, jamais d'agent de tache apres l'alarme.
- L5-2 : appliquee : ids ecrits dans le tableau au lancement ; aucun SendMessage au mauvais agent (2 messages a Elrond-2 et Elrond-3, ids relus).
- L3-1 / L4-4 : appliquee : 3 agents C++ (Galadriel, Arwen, Sauron) ; 3 builds ont attendu de 25 a 30 min en file derriere les builds de rattrapage (Elrond a lance un build de rattrapage de 397 s sans modifier de C++).
- L1-10 : appliquee : commit_docs.ps1 avant chaque lancement/fusion, copie principale sans fichier de code modifie ; 4 fichiers de docs non couverts (Placeholders*, TechnologiesInProgress, SessionEnd) commites par nom a 10:33.
- L4-1 / L4-3 : appliquee : alarme 10:40 posee (CronCreate), fin dure 10:55 ; 3 agents de fusion lances a 10:35, 10:44 et 10:50 (avant la fin), dernier fini 10:55.
- L3-8 / L4-9 / Q5 : NON appliquee : aucun integrateur de recette independante nomme ; Elrond a fusionne sans rejouer d'essai a l'ecran ; aucun essai donne a l'utilisateur.
- L5-3 / L7-1 : partiellement appliquee : 1 agent hors regle (Arwen 85 tool_uses > 80), aucun > 100.
- L1-2 : appliquee : tous les agents ont commite dans les 30 min (dernier commit visible au veilleur).
- L1-3 : appliquee : merge_main_fait = oui dans tous les rapports.
- L3-2 / L4-5 : partiellement appliquee : Galadriel-1 a rendu du code NON COMPILE (erreur de header) ; repris par un agent neuf (Galadriel-2) qui a constate le build vert ; builds de Sauron et Arwen lances en run_in_background.
- L6-3 / L8-3 : appliquee : fin lue dans Docs\SessionEnd.txt (10:55) ; session_gate Merge a refuse a 10:33 (main avance, copie principale sale) : docs commites, branches fusionnees dans tiers puis main en avance rapide.
- L9-1 : appliquee : aucune notification en boucle ; 4 agents ont d'abord notifie « waiting on background work » (Arwen 4 fois, Elrond-4) sans relance.
- L4-14 / L8-9 : partiellement appliquee : sync_worktrees au debut (10:02, deja a jour) et a la fin (10:55, -NoBuild) ; Glorfindel saute (uasset non commite d'une regeneration).
- Q1-Q4 / Q10 : sans objet : aucun essai donne ; arquebusier et fog_trees passes au pre-vol par leurs agents (PNG ouverts), non joues par l'utilisateur.
- L7-4 : appliquee : PROCESSUS_DETACHES = aucun dans les 14 rapports ; Faramir-2 a du arreter 2 editeurs laisses par play.ps1 -Hidden (anomalie signalee, a corriger).

## Agents du 2026-11-11

Test d'1 h, 13:30-14:30 (agents arret 14:12, fusion groupee ensuite), fusions dans main OK, jamais de push. Nettoyage 0a fait (e1db39a, 34717c0, 89a5a0b, e09d1cb). Pas de veilleur (1 h, les notifications suffisent). Non lances : Celeborn (lignes melee perimees, D641 ecarte, D769 fait), Faramir (aucune ligne), Glorfindel et Haldir (file de build trop longue pour 1 h, taches a part).

| Nom | Id | Worktree | Branche | Tache | Lance |
|---|---|---|---|---|---|
| Arwen-N7 | aee5251a39a3a634a | HeroesOfLiberty-Arwen | d751-root | C++ Neutres N7 GetCampState | 13:31 |
| Elrond-G4 | a00898a21645ceedb | HeroesOfLiberty-Elrond | tiers | C++ Palier 3 G4 | 13:31 |
| Galadriel-T4b | a7277cf1a836dc226 | HeroesOfLiberty-Galadriel | galadriel-tab | C++ Talents T4b Mains du guerisseur | 13:31 (arret 13:37 : 55 appels, test rouge seul commite 902dad0) |
| Galadriel-T4b-2 | a5e9988c064f438c3 | HeroesOfLiberty-Galadriel | galadriel-tab | C++ T4b correctif (agent neuf) | 13:38 |
| Sauron-CN | a8b90f320e2d826a2 | HeroesOfLiberty-Sauron | visual | sans C++ scenario neutrals ClearNear | 13:31 |
| Fusion-1 | acb66774812037546 | HeroesOfLiberty-Faramir | fog | fusion groupee d751-root, tiers, visual, galadriel-tab | 13:52 (arret 14:30) |
| Sauron-PQ1 | a942452d8e7491995 | HeroesOfLiberty-Sauron | visual | sans C++ placeholders ressources/decor, dix echantillons | 13:53 |
| Arwen-outils | a10ee7ddc751061e5 | HeroesOfLiberty-Arwen | d751-root | sans C++ commit_docs, play -Hidden | 13:53 |
| Glorfindel-L | afdb8e6fd350c6bc9 | HeroesOfLiberty-Glorfindel | net-playtest | sans C++ gestes Legions + quick_check -Full | 13:53 |
| Galadriel-heal | a7f4f37dbeb73c398 | HeroesOfLiberty-Galadriel | galadriel-tab | sans C++ scenario talents_heal | 13:55 |

PROLONGATION 13:52 sur demande de l'utilisateur (« repart pour 1h de plus ») : fin de seance 15:30 (14:30 + 1 h), agents d'arret 15:12, fusion groupee 2 ensuite. Elrond libre (G4 partiel, pas de relance : cap de 3 agents C++).
| Galadriel-L4 | a4887747d0e3044e3 | HeroesOfLiberty-Galadriel | galadriel-tab | C++ Legions L4 etape 1 | 13:58 |
| Sauron-PH | a741877d2e87b3bd0 | HeroesOfLiberty-Sauron | visual | sans C++ placeholders [?] _compare | 14:01 |
| Elrond-G5 | a4695a05182a3066a | HeroesOfLiberty-Elrond | tiers | C++ Palier 3 G5/G6 | 14:10 |
FUSION 1 faite 14:09 : main = 0ba3651 (e24d24f), d751-root + tiers + visual + galadriel-tab(71b35d9), build PASS, tests 1148/1148, aiserver, aiduel PASS. Pas dans main : a399b77 (scenario talents_heal), Sauron 376d95c+, Arwen outils, Glorfindel. Fusion 2 vers 14:45 puis integrateur (rejeu essais sur main).

PROLONGATION 15:03 sur demande de l'utilisateur (« tourne jusqu'a 18h ») : fin de seance 18:00 (remplace 15:30), agents d'arret 17:40, fusion groupee 3 a 17:40-18:00. Vague 2 (15:05) : Galadriel-L4f, Elrond-G6, Glorfindel-Full3, Arwen-outils2, Sauron-essais, Celeborn-neutrals (ids ci-dessous). Fusion-2 (a1f58945f52967071) finit : main + db4598e.
