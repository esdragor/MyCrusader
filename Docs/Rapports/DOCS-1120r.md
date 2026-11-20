# DOCS-1120r (2026-11-20, 12:40 -> 12:43, docs seulement, aucun code)

APPELS_DECLARES: 7 (arrêt à 45 vus)
HEURE_DEBUT/HEURE_FIN: 12:40:59 / ~12:43 (arrêt dur 12:55 respecté)
PREMIER_BUILD: sans C++   BUILD: sans objet   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (docs seulement) + assertit l'effet : sans objet
COMMITS: 1 (3667f5b, 12:42), merge_main_fait: non (sans objet, copie principale)
TESTS: aucun ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune créée ; 7 fiches `Retours\Q-*.md` répondues puis déplacées (git mv) dans `Docs\Archive\Retours_2026-11-20\` (aucune supprimée)

FAIT:
- GDD.md : D862 (Sceau sans Honneur, capacité du Moine), D863 (Brigands gardent le Lancier), D864 (A2 le plus proche seul), D865 (A2 passif 30 % / 8 m), D866 (moral : Q1 à Q5 = A), D867 (murs sans plafond, chaque case = fantôme à construire), D868 (3e case Templiers en 5-6-7) ; prochain numéro : D869. `git diff GDD.md` ne montrait que ces lignes : commité.
- Chaque fiche porte `Reponse: ...` (Q-brigands-lancier : remplacement de « (à remplir) »).
- Journaux : AubeInProgress, MoraleInProgress, FortificationsInProgress, SpecializationInProgress, NeutralsInProgress : une ligne datée chacun.
- `Auto_Journey_File.md` : 5 lignes `[ ]` en tête de « Socle commun des factions » (D862 Sceau, D867 murs, D868 3e case, D864+D865 A2 table, D866 moral M3).
- `EtapeDefini.md` : une ligne (après la ligne RÉPONSES du 2026-11-19).

RESTE (file, prochaine séance, aucun code lancé) : Sceau via capacités du Moine ; cvar `hol.Wall.MaxLineCells` sans plafond + vérifier fondations dans `ServerPlaceWallLine` ; `GetSpecializationCell` 5-6-7 + libellé 3e voie (lot C++) ; colonnes de table A2 ; moteur de moral M3 C++.
NON VÉRIFIÉ: aucun code ni test touché ; l'affirmation « ServerPlaceWallLine pose des fondations » vient de HALDIR-1120i, non relue dans le code.
PROCESSUS_DETACHES: aucun (aucun processus lancé)
ANOMALIES_PROCESSUS: aucune. Note : `git add` a embarqué les modifications non commitées d'autres sessions dans EtapeDefini.md, Auto_Journey_File.md, FortificationsInProgress.md (docs de suivi communs) ; AlertsMinorInProgress.md et ChatInProgress.md non commités (pas les miens).
GIT_STATUS_PRINCIPALE: après commit, reste ` M AlertsMinorInProgress.md`, ` M ChatInProgress.md` et 6 `??` (Docs\Concept_Faction_Theatre, HSR_Aha, Priorite_Socle, Rapports ELROND-1120h / GLORFINDEL-1120i / HALDIR-1120j) ; inchangé hors docs voulus : oui
