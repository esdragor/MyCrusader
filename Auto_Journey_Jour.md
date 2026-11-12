# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir) et `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Dernier bilan : 2026-11-11 (13:24-18:02, détail dans l'archive du 2026-11-12)

- Main = fd03266 après 3 fusions (rien poussé) ; tests 1173/1173, aiserver, aiduel PASS ; net non relancé sur le résultat fusionné.
- Fusionné : N7 état des camps, Palier 3 G4-G6 d (bouton), T4b Mains du guérisseur, Légions L4 (rage, IA, traductions), S5 prouvé sur le réseau, notereset/preflight, neutrals 3v3, essais de Sauron ; build de main réparé par Haldir (39ba5ec).
- Non fusionné : galadriel-tab a3e90a8 (conflit notereset avec fog), net-playtest c8bde0e/WIP S6 (test ROUGE), visual f0670cf+ (scénarios) : lignes 95, 100 et 57 de `Auto_Journey_File.md`.
- Scénario neutrals ramené à 3 Hommes d'armes : masque le résidu D641 (noté dans `NeutralsInProgress.md` et `Retours\E-neutrals.md`).
- Réponses de l'utilisateur du 2026-11-11 aux Q-*.md reportées (commit bacee4b) ; `Retours\ESSAIS-A-JOUER.md` nettoyé le 2026-11-12 (0 essai prêt).
- Leçons non appliquées le 2026-11-11 : plafond de 45 appels (agents à 46-64), commits de docs sur main pendant une fusion, calls.md partagé ; 2 tips rouges commités par des agents.

## Chapitres du GDD non commencés (relevé Arwen du 2026-10-30, état revu le 2026-11-12 au seul nom des `*InProgress.md`)

Relevé complet d'origine : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`, section du même nom. Depuis, un chantier est ouvert pour : Volcan (`VolcanoInProgress.md`), Technologies (`TechnologiesInProgress.md`), Arbre de talents (`TalentsInProgress.md`), Duel (`DuelInProgress.md`), Marché (`MarketInProgress.md`), Siège (`SiegeInProgress.md`), Neutres (`NeutralsInProgress.md`), Relais (`RelaysInProgress.md`), Légions Noires (`LegionsInProgress.md`). Reste sans chantier :
- **§ 16.2 World State Manager** (D29) : se construit avec le volcan, pas à part.
- **§ 5.2 Ressources spéciales, § 6.2 Bonus de hauteur** : décidés ; la hauteur est exclue du prototype pour les effets de terrain (M-Q2) ; à relire avant d'ouvrir.
- **D384 Centres secondaires** (XP de territoire) : à vérifier contre `RelaysInProgress.md` avant d'ouvrir.
- **Après le prototype, ne se prennent pas** : § 13 les quatre autres factions (Enfants du Dragon, Cercle de l'Ombre, Héritiers du Feu, Templiers), § 12.3 Événements futurs (D209), § 14.3 Modes Battlefield et Reliques (D416, D102), § 9.9 Progression hors partie.

## Programme du 2026-11-12 (séance AUTONOME, fin 13:00, plan de l'étape 0 écrit à 10:40)

Départ : main = b62464f (nettoyage des docs commité). Fusion groupée dans main autorisée par l'utilisateur, jamais de push. sync_worktrees lancé avec -NoBuild (exception : séance de 2 h 30, les agents compilent à leur premier build) ; Galadriel en conflit (fusion de main annulée, `Source/HeroesOfLiberty/Dev/HOLTestSequence.cpp`, verbe notereset en double avec fog). Glorfindel (net-playtest 6941e5b) : binaire périmé, +5 commits hors de main dont le WIP S6 rouge (2d47271, 0c8792c).

Plafond : 3 agents C++ en même temps. Premiers C++ : Galadriel, Glorfindel, Elrond. Arwen et Celeborn démarrent sans C++ et passent au C++ à la libération d'une place.

| Worktree | Branche | Tâche de départ (ligne de `Auto_Journey_File.md`) | C++ | Suivante |
|---|---|---|---|---|
| Arwen | d751-root | Neutres N7 côté script (ligne « sans C++ » N7), puis ligne C++ N7 point sur la minicarte + réplication des camps vus ; `NeutralsInProgress.md` | non au départ | N7 C++ à la 1re place libre |
| Celeborn | hero-j8-close | BUG neutrals : bras du Lancier dans le corps, diagnostic par capture d'abord (Lancier neutre contre Lancier du joueur) ; `NeutralsInProgress.md` | non au départ | correctif (C++ si la pose est codée), puis BUG-melee-stuck-attack |
| Elrond | tiers | Palier 3 G6 e (réplication de la phase) puis f (scénario deploy) ; `Palier3InProgress.md` | oui | bouton case D, G4 filtre Faction |
| Faramir | fog | Scénarios qui ne démarrent jamais chez l'utilisateur (legions_roster_gesture, disconnect_1v1, audio_compare_buildings_stone/_metal) ; cas ajouté au preflight | non | Q-play-hidden-wait (A en place, C verbe quit) ; worktree de la fusion groupée |
| Galadriel | galadriel-tab | 1er geste : fusion de main, conflit HOLTestSequence.cpp (garder notereset de fog), build ; puis Rage L4 plafond 40 % (script), Rage visuel défaut 4 ; `LegionsInProgress.md` | oui | REGLAGE market, REGLAGE talents_icons_compare ; infobulle Caserne après la fusion de net-playtest |
| Glorfindel | net-playtest | 1er geste : build (binaire périmé) ; infobulle de revenu par ressource (Q-sites-gold-line = Autre), abandon de GoldLine et du rouge 0c8792c ; `SacredSitesInProgress.md`, `InterfaceInProgress.md` | oui | Sites S7 option de partie |
| Haldir | maps | Pas de régénération des 7 cartes (éditeur seul, file de build vide : impossible avec 7 agents) ; chapitre du GDD : D384 Centres secondaires contre `RelaysInProgress.md` (plan et règles pures) | non | § 5.2 / § 6.2 à relire |
| Sauron | visual | Placeholders ressources : figer Arbre = C, baies / pierre / or = A ; `PlaceholdersInProgress.md` | non | ligne `[?]` Trajets d'ordres de `Placeholders.txt` (OK de l'utilisateur) |
| Outils/docs | (session principale) | Reporter Q-legions-ai-warrior, Q-play-hidden-wait, Q-talents-heal-visual dans leur journal ; `-NoCpp` au modèle d'énoncé | non | réécrire la ligne « essais ratés du 10-31 » |

Fusion groupée de fin de séance : dans Faramir (fog = main, arbre propre), candidates = tiers, d751-root, hero-j8-close, fog, visual, galadriel-tab (conflit résolu), net-playtest (seulement si le WIP S6 est remplacé par un tip vert), maps (docs seules).

## Agents du 2026-11-12

| Nom | Id | Worktree | Branche | Tache | Lance |
|---|---|---|---|---|---|
| ARWEN-1112a | adfd8d813d0ee3470 | Arwen | d751-root | Neutres N7 (script puis C++ minicarte) | 10:39 |
| CELEBORN-1112a | a0ac31f1f11ca88cc | Celeborn | hero-j8-close | BUG bras du Lancier neutre, puis melee-stuck | 10:39 |
| ELROND-1112a | ad24ecdf475537e57 | Elrond | tiers | Palier 3 G6 e/f, bouton case D, G4 filtre (C++ 1) | 10:39 |
| FARAMIR-1112a | ab77d8eddd5c5a203 | Faramir | fog | Scenarios jamais demarres + preflight, Q-play-hidden-wait | 10:39 |
| GALADRIEL-1112a | ae7e4e7f809c1165b | Galadriel | galadriel-tab | Conflit de fusion, Rage L4 plafond et visuel (C++ 2) | 10:39 |
| GLORFINDEL-1112a | a2bf758e6f3867c63 | Glorfindel | net-playtest | Build, infobulle de revenu, retrait GoldLine (C++ 3) | 10:39 |
| HALDIR-1112a | a97ce746cad9802ef | Haldir | maps | Chapitre GDD D384 Centres secondaires (sans C++) | 10:39 |
| SAURON-1112a | a6d3ffa74439af3cb | Sauron | visual | Styles des ressources figes, Placeholders | 10:39 |
| DOCS-1112a | aaffdd14aada5859e | principale | main | Reponses a reporter, file Outils, checklist L9-4 | 10:39 |
| FARAMIR-1112b | a08fc0e0731156818 | Faramir | fog | Verification de bout en bout des correctifs de lancement (build, preflight complet) | 10:53 |

Rapports : DOCS-1112a fini 10:45 (6d77b69, 44 appels). FARAMIR-1112a fini 10:50 (fog 323b73a/42e5e41/953fafd, main 3b94faf, 45 appels) : causes des 4 essais jamais demarres = joueur non nomme (disconnect_1v1), sons non commites d Arwen (audio_compare), binaire perime (legions_roster_gesture) ; binaire de la copie principale perime : a recompiler apres la fusion.
Exception : FARAMIR-1112b lance avec le gate -NoCpp alors qu il compile (7 worktrees dans la file de build) : la verification des essais est la priorite de la seance, il attend son tour par build.ps1 -Wait.

Modeles (consigne du 2026-11-12) : etape 0 et verification sur Fable 5.2, tout le reste sur Opus 5.5. Arret dur des agents 12:30, fusion groupee dans Faramir ensuite (sans push), fin 13:00.
