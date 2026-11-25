# HALDIR-1125n-peri : preuves des lignes perimees de Auto_Journey_File.md (relevé H3, execute par GLORFINDEL-1125c, 16:32-16:41)

Pas de test rouge (releve de docs). Methode : `git merge-base --is-ancestor <sha> main` (code 0 = DANS main) + `git grep` sur main. Numeros de ligne revérifies sur le fichier de 16:32 (HEAD f050481). Les 9 lignes sont passees `[x]` avec leur sha par `edit_line.ps1` ; aucune archivee (nettoyage 0a de la prochaine seance).

| l. | Verdict | Preuve |
|---|---|---|
| 17 | PERIMEE, fait | branche `tools-1124si` : tip 867e771 ancetre de main (exit 0) ; fusionnee par dff1e85 (`Merge branch 'tools-1124si' into integ-77`) ; 13a1f44 (les 3 FAIL gate_test) ancetre de main (exit 0) |
| 18 | PERIMEE, fait | branche `arwen-1124sd` : tip 1118093 ancetre de main (exit 0) ; fusionnee par 4f6ddc4 (`Merge branch 'arwen-1124sd' into integ-77`) ; le conflit de HOLDuelLedger_Test.as est donc resolu a la fusion |
| 23 | PERIMEE, fait | `Content/Localization/Game/ja/Game.po` sur main : `msgid "Voie A du palier {0}"` -> `msgstr "段階{0}のルートA"` (l.2855), Voie B (l.2862) ; ajoute par e2b5ab4 ; test `Script/Tests/HOLSpecializationSlot_Test.as:72-73` lit « 段階1のルートA/B » |
| 47 | PERIMEE, fait | D894 colonne faction : rouge aac9d9a (stub HashFactionState) puis ea2c420 `Step hash: faction column ... HashUnits unchanged`, ef5f1ef (membre Faction), ad0288f (correctif de build) ; tous ancetres de main (exit 0) |
| 56 | PERIMEE, fait | S2-c : rouge 8b6fd50, correctif 04b5db0 (`Sacred Light clears the malus around its zone`), ancetres de main ; `Script/Factions/HOLFactionPowerServer.as:452` appelle `HOLMorale::ClearMalusAround(State, Clear.CenterXCm, Clear.CenterYCm, Clear.RadiusCm, Clear.Slot)` ; test `Script/Tests/HOLFactionPowerClear_Test.as` ; C++ `HOLMoraleLibrary.cpp:143` |
| 122 | PERIMEE, fait | colonne `Produces` sur main (CSV parse) : Chapel `("Monk","InfirmaryBrother")`, Barracks `...,"TempleSergeant"`, ArcheryRange `...,"PaviseCrossbowman"` ; 0746888 (Chapel, D902 etape 3) et d43fc39 (Barracks/ArcheryRange), ancetres de main. Reste de la ligne 122 cite par A2 (« Sanctuary : question ouverte ») n'est PAS cette ligne : non touche |
| 154 | PERIMEE, fait | `Placeholders.txt:226` (main) « Pastille de moral ... [style] », GLORFINDEL-1125b H2 ; commite dans f050481 (HEAD de main, `git status --short Placeholders.txt` vide) |
| 187 | PERIMEE, fait (avec une reserve) | marque de l'arret du heros : rouge 09cee26 (`HOLFactionAim_Test.as:69`), correctif f94a172, ancetres de main ; `HOLFactionPowerServer.as:67` `ApproachPointOf`. Scenario rejoue : RECETTE-1125mb sur main 7922539 (f94a172 y est ancetre, exit 0) : `aim_far` cercle ORANGE hors anneau, PASS ; RESERVE de cette recette : un petit anneau blanc isole a ~120 px du centre, « non explique » (probablement la marque d'arret, non confirme) |
| 220 | PERIMEE, fait | outil F8 `Tools/AutoJourney/stale_quickcheck.ps1` (0602b09, ancetre de main) + `stale_quickcheck_test.ps1` ; GLORFINDEL-1124d : test 7 PASS / 0 FAIL, rouge 12d8040. La ligne demandait un grep des identifiants avant chaque passe : l'outil le fait. Le « DIAG apres FAIL » n'a pas de preuve propre |

Hors liste, non touchees : l.15, 16, 19, 20, 21, 22, 24 (non relevees ici, H3 de l'enonce ; a refaire par un agent suivant).

Non verifie : l'EXECUTION de `stale_quickcheck_test.ps1` aujourd'hui (je n'ai lance aucun script qui n'est pas le mien) ; le contenu visuel des PNG de RECETTE-1125mb (lu au rapport, pas rouvert).
