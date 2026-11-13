# [NE PAS JOUER] E-disconnect_1v1 : Deconnexion en 1 contre 1 : Bob (seul adversaire) part, la fenetre finit, Bob est vaincu et Alice gagne

STATUT : NE PAS JOUER (LOGIQUE SEULEMENT) ; cause : lance sans -PlayerName chez l'utilisateur (spectateur) et binaire de la copie jouee perime (diagnostic 3b94faf) ; redonner quand la copie choisie est recompilee et que preflight.ps1 disconnect_1v1 -Root <cette copie> -PlayerName Alice complet PASS (pre-vol du 2026-11-09 ci-dessous : historique, plus valable)
GESTE: interne      (commandes hol.Net.FakeAbsent End 1 : le depart reseau est simule sans second client ; Entree = temps suivant)
SON: sans objet
branches: net-playtest

Pre-vol (2026-11-09 16:3x, agent Glorfindel-22 sans C++, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, main e9a349a) : preflight.ps1 disconnect_1v1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -PlayerName Alice PASS (35 s, 2 PNG). WARN : pas de ligne HUD overlap ni Sanity.
PNG OUVERTS et vus :
- dc1v1_away : base de Bob (forum, tours, baies, arbres), pastille "Connexion perdue" a cote de Bob (2 vies) dans la liste des joueurs, bandeau "Bob : connexion perdue - 1:00" sous le panneau du scenario
- dc1v1_end : ecran "Victoire" (Continuer / Quitter vers le menu / Statistiques), Bob "Vaincu" dans la liste des joueurs, ligne de journal "Bob a quitte la partie"
NON VERIFIE : le depart reel d'un second jeu (lobby_match -Reconnect/-Team) ; la liste de Bob montre encore "2 vies" apres "Vaincu" (non juge : c'est un affichage de la defaite comptee, pas un point de l'essai) ; le texte "en preparation" du panneau du scenario reste affiche pendant tout le scenario (le panneau des notes recouvre le bandeau si on ajoute des notes en cours de route : les notes sont donc a la fin) ; le scenario porte un waitkey (Entree) voulu : deux temps.

Lancer UNE commande (-PlayerName Alice obligatoire) : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\play.ps1 disconnect_1v1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -PlayerName Alice
Duree : environ 1 min. Reponse attendue : OK, BUG: ... ou REGLAGE: ...

A regarder : temps 1 = Bob "Connexion perdue" avec le bandeau 1:00 ; Entree ; temps 2 = Bob Vaincu, "Bob a quitte la partie", ecran Victoire d'Alice.

Reponse: le scenario n'a jamais demarre (erreur etape 5 « jamais venu »), 2026-11-11 soir

## STATUT 2026-11-12 (FARAMIR-1112a) : LOGIQUE SEULEMENT, NE PAS JOUER
Cause (FARAMIR-1112a, 2026-11-12, journal de l'utilisateur HeroesOfLiberty-Haldir\Saved\Logs\HeroesOfLiberty.log 18:19) : lance SANS -PlayerName (ligne de commande du journal), donc Â« 257 joined as a spectator (name '') Â», etape 5 Â« waitlog Camera above the start of slot Â» jamais venue. Correctif (branche fog, 42e5e41) : play.ps1 et check.ps1 prennent seuls le premier siege Humain nomme du .json (Alice) ; preflight FAIL si play.ps1 ne nomme pas le joueur. binaire de main (copie principale, dll 2026-11-11 11:06) PERIME : 9 commits C++ depuis (rage L4, Monter/Demonter, camps) ; au lancement Angelscript: Error (Namespace 'HOLAIRageOrder' doesn't exist, UHOLCampSubsystem::GetNumCamps) et le jeu reste a l'image 0 (rejoue par FARAMIR-1112a, play.ps1 -Hidden depuis C:\Users\baudo). Arwen (dll 11-11 13:43) et Faramir (dll 11-11 16:02) sont aussi plus vieux que leur dernier commit C++ : recompiler la copie choisie avant de donner la commande.
Commande a redonner seulement apres : fog fusionnee dans main, copie choisie recompilee, puis preflight.ps1 <nom> -Root <cette copie> PASS (le preflight verifie maintenant le binaire, le joueur nomme et les fichiers non commites).

## ETAT 2026-11-13 (nettoyage Retours) : toujours NE PAS JOUER
Fait depuis : fog fusionnee dans main (b689e5a via maps, 4851635) et correctif du joueur nomme (42e5e41) dans main. Branche net-playtest toujours PAS dans main. Reste : recompiler la copie choisie, puis preflight.ps1 disconnect_1v1 -Root <cette copie> -PlayerName Alice PASS. NON VERIFIE : etat des binaires ce jour.
