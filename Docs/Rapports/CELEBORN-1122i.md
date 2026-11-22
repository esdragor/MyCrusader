# CELEBORN-1122i : F8 T5 (vue du proprietaire) + T5b (garde Combat.Enabled 0). NON COMPILE, NON LANCE

Joueur : une tour ne tire plus sur ce que son proprietaire ne voit pas (brouillard), et combat coupe = aucune fleche de batiment.
Technique : `ReleaseBuildingArrows` recoit la vision de `Execute` ; `BuildingCooldownMs.Reset()` dans le bloc `!bFighting`.

APPELS_DECLARES: ~20 (arret 25)
HEURE_DEBUT/HEURE_FIN: 18:05 / 18:09 (Get-Date)
PREMIER_BUILD: sans build (consigne : ne rien lancer)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (build en cours sur le worktree)
TEST_ROUGE: 6fdbaea (tests HeroesOfLiberty.Combat.BuildingShotFog et BuildingShotGuard + bouchon = parametre Vision ignore), rouge NON VU a l'execution ; assertit l'effet : oui (fleches lancees, PV perdus, comptes a rebours tenus)
TEST_ROUGE_SEPARE: PARTIEL. Le rouge precede bien chaque correctif (6fdbaea avant 7d9870b et 5d3b09f), mais les DEUX paires (T5 et T5b) partagent UN seul commit rouge, pas deux (erreur de ma part : tests ecrits ensemble).
COMMITS: 4, dernier 18:09 ; merge_main_fait: non (consigne : rien d'autre)
  - 9fa0842 Fix: HOLCombatRules.h includes what it uses (demande du coordinateur, commit separe)
  - 6fdbaea red : bouchon + 2 tests
  - 7d9870b green T5 : `|| (Vision && !Vision->IsVisible(Step.OwnerPlayer, Candidate.Location))` dans le filtre des candidats
  - 5d3b09f green T5b : `BuildingCooldownMs.Reset();` dans `if (!bFighting)` de Execute
TESTS: aucun lance. TESTS_TAUTOLOGIQUES: 0 (les valeurs viennent de la vraie chaine : processeur de combat, vision reelle, compteur de fleches, PV de la cible, TMap du processeur).

## Fix includes (demande du coordinateur)
`HOLCombatRules.h` : remplace `struct FHOLUnitTypeConstants;` + `enum class EHOLAttackType : uint8;` par `#include "Simulation/HOLUnitTypes.h"` (declare EHOLAttackType l.79, EHOLArmorCategory l.20, FHOLUnitTypeConstants et HasArmorCategory ; `.generated.h` y est inclus en dernier). GetBuildingShotDamage (inline, l.52-58) utilise les trois.
Includes verifies par grep dans HOLCombatRules.h : FBox2D / FVector2D / FMath / int32 (CoreMinimal.h, deja present) ; FHOLUnitTypeConstants, EHOLAttackType, EHOLArmorCategory (HOLUnitTypes.h, ajoute) ; AHOLBuilding, FHOLCombatFragment, UHOLFullSpeedProcessor : uniquement dans des commentaires (l.276, 451, 742), rien a inclure. Les tests de T1/T3 qui n'incluent que HOLCombatRules.h compilent donc.
Autres includes : HOLCombatProcessor.cpp inclut deja Map/HOLVisionSubsystem.h (l.16) ; le .h declare `const class UHOLVisionSubsystem*` ; HOLBuildingShotTests.cpp ajoute Map/HOLFogMode.h, Map/HOLVisionSubsystem.h, Match/HOLMatchConfig.h (comme HOLCampStateTests.cpp).

## T5
Meme test que `IsSeenBy` (Execute l.954) : `Vision` est nullptr si le brouillard n'est pas demarre ou en mode Disabled (l.950-953), donc "absence de brouillard" = tout vu. Filtrage par pas : une cible qui sort de vue n'est plus candidate. Test `BuildingShotFog` : tour SightRadius 6000 (tire, le pipeline vit), 0 (aveugle : 0 fleche, 0 PV perdu), 0 + fog Disabled (tire), 0 + fog jamais demarre (tire), SightRadius 1500 : cible proche tiree, cible a 2500 (a portee, hors vue) non tiree.
## T5b
Le bloc `!bFighting` de Execute retourne avant `ReleaseBuildingArrows` : aucun tir deja, mais les comptes a rebours restaient (fuite si le batiment meurt combat coupe). Maintenant Reset (au retour du combat chaque batiment est "rencontre" a neuf : 1re fleche aussitot). Test `BuildingShotGuard` : Enabled 0 : 0 fleche, 0 PV, 0 entree ; coupe en fin de fenetre puis tours detruites : 0 entree (rouge attendu sur ce point).

RELECTURE_CPP (L20-2, relu a l'ecran) : seule la signature de `ReleaseBuildingArrows` change (2 appels : definition l.839 et Execute l.1505) ; `Vision` du call site est `const UHOLVisionSubsystem*` comme le parametre ; `Candidate.Location` est la meme expression que dans IsSeenBy ; `CombatEnabledVar` du test est declare dans le meme bloc que ses usages ; `Row.SightRadius` existe (HOLBuildingTypes.h:167). Un premier script de remplacement a refuse une substitution (paire mal formee), corrigee et verifiee par `git diff`.

Commande a lancer par la session principale (apres le build) : `quick_check -Parts tests` ou le filtre automation `HeroesOfLiberty.Combat.BuildingShotFog`, `...BuildingShotGuard`, `...BuildingShot`, `...BuildingShotCadence`. Rouge attendu sur 6fdbaea : BuildingShotFog (aveugle : fleches lancees) et BuildingShotGuard (entrees apres destruction).
ESSAIS: aucun. QUESTIONS: aucune.
FAIT / RESTE: T5 et T5b faits ; reste T6, T7 ; rouge a voir a l'execution.
NON VERIFIE: compilation ; execution des tests (donc rouge non vu) ; furtivite (meme appel IsVisible, aucun cas de test dedie) ; geometrie du test sight 1500 (la cible a 200 cm du bord est supposee vue par la tour, SightRadius multiplie par GetBuildingSight selon l'etat) ; que SightRadius 0 ne cree pas d'observateur ; que Vision->Start/Update d'un monde de test avec slots 0 et 3 laisse les camps du combat inchanges.
PROCESSUS_DETACHES: aucun (stray_processes.ps1 -Stop : "aucun processus detache")
ANOMALIES_PROCESSUS: aucune. 3 Edit refuses "not read" (EDITS_REFUSES: 3), refaits en PowerShell.
GIT_STATUS_PRINCIPALE: docs voulus seulement (FortificationsInProgress.md, EtapeDefini.md, Auto_Journey_File.md, ce rapport) : oui
