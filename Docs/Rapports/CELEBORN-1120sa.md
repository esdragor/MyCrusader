# CELEBORN-1120sa : tri des fichiers non suivis (Celeborn, Haldir)

APPELS_DECLARES: 15 (arrêt à 25)
HEURE_DEBUT/HEURE_FIN: 22:51 / 22:5x (Get-Date PowerShell)
PREMIER_BUILD: sans C++   BUILD: sans objet (aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (aucun code, aucun test)
COMMITS: 1 (Celeborn, duel-rewards 7f4296c, 22:53), merge_main_fait: non (inutile : HEAD de duel-rewards = d41c8d2 = main hors docs)
TESTS: aucun ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune

## 1. Celeborn (duel-rewards) : 16 fichiers non suivis

Origine (Docs\Rapports\ETAPE0-1116.md, ETAPE0-1119n-B.md, CELEBORN-1119r.md ; NeutralsInProgress.md l.15 et l.20) : scénarios de sonde du chantier Neutres / BUG-melee-stuck-attack (D641, D769), datés du 2026-11-10, jamais commités.

- `c28_b20.*`, `c28_w20.*` (4 fichiers : .txt + .json, pas de .look) : Celeborn-28, mesure A/B N=16 de la cause du stall de `playtest_hero_army` (« c est le scénario, pas le combat » : b20 = scénario d'origine, 16/16 passes avec stall ; w20 = combat déplacé à l'ouest du Forum, 0/16). Cités par leur nom dans NeutralsInProgress.md l.20 ; ne sont pas des doublons (b20 et w20 diffèrent par les positions de spawn ; ni l'un ni l'autre n'est `playtest_hero_army`). **COMMITÉS** : 7f4296c « Scenarios: add c28/c31 scenarios left untracked » (git add par nom, sans Co-Authored-By, sans push).
- `c31_f3.*`, `c31_f4.*`, `c31_f5.*`, `c31_f8.*` (12 fichiers) : copies `c31_f<k>` de Celeborn-31 (NeutralsInProgress.md l.15 : « copies c31_f<k> ... laissées »). Les quatre jeux sont identiques entre eux octet pour octet (hashes .txt/.json/.look égaux) et égaux en contenu (lignes) aux `c31_f1/f6/f7/f9` DÉJÀ suivis (commit 6e40600) : toutes les variantes « frames avant le clic » ont été réécrites en `frames 2` le 11-17, donc plus aucune ne se distingue (seul écart : fins de ligne CRLF/LF). Doublons sans valeur propre : **NON COMMITÉS, à jeter par l'utilisateur** (je n'ai rien supprimé). Commande si accord : `git -C C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn clean -f Tools/Scenarios/c31_f3.* Tools/Scenarios/c31_f4.* Tools/Scenarios/c31_f5.* Tools/Scenarios/c31_f8.*`.

## 2. Haldir (wall-line) : Content/Maps/Crown.umap

- Grep : `Crown` est la clé de recette dans `Tools\Data\Maps.csv` (« Crown,La Couronne,crown,centre ») ; les cartes générées et suivies sont `Content/Maps/Crown_{Small,Medium,Normal,Large,Huge}.umap` (main et wall-line), produites par `Tools\Editor\maps\crown.py` via `common.py` (`/Game/Maps/<Map>_<Size>`, seule `Plain` garde un nom sans taille, D494).
- `Crown.umap` (1 140 220 octets, 2026-11-07 09:38) : ancien nom sans taille, sortie d'une régénération de l'éditeur (ETAPE0-1120-B.md l.66 ; MapsInProgress.md 2026-11-07 09:45). Contient le sol `MI_Ground_Plain` (ancien). Absent de main, d'aucune branche (`git log --all` vide), d'aucun autre worktree (seul Haldir l'a), non ignoré par `.gitignore`.
- Attendue par un scénario ? `Tools/Scenarios/map_crown.json` écrit `"map": "/Game/Maps/Crown.Crown", "mapSize": "Medium"`, le même schéma que `map_forest.json` (`/Game/Maps/Forest.Forest` + `Small`, alors qu'il n'existe aucun `Forest.umap` suivi) : le moteur ajoute le suffixe de taille, donc `Crown_Medium` est chargé, pas `Crown.umap`. Aucune autre référence à `/Game/Maps/Crown` sans taille dans Tools, Source, Script.
- Verdict : **carte générée, régénérable, non référencée : NON COMMITÉE** (1,1 Mo binaire inutile). À supprimer par l'utilisateur si accord (`Remove-Item C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Content\Maps\Crown.umap`), ce qui évite aussi le FAIL de preflight « Crown.umap non suivi » vu le 2026-11-14.

## Journaux

EtapeDefini.md (ligne CELEBORN-1120sa après 1120pd), NeutralsInProgress.md (dernière ligne). IN_USE.txt des deux worktrees : OCCUPE à l'entrée, libre à la sortie (CPP: non).

NON VÉRIFIÉ : que le moteur résout bien `Crown.Crown` + `mapSize` vers `Crown_Medium` (déduit de map_forest sans Forest.umap et de common.py, pas lancé : aucun éditeur) ; que `Maps.csv` du worktree ne cite plus `Crown` seul au sens carte (il cite la clé de recette `Crown`, normale).
ANOMALIES_PROCESSUS: aucune. Avertissement git LF->CRLF sur c28_b20.txt / c28_w20.txt (autocrlf, sans effet).
GIT_STATUS_PRINCIPALE: EtapeDefini.md et NeutralsInProgress.md modifiés (docs voulus) en plus de l'état de départ : oui.
