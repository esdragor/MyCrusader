# NETTOYAGE-1116-file : étape 0a, bloc `Auto_Journey_File.md` (2026-11-16)

main = 9c9d22b au moment du nettoyage. Fichier en UTF-8 sans BOM, fins de ligne **LF** (le « Mode d'emploi 5/5 » dit CRLF : c'est faux depuis un moment, non corrigé ici).

## 1. Lignes `[x]` archivées telles quelles (14)

Archive : `Docs\Archive\Auto_Journey_File_faits_2026-11-16.md` (texte copié octet pour octet, section d'origine en titre).

| Section | Ligne (avant nettoyage) | Sujet |
|---|---|---|
| Prochaine séance | 13 | sites_option Oui en geste réel (GALADRIEL-1115d, b9b66f1) |
| Arwen | 26 | audio tour 5 (D801), fdbe318, écoute faite (D826) |
| Arwen | 27 | RECETTE-1115a audio tour 5, résolu ARWEN-1115e/f |
| Elrond | 44 | B1 Garnison F7 lots 1-2 (45be389) |
| Galadriel | 59 | Rage L4 visuel |
| Galadriel | 60 | Infobulle de la Caserne |
| Galadriel | 61 | REGLAGE talents_icons_compare |
| Galadriel | 62 | B6 raccourcis reconfigurables, règle pure |
| Galadriel | 63 | B4 F4/F5 règles pures |
| Glorfindel | 72 | Signals.DefeatedSpectator + netsig3 3e client |
| Haldir | 86 | réseau Nomade NomadSeats (D799) |
| Outils | 121 | gate Close L15-1 + edit_line -Append |
| Outils | 122 | rouge quick_check values |
| Outils | 124 | quick_check values étiquette REGRESSION |

## 2. Lignes marquées `[périmée? <sha>]` (12, non supprimées, état d'origine gardé derrière la marque)

| Section | Ligne | SHA | Raison |
|---|---|---|---|
| Arwen | B3 zones de furtivité `[~]` | d8bbe87 | règle pure verte dans main ; le reste (branchement C++) est la ligne D824 en tête de section |
| Arwen | PREMIER GESTE sync vers ecff29a | 9c9d22b | ecff29a ancêtre de d751-root (pointe 9c9d22b) |
| Celeborn | PREMIER GESTE build + quick_check (« hero-j8-close NON fusionnée ») | 9c3f180 | hero-j8-close fusionnée (fusion 5) ; branche à 9c9d22b |
| Celeborn | Neutres 2/4 `[~]` | 361b190 | la mesure x16 restante est faite (361b190, 8cba66d) ; reste seulement la réponse à Q-march-member-detour |
| Faramir | PREMIER GESTE sync | 9c9d22b | fog à 9c9d22b |
| Galadriel | recompiler le worktree Galadriel (11-14) | b9b66f1 | agents GALADRIEL-1115a..e ont compilé, testé, préflighté (b9b66f1 dans main) |
| Galadriel | PREMIER GESTE sync | 6a77f35 | ecff29a ancêtre de galadriel-tab (pointe 6a77f35, en retard sur main 9c9d22b de quelques commits docs) |
| Glorfindel | PREMIER GESTE sync | 9c9d22b | net-playtest à 9c9d22b |
| Glorfindel | Signaux 3e client réseau (« refonte, PAS aujourd'hui ») | f213c0c | netsig3 à 3 clients fait et dans main (f213c0c, 19651e4) |
| Sauron | B2 XP d'exploration, branchement C++ | 3d94384 | FogPaysTheHero vert dans main ; le reste (lieux) est la ligne D809 en tête |
| Sauron | PREMIER GESTE sync | 9c9d22b | visual à 9c9d22b |
| Outils | AX-7 tour 5 des sons | fdbe318 | tour 5 fait (0e98c8b, fdbe318) ; le tour 6 est la ligne D826 d'Arwen |

## 3. `[~]` d'hier repassées `[ ]` avec « reprise »

- Faramir L15-15 waitkey Backspace : `[ ]` + « reprise : Backspace dans main (53c106f), rejeu des 2 compare et texte du waitkey non faits ».
- Les deux autres `[~]` d'hier (Arwen B3, Celeborn Neutres 2/4) sont marquées périmées (voir 2), pas reprises.

## 4. Réponses de l'utilisateur en tête de chantier

- D825 REGLAGE signals_altclick : présente, 1re ligne de Haldir. OK.
- D826 sons tour 5 / tour 6 : présente, 1re ligne d'Arwen. OK.
- Lignes D en tête aussi présentes : D804, D806-D810, D813, D814, D816-D818, D820, D823, D824. D811 et D821 sont citées dans d'autres lignes (F7 4c, signaux). **Non vues dans la file : D805, D812, D815, D819, D822** : peut-être sans suite de code, à vérifier dans `Auto_Journey_Retours.md` par le bloc Jour/Retours (non lu ici, pas inventé).

## 5. Doutes (laissés en place, à trancher par la session principale)

1. **Travail rouge non commité possible dans Elrond et Galadriel : NON vérifié** (consigne). Elrond : note l. « ELROND-1115j : arrêt ; test ForumGrid étendu vu ROUGE (non commité) » laissée sous la ligne SP3. Galadriel : aucune ligne de la file ne le mentionne.
2. Restes orphelins après archivage des `[x]` : B6 raccourcis (branchement C++ à M4 après Q-rebind) et Nomade réseau (« fin du premier forum non répliquée au client ») n'ont plus de ligne `[ ]` dans la file ; à rouvrir si voulu (les textes sont dans l'archive).
3. Ligne « gestes réels » (Prochaine séance, L14-5) et AX-6 (Outils) : talents_icons_compare, income_tooltip, sites_option faits ; restent nomad_start et le Backspace (ligne Faramir) : partiellement périmées, laissées.
4. Ligne outil L14-7/L14-8 : `Tools\build.ps1` cite déjà `Get-StaleBinaryProblem` (2 fois) ; la partie `frames > 30` de check.ps1 non vérifiée : laissée.
5. Haldir « Nomade N2 » (HALDIR-1115i) : son reste (CSV 150 s + test rouge + rejeu nomad_ai) double les lignes D816/D817 ; « signals_altclick écrit non lancé » est périmé. Laissée.
6. Notes sans état SAURON-1115c/d/e (« à fusionner ») : visual est fusionné (478b6b7, da3054c) ; notes historiques laissées.
7. « Prochaine séance » item 4 (Audio Arwen, `[?]` stone/metal du 2026-11-13) : probablement dépassé, non tranché.
8. Ligne `(docs)` « GDD.md l. 3024 porte un vieux Prochain numéro D214 » : grep sans résultat sur cette chaîne exacte (accents ou ligne déjà corrigée) : non tranché, laissée.
9. Mode d'emploi 5/5 dit CRLF, le fichier est en LF.
