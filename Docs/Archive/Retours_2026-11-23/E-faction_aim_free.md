# [À ESSAYER] E-faction_aim_free : vise toi-même la Lumière sacrée avec la souris (clic sur la case, puis clic au sol) et vois la zone dorée se poser sous ton curseur et soigner tes soldats blessés

STATUT : VU À L'ÉCRAN
GESTE: réel
SON: non entendu
branches: main

Règle (L7-6) : un essai de GESTE ou de SON reste LOGIQUE SEULEMENT tant que la ligne dit `interne` ou `non entendu`. (Ici le statut vu à l'écran porte sur le lancer de contrôle scripté ; la visée à la souris est le geste que TU joues, l'agent n'a cliqué avec aucune souris : `SON: non entendu` signalé, aucun son attendu de cet essai.)
`session_gate.ps1 -Phase Close` signale (WARN) tout E-*.md `[À ESSAYER]` sans les lignes `GESTE:` et `SON:`.

Pré-vol (2026-11-23 17:13, agent RECETTE-1123pc, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, main 3b6d29c + scénario, tip 731560e) : `preflight.ps1 faction_aim_free` PASS (46 s, 2 PNG), 2 passages (le 1er à e94285c, le 2e après la retouche de la note).
PNG OUVERTS et vus :
- faction_aim_free_ready : le Forum au centre, 5 petits soldats bleus et le héros juste sous sa façade, aucune bannière plantée ; panneau héros « PV 180 / 450 » (barre verte entamée) ; jauge « 100 / 100 Lumière sacrée : coût 25, prêt » ; un grand cercle fin pâle (rayon ~180 px) centré sur l'armée, NON identifié ; panneau du scénario « en préparation » (la consigne arrive après la capture). Les barres de vie des soldats ne sont pas lisibles à cette échelle.
- faction_aim_free_after : deux anneaux dorés (6 m et 3 m) posés un peu à droite et au-dessus du groupe (clic scripté à 5 m) ; jauge « 84 / 100 » (le soin rend de l'Honneur : 75 puis +~10) et case « recharge 18 s » ; panneau héros « PV 242 / 450 » (barre plus longue) ; la consigne s'affiche en haut. Barres de vie des soldats toujours illisibles (soldats ~12 px).
Journal : `Heal report` total 0 avant, 498 après ; `Hero sheet slot 0 hp 180` puis `hp 245` ; `FactionAim: cast 0 send true slot 0 at (-17026, 293)` ; `Sacred Light placeholder: golden ring of 600 cm for 5.0 s at (-17026, 293)` ; Honneur 100 -> 85 (-25 coût, +10 soin).
Refus à plus de 12 m : NON JOUÉ par le scénario (le contrôle scripté ne tire qu'à 5 m). Code lu : `Script\Factions\HOLFactionPowerCast.as:97-107` (portée 1200 cm = refus 3) et `Script\Factions\HOLFactionPowerServer.as:117-120` : le refus n'écrit qu'une ligne de journal (`RpcPower: cast refused`), AUCUN message à l'écran, ni Honneur dépensé ni recharge ; côté client la visée se termine après le clic (`HOLFactionAim.as:54`). Ce que tu verras : rien ne se pose. La note du scénario le dit maintenant (commit 731560e).
NON VÉRIFIÉ : aucun clic humain à la souris (la visée libre, l'anneau sous TON curseur, n'est jamais rejouée par l'agent) ; le refus à plus de 12 m (non joué, code lu seulement) ; les barres de vie des soldats (trop petites à cette caméra) ; le grand cercle pâle (identité inconnue) ; le son (non entendu) ; le déroulé en fenêtre visible (3 tours avec Entrée puis lancer de contrôle automatique après 21 s) n'est joué qu'en mode auto (preflight).

Gestes précis :
1. Attends que le panneau du scénario dise « prêt » : héros « PV 180 / 450 », jauge « 100 / 100 ».
2. Clique la case « Lumière sacrée » (panneau en bas à gauche) : la visée s'arme.
3. Clique le sol à 5 à 8 m de tes soldats : une zone dorée (6 m) doit se poser SOUS TA SOURIS (pas sur l'armée). L'Honneur passe à ~85, les PV du héros remontent (180 -> ~245), la case affiche la recharge de 20 s. Si l'anneau se pose sur l'armée au lieu de ta souris : BUG.
4. Entrée (Honneur 100, tout le monde blessé de nouveau), attends la fin de la recharge de 20 s, clique la case, puis le sol TRÈS LOIN (plus de 12 m de l'armée) : refus silencieux attendu (aucune zone, Honneur inchangé, pas de recharge). Si tu veux un message à l'écran, dis `REGLAGE: message de refus`.
5. Entrée, 3e lancer au choix ; Entrée : un lancer de contrôle automatique suit après 21 s (anneau doré à 5 m du groupe).

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle, jamais « même commande avec… ».
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 faction_aim_free -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
Durée : environ 3 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

Reponse: OK mais je n'ai pas de previsu (utilisateur, 2026-11-23 18:0x)
