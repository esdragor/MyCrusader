# SAURON-1122a : neutrals_clairiere (tâche fermée sans rejeu)

APPELS_DECLARES: 6 (arrêt à 25 vus)
HEURE_DEBUT/HEURE_FIN: 2026-11-22 16:31:55 / 16:32:44
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (sans objet)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (aucun code) ; assertit l'effet : sans objet   TEST_ROUGE_SEPARE: sans objet
COMMITS: 1 (350223e, 16:32, NeutralsInProgress.md sur integ-48), merge_main_fait: non (inutile, integ-48 = main + 1 doc)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: neutrals_clairiere : préflight NON relancé volontairement. L'essai a déjà été joué et validé par l'utilisateur (voir preuve). Aucune fiche créée. Commande de l'essai déjà jouée : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 neutrals_clairiere -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
QUESTIONS: aucune

## Preuve (L11-11, L22-4)
- `Docs\Archive\Retours_2026-11-20\E-neutrals_clairiere.md` (nom avec UNDERSCORE, d'où le « fiche absente » du plan ETAPE0-1122 qui cherchait `E-neutrals-clairiere.md` avec tiret) : STATUT VU À L'ÉCRAN, GESTE interne, SON non entendu, branches: main, préflight PASS le 2026-11-20 16:49 (RECETTE-1120pa, 3 PNG ouverts et décrits : avant = bosquet d'une quinzaine d'arbres cachant les 3 Lanciers gris ; large = bosquet disparu, Lanciers à découvert, un buisson à la limite des 25 m ; camp = gros plan dégagé) ; dernière ligne : `Reponse: OK` (réponse de l'utilisateur).
- `NeutralsInProgress.md` (worktree Sauron) l. 517 : « 2026-11-20 17:2x : essai neutrals_clairiere joué OK par l'utilisateur (Reponse: OK, fiche archivée ...) ».
- `Auto_Journey_Jour.md` l. 59 : « clairière des neutres sans bosquet (VU À L'ÉCRAN, essai joué OK) ». `Auto_Journey_Jour_2026-11-21_soir_archive_2026-11-22.md` l. 24 : SAURON-1121sa avait déjà conclu « déjà joué OK ; rien sans C++ ».
- Le scénario `neutrals_clairiere.txt/.look/.json` n'a pas changé depuis (main = 1447448, a3c3ce5 dedans).

## FAIT / RESTE
- Fait : fermeture de la ligne n°2 de `Auto_Journey_File.md` (l. 15), ligne `SAURON-1120g` d'`EtapeDefini.md` (l. 354), ligne du journal `NeutralsInProgress.md` (commit 350223e).
- Reste : corriger le plan `Docs\Rapports\ETAPE0-1122.md` (§1 Sauron n°1, §3 l. 119, §4 l. 130 : « fiche absente » est faux, elle est archivée sous le nom `E-neutrals_clairiere.md`) ; ce fichier n'est pas à moi, signalé seulement.
- Lacunes connues de l'essai joué (déjà dans la fiche archivée) : GESTE interne, SON non entendu, buisson isolé à la limite du rayon non confirmé. Un essai à geste réel resterait à faire si on veut un statut GESTE: réel.

NON VÉRIFIÉ: aucun PNG rouvert ni préflight relancé dans cette séance (non nécessaire : l'utilisateur a répondu OK sur le rendu réel) ; l'état du binaire de Sauron pour ce scénario non testé.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
ANOMALIES_PROCESSUS: aucune ; anomalie documentaire : le plan ETAPE0-1122 cherchait un nom de fiche avec tiret (L22-4 : confronter les noms de fichier à `Docs\Archive\Retours_*` avant de lister une tâche)
GIT_STATUS_PRINCIPALE: ` M Auto_Journey_File.md`, ` M EtapeDefini.md` (mes lignes), 4 non suivis Docs préexistants (Concept_Faction_Theatre, HSR_Aha, Priorite_Socle, Rapports\ETAPE0-1122) : oui, inchangée hors docs voulus
