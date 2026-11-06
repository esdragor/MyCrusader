# [À ESSAYER] E-audio_compare : ecouter dix sons medieval fantasy par cue (remplace les sons « vaisseau spatial »)

STATUT : fenetre VU A L'ECRAN (panneau du scenario, 3 PNG _start ouverts le 2026-11-06 12:0x) ; SON = LOGIQUE SEULEMENT (lance en -nosound, rien entendu par l'agent : les 240 sons ont ete charges et joues sans erreur, `candidate played` x100 / x70 / x70, `candidate missing` 0). Le choix se fait a ton oreille.
Rejoue le 2026-11-06 12:00 sur le build a jour : worktree HeroesOfLiberty-Arwen, branche heal-sleep = main 2bb0d6b (avance rapide), build.ps1 PASS 11:55, session_gate -Phase Essay sans FAIL (binaire 11:55 >= dernier code 11:24), preflight PASS pour audio_compare (134 s), audio_compare_orders (106 s), audio_compare_alerts (98 s).
Les 145 candidats ne sont pas commités (décision utilisateur) : l'essai se joue depuis ce worktree.
SON: non entendu
GESTE: interne (Entree = vraie touche `waitkey`, mais le son est declenche par la commande console `hol.Audio.Candidate <Cue> <A-J>`, pas par un vrai geste de jeu ; rejouer une variante : console, touche ², `hol.Audio.Candidate Ui.Open C`).

Trois scenarios (tous : preflight PASS le 2026-11-06 11:2x, worktree HeroesOfLiberty-Arwen, SANS build : script seulement, PNG `<nom>_start` ouvert et vu : panneau avec les notes) :
- `audio_compare` : les 10 cues de l'interface (clic, survol, ouvrir, fermer, retour, erreur, confirmer, bascule, tic, defilement), 100 sons ;
- `audio_compare_orders` : 5 ordres + 2 selections, 70 sons ;
- `audio_compare_alerts` : 7 alertes (attaque, batiment fini, entrainement fini, technologie, commerce, volcan, diplomatie), 70 sons.
Sources (CC0, registre `Docs/assets_audio.csv`) : Kenney rpg-audio et impact-sounds (bois, cuir, tissu, livres, portes, metal, cloches), VSCO 2 CE (harpe, pizzicato de violon/violoncelle/contrebasse, glockenspiel, cloches, tambour de guerre, claves, gong). Aucun son du pack interface-sounds ni ui-audio.
Le panneau dit « <Cue> <lettre> (<fichier>) vient de jouer. Entree : <suivante> ». Pas de limite de 3 min : tu t'arretes quand tu veux.

Lancer UNE commande par scenario (avec le son du poste) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 audio_compare -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
(puis `audio_compare_orders`, `audio_compare_alerts` de la meme facon.)
Reponse attendue : par cue, la lettre preferee (ou plusieurs) ; « aucune, dix autres » + ce que tu aimes (bois, cloche, harpe...) ; ou `REGLAGE: ...`.

NON VERIFIE : l'ecoute (volumes relatifs, longueur, tout) ; le son dans la partie reelle (le jeu garde ses sons actuels tant que tu n'as pas choisi, la table n'est pas touchee) ; la commande console tapee a la main. Voir `Q-audio-candidates.md` (les .uasset candidats ne sont pas commites).

Reponse (utilisateur, 2026-11-06 12:1x, texte brut) : audio_compare : UI.Click H, pas de son quand tu hover, UI.Open F, UI.Open A, Ui.Back B, Ui.Error A, Ui.Confirm G, Ui.Toggle pas de son, pas de son pour le tick, pas de son pour le scroll

Reponse (utilisateur, 2026-11-06 12:1x) audio_compare_orders : Select.Unit D, pas de son pour tous les buildings en general (chaque building devrait avoir son propre son), le reste, pas de son

Reponse (utilisateur, 2026-11-06 12:2x) audio_compare_alerts : Alert.UnderAttack A mais faute de mieux, pas de son pour les fin de building, TrainingDone prendre celui de TechDone G (il faut un truc martial), pas de son pour les tech done, Trade A, Alert Volcano A (mais plusieurs fois), Alert.Diplomacy A

STATUT 2026-11-06 12:3x : REPONSE APPLIQUEE (audio_compare, audio_compare_orders, audio_compare_alerts) dans la table du jeu, worktree Arwen heal-sleep, commits 41b0607 et 76a1ab1 ; Q-audio-open = option A ; 12 cues muets ; voir AudioInProgress.md du worktree. SON: non entendu (reste a ecouter en jeu).
