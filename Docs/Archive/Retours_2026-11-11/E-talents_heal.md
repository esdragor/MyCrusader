# [À ESSAYER] E-talents_heal : prends « Mains du guérisseur » (T puis la case Capitaine rangée 3) et regarde le trait de soin du Moine passer du vert « + » au doré « ++ »

STATUT : VU À L'ÉCRAN (rejoué par l'intégrateur sur main fusionné : trait de soin vert « + » sans talent, doré « ++ » avec le talent, case teintée et « Talent pris : Mains du guérisseur » sans « pas encore actif », les 3 PNG ouverts plein format)
GESTE: interne   (hol.HUD.Click TalentKey puis HeroTalentCell_3_1 = commandes hol.*, pas la souris ni le clavier ; rétrogradé de « réel » par l'intégrateur ; l'essai ne juge que le visuel du trait, le geste T puis clic à la main reste à faire par l'utilisateur)
SON: non entendu   (lancé avec -nosound ; le son du talent pris n'a pas été écouté)
branches: galadriel-tab

Règle (L7-6) : un essai de GESTE ou de SON reste LOGIQUE SEULEMENT tant que la ligne dit `interne` ou `non entendu`. Ici l'essai porte sur le trait de soin (visuel), pas sur le son.

Pré-vol (2026-11-11 13:55, agent Galadriel-heal, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel, main 61cc74a) : `preflight.ps1 talents_heal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel` PASS (37 s de lancement, 13,5 s de jeu, 3 PNG ; deux WARN : pas de `hol.HUD.Report` ni de `hol.Test.Sanity` dans ce scénario).
PNG OUVERTS et vus (dossier C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Saved_Tests\Screenshots\WindowsEditor\HOL\) :
- talents_heal_base.png : un Moine au milieu d'Hommes d'armes blessés, trait de soin VERT et « + » vert vers un Homme d'armes ; bloc du héros « Talents à dépenser : 1 », une icône « B » (Boucliers levés seul).
- talents_heal_cell.png : la fenêtre « Arbre de talents » : cases Capitaine rangées 2 et 3 teintées (Boucliers levés, Mains du guérisseur) ; bloc du héros : « Talent pris : Mains du guérisseur — Moines dans l'aura : soins +20 % » (plus de « pas encore actif »), icônes « B M ». Le « ++ » doré se voit déjà derrière la fenêtre.
- talents_heal_boosted.png : mêmes blessés (re-blessés à 50 %), trait de soin DORÉ (255, 215, 60) avec « ++ » doré, visible et lisible (plus petit que le « + » vert au zoom 10, mais net).
Le héros est blessé lui aussi (PV 261/522) : `hol.Cheat.Wound` blesse tout sauf le Roi ; sans effet sur le trait.
NON VÉRIFIÉ : le facteur 1,2 en jeu (couvert par le test `HeroTalent.HealersHands`, pas mesuré dans le scénario) ; le cas bannière ; un client en réseau (il voit le trait vert normal : option A de `Q-talents-heal-visual.md`, à trancher) ; le son ; le geste au clavier/souris du système. Le trait se voit : design du trait (couleur, taille du « ++ ») NON TRANCHÉ, option A codée par défaut.

Lancer UNE commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 talents_heal -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir  (rejoue sur le build de main 521adcb fusionne, worktree Faramir ; sans -Hidden = fenetre, reservee a l'utilisateur ; un agent ajoute -Hidden)
(sans `-Hidden`, `play.ps1` ouvre une fenêtre : réservé à l'utilisateur ; l'utilisateur la lance donc sans `-Hidden`.)
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...` (et la réponse à `Retours\Q-talents-heal-visual.md`).

## Integrateur Faramir 2026-11-11 15:09-15:16, main 521adcb, binaire 15:03 (session_gate -Phase Essay PASS)
PNG vus : talents_heal_base (trait vert + du Moine vers un Homme d'armes, bloc héros « Talents à dépenser : 1 »), talents_heal_cell (fenêtre Arbre de talents, 2 cases Capitaine teintées, bloc « Talent pris : Mains du guérisseur — Moines dans l'aura : soins +20 % »), talents_heal_boosted (trait doré avec « ++ » doré, petit mais net). Défaut constaté : dans talents_heal_cell le « ++ » doré est dessiné PAR-DESSUS la fenêtre Arbre de talents (marque du monde au-dessus de l'interface) ; le bandeau « Scénario talents_heal en préparation… » reste affiché sur les 3 PNG. Durée : environ 1 min.

## Faramir 2026-11-11 15:3x (branche fog, sans C++)
talents_heal_cell : le « ++ » dore (marque du monde du soin) est dessine PAR-DESSUS la fenetre Arbre de talents : probleme de couche de dessin, note dans Auto_Journey_File.md sans trancher (la marque doit-elle passer sous les fenetres du HUD ?). Le bandeau « en preparation... » sur les 3 PNG = ligne (C++) de Auto_Journey_File.md (l'etat ne passe a « pret » qu'a la fin de la sequence). Scenario non modifie.
