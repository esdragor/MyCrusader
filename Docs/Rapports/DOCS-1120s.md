# DOCS-1120s : enregistrement des réponses aux 5 essais (2026-11-20 ~13:05), docs seulement

APPELS_DECLARES: ~18 (arrêt dur 13:30, tenu)
HEURE_DEBUT/HEURE_FIN: 13:12 / 13:15 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (aucun code touché)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (docs seulement) + assertit l'effet : sans objet
COMMITS: 3, dernier à 13:14 (seuil 30 min), merge_main_fait: sans objet (copie principale, pas de worktree)
TESTS: aucun ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun créé ; 5 fiches répondues et archivées
QUESTIONS: aucune

## Commits (sur main, locaux, rien poussé, fichiers par nom)
- 3712ca9 : 5 fiches `E-*.md` (ligne `Reponse:` remplie) déplacées par `git mv` vers `Docs\Archive\Retours_2026-11-20\` + `Retours\ESSAIS-A-JOUER.md` et `Retours\LISEZ-MOI.md` (ces deux listes ne citaient pas les 5 essais ; une ligne de mise à jour ajoutée à chacune).
- e2ca9e8 : `GDD.md` seul, D869 et D870 ajoutés après D868, « Prochain numéro » passé à D871. `git diff --numstat GDD.md` avant commit : 5 ajouts, 1 retrait (mes lignes seulement).
- 59973f0 : journaux (`Palier3InProgress.md` G6, `SignalsInProgress.md` H2/H3, `SpecializationInProgress.md`, `AubeInProgress.md`), `Auto_Journey_File.md` (2 lignes `[ ]` en tête de « Socle commun des factions »), `Auto_Journey_Retours.md` (en-tête ligne 3), `EtapeDefini.md` (une ligne après la ligne « SÉANCE DE FONCTIONS »).

## Réponses enregistrées
1. E-deploy : BUG/REGLAGE, « le démontage est instantané, comme le montage ; ça doit prendre du temps (1-2 s à chaque fois) » = D869.
2. E-signals_altclick : REGLAGE, taille ÷ 3, pas de nouvel essai = D870.
3. E-signals_minimap_alt : REGLAGE, même chose = D870.
4. E-specialization_cheat : OK.
5. E-faction_slot : OK.

## Ce qui est en cause pour E-deploy (lecture, rien corrigé)
`Source\HeroesOfLiberty\Siege\HOLDeployRules.h:76-77` : `ProposedDeployMs = 8000`, `ProposedPackMs = 5000` ; `HOLDeployRules.cpp:34-35` les met dans le profil (`HOLDeployRead::DefaultProfile`), `HOLDeployRules.cpp:69,74` les raccourcit selon le Contrepoids. Donc la règle a bien des durées (8 s / 5 s, plus longues que les 1-2 s voulus) : l'instantané vu n'est pas une durée de règle trop courte. Deux pistes, non départagées : (a) l'essai court-circuite la durée (scénario / ordre interne), (b) aucun signe visible du changement (la case D change seulement de libellé, pas d'animation ni de barre). La fiche E-deploy disait « 5 s de démontage ». La ligne de file D869 demande d'établir la cause, de ramener chaque durée à 1-2 s et de rendre le changement visible, puis un rejeu par l'intégrateur (règle 19).

## FAIT / RESTE
- Fait : (a) à (f) de l'énoncé. Reste : les deux lignes de file D869 et D870 pour la prochaine séance de fonctions (D870 : `HOLSignalMarks` / `CVarSignalGroundMarkSize`, sans nouvel essai).

NON VÉRIFIÉ: les noms `HOLSignalMarks` / `CVarSignalGroundMarkSize` viennent de l'énoncé, pas relus dans le code ; l'en-tête de `Auto_Journey_Retours.md` (ligne 3) a été remplacé en entier par PowerShell sur contrôle du début de ligne (pas par Edit), l'ancien texte reste dans l'historique git (991a8bc / 868e2a7). `Placeholders.txt` non touché (aucun provisoire ajouté ou retiré).
PROCESSUS_DETACHES: aucun (aucun processus lancé)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: `?? Docs/Concept_Faction_Theatre_2026-11-18.md`, `?? Docs/HSR_Aha_Allegresse_Nihilux.md`, `?? Docs/Priorite_Socle_Factions_2026-11-16.md` (les 3 non suivis d'avant ma séance) ; inchangée hors docs voulus : oui
