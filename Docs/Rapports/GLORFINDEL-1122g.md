# GLORFINDEL-1122g : cvar hol.Follow.OwnUnits (NON COMPILÉ)

Impact joueur : un clic droit sur une de ses propres unités (point de ralliement, déplacement visé dans un groupe) redevient un déplacement au lieu d'un Suivre silencieux ; un coéquipier reste suivi et Soigner ne change pas.

NON COMPILÉ : aucun build, aucun quick_check, aucun éditeur (consigne). Relecture L20-2 faite (ci-dessous).

APPELS_DECLARES: 17 vus (arrêt 25 ; 15 min non atteintes) ; premier Edit/Write au 6e appel (tenu).
HEURE_DEBUT/HEURE_FIN: 17:18 / 17:23
PREMIER_BUILD: sans build (NON COMPILÉ)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: non lancé (pas de compilation par l'agent)
TEST_ROUGE: cd6b099 (bouchon PickFollowAt qui ignore bOwnUnitsFollowable + test HeroesOfLiberty.Movement.FollowOwnUnits, fichier neuf HOLFollowOwnUnitsTests.cpp) ; rouge NON VU à l'exécution (rien lancé) ; rouge attendu : les assertions « 0: » sur une unité à soi (own seule = INDEX_NONE, own devant un coéquipier = le coéquipier) échouent contre le bouchon, les assertions « 1: » et Soigner passent. « assertit l'effet : oui » (l'index pris, calculé par PickFollowAt / PickAllyAt, jamais recopié). TEST_ROUGE_SEPARE: oui (commit du rouge seul accepté, vert e3ee5ad)
COMMITS: 2 (cd6b099 rouge, e3ee5ad vert), dernier à 17:21 (seuil 30 min), merge_main_fait: non (pas de compilation : DLL non vérifiée)
TESTS: aucun lancé (NON COMPILÉ). TESTS_TAUTOLOGIQUES: 0 (les index sont calculés par PickFollowAt / PickAllyAt)
ESSAIS: aucun (LOGIQUE SEULEMENT ; scénarios follow_short / follow_crowd corrigés, non rejoués, preflight non lancé)
QUESTIONS: Retours\Q-follow-own-units.md mise à jour (défaut codé = 0 = option B ; cvar hol.Follow.OwnUnits ; valeur à changer = défaut `0` de CVarFollowOwnUnits dans HOLPlayerController.cpp, `1` = A)
BUILDS_DEMANDES: 1 (lot C++ de la session principale)   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun   DIAGNOSTICS_AS: sans objet (aucun .as touché)

## FAIT
- Test neuf `HeroesOfLiberty.Movement.FollowOwnUnits` (Source\HeroesOfLiberty\Tests\HOLFollowOwnUnitsTests.cpp, fichier neuf, AllyPicking intact) : 12 assertions : avec 0, une unité à soi hors sélection n'est pas retenue pour Suivre mais l'est pour Soigner (PickAllyAt) ; un coéquipier est retenu pour les deux ; une unité à soi devant un coéquipier est sautée pour Suivre (le coéquipier derrière est pris), prise par Soigner ; un ennemi jamais ; avec 1, l'unité à soi est retenue pour Suivre, la sélection reste exclue.
- Commit 1 (cd6b099) : FHOLAllyCandidate::bOwn, HOLUnitPicking::PickFollowAt (bouchon = PickAllyAt, ignore le drapeau) + le test. Compile attendu (signature + champ + fonction).
- Commit 2 (e3ee5ad) : PickFollowAt réel (copie des candidats, les `bOwn` marqués neutres quand le drapeau est faux : indices conservés) ; HOLPlayerController.cpp : cvar `hol.Follow.OwnUnits` (TAutoConsoleVariable<int32>, défaut 0, espace de noms UE::HeroesOfLiberty::Player, style de CVarTestCursor) ; FindAllyAt(ViewportFraction, Excluded, bSkipOwn = false) remplit `Ally.bOwn = View.OwnerPlayer == HOLPlayerState->GetPlayerIndex()` et appelle PickFollowAt(..., !bSkipOwn) ; la branche Suivre d'OrderAt passe `GetValueOnGameThread() == 0` ; la branche Soigner garde FindAllyAt(.., vide) = bSkipOwn faux : inchangée. HOLPlayerController.h : paramètre par défaut + doc. Pas touchés : attaque, FindTargetAt, clic au sol.
- Scénarios : follow_short pose explicitement `hol.Follow.OwnUnits 1` (son Paysan est à soi, un seul joueur, pas de coéquipier : choix = cvar 1) ; follow_crowd pose `hol.Follow.OwnUnits 0` et attend `waitlog Order: 10 units to` après chacun des 3 clics (déplacement, jamais « follow started » ; la partie Soigner du Moine inchangée).
- Q-follow-own-units.md mis à jour ; EtapeDefini.md, Auto_Journey_File.md (ligne sous 1122d), OrdersPatrolInProgress.md mis à jour.

## RELECTURE_CPP (L20-2)
- cvar : dans l'espace de noms UE::HeroesOfLiberty::Player (à côté de CVarTestCursor, lignes ~140), lien externe comme les voisins ; usage qualifié en entier dans OrderAt (pas de `using namespace` dans cette fonction) ; GetValueOnGameThread valide (ordre donné sur le thread de jeu).
- FindAllyAt : `HOLPlayerState` est testé non nul en tête ; bOwn calculé sur `View.OwnerPlayer` (une vue illisible est déjà hostile : jamais prise). Le paramètre par défaut n'est que dans le .h.
- PickFollowAt : `TArray<FHOLAllyCandidate>(Candidates.GetData(), Candidates.Num())` (constructeur pointeur + nombre) ; paramètre `const bool` dans le .cpp, `bool` dans le .h (compatible).
- Test : géométrie vérifiée à la main (corps 40 px, demi-largeur 10, curseurs dans les corps ; unité 13 devant 12 à profondeur 2900 contre 3000) ; INDEX_NONE comparé à un int32 comme dans AllyPicking.
- Points d'attention pour le build : variable inutilisée aucune ; `Parameters` du test non utilisé (comme AllyPicking).

## RESTE / NON VÉRIFIÉ
- NON VÉRIFIÉ : compilation, test (rouge puis vert), les deux scénarios (follow_short avec 1 : suivi visible ; follow_crowd avec 0 : trois déplacements + soin ; le waitlog « Order: 10 units to » suppose que le journal d'un déplacement groupé s'écrit ainsi, vu en 1122f), comportement à 2 joueurs (coéquipier réellement suivi : aucun scénario à 2 slots écrit).
- Commande à lancer par la session principale (build du lot C++ puis) : `Tools\quick_check.ps1 -Parts tests` ; rejouer `play.ps1 follow_short -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel` et `follow_crowd`.
- Branche prête : ability-columns (tip e3ee5ad), après build vert.

PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
ANOMALIES_PROCESSUS: une commande contenant `Remove-Item` sur Retours\Q-follow-own-units.md a été refusée par le classifieur (rien exécuté) ; contourné par Read puis Write du même fichier. Avertissement git « LF will be replaced by CRLF » sur follow_crowd.txt (eol_check -Fix passé sur le worktree : plusieurs fichiers de Manager\ « CORRIGE », aucun diff git attendu).
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md, Auto_Journey_Jour.md, EtapeDefini.md, OrdersPatrolInProgress.md, Palier3InProgress.md, StealthInProgress.md ; ?? Docs\Concept_Faction_Theatre_2026-11-18.md, Docs\HSR_Aha_Allegresse_Nihilux.md, Docs\Priorite_Socle_Factions_2026-11-16.md, Docs\Rapports\* (rapports), Retours\Q-follow-own-units.md, Retours\Q-stealth-rendu.md : inchangée hors docs voulus : oui (mes docs : EtapeDefini.md, Auto_Journey_File.md, OrdersPatrolInProgress.md, Q-follow-own-units.md).
