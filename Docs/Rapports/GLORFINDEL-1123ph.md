# GLORFINDEL-1123ph : scenario faction_aim_free (essai de visee libre D897)

Impact joueur : l'utilisateur peut enfin viser lui-meme la Lumiere sacree avec sa souris (essai nouveau), et voir le soin sur son heros et la jauge.

- Commits (branche ability-columns, worktree HeroesOfLiberty-Glorfindel) : 4ea80d0 (scenario .txt/.look/.json), 6f53e2d (attente de la jauge apres le soin), 8ddfb09 (.look). Dernier commit = dernier rejeu : preflight faction_aim_free PASS (49 s), 3 rejeux faits au total (1 FAIL : waitlog "gauge 75" alors que le soin rend de l'Honneur -> 85).
- Sans C++, aucun build, aucun changement de Source\ ni Script\. BUILDS_DEMANDES: 0. TESTS_SCRIPT_AJOUTES: 0.
- waitkey : fenetre visible = attend Entree sans limite (panneau du scenario), `quit` ignore (l'utilisateur joue). Sous preflight / -Hidden / -unattended = passe tout seul apres 1 s ("Scenario key: go on (auto)"). Donc le lancer scripte de controle (apres le 3e waitkey, `wait 21` pour la recharge de 20 s) tourne AUSSI en fenetre visible apres le dernier Entree : il est annonce par une note.
- Recharge : AUCUNE commande n'existe pour remettre la recharge a zero (Grep Script\ et Source\ : seulement SetPowerCooldownEndMs). Le waitkey dit d'attendre 20 s apres un lancer reussi. Un refus (trop loin) ne consomme ni Honneur ni recharge.
- Etat prepare : `hol.Cheat.Wound 40` blesse TOUTES les unites du joueur sauf le roi (11 unites : 5 Hommes d'armes + paysans + heros) a 40 % ; Honneur 100 ; pas de banniere ; origine = armee.

## Ce que montre chaque PNG (vu, Read, 1066x600)
- faction_aim_free_ready : le Forum au centre, 5 petits soldats bleus et le heros juste sous sa facade, camera haute : les soldats sont minuscules, AUCUNE barre de vie de soldat lisible a cette echelle. Lisible : panneau heros "PV 180 / 450" (barre verte entamee), jauge "100 / 100 Lumiere sacree : cout 25, pret". Un grand cercle fin pale (rayon ~180 px) centre sur l'armee est visible (non identifie : probablement le repere de portee/selection, pas ma pose). Panneau du scenario "en preparation" (la note arrive apres la capture).
- faction_aim_free_after : deux anneaux dores (6 m et 3 m) poses legerement au-dessus-droite du groupe (point (-17026, 293), clic scripte 5 m d'ecart ; a cette echelle l'ecart se voit peu), la note et la consigne affichees en haut, jauge "85 / 100" et case "recharge 18 s", panneau heros "PV 245 / 450" (barre verte plus longue). Barres de vie des soldats : NON lisibles a cette echelle (soldats ~12 px).

## Le soin est-il VISIBLE ou LISIBLE : OUI, lisible
- Journal : `Heal report: ... healed total 0.00` avant, `healed total 507.00` apres la zone (le journal du rapport existe, defaut (b) "aucun log de PV de la zone" : FAUX sur ce build, le total de PV rendus est lu par hol.Heal.Report).
- `Hero sheet slot 0 hp 180` avant, `hp 245` apres (hol.Hero.Report) et dans le panneau du heros a l'ecran.
- L'Honneur monte 75 -> 85 apres le soin (le soin rend de l'Honneur, +10 sur 507 PV). A noter : le scenario attend donc "slot gauge 8" (80 a 89), plus "75" exact.
- NON visible : barre de vie de chaque soldat (trop petite) ; aucune marque "+" (le dessin du soin du Moine n'est pas celui de la zone).

## NON VERIFIE
- Aucun clic humain a la souris : la visee libre n'est pas rejouee ici (preflight = clic scripte).
- Le refus a plus de 12 m n'est pas dans le scenario scripte (couvert par les tests 2177).
- La camera trop haute pour voir les barres des soldats : un cadrage plus serre (hol.Camera.Zoom) n'a pas ete essaye (budget d'appels).
- `-Hidden` : le 3e Entree auto puis `wait 21` ; en fenetre visible la duree depend de l'utilisateur (3 tours + 21 s).

## A FAIRE et VOIR (texte propose pour la fiche E, a creer par la recette sur main)
Statut propose : VU A L'ECRAN (ready + after vus ; la visee a la souris elle-meme reste a jouer).
Commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\play.ps1 faction_aim_free   (puis chemin du worktree/main selon l'integration)
1. Attends le panneau "pret" : le heros a "PV 180 / 450", la jauge Honneur dit 100 / 100.
2. Clique la case "Lumiere sacree" (panneau en bas a gauche) : le curseur passe en visee.
3. Clique le sol A COTE de tes soldats (5 a 8 m) : un anneau dore (6 m) apparait SOUS ta souris, pas sur l'armee. L'Honneur passe a 75 puis monte a ~85 ; "PV" du heros remonte (180 -> ~245), la case affiche la recharge 20 s.
4. Appuie sur Entree (Honneur remis a 100, tout le monde de nouveau blesse). Attends la fin de la recharge, clique la case puis le sol TRES LOIN (plus de 12 m de ton armee) : le jeu refuse (pas d'anneau, Honneur inchange, pas de recharge).
5. Entree, 3e lancer au choix ; Entree : un lancer de controle automatique suit apres 21 s (anneau dore a 5 m du groupe).
Defaut a signaler le cas echeant : l'anneau doit etre sous la souris ; si il se pose sur l'armee, c'est un BUG.