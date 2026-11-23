# NETTOYAGE-1123g (etape 0a, dossier Retours\, 2026-11-23 14:47 -> 14:54, docs seulement)

COMMIT : 284c846 (9 fichiers : 7 de Retours\ + 2 copies d'archive dans Docs\Archive\Retours_2026-11-23\ des versions d'avant de LISEZ-MOI et ESSAIS-A-JOUER). Pas de report au journal d'un chantier : aucune reponse a reporter.

## Archive
RIEN archive : aucune des 7 fiches restantes (5 E, 2 BUG) n'est repondue ni jouee. Les 7 Q du matin etaient deja archivees (D890 a D895). Aucun `git mv`, aucun refus du classifieur.

## Fiches E : statut, commande, branche
- E-deploy : statut de la 1re ligne `VU A L'ECRAN` contredisait `GESTE: interne` (regle L7-6) : CORRIGE en LOGIQUE SEULEMENT ; titre `BUG EN FILE D877` perime (D877 codee dans main : 6baa6ba, 49363b6, 90ff8d3, 593c606 ; RECETTE-1122c PASS VU A L'ECRAN sur integ-52 edc631a, dans main) -> `D877 CODE DANS MAIN, GESTE INTERNE`. Commande Arwen en chemin absolu, worktree existant, contient ces commits et 0e4b0c5. Manque : pre-vol sur une copie a jour de main, selection et camera au geste reel, recette independante.
- E-talents_heal : LOGIQUE SEULEMENT ; note `branches: zone-effect` perimee (64a212a, 4e58051, 2a11ec2, 4146ec1, 0e328e9 sont dans main). Commande Sauron complete. Manque : trait +/++ hors DrawDebugString (D886, ligne [ ] File l.118, C++), geste reel, son.
- E-tower_wood : LOGIQUE SEULEMENT ; la precision `Sauron ne contient pas integ-41` est perimee (0d1c150 et af8ff83 sont dans main et dans Sauron). Commande jouable. Manque : fleche EN VOL (lot C++ File l.46 : journal au lancer + ArrowScale), geste reel, son.
- E-wall_line_long : LOGIQUE SEULEMENT ; commande Sauron = integ-55 avec EndWallDrag 68c86d0 (message `NOT COMPILED`, absent de main) : dit maintenant en PREMIERE ligne. RECETTE-1123a (3) avait vu FAIL (1 fondation) avant la refonte de la fiche (8 fondations). Manque : integration + compilation de 68c86d0, pre-vol sur main, geste reel D + glisser.
- E-playtest_hero_army : LOGIQUE SEULEMENT ; commande Elrond (integ-25) complete, worktree existant. Manque : correctif de la mele (D641, BUG-playtest_hero_army-spin.md, stall #18 a ~110 cm de la portee) + cadrage A/B.
- BUG-melee-spin-sanity, BUG-playtest_hero_army-spin : gardes (defaut garde par le jeu, D641, aucune decision de l'utilisateur enregistree dans D641/D815 sur leur priorite).

Chaque fiche E a une ligne `NETTOYAGE-1123g` en bas (E-deploy, E-wall_line_long, E-tower_wood, E-talents_heal, E-playtest_hero_army).

## Index (LISEZ-MOI.md et ESSAIS-A-JOUER.md recomptes)
9 fichiers ; [A REPONDRE] 0 ; [A ESSAYER] en titre 3 (talents_heal, tower_wood, wall_line_long) tous LOGIQUE SEULEMENT ; prets VU A L'ECRAN 0 ; LOGIQUE SEULEMENT 5 ; PAS PRET 1 (deploy) ; BUG 2.

## Doutes laisses en place
- Les 2 BUG-* : ouverts, mais pas de fiche d'attente utilisateur ; garde faute de reponse enregistree. A trancher par la session principale (archiver comme faits-et-mesures ou garder).
- Les 3 titres `[A ESSAYER]` alors que LOGIQUE SEULEMENT : non retitres (la 1re ligne de statut dit la verite ; Auto_Journey_Retours.md les compte deja a part).
- Auto_Journey_Retours.md l.19 dit `le worktree d'Arwen n'a pas main` : FAUX d'apres git (Arwen contient 49363b6, 0e4b0c5, af8ff83) ; hors de mon perimetre, non touche.

## NON VERIFIE
DLL / binaire a jour des worktrees Arwen, Sauron, Celeborn, Elrond (aucun pre-vol lance) ; aucun scenario rejoue ; le contenu exact de `integ-52` au regard de main (edc631a est ancetre de main : verifie).

## Anomalies
- `Rd` est un alias PowerShell (Remove-Item) : mon premier script d'edition l'a appele, erreur sans effet (chemin inexistant), renomme. `git add`/`commit` ont affiche des avertissements LF/CRLF (sans effet). Les fiches E-* ont des fins de ligne melangees (LF d'origine, mes lignes en CRLF). EtapeDefini.md : 1 ligne ajoutee, NON commitee (comme le rapport).
