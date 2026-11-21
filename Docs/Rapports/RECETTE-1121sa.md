# RECETTE-1121sa (recette independante, main 9e7a324, worktree Sauron)

APPELS_DECLARES: ~14 (arret a 25 vus ; seuil juge sur tool_uses de la session principale)
HEURE_DEBUT/HEURE_FIN: 19:36 / 19:39 (Get-Date PowerShell)
PREMIER_BUILD: sans C++   BUILD: aucun (binaire deja a jour)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (recette, aucun code ecrit)
COMMITS: 0, merge_main_fait: non (sans objet)
TESTS: preflight chat_input PASS (31 s), preflight order_lines_blocks PASS (21 s) ; TESTS_TAUTOLOGIQUES: 0 (aucun test ecrit)

ESSAIS:
- chat_input : PASS, VU A L'ECRAN sur main 9e7a324. 4 PNG ouverts (Saved_Tests\Screenshots\WindowsEditor\HOL\chat_input_*.png) :
  - typing : champ ouvert en bas a gauche, marque « (Alliés) : » + boite vide avec curseur, Diplomatie ouvert avec la ligne du joueur 2.
  - typed : « salut les allies » DANS la boite.
  - shown : champ ferme, fil en haut a droite « Vous : (Alliés) salut les allies » (une seule ligne).
  - target : apres clic ligne joueur 2 (ligne encadree), Entree, Tab Tab : champ « Joueur 2 : » + boite vide ; fil « (Alliés) salut les allies » et « (Privé) salut ». Tab passe bien sur le prive.
  WARN preflight : `frames 90` ligne 14 du scenario (preexistant).
  Question « Joueur 2 : » vs « (Privé) » : le jeu est conforme au design, le .look est faux. D833 (GDD.md:3624, alerte « Population pleine » 15 s) et D871 (Mangonneau, montage) ne concernent PAS le chat. La decision applicable est D807 (GDD.md:3572, Q-chat-ui) : « (Privé) » n'y est fixe que pour la LIGNE RECUE (« Bob : (Privé) texte ») ; la marque du champ pour le prive n'y est pas fixee, et le code `ChannelMark` prend le nom du joueur choisi (ChatInProgress.md:37 et :56 « Privé = nom du joueur, design »). Le texte de `Tools\Scenarios\chat_input.look` (chat_input_target : « marque (Prive) et le NOM ») est ambigu, a reformuler ; pas un bug du jeu. Reste a faire trancher par l'utilisateur seulement s'il veut aussi « (Privé) » dans le champ.
  Commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 chat_input -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
- order_lines_blocks : PASS, VU A L'ECRAN sur main 9e7a324. PNG order_lines_blocks_pendant ouvert : 12 unites (Tous 12 = Homme d'arme 6 + Lancier 6) en deux blocs separes gauche / droite, DEUX lignes jaunes, chacune partant de son bloc vers le point d'ordre commun ; pas de ligne unique depuis le milieu. WARN preflight : pas de `HUD overlap:` ni `Sanity:` (preexistants, scenario sans hol.HUD.Report / hol.Test.Sanity). GESTE reste interne (hol.Select.OrderGround) : un vrai clic droit non rejoue.
  Commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 order_lines_blocks -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron

Fiches mises a jour : Retours\E-chat_input.md (ligne `recette:` l.7, commande l.21), Retours\E-order_lines_blocks.md (ligne `recette:` l.7, commande l.15), commandes avec -Root Sauron. Rien ajoute a Auto_Journey_Retours.md.

QUESTIONS: aucune
FAIT / RESTE: 2 essais rejoues et verifies sur main ; reste : corriger la phrase du chat_input.look (« (Prive) » vs nom), vrai clic droit / Tab physique non rejoues, tracé Maj + clic droit a deux blocs non couvert.
NON VERIFIE: Tab physique (relais Slate synthetique hol.UI.Key) ; vrai clic droit (ordre interne) ; file Maj+clic droit a deux blocs ; marche ligne par ligne (une seule image) ; son sans objet.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: non relance (docs seuls modifies : Retours\E-chat_input.md, Retours\E-order_lines_blocks.md, ce rapport) ; code inchange : oui
