# GLORFINDEL-1122e : FindAllyAt (clic droit sur un allie : Soigner et Suivre enfin atteints) - NON COMPILE

Impact joueur : un clic droit sur un allie (ou un Paysan a soi) declenchait ni Soigner (Moine) ni Suivre : l'unite marchait au sol. Apres le lot C++, ces deux branches sont atteintes par un vrai clic.

APPELS_DECLARES: 18 vus (arret a 25 ; premier Edit/ecriture au 9e appel, consigne "avant le 8e" non tenue d'un appel : un chemin faux a coute un appel)
HEURE_DEBUT/HEURE_FIN: 17:04 / 17:07 (Get-Date PowerShell ; l'horloge de la machine avance lentement entre appels)
PREMIER_BUILD: sans build (NON COMPILE)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (CPP: oui dans IN_USE.txt, mais aucun build demande a l'agent)
TEST_ROUGE: 2e8af0b (stub PickAllyAt qui renvoie INDEX_NONE + test HeroesOfLiberty.Movement.AllyPicking) ; assertit l'effet : oui (l'indice de l'allie retenu est CALCULE par la fonction) ; rouge NON VU a l'execution (non compile) ; scenario de bout en bout : follow_short, deja rouge.
TEST_ROUGE_SEPARE: oui (2e8af0b rouge, 6494b40 vert)
COMMITS: 2 (2e8af0b, 6494b40, 17:06), merge_main_fait: non
TESTS: aucun lance (NON COMPILE) ; commande a lancer apres le build : `Tools\build.ps1 -LiveCoding=0 -Wait` puis `quick_check -Parts tests` (attendu : HeroesOfLiberty.Movement.AllyPicking rouge au commit 2e8af0b, vert a 6494b40) puis `play.ps1 follow_short -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -TimeoutSec 150` ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (follow_short non rejoue : pas de binaire)
QUESTIONS: aucune (voir choix pris ci-dessous)
BUILDS_DEMANDES: 1 (lot C++)   TESTS_SCRIPT_AJOUTES: 0 (test C++ HOLAllyPickingTests.cpp, pur)   TYPES_A_EXPOSER: aucun
DIAGNOSTICS_AS: sans objet (aucun .as touche)

## Ce qui change (Source\HeroesOfLiberty)
- Player\HOLUnitPicking.h/.cpp : struct `FHOLAllyCandidate { Pick, UnitId, bHostile, bNeutral }` et `HOLUnitPicking::PickAllyAt(Candidates, Excluded, Cursor)` : filtre (ni hostile, ni neutre, ni dans Excluded) puis `PickAt` (corps touche DIRECTEMENT, le plus proche de la camera, aucune marge) ; renvoie l'indice dans Candidates ou INDEX_NONE.
- Player\HOLPlayerController.h:122-127 / .cpp:1192-1218 : `FindAllyAt(ViewportFraction, Excluded)` : GatherPickCandidates (tous camps), lit la vue de chaque candidat (HOLUnitView::Find) pour calculer hostile / neutre, appelle PickAllyAt. Illisible = compte hostile (jamais retenu).
- HOLPlayerController.cpp:1464-1501 (OrderAt) : apres l'attaque (inchangee, `FindTargetAt` inchange, l.3031 aussi), `AllyId = (!Source && !EnemyBuilding) ? FindAllyAt(.., aucune exclusion) : 0` pour Soigner (sa garde "un soigneur dans la selection" est gardee) ; `FollowId = (!Source && !EnemyBuilding) ? FindAllyAt(.., Own) : 0` pour Suivre (NativeRightClickOnUnit, true). Un clic au sol : aucun candidat touche, donc deplacement comme avant.
- Tests\HOLAllyPickingTests.cpp (neuf) : allie touche, ennemi devant l'allie ecarte (l'allie derriere est pris), ennemi seul, neutre, deux allies superposes (celui de devant), 11 px a cote d'un corps (rien, pas de marge), selection ecartee (l'allie derriere est pris), seulement des unites de la selection (rien), aucun candidat. Les coordonnees des cas ennemi seul (112, 200) et 11 px (515, 190) ont ete corrigees dans le commit vert (le commit rouge avait un point hors du corps : Y 210 > 209).

## RELECTURE_CPP (L20-2, relu avec le diff, non compile)
- `Own` est un `TArray<uint32>` : converti en `TConstArrayView<uint32>` a l'appel ; `TConstArrayView<uint32>()` pour "aucune exclusion" ; `Excluded.Contains` existe sur la vue.
- `Under` (FHOLUnitView) est declare l.1427 avant les branches : reutilise par Soigner et Suivre. `HOLPlayerState` n'est plus lu dans les branches (FindAllyAt le teste) : pas de variable inutilisee neuve.
- `UnderId` et `bDirect` restent utilises par la branche d'attaque (inchangee). Les deux journaux `Order: ... heal / follow #%u` emploient AllyId / FollowId (types uint32, memes formats).
- FHOLAllyCandidate est declaree dans HOLUnitPicking.h (deja inclus par le controleur et par le test). Le test n'inclut que Misc/AutomationTest.h et Player/HOLUnitPicking.h ; son flag est `EditorContext | EngineFilter` (copie de Movement::Flags, pas de dependance au namespace des autres tests).
- Fichiers : UTF-8 sans BOM conserve, fins de ligne CRLF conservees (diff -U0 : seules les lignes voulues, aucune ligne 1).
- Non vu : la compilation (HOLUnitView::Find, FHOLCampTable::AreHostile, NeutralOwner deja employes plus haut dans le meme fichier avec les memes formes).

## Comportements de clic changes pour le joueur
1. Clic droit sur un allie (ou une unite a soi hors selection) avec un Moine selectionne : ordre Soigner (avant : deplacement au sol).
2. Clic droit sur un allie ou une unite a soi hors selection, sans Moine : Suivre (pont script), avant : deplacement. ATTENTION : un clic droit "au sol" qui tombe SUR le corps d'une unite a soi (arret, groupe voisin) devient un Suivre au lieu d'un deplacement ; sans marge (corps touche seulement), mais une cible de rassemblement posee derriere une unite a soi est desormais un Suivre.
3. Une unite de la selection sous le curseur est ecartee pour Suivre : l'allie derriere elle est pris s'il est sous le curseur, sinon deplacement (comportement de "pas de suivi de soi").
4. Une source (arbre, mine) ou un batiment ennemi sous le curseur : l'ordre habituel (recolte / attaque batiment) garde la priorite, aucun Soigner / Suivre. (Choix lu a la lettre de l'enonce ; l'attaque d'un ennemi garde sa priorite et sa marge.)
5. Un Moine selectionne seul qui clique sur lui-meme : Soigner (Excluded vide pour Soigner) ; non verifie que RequestHealUnit le traite sans effet de bord.
Aucun changement : attaque d'un ennemi, clic au sol, batiments, FindTargetAt (l.3031 compris).

## Le correctif de Soigner peut-il s'extraire seul sur main ?
Oui. Le correctif ne touche que HOLUnitPicking.h/.cpp, HOLPlayerController.h/.cpp et le test neuf ; les deux branches Soigner / Suivre y sont deja ecrites. Sur main, la branche Soigner (Q-chapelle-moine = A) existe (l.1437 equivalent), la branche Suivre (D818, UHOLFollowBridge) vient d'ability-columns : sur main seul, il faudrait ne prendre que la partie Soigner (AllyId + le test) et laisser FollowId pour la fusion d'ability-columns. Cherry-pick de 2e8af0b + 6494b40 : conflit probable sur le bloc Suivre (absent de main).

## TEST_ROUGE_SEPARE / classifieur
Deux commits passes sans refus du classifieur. Le rouge n'a PAS ete vu a l'execution (aucun build) : a verifier a la reception (quick_check -Parts tests sur 2e8af0b, puis 6494b40).

## Attentes touchees (L22-3)
quick_check.ps1 : les occurrences de heal / follow (l.451-557, T6 reseau du Moine) passent par la triche reelle et le pont, pas par OrderAt sur une unite : non modifie, non revu une a une. Scenarios : follow_short.txt:18 (attend `units follow`) est le scenario rouge de bout en bout ; talents_heal.txt ne matche pas "units heal". RISQUE NON VERIFIE : les scenarios dont un OrderAt tombe sur le corps d'une unite a soi (foules, formations : building_crowd, formation_*, melee_reinforce, j7_army, blocked_bodies...) peuvent maintenant journaliser `units follow` au lieu de `move` : a rejouer apres le build.

## Docs
EtapeDefini.md (ligne apres "Suivre, pont (GLORFINDEL-1122d"), Auto_Journey_File.md (sous la ligne [ ] FindAllyAt, laissee ouverte : build et follow_short a rejouer), OrdersPatrolInProgress.md (ligne du 2026-11-22). Placeholders.txt : aucun provisoire ajoute ni retire. IN_USE.txt de Glorfindel : libre.

NON VERIFIE : compilation ; rouge et vert du test ; follow_short (suivi vu, annulation par ordre manuel, cible mobile) ; heal reel en clic ; effets de bord sur les scenarios de foule ; Soigner sur soi-meme ; main seul (cherry-pick).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: modifs de docs seulement (EtapeDefini.md, Auto_Journey_File.md, OrdersPatrolInProgress.md, ce rapport, autres sessions) : oui
