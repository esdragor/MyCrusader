# NETTOYAGE-1120-retours (etape 0a, 2026-11-20 matin)

Dossier : C:\Users\baudo\Documents\MyCrusader\Retours\ (7 fichiers avant, 7 apres). Heure : 08:50-08:57 (Get-Date). Aucune fiche close a archiver : le nettoyage du soir du 2026-11-19 (NETTOYAGE-1119n) avait deja vide le dossier, il ne reste que les fiches que la consigne dit de garder.

## Fiches closes deplacees
Aucune (0). Rien n'a ete deplace ni supprime parmi les fiches. Dossier neuf Docs\Archive\Retours_2026-11-20\ cree : il contient seulement les anciennes versions des deux index (copies, comme le 11-19) : LISEZ-MOI_avant_nettoyage_1120.md et ESSAIS-A-JOUER_avant_nettoyage_1120.md.

## Fiches qui restent (5 fiches + 2 index)
| Fiche | Type | Date | Etat verifie | Doute |
|---|---|---|---|---|
| BUG-melee-spin-sanity | bug | 2026-11-05 (mesures 11-10, 11-11) | ouvert, defaut garde par D641 | non : garde (consigne) |
| BUG-playtest_hero_army-spin | bug | 2026-11-05 | ouvert ; EtapeDefini.md l. 629 : stall #18 = scenario, spins restants D641 | non : garde (consigne) |
| BUG-order_lines_multi_blocs | bug | 2026-11-16 | ouvert : Auto_Journey_File.md l. 73 [B], EtapeDefini.md l. 60 | non : garde (consigne) |
| E-playtest_hero_army | essai | 2026-11-05 | LOGIQUE SEULEMENT, preflight FAIL, ne pas jouer | non : garde (consigne) |
| E-audio_panel | essai | 2026-11-08 | NE PAS JOUER ; D826 et D832 ont elu des sons depuis | OUI, petit : probablement perimee (pre-vol du 11-08 sur un main perime, candidats jamais commites) ; gardee par consigne, a reevaluer ou archiver quand l'utilisateur le dit |

## Doutes
1. E-audio_panel : plausiblement a archiver (son panneau a ete depasse par les elections D826, D832), mais la consigne dit de la garder ; je n'ai pas verifie dans le GDD que tous les cues restent couverts.
2. Les 3 BUG ne sont pas des attentes de l'utilisateur au sens strict (aucun essai a jouer) ; gardes parce que non traites.
Aucune fiche douteuse parmi celles qu'il aurait fallu archiver.

## Fichiers modifies
- Retours\LISEZ-MOI.md et Retours\ESSAIS-A-JOUER.md reecrits (etat du matin : 0 a jouer, 0 question, suppression des paragraphes de mise a jour du 11-19 devenus faux ou redondants ; liste detaillee des archives renvoyee a l'ancienne version archivee).
- Docs\Archive\Retours_2026-11-20\ (2 fichiers, copies des anciens index).
- Non touches : Auto_Journey_Retours.md (session principale), EtapeDefini.md, Placeholders.txt, GDD.md (aucune avancee de contenu, ni provisoire ajoute ou retire).

## NON VERIFIE
- Aucun scenario lance, aucun pre-vol : l'existence des .txt de scenario n'a meme pas ete retestee (aucun essai a donner).
- Les references de lignes (Auto_Journey_File.md l. 73, EtapeDefini.md l. 60, l. 629) viennent d'un grep de 08:51.
