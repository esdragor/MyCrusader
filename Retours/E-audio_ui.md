# [À ESSAYER] E-audio_ui : sons de l'interface (ouvrir / confirmer / fermer l'arbre de talents), A2b

STATUT : fenêtre VU À L'ÉCRAN ; SON = LOGIQUE SEULEMENT (lancé avec -nosound, rien entendu par l'agent). GESTE : clics par `hol.HUD.Click HeroTalentButton / HeroTalentCell_2_1 / TalentWindowClose` (appel interne, pas un vrai clic souris) ; `hol.Hero.SetXP 0 600` donne 2 talents.

Pré-vol (2026-11-05 21:17, agent d'intégration n°2, worktree HeroesOfLiberty-Elrond = main eb7b5fe, SANS build) : `preflight.ps1 audio_ui -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond` PASS (check 18 s, 1,6 s d'attente, 0 waitkey, audit Sanity OK). WARN : pas de `hol.HUD.Report` au moment de la capture.
Journal (`Saved_Tests\Logs\scenario_audio_ui.log`) : `cue played Ui.Open file 1 asset /Game/Audio/Kenney/interface-sounds/SW_open_002 volume 70`, `cue played Ui.Confirm file 0 ... SW_confirmation_001 volume 80`, `cue played Ui.Close file 2 ... SW_close_003 volume 70` : les trois cues sont écrits, dans l'ordre.

PNG OUVERT et vu : audio_ui_window : la fenêtre « Arbre de talents » ouverte (Talents à dépenser : 2, bouton Fermer, colonnes Gardien / Capitaine / Champion, rangées 2 à 9, rangées 7-9 grisées, bouton « Choix conseillé »), niveau 3, « Palier 1 atteint ». Rien ne cache la fenêtre (le panneau du scénario est replié en haut).

NON VÉRIFIÉ : les sons eux-mêmes (jamais entendus : volume, choix des fichiers Kenney) ; un vrai clic ou la touche T ; que `Ui.Confirm` soit joué aussi quand le talent est refusé/déjà pris ; le son à la fermeture par Échap.

Lancer UNE commande (sans -nosound, avec le son du poste) : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 audio_ui -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 10 s. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...` (dire quels sons garder / changer).

Reponse: REGLAGE : changer TOUS les sons de l'interface : (utilisateur, ecoute reelle, 2026-11-05 22:4x) ils font « console sci-fi de vaisseau spatial », pas du tout une interface de jeu medieval fantasy. Les trois cues entendues : Ui.Open SW_open_002, Ui.Confirm SW_confirmation_001, Ui.Close SW_close_003 (pack Kenney interface-sounds). Regle 6 de Qualite des essais : balayage, l'utilisateur choisit a l'oreille (methode des dix echantillons).
