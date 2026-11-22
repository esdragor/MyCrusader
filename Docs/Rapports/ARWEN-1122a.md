# ARWEN-1122a : NON COMPILE (opacite de furtivite dans la donnee d'instance des unites, D824 Q3)

Impact joueur : aucun visible tant que le materiau M_UnitPlayerColor ne lit pas l'index 4 (voir ENTREE MATERIAU) ; ensuite, ses propres unites en zone de furtivite sont dessinees a demi transparentes.

APPELS_DECLARES: 16 (arret a 25 vus ; 15 appels avant le rapport)
HEURE_DEBUT/HEURE_FIN: 16:31:52 / 16:34:45 (Get-Date PowerShell ; arret dur 16:47 non atteint)
PREMIER_BUILD: sans build (consigne : NE PAS COMPILER)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build)
TEST_ROUGE: 09868d9 (commit du test + bouchon, anterieur au correctif b620434) + assertit l'effet : oui (la valeur lue dans la donnee d'instance est calculee par la vraie fonction MakeUnitInstanceCustomData, apres un vrai SetStealthOpacity). Rouge ATTENDU, PAS VU a l'execution : bouchon = toujours 1.0, donc 2 tests sur 3 rouges (InZone, Layout), Outside vert.
TEST_ROUGE_SEPARE: oui (09868d9 bouchon + tests, b620434 vraie valeur ; le classifieur n'a rien refuse)
COMMITS: 3 (09868d9 rouge, b620434 vert, facffa5 docs StealthInProgress.md) + merge main 8b1a6d5 (main n'avait que des docs : `git diff HEAD...main -- Source Script` vide), dernier a 16:34, merge_main_fait: oui
TESTS: aucun lance (NON COMPILE). 3 tests C++ neufs dans Source\HeroesOfLiberty\Tests\HOLUnitOpacityDrawTests.cpp : HeroesOfLiberty.Presentation.UnitDrawOpacity.{InZone,Outside,Layout} ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (ce lot C++ dans le build de la session principale)   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun

## FAIT
- HOLUnitOpacity.h : namespace HOLUnitDraw (constexpr NumInstanceCustomFloats = 5 ; FInstanceCustomData {FVector3f Color; float Prop; float Opacity;} avec static_assert 5 floats ; MakeInstanceCustomData(Color, Prop, Opacity) ; MakeUnitInstanceCustomData(Color, Prop, UnitId)).
- HOLUnitOpacity.cpp : MakeUnitInstanceCustomData = MakeInstanceCustomData(..., UHOLUnitOpacityLibrary::GetStealthOpacity(UnitId)) (0.5 en zone, 1.0 sinon).
- HOLInterpolatedInstancesProcessor.cpp : FUnitCustomData devient un alias de HOLUnitDraw::FInstanceCustomData ; site unites (l.580) appelle MakeUnitInstanceCustomData(..., State.UnitId) ; GetCustomData (corps) ecrit 1.0.

## SITES D'AJOUT / MISE A JOUR DE DONNEE PERSONNALISEE (decalage d'index)
Lus par grep sur Source\ (NumCustomDataFloats / AddBatchedCustomData / SetCustomData) :
1. HOLInterpolatedInstancesProcessor.cpp ~l.580 : unites vivantes, AddBatchedCustomData (5 floats maintenant : couleur 0-2, prop 3, opacite 4).
2. HOLInterpolatedInstancesProcessor.cpp ~l.654 : corps (CorpseQuery), meme composant Mass, AddBatchedCustomData via GetCustomData -> 5 floats aussi (opacite 1.0). Les deux sites ecrivent le meme nombre : Mass derive le nombre de floats du sizeof de la structure (commentaire d'origine : « the float count is one per component, and Mass ensures it »), donc aucun mixte 4/5.
3. Hors sujet (composants ISM separes, non touches) : HOLSelectionMarkers.cpp l.52/274/316 (NumCustomData = 4, SetCustomData), HOLArrowSubsystem.cpp l.123/155/351 (1 float, opacite de fumee), tests HOLCombatTests.cpp l.624-626 et HOLSignalGroundMarkTests.cpp l.62.
Aucun `NumCustomDataFloats` n'est declare pour les unites dans Source\ : le compte vient de la structure (donc 4 -> 5 d'un seul endroit) ; un descripteur de mesh dans un data asset Mass (.uasset) pourrait fixer ce nombre : NON VERIFIE (aucun .uasset ouvert). Les index 0-3 n'ont pas bouge (couleurs et props intacts) ; l'index 4 est neuf.

## ENTREE MATERIAU (tache d'editeur a part, AUCUN .uasset touche)
Dans M_UnitPlayerColor (et M_UnitPlayerColorProps s'il a son propre graphe) : ajouter un noeud `PerInstanceCustomData` (DataIndex 4, valeur par defaut 1.0) ; le brancher sur l'opacite : Blend Mode = Masked (ou Dithered via `DitherTemporalAA(Alpha = valeur)` -> Opacity Mask) pour un rendu instancie sans tri translucide ; verifier la compatibilite avec le skinned instance (GPU anim) et ombres. Tant que ce noeud n'existe pas, le rendu est INCHANGE (le 5e float est ignore). Le nombre de CustomData du composant passe de 4 a 5 sans effet sur les entrees 0-3.

## RELECTURE_CPP (L20-2)
Diff `git diff main..HEAD -- Source/HeroesOfLiberty/Presentation` relu ligne a ligne : alias + 2 sites coherents, static_assert 5 floats, aucune autre utilisation de FUnitCustomData / GetCustomData. Une exigence connue : `UnitId` du Mass (uint32) converti en int32 (meme id que HOLStealthOpacityApply.as, a confirmer). Cout : un FCriticalSection par unite et par pas de dessin (GetStealthOpacity) ; a mesurer si des milliers d'unites (optimisation possible : un seul verrou/copie par frame).

## NON VERIFIE
Compilation, les 3 tests (rouge attendu 2/3 puis 3/3), l'identite des ids (State.UnitId du Mass == id passe a SetStealthOpacity par HOLStealthOpacityApply.as), l'absence d'un compte de floats fixe dans un asset Mass, le rendu a l'ecran.
A lancer (session principale) : Tools\build.ps1 -LiveCoding=0 -Wait puis quick_check -Parts tests (filtre HeroesOfLiberty.Presentation.UnitDrawOpacity) ; pour VOIR le rouge : poser le worktree sur 09868d9.

PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Arwen
ANOMALIES_PROCESSUS: aucune ; sortie `edit_line` correcte x3 ; un `tail` Unix invoque par erreur sous PowerShell (sans effet).
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md, M Auto_Journey_Jour.md, M EtapeDefini.md, ?? Docs/Concept_*, HSR_*, Priorite_*, Rapports/ETAPE0-1122.md, GALADRIEL-1122a.md, SAURON-1122a.md : docs seulement (mes modifications : EtapeDefini.md et Auto_Journey_File.md) ; code inchange : oui
