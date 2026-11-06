# [À ESSAYER] E-audio_compare : ecouter dix sons medieval fantasy par cue (remplace les sons « vaisseau spatial »)

STATUT : fenetre VU A L'ECRAN (panneau du scenario) ; SON = LOGIQUE SEULEMENT (lance en -nosound, rien entendu par l'agent : les 240 sons ont ete charges et joues sans erreur, `candidate played` x100 / x70 / x70, `candidate missing` 0). Le choix se fait a ton oreille.
GESTE : touche Entree (vraie touche, `waitkey`) ; le son est joue par la commande console `hol.Audio.Candidate <Cue> <A-J>` (rejouer une variante : console, touche ², `hol.Audio.Candidate Ui.Open C`).

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
