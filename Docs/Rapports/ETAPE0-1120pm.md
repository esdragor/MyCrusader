# ETAPE0-1120pm : plan de la séance Auto_Journey du 2026-11-20 (15:36 -> 18:00, interactif), lecture seule

Joueur : rien de nouveau à l'écran avant la première fusion ; ce plan vise D869 (montage / démontage du Mangonneau visibles), D870 (marque du signal ÷ 3), D862 (Sceau du Moine), D867 (murs sans plafond), puis les branchements d'écran restés « pas branchés ».
Base lue à 15:34-15:40 : main = a831674, 8 worktrees sur main (`status.ps1`), tous « libre ». Aucun fichier du dépôt modifié.

## Plan par worktree (agents Sonnet 15 min / 25 appels, sans build ni quick_check)

- **Celeborn (duel-rewards)** : lancer « IA Nomade : brancher `HOLAINomad::HelpersFor` dans `Script\AI\HOLAINomadThink.as` (appelé par `HOLAIBrain.as:200-203`) » (file l. 118, D817 A + A) ; lire `HOLAINomadThink.as`, `HOLAINomadHelpers_Test.as:55-108`, `NomadInProgress.md`, `HOLAIBrain.as:195-210` ; premier test rouge : script `HOLAINomadThink_Test.as` (neuf) = après `Think`, les Paysans oisifs les plus proches (plafond `hol.AI.NomadMaxB`) reçoivent l'ordre de bâtir la fondation ; C++ non. (Repli : D859 valeurs de départ, `Docs\Rapports\CELEBORN-1119n2.md`, file l. 33, reste 1a / 1b / 1d.)
- **Elrond (integ-25)** : lancer D869 (file l. 21) : cause de « instantané » d'abord (lire `HOLDeployRules.cpp:31-36` `DefaultProfile`, `HOLDeployRules.h:76-77` Proposed 8000 / 5000 ms, `HOLDeployProcessor.cpp:63`, `Tools\Scenarios\deploy.txt:9-47`, `Palier3InProgress.md:282-284`), puis ramener à 1500 / 1000 ms et rendre visible ; premier test rouge : `HOLDeployProcessorTests.cpp:81-82` (DefaultProfile().DeployMs == 1500, PackMs == 1000, rouge sur 8000 / 5000) ; C++ OUI (processeur Mass, lot C++), tests C++ à mettre à jour : `HOLDeployButtonTests.cpp:124-125, 297, 415-453, 548`, `HOLDeployInMatchTests.cpp:146`.
- **Faramir (zone-effect)** : lancer D862 (file l. 23) : le Sceau de lumière passe par les capacités d'unité du Moine (`LightSeal` 40 s, ligne déjà dans `UnitAbilities.csv`, GLORFINDEL-1120a 1c), sans jauge ; lire `Script\Factions\HOLFactionPowerCast.as`, `Script\Net\HOLRpcPlayer.as:1060-1100` (`ServerCastFactionPower`), `Script\Tests\HOLFactionPowerCast_Test.as:21-49` ; premier test rouge : Honneur inchangé après un Sceau (`Test_FactionPowerCastChain` étendu ou test voisin) ; C++ non (liaison `HOLZoneEffect::StartLightSeal` existe).
- **Galadriel (specialization-trial)** : lancer « saisie du chat branchée à l'écran du jeu » (file l. 107, D807 A, `GLORFINDEL-1120i.md`) : Entrée ouvre `HOLChatInputScreen`, Tab bascule Allies > Tous > privé, depuis `HOLGameScreenScript.as` ; lire `Script\UI\HOLChatInputScreen.as`, `HOLChatInput.as`, `HOLChatInputScreen_Test.as`, `HOLGameScreenScript.as` (relais Entrée de 09207f7) ; premier test rouge : script `HOLGameScreenChat_Test.as` neuf = Entrée ouvre l'écran, Échap le ferme ; C++ non. Prendre `HOLGameScreenScript.as` seule (alertes mineures et garnison attendent la vague 2, même fichier).
- **Glorfindel (ability-columns)** : lancer D870 (file l. 22) : ÷ 3 de la marque, sol `CVarSignalGroundMarkSize` défaut 500 -> 167 (`HOLSelectionMarkers.cpp:237`, lu à :898) et mini-carte `HOLSignalMarks.as:7` (`BaseSize` 10 -> 3.3, `HOLSignalMarks_Test.as`) ; premier test rouge : `HOLSignalMarks_Test.as` (taille mini-carte = tiers) ; C++ OUI mais minuscule (une valeur, lot C++), pas de nouvel essai.
- **Haldir (wall-line)** : lancer D867 (file l. 24) : murs sans plafond ; lire `Script\Buildings\HOLWallLineRules.as:12, 142-146, 219`, `Script\Net\HOLRpcPlayer.as:1490` (`ServerPlaceWallLine`), `Script\Tests\HOLWallLine*_Test.as:89 et :250-253` ; premier test rouge : `MaxLineCells() == 0` = sans limite, une ligne de 40 cases acceptée par `DragCellsFromPoints` et `PlanFromRequest` (le test :250-253 qui fige 16 est à renverser, il devient le rouge) + `ServerPlaceWallLine` pose un fantôme par case, jamais un mur fini ; C++ non (tout est en script).
- **Sauron (tool-known-reds)** : lancer D864 + D865 (file l. 26) : colonnes `FHOLUnitTypeConstants` (part 30 %, rayon 800 cm), mode Nearest ; lire `HOLUnitTypes.h` (struct, chaque champ `UPROPERTY`), `HOLDamageRedirectRules.h/.cpp`, `HOLCombatProcessor.cpp:638-730`, `HOLDamageRedirectTests.cpp` ; premier test rouge : `HeroesOfLiberty.DamageRedirect.Rules` avec part / rayon lus de la table ; C++ OUI (processeur de dégâts, colonnes de table, lot C++). Le CSV + l'asset régénéré demandent l'éditeur : à la session principale.

## Tâches C++ à regrouper dans UN lot (un build, 3 agents C++ au plus : Elrond, Glorfindel, Sauron)

1. D869 : `HOLDeployRules.h:76-77` / `.cpp:34-35` + 4 fichiers de tests C++ du déploiement (Elrond).
2. D870 : `HOLSelectionMarkers.cpp:237` (défaut du cvar) (Glorfindel).
3. D864 + D865 : `HOLUnitTypes.h` (en-tête central : tout champ de `FHOLUnitTypeConstants` en `UPROPERTY`), `HOLCombatProcessor.cpp` (Sauron).
4. Vague 2 si le temps (dernier lancement C++ ~17:00) : D868 (`HOLCommandGrid.cpp:63` / `.h:327` `GetSpecializationCell`, cases 5-6-7 Templiers) et morceaux restants du lot socle (`LOT-CPP-socle-factions-1119.md` : 2, 3, 6, 7, 8 ; en-têtes centraux `HOLGameState.h`, `HOLScriptRpcPlayerActor.h`, `HOLPlayerController.h`, `HOLScriptScreen.h`). Garnison (liaison C++ occupants, ELROND-1120g) et geste de glisser des murs (dessin, `HOLPlayerController`) : vague 2.
Aucun agent n'écrit dans `HOLUnitTypes.h`, `HOLGameState.h`, `HOLPlayerController.*` en même temps qu'un autre.

## Branchements « pas branchés à l'écran » restés du matin

- Saisie du chat : ligne prête (file l. 107, ci-dessus, Galadriel).
- Garnison : règle `HOLGarrisonView.as` faite, RESTE liaison C++ (occupants lus par le client) puis widget : pas de ligne sans C++ ; vague 2 (Elrond, `ELROND-1120g.md`).
- Alertes mineures : `HOLAlertRules.as::MinorAlert` + `GetMinorEventsNow` (09207f7) faits, aucun appelant dans l'écran (file l. 100) : ligne sans C++ prête pour la vague 2 de Galadriel (même fichier que le chat).
- Glisser des murs : `DragCells`, pont et RPC DANS main (86e27d1, 4722aa4) ; reste dessin du fantôme et `WallTypes` vide jusqu'à F5 (file l. 51 périmée en partie) : vague 2 Haldir, C++.
- `neutrals_clairiere` : scénario NEUF DANS main (a3c3ce5), LOGIQUE SEULEMENT ; reste preflight PASS, PNG ouverts, `E-neutrals-clairiere` (file l. 13) : tâche de la session principale (éditeur), pas d'agent.

## Lignes périmées de la file (à marquer)

- l. 50 (`HOLDuelRewards::Orders` dans `HOLDuelLedger.as`) : FAIT, `HOLDuelLedger.as:224-230` appelle `OrdersOfEnded` et `HonorOfEnded`.
- l. 28, 29, 30, 32, 118 (et 107 / 81 en partie) : « NON COMPILE » périmé, ces commits sont dans main et integ-29 (build PASS, tests 1922/1922 à 991a8bc) ; à clore après lecture du rouge/vert de `AttackWhilePackedDeploysThenFires` et `AtTheCombatReachButBeyondDeployRange` (D861 a, cause « non prouvée »).
- l. 87 (« aucun agent actif ; E-deploy [À ESSAYER] ») : essai joué, devenu D869.
- l. 51 : « reste geste de glisser, appelant du RPC » faits (86e27d1, 4722aa4).
- l. 43 (« premier build = morceaux 1 à 8 ») : morceau 1 (8164ba8), 4 (Faramir), 10 (Sauron `a1fa9dc`) déjà faits ; restent 2, 3, 5-8.
- l. 100 : « pas dans main » à revérifier (migration finie D857, écrire en script).

## Questions bloquantes

Aucune : `Auto_Journey_Retours.md` = 0 question, 0 essai à 13:14. Les `[?]` de la file (japonais des Légions, essai réseau E5, El Dorado, Biomes B4, Q-neutrals-camp-cache) n'empêchent aucun lancement. À NE PAS lancer : Palier 3 G1 (l. 111, « dire à l'utilisateur AVANT », C++ MinTier).

## Charge machine (15:34-15:37)

Mémoire virtuelle libre 28,7 Gio (30 068 340 Kio sur 83,6 Gio), physique libre 36,2 Gio, CPU ~37 %. 3 `cl.exe` vivants à 15:34, plus aucun à 15:36 (compilation finie ; pas de ProjectShiva reconnu). Un `UnrealEditor` (pid 28964) démarré à 15:35:31, projet non identifié : vérifier avant toute tâche d'éditeur. `BuildQueue.state` : pas de ticket vivant (`editors` vide, `queue.lock` du 11-06, un `.gone` du 11-16). `status.ps1` : BUILD LENT Faramir 15:13 (462 s contre médiane 149,5 s, 20 actions, vm 18,7 Gio) : prévenir l'utilisateur ; garder les builds sérialisés.

## integrate.ps1 (L19-9)

Non, `Tools\AutoJourney\integrate.ps1` n'existe pas (ligne L19-9 de la file, l. 154, encore `[ ]`) ; usage prévu : fusion des rouges, build, tests, fusion des verts, build, tests, `net,build`, résumé court ; à faire à la main d'ici là.
