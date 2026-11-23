# DOCS-1123pb (2026-11-23, 16:0x) : reponses des essais faction_click_cast et ai_sacred_light

Docs seulement : aucun code, aucun build, aucun worktree.

## Faits enregistres (mot pour mot, 2026-11-23 15:59, dans la conversation)
- « faction_click_cast : REGLAGE: actuellement, ca se met auto au milieu de l'armée, je devrai pouvoir choisir avec la position de ma souris »
- « ai_sacred_light : OK »

## Fait
1. GDD.md : D897 (suite de D896), « Prochain numéro » = D898. Regle : la zone de Lumiere sacree du joueur se pose au point vise par la souris. A preciser, SANS trancher : portee max du point vise (1200 cm de D890 comptes depuis le heros, la banniere ou le centre de l'armee), sort de la banniere comme origine pour le joueur (D891 reste pour l'IA), hors portee (refus ou zone ramenee). Contexte cite : CastOriginOf (Script/Factions/HOLFactionPowerServer.as), le clic ne sert qu'a valider. GDD.md n'avait aucune modification non commitee avant mon edit.
2. AubeInProgress.md (ligne 120, section A6, edit_line -Append) et EtapeDefini.md (ligne 15) : une ligne chacun.
3. Auto_Journey_File.md : ligne `[ ]` ajoutee en tete du « Socle commun des factions » (ligne 25, juste sous le titre), texte demande, D897.
4. Retours\Q-lumiere-sacree-visee.md : [À RÉPONDRE], options A refus (defaut code, recommande) / B point le plus proche / C le heros avance.
5. Fiches archivees par `git mv` dans Docs\Archive\Retours_2026-11-23\ : E-ai_sacred_light.md (Reponse: OK, retitre [RÉPONDU : OK]) et E-faction_click_cast.md (Reponse: REGLAGE ... D897, retitre [RÉPONDU : REGLAGE, à rejouer après correction]).
6. Auto_Journey_Retours.md (en-tete, sections 1, 2, 3), Retours\LISEZ-MOI.md, Retours\ESSAIS-A-JOUER.md recomptes d'apres le listage de Retours\ (voir ci-dessous).

## Comptes de Retours\ (listage 16:03 puis fin de seance)
- 12 fichiers : 6 E (ai_garrison, deploy, playtest_hero_army, talents_heal, tower_wood, wall_line_long) + 2 BUG + 2 Q + LISEZ-MOI + ESSAIS-A-JOUER.
- Questions ouvertes : 2 (Q-ligne-case-bloquee, Q-lumiere-sacree-visee). Aucune autre Q-*.md vue au listage.
- Essais : 0 prets. [A ESSAYER] en titre : 4. VU A L'ECRAN, entres dans main a 16:01 (c3578b1), a rejouer sur main : ai_garrison, wall_line_long. LOGIQUE SEULEMENT : talents_heal, tower_wood (+ deploy, playtest_hero_army hors titre [A ESSAYER]).
- main = c3578b1 (16:01, local, rien pousse depuis le push de 12:24).

## NON VERIFIE
- « main = c3578b1 » et « entres dans main a 16:01 » repris de la consigne ; `git rev-parse main` = c3578b1 verifie, le contenu (ai_garrison, wall_line_long) non verifie.
- Les 7 branches non integrees de l'en-tete de Retours : liste NON recomptee, signalee perimee.
- Le « defaut code : refus avec message » de la question est celui de la consigne ; non verifie dans le code (aucun code lu).

## Anomalies
- AubeInProgress.md, Auto_Journey_File.md, EtapeDefini.md avaient deja des modifications non commitees d'autres sessions : le commit par chemins les embarque (voir le commit).
- Commit 19182fc (sur main, local, rien pousse) : 10 fichiers ; le commit embarque aussi les lignes d'autres sessions deja presentes dans AubeInProgress.md (2 lignes), Auto_Journey_File.md (1 ligne modifiee + 1 ajoutee) et EtapeDefini.md (1 ligne).
- Apres mon listage de 16:03, Retours\Q-commanderie-batiment.md (non suivie, d'un autre agent) est apparue a 16:05 : NON comptee dans mes en-tetes (2 questions ouvertes comptees : en realite 3 des qu'elle est titree [A REPONDRE]) ; a recompter par la session principale ou l'agent qui l'a creee.
