LA BRANCHE mut-d882 NE SE FUSIONNE JAMAIS (mutation de contrôle, à supprimer après usage : `git branch -D mut-d882`).

# ELROND-1122f : mutation de contrôle du test D882 B

NON COMPILÉ (CPP: non). Début 17:25, fin ~17:32 (Get-Date).

## Fait
- Branche locale `mut-d882` depuis 824693a, worktree propre avant. Commit de la mutation : **8c1e094** `MUTATION (never merge): Stop no longer clears bMarching in ApplyCombatOrder, control mutation for the D882 B test` (1 fichier, +2 -1).
- Mutation : `Source/HeroesOfLiberty/Commands/HOLUnitOrderProcessor.cpp` l.3224-3225 (ApplyCombatOrder), la remise `CutDeploy->bMarching = 0` n'a plus lieu quand `Order.Kind == EHOLOrderKind::Stop` (condition `CutDeploy && Order.Kind != EHOLOrderKind::Stop`, commentaire « MUTATION DE CONTROLE, jamais fusionnee »). Attack, AttackBuilding et Hold remettent toujours bMarching à 0. Compilable (même type, un `if` avec initialiseur).
- Retour sur integ-25 : `git status` propre (0 ligne), tip inchangé 824693a. IN_USE.txt : OCCUPE à l'entrée, `libre` à la sortie.

## Tests attendus sur mut-d882 (non lancés : pas de build)
ROUGE :
- `HeroesOfLiberty.Siege.Deploy.StopDuringMarchStaysPackedAndDoesNotMountByItself` (FHOLDeployStopCutsMarchStaysPackedTest) : après Stop, bMarching reste 1 (assertion « bMarching 0 » rouge), puis UHOLDeployProcessor lit « marche finie » (MoveOrder.Phase None) et monte l'engin pendant les 10 s (Phase hors Packed).

VERTS (le chemin de la mutation n'est pas leur chemin) :
- `HeroesOfLiberty.Siege.Deploy.AttackCutsAnOpenMarchAndComesInMountsAndFires` (l'attaque coupe la marche : Kind Attack, non muté).
- `HeroesOfLiberty.Siege.Deploy.NormalMarchEndWhilePackedMountsTheEngine` (fin normale, aucun Stop).
- Aussi `...PackedEngineMountsByItselfAtTheEndOfAMarch` (D877, fin normale).

## Pour lancer
Dans un worktree dédié (jamais sur integ-25) : `git switch mut-d882`, `Tools\build.ps1 -LiveCoding=0 -Wait`, `quick_check -Parts tests`, attendre exactement 1 rouge (le test Stop). Si 0 rouge : le test ne protège pas (à durcir). Puis revenir sur integ-25 et supprimer la branche.

## Champs agent_rules
TEST_ROUGE: sans objet (mutation, pas de correctif). COMMITS: 1 (8c1e094), merge_main_fait: non (hors sujet). TESTS: aucun lancé. TESTS_TAUTOLOGIQUES: 0. QUESTIONS: aucune. ESSAIS: aucun.
NON VÉRIFIÉ: compilation de la mutation, rougeur effective du test (raisonnement par lecture du code).
ANOMALIES_PROCESSUS: un heredoc Bash + python3 a bloqué 120 s (tâche arrêtée par TaskStop, aucun effet sur le dépôt) ; l'Edit multi-lignes a été refusé (CRLF), remplacé par sed sur la ligne 3224. Aucun refus du classifieur. Aucun lancement d'éditeur : PROCESSUS_DETACHES: aucun.
GIT_STATUS_PRINCIPALE: non relu (aucune écriture dans la copie principale hors ce rapport).
