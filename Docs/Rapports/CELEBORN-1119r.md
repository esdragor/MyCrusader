# CELEBORN-1119r : preuve du panneau de scenario (C++ -> script), 2026-11-19

Agent de preuve, 09:45 a 09:54. Celeborn = script (maps, 21ac199). Arwen = C++ (main, gele, aucun fichier suivi touche).
Methode : scenario jetable `zz_echec_voulu` (.txt/.json mapSize Small/.look), non suivi, copie dans les deux worktrees puis supprime.
Les deux passages utilisent le MEME `preflight.ps1` (celui de Celeborn) avec `-Root` different : seul le binaire/jeu change.

## A. Scenario qui reussit (script)
`preflight.ps1 ecran_hud_joueurs` (Celeborn) : ligne finale `preflight ecran_hud_joueurs : PASS (list the trial only once every PNG above has been opened and seen)`, exit 0.
Controles : `OK no HUD panel covers another (2 report(s))`, `OK no visible widget covers another (2 HUD rects line(s))`, `OK no missing asset, timed-out step or spectator in the log`, `OK sanity audit passed`.
Journal : `LogHOLUI: Display: Scenario panel: scenario ecran_hud_joueurs preparing, 0 notes` / `... ready, 1 notes, waiting for the key` / `... ready, 1 notes`.

## B. Scenario qui echoue expres
Etape qui echoue : `waitlog ZZ_TEXTE_QUI_NEXISTE_PAS_JAMAIS` (timeout moteur 60 s, StepTimeoutSeconds).
Note : `play.ps1 -Hidden` ne rend PAS de verdict (exit 0 meme apres le timeout du script, "hidden game ended after 120.2 s (-TimeoutSec 120)") : verdict pris par preflight.ps1 (exit 1).

| | C++ (Arwen) | Script (Celeborn) |
|---|---|---|
| exit | 1 | 1 |
| verdict outil | `check zz_echec_voulu : FAIL (a step timed out)` ; `preflight zz_echec_voulu : FAIL (4)` | identique |
| ligne d'echec moteur | `LogHOLTest: Error: Test sequence: timeout at step 15: waitlog ZZ_TEXTE_QUI_NEXISTE_PAS_JAMAIS` | identique |
| `FAIL 2 missing asset / failed step line(s)` | oui (timeout + Scenario panel failed) | oui, identique |
| Grep `Scenario panel:` (3 occurrences) | preparing 0 notes ; preparing 1 notes ; `LogHOLUI: Display: Scenario panel: scenario zz_echec_voulu failed, 1 notes` | identique, meme niveau Display, meme categorie LogHOLUI, meme texte |

Le FAIL (4) comprend aussi `user's launch: 3 uncommitted file(s)` (mes fichiers jetables non suivis) : sans rapport avec le panneau.
Aucun DEFAUT GRAVE : la ligne `Scenario panel: scenario X failed` est ecrite au niveau Display, une fois, au meme moment (juste apres le `Error: ... timeout at step`), et LaunchCheck la compte (le decompte "2 lines" le prouve).
Limite : seul le cas d'echec par timeout de step a ete joue (pas une erreur Angelscript ni un `waitui` rate).

## C. Image du panneau ouvert (1920x1080 des deux cotes)
Fichiers : scratchpad `compare\panneau_scenario\` : `cpp_panneau_ouvert.png`, `script_panneau_ouvert.png`, `cote_a_cote_haut_cpp_bas_script.png` (zone 560..1360 x 60..160, C++ en haut, script en bas).
Panneau de scenario (rect 565,70 a 1355,155) : titre en gras "Scenario zz_echec_voulu" + "en preparation...", note, place (sous la barre du haut, centre), largeur, fond gris sombre arrondi, boutons "Relancer" et "x" : IDENTIQUE A L'OEIL ; difference pixel : 15 px sur ~67 000 (seuil somme RGB > 12, max 43), du meme ordre que le bruit du monde derriere (barre du haut 776 px max 41), donc rien dans le panneau.
Barre de ressources, tableau des joueurs (haut droite), mini-carte, boutons Diplomatie/Menu : identiques.
SEULE DIFFERENCE de la capture, hors panneau de scenario : le panneau de TRICHES (haut gauche) est deploye cote C++ (Triches -, 8 boutons + curseurs, 68 741 px de difference) et replie cote script ("Triches +"). Etat par defaut/persiste du panneau de triches (ou reglage utilisateur dans le Saved_Tests de Arwen) : a verifier, ce n'est pas le panneau de scenario ; il n'est pas touche par les scenarios existants a ma connaissance (non verifie).
Affichage de l'ERREUR : aucune capture ne le montre (la capture est prise avant l'echec, la sequence s'arrete au timeout) : NON COMPARE.

## Etat final
Celeborn : `git status --short` = seulement c28_b20/c28_w20/c31_f3/f4/f5/f8 (.json/.txt/.look) non suivis d'un autre chantier ; zz_echec_voulu.* supprimes.
Arwen : `git status --short` VIDE. stray_processes -Stop : aucun processus detache des deux cotes. Aucun build, aucun commit.
