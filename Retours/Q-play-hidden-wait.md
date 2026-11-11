# [À RÉPONDRE] play.ps1 -Hidden : attendre la fin du jeu, ou rendre la main tout de suite ?

Contexte : `play.ps1 -Hidden` laissait 1 à 2 éditeurs vivants (L11-6). Aucun scénario ne se termine seul (pas de `quit`), donc pour « fermer ses éditeurs à la fin » il faut une fin : j'ai codé -Hidden = attendre au plus `-TimeoutSec` (300 s), puis fermer l'éditeur et tous ses fils ; `-NoWait` rend l'ancien comportement (rend la main, l'éditeur reste, `stray_processes -Stop` à la fin). Branche d751-root, commit e93b21e.

Options
- A (codé) : -Hidden bloque jusqu'à 300 s puis ferme ; un agent qui veut lire des journaux pendant le jeu lance play.ps1 en `run_in_background` ou ajoute `-NoWait`.
- B : -Hidden rend la main tout de suite (comme avant) mais lance un petit guetteur détaché qui ferme l'éditeur après `-TimeoutSec` (pas d'attente, mais un processus de plus : contraire à la règle 20 « aucun détaché »).
- C : -Hidden bloque jusqu'à une fin signalée par le scénario (nouveau verbe `quit` dans HOLTestSequence, du C++, donc hors de ce chantier) ; la fermeture à 300 s reste le filet.
- Autre : ...

Conséquences visibles : A change l'appel des agents qui enchaînaient `play.ps1 -Hidden` puis lecture des captures (ils attendent maintenant jusqu'à 300 s ou ajoutent -NoWait). B garde le flux mais crée un détaché. C est la plus propre mais demande du C++.

Recommandation : A maintenant, C plus tard (un `quit` de scénario ramènerait l'attente à la durée réelle du scénario).

Valeur par défaut codée : A. Une ligne à changer : `[int] $TimeoutSec = 300` dans `Tools\Scenarios\play.ps1` (ou passer `-NoWait`).
