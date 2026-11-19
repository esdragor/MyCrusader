# NETTOYAGE-1119n-retours (etape 0a, 2026-11-19 soir)

Dossier : C:\Users\baudo\Documents\MyCrusader\Retours\ (22 fiches avant, 10 apres). Aucun commit (la session principale commite). Les 12 deplacements sont faits par `git mv` (indexes, non commites).

## Fiches closes, deplacees vers Docs\Archive\Retours_2026-11-19\ (12)
Aucun nom en double dans l'archive, donc pas de suffixe _soir.
| Fiche | Reponse | Report verifie |
|---|---|---|
| E-ecran-multijoueur | OK 2026-11-17 ~22:5x | EtapeDefini.md l. 32 |
| E-ecran-nouvelle-partie | OK 2026-11-19 ~00:40 (ligne Reponse: de la fiche) | Auto_Journey_Jour.md l. 28 |
| E-ecran-reglages | OK 2026-11-17 ~16:2x | EtapeDefini.md l. 712, Auto_Journey_File.md l. 152 |
| E-ecran-statistiques | OK 2026-11-19 ~00:40 | Auto_Journey_Jour.md l. 28 |
| E-hud-bannieres | OK 2026-11-19 ~00:40 | Auto_Journey_Jour.md l. 28 |
| E-hud-message-court | OK 2026-11-19 ~00:40 | Auto_Journey_Jour.md l. 28 |
| Q-ecran-multijoueur-fond | Q1 A, Q2 B | D843 (GDD.md l. 3644) |
| Q-march-files | A (valeur 2) | D855 (GDD.md l. 3668), EtapeDefini.md l. 6 |
| Q-mig2-s3 | 1A 2A | D842 (GDD.md l. 3642) |
| Q-migration-r9 | reponses directes | D836 (GDD.md l. 3630) |
| Q-terrain-formation | on enleve le ralentissement en foret | D844 (GDD.md l. 3646) |
| Q-tests-monde-mass | A (restent en C++) | D854 (GDD.md l. 3666) |

Aucune reponse n'a eu besoin d'etre reportee par moi dans un journal : toutes l'etaient deja.

## Fiches qui restent (10 fichiers dans Retours\)
Aucune question sans reponse. Rien ne se joue maintenant (0 essai pret).
| Fiche | Type | Date | Attendu de l'utilisateur | Commande / scenario |
|---|---|---|---|---|
| E-signals_altclick | essai | 2026-11-16 | Jouer la marque au sol du signal (Alt + clic) et dire OK / BUG / REGLAGE ; NON PRET : l'integrateur doit refaire le pre-vol sur le main du jour (la fiche le dit) | play.ps1 -> Haldir (forge-lines 2ff2dbf) ; `Tools\Scenarios\signals_altclick.txt` existe dans Haldir : OUI |
| E-specialization_button | essai | 2026-11-16 17:18 | Cliquer la case D pendant le decompte et dire si « N s / Annuler » tient et si l'infobulle cache le bas ; DOUTE (voir plus bas) | play.ps1 -> Elrond (integ-22 2ff2dbf) ; `specialization_button.txt` existe dans Elrond : OUI |
| E-deploy | essai | 2026-11-13 | Monter / demonter le Mangonneau avec D et regarder la Maison perdre des PV ; decision ouverte : projectile visible du coup de siege (Auto_Journey_File.md l. 11) ; commande a refaire | play.ps1 -> Faramir (fog 12ede11, plus ancien que main) avec `Tools\Scenarios\play.ps1` de MyCrusader ; `deploy.txt` existe dans Faramir : OUI ; binaire non verifie |
| E-playtest_hero_army | essai | 2026-11-05 | Rien : LOGIQUE SEULEMENT, ne pas jouer (audit Sanity D641) | play.ps1 -> Elrond ; `playtest_hero_army.txt` existe dans Elrond : OUI ; non lance |
| E-audio_panel | essai | 2026-11-08 | Rien tant que non reevalue : NE PAS JOUER (96 sons, candidats jamais commites ; Arwen) ; D826 et D832 ont depuis elu des sons | play.ps1 -> Arwen ; `audio_panel.txt` existe dans Arwen : OUI |
| BUG-melee-spin-sanity | bug | 2026-11-05 (mesures 11-10, 11-11) | Rien d'urgent ; defaut garde par D641, une decision de priorite (traiter ou archiver) | - |
| BUG-playtest_hero_army-spin | bug | 2026-11-05 (rejeu 11-10) | Idem ; EtapeDefini.md l. 209 dit que le stall #18 etait le scenario, spins restants D641 | - |
| BUG-order_lines_multi_blocs | bug | 2026-11-16 | Rien : ligne [B] de Auto_Journey_File.md l. 82, non corrige ; il faut un agent | - |
Plus ESSAIS-A-JOUER.md et LISEZ-MOI.md (index, mis a jour).

## Doutes
1. E-specialization_button : le brief et SpecializationInProgress.md l. 114 disent « OK le 2026-11-16 », mais ce OK porte sur la version precedente de la fiche, archivee dans Retours_2026-11-16 (14:40). La fiche presente est un rejeu ecrit a 17:18 (debordement corrige, infobulle encore defectueuse), sans ligne Reponse:. Je l'ai gardee. Si l'utilisateur confirme que le OK couvre aussi le rejeu, on l'archive (nom a suffixer _soir : le nom existe deja dans Retours_2026-11-16, pas dans Retours_2026-11-19).
2. E-signals_altclick : meme situation (version H1 jamais jouee, l'ancienne est archivee). Reste.
3. E-audio_panel, E-playtest_hero_army, E-deploy : fiches anciennes, probablement perimees ; aucune reponse ; gardees.
4. Les 3 BUG : pas des attentes de l'utilisateur au sens strict ; gardes car non traites.

## Fichiers modifies
- Retours\LISEZ-MOI.md et Retours\ESSAIS-A-JOUER.md reecrits ; anciens textes copies dans Docs\Archive\Retours_2026-11-19\LISEZ-MOI_avant_nettoyage_soir.md et ESSAIS-A-JOUER_avant_nettoyage_soir.md.
- Auto_Journey_Retours.md non touche (la session principale peut y dire : rien a jouer, 8 fiches gardees, 12 archivees).

## NON VERIFIE
- Aucun scenario lance ; seul l'existence des .txt a ete testee. Fraicheur des binaires des worktrees non verifiee.
- Les lignes `Reponse:` des fiches du 2026-11-19 sont telles quelles dans les fiches ; je n'ai pas relu la conversation.
- EtapeDefini.md, Placeholders.txt et le GDD non modifies (aucune avancee de contenu, deplacements de docs seulement).
