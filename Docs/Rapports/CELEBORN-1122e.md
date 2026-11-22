# CELEBORN-1122e : F8 T1, portée de tir depuis le bord du bâtiment. NON COMPILÉ

Joueur : rien ne change encore en jeu (règle pure, non branchée) ; elle servira à la Tour de guet en bois pour viser depuis le bord de son corps.
Technique : `HOLCombat::IsInBuildingShotRange` inline dans `HOLCombatRules.h`, un test C++ neuf, aucun branchement (T2).

APPELS_DECLARES: 13 (arrêt à 25 vus ; le seuil se juge sur `tool_uses` de la session principale)
HEURE_DEBUT/HEURE_FIN: 17:41:23 / 17:43:56 (Get-Date)
PREMIER_BUILD: sans build (consigne : NE COMPILE PAS)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: non lancé (pas de build, consigne)
TEST_ROUGE: 548cf14 (bouchon `return false` + test `HeroesOfLiberty.Combat.BuildingShotRange`, même commit), rouge NON VU à l'exécution (aucun build) ; assertit l'effet : non (règle pure : l'effet est la valeur rendue par la règle ; le branchement est T2)   TEST_ROUGE_SEPARE: oui (548cf14 rouge, 73d7571 vert, b986b94 mise en page)
COMMITS: 3 sur integ-41 (548cf14, 73d7571, b986b94), dernier à 17:43 ; merge_main_fait: non (consigne : pas de fusion ; branche prête non compilée)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0 (la limite de portée est trouvée par un balayage de X dont la sortie est comparée à bord + rayon + portée ; le bord depuis le centre à 3300 prouve que le bord est utilisé)
ESSAIS: aucun (règle pure)
QUESTIONS: aucune

## Fait
- `Source\HeroesOfLiberty\Simulation\HOLCombatRules.h:116-132` : `inline bool IsInBuildingShotRange(const FBox2D& Body, const FVector2D& UnitLocation, float UnitRadius, float Range)`. Pince la position de l'unité dans la boîte, compare `DistSquared` au carré de `Max(UnitRadius,0) + Range` (pas de racine), 2D (hauteur ignorée), `Range <= 0` = jamais, unité dans le corps = distance 0 = à portée. Pas de `.cpp` (inline, appelée par cible et par pas, comme `IsInRange`). Aucun jumeau script.
- `Source\HeroesOfLiberty\Tests\HOLBuildingShotRangeTests.cpp` (neuf) : un test `HeroesOfLiberty.Combat.BuildingShotRange` : pile 3000 au bord (centre à 3440), 1 cm dedans (3439), 1 cm dehors (3441), côté gauche, centre à 3300 (à portée par le bord), balayage de la limite, coin en diagonale (2400/2400 dedans, 2600/2600 dehors), cible au centre / dans le corps / sur le mur, portée 0 et négative = jamais.

## RELECTURE_CPP (L20-2)
Relu après écriture : `FBox2D` (UE5 : `Min`/`Max` en double, `FBox2D(Min, Max)` existe), `FMath::Clamp` sur doubles, `FMath::Max(float,float)`, `FVector2D::DistSquared`, `TestEqual` double/double, include `Math/Box2D.h` + `CoreMinimal.h` (via `HOLCombatRules.h`) ; fins de ligne CRLF conservées, pas de BOM ajouté. Le bouchon utilise `(void)` sur les 4 paramètres (pas d'avertissement de paramètre inutilisé). Pas d'autre `IsInBuildingShotRange` dans le dépôt (nom neuf, non vérifié par grep exhaustif : NON VÉRIFIÉ).

## Reste
- Build (`Tools\build.ps1 -LiveCoding=0`) puis `quick_check -Parts tests` : rouge attendu sur 548cf14 (le test échoue, la règle rend toujours false), vert sur 73d7571.
- T2 (première flèche dans `UHOLCombatProcessor::Execute`, utilise cette règle), T3 à T8 : voir `Docs\Rapports\CELEBORN-1122d.md`.

## NON VÉRIFIÉ
Compilation, tous les tests, le rouge et le vert à l'exécution ; l'inclusion de `Math/Box2D.h` ; l'exactitude des deux valeurs de coin (calculées à la main : 2828 et 3111 contre une portée effective de 3040).

PROCESSUS_DETACHES: aucun (stray_processes.ps1 -Root HeroesOfLiberty-Celeborn -Stop : « aucun processus detache »)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: inchangée hors docs voulus : oui (seuls des docs et Retours\ modifiés, dont FortificationsInProgress.md, EtapeDefini.md, Auto_Journey_File.md pour ma ligne)
