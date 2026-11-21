# GALADRIEL-1121sh (ligne d'affichage des pouvoirs du slot de faction), worktree Galadriel, branche specialization-trial

Impact joueur : la case de pouvoir du slot de faction n'affiche plus « SacredLight cost 25 soon » (ASCII des tests) mais une ligne traduite FR / EN / JA (« Lumiere sacree : cout 25, bientot »).

APPELS_DECLARES: ~19 (arret 25 / 20:59 non atteints) ; EDITS_REFUSES: 0
HEURE_DEBUT/HEURE_FIN: 20:47 / 20:53 (arret dur 20:59)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (sans objet)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 0efcce3 (test + stub qui compile) ; assertit l'effet : oui (cle de texte et arguments calcules par HOLPowerDisplay::Make a partir de l'etat REEL HOLPowerBook::StateOf). Rouge VU a l'execution : NON (aucun quick_check dans l'agent) ; le stub renvoie TextKey vide, donc Test_PowerDisplayKeyAndArgumentFollowTheState, ...ComposesAPattern sont rouges ; ...LeavesTheStableLineAlone vert d'emblee.
COMMITS: 2 (0efcce3 test + stub 20:50 ; 5a0bdd7 regle + affichage 20:51), merge_main_fait: non (branche = main 204f769 + commits sf/sg ; pas de merge ce soir)
TESTS: preflight income_tooltip PASS (34 s, WARN HUD overlap / Sanity anciens) ; TESTS_TAUTOLOGIQUES: 0 ; TESTS_SCRIPT_AJOUTES: 3 (Script\Tests\HOLPowerDisplay_Test.as : Test_PowerDisplayKeyAndArgumentFollowTheState, Test_PowerDisplayComposesAPattern, Test_PowerDisplayLeavesTheStableLineAlone) ; BUILDS_DEMANDES: 0 ; TYPES_A_EXPOSER: aucun
COMPILE_AS: 0 erreur (Saved_Tests\Logs\scenario_income_tooltip.log du preflight de 20:51 : 546 lignes « Angelscript », 0 contenant « rror »). DIAGNOSTICS_AS (serveur de langage) : non releves.
ESSAIS: aucun (pas d'essai ecran : income_tooltip n'affiche pas le slot de faction) ; LOGIQUE SEULEMENT.
QUESTIONS: aucune (noms de pouvoir provisoires, voir Placeholders)
FAIT:
- Script\Factions\HOLPowerDisplay.as (neuf) : FHOLPowerDisplay {TextKey, Name, Cost, Extra} ; HOLPowerDisplay::Make (regle pure : ligne, etat, recharge -> cle PowerReady/Cooling/Short/Tier/Soon + arguments) ; Compose ({0} nom, {1} cout, {2} secondes ou palier) ; PatternOf / NameOf (NSLOCTEXT "HOLFactionSlot") ; LineOf(Box) pour le HUD.
- Script\UI\HOLGameScreenScript.as:2034 seule ligne : Lines += HOLPowerDisplay::LineOf(FactionView.Powers[Index]) (au lieu de .Describe). HOLPowerBook::Describe INCHANGE (tests faction_slot.txt et HOLPowerBookRules_Test.as intacts).
- HORS LISTE (necessaire, 4 lignes) : Script\UI\HOLFactionSlot.as, FHOLFactionPowerBox gagne Cost et MinTier (remplis a cote de Box.Key) : la case n'avait ni cout ni palier hors de la chaine ASCII.
- Game.po (fr, en, ja) : HOLFactionSlot,PowerReady / PowerCooling / PowerShort / PowerTier / PowerSoon / PowerNameSacredLight (n'existaient pas). Placeholders.txt : ligne du slot de faction mise a jour (nom « Lumiere sacree » / « Sacred Light » / « 聖なる光 » provisoire).
RESTE:
- Compiler les textes : Game.locres n'est PAS regenere (je n'ai pas lance la chaine de localisation) ; tant que non fait, EN / JA affichent le francais source. Ajouter aussi les six cles a Tools\Localization\fill_po.py (non fait) si c'est lui qui alimente les .po.
- FactionPowersText suit le changement de langue seulement si RefreshFactionSlot est rappele (non verifie) ; la ressource (Honneur) n'est pas dans la ligne : le brief disait « ressource », Describe ne la porte pas.
- Executer les tests : quick_check -Parts tests (rouge attendu sur 0efcce3, vert sur 5a0bdd7).
NON VERIFIE: tests script jamais executes ; signature AssertEquals(attendu, reel, msg) copiee des tests existants ; FText.ToString() / Replace sur FString suppose valide (preflight charge le module sans erreur) ; rendu a l'ecran non vu ; fins de ligne CRLF des deux fichiers neufs (warning git LF->CRLF) ; HOLFactionSlot.as modifie hors liste.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
ANOMALIES_PROCESSUS: un `python -` a pendu 120 s (stdin vide), arrete par TaskStop ; sans suite.
GIT_STATUS_PRINCIPALE: inchange hors docs voulus (EtapeDefini.md, Placeholders.txt, ce rapport) : oui
