# [À RÉPONDRE] Gate Close (L19-1) : comment ne plus compter les commits des agents DIAG comme du code de la session principale

Contexte : le gate Close compte les commits de code de la session principale (règle L19-1, seuil 3) ; il en a compté « 25 fois » qui venaient d'agents de diagnostic sur `integ-*` (L22-6, faux positif).
Rien dans git ne distingue un commit d'agent d'un commit de la session principale : même auteur partout (Baudouin L), même format de message.

Faits (SAURON-1122b, `Docs\Rapports\SAURON-1122b.md`) :
- Ancienne règle : exempter les branches `integ-*` encore extraites dans un worktree `HeroesOfLiberty-*` autre qu'Arwen. Dès qu'un worktree d'agent change de branche, ses `integ-N` redeviennent « session principale » (c'est le faux positif).
- Le journal HEAD de chaque worktree (`<common>/worktrees/<nom>/logs/HEAD`) dit où un commit a été fait ; c'est la seule trace disponible sans changer les commits.

Question : quelle règle le gate doit-il appliquer pour exempter les commits d'agents ?
- A. Correctif prêt sur `integ-48` (rouge 756cf78, vert fc6230a, branche prête, non intégrée) : le gate exempte tout commit fait dans un worktree déclaré au tableau du jour (`## Agents du <date>`), d'après le journal HEAD ; Arwen jamais exemptée ; seuil inchangé. Effet : plus de faux positif. MAIS un commit de code que la session principale ferait elle-même dans ce worktree ne serait plus vu : le contrôle L19-1 s'affaiblit. Résiduel : un worktree d'agent supprimé (`git worktree remove`) perd son journal, ses commits redeviennent comptés.
- B. Marque explicite : chaque agent ajoute une ligne « Agent: <nom> » (trailer) à ses messages de commit ; le gate n'exempte que les commits portant cette ligne. Effet : plus strict (un commit de la session principale n'est jamais exempté, même dans un worktree d'agent), indépendant des worktrees. Mais cela repose sur la discipline de l'énoncé (un agent qui oublie le trailer est compté) et il faut votre accord : c'est une règle de format des commits (aujourd'hui anglais, sans Co-Authored-By). Le correctif A d'integ-48 ne servirait pas tel quel : le gate est à réécrire (petit lot d'outil PowerShell).
- C. Rien : garder le faux positif ; une exception écrite au Jour à chaque clôture. Effet : aucun code, mais chaque Close affiche un FAIL L22-6 à expliquer à la main (c'est la ligne L22-6 actuelle « en attendant »).
- Autre : (à préciser).

Recommandation : B (le seul qui garde le contrôle L19-1 entier tout en supprimant le faux positif ; A est plus simple mais cache précisément ce que L19-1 veut voir).

Valeur par défaut : C (rien d'intégré ; `integ-48` (756cf78, fc6230a) reste de côté, non fusionné ; exception écrite au Jour à chaque clôture).

Reponse: B, comme recommandé (2026-11-22 ~22:05, dans la conversation ; file, ligne [ ] OUTIL L22-6)
