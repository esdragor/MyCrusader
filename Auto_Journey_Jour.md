# Auto_Journey : l'état du jour

Mode d'emploi : ce fichier ne garde que la dernière journée de travail (tableaux « ## Agents du <date> », bilans, « ### Lecons appliquees <date> » : titres lus par `Tools\AutoJourney\session_gate.ps1`, ne pas les renommer). Procédure : `Auto_Journey.md` ; file de travail : `Auto_Journey_File.md` ; ce qui attend l'utilisateur : `Auto_Journey_Retours.md` et le dossier `Retours\`.
Les séances plus anciennes sont archivées telles quelles dans `Docs\Archive\Auto_Journey_Jour_jusqu_au_2026-11-09_matin.md` (jusqu'au 2026-11-09 matin), `Docs\Archive\Auto_Journey_Jour_2026-11-09_soir_archive_2026-11-11.md` (séance du 2026-11-09 soir), `Docs\Archive\Auto_Journey_Jour_2026-11-10_et_2026-11-11_archive_2026-11-12.md` (séances du 2026-11-10 et du 2026-11-11 : tableaux d'agents, bilans, leçons), `Docs\Archive\Auto_Journey_Jour_2026-11-12_archive_2026-11-13.md` (séance du 2026-11-12 : programme, tableau d'agents, bilan, leçons ; résumé du 2026-11-11), `Docs\Archive\Auto_Journey_Jour_2026-11-13_matin_archive_2026-11-13.md` (séance interactive du 2026-11-13 matin et reprise : tableau d'agents, programme, bilans, leçons) et `Docs\Archive\Auto_Journey_Jour_2026-11-13_soir_archive_2026-11-14.md` (séance autonome du 2026-11-13 soir : bilan, leçons, tableau d'agents ; résumés du 2026-11-13 matin et du 2026-11-12) et `Docs\Archive\Auto_Journey_Jour_2026-11-14_archive_2026-11-15.md` (séance autonome du 2026-11-14 : tableau d'agents, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-15_archive_2026-11-16.md` (séance autonome du 2026-11-15 : tableau d'agents, fusions n°1-7, bilan, leçons ; résumé dans ce fichier) et `Docs\Archive\Auto_Journey_Jour_2026-11-16_apres-midi_archive_2026-11-16.md` (séance interactive du 2026-11-16 après-midi : tableau d'agents, lignes d'exception, bilan, leçons ; résumé dans ce fichier) ; à la prochaine séance, déplacer de même la journée précédente.
Historique du test du 2026-10-30 : `Docs\Archive\Auto_Journey_Test1_2026-10-30.md`.

## Chapitres du GDD non commencés

**Relevé du 2026-11-15** : voir Docs\ReleveGDD_2026-11-15.md (état réel de A1-A4 et B1-B6, branchements C++ à faire, questions en attente). L'ancien relevé du 2026-11-12 est archivé dans Docs\Archive\Auto_Journey_Jour_Chapitres_GDD_releve_2026-11-12_archive_2026-11-15.md.

## Agents du 2026-11-19

SÉANCE DE FONCTIONS DU SOIR (« commence ton Auto_Journey », 21:08 -> 23:00, AUTONOME ; la première depuis la fin de la migration, D857). Réponses de l'utilisateur à 21:1x (interface de choix) : fin à 23:00 (`Docs\SessionEnd.txt`) ; fusion dans main acceptée EN LOCAL si build PASS et tests verts sur le résultat, jamais de push ; mode autonome ; les trois docs non suivis de `Docs\` ne sont pas commités, et `Docs\Priorite_Socle_Factions_2026-11-16.md` ORIENTE la séance (socle commun des factions : 1 emplacement de faction + jauge + livre de pouvoirs, 2 capacité visée au sol, 3 moteur de moral, 4 spécialisation, 5 récompenses du duel, 6 fortifications, 7 IA d'une autre faction ; en parallèle Aube A1 et A2). Règles : agents Sonnet 5.5 de 15 min / 25 appels, sans build ni quick_check ; code neuf en Angelscript (D839) ; builds, tests (UN éditeur à la fois) et fusion groupée par la session principale, qui est l'intégrateur ; essais donnés depuis un worktree qui porte main compilé. État au lancement : main 2ff2dbf (local, 456 commits d'avance sur origin), les 8 worktrees sur main (Faramir `fog` +26 commits propres à sa branche), Celeborn 16 fichiers modifiés non commités, Haldir 1 ; veilleur toutes les 30 min, alarme de clôture 22:45. Le tableau de la séance de migration du même jour (reprises 13 à 17) est archivé : `Docs\Archive\Auto_Journey_Jour_2026-11-16_soir_au_2026-11-19_migration_archive_2026-11-19.md`.

| Agent | id | worktree | branche | tâche | C++ | lancé |
|---|---|---|---|---|---|---|
| NETTOYAGE-1119n-file | ab66fb6f | MyCrusader (docs) | main | étape 0a : nettoyage d'`Auto_Journey_File.md` | non | 21:12 |
| NETTOYAGE-1119n-retours | ab148a9e | MyCrusader (docs) | main | étape 0a : nettoyage du dossier `Retours\` | non | 21:12 |
| NETTOYAGE-1119n-procedure | a30db92b | MyCrusader (docs) | main | étape 0a : `Auto_Journey.md`, check-list, `brief_commun.md` | non | 21:12 |
| (session principale) | - | MyCrusader (docs) | main | étape 0a : archive d'`Auto_Journey_Jour.md` et de l'en-tête d'`Auto_Journey_Retours.md` ; FINI 21:16 : commit d02ae9f, 8 worktrees synchronisés, gate Launch sans FAIL | non | 21:13 |
| ETAPE0-1119n-A | a021779b | MyCrusader (lecture seule) | main | étape 0 : priorités 1-2 + Aube A1 ; FINI 21:20 : 4 tâches, rapport `Docs\Rapports\ETAPE0-1119n-A.md` | non | 21:14 |
| ETAPE0-1119n-B | a4d2c8ac | MyCrusader (lecture seule) | main | étape 0 : priorités 3-5 + Aube A2 + état de Celeborn et Haldir ; FINI 21:20 : 3 tâches, rapport `Docs\Rapports\ETAPE0-1119n-B.md` | non | 21:14 |
| ELROND-1119n1 | ae261387 | HeroesOfLiberty-Elrond | faction-resource | règle pure de la jauge de ressource de faction (priorité 1, A4) | non | 21:23 |
| GALADRIEL-1119n1 | aea92e83 | HeroesOfLiberty-Galadriel | power-book | règle pure de l'état des boutons du livre de pouvoirs (priorité 1, A6a) | non | 21:23 |
| GLORFINDEL-1119n1 | a91ae324 | HeroesOfLiberty-Glorfindel | ground-ability | règle pure d'une capacité visée au sol (priorité 2, L6a) | non | 21:23 |
| CELEBORN-1119n1 | aa4556ac | HeroesOfLiberty-Celeborn | duel-rewards | règle pure des récompenses du duel (priorité 5, D7) | non | 21:23 |
| HALDIR-1119n1 | a794ea96 | HeroesOfLiberty-Haldir | templar-options | spécialisation : Templiers à 3 options (priorité 4) | non | 21:23 |
| SAURON-1119n1 | a0f2c420 | MyCrusader (docs) | main | question `Retours\Q-hallebardier-valeurs.md` (Aube A1, valeurs à choisir, D345) | non | 21:23 |
| GALADRIEL-1119n2 | a8265f09 | HeroesOfLiberty-Galadriel | faction-slot | vue pure de l'emplacement de faction du HUD (D201, D823), depuis c7067cf | non | 21:30 |
| ELROND-1119n2 | a6f8e6f6 | HeroesOfLiberty-Elrond | ai-power-spend | IA : règle pure de dépense des pouvoirs de faction (priorité 7, D688), depuis c7067cf | non | 21:30 |
| HALDIR-1119n2 | a8830f54 | HeroesOfLiberty-Haldir | wall-line | fortifications F4 : règle pure de la pose en ligne (priorité 6), depuis c7067cf | non | 21:30 |
| CELEBORN-1119n2 | aafeb08a | MyCrusader (docs) | main | question groupée `Retours\Q-socle-factions-valeurs.md` (valeurs de départ du socle, D345) | non | 21:30 |
| intégrateur = session principale | - | HeroesOfLiberty-Arwen | integ-24 | vague 1 : les 5 commits rouges fusionnés, `quick_check -Parts tests` 21:25-21:28 = 1785/1800, les 15 rouges attendus et pas un de plus (4 jauge, 3 livre, 3 capacité au sol, 3 duel, 2 Templiers) ; les 5 commits verts fusionnés (c7067cf), passe verte lancée 21:29 | non | 21:24 |
| ELROND-1119n3 | a3fbdb7a | HeroesOfLiberty-Elrond | faction-resource-enum | correctif : `HasResource(EHOLFaction)`, l'Aube seule (depuis 84f36fb) | non | 21:36 |
| GLORFINDEL-1119n2 | a1d335be | MyCrusader (docs, lit Arwen) | main | plan du lot C++ du socle : `Docs\Rapports\LOT-CPP-socle-factions-1119.md` ; FINI 21:40, 11 morceaux | non | 21:36 |
| GLORFINDEL-1119n3 | a2f81c98 | HeroesOfLiberty-Glorfindel | ability-columns | colonnes cible / rayon / portée de `UnitAbilities.csv` (morceau 5 du lot, données seules ; garde-fou : rien si une struct C++ ou un asset est en cause), depuis 2f479dd | non | 21:42 |
| GLORFINDEL-1119n4 | a72b7ee1 | HeroesOfLiberty-Glorfindel | ability-columns | réparation du générateur de la table des capacités (`StopHp` perdu) ; FINI 21:46 : rouge a0d7c5d / vert b524196, DANS main a4c6a55 | non | 21:44 |
| ELROND-1119n4 | a40fe01c | HeroesOfLiberty-Elrond | faction-gauge-cpp | lot C++ morceau 1 : jauge de faction répliquée (`AHOLGameState`, `UHOLFactionResourceLibrary`), NON COMPILÉ par l'agent | oui | 21:56 |
| CELEBORN-1119n3 | ae38bd63 | HeroesOfLiberty-Celeborn | cheat-specialization | lot C++ morceau 8 : `hol.Cheat.Specialization` (pont RPC, contrôleur, plan de commande), NON COMPILÉ par l'agent | oui | 21:56 |
| (session principale) | - | HeroesOfLiberty-Arwen | integ-24 = main | pré-vols de `specialization_button`, `signals_altclick`, `deploy` sur main (21:57) | non | 21:57 |

Exception écrite (21:55) : il restait 65 min et la machine était calme (8 builds de synchronisation en 3 à 6 s), donc un petit lot C++ part sur deux branches (morceaux 1 et 8 du plan, fichiers disjoints). Il sera fusionné dans UNE branche d'essai, compilé et testé par la session principale dans UN worktree, et NE RENTRERA PAS dans main ce soir : du C++ dans main imposerait 8 recompilations à la synchronisation de fin (L15-1) et ferait dépasser 23:00. Il attend sur sa branche la prochaine séance.

Résultats des vagues 2 et 3 (rapports 21:32-21:40, aucun agent n'a rien exécuté) : ELROND-1119n2 rouge 97a29d8 / vert 7928566 (`Script\AI\HOLAIPowerSpend.as`, `Choose(Rows, CooldownLeftMs, Wanted, Stock, Tier, Reserve)`, 4 tests) ; GALADRIEL-1119n2 rouge 317abf0 / vert 84f36fb (`Script\UI\HOLFactionSlot.as`, 4 tests) ; HALDIR-1119n2 rouge 812d595 / vert f728029 (`Script\Buildings\HOLWallLineRules.as`, 5 tests ; choix à confirmer : glisser hors axe aligné sur la plus proche des 8 directions) ; CELEBORN-1119n2 : `Retours\Q-socle-factions-valeurs.md` (10 valeurs) ; ELROND-1119n3 rouge 147591b / vert ce85278 (2 tests rouges attendus). Intégration (Arwen, integ-24) : rouge de la vague 2 vu 21:33-21:36 = 1800/1813, les 13 attendus et pas un de plus ; vert 21:36-21:39 = 1813/1813 (2f479dd) ; rouge du correctif de la jauge lancé 21:40. Constats du plan du lot C++ (lus dans le code) : `HOLCorpse` n'est ni répliqué ni dans l'empreinte (modèle à suivre : `AHOLGameState::TierOverrides`) ; l'empreinte `HashUnits` ne couvre que les unités ; `DoCheatTier` est à `HOLPlayerController.cpp:3484` ; le processeur Mass ne peut pas appeler le script, donc `IsInZone` / `ZoneActive` passeront en C++ avec le morceau 4 et leur copie en script sera retirée (D839).

Résultats de la vague 1 (rapports 21:24-21:27, aucun agent n'a rien exécuté) : ELROND-1119n1 rouge 026c751 / vert eb8e305 (`Script\Factions\HOLFactionResourceRules.as`, 4 tests ; à revoir : `HasResource` rend faux pour « Legions » seulement, donc vrai pour toute autre faction) ; GALADRIEL-1119n1 rouge 621c050 / vert f4b6130 (`Script\Factions\HOLPowerBookRules.as`, 3 tests) ; GLORFINDEL-1119n1 rouge 48da32b / vert c599a11 (`Script\Units\HOLGroundAbilityRules.as`, 3 tests) ; CELEBORN-1119n1 rouge fe8fbce / vert a9df9f0 (`Script\Duel\HOLDuelRewards.as`, 5 tests dont 3 rouges sur le bouchon ; signature `Orders(S, Duel, RadiusCm)`) ; HALDIR-1119n1 rouge 5d2709f / vert bd5c239 (`OptionCountOf`, `NewStateFor`, `IsValidOptionFor` dans `HOLSpecializationRules.as`, 3 tests dont 2 rouges) ; SAURON-1119n1 : `Retours\Q-hallebardier-valeurs.md` (2 questions, recommandation B puis A) et plan d'A1 `Docs\Rapports\SAURON-1119n1.md`. Glorfindel et Sauron non relancés en vague 2 : la suite de leur tâche (colonnes de `UnitAbilities.csv`, ligne `Halberdier`) touche un asset régénéré par l'éditeur ou attend une réponse.

Plan de lancement (étape 0, 21:20) : Elrond, Galadriel, Glorfindel, Celeborn, Haldir = une règle pure en script chacun (ci-dessus) ; Sauron = docs (question du Hallebardier) ; Arwen = intégration (porte main compilé) ; Faramir NON LANCÉ : branche `fog` à +26 commits propres (WIP D827/D828), aucune tâche du socle des factions n'y tient ce soir. Exceptions : aucun C++ ce soir (pas de build avant 23:00) ; écartés et reportés au lot C++ d'une autre séance : réplication et empreinte de la jauge, commande serveur de ciblage au sol, effet de zone côté Mass, moteur de moral M3-M5 et M7, Aube A2 (redirection de dégâts dans `HOLCombatProcessor`), effet de `hol.Cheat.Specialization` (pont `HOLRpcBridge`), faction du siège dans le tableau de spécialisation + 3e case de grille ; Aube A1 (ligne `Halberdier`) attend la réponse à `Q-hallebardier-valeurs` et une régénération de l'asset `UnitTypes` par l'éditeur.

### Bilan de la séance de fonctions du 2026-11-19 soir (21:08 -> clôture avant 23:00, session principale + 22 agents Sonnet)

**État à la fin.** main = a4c6a55 depuis 21:52 pour le code (LOCAL, RIEN POUSSÉ ; accord de fusion locale donné au lancement), puis e4605c4 à 22:03 (un scénario d'essai corrigé, aucun code) + docs. Tests 1815/1815 sur a4c6a55 (21:49-21:52, Arwen, `quick_check -Parts tests`), contre 1782 au lancement : 33 tests neufs, tous en script, tous vus rouges à l'exécution avant d'être verts. Aucun C++ dans main ce soir. Questions ouvertes pour l'utilisateur : 2 (`Retours\Q-socle-factions-valeurs.md`, `Retours\Q-hallebardier-valeurs.md`). Essais donnés : 3, pré-vols PASS sur main dans Arwen, captures ouvertes et vues, gate Essay sans FAIL (22:00) : `signals_altclick` (2 captures), `deploy` (5 captures), `specialization_button` (3 captures ; il échouait à 21:55 : le scénario relisait la case 10 images après l'annulation, avant la réponse du serveur ; cause lue dans le journal par GALADRIEL-1119n3, scénario corrigé fc10863, pas de régression de la grille en script). Un petit lot C++ (morceaux 1 et 8 du plan) attend sur la branche `integ-25` dans Elrond : voir « Clôture ».

**Ce qui est entré dans main (socle commun des factions, `Docs\Priorite_Socle_Factions_2026-11-16.md`).** Neuf lots, tous des règles ou des données SANS effet en jeu tant que le lot C++ n'est pas fait (rien n'est branché : le joueur ne voit aucune différence ce soir) :
1. Priorité 1 : jauge de ressource de faction (`Script\Factions\HOLFactionResourceRules.as` ; l'Aube seule, faction par `EHOLFaction`), état des boutons du livre de pouvoirs (`Script\Factions\HOLPowerBookRules.as`), vue de l'emplacement de faction partagé avec le bouton de spécialisation (`Script\UI\HOLFactionSlot.as`, D201, D823).
2. Priorité 2 : règle d'une capacité visée au sol (`Script\Units\HOLGroundAbilityRules.as`) et colonnes cible / rayon / portée de `Tools\Data\UnitAbilities.csv` (générateur + table générée + contrôle Python).
3. Priorité 4 : Templiers à 3 options (`OptionCountOf`, `NewStateFor`, `IsValidOptionFor` dans `HOLSpecializationRules.as`).
4. Priorité 5 : récompenses de fin de duel (`Script\Duel\HOLDuelRewards.as`).
5. Priorité 6 : pose en ligne des murs, F4 (`Script\Buildings\HOLWallLineRules.as`).
6. Priorité 7 : dépense des pouvoirs par l'IA (`Script\AI\HOLAIPowerSpend.as`, D688).
Priorité 3 (moteur de moral) : rien à faire en script (M1, M2, M6 et la règle de M8 sont faits), le reste est du C++ Mass.

**Preuves (session principale, un éditeur à la fois, worktree Arwen, branche integ-24).** Vague 1 : rouge 1785/1800 (les 15 attendus, pas un de plus) puis vert 1800/1800. Vague 2 : rouge 1800/1813 (les 13 attendus) puis vert 1813/1813. Correctif de la jauge : rouge 1812/1814 (les 2 attendus) puis vert 1814/1814. Table des capacités : rouge 1814/1815 (le test attendu) puis vert 1815/1815 sur le commit fusionné avec main. Pré-vol de fusion et gate Merge sans FAIL avant l'avance rapide.

**Ce que la séance a trouvé (à garder).**
- **Un défaut attrapé par la première exécution, pas par l'agent** : la table des capacités régénérée (GLORFINDEL-1119n3) avait perdu le champ `StopHp` du struct (remplacé au lieu d'être complété) : tout le module script ne compilait plus (tests 0/0 à 21:42). Rien n'était entré dans main. Réparé par GLORFINDEL-1119n4 (générateur corrigé, contrôle Python « chaque champ affecté est déclaré », rouge sur la table cassée). Leçon : un fichier GÉNÉRÉ modifié par un agent sans exécution se relit en `git diff` par la session principale avant le premier lancement (les `-` d'un diff de table générée sont suspects).
- **Deux règles nommaient la faction différemment** (texte dans la jauge, `EHOLFaction` dans la spécialisation) : vu en comparant deux rapports, corrigé (ELROND-1119n3). Leçon : l'énoncé d'une règle neuve donne le type canonique des entrées communes (faction, siège, case).
- **Le relevé du 2026-11-16 avait deux erreurs** (lues dans le code par GLORFINDEL-1119n2) : `HOLCorpse` n'est ni répliqué ni dans l'empreinte (le modèle d'entier répliqué par emplacement est `AHOLGameState::TierOverrides`) ; l'empreinte `HashUnits` ne couvre que les unités.
- **Le processeur Mass ne peut pas appeler le script** : `IsInZone` et `ZoneActive` de la capacité au sol passeront en C++ avec le morceau 4 du lot, et leur copie en script sera retirée dans le même commit (une seule source, D839) ; `TargetRefusal` reste en script.
- **Cadence** : des agents de 2 à 4 min (7 à 25 appels, 63k à 129k tokens) sur des tâches « une règle pure + ses tests », énoncé de 15 lignes avec les fichiers à lire ; deux commits (bouchon qui compile + tests, puis la règle) ; la session principale fusionne les rouges de toute la vague, lance UNE passe, puis les verts, UNE passe : 2 passes de 150 s par vague.

**Non vérifié.** Aucune règle n'est branchée : pas d'essai à l'écran, pas de partie naturelle, pas de partie `net` rejouée (aucun RPC, aucune réplication, aucune empreinte touchés). Les valeurs des tests sont des valeurs de test (fixtures), pas des valeurs de jeu. Les temps de combat de `Q-hallebardier-valeurs.md` sont calculés à la main. Pose en ligne : l'alignement d'un glisser hors axe sur 8 directions est un choix de l'agent. `HOLAIPowerSpend` : l'ordre de préférence entre pouvoirs est celui du livre (D688 ne donne que des exemples). La branche `ability-columns` garde deux commits intermédiaires qui ne compilent pas (32a8c62, 582fadd) : à ne pas prendre comme point de bissection. `Auto_Journey_File.md` est en LF dans l'arbre de travail depuis le nettoyage (l'index git est en LF de toute façon, `.gitattributes`) ; la reconversion demandée par l'agent de nettoyage après un refus d'autorisation n'a PAS été faite par la session principale.

**Suite.** (1) Réponses de l'utilisateur aux 2 questions. (2) Lot C++ groupé du socle, un build : `Docs\Rapports\LOT-CPP-socle-factions-1119.md` (11 morceaux, 1 à 8 dans le premier build ; parties `net` à rejouer). (3) Puis brancher la vue de l'emplacement de faction dans l'écran de jeu en script, avec un essai de 3 min. (4) Aube A1 (Hallebardier) après la réponse : plan `Docs\Rapports\SAURON-1119n1.md`. Lignes dans `Auto_Journey_File.md`, section « Socle commun des factions ».

### Lecons appliquees 2026-11-19

Séance de fonctions du soir (21:08 -> clôture avant 23:00). Le tableau de la séance de migration du même jour est dans l'archive citée plus haut.

| Regle | Etat | Preuve |
|---|---|---|
| 0a nettoyage | appliquee | 3 agents + la session principale (journal du jour archivé par copie intégrale, 252 Ko), commit d02ae9f à 21:16 avant tout agent de travail ; 12 fiches closes déplacées, rien supprimé |
| T2-1 | appliquee | étape 0 par 2 agents en lecture seule (ETAPE0-1119n-A et -B, 21:14-21:20), plan de lancement écrit sous le tableau des agents, une ligne par worktree avec la raison pour Faramir |
| T2-2 | appliquee en partie | 5 worktrees lancés d'emblée sur 8 ; Sauron en docs, Arwen en intégration, Faramir non lancé (raison écrite) |
| L3-12 | appliquee | étape 0 non sautée |
| L5-7 | appliquee | les 3 docs non suivis de `Docs\` posés à la première question (21:1x) ; réponse : ne pas commiter, `Priorite_Socle_Factions` oriente la séance |
| L4-1, L4-3 | appliquee | fin 23:00 demandée au lancement, écrite dans `Docs\SessionEnd.txt` et le titre du tableau ; alarme 22:45 et veilleur de 30 min créés à 21:13 (CronCreate) |
| L3-8, L4-9, Q5, L1-14 | appliquee | intégrateur nommé au lancement = session principale, worktree Arwen, ligne du tableau |
| L3-9 | non verifie | appels de la session principale non comptés |
| T2-3 | appliquee | chaque énoncé commence par « lis brief_commun.md », arrêt dur donné en heure PowerShell lue juste avant |
| L1-1, L3-6, L4-7, L5-3 | appliquee | 19 agents ; pire cas 25 appels (GLORFINDEL-1119n2 et n3), 129k tokens, 4 min 18 ; aucun au-delà de 15 min ni de 300k |
| L1-2 | appliquee | chaque agent de code : 2 commits en moins de 5 min ; veilleur de 21:35 : aucun agent sans commit |
| L1-3 | sans objet | worktrees synchronisés à 21:16, agents de 2 à 4 min partis de main ou de la branche d'intégration ; `git merge main` fait par la session principale dans integ-24 avant l'avance rapide |
| L3-2, L4-5, L3-5 | sans objet | aucun C++, aucun build dans un agent |
| L1-8 | appliquee | un journal `*InProgress.md` par agent (ligne datée) ; `EtapeDefini.md` et la file tenus par la session principale |
| L1-9, L4-8 | appliquee | aucun numéro D attribué, `GDD.md` non touché ; valeurs non tranchées = fixtures, « valeur par défaut codée : aucune » dans les deux fiches Q |
| L1-7, T2-4 | appliquee | 2 fiches Q (Aube A1 : 2 questions ; socle : 1 tableau de 10 valeurs), chacune avec recommandation et option « Autre » |
| L5-6 | appliquee | `Q-socle-factions-valeurs.md` : un tableau, colonne recommandée, raccourci « OK recommandées partout » |
| T2-5 | sans objet | pas de build ; binaire à jour vérifié (`Get-StaleBinaryProblem` vide sur les 8 worktrees à 21:17 et sur Arwen à 21:25) |
| T2-6, L5-8 | appliquee | le générateur de table réparé a été lancé de bout en bout avec son test Python (3 FAIL sur la table cassée, 0 après) |
| T2-7, L1-5 | appliquee | les agents ont écrit dans `Retours\`, la session principale seule dans `Auto_Journey_Retours.md` |
| L4-12, T2 | appliquee | outil corrigé plutôt que contourné : `make_unit_abilities.py` + contrôle dans `unit_abilities_test.py` |
| L3-7, L1-4 | appliquee | veilleur de 21:35 silencieux (rien d'anormal) |
| L7-5, L15-5, L1-13 | appliquee | 33 tests neufs vus rouges à l'exécution avant le vert : 1785/1800, 1800/1813, 1812/1814, 1814/1815 ; commit du bouchon antérieur au commit de la règle pour chaque lot |
| L12-5 | non verifie | règles pures non branchées : les tests passent par la règle seule, aucune chaîne de jeu (tautologie non recherchée) |
| L15-9 | appliquee | une tâche = une règle + ses tests, fichiers à lire et lignes donnés par l'étape 0 ; aucun agent arrêté sans livrer |
| L15-10 | appliquee | énoncés de 15 lignes renvoyant à `brief_commun.md`, réponses de 6 à 10 lignes, rapports complets dans `Docs\Rapports\` |
| L15-7 | appliquee | l'étape 0 a revérifié le relevé du 2026-11-16 dans main (2 erreurs trouvées ensuite par GLORFINDEL-1119n2) |
| L16-3, L3 / T2 | appliquee | UNE fusion groupée (integ-24), avance rapide de main seulement après 1815/1815 sur le commit exact a4c6a55 ; `preflight_merge.ps1` et gate Merge sans FAIL (21:49) |
| L9-5, L13-3 | appliquee | `commit_docs.ps1` avant chaque gate Launch et avant la fusion |
| L16-5, L11-13 | appliquee | heures et comptes annoncés lus dans la sortie d'une commande du même tour |
| L16-4 | appliquee | messages et questions en français |
| L16-7 | appliquee | sorties PASS / FAIL filtrées par motif, jamais par les N dernières lignes |
| L17-a | appliquee | premier rouge inattendu (0/0, module non compilé, 21:42) LU avant de conclure : défaut du générateur, pas une régression du jeu |
| L17-b | appliquee | archive du journal du jour par script de la session principale (copie intégrale puis réécriture), file nettoyée par section |
| Q1, Q2, Q3 | appliquee | 3 essais donnés : `preflight.ps1` PASS sur main dans Arwen (34 s, 49 s, 37 s), 10 captures ouvertes et vues par la session principale, statut VU À L'ÉCRAN, moins de 3 min chacun |
| Q5, L5-4, L16-2, L8-1 | appliquee en partie | gate Essay sans FAIL (22:00), `Get-StaleBinaryProblem` vide sur Arwen, commandes complètes pointant Arwen = main ; pas de recette par un agent tiers : la session principale a relu chaque capture contre le `.look` |
| Q7 | appliquee | `specialization_button` en échec : cause cherchée dans le journal (ordre des images) avant tout réglage ; scénario corrigé, pas le jeu |
| Q11 | appliquee | 3 essais donnés (plafond ~6) |
| Q4, Q6, Q8, Q9, L4-11 | sans objet | pas d'essai d'IA, pas de réglage à l'œil, pas de défaut commun neuf |
| L10-3 | appliquee | lot C++ : build PASS avant chaque passe de tests dans Elrond (200 s, bouchons ; puis implémentation) |
| L8-9, L15-1 | voir la clôture | `sync_worktrees.ps1 -WaitBuilds` lancé à 21:53 après l'avance de main ; résultat et gate Close : paragraphe « Clôture » ci-dessous |
| L5-bilan | appliquee | ce tableau |
| Autres IDs archives (voir « Devenues des controles » de la check-list) | non verifie | non évalués un par un ; sortie du gate Close au paragraphe « Clôture » |

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
