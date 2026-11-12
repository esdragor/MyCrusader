# [A ESSAYER] E-talents_icons_compare : les icones des talents (Placeholders.txt : Icones des talents (arbre du heros) [?]) ; dis aime / pas aime par lettre

STATUT : VU À L'ÉCRAN (rejoué par l'intégrateur sur main fusionné : 2 PNG ouverts, A : fenêtre Arbre de talents avec 3 cases teintées au nom écrit sans image (Boucliers levés, Mains du guérisseur, Pas de l'Aube) ; B : bloc du héros fenêtre fermée avec les pastilles-lettres B M P ; aucun goût jugé)
GESTE: interne      (hol.HUD.Click des cases ; la touche Entree est le seul geste de l'utilisateur, passee en auto par preflight)
SON: non entendu
branches: visual

Options : A la fenetre Arbre de talents avec trois talents pris ; B la fenetre fermee, le bloc du heros avec les pastilles B M P.
VU (agent) : A (talents_icons_A_window) : trois cases teintees dorees (Boucliers leves, Mains du guerisseur, Pas de l'Aube), noms en texte, aucune image de talent ; rangees 7 a 9 grisees. B (talents_icons_B_letters) : sous la liste des talents pris, trois lettres B, M, P en jaune (pas de pastille visible en capture), a cote des trois petites images des capacites du heros (au-dessus de la ligne dorée). Les lettres sont donc le seul visuel d'un talent pris.
NON VERIFIE : l'infobulle au survol (texte) n'est pas dans les PNG ; la couleur par famille n'est pas distinguable, les trois talents etant de la meme famille (Capitaine).
Pre-vol : preflight talents_icons_compare PASS (2026-11-11, agent Sauron-R3, HUD overlap OK, sanity OK).
Lancer UNE commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 talents_icons_compare -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir  (rejoue sur le build de main 521adcb fusionne, worktree Faramir ; sans -Hidden = fenetre, reservee a l'utilisateur ; un agent ajoute -Hidden)
Reponse attendue : A et B `aime` / `pas aime` ou `REGLAGE: ...`. Le choix de la ligne [?] est le tien, l'agent n'y touche pas.

## Integrateur Faramir 2026-11-11 15:09-15:16, main 521adcb, binaire 15:03 (session_gate -Phase Essay PASS)
Défaut constaté : sur talents_icons_B_letters le bandeau affiche encore « Option A (1/2) : la fenêtre Arbre de talents... » alors que la fenêtre est fermée (texte en retard d'une option). Durée : 2 options, environ 1 min.

## Correctif du bandeau Faramir 2026-11-11 15:3x (branche fog, sans C++)
Cause : la commande `note` ajoute son texte au bandeau a l'instant ou elle est lue, et le scenario la plaçait APRES `capture` : le PNG montrait le texte de l'option d'avant. Corrige a la source : note avant la 1re capture de l'option. preflight PASS ; PNG ouverts a plein format : talents_icons_A_window (bandeau = texte de A, fenetre ouverte, 2 cases teintees dont Mains du guerisseur et Pas de l'Aube) et talents_icons_B_letters (bandeau = texte de B, fenetre fermee, pastilles B M P visibles dans le bloc du heros). Reste : le bandeau empile A puis B (la commande n'ajoute que : ligne C++ dans Auto_Journey_File.md) et dit « en preparation... ». Statut : VU A L'ECRAN pour le texte du bandeau.

## Faramir-5 2026-11-11 16:2x (branche fog tip 3fd83e2 : `notereset` + etat « pret » des la 1re waitkey ; scenario converti)
preflight talents_icons_compare PASS (aucun WARN ; le FAIL « no note line » d'avant = regex perimee de preflight.ps1, corrigee). PNG ouverts a plein format : talents_icons_A_window (bandeau = « Option A (1/2) » SEUL, etat « en preparation... » car capture avant la 1re waitkey ; fenetre Arbre de talents ouverte, 3 cases teintees Boucliers leves / Mains du guerisseur / Pas de l'Aube, la fenetre n'est pas couverte par le bandeau) et talents_icons_B_letters (bandeau = « Option B (2/2) » SEUL, etat « pret » ; fenetre fermee, pastilles B M P nettes dans le bloc du heros). Le bandeau n'empile plus A puis B. Statut : VU A L'ECRAN (bandeau juste, points nets). Aucun gout juge.
