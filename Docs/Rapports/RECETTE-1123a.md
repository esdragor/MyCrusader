# RECETTE-1123a (2026-11-23 10:53-11:07, Sauron integ-55, sans C++)

Aucune fiche E ecrite (aucun essai PASS avec effet vu).

## (1) faction_click -> variante faction_click_cast (commits 3e9ebce + 2 suivants sur integ-55) : FAIL
- Vrai clic (hol.Mouse.Click FactionBox0) : OK. Journal "FactionAim: aim 0"; PNG faction_click_cast_aim : cercle de visee (anneau double) autour du centre, case "Lumiere sacree : cout 25, pret", jauge 100/100.
- Clic sol (hol.Select.LeftClickGround) a 600 cm puis 0 cm du heros : "RpcPower: cast refused (4)" = RefusalOutOfRange (HOLFactionPowerCast.as:52). Jauge reste 100/100, pas de recharge, aucune zone. PNG faction_click_cast_after jamais pris (timeout etape 31, attendu 75/100).
- Cause probable (non prouvee) : HOLFactionPowerServer.as:12 prend l'origine de portee = HOLHero::GetHeroBannerLocation(Slot) ; sans banniere plantee, l'origine n'est pas la position du heros (cf. correctif IA ffd0325 "OriginOf", non present dans integ-55). Un joueur qui n'a pas plante de banniere ne peut donc pas lancer la Lumiere sacree. A confirmer en plantant la banniere dans le scenario ou en lisant GetHeroBannerLocation.
- Cercle dore : non juge (6a4b2c8 absent, consigne coordinateur).

## (2) ai_sacred_light : NON JOUE (consigne coordinateur 11:01)
Scenario, hol.AI.HonorSet, hol.AI.LogPowers et ffd0325/6a4b2c8 absents de integ-55 (ils sont sur la branche ability-columns / worktree Glorfindel).

## (3) wall_line_long : FAIL (preflight PASS en 31 s, mais l'effet attendu n'est pas la)
- Le scenario arme la Palissade par `hol.Build.Place Palisade` (interne), PAS par la touche D : geste interne, ne prouve pas l'armement a D au palier 0.
- wall_line_long_during : camera sur le Forum, 5 dalles vertes claires a l'ecran seulement (loin de 16+), l'arriere-plan est le batiment ; la ligne descend vers le sud du point (-17420,-1100) (hors-ecran a gauche/bas), pas visible comme "ligne longue".
- wall_line_long_after : UNE seule fondation (cadre bleu pale, bas gauche), Bois 20200 -> 20195 (un seul mur paye, cout 5) ; pas de ligne de fondations. Le journal montre le glisser (ghosts Palisade valid de Y -1000 a 600) mais aucune pose en ligne visible/payee case par case.
- Defaut : soit le glisser relache ne pose qu'une case, soit la capture est trop tot/camera mal cadree (Zoom 3 ne recule pas assez ; en-tete .txt dit toujours "NON JOUABLE"). A creuser : la ligne de pose apres relache (HOLWallLine*), "wall line (" jamais ecrit dans le journal (waitlog sur cette ligne n'a pas bloque : non confirme).

## Anomalies
- Un job de fond (Start-Job) pour lancer deux preflights en parallele est mort avec l'appel : rien lance en double.
- Un heredoc python3 a bloque un appel Bash 120 s (arrete, aucun fichier ecrit).
- Commits integ-55 : 3e9ebce (faction_click_cast), commit cible LeftClickGround camera, commit hero banner point.
