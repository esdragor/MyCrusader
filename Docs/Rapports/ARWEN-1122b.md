# ARWEN-1122b : NON COMPILE (opacite de furtivite lue UNE fois par execution du processeur, plus un verrou par unite)

Impact joueur : aucun visible (meme rendu) ; le dessin des unites ne prend plus un FCriticalSection par unite et par pas, et le cas normal (aucune zone) ne cherche plus rien.

APPELS_DECLARES: ~27 vus (arret demande a 25 : depasse de 2 pour journal + rapport ; le 1er Edit est au 14e appel vu, pas avant le 8e : 6 appels de lecture groupes ont ete comptes un a un)
HEURE_DEBUT/HEURE_FIN: 16:36 / ~16:41 (Get-Date PowerShell ; arret dur 16:52 non atteint)
PREMIER_BUILD: sans build (consigne : NE PAS COMPILER)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance
TEST_ROUGE: c69b0e4 (bouchon qui compile + 3 tests, anterieur au correctif 1b1d194) + assertit l'effet : oui (valeurs calculees par la vraie chaine SetStealthOpacity -> TakeOpacitySnapshot -> MakeUnitInstanceCustomData). Rouge ATTENDU, PAS VU a l'execution : bouchon = instantane toujours vide, donc SnapshotInZone et SnapshotStable rouges, SnapshotEmpty vert.
TEST_ROUGE_SEPARE: oui (c69b0e4 bouchon + tests, 1b1d194 correctif ; le classifieur n'a rien refuse)
COMMITS: 2 (c69b0e4, 1b1d194), dernier a 16:38, merge_main_fait: non (non demande ; integ-37 a deja fusionne main en 8b1a6d5)
TESTS: aucun lance. 3 tests C++ neufs dans Source\HeroesOfLiberty\Tests\HOLUnitOpacityDrawTests.cpp : HeroesOfLiberty.Presentation.UnitDrawOpacity.{SnapshotInZone,SnapshotEmpty,SnapshotStable} ; les 3 existants {InZone,Outside,Layout} restent inchanges (ancienne surcharge MakeUnitInstanceCustomData a 3 arguments, conservee, un verrou par appel, pour appels isoles et tests). TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lot C++ de la session principale, meme build que 1122a)   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun

## ORDRE D'INTEGRATION (shas, rouge puis vert, dans l'ordre de la branche integ-37)
1. 09868d9 ROUGE (1122a : bouchon 5e float + tests InZone/Layout)
2. b620434 VERT (1122a : vraie valeur)
3. c69b0e4 ROUGE (1122b : API FOpacitySnapshot, TakeOpacitySnapshot bouchon vide + SnapshotInZone / SnapshotStable rouges)
4. 1b1d194 VERT (1122b : copie unique de la table + processeur)
(8b1a6d5 = merge main, facffa5 = docs 1122a, entre b620434 et c69b0e4.) Verifier le rouge : poser le worktree sur c69b0e4 ; le vert : 1b1d194.

## FAIT
- HOLUnitOpacity.h : classe publique HOLUnitDraw::FOpacitySnapshot (table TMap<int32,float> privee, IsEmpty(), Get(UnitId) : 1.0 sans recherche si vide) ; TakeOpacitySnapshot() ; surcharge MakeUnitInstanceCustomData(Color, Prop, UnitId, const FOpacitySnapshot&). Les ecritures (SetStealthOpacity / ClearStealthOpacities) et GetStealthOpacity gardent leur API et leur verrou.
- HOLUnitOpacity.cpp : TakeOpacitySnapshot = UN FScopeLock, rien copie si la table est vide, sinon copie de la petite table (seules les unites < 1 y sont).
- HOLInterpolatedInstancesProcessor.cpp : `const FOpacitySnapshot OpacitySnapshot = TakeOpacitySnapshot();` avant EntityQuery.ForEachEntityChunk (l.~396), capture par reference dans le lambda, site unites (l.~584) passe le snapshot. Site corps inchange (1.0 fixe, aucune recherche).

## SECURITE DE LA COPIE (lu)
- HOLInterpolatedInstancesProcessor.cpp:268 `bRequiresGameThreadExecution = true` ; ExecutionFlags Client | Standalone (l.266). La requete utilise ForEachEntityChunk (l.394), sequentiel, pas ParallelForEachEntityChunk : le lambda tourne sur le thread de jeu, un seul a la fois. Le snapshot est une copie par valeur privee a cet appel : aucune donnee partagee apres le verrou, donc sur meme si un jour le processeur passait en parallele (lecture seule de l'objet const).
- Les ecritures script (game thread) se font entre deux Execute ou, au plus, pendant un Execute depuis un autre thread : le verrou de TakeOpacitySnapshot les serialise, l'instantane pris n'est plus modifie (test SnapshotStable).

## IDENTITE DES IDS (grep, demande par 1122b)
- Cote processeur : `State.UnitId` (FHOLUnitStateFragment, tableau `States` l.408 ; champ `uint32 UnitId = 0;` HOLSimulationFragments.h:410, a confirmer comme appartenant a FHOLUnitStateFragment), converti `static_cast<int32>(State.UnitId)` au site unites (HOLInterpolatedInstancesProcessor.cpp l.~584).
- Cote script : Script\Fog\HOLStealthOpacityApply.as:9 `ApplyOwnUnit(Zones, int UnitId, Position)` -> l.11 `HOLUnitOpacity::SetStealthOpacity(UnitId, ...)` ; l.14 `ApplyOwnUnits(Zones, TArray<int> UnitIds, Positions)`.
- RESULTAT : AUCUN appelant de production. Le seul appel d'`ApplyOwnUnits` est Script\Fog\HOLUnitOpacity_Test.as:46 (test, ids 7/8...). L'identite State.UnitId == id passe au setter NE PEUT PAS etre confirmee : la fonction n'est branchee nulle part (grep `StealthApply::|SetStealthOpacity(` sur Script\ : 5 lignes, toutes dans HOLStealthOpacityApply.as et HOLUnitOpacity_Test.as). A faire par le chantier qui branche la furtivite : appeler ApplyOwnUnits avec l'id de simulation de l'unite (celui de FHOLUnitStateFragment::UnitId, uint32 -> int), pas un index de tableau ni un handle Mass.

## RELECTURE_CPP (L20-2)
Relu apres edition : `#include "HOLUnitOpacity.generated.h"` reste le DERNIER include de HOLUnitOpacity.h (l.7, avant l'UCLASS) ; FOpacitySnapshot : constructeur, IsEmpty, Get publics (section `public:` avant `private: Table`) ; TakeOpacitySnapshot exportee (HEROESOFLIBERTY_API) ; HOLUnitOpacity.cpp inclut deja Misc/ScopeLock.h ; bouchon retire (commentaire STUB remplace, plus de `return FOpacitySnapshot();` inconditionnel) ; la surcharge a 4 arguments n'est pas ambigue avec celle a 3. Non compile : risques residuels = copie de TMap en `FOpacitySnapshot(TMap<int32,float>(Table))` (constructeur explicite par rvalue, attendu OK), lambda capturant `&OpacitySnapshot` (const, vivant plus longtemps que la requete synchrone).

## NON VERIFIE
Compilation ; les 6 tests (rouge attendu 2/3 des neufs sur c69b0e4) ; que FHOLUnitStateFragment est bien la structure de HOLSimulationFragments.h:410 ; identite des ids (aucun appelant) ; cout reel mesure (non mesure : seulement raisonne : un verrou par execution au lieu de un par unite).
A lancer (session principale) : Tools\build.ps1 -LiveCoding=0 -Wait puis quick_check -Parts tests (filtre HeroesOfLiberty.Presentation.UnitDrawOpacity).

PROCESSUS_DETACHES: aucun (aucun processus lance)
ANOMALIES_PROCESSUS: depassement du budget d'appels (27 pour 25) ; deux commandes PowerShell en erreur de syntaxe (Get-ChildItem -Filter avec deux motifs, chemin Mass\ inexistant), sans effet.
GIT_STATUS_PRINCIPALE: code inchange : oui (j'ecris seulement dans Docs\Rapports\ARWEN-1122b.md et mes lignes de suivi)
