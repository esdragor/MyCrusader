# CELEBORN-1121a (worktree HeroesOfLiberty-Celeborn, branche duel-rewards) : NON COMPILE

Tache : D866, sous-etape 3 du plan du moteur de moral (PLAN-MORAL-1120) : fragment de moral par unite + empreinte.
Impact joueur : aucun a l'ecran (le fragment n'est pas encore pose sur les unites ni lu par le combat ; sous-etapes 4 et 5).

APPELS_DECLARES: 13 (arret a 25 vus). Premier Edit a mon 9e appel (le 8e a servi a lire l'en-tete de la regle pour ne pas ecrire a l'aveugle : ecart a la consigne « avant le 8e »).
HEURE_DEBUT/HEURE_FIN: 10:50:38 / 10:52:51 (Get-Date)
PREMIER_BUILD: sans build (agent, pas de build demande)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (C++ ecrit seulement, pas de build dans l'agent)
TEST_ROUGE: 9792b9f (test C++ HeroesOfLiberty.Morale.FragmentPosesRefreshesExpiresRemoves + stub FingerprintKey = 0) ; rouge NON VU a l'execution (pas de build) ; assertit l'effet : oui (Total(Taken) apres pose / refraichissement / expiration / RemoveMalus lus par la vraie regle, et cle d'empreinte 0 pour une unite vide)
COMMITS: 2 (9792b9f test + stub, 61ee604 correctif), dernier a 10:52 (seuil 30 min), merge_main_fait: non (worktree deja a main bce2d69 au depart ; DLL non rebuildee)
TESTS: aucun lance. TESTS_TAUTOLOGIQUES: 0 (les cles et totaux sont calcules par le code teste, comparaisons entre etats, pas de constante recopiee)
ESSAIS: aucun (regle pure, rien a l'ecran)
QUESTIONS: aucune (Q-moral-valeurs deja repondue : defauts partout)
FAIT :
 - Simulation\HOLSimulationFragments.h : FHOLMoraleFragment { HOLMorale::FHOLMoraleState State; } (+ include HOLMoraleRules.h), avant FHOLCombatFragment.
 - Simulation\HOLMoraleRules.h/.cpp : HOLMorale::IsEmpty (Num == 0 && ImmunityMs == 0) ; HOLMorale::FingerprintKey (0 si vide, sinon FNV-1a sur Num, ImmunityMs et, par emplacement, Id / Dealt / Taken / Rate / DurationMs / bAura ; jamais 0 si non vide).
 - Simulation\HOLSimulationSubsystem.cpp : dans HashUnits, la cle est melee a Parts->Combat seulement si non nulle (patron RageMs, ancien hash inchange pour toute unite sans moral).
 - Tests\HOLMoraleRulesTests.cpp : HeroesOfLiberty.Morale.FragmentPosesRefreshesExpiresRemoves.
RESTE :
 - Ajouter FHOLMoraleFragment aux archetypes d'unite a la creation (pas fait : sans lui GetFragmentDataPtr renvoie nul et rien n'est melange) ; mesurer la taille a 2 000 unites (D190) : a faire avec la sous-etape 4 (UHOLMoraleProcessor).
 - Sous-etapes 4 a 10 du plan, pont ApplyEffectAround (sous-etape 6) ; l'empreinte ne change pour les unites sous effet qu'a partir de la sous-etape 4.
NON VERIFIE : compilation (include de HOLMoraleRules.h dans HOLSimulationFragments.h, USTRUCT avec membre non UPROPERTY, lambda de Mix, include de HOLSimulationSubsystem.cpp), le rouge du premier commit, les cinq tests Morale.* existants et le nouveau, l'accord des cles apres RemoveMalus (suppose que RemoveMalus compacte les emplacements, sinon l'egalite « cle revenue au bonus seul » du test peut etre rouge pour une raison de forme, pas de regle), la fin de ligne des fichiers modifies (avertissements LF -> CRLF de git, sans consequence).
ANOMALIES_PROCESSUS: aucune. Ecart de consigne : lecture de HOLMoraleRules.h au 8e appel avant le premier Edit au 9e.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
GIT_STATUS_PRINCIPALE: docs modifies (AubeInProgress, Auto_Journey_File, Auto_Journey_Jour, ChatInProgress, EtapeDefini, MoraleInProgress) + non suivis Docs\* et Rapports\* ; hors docs voulus : non verifie fichier par fichier, aucun fichier de code touche par moi dans la copie principale ; mes ecritures : MoraleInProgress.md, EtapeDefini.md (1 ligne), Auto_Journey_File.md (1 ligne), ce rapport.
IN_USE.txt : « libre : » ecrit a la sortie.

## Commande a lancer (session principale, un build a la fois)
powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\build.ps1 -LiveCoding=0 -Wait
powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Tests\quick_check.ps1 -Parts tests
(attendu : Morale.FragmentPosesRefreshesExpiresRemoves vert, les 5 Morale.* de la sous-etape 2 toujours verts ; pour voir le rouge du test, compiler d'abord 9792b9f seul, ou `git revert --no-commit 61ee604`.)
Rapport DIAGNOSTICS_AS: sans objet (aucun .as touche).
