# RECETTE-1123pc : essai faction_aim_free (visee libre de la Lumiere sacree, D897)

Impact joueur : l'utilisateur peut poser la Lumiere sacree a l'endroit de sa souris ; l'essai est liste, mais un clic hors portee ne dit rien a l'ecran.

VERDICT : PASS. Fiche `Retours\E-faction_aim_free.md` ecrite (VU A L'ECRAN, GESTE: reel, SON: non entendu, branches: main).
- Preflight : 2 passages PASS (53 s puis 46 s apres retouche), 2 PNG ouverts.
- Retouche : commit 731560e (branche du worktree Haldir, `Agent: RECETTE-1123pc`) : la note et le waitkey 2 disent que le refus a plus de 12 m est SILENCIEUX. NON fusionne dans main : la session principale doit integrer 731560e (1 fichier : Tools\Scenarios\faction_aim_free.txt) ; la commande de la fiche pointe le worktree Haldir qui le contient.
- Gate Essay : `=> aucun FAIL` (PASS ecart de code 0 / worktree propre / binaire 17:09 >= 16:54 / tip de main contenu ; WARN de rappel seulement).

PNG vus :
- ready : Forum, 5 soldats bleus et le heros sous la facade, jauge 100/100 "Lumiere sacree : cout 25, pret", heros PV 180/450, panneau scenario "en preparation", grand cercle pale (identite inconnue), barres des soldats illisibles.
- after : deux anneaux dores (6 m et 3 m) un peu a droite/dessus du groupe, jauge 84/100, case "recharge 18 s", heros PV 242/450, consigne affichee.
Journal : Heal report total 0 -> 498 ; hp heros 180 -> 245 ; `FactionAim: cast 0 send true slot 0 at (-17026, 293)` ; zone doree 600 cm a (-17026, 293) ; Honneur 100 -> 85.

Defauts du jeu vus :
1. Refus hors portee (>12 m) : AUCUN message a l'ecran. `HOLFactionPowerServer.as:117-120` ne fait que `Log(... "RpcPower: cast refused")` ; la decision (`HOLFactionPowerCast.as:97-107`, refus 3) ne cree pas de feedback ; le client termine la visee apres le clic (`HOLFactionAim.as:54`). Le joueur voit simplement qu'il ne se passe rien. Candidat a un REGLAGE (message de refus, ou point ramene au bord de portee : Retours\Q-lumiere-sacree-visee.md).
2. Le grand cercle pale (~180 px de rayon) est visible AVANT d'armer la visee et apres le lancer : identite inconnue (portee 12 m ? repere de selection ?) ; non verifie.
3. Barres de vie des soldats illisibles a cette camera (le soin ne se lit que par le panneau heros et la jauge).

NON VERIFIE : aucun clic humain (anneau sous la souris jamais rejoue par l'agent, clic scripte a 5 m) ; refus a plus de 12 m non joue (code lu seulement) ; barres de soldats ; grand cercle ; son ; deroule en fenetre visible (3 Entrees puis controle auto a 21 s) seulement en mode auto.

Anomalies : le worktree Haldir est maintenant 1 commit (731560e) en avance sur main ; le gate Essay l'accepte. Aucun fichier de code de jeu touche (un .txt de scenario). Appels : ~18.
