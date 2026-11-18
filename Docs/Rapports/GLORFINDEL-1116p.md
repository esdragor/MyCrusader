# GLORFINDEL-1116p : lot E3-12, HOLManagerErrorTextTests porte en script

Date : 2026-11-16 22:52 -> 22:59. Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel`, branche net-playtest, commit `6727647` (branche prete, merge main non fait, aucun push). Sans C++, aucun build.

## Fait
`Script\Tests\HOLManagerErrorText_Test.as` (neuf) contre `UHOLManagerErrorTextReadLibrary` (`HOLManagerErrorTextRead::`) : Test_ManagerErrorTextCodes (= ...ManagerErrorText.Codes : unreachable, unreadable, version_mismatch avec/sans versions, MakeVersionMismatchError, 16 codes, 3 codes generaux), Test_MatchLossText (= ...MatchLossText : 10 cas ENetworkFailure, MakeMatchTravelError, texte bad_key, CanCreateLobbyAfter x4), Test_ManagerErrorTextReturnToLobby (= ...ManagerErrorText.ReturnToLobby : 4 codes + unreachable). Memes vecteurs, accents en UTF-8 (compile et tourne).

## Rouge / vert
- Rouge VU 22:55 : 3 valeurs faussees (une par test), tests 1360/1365 ; les 3 tests script en echec + Nomad.
- Vert VU 22:58 : tests 1363/1365, seuls rouges Match.StartingHero.NomadFirstForum et .NomadReplicated (D831, known_reds). Le total monte de 3.
- Le script compile avec ENetworkFailure::X et FHOLManagerErrorValue (liaisons du rapport S1D-1116c confirmees a l'execution).

## Non porte (le test C++ correspondant reste)
- `FHOLManagerClientRules::ReadFailure(true, 401, json)` du test MatchLossText (non expose) : remplace par un bad_key construit a la main (texte + CanCreateLobbyAfter portes), mais l'analyse du JSON 401 reste couverte par le C++ seul.
- Aucun controle FTextInspector / .locres / CSV / MakeMeBody dans ce fichier C++ (rien d'autre a lister).

## Registre test C++ -> script
HeroesOfLiberty.UI.ManagerErrorText.Codes -> Test_ManagerErrorTextCodes ; HeroesOfLiberty.UI.MatchLossText -> Test_MatchLossText (sauf ReadFailure) ; HeroesOfLiberty.UI.ManagerErrorText.ReturnToLobby -> Test_ManagerErrorTextReturnToLobby. C++ non supprime (MIG-C++-FINAL).

## Champs
BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 3 (Test_ManagerErrorTextCodes, Test_MatchLossText, Test_ManagerErrorTextReturnToLobby) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 (test C++ garde ; 148 l. a retirer en MIG-C++-FINAL) ; TYPES_A_EXPOSER: aucun de plus (FHOLManagerClientRules::ReadFailure si on veut porter le 401) ; TEST_ROUGE: vu a 22:55 : 1360/1365 ; TESTS_TAUTOLOGIQUES: 0 ; EDITS_REFUSES: 0 ; PROCESSUS_DETACHES: aucun ; C++: non.

## Suivi
Ligne E3-12 de la file ([x]), EtapeDefini.md (1 ligne), AngelscriptInProgress.md (1 ligne), IN_USE.txt (libre, CPP: non ; fichier ignore par git, non commite). Placeholders : rien ajoute ni retire.

## NON VERIFIE
Aucun `quick_check` hors partie tests (net, netsig3 non relances) ; merge main non fait.
