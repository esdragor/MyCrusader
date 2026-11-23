# DOCS-1123pc (2026-11-23, 17:0x) : reponses des essais wall_line_long et ai_garrison

Docs seulement : aucun code, aucun build, aucun worktree. Debut 17:00, fin 17:06.

## Faits enregistres (mot pour mot, 2026-11-23 17:00, dans la conversation)
- « wall_line_long : BUG : Quand je relache le tracé, ils vont pas automatiquement construire + quand il s'agit de mur en pierre ou en bois, une casese divise en 4 cases plus petite de longeur est largeur et dans le tracé, ce qui est le plus important c'est le debut et la fin, le reste c'est juste un tracé (plus simple pour les ligne ni horizontale ni verticale) »
- « ai_garrison : OK mais je crois que la tour ne tirait qu'une fleche et pas (1 + nombre d'occupants) * fleche »

## Fait
1. GDD.md (aucune modification non commitee avant mon edit : `git diff --stat GDD.md` vide) : D898 (grille fine des murs et trace debut / fin, avec 6 « a preciser » : palissade probable a confirmer, cout, epaisseur et collision, portes, alignement 8 directions, case bloquee / Q-ligne-case-bloquee) et D899 (volee de la tour garnie = (1 + occupants) fleches, « regle confirmee par l'utilisateur », constat de l'utilisateur a verifier, cause non ecrite). « Prochain numero » = D900. Le BUG (Paysans qui ne construisent pas au relache) est dans D898 comme fait, sans numero propre.
2. Ce que le GDD disait deja de la volee : D884 « Le nombre de flèches par occupant (proposé : 1 + 1 par occupant) reste à confirmer à part » (par fleche 5 degats, cadence 1,625 s, portee 3000, la tour tire meme vide) ; D715 (§ 11.2, Levée de la garnison) : « plus de flèches par occupant (départ +50 %, arrondi) » pour forum, centres secondaires et Bastions. La formule n'etait que proposee : la phrase de l'utilisateur la confirme.
3. FortificationsInProgress.md (ligne 269) et EtapeDefini.md (ligne 97, apres la ligne « F10 murs ») : une ligne chacun (les trois points). EtapeDefini.md, section « Fichiers de developpement » : non touchee (pas de nouveau *InProgress.md).
4. Auto_Journey_File.md : 3 lignes `[ ]` en tete du « Socle commun des factions » (lignes 25 a 27) : BUG sans numero, D898 (PLAN d'abord), D899 (test rouge : tour avec 3 occupants, 4 fleches par volee).
5. Retours\Q-murs-grille-fine.md [À RÉPONDRE] : 3 questions (palissade sur la grille fine, cout par sous-case, sous-case bloquee sur le trace), options, consequences en jeu, recommandation A / A / A.
6. Fiches E-ai_garrison.md et E-wall_line_long.md : ligne `Reponse:` mot pour mot, retitrees, archivees par `git mv` dans Docs\Archive\Retours_2026-11-23\.
7. Auto_Journey_Retours.md (en-tete, sections 1, 2, 3), Retours\LISEZ-MOI.md, Retours\ESSAIS-A-JOUER.md recomptes d'apres le listage de 17:04.

## Comptes de Retours\ (listage 17:04, 15 fichiers)
- 5 E (deploy, playtest_hero_army, specialization_slot, talents_heal, tower_wood) + 2 BUG + 6 Q + LISEZ-MOI + ESSAIS-A-JOUER.
- Questions ouvertes : 6 : Q-commanderie-batiment, Q-ligne-case-bloquee, Q-lumiere-sacree-visee, Q-moral-attenuation, Q-murs-grille-fine, Q-sp3-libelles.
- Essais : 0 prets. [A ESSAYER] en titre : 2 (talents_heal LOGIQUE SEULEMENT ; tower_wood VU A L'ECRAN sur la branche zone-effect, pas dans main). specialization_slot VU A L'ECRAN sur integ-25, pas dans main (sa premiere ligne n'est pas un titre [A ESSAYER]). deploy PAS PRET. playtest_hero_army LOGIQUE SEULEMENT.
- main = a984c61 (`git rev-parse --short main` verifie 17:04), local, rien pousse depuis le push de 12:24.

## NON VERIFIE
- Le code : rien lu (la cause du doute sur la volee est l'affaire de VERDICT-1123pa ; la cause du BUG de construction n'est pas cherchee).
- Q-commanderie-batiment, Q-moral-attenuation, Q-sp3-libelles : seul le titre lu, leur contenu non relu ; ils sont comptes ouverts d'apres leur titre [À RÉPONDRE].
- « rien pousse depuis 12:24 » et « origin/main = 0fef853 » repris des textes existants, `git ls-remote` non lance.
- Les 7 branches non integrees de l'en-tete de Retours : liste non recomptee, signalee perimee.
- Que « mur en bois » inclue la palissade : écrit « probable, à confirmer », non tranche.

## Anomalies
- Auto_Journey_File.md, EtapeDefini.md, FortificationsInProgress.md avaient deja des modifications non commitees d'autres sessions : le commit par chemins les embarque (voir `git log -1 --stat`).
- Retours\E-tower_wood.md est modifie par un autre agent (non commite par moi). Retours\LISEZ-MOI.md a ete signale modifie sur disque par une autre session pendant mon edit : relu apres, mon contenu present.
- E-specialization_slot.md : premiere ligne « branche integ-25, pas encore dans main », sans titre `# [...]`.
