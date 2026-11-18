# HALDIR-1118a : lot R partie C (HOLPlayerController.cpp), NON COMPILÉ

APPELS_DECLARES: ~17 (arrêt à 25 vus)
HEURE_DEBUT/HEURE_FIN: 13:42 / 13:47
PREMIER_BUILD: sans C++ (aucun build, consigne)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: sans C++ (aucun build)
TEST_ROUGE: sans objet (retrait de code, pas de test ; la porte no-cpp-rpc du lot T est le rouge) ; assertit l'effet : non
COMMITS: 1 (092592d sur maps, 13:46), merge_main_fait: non
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0

ESSAIS: aucun
QUESTIONS: aucune

FAIT: points a, b, c, d, e appliqués par Saved\lotr_c.ps1 (non commité), motifs regex exacts avec refus si le compte diffère : b 16, a 16, c 16, e 16, d1 1 (OrderScriptCVarName / OrderScriptOn / OrderSend / OrderRecv du namespace), d2 1 (4 lignes de déclaration en tête). CRLF et UTF-8 sans BOM gardés (7163 lignes, 7163 CR, 0 LF nu).
Lignes : 7449 -> 7163 (delta -286, attendu -286). Accolades : 1348/1348 -> 1295/1295.
CONTROLE FINAL : grep `OrderScript|OrderTrace|OrderSend|OrderRecv` = 0. Grep `Server(<16 noms>)\b` = 19 occurrences au départ de la vérification : 16 sont les chaînes `TEXT("Server<X>")` passées à CallFull dans les Route<X> (noms des fonctions du script RPC actor : DOIVENT rester) + 3 commentaires « As ServerMoveUnits/ServerAttackUnit (D288) » dans DoMoveUnits / DoAttackUnit / DoMoveUnitsInLine, reformulés en « As the Move order / AttackUnit order ». Reste donc 0 hors les 16 littéraux de nom script (à adapter dans le critère de la porte no-cpp-rpc : elle ne cherche que `UFUNCTION(Server...) void <Nom>(` dans le .h, donc pas concernée).
Route<X> : 16 déclarées (l.~71-86), 16 définies, 16 appelées par les Request (grep `Route<X>(` = 48 = 16 x 3). Les 16 Do<X> (16 définitions `::Do<X>(`) intacts. IsOn, LogNotSent, CallFull, ToIds, CallEx/CallN/Call gardés. Les 16 CallFull passent `nullptr` comme CVarName.
Relu : RequestMoveUnits, RequestPlaceBuilding (OnOrderGiven + journal « Placement » gardés avant l'appel Route), RequestSetRallyPointOf, DoMoveUnits : conformes.
RESTE : rien pour le point C. Hors périmètre (autre agent / autres lots) : le .h (CELEBORN-1118a), HOLHealProcessor.h:46 (commentaire « ServerHealUnit »), AS, scénarios, porte no_cpp_rpc_gate.

NON VÉRIFIÉ: la compilation (aucun build) ; en particulier que le .h ne déclare plus les 16 `Server<X>` (sinon erreur de lien : _Implementation absente) ; que `Builders` (TArray<uint32>) se convertit bien en TConstArrayView dans RoutePlaceBuilding (inchangé depuis la branche OrderScriptOn, donc compilé avant) ; aucun test lancé.
PROCESSUS_DETACHES: aucun (aucun processus lancé)
ANOMALIES_PROCESSUS: aucune. Les numéros de ligne de la carte valaient aussi pour 0bff683 (aucun décalage constaté). Git a averti « LF will be replaced by CRLF » (autocrlf), sans effet sur le diff (1 fichier, +35 / -321 : les +35 viennent des lignes Route réécrites en LF puis normalisées, mêmes lignes).
GIT_STATUS_PRINCIPALE: non relevé (aucune écriture hors worktree Haldir sauf ce rapport) : oui
IN_USE.txt : entrée 13:42 (CPP: oui), sortie « libre : ... ».
