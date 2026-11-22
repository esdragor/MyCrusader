# GLORFINDEL-1122a : pont de Suivre = INSTANCE (NON COMPILÉ)

APPELS_DECLARES: 24 (arrêt à 45 vus par moi ; limite du brief 25) ; le premier Edit/Write est venu au 12e appel (consigne « avant le 8e » non tenue : 7 appels de lecture)
HEURE_DEBUT/HEURE_FIN: 16:31 / 16:34 (Get-Date PowerShell ; la dernière heure vient d'un Get-Date, le `date` de Git Bash a affiché 16:33)
PREMIER_BUILD: sans build (NON COMPILÉ, consigne)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: non lancé (aucun build dans cet agent)
TEST_ROUGE: 6aee2a7 (bouchon GetFor = Get() donc le CDO + test HeroesOfLiberty.Follow.BridgeIsAnInstanceOfTheController), rouge NON VU à l'exécution (pas de build) ; assertit l'effet : oui (un pont par contrôleur, stable d'un appel à l'autre, propriétaire = son contrôleur ; avec le CDO : TestFalse RF_ClassDefaultObject, GetOuter et « autre contrôleur = autre pont » échouent)   TEST_ROUGE_SEPARE: oui (commit seul, pas de refus)
COMMITS: 2 (6aee2a7, cf625af), dernier 16:34, merge_main_fait: non (non demandé ; la branche contient déjà 8841054 merge de main)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0 (valeurs calculées par GetFor sur de vrais contrôleurs spawnés)
ESSAIS: aucun
QUESTIONS: aucune
FAIT / RESTE:
- Commit 1 6aee2a7 : Source\HeroesOfLiberty\Player\HOLFollowBridge.h/.cpp (déclaration + bouchon `GetFor` rend `Get()`), Tests\HOLFollowBridgeTests.cpp NEUF.
- Commit 2 cf625af : `Get()` SUPPRIMÉ ; `FindScriptClass()` (espace anonyme, cache + CLASS_NewerVersionExists) ; `GetFor` = `NewObject<UHOLFollowBridge>(Controller, ScriptClass)` gardé dans `AHOLPlayerController::FollowBridge` (UPROPERTY(Transient), section `public:`, déclaration avancée `class UHOLFollowBridge;`), remplacé si la classe a changé (rechargement à chaud) ; NativeRightClickOnUnit/NativeStep/NativeManualOrder passent par GetFor ; `NativeManualOrder(AHOLPlayerController*, TConstArrayView<uint32>)` ; les 7 sites du contrôleur passent `this`.
- Retrait de Script\Orders\HOLFollowBridge_Test.as et HOLFollowBridgeRequests_Test.as (suivis dans la branche, appelaient `UHOLFollowBridge::Get()` supprimé : erreur de compilation AS ; ce sont les fichiers qu'integ-47 a retirés, ad9fc7e).
- RESTE : build par la session principale, rouge vu sur 6aee2a7 puis vert sur cf625af ; scénario court follow_short (n°2) ; `HOLFollowBridge.as` utilise `UHOLFollowBookHolder::Get()` (autre classe, non traitée).
- Pas de journal *InProgress.md propre au chantier Suivre : lignes ajoutées dans EtapeDefini.md (l. 34) et Auto_Journey_File.md (l. 138) seulement.
NON VÉRIFIÉ: tout (aucune compilation, aucun test). Le test suppose qu'une classe script dérivée existe au lancement (Script\Orders\HOLFollowBridge.as, jamais compilé : « NOT COMPILED » dit son en-tête) ; sinon TestNotNull échoue pour une autre cause que le CDO. Les `.generated.h` / UHT : UPROPERTY après `public:` dans une classe à `GENERATED_BODY()` (déjà présent).
RELECTURE_CPP (L20-2) : `.generated.h` reste le dernier include du .h du pont (inchangé) et du contrôleur (inchangé) ; accès public du membre FollowBridge (GetFor y écrit depuis une statique du pont) ; aucun `case` ajouté ; bouchon retiré au commit 2 (plus de `Get()`, grep `UHOLFollowBridge::Get` : aucun reste dans Source ni Script) ; aucun paramètre modifié n'est const (`AHOLPlayerController* Controller` non const, écrit via Controller->FollowBridge). Les autres ponts (HOLWallLineBridge.cpp:23, HOLGameMenuRulesBridge.cpp:23) renvoient encore le CDO : hors périmètre.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
ANOMALIES_PROCESSUS: aucune. Le hook rtk a refusé une fois un Bash (relancé à l'identique, passé).
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md, M Auto_Journey_Jour.md (pas moi), M EtapeDefini.md, ?? 4 docs non suivis + Docs/Rapports/SAURON-1122a.md (pas moi) : docs voulus seulement : oui
Commande à lancer par la session principale : `Tools\build.ps1 -LiveCoding=0` dans C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel puis le test `HeroesOfLiberty.Follow.BridgeIsAnInstanceOfTheController` (rouge attendu sur 6aee2a7, vert sur cf625af).
