# ELROND-1119y : coût du pont de la grille de commandes (planche script AVEC / SANS)

Date : 2026-11-19, worktree `HeroesOfLiberty-Elrond`, branche `integ-16`, aucun build, rien de commité.

## Méthode
- Mesure au niveau de l'image : le code de la grille n'a aucun compteur (`SCOPE_CYCLE_COUNTER` / `TRACE_CPUPROFILER` absents de `HOLCommandGrid*`, pas de `hol.Perf`). Modèle suivi : `Tools\Scenarios\zz_perf_*.txt` (`csvprofile start` / `stop`).
- Scénarios temporaires non suivis `perf_grille` (forum sélectionné, 20 s) et `perf_grille_groupe` (6 maisons nées par `hol.Cheat.SpawnBuilding`, puis `hol.Select.Category Economic` = F2 : forum + 6 maisons, N = 7, 20 s). Lancés par `play.ps1 <nom> -Hidden`, fenêtre cachée sans limite d'images (~125 images/s).
- AVEC = `HOLCommandGridBoard.as` + deux compteurs de membres (appels `GetCommandAnswer` / `GetOrderAnswer`, un `Log` tous les 1000 appels de commande ; coût de l'instrumentation négligeable devant un appel de pont). SANS = `HOLCommandGridBoard.as` supprimé le temps de la passe : la classe de base C++ est `UCLASS(Abstract)`, `ShouldCreateSubsystem` la refuse, `FindAny()` rend null, `AskGetCommand/AskGetOrder` rendent faux et `HOLCommandGrid.cpp:60/179` prend `GetCommandNative/GetOrderNative`. Vérifié dans le journal : aucune ligne `GRIDBRIDGE` en SANS (le compteur n'existe pas), présente en AVEC. Pas de ligne de journal propre au repli C++ (non vérifié autrement que par la lecture du code et l'absence du compteur).
- 3 passes par cas et par mode, alternées (ordre inversé à la passe 2), colonnes `FrameTime` et `GameThreadTime` du CSV profiler, 10 premières images écartées.

## Résultats (ms par image, moyenne du temps de fil de jeu ; pire image entre parenthèses)
| Cas | AVEC (3 passes) | SANS (3 passes) | Écart AVEC - SANS |
|---|---|---|---|
| Forum seul, moyenne | 8,12 / 8,53 / 7,76 = 8,14 (sd 0,39) | 8,18 / 10,46 / 7,54 = 8,73 (sd 1,5) | -0,59 ms (-6,8 %) ; passe la plus calme : +0,21 ms |
| Forum seul, pire image | 99,5 / 58,0 / 22,7 | 72,1 / 90,1 / 27,0 | non significatif |
| Groupe (N = 7), moyenne | 7,79 / 8,04 / 7,46 = 7,76 (sd 0,29) | 9,37 / 8,75 / 8,28 = 8,80 (sd 0,55) | -1,04 ms (-11,8 %) |
| Groupe (N = 7), pire image | 74,8 / 58,3 / 32,2 | 58,9 / 81,4 / 36,3 | non significatif |

Mesures complètes : 12 passes, `FrameTime` ~ `GameThreadTime` (+0,01 ms) ; images par passe 1891 à 2664.

## Appels de pont réels (compteur du journal, tous temps depuis le lancement)
- Forum seul : moins de 2000 appels `GetCommand` sur toute la passe (1000 atteint à ~20 s du lancement), `GetOrder` = 36 : de l'ordre de 50 appels/s, bien moins de 1 par image.
- Groupe de 7 : `GetCommand` 1000 -> 7000 en ~18 s = ~330 appels/s (~2,6 par image à ~125 images/s, ~5,5 par image à 60 images/s) ; `GetOrder` = 60 sur toute la passe.
- Donc la grille n'est PAS reconstruite 12 fois par image en continu : le nombre réel est de 1 à 2 ordres de grandeur sous le pire cas théorique (12 x N par image).

## Conclusion
Le pont ne coûte pas de temps mesurable par image : l'écart AVEC - SANS est négatif (AVEC plus rapide) dans les deux cas, donc entièrement dans le bruit (dispersion entre passes 0,3 à 1,5 ms, supérieure à tout écart) ; borne haute lue sur la passe la plus calme : au plus ~0,2 ms par image pour le forum (+1,3 % d'une image de 16,7 ms), aucun écart positif pour le groupe de 7.

## Ce qui n'a PAS pu être mesuré
- Le coût propre d'un appel de pont : trop petit devant le bruit (la machine n'était pas calme : deux éditeurs `ProjectShiva` d'un autre projet tournaient pendant toute la mesure, ~65 % CPU total ; la passe 2 est la plus perturbée).
- Le nombre final exact d'appels par passe (le journal ne marque que chaque millier) ; le taux du forum seul est une borne haute.
- Un groupe plus grand que 7, une partie en situation réelle (50+ bâtiments, réseau, rafraîchissements du HUD par souris), fenêtre visible plein écran ou plafonnée à 60 images/s, rendu GPU (fenêtre cachée sans fenêtre) : non testés.
- Les images d'initialisation (premier appel qui construit la planche) : écartées avec les 10 premières images.
- Une ligne de journal prouvant le repli C++ : il n'en existe pas ; preuve par code + absence du compteur.

## État
Fichiers suivis restaurés (`git checkout -- Script/Player/HOLCommandGridBoard.as`), scénarios `perf_grille*` supprimés, `stray_processes.ps1 -Stop` passé.
