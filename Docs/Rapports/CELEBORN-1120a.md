# CELEBORN-1120a : D859 n. 8 (sous-étape 1d) et tâche 3a (OrdersOfEnded)

Impact joueur : rien ne change à l'écran aujourd'hui ; les récompenses morale d'un duel (Triomphe, Démoralisé, Hésitation) portent désormais le rayon de 12 m de la table, et le ledger garde les ordres des duels finis en attente du pont C++ (morceau 9).

APPELS_DECLARES: 16 vus par moi (plafond 25)
HEURE_DEBUT/HEURE_FIN: 09:03 / 09:07 (Get-Date PowerShell ; arrêt dur 09:27)
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 7b368ae (Test_DuelOrdersRadiusFromTable, correctif fbe3488) et 5a3b8d4 (Test_OrdersOfEndedCoversEveryEndedDuel, correctif 7fea3dc) ; assertit l'effet : oui (rayon et durées CALCULÉS par Orders / OrdersOfEnded depuis la table, plus les valeurs GDD 1200 / 40000 / 40000 / 25000). Rouge NON VU à l'exécution (aucun build ni quick_check dans l'agent) : tests rouges par construction (stub Orders(S, Duel) rend le rayon 0 et la table valait 0 ; stub OrdersOfEnded rend une liste vide).
COMMITS: 4 (7b368ae, fbe3488, 5a3b8d4, 7fea3dc), dernier à 09:06, merge_main_fait: non (la branche = main 901e391, rien à fusionner)
TESTS: aucun lancé ; TESTS_SCRIPT_AJOUTES: 2 (Test_DuelOrdersRadiusFromTable, Test_OrdersOfEndedCoversEveryEndedDuel) ; TESTS_TAUTOLOGIQUES: 0 ; BUILDS_DEMANDES: 0 ; TYPES_A_EXPOSER: aucun
ESSAIS: aucun (logique seulement)
QUESTIONS: aucune

## Fait
1d : `Tools\Data\Effects.csv` Triumph / Demoralised / Hesitation RadiusCm 0 -> 1200 ; `python Tools\Data\make_effects.py` : `git diff` de `Script\Morale\HOLMoraleTable.as` = 3 lignes `R.RadiusCm = 0 -> 1200`, aucun champ perdu ; `--check` vert. Vérification AVANT : grep `RadiusCm` dans Script, Source (C++) et Tools : aucun C++ ne lit `RadiusCm` d'un effet de Effects.csv ; `HOLMoraleTable_Test.as` ne lit `RadiusCm` qu'aux l. 19 (>= 0, vrai) et 32 (aura seulement, `bAura`) et pour les lignes `Gives` ; les tests C++ / python qui lisent Effects.csv ne regardent que NecroticRage (colonnes 3, 5, 7). Donc le paramètre a été retiré : `HOLDuelRewards::Orders(S, Duel)` et `MakeOrder(Slot, EffectId)` lisent `HOLMoraleTable::Row(EffectId).RadiusCm`. Le test `Test_DuelRewardsRadiusIsTheDuelsOwn` (qui prouvait « le rayon est celui passé ») et les constantes `RewardRadiusA/B` sont retirés, les autres tests de `HOLDuelRewards_Test.as` appellent la forme à 2 paramètres.
3a : `HOLDuelRewards::OrdersOfEnded(S, TArray<FHOLDuelEnded>)` pure (concatène `Orders` de chaque duel fini) ; `UHOLDuelLedger` a un membre `PendingMorale` (TArray<FHOLDuelMoraleOrder>), rempli par la boucle `AdvanceDuels` juste après `HOLDuel::Tick`. L'APPLICATION au moral n'existe pas : elle attend le pont C++ (morceau 9 du lot, `MoraleInProgress.md` M7) ; `PendingMorale` n'a aucun lecteur ni purge pour l'instant (le pont devra le vider).
Journaux : DuelInProgress.md (D7), AubeInProgress.md, Auto_Journey_File.md, EtapeDefini.md (une ligne chacun, ajout). `Placeholders.txt` : rien de provisoire ajouté ou retiré.

## Reste
- Lancer `quick_check -Parts tests` (session principale) : voir les deux rouges sur 7b368ae et 5a3b8d4, vert sur 7fea3dc ; vérifier que `Orders(S, Duel)` à 2 paramètres et `All.Append(TArray)` compilent (Append sur TArray : NON VÉRIFIÉ à la compilation).
- Pont C++ du moral (morceau 9) : lire `PendingMorale`, appliquer, vider.
- Les sondes `Tools\Scenarios\c28_*` / `c31_*` (16 fichiers non suivis) sont laissées telles quelles, non commitées.

## NON VÉRIFIÉ
Compilation Angelscript et exécution des tests (aucun build) ; `TArray::Append(TArray)` ; `IN_USE.txt` écrit `libre` (il n'avait pas de ligne OCCUPE ; fichier ignoré ou non suivi par git, pas commité).

PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: modifiés hors de mon fait : Placeholders.txt, SignalsInProgress.md (autres sessions) ; modifiés par moi : AubeInProgress.md, Auto_Journey_File.md, DuelInProgress.md, EtapeDefini.md ; non suivis docs (Docs\Concept_*, HSR_*, Priorite_*, Rapports\ETAPE0-1120-A/B, GALADRIEL-1120a). Inchangé hors docs voulus : oui.
