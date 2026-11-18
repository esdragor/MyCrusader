# HALDIR-1117a : L17-06 ConfirmDialog en script (MIG-CPP, D830/D839)

Commit 47e222a sur `maps` (worktree HeroesOfLiberty-Haldir). Branche prete, merge main non fait, NON COMPILE (consigne : pas de build).

## Fait
- Base C++ `UHOLConfirmDialogScreen` (`Source\HeroesOfLibertyUI\Screens\HOLConfirmDialogScreen.h/.cpp`, ~95 + ~125 l.), Abstract/Blueprintable, `bIsModal = true`. Motif C++ : liaisons a exposer (UHOLScreen::AddText/AddButton non UFUNCTION) + callback C++ (lambdas des appelants).
  - `Ask(PC, question, libelle, TFunction)` : `PushModal(FindScriptClass())` puis SetUp ; `FindScriptClass`, `YesLabel`/`NoLabel` (LOCTEXT garde le namespace `HOLConfirmDialog`, cles Yes/No inchangees), `MakeConfirmColumn`, `AddConfirmText`, `AddConfirmRow`, `AddConfirmButton`, `Confirm`, `Cancel`, proprietes `Question`/`ConfirmButton`/`CancelButton`, evenement `BP_BuildDialog`.
  - Ecart a la consigne : SetUp n'est PAS en delegue dynamique. Les deux appelants sont des lambdas C++ (quitter le jeu, abandonner) ; le script n'appelle jamais SetUp. SetUp reste prive, TFunction, atteint par `Ask`.
- Script `Script\UI\HOLConfirmDialog.as` (`UHOLConfirmDialogScreenScript`, ~35 l.) : colonne, question, deux boutons ; clic Oui -> `Confirm`, Non -> `Cancel` ; Non a le focus (NativeGetDesiredFocusTarget en C++).
- Test `Script\UI\HOLConfirmDialog_Test.as` : `Test_ConfirmDialogScreenScriptClass` (classe trouvee, nom `HOLConfirmDialogScreenScript`, libelles Oui / Non).
- Appelants migres (les 2 seuls) : `HOLTitleScreen.cpp` (AskToQuit), `HOLGameMenuScreen.cpp` (AskToSurrender). `HOLConfirmDialog.cpp/.h` SUPPRIMES (107 l.).
- Placeholders.txt relu : aucune ligne « dialogue de confirmation » (rien a retirer).

## Champs
- BUILDS_DEMANDES: 0 (la session principale compile)
- TESTS_CPP_RETIRES: 0 (aucun test C++ ne visait le dialogue)
- TESTS_SCRIPT_AJOUTES: 1 (Test_ConfirmDialogScreenScriptClass)
- LIGNES_CPP_SUPPRIMEES: 107 ; LIGNES_CPP_AJOUTEES: ~220 (base + liaisons)
- LIGNES_MIGREES_PAR_LOT: ~30 (BuildLayout + clics vers le script)
- TYPES_A_EXPOSER: aucun nouveau ; UHorizontalBox/UHOLButton/UCommonTextBlock deja exposes
- TEST_ROUGE: PAS VU. Apres build : fausser une fois "HOLConfirmDialogScreenScript" (ou "Oui") dans le test, voir rouge, remettre.
- EDITS_REFUSES: 0 ; PROCESSUS_DETACHES: aucun lance

## Commande pour la session principale
`C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\build.ps1 -LiveCoding=0 -Wait`, puis `quick_check -Parts tests,ui` ; verifier Test_ConfirmDialogScreenScriptClass, puis un essai : Titre > Quitter (dialogue Quitter / Non) et menu en match > Abandonner.

## Reste
- [ ] Build + rouge vu (ci-dessus).
- [ ] `Content\Localization\Game\Game.manifest` et les .po citent encore `HOLConfirmDialog.cpp(20/21)` : se regenere au prochain gather, les cles ne changent pas.

## NON VERIFIE
- Compilation C++ ; chargement du script ; syntaxe Angelscript (`Button == ConfirmButton` entre UHOLButton et UCommonButtonBase, `FText()` ; si refuse : `Button.GetName() == n"ConfirmButton"`).
- Les proprietes `Question`/`ConfirmButton`/`CancelButton` (UPROPERTY BlueprintReadWrite, non BindWidgetOptional) accessibles depuis le script comme `Title`/`FocusTarget` de la banniere.
- Ordre : le SetUp de `Ask` s'applique apres PushModal, donc apres BuildLayout (comme avant : le widget existe deja).
- Aucun scenario ni script ne cite le dialogue par classe (Grep hors .md/.archive/.po : rien).