# RECETTE-1120pb (2026-11-20, 17:04-17:12) : worktree C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, integ-30 = main 2868866

| Essai | Pré-vol | Fiche |
|---|---|---|
| deploy | FAIL (9) | non |
| chat_input | FAIL (4) | non |
| virtuous_knight | FAIL (5) | non |

Aucune fiche `Retours\E-*.md` créée (aucun PASS). Rien corrigé. `stray_processes.ps1 -Stop` lancé avant chaque pré-vol et à la fin : « aucun processus detache ».

## deploy : FAIL (progrès partiel, cause JEU)
- Étape 77 `waitlog D=Pack` expire (60 s). Cycle 1 : OK. HUD report frame 119 `D=Mounting0`, frame 345 `D=Pack` ; `deploy_deployed.png` (vu) : case D « Démonter », Mangonneau en tir. `deploy_packed.png` (frais 17:05) : case D « Monter ».
- Défaut 1 (D869b, montage) : `deploy_deploying.png` (vu) : `Montage… [--------]`, 0 dièse, prise 34 images (0,3 s) après l'ordre (phase 0->1 frame 86, capture frame 120, fin du délai frame 225 : 24 % attendu, au moins 1 dièse). Le .look n'est pas satisfait.
- Le démontage progresse : `deploy_packing.png` (vu) : « Démontage… [##------] » (HUD report frame 167 `D=Dismounting2`). Seule la barre du MONTAGE est figée à 0.
- Défaut 2 : 2e cycle de montage : phase 1->2 « delay over » frame 572, HUD report frame 573 `D=Mounting6` (étiquette pas repassée à « Démonter », aucun `D=Pack` ensuite) : l'étape 77 expire. Reproduit le défaut du 16:52 (la case reste sur Montage…), le correctif 9469ade n'est donc que partiellement effectif (cycle 1 passe, cycle 2 non ; barre du montage à 0 à 0,3 s).
- PNG fraîchement pris et vus : deploy_packed, deploy_deploying, deploy_deployed, deploy_packing. Non lus : fire, fire_hit, packed_again. Non pris (étape bloquée, anciennes captures de 12:54 périmées) : deploy_move_start, _unmounting, _walked, deploy_attack_mounting, _mounted, _hit.

## chat_input : FAIL (accents corrigés, 2e Entrée toujours KO, cause JEU)
- Corrigé et VU : `chat_input_shown.png` : fil en haut à droite « Vous : (Alliés) salut les allies », accents corrects ; journal `Chat message: Vous : (Alliés) salut les allies (slot 0)`. mapSize Small : pré-vol `OK map: generated`, minimap en jeu verte.
- Non corrigé : étape 39 `waitlog chat: open` expire. Après l'envoi (frame 395), l'Entrée 2 est `Key pressed: Enter (focus: none)` frame 433, `Key released ... down not handled, up not handled`, et le HUD logge `chat: closed (last sent 'salut les allies')` (frame 551) : le champ ne se rouvre pas. e3d1969 sans effet constaté (focus none : l'Entrée n'est traitée par personne). `/w Joueur 2 salut`, clic DipName_1 et `chat_input_target` jamais joués.
- PNG vus : chat_input_shown (accents OK, panneau Diplomatie ouvert au milieu). Non relus : typing, typed (WARN frames 90 > 30 sur la ligne 14 du .txt).

## virtuous_knight : FAIL (cause SCÉNARIO, sélection par index)
- Étape 37 `waitlog Selection: VirtuousKnight` expire. Journal : `SpawnHere 1 VirtuousKnight`, `3 Spearman slot 1`, puis `Selection: Peasant #1 of slot 0 (own)` : `hol.Select.ClickUnit 0` désigne le Paysan de départ (index 0 de l'Aube), pas le Chevalier (index à vérifier : 0 = 1re unité du slot, déjà un Paysan). Le recentrage caméra de 1a2e19c (FocusNear juste avant le clic) est bien appliqué mais ne règle pas l'index. Le .look dit lui-même « index à vérifier, non joué ».
- PNG vu : `virtuous_knight_grid.png` : case E « Chevalier Vertueux », A « Hallebardier » (actif), Z « Homme d'armes », W « Hallebardes trempées Bientôt » : les trois libellés du .look sont CORRECTS (corrigé depuis 1120pa). Cases Z/E grisées (peu contrastées), Hallebardier seul actif ; un texte parasite « Shader en préparation (2) » recouvre le bouton +1000 Bois en haut à gauche.
- Non vus : anneau de 8 m, PV du Chevalier qui baissent (before et hp non pris). Le geste réel de la touche E n'est pas joué (spawn direct), waitkey = attente utilisateur.
