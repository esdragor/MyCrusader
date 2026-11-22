# CELEBORN-1122n : F8 T6 tire vide ou garnie (BuildingFiresEmpty), NON COMPILÉ

Joueur : une tour dont la ligne dit « ne tire pas vide » ne tire qu'avec un occupant et s'arrête quand il sort ; la Tour de guet en bois (FiresEmpty vrai) tire comme avant.
Technique : `ReleaseBuildingArrows` lit la garnison (`GetGarrisonUnitIds`) ; nombre de flèches par tir = `GetBuildingArrowCount` (1).

APPELS_DECLARES: 20 (arrêt à 24 vus par moi ; 3 refus « not read » comptés dedans)
HEURE_DEBUT/HEURE_FIN: 18:52 / 18:55 (Get-Date PowerShell ; arrêt dur 19:07 non atteint)
PREMIER_BUILD: sans build (NON COMPILÉ)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: non lancé (aucun build demandé)
TEST_ROUGE: ff94f7a (bouchon GetBuildingArrowCount + test HeroesOfLiberty.Combat.BuildingShotFiresEmpty), rouge NON VU à l'exécution (rien compilé) ; assertit l'effet : oui (flèches lâchées, santé de l'ennemi, compte à rebours)   TEST_ROUGE_SEPARE: oui (ff94f7a rouge seul, 73f1760 vert)
  Rouge attendu avec le bouchon : « garnie + FiresEmpty faux : tire » (le bouchon lit encore BuildingFiresEmpty seul) et « un occupant qui sort : arrête » (ReleasedAtLeave >= 1 faux). Passent déjà : vide + vrai tire, vide + faux ne tire pas.
COMMITS: 2 (ff94f7a rouge, 73f1760 vert), dernier 18:55, merge_main_fait: non
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0 (la valeur lue est celle que le processeur produit : TotalArrowsReleased, santé, compte à rebours ; aucun test ne fige un nombre de flèches autre que 1, « une flèche par tir » = pas de doublon de pas)
ESSAIS: aucun
QUESTIONS: aucune ; valeur par défaut : GetBuildingArrowCount rend 1 (D884 à confirmer), seule fonction du nombre
FAIT / RESTE:
- HOLCombatProcessor.h : `static int32 GetBuildingArrowCount(int32 Occupants)` (public, commentaire D884) ; .cpp : corps (rend 1).
- ReleaseBuildingArrows (.cpp ~l.866-885) : n'exclut plus FiresEmpty faux ; lambda CountOccupants (UHOLUnitSubsystem, const UWorld::GetSubsystem) ; FiresEmpty faux sans occupant : `continue` avant le compte à rebours (purgé au pas suivant : un occupant qui sort arrête la tour) ; FiresEmpty vrai : occupants lus une fois par tir seulement.
- Flèche(s) du tir dans une boucle `for Shot < max(1, GetBuildingArrowCount(Occupants))` (bloc reindenté).
- Execute, saut FastIdle (~l.1239) : `BuildingFiresEmpty` retiré de `bBuildingsLive` (une tour armée garnie ne doit pas être sautée).
- Test : HOLBuildingShotTests.cpp : options Occupants / LeaveAtStep, lignes de test BuildingShotTestTower (vrai) et BuildingShotTestTowerNoEmpty (faux) ; 4 cas : vide+vrai tire, vide+faux ne tire pas (0 flèche, 0 compte à rebours), garnie+faux tire (une flèche par pas), occupant qui sort après le pas 5 : 0 flèche ensuite et 0 compte à rebours.
- RESTE : compiler (Tools\build.ps1 -LiveCoding=0) puis `quick_check -Parts tests` ; Auto_Journey_File.md : pas de ligne à moi trouvée, non touché.
NON VÉRIFIÉ: compilation ; `const UWorld::GetSubsystem<T>()` const (attendu) ; `TSet` via includes transitifs ; SetGarrison(Id, nullptr) remet bien l'unité hors tag de garnison (lu l.1255-1290, tag retiré hors du bâtiment : non relu en entier) ; une Spearman garnie via SetGarrison sans vérification de GarrisonSlots (la ligne de test copie House) ; rouge à l'exécution ; effet d'un autre test du fichier de la modification de bBuildingsLive (tours FiresEmpty faux : aucune dans le jeu).
RELECTURE_CPP (L20-2) : fait sur le diff `git diff` de 73f1760 : GetBuildingArrowCount déclarée dans le .h avant son emploi, appelée sans qualification dans une méthode de la classe ; UHOLUnitSubsystem déjà inclus (HOLCombatProcessor.cpp l.43) ; aucun `using namespace` nécessaire (le bloc est dans ReleaseBuildingArrows, avant le `using` d'Execute) ; lambda capture `&World` et `&Step` (vivants dans la boucle) ; `continue` dans le for de la boucle principale seulement ; noms (Occupants, Shot, ArrowCount, Inside, UnitSubsystem) sans collision lue dans la fonction.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: 3 écritures refusées (Edit/Write « not read » avant lecture) au début, refaites en PowerShell sur chaîne exacte ; un 1er remplacement de boucle refusé (ancre à deux tabulations trouvée dans 3 blocs), refait avec ancre précise (aucun fichier écrit entre-temps).
GIT_STATUS_PRINCIPALE: copie principale : seuls docs modifiés (EtapeDefini.md, FortificationsInProgress.md, plus l'état déjà là) ; inchangée hors docs voulus : oui
