# CLEAN-C-1125a : nettoyage 0a, bloc Retours\ (2026-11-25 apres-midi, seance autonome)

HEURE_DEBUT: 15:06  APPELS: 6
ARCHIVE PAR CET AGENT : rien. DOCS-1125mg avait deja archive 3 fiches a 14:31 (a814047 5 Q repondues D915-D919 ; 2059989 ; fd8b120) dans Docs\Archive\Retours_2026-11-25\ (E-faction_aim_free, E-sacred_light_heal, E-wall_line_skip, E-monk_seal, E-tower_wood, E-morale_look, 5 Q-*). Les 15 fichiers restants (10 E, 3 BUG, 2 index) : aucune reponse de l'utilisateur reportee (verifie : Auto_Journey_Retours.md, AIInProgress.md, EtapeDefini.md, GDD.md).
MIS A JOUR : Retours\LISEZ-MOI.md (section ETAT, liste des archives jusqu'a Retours_2026-11-25) et Retours\ESSAIS-A-JOUER.md (reecrit : les index citaient encore E-monk_seal, E-tower_wood, E-sacred_light_heal, E-faction_aim_free, E-wall_line_skip et les 3 Q, tous archives ; E-wall_fine_ghost, absente des index, ajoutee ; questions sans reponse = 0).
DOUTES (fiches laissees) :
- BUG-ai_peasant-spin : correctif 17e7722 (test rouge 8abd654) code mais NON COMPILE (FARAMIR-1125mf) ; a archiver apres build + rejeu ai_tower.
- BUG-melee-spin-sanity, BUG-playtest_hero_army-spin, E-playtest_hero_army : D641 (le jeu garde le defaut), ouverts, a fusionner ou clore par un agent qui rejoue Sanity.
- E-deploy, E-wall_fine_block, E-talents_heal : logique seulement / pas pret, attendent un correctif, pas l'utilisateur.
- E-duel_challenge_accept, E-wall_fine_ghost : recette FAIL, a rejouer avant de donner.
NON VERIFIE : l'etat de main apres integ-82 (ne pas relu les branches).