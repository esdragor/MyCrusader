NON COMPILÉ : aucun build, aucun test lancé (C++ écrit dans le worktree, à compiler par la session principale).

# SAURON-1120f : Aube A2, sous-étape 1 (règle pure de redirection de dégâts)

Joueur : rien de visible encore ; c'est la règle qui dira, quand le combat l'appellera, qui encaisse quelle part des coups (le Chevalier Vertueux prend une part des dégâts des alliés proches).

APPELS_DECLARES: 22 (arrêt à 25)
HEURE_DEBUT/HEURE_FIN: 10:30:13 / 10:34 (arrêt dur 10:48)
PREMIER_BUILD: sans build (C++ NON COMPILÉ)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: sans build
TEST_ROUGE: d2762d8 (stub qui ne redirige rien + test) ; assertit l'effet : oui (PV gardés par l'allié, part du Chevalier, conservation Kept + parts = dégâts) ; rouge pas encore vu à l'exécution.
COMMITS: 2 (d2762d8 test + stub, a1fa9dc correctif), dernier à 10:32, merge_main_fait: non (merge integ-26 fait en premier geste)
TESTS: aucun lancé ; test neuf C++ `HeroesOfLiberty.DamageRedirect.Rules` (Source\HeroesOfLiberty\Tests\HOLDamageRedirectTests.cpp) ; TESTS_TAUTOLOGIQUES: 0 (chiffres à la main, triangle 3-4-5 pour le bord)
BUILDS_DEMANDES: 1 (lot C++ groupé : Tools\build.ps1 -LiveCoding=0 -Wait dans le worktree Sauron, puis quick_check -Parts tests ; attendu : rouge sur d2762d8, vert sur a1fa9dc)
TESTS_SCRIPT_AJOUTES: 0 (règle lue par le processeur Mass = C++, D839 une seule source ; test de la règle en C++ car elle vit dans le module de simulation)
TYPES_A_EXPOSER: aucun (pas de liaison script ; le script n'a pas besoin de la règle)
ESSAIS: aucun
QUESTIONS: Retours\Q-aube-a2-cumul.md (défaut codé : HOLDamageRedirect::DefaultCumulMode = Nearest, recommandation A), Retours\Q-aube-a2-valeurs.md (part du passif et rayon : paramètres, pas de défaut codé en dur ; recommandation 30 % et 8 m)

## Ce que A2 demande exactement (AubeInProgress.md:79-87, GDD D19 lignes 1121-1128)
Chevalier Vertueux (Caserne, palier 1, catégories HeavyInfantry + Heavy, Faction Dawn, ligne VirtuousKnight d'UnitTypes.csv + Produces de la Caserne) ; passif : une part des dégâts subis par les alliés proches est redirigée vers lui ; héros exclus (ni protégés ni provoqués) ; part et rayon = valeurs de table (nouveaux champs de FHOLUnitTypeConstants, tous UPROPERTY pour l'empreinte des constantes, HOLUnitSubsystem.cpp:486-491) ; redirection dans HOLCombatProcessor à côté de l'Aura de l'Aube (:638-730) ; déterminisme et empreinte ; anneau au sol à la sélection (HOLSelectionMarkers) ; test rouge d'abord (allié à portée reçoit moins, Chevalier la part, somme conservée, hors rayon aucun effet, héros ni protégé ni redirigé, deux Chevaliers ne redirigent pas deux fois, deux simulations identiques, grille de la Caserne au palier 1 seulement).

## Déjà tranché
Héros exclus (D19) ; Défi ACTIF : 100 %, 8 s, 8 m, recharge 45 s (D859) ; le passif existe (D19 « une partie »).
## Non tranché (donc paramètres, jamais choisis)
Part du passif, rayon du passif, cumul de plusieurs Chevaliers (explicitement « à demander » en A2). Le mode par défaut Nearest est un réglage d'une ligne.

## Fait
`Source\HeroesOfLiberty\Simulation\HOLDamageRedirectRules.h/.cpp` : `HOLDamageRedirect::Redirect(Damage, bAllyIsHero, AllyId, AllyXCm, AllyYCm, Knights, RadiusCm, PercentPerKnight, ECumulMode, CapPercent)` -> `FResult{Kept, Shares}`. Cas : héros allié, dégâts <= 0, pourcent 0 : rien ; Chevalier mort ou frappé lui-même : ignoré ; bord du rayon inclus (réutilise HOLZoneEffect::IsInZone, une seule source de la géométrie) ; pourcent borné 0..100 (100 = version active) ; Nearest : le plus proche seul (égalité : plus petit Id) ; CappedSum : somme plafonnée à CapPercent, partagée à égalité.
## Reste (agent neuf)
(2) champs RedirectPercent / RedirectRadiusCm dans FHOLUnitTypeConstants + colonnes + ligne VirtuousKnight ; (3) appel de Redirect dans la chaîne de dégâts de HOLCombatProcessor + test Mass d'effet ; (4) anneau HOLSelectionMarkers ; rappel Placeholders.txt (maillage du Chevalier) à la ligne de données.

NON VÉRIFIÉ: la compilation de tout (TInlineAllocator dans une struct retournée, `Shares.Add({Id, Damage})` en aggregate init, TConstArrayView sur un tableau C `FKnight[]`) ; le rouge à l'exécution ; EOL des 3 fichiers neufs (écrits LF, git les normalise en CRLF).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
ANOMALIES_PROCESSUS: aucune ; Auto_Journey_File.md : -After 'FARAMIR-1120c' refusé (ligne seulement dans EtapeDefini.md), relancé avec 'Aube A1 Hallebardier' (ligne 33) ; horloge Get-Date : 10:34 en fin.
GIT_STATUS_PRINCIPALE: inchangée hors docs voulus : oui (docs de suivi modifiés, Q-aube-a2-cumul.md et Q-aube-a2-valeurs.md ajoutés ; les autres ?? sont d'autres agents)
