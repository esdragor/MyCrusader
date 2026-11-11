# Archive du nettoyage de Auto_Journey.md et Auto_Journey_Checklist.md du 2026-11-11 (bloc 4)

Rien n'est supprimé : tout ce qui est sorti de `Auto_Journey.md` est ici, tel quel. Motif : leçons déjà reprises en contrôles mécaniques (`Auto_Journey_Checklist.md`, `Tools\AutoJourney\session_gate.ps1`, `Tools\AutoJourney\agent_rules.md`) ; lignes corrigées parce qu'une règle datée plus récente les contredit.

## A. Section « Leçons de la séance interactive du 2026-11-05 » (lignes 181-206, verbatim)
Reprise : check-list avant lancement = Checklist L5-1, L5-2, L3-1 (GATE `Launch`) ; plafond de tokens = L1-1, L7-1, L9-2 et `agent_rules.md` règle 2 ; effet mesuré par un geste réel = Q5, L5-4, `agent_rules.md` règles 7 et 19 ; questions et réponses = L5-6, L5-7 ; cartes et mesures = L3-11, L5-5, L5-8, `agent_rules.md` règle 13.

## Leçons de la séance interactive du 2026-11-05 (10:30 -> close à 11:20 sur demande, fusion 11:50 ; approuvées par l'utilisateur, toutes OBLIGATOIRES)

**Constat de l'utilisateur : des leçons reviennent de séance en séance sans être appliquées (plafond de tokens : écrit le 2026-11-01, 11-03 et 11-04, dépassé à 250-336k le 11-05 par 4 agents).** Cause : une règle en prose relue au démarrage ne se déclenche pas toute seule. **Règle de méthode : une leçon n'est « intégrée » que si elle devient un contrôle mécanique (un nombre qu'un agent peut compter, une commande à lancer, une ligne à cocher), jamais une phrase.** Les leçons ci-dessous sont écrites ainsi ; à chaque fin de séance le bilan liste, pour chaque leçon, « appliquée / non appliquée » avec la preuve (nombre d'appels, heure lue).

### Check-list avant CHAQUE lancement ou envoi (la session principale la déroule, une ligne par lancement dans `Auto_Journey_Jour.md`)
1. `Get-Date` PowerShell lu juste avant ; heure d'arrêt dur de l'agent = calculée depuis cette lecture ; **aucun lancement s'il reste moins de 5 minutes avant la fin** (règle de durée du 2026-11-05 PM ; l'ancien seuil de 20 min laissait la séance vide) (2026-11-05 : agent lancé à 13:19 avec un arrêt à 13:15).
2. **Tableau agent -> identifiant -> worktree -> branche -> tâche** écrit dans `Auto_Journey_Jour.md` AU LANCEMENT ; tout `SendMessage` relit ce tableau (2026-11-05 : 3 consignes envoyées au mauvais agent).
3. Nombre d'agents C++ en cours <= 3 (`Docs\BuildQueue.state\` + tableau).
4. Pas d'agent « noop » ni de test d'outil par un agent : un outil se teste avec la commande elle-même.

### Plafond de tokens = plafond d'appels d'outils
- Dans l'énoncé de chaque agent : le modèle d'énoncé arrête à 45 appels vus par l'agent ; le seuil de 80 se juge sur `tool_uses`. Mesure du 2026-11-05 : ~2k tokens par appel (96 appels = 242k, 133 = 284k, 150 = 250k, 185 = 336k).
- À la réception du rapport, la session principale **lit `tool_uses` de la notification** : au-delà de 100, l'agent est jugé hors règle, noté dans le bilan, et la tâche suivante du même worktree part sur un agent NEUF (jamais un `SendMessage` qui reprend l'agent).
- Un agent qui revient « waiting on its own background work » n'est pas relancé : il revient seul à la fin du build (2026-11-05 : 6 notifications intermédiaires, aucun doublon lancé).

### Une correction n'est « faite » que si l'effet est mesuré par un geste réel
- **Un test qui assertit un prédicat ou une ligne de journal ne prouve rien** (2026-11-05 : `IsPassiveAdversary` testé, Q+clic sur neutre qui ne fait rien en jeu ; l'utilisateur l'a trouvé). Le test rouge d'un REGLAGE/BUG assertit l'**effet** (PV de la cible qui baissent, ressource reçue, bouton actif) sur la vraie chaîne, déclenché par la même entrée que l'utilisateur (touche puis clic).
- **Tout essai d'un REGLAGE/BUG passe par l'intégrateur** (autre agent que l'auteur) sur le build de `main` fusionné, geste réel + PNG avant/après, AVANT que la commande `play.ps1` soit donnée à l'utilisateur. Sans cela l'essai est `LOGIQUE SEULEMENT`. La commande donnée à l'utilisateur pointe le worktree qui porte le build fusionné.

### Questions et réponses
- Dès le premier envoi : **un tableau unique** (question, option recommandée, ce que ça change), et le raccourci « `recommandées partout` » accepté.
- Demander **dès le lancement** l'accord pour commiter les dossiers de fonctionnalités non suivis de la copie principale (ils bloquent la fusion).

### Cartes et mesures
- La régénération des 7 cartes x 5 tailles est **une tâche à part** (éditeur seul, build vide, mesure de perf), jamais mêlée à un réglage de budget ; un changement de budget de cartes ne se fusionne qu'après régénération complète, build et `tests,sizes`.
- Un `-Parts net` qui échoue sans changement de code (2026-11-05) est relancé deux fois avant d'être cru ; vérifier qu'aucun `UnrealEditor` d'un autre projet (AS_3 / ProjectShiva) ne tourne.

## B. « Pièges techniques », dernière puce (ligne 74, verbatim)
Reprise : plafond d'appels = `agent_rules.md` règle 2 ; pas de `play.ps1` fenêtre = `agent_rules.md` règle 0 ; binaire de fusion = Checklist L10-3 ; PNG des pré-vols = Q5.

- Plafond d'appels : 45 vus par l'agent respecté à ±4 sauf 3 agents (115, 81, 55) ; l'énoncé d'un agent ne demande JAMAIS `play.ps1` fenêtre ; un agent de fusion « sans C++ » vérifie la date du binaire ; les PNG des pré-vols de fusion sont ouverts par l'agent d'intégration avant de donner un essai.

## C. Lignes corrigées (version d'AVANT, verbatim ; la version la plus récente est `agent_rules.md` règles 3, 4, 17, 18)

Ligne 14 :
- **Archives** : `Docs/Archive/Auto_Journey_Lecons_jusqu_au_2026-11-09.md`, `Docs/Archive/Auto_Journey_Sections_historiques_2026-11-10.md`, `Docs/Archive/Auto_Journey_Test1_2026-10-30.md`.

Ligne 39 :
- Chaque agent : met à jour `EtapeDefini.md` (relire juste avant, ne toucher que ses lignes : aucun conflit observé) , le journal de son chantier, `Auto_Journey_File.md` (sa ligne) et sa ligne du tableau des agents (`Auto_Journey_Jour.md`) à chaque avancée, `IN_USE.txt` à l'entrée et à la sortie (fichier non suivi par git), commite sur la branche de son worktree (anglais, sans `Co-Authored-By`, jamais de push).

Ligne 43 :
- Tout ce qui attend l'utilisateur est dans **`Auto_Journey_Retours.md`** (1 essais, 2 questions, 3 commits et fusions). Les agents y écrivent, jamais ici.

Ligne 44 :
- **`Auto_Journey_Retours.md` est léger et sans historique** : dès qu'une réponse de l'utilisateur est reportée dans son chantier (`*InProgress.md`, GDD, file de travail), l'item est **supprimé** du fichier ; idem un essai fait et reporté. Avant de supprimer, vérifier que la réponse est bien écrite dans le chantier (sinon l'y écrire d'abord). Le compteur en tête est recalculé à chaque modification.

Ligne 68 :
- L'horloge de Git Bash retarde de 5 minutes sur PowerShell. Les chemins `\` sont mangés dans Bash (12 minutes perdues) : **PowerShell pour tout chemin Windows**.

Ligne 114 :
Ne tranche aucun design : pose-le dans Auto_Journey_Retours.md section 2 en VRAIE question (contexte 2 lignes, options A/B/C

Ligne 117 :
Un essai n'est listé dans Auto_Journey_Retours.md section 1 que si Tools\Scenarios\preflight.ps1 <nom> passe (scénario + <nom>.look, un PNG ouvert et vu par point, <= 3 min, waitkey pour toute attente) : sinon écris-le « LOGIQUE SEULEMENT » et ne le compte pas prêt. Question de réglage rapide : construis aussi <nom>_compare (toutes les options à l'écran, Entrée = option suivante) au lieu de choisir à sa place.

Ligne 119 :
Tiens à jour EtapeDefini.md (relis avant, tes lignes seulement), ton *InProgress.md, Auto_Journey_File.md (ta ligne), ta ligne du tableau

Ligne 120 :
des agents de Auto_Journey_Jour.md, IN_USE.txt à l'entrée et à la sortie. PowerShell pour tout chemin Windows.

Ligne 161 :
Leçons des séances du 2026-11-01, 2026-11-03 et 2026-11-04 : voir `Auto_Journey_Checklist.md` (archive : `Docs/Archive/Auto_Journey_Lecons_jusqu_au_2026-11-09.md`). Celles du 2026-11-05 restent ci-dessous.
