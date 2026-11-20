# NETTOYAGE-1120-file : treizième nettoyage de Auto_Journey_File.md (2026-11-20, étape 0a, main = 410cb7d)

Fichier nettoyé : `Auto_Journey_File.md` (134 lignes avant, 119 après). Archive (copie intégrale AVANT nettoyage, fichier neuf) : `Docs\Archive\Auto_Journey_File_faits_2026-11-20.md` (134 lignes, 38467 octets, identique à l'ancien fichier). Fichier vivant : UTF-8 sans BOM, 119 lignes en CRLF, 0 LF seul (l'ancien était en LF seul sur le disque : une session avait écrit en LF ; git a `eol=crlf`, aucun bruit de diff). Aucune ligne `[x]` restante. Aucun code, aucun build.

## Retiré (numéros = lignes avant nettoyage ; tout est dans l'archive)
| Ligne | Contenu | Motif et preuve |
|---|---|---|
| 24 | `[x]` integ-24, 8 règles pures en script | `[x]` |
| 32 | `[x]` integ-25, morceaux 1 et 8 (DANS main df895b0) | `[x]` |
| 52 | Celeborn : 2 rouges NomadFirstForum / NomadReplicated | périmée : ed5947b (tests lisent le cvar), 5c7555d (known_reds.txt purgé, tests 1525/1525), 4a231ac ; `HOLStartingHeroTests.cpp` lit `hol.Nomad.ForumCell` |
| 58 | Elrond : specialization_button ÉCHOUE sur 2bd5fd3 | périmée : fc10863 (scénario corrigé), c1faa77 « two trials replayed green on main and listed » |
| 63 | note ELROND-1115j « arrêt, correctif à écrire » | redite dépassée par la note ELROND-1116a gardée (correctif fait) |
| 68 | Faramir : D827 + D828 Backspace des scénarios `_compare` (WIP non compilé fog de79413) | 53c106f et 596460d DANS main (`git merge-base --is-ancestor`), « Backspace previous option done » (80cb749), `HOLTestSequence.cpp` contient Backspace |
| 81 | Glorfindel : jumeau C++ `HOLChatRules.cpp` (patch D806, arrêté 2026-11-16) | périmée : aucun fichier `HOLChatRules.cpp` dans `Source\`, D839 interdit les jumeaux C++/script |
| 92 | Haldir : D825 signaux plus visibles (H1 faite, reste minicarte animée + Alt + clic minicarte) | redite de la ligne D860 de « Socle commun » (même reste) ; H1 d4c6155 |
| 99 | Haldir : Nomade N2 historique (forum IA achevé en 1 pas, D816/D817) | redite : cause corrigée par 97f757c (DANS main), reste porté par les lignes D816 et D817 gardées |
| 100 | HALDIR-1115j signals_altclick carré VU | note sans case ; 5a75423 DANS main ; redite de la ligne D860 |
| 111 | SAURON-1116c D820 suite (88dd5c2) | note « à fusionner » : 88dd5c2 DANS main ; D809 déjà dans sa ligne `[ ]` ; V9c au journal `VolcanoInProgress.md` |
| 112, 113, 114 | SAURON-1115c, 1115d, 1115e (XP d'exploration B2, « à fusionner ») | 960714b, 3d94384, 3c02c56 DANS main ; reste porté par la ligne D809 gardée |
| 132 | GLORFINDEL-1115e transport du chat A4 VERT 8b8e309, Q-chat-ui [À RÉPONDRE] | 8b8e309 DANS main ; Q-chat-ui répondue (D807 = A, ligne gardée) |

Ligne 2 (historique des nettoyages) remplacée par une ligne courte annonçant le treizième nettoyage et les deux archives.

## Gardé en tête : section « Socle commun des factions »
Ordre actuel après nettoyage : D861 (démonté + attaque), D861 (monté + déplacement), D860, D859, D858, puis le lot C++ groupé et ses suites. Les 4 décisions sont donc en tête de la section, dans l'ordre D861, D860, D859, D858. La section vient juste après « Prochaine séance » (lignes 10-18), qui ne contient que des points sans case `[x]` ni preuve de péremption.

## Tableau des doutes (lignes gardées, à confirmer par la session principale)
| Ligne (section) | Doute | Proposition |
|---|---|---|
| `[~]` MIG-C-MatchConfig (Restes de migration) | texte dit « retrait des 3 » alors que PlayerColors et NeutralUnitColor sont retirés (3ec9b28, c5d6938 dans main) ; SubsystemFile reste en C++ exprès ; build + rouge vu des ports toujours « NOT COMPILED » | réécrire « retrait des 2 » ; archiver après un build montrant les ports verts |
| Note MIG-C++-FINAL point 3 | `HOLMapRulesTests.cpp`, `HOLStartingPeasantsTests.cpp`, `HOLFactionChoiceTests.cpp` existent encore ; MatchLossText, GameNames, Market.Table non revérifiés | rejouer la liste avant d'archiver |
| Haldir : `(donnees FAITES, C++ reste ; D816 ...)` | le reste « test C++ `MainCentre: 150 s for one builder` » vise `HOLConstructionTests.cpp`, qui n'existe plus ; aucun port script trouvé par grep (`Script\Tests`) | transformer en tâche script (test de BuildTime 150 s) ou archiver si le test est jugé inutile |
| Galadriel : alertes mineures D804 | dit « pas dans main » : a11ed2a est DANS main | corriger le texte, le reste (lignes recherche/bâtiment, ping minimap) tient |
| Galadriel : Population pleine D833, infobulle de revenu D814 | texte « (C++) » ; 0688481 (infobulle D788) dans main, état du style D814 non relu | relire `AlertsMinorInProgress.md` / `ScenariosInProgress.md` |
| Elrond : SP3 bouton, F7 lots 3-4, note ELROND-1116a | mélange « DANS main » et « suivant » ; 3949fd5, 76ab50a, a28395b, 9e010ac DANS main | relire `SpecializationInProgress.md` et `FortificationsInProgress.md` |
| Sauron : lignes Arquebusier (VOIE A, Modèle et animations) | deux lignes qui racontent la même histoire (b61178a FUSIONNÉ, 8fd7cef dans main) ; `Prochaine séance` point 5 la répète | fusionner en une ligne après décision de l'utilisateur sur la voie |
| Sauron : deploy « Mangonneau sur la crête » (SAURON-1116d) | proposition `FocusNear 3000 1500` jamais appliquée ? D861 (essai deploy) touche le même scénario | relire `deploy.txt` avant de garder |
| Note 2026-11-17 écran Réglages | essai OK (4e18ee5), mais « Multijoueur reste à essayer » | archiver quand Multijoueur est joué |
| Celeborn : D641 mêlée et D617 groupe | lignes très anciennes (2026-11-14), « à écrire en Angelscript » | confirmer qu'elles sont toujours voulues |
| Haldir / Glorfindel : `bSitesLaidByMaps`, Sites sacrés S5-S7, Légions modèle client | non revérifiées (hors budget) | relire les journaux |

## Appels
Environ 12 appels d'outils vus par moi (limite 25), fin à 08:53 (arrêt dur 09:08).
