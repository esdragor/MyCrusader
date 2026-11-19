# GALADRIEL-1119aa : lot R2b (verdict de mise en file de ApplyOrders), worktree HeroesOfLiberty-Galadriel, branche galadriel-tab

Impact joueur : aucun visible (memes refus, memes lignes de journal, meme ordre des effets) ; l'ordre des refus d'une mise en file est decide par le script. NON COMPILE, aucun build, aucun quick_check, aucun jeu.

## Commits (anglais, sans Co-Authored-By, pas de push, pas de merge)
4359f56 Production: board event for the queue verdict, ApplyOrders asks it with a C++ fallback (R2b, not compiled)
6d4dab4 Script: production board decides the queue verdict, research and unit orders crossed (R2b, not compiled)
4b8c2de Script tests: production board queue verdict, codes 0 to 8 and crossed research / unit order (R2b, not compiled)
fbd42df Merge branch 'main' into galadriel-tab
(tests 4b8c2de, script 6d4dab4, C++ = le dernier ; R3 (issue du ralliement) NON FAITE : budget d'appels atteint)

## Forme
- Script : EHOLQueueVerdict (Enqueue 0, NotProduced 1, QueueFull 2, NotEnoughResources 3, TechFaction 4, TechBlocked 5, SiegeBlocked 6, TierTooLow 7, UnitFaction 8) ; HOLProductionBoard::QueueVerdict(bProduces, bTechnology, bTechFactionOpen, bTechBlocked, bSiegeBlocked, bTierTooLow, bUnitFactionOpen, QueueLength, MaxQueue, Stock, Cost). Recherche : faction (4), bloc de la technologie (5), puis file. Unite : pas produite ici (1), siege (6), age (7), faction (8), puis file pleine (2) AVANT ressources (3). La file et le cout ne sont PAS recopies : HOLProductionRead::TryEnqueue sur une file brouillon de QueueLength elements (les codes 1 a 3 sont ceux de EHOLProductionRefusal).
- Pont : UHOLProductionBoard::AskQueue / QueueAnswer (_Implementation = -1, repli C++ garde) ; classe script : QueueAnswer.
- ApplyOrders : les faits (propose/produit, technologie connue, faction, bloc de HOLTechnology::Check, bloc de HOLSiege::Check, CheckUnit, IsUnitOpenTo) sont lus en C++ puis AskQueue ; -1 = meme chaine en C++ (incl. Queue.Num() >= MaxQueue, Covers). L'effet reste HOLProduction::TryEnqueue (re-verifie), puis SetResources / NotifyProductionChanged. Textes et niveaux des 8 lignes de journal inchanges. AskStockAfterQueue non cree (inutile : l'effet C++ debite).
- Tests (3, dans HOLProductionBoard_Test.as) : Test_ProductionBoard_QueueCodes (codes 0 a 8), _QueueCrossed (unite file pleine ET sans ressources -> 2 ; recherche sans ressources ET file pleine -> 5 ; bord file = max), _QueueOrder (1 avant 6/7/8 ; 4 avant 5 ; 6 avant 7 avant 8 ; cout = stock -> 0 ; 1 or de moins -> 3).

## Rouge a voir (valeur a fausser)
Dans QueueVerdict : permuter TierTooLow et !bUnitFactionOpen (casse QueueOrder, QueueCodes), ou mettre le bloc TechBlocked apres le calcul de la file (casse QueueCrossed). Le >= de la file est la regle C++ (HOLProductionRules.cpp:13), non recopiee : le > s'y fausse cote C++ (le test QueueCrossed bord 5/5 tombe via TryEnqueue).

## Preuves a rejouer (session principale)
1. Un build (lot C++) : Tools\build.ps1 -LiveCoding=0 -Wait dans le worktree Galadriel (verifier Get-StaleBinaryProblem avant).
2. quick_check : 3 tests Test_ProductionBoard_Queue* rouge puis vert ; total = ancien + 3.
3. Scenarios : quick_check.ps1 :763 (Peasant done), tests C++ Production.UnitTier, Production.PopulationCheckedAtEnd, scenarios de recherche (technologie), pc_orders_p1p2_run.ps1.

## Risques de compilation (non compile)
- C++ : FHOLProductionCost Cost; doit etre constructible par defaut ; DescribeRefusal rend un const TCHAR* ; static_cast<EHOLProductionRefusal>(Verdict) suppose None0/NotProducible1/QueueFull2/NotEnough3 (header de la lecture : oui). Variable HOLGameState declaree maintenant une fois dans le bloc Queue (les anciennes declarations internes ont disparu avec le bloc remplace) ; UnitMinTier / TierState supprimes.
- Script : Scratch.StartSteps.Add(-1) (int64), 
"Item".
- Difference d'ordre benigne : les faits (purs : GetCost, Check, CheckUnit, FindEngine) sont lus avant le verdict au lieu d'etre lus a la demande ; aucun effet ne bouge.

## Champs
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 3 (Test_ProductionBoard_QueueCodes, _QueueCrossed, _QueueOrder) ; LIGNES_CPP_SUPPRIMEES: 0 (repli garde, ~86 l. du bloc remplacees par ~110) ; LIGNES_MIGREES_PAR_LOT: ~45 (verdict) ; TYPES_A_EXPOSER: aucun ; TEST_ROUGE: pas vu (non compile) ; EDITS_REFUSES: 1 (ancre trouvee trop tot dans le fichier, commit defait par reset --soft avant re-application, aucune trace) ; PROCESSUS_DETACHES: aucun lance (stray_processes non execute).

## Anomalies / reste
- R3 (issue du ralliement, lignes ~327-379, EarlyRally inverse a fausser) NON FAIT : arret a 25 appels (plusieurs perdus : 2 refus du classificateur sur Bash modifie par un hook, 1 ancre d'edition fausse).
- EtapeDefini.md et Auto_Journey_File.md non touches (consigne).