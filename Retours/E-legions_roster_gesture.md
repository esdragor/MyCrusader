# [A ESSAYER] E-legions_roster_gesture : les noms des Legions (FR, EN, JA) apparaissent quand on clique les batiments et les unites, l'Aube garde ses noms

STATUT : VU À L'ÉCRAN (rejoué par l'intégrateur sur main fusionné : 9 PNG ouverts en 3 planches ; Caserne des Légions : grille Lancier squelette (A) / Garde noire (Z) ; bloc Garde noire et Cavalier funeste avec leurs stats ; Lancier d'Aube rouge nommé Lancier ; Caserne d'Aube sans grille Légions ; EN : Skeleton Spearman, Black Guard, Bone Ram ; JA : 兵舎, スケルトン槍兵, 黒衛兵, 骨の破城槌)
GESTE: interne   (hol.Select.ClickGround / Grid / ClickUnit = commandes hol.*, pas de vraie souris ; l'objet de l'essai est l'AFFICHAGE des noms)
SON: sans objet
branches: aucune

Pre-vol (2026-11-11 13:56, agent Glorfindel-26 sans C++, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, main = merge de af56ff8) : `preflight.ps1 legions_roster_gesture -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel` PASS (43 s, 9 PNG ; pan_exposure_test.ps1 PASS avant). WARN : pas de ligne Sanity.
PNG OUVERTS et vus :
- legions_gesture_barracks : Caserne bleue selectionnee, bloc du bas "Caserne / Vous / 1500", grille A "Lancier squelette", Z "Garde noire"
- legions_gesture_blackguard : bloc "Garde noire" (PV 155, attaque 12, armures 4/4/2, vitesse 7 m/s), pop 5/70
- legions_gesture_rider : bloc "Cavalier funeste" (PV 120, attaque 9, vitesse 8,8 m/s), unite grise selectionnee
- legions_gesture_aube_unit : unite rouge (IA Normal 2) "Lancier" PV 80 : nom d'Aube, pas Lancier squelette
- legions_gesture_aube_barracks : Caserne rouge "Caserne / IA Normal 2" sans aucune grille
- legions_gesture_en_barracks : "Barracks", grille "Skeleton Spearman" (A) et "Black Guard" (Z)
- legions_gesture_en_ram : "Bone Ram" 400/400, vitesse 2,2 m/s
- legions_gesture_ja_barracks : barre et bloc en japonais, grille "スケルトン槍兵" (A) et "黒衛兵" (Z)
- legions_gesture_ja_ram : bloc "骨の破城槌" 400/400
NON VERIFIE : un vrai clic de souris et le son (gestes internes, -RenderOffScreen) ; la grille de la Caserne rouge est absente parce qu'elle est ennemie (le point prouve donc surtout le nom, pas l'absence de noms Legions dans la grille) ; en EN le bloc du Belier garde "Attaque batiments 60" en francais (non juge : hors de l'essai, a signaler) ; la traduction japonaise reste une proposition a relire.

Lancer UNE commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 legions_roster_gesture -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir  (rejoue sur le build de main 521adcb fusionne, worktree Faramir ; sans -Hidden = fenetre, reservee a l'utilisateur ; un agent ajoute -Hidden)
Duree : environ 1 min. Reponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

## Integrateur Faramir 2026-11-11 15:09-15:16, main 521adcb, binaire 15:03 (session_gate -Phase Essay PASS)
Défaut constaté : la 3e case de la grille de la Caserne affiche « Damned Warrior » (E) en anglais dans l'interface FR et JA (non traduit) ; le bandeau du scénario reste en français en EN et JA. session_gate -Phase Essay dit « aucune branche citée » alors que la ligne est « branches: aucune » (gabarit : aucune est permis) : le contrôle du gate est trop strict. Durée : environ 1 min de jeu.
