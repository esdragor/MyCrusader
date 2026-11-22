# GALADRIEL-1122f

APPELS_DECLARES: 13 (arret a 22 ; le 1er Edit est arrive au 10e appel, pas avant le 8e : les lectures de la tache ont pris 9 appels)
HEURE_DEBUT/HEURE_FIN: 2026-11-22T18:06:34 / 2026-11-22T18:08:10.1487685+01:00
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (aucun build demande)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: aucun (tache 2 : pas de correctif script, donc pas de test) ; assertit l'effet : sans objet   TEST_ROUGE_SEPARE: non (aucun test ecrit, raison : cause C++)
COMMITS: 2 (5c3a7ce chat_input.look, puis InterfaceInProgress.md) sur specialization-trial, merge_main_fait: non (DLL non verifiee, pas de code, la session principale synchronise)
TESTS: aucun lance (aucun editeur, build ni quick_check) ; TESTS_TAUTOLOGIQUES: 0 ; BUILDS_DEMANDES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; TYPES_A_EXPOSER: aucun ; DIAGNOSTICS_AS: sans objet (aucun .as touche)
ESSAIS: aucun

## Tache 1 : chat_input.look (fait, 5c3a7ce)
Tools\Scenarios\chat_input.look, ligne chat_input_target. Avant : « le champ ... avec la marque « (Prive) » et le NOM du joueur 2 ». Apres : le champ montre le NOM du destinataire (« Joueur 2 : ») et SANS marque « (Prive) » dans le champ.
Decision citee : D807 (GDD.md) : « la ligne recue dit « Bob : (Allies) texte », « Bob : (Prive) texte », « Bob : texte » pour Tous » : la marque « (Prive) » n'existe que sur la ligne RECUE ; D872 : le destinataire est choisi par clic sur la ligne du panneau Diplomatie (le champ affiche donc son nom). Rien d'autre change (les trois autres lignes du .look gardent leurs marques « (Allies) », conformes). Non rejoue (pas d'editeur) : la recette 1122b avait deja VU le champ « Joueur 2 : ».

## Tache 2 : « Bientot » tronque en case W (diagnostic, rien ecrit)
Ou le texte est pose : PAS dans Script\UI (aucun « Bient » dans les .as). C++ : Source\HeroesOfLibertyUI\HUD\HOLGameScreen.cpp:1165-1167 : CellName = Format("{0}\nBientot", Name) (LOCTEXT ResearchSoonCell) pour toute technologie non implementee (Technology->Implemented faux, T-Q10/T-Q11, D601), passee a GridCell->SetCommand.
Pourquoi tronque : la case est un USizeBox fixe 76 x 76 (Script\UI\HOLCommandGridPanel.as:136-138, CellSize() 76.0, miroir de HOLGameScreen.cpp:225). Dans Source\HeroesOfLibertyUI\Widgets\HOLGridCell.cpp:89-99 le nom est un UCommonTextBlock en style UHOLTextStyleHUDTiny, justification centre, SetAutoWrapText(true), dans une UVerticalBox centree verticalement avec 12 px de marge en haut (UnitSlot->SetPadding(0,12,0,0)), cle (touche W) en haut a gauche sur la meme case. « Cloche d'alarme amelioree » (25 car.) se coupe en 2 a 3 lignes dans ~76 px de large, plus la ligne « Bientot » = 3 a 4 lignes : la hauteur depasse les 76 - 12 px disponibles, le bloc centre deborde et la derniere ligne (« Bientot ») est rognee par la case. Ni la taille de police ni la largeur ne s'adaptent au contenu ; pas de cesure possible pour « Bientot » (un seul mot).
Correctif script < 10 lignes : AUCUN. Le texte est compose en C++ (HOLGameScreen.cpp:1166), le rendu du nom est dans UHOLGridCell (C++, widget de base, D839 : exception admise) ; la seule partie script (la taille 76 du USizeBox) est la constante de la grille, que l'on ne change pas seule (elle est le miroir de HOLGameScreen.cpp:225 et fixe GridWidth/BarHeight). Pas de test script possible sur la vraie chaine sans UHOLGridCell compile.
Lot C++ propose (a regrouper dans le lot de fin) : (a) ne plus coller « Bientot » au nom dans HOLGameScreen.cpp:1166 : donner a UHOLGridCell une ligne d'etat distincte (comme la ligne GainLine, HOLGridCell.cpp:109) placee en bas de case et bornee a une ligne ; ou (b) reduire le nom des cases grisees Bientot (nom court) ; ou (c) un UScaleBox / taille mini de police sur UnitBox. Choix (design/visuel) laisse a la session principale : je recommande (a). Test a ecrire apres : test C++ ou script sur la hauteur mesuree de la case (DescribeRects / geometrie de GridCell vs de UnitBox) avec un nom de 25 caracteres. Ligne ajoutee a Auto_Journey_File.md, sous la ligne D872 de Galadriel.
Placeholders.txt:186 liste deja « Technologies grisees Bientot (sans effet) » ; aucun provisoire ajoute ni retire.

QUESTIONS: aucune
FAIT / RESTE: fait = tache 1 ; reste = lot C++ Bientot (ligne [ ] de Auto_Journey_File.md), rejouer chat_input pour revoir la capture target apres coup (pas necessaire).
NON VERIFIE: chat_input.look non rejoue ; la cause de la troncature est lue dans le code (largeur 76, Tiny, auto-wrap, padding 12), pas mesuree a l'ecran (pas de capture, pas d'editeur) ; la taille exacte de la police Tiny (UHOLTextStyleHUDTiny) n'a pas ete lue.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel

ANOMALIES_PROCESSUS: aucune ; 1er Edit au 10e appel (consigne : avant le 8e)
GIT_STATUS_PRINCIPALE: (voir ci-dessous) ; inchangee hors docs voulus : oui (EtapeDefini.md, Auto_Journey_File.md, ce rapport)
 M Auto_Journey_File.md
 M Auto_Journey_Jour.md
 M EtapeDefini.md
?? Docs/Concept_Faction_Theatre_2026-11-18.md
?? Docs/HSR_Aha_Allegresse_Nihilux.md
?? Docs/Priorite_Socle_Factions_2026-11-16.md
?? Docs/Rapports/ARWEN-1122a.md
?? Docs/Rapports/ARWEN-1122b.md
?? Docs/Rapports/ARWEN-1122c.md
?? Docs/Rapports/ARWEN-1122d.md
?? Docs/Rapports/ARWEN-1122e.md
?? Docs/Rapports/ARWEN-1122f.md
?? Docs/Rapports/CELEBORN-1122a.md
?? Docs/Rapports/CELEBORN-1122b.md
?? Docs/Rapports/CELEBORN-1122c.md
?? Docs/Rapports/CELEBORN-1122d.md
?? Docs/Rapports/CELEBORN-1122e.md
?? Docs/Rapports/CELEBORN-1122f.md
?? Docs/Rapports/CELEBORN-1122g.md
?? Docs/Rapports/CELEBORN-1122h.md
?? Docs/Rapports/ELROND-1122a.md
?? Docs/Rapports/ELROND-1122b.md
?? Docs/Rapports/ELROND-1122c.md
?? Docs/Rapports/ELROND-1122d.md
?? Docs/Rapports/ELROND-1122e.md
?? Docs/Rapports/ELROND-1122f.md
?? Docs/Rapports/ELROND-1122g.md
?? Docs/Rapports/ELROND-1122h.md
?? Docs/Rapports/ELROND-1122i.md
?? Docs/Rapports/ETAPE0-1122.md
?? Docs/Rapports/FARAMIR-1122a.md
?? Docs/Rapports/FARAMIR-1122b.md
?? Docs/Rapports/FARAMIR-1122c.md
?? Docs/Rapports/FARAMIR-1122d.md
?? Docs/Rapports/GALADRIEL-1122a.md
?? Docs/Rapports/GALADRIEL-1122b.md
?? Docs/Rapports/GALADRIEL-1122c.md
?? Docs/Rapports/GALADRIEL-1122d.md
?? Docs/Rapports/GALADRIEL-1122e.md
?? Docs/Rapports/GLORFINDEL-1122a.md
?? Docs/Rapports/GLORFINDEL-1122b.md
?? Docs/Rapports/GLORFINDEL-1122c.md
?? Docs/Rapports/GLORFINDEL-1122d.md
?? Docs/Rapports/GLORFINDEL-1122e.md
?? Docs/Rapports/GLORFINDEL-1122f.md
?? Docs/Rapports/GLORFINDEL-1122g.md
?? Docs/Rapports/GLORFINDEL-1122h.md
?? Docs/Rapports/GLORFINDEL-1122i.md
?? Docs/Rapports/GLORFINDEL-1122j.md
?? Docs/Rapports/HALDIR-1122a.md
?? Docs/Rapports/RECETTE-1122a.md
?? Docs/Rapports/RECETTE-1122b.md
?? Docs/Rapports/SAURON-1122a.md
?? Docs/Rapports/SAURON-1122b.md
