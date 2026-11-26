# [À ESSAYER] E-spec_voie_queue : un siège Templiers clique le Frère infirmier dans la Chapelle : refusé sans voie, mis en file après la voie Foi

STATUT : VU À L'ÉCRAN (3 PNG ouverts et vus, refus et mise en file prouvés au journal)
GESTE: réel (vrai clic hol.Mouse.Click GridCell00 = « Mouse click GridCell00 x1 at 226,477: on SCommonButton » au journal ; la voie Foi est posée par hol.Cheat.Specialization 1 1, interne, seul le clic est réel)
SON: non entendu
branches: spec-voie-1125m

Pré-vol (2026-11-26 14:21, ARWEN-1126m4, C:\Users\baudo\Documents\HeroesOfLiberty-Arwen, main f172741) : `preflight.ps1 spec_voie_queue -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` PASS (56 s, 3 PNG, sanity OK, aucun avertissement). `session_gate.ps1 -Phase Essay -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen -Essay spec_voie_queue` : aucun FAIL (binaire du 13:50 >= dernier code, le worktree contient le tip de spec-voie-1125m).
PNG OUVERTS et vus :
- spec_voie_queue_before : Chapelle (bâtiment bleu au centre) sélectionnée ; grille en bas à droite avec UNE case remplie, A « Frère infirmier » (pas de Moine : il est de l'Aube) ; aucune file de production.
- spec_voie_queue_refused : même vue, infobulle Frère infirmier (100 nourriture, 100 or) ; ressources inchangées (5200 / 5200), aucune barre de file. Journal : `RpcEconomy: queue refused (4, lane not unlocked) for sender slot 0 type InfirmaryBrother`.
- spec_voie_queue_queued : après la voie Foi et le même clic, nourriture et or 5100 (coût payé), barre de production « Frère infirmier » avec « Clic : annuler » au-dessus de la grille. Journal : `Production: slot 0 queued InfirmaryBrother (1 in the queue ...)` puis `InfirmaryBrother of slot 0 done at step 174 (1 born here)`.
Ancien défaut (ARWEN-1125i : case Z vide, Frère infirmier absent de la grille) : plus reproduit ; le scénario clique maintenant la case A.
NON VÉRIFIÉ : son non entendu ; le texte de la note sur le 3e PNG est encore la note 2/3 (la note 3/3 arrive 3 s après la capture) ; la voie Foi est posée par cheat, pas par le bouton de spécialisation ; le Frère infirmier né n'est pas capturé à l'écran (journal seulement).

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle.
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 spec_voie_queue -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
