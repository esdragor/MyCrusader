# NETTOYAGE-1119n-file : douzième nettoyage de Auto_Journey_File.md (2026-11-19 soir, étape 0a, main = 2ff2dbf)

Fichier nettoyé : `Auto_Journey_File.md` (154 lignes avant, 118 après). Archive (copie telle quelle, créée neuve) : `Docs\Archive\Auto_Journey_File_faits_2026-11-19_soir.md` (60 lignes).

## Archivé (numéros = lignes avant nettoyage)
| Lignes | Contenu | Remplacé par |
|---|---|---|
| 2 | historique des 11 nettoyages précédents | ligne courte annonçant le douzième nettoyage |
| 21, 22, 23 | 3 lignes `[x]` de « Migration D851 » (integ-23, fin de migration, forge_lignes) | 1 ligne de renvoi sous le titre gardé |
| 25 à 37, 39 à 61 | sous-sections `###` de « Migration D830/D835 » qui ne contenaient que « Section archivée le 2026-11-19 ... » (Glorfindel, Galadriel, Arwen, Elrond, Faramir, Haldir, Sauron, lot C++ de fin, lignes périmées, D840-D844, titre « Prochaine séance de migration ») | 1 ligne de renvoi vers `Docs\Archive\Auto_Journey_File_migration_archive_2026-11-19.md` |
| 63 | `[~]` L17-12 HOLProductionTests : FAIT (HOLProductionTests.cpp n'existe plus, retiré par ef67576 dans main ; ports `Script\Tests\HOLProduction_Test.as` + `HOLProductionQueue_Test.as` présents) | rien |
| 110 | `[x]` Haldir signals_altclick | rien |
| 153, 154 | `[x]` MIG-T6-LOTS et MIG-L2-Ordres | 1 ligne de pointeur sous « Outils » |

Total : 8 lignes de fond (2, 21, 22, 23, 63, 110, 153, 154) + ~34 lignes de titres et de renvois copiées. Plus aucune ligne `[x]` dans le fichier.

Autres changements : suffixe « (C++ : attend la migration D830) » remplacé partout (replace_all) par « (migration finie D857 : à écrire en Angelscript, D839 ; C++ seulement si une liaison manque) » ; plus aucune occurrence de « attend la migration D830 ». « Prochaine séance » points 1 à 7 : tous gardés (aucun ne dit FAIT sans reste).

## Section « ## Restes de migration à relire » (créée, 2 éléments gardés)
- `[~]` MIG-C-MatchConfig (ex-Celeborn) : GARDÉE. `git log` : c5d6938 (8 tests retirés) et 3ec9b28 (PlayerColors et NeutralUnitColor retirés) sont dans main ; il reste `FHOLMatchSubsystemFileTest` dans `Source\HeroesOfLiberty\Tests\HOLMatchConfigTests.cpp` EXPRES (refus du fichier absent, Error attendue que seul le C++ peut déclarer). Le texte de la ligne dit « retrait des 3 » : à mettre à jour. Reste à confirmer : build + rouge vu des ports PlayerColors/NeutralUnitColor (3ec9b28 dit « NOT COMPILED »).
- Note MIG-C++-FINAL point 3 : GARDÉE. `HOLMapRulesTests.cpp:24` (Map.Rules) et `HOLStartingPeasantsTests.cpp:21` (StartingPeasants.Count) existent encore ; HOLFactionChoiceTests.cpp aussi.

## Tableau des doutes (lignes gardées)
| Ligne (actuelle, section) | Doute | Proposition |
|---|---|---|
| `[~]` MIG-C-MatchConfig | voir ci-dessus : 2 des 3 retraits faits, le 3e (SubsystemFile) volontairement C++ | archiver après un build qui montre PlayerColors/NeutralUnitColor verts en script ; réécrire « retrait des 3 » en « retrait des 2 » |
| Note MIG-C++-FINAL point 3 | tests C++ cités encore présents | garder tant que la liste des tests n'est pas rejouée ; vérifier lesquels du point 3 restent (MatchLossText, GameNames, Market.Table non retrouvés par grep de noms de test, peut-être déjà retirés) |
| Glorfindel : « jumeau C++ HOLChatRules.cpp ... ARRETE 2026-11-16 ... a revoir avec le passage en Angelscript » | le pilote Chat est archivé en migration (D857) ; ligne probablement périmée | la session principale confirme ; archiver si HOLChatRules.cpp n'existe plus |
| Glorfindel : « (C++) Légions : modèle client », « Sites sacrés S5-S7 » | lignes `[ ]` anciennes, texte long avec sha de novembre ; reste non revérifié | relire `LegionsInProgress.md` / `SacredSitesInProgress.md` |
| Galadriel : « (C++, D804 Q-alerts-minor = B) alertes mineures » | « pas dans main » (a11ed2a sur galadriel-tab) ; peut avoir été fusionné par la fusion D857 | vérifier `git branch --contains a11ed2a` |
| Haldir : ligne `(donnees FAITES, C++ reste ; D816 ...)` | cite « DANS main 131b77... » et un reste ; état du reste non vérifié | relire `NomadInProgress.md` |
| Sauron : 3 lignes de notes sans case (SAURON-1115c, 1115d, 1115e « a fusionner ») | historiques, probablement fusionnées | archiver au prochain passage après `git branch --contains 3d94384` |
| Elrond : notes sans case `ELROND-1115j`, `ELROND-1116a` ; Haldir : `HALDIR-1115j` ; Glorfindel : `GLORFINDEL-1115e` ; Outils : note 2026-11-17 écran Réglages | notes datées sans case `[ ]`, laissées faute de budget de vérification | à archiver par la session principale si faites |
| Elrond : lignes F7 lots 3-4, SP3 bouton (a28395b DANS main 78891c6) | mélange « DANS main » et « suivant » : le reste est-il encore ouvert ? | relire `FortificationsInProgress.md` / `SpecializationInProgress.md` |
| Celeborn : 2 rouges quick_check NomadFirstForum / NomadReplicated (CELEBORN-1116e) | texte « avec la migration D830 » périmé, état des 2 rouges inconnu | rejouer quick_check tests ; sinon archiver |

## Compte des lignes `[ ]` par worktree (après nettoyage)
Total 51 lignes `[ ]`. « Sans C++ neuf » = heuristique de texte (mot « sans C++ », « script », « cartes », « docs », « données » et ligne ne commençant pas par « (C++ ») ; grossière, car le suffixe renvoie désormais tout vers Angelscript d'abord (D839), donc beaucoup de lignes marquées C++ sont faisables en script si les liaisons existent.
| Worktree | `[ ]` | sans C++ (texte) |
|---|---|---|
| Arwen | 2 | 1 |
| Celeborn | 3 | 0 |
| Elrond | 6 | 1 |
| Faramir | 2 | 0 |
| Galadriel | 4 | 1 |
| Glorfindel | 6 | 1 |
| Haldir | 13 | 5 |
| Sauron | 6 | 4 |
| Outils | 9 | 8 |
| Total | 51 | 21 |
Les lignes `[B]` et `[?]` ne sont pas comptées. Les lignes de « Prochaine séance » (11 à 18) ne sont pas dans ces comptes.

## PROBLEME DE FIN DE LIGNE (à corriger par la session principale)
L'outil Edit a réécrit `Auto_Journey_File.md` avec des fins de ligne LF seules (118 LF, aucun CRLF) ; le mode d'emploi 5/5 exige CRLF. Ma tentative de reconvertir (PowerShell WriteAllText) a été REFUSÉE par le contrôle d'autorisation, je n'ai pas contourné. Pas de BOM. À faire : convertir en CRLF (`edit_line.ps1` ou un autre outil autorisé) avant le commit, ou accepter LF. Le fichier d'archive neuf est en CRLF.

## NON VÉRIFIÉ
Les lignes `[ ]` hors section migration n'ont pas été comparées à git (sauf celles du tableau ci-dessus, par lecture de texte seulement) ; le contenu exact de l'archive de migration `Auto_Journey_File_migration_archive_2026-11-19.md` n'a pas été relu ; pas de contrôle que chaque test C++ de la note point 3 existe encore (seuls 2 noms trouvés).
