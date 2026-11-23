# GALADRIEL-1123b : voie = unité (défaut A) : NON CODÉ, unités absentes

APPELS_DECLARES: 14 (arrêt à 45 vus par toi)
HEURE_DEBUT/HEURE_FIN: 09:10 / 09:11 (Get-Date PowerShell ; les horodatages 09:14/09:15 écrits dans les journaux sont des estimations, l'horloge réelle lue en dernier est 09:11)
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (aucun code)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (aucun code écrit) + assertit l'effet : sans objet   TEST_ROUGE_SEPARE: non (aucun code, raison : précondition fausse)
COMMITS: 1 (docs, a915a1f sur socle-ai), merge_main_fait: non
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: `Retours\Q-spec-voies.md` mise à jour (constat de 1 paragraphe) ; réglage `hol.Spec.UnlockUnit` NON créé
FAIT / RESTE:
- Vérifié : `Tools\Data\UnitTypes.csv` contient 18 unités (LightInfantry, LightInfantryNanite, Archer, Peasant, Zombie, Spearman, ManAtArms, DamnedWarrior, King, Hero, Crossbowman, Monk, Ram, Mangonel, LightCavalry, Arquebusier, Halberdier, VirtuousKnight). Aucune des 9 unités de commanderie (Sergent du Temple, Frère infirmier, Arbalétrier à pavois, Sergent à cheval, Prêcheur de croisade, Frère maçon, Maréchal du Temple, Sénéchal, Beffroi) ; aucune unité Templiers ; Grep sur les noms FR : 0 résultat.
- Mécanisme de production (lecture) : refus serveur `HOLRpc::QueueProductionRefusal(Slot, bHasBuilding)` (`Script\Net\HOLRpcPlayer.as:235`, appel l.1292 ; contrôle siège/bâtiment seulement), verdict `EHOLProductionRefusal::NotProducible` (1) dans `HOLProductionRead::TryEnqueue` / `HOLProductionBoard.as:28`. Aucune table de disponibilité par faction ou par voie lue dans ce chemin (la colonne Faction/MinTier du CSV existe ; à confirmer au moment du code).
- Conformément à la consigne : pas de création d'unité, arrêt ; question et journaux mis à jour (Q-spec-voies.md, Auto_Journey_File.md l.45, EtapeDefini.md l.18, SpecializationInProgress.md du worktree, commit a915a1f).
- RESTE : décision de l'utilisateur (créer les unités du palier 1 d'abord = option C, ou autre) ; ensuite namespace `HOLSpecUnlock` + réglage `hol.Spec.UnlockUnit` branché sur le refus serveur (HOLRpcPlayer.as, interdit à cette tâche) ou sur `TryEnqueue`.
NON VÉRIFIÉ: le point d'insertion exact du filtre par faction/voie (non lu : HOLProductionQueue / HOLProductionBoard au-delà des lignes citées) ; aucun test, aucun build, aucun diagnostic Angelscript (aucun .as touché).
PROCESSUS_DETACHES: aucun processus détaché pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
ANOMALIES_PROCESSUS: aucune. Heure : le deuxième Get-Date (09:11) est antérieur au premier (09:10 + durée) à la minute près ; horodatages de journal estimés.
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md, M Auto_Journey_Jour.md, M EtapeDefini.md, M Placeholders.txt + rapports et Retours non suivis d'autres agents (dont Retours/Q-spec-voies.md) ; inchangée hors docs voulus : oui
