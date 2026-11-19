# ELROND-1119bb (correcteur) - reprise 16, 2026-11-19

Worktree HeroesOfLiberty-Elrond, branche integ-17. Commit 44d77e5 (sur 1ace000). Aucun build, aucun quick_check, aucun editeur lance.

Impact joueur : aucun (tests de commandes console de pilote). Les regles script etaient fideles au C++ ; les deux tests se trompaient.

## Rouge 1 : HOLControllerCommandsC9a_Test / Test_ControllerCommandsC9aArguments -> TEST FAUTIF

Assertion en cause (deduite par lecture, le journal ne porte que "::Test_..." sans le texte du message) :
`T.AssertFalse(MoveShortShift.bValid, "AttackMove with one number is refused")` avec `C9aArgs("1", "shift")`.

- Le test pretend « un nombre seul est refuse » mais passe DEUX mots (`1`, `shift`).
- C++ d'origine (`main:Source/HeroesOfLiberty/Player/HOLPlayerController.cpp`, hol.Units.AttackMove, ~l.1459) : refus sur `Args.Num() < 2` uniquement ; `1 shift` est accepte (Y = Atod("shift") = 0), le drapeau shift est cherche a partir de l'index 2 donc absent.
- La regle script (`HOLControllerCommandsC9a.as`, PlanOf) fait exactement cela : `Args.Num() < 2`. Elle est donc correcte.
- Correction du test : le refus est teste avec UN mot (`C9aArgs("1")`), et deux nouvelles assertions fixent le comportement C++ pour `1 shift` (valide, bShift faux). Aucune assertion affaiblie : une assertion corrigee + deux ajoutees.

## Rouge 2 : HOLControllerCommandsC9b_Test / Test_ControllerCommandsC9bTypeLast -> TEST FAUTIF

Assertion en cause : `T.AssertFalse(PlanOf("hol.Select.Type", C9bArgs("Monk", "LAST")).bLast, "`last` is case sensitive as in the C++")`.

- C++ d'origine (hol.Select.Type, ~l.1400 de main) : `Args.Num() > 1 && Args[1] == TEXT("last")` sur un `FString`. `FString::operator==` est INSENSIBLE a la casse dans Unreal (ESearchCase::IgnoreCase) : `LAST` comptait comme `last` en C++. Le commentaire « case sensitive » du test et de la regle etait faux.
- La regle script `Args[1] == "last"` utilise le meme operateur : meme comportement, donc correcte. Coherent avec le rouge observe (« Expected false, got true »), seule assertion de ce test pouvant renvoyer true.
- Correction du test : `LAST` -> bLast vrai (comme le C++), plus une assertion `lastly` -> faux (un autre mot n'est pas `last`). Commentaire faux corrige dans `HOLControllerCommandsC9b.as` (commentaire seul, pas de changement de code).

## Meme defaut dans les autres commandes des deux fichiers ?

Revu par lecture contre le C++ de main :
- Drapeaux `shift` / `ctrl` (C9a) : C++ `HasProofFlag` (IgnoreCase, depuis l'index From) ; script `HasFlag` avec `ToLower() == Flag` : equivalent. Les cas `Shift`, `SHIFT` du test sont corrects.
- GatherOrder : C++ `Equals(..., IgnoreCase)` sur Args[0] et Args[1] ; script `ToLower() != "shift"` : equivalent. OrderLine (`Num < 4`, drapeaux des l'index 4), DropOff, Build, AttackBuilding, RallySite, Garrison, UngarrisonAll : arguments identiques au C++.
- Autres assertions AssertFalse de C9a (shift avant l'index 2, OrderLine sans shift, Gather `shift` seul / `100 shift` / vide, Build sans shift) verifiees a la main : toutes vraies avec la regle script. Pas d'autre cas du meme defaut trouve ; `grep "case sensitive"` dans Script/Player/HOLControllerCommands*.as ne trouve plus rien.

## NON VERIFIE

- Aucune execution : les tests n'ont pas ete rejoues (consigne). La session principale doit rejouer `Angelscript.UnitTests.Player.HOLControllerCommandsC9a_Test` et `C9b_Test` (attendu 1755/1755).
- Le message exact d'assertion n'est pas dans quick_tests.log (seulement `Angelscript: Error: ::Test_...`) ; l'assertion fautive de C9a est deduite par lecture du code (une seule AssertFalse du test peut etre vraie avec la regle script). Si C9a reste rouge apres rejeu, c'est une autre assertion : relire.
- Hypothese : `==` de FString en Angelscript lie bien `FString::operator==` (insensible a la casse). Le rouge de C9b la confirme indirectement.

## Anomalie

- Le journal des tests n'imprime pas le texte des assertions Angelscript (juste `::NomDuTest`), ce qui oblige a deviner l'assertion en cause ; a corriger cote runner si facile.
- Le brief attribuait la tache a « 25 appels » ; environ 20 utilises, dont 3 Edit refuses faute de Read prealable.
