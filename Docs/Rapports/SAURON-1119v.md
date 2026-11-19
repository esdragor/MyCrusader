# SAURON-1119v : fin du lot C2 de l'etape 5 (15 commandes console en script, ecarts de journal corriges)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, branche `envelope`. Branche prete, pas de push, pas de merge. Depart 11:48, fin ~12:08.
Commits : `da448c2` (cherry-pick 295f224, ServerHeroAbility en FVector_NetQuantize), `d4b247b` (niveaux de journal C1 + C2), `5dbbcaf` (C2b script + tests), `c98c584` (retrait C++).

BUILDS_DEMANDES: 1 (le retrait C++ de `HOLPlayerController.cpp` n'est pas compile : la session principale le compile ; aucun build lance par moi)
TESTS_CPP_RETIRES: 0
TESTS_SCRIPT_AJOUTES: 4 (`HOLControllerCommandsC2b_Test.as`) : Test_ControllerCommandsC2bEachNameOnceAndKnown, Test_ControllerCommandsC2bMarketAndTribute, Test_ControllerCommandsC2bProduction, Test_ControllerCommandsC2bReportsAndDiplomacy
LIGNES_CPP_SUPPRIMEES: 297 (`git diff --stat` : HOLPlayerController.cpp, 297 deletions, 0 ajout)
LIGNES_MIGREES_PAR_LOT: 297 C++ -> ~290 script (`HOLControllerCommandsC2b.as`) + ~110 script de test
TYPES_A_EXPOSER: aucun nouveau (liaisons de 1119u utilisees telles quelles)
TEST_ROUGE: vu a 12:00 : `ResourceOf("wood")` rendait 2 au lieu de 1 -> quick_check tests 1692/1694 (Test_ControllerCommandsC2bMarketAndTribute et Test_ControllerCommandsC2bReportsAndDiplomacy en echec) ; restaure (`git checkout`) ; vert 1694/1694 vu a 11:56 (avant le bouchon) et relance apres restauration : 1694/1694 PASS a 12:04.
Total de depart : non mesure (pas de passe sur le seul cherry-pick) ; vert apres ajout = 1694, dont les 4 tests neufs (donc 1690 avant, deduit).
EDITS_REFUSES: 0 ; PROCESSUS_DETACHES: 0 (stray_processes -Stop : aucun).

## Commandes migrees (15, memes noms `hol.*`) : `Script\Player\HOLControllerCommandsC2b.as`
hol.Hero.Rage, hol.Market.Trade, hol.Tribute, hol.Tribute.Notice, hol.Prod.Rally, hol.Prod.Queue, hol.Prod.Cancel, hol.Signal.Report, hol.Chat.Report, hol.Spec.Report, hol.Prod.Report, hol.Market.Report, hol.Diplomacy.Force, hol.Diplomacy.ForceVision, hol.Diplomacy.Report.
Chaque commande : plan pur `PlanOf` (teste), puis `HOLControllerCommand::*` (liaison 1119u) par controleur local, ou une fois pour celles du monde (Market.Report, Diplomacy.*, Tribute.Notice). Lignes Display via `HOLScriptLog::LogDisplay`, lignes Usage / unknown resource via `LogWarning`, categorie `LogHOLMatch`.
Fidelite : gold refuse par Market.Trade (unknown resource, comme le C++), ressource inconnue = gold pour Tribute.Notice, Prod.Rally a moins de 2 nombres = silence, Diplomacy.Force : mot inconnu = enemy.

## Ecarts de journal corriges
- C2 : `LogNotSent` rend maintenant `HOLScriptLog::LogError` (Error, LogHOLMatch) ; les lignes Usage passent par `LogWarning`.
- C1 (`HOLControllerCommands.as`) : memes ecarts, corriges : « RpcRoute ... not sent » -> Error (2 endroits), Usage -> Warning, `hol.Units.SpawnHere: on the server or in solo...` -> Warning (le C++ d'origine, commit 53c3f92, l'avait en Warning ; la ligne `SpawnHere: N ...` reste Display).

## Reste / non corrige
- Spectateur : AUCUNE liaison `IsSpectator` n'existe (grep Source) : les 11 commandes de C2 (RPC script direct) envoient toujours le RPC, le serveur refuse (ligne « refused »). Les commandes de C2b passent par `Request<X>` de la liaison : garde spectateur C++ conservee. Pour fermer l'ecart C2 : exposer `HOLControllerCommand::IsSpectatorSeat(Controller)` (lot C++) puis `if (...) continue;` dans `HOLControllerCommandsC2::Run`.
- Non retires de `HOLPlayerController.cpp` : les `Do<X>` / `Request<X>`, les ~50 commandes pilote de test, hol.Build.Report, hol.Tech.Report, hol.Victory.Surrender, hol.Prod.RallySite (hors lot). `FindOwnBuilding` reste utilise (lignes ~1648). Verifie par Grep : aucun autre appelant des 15 variables retirees ni des noms de commande dans `Source\`.
- Doublon `FindOwnBuilding` dans `HOLControllerCommandLibrary.cpp` : a laisser tant que le C++ de `Prod.RallySite` l'utilise.

## NON VERIFIE
- Compilation C++ du retrait (a faire : `Tools\build.ps1 -LiveCoding=0 -Wait`) ; apres ce build, le binaire n'a plus les commandes C++ : relancer `quick_check -Parts tests`.
- Comportement en jeu des 15 commandes (aucun jeu lance : seuls les plans sont testes ; l'appel des liaisons `HOLControllerCommand::`, dont la resolution du WorldContext sans argument, n'est exerce par aucun test) ; les niveaux de journal LogWarning / LogError ne sont pas lus par un test.
- Double enregistrement : dans le binaire actuel, les commandes C++ existent encore ; aucun avertissement de double enregistrement vu au log (0 `Angelscript: Error`), a reverifier au 1er lancement reel.
- Scenarios : verifier par grep dans `Tools\Scenarios` qu'aucun ne lit les lignes Usage en Display.
