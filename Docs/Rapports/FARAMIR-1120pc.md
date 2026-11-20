# FARAMIR-1120pc (aucun code : deux questions posees)

NON COMPILE (sans C++, pas de build, pas de quick_check). Aucun fichier de code modifie.

Ligne de la file annoncee : D859 (« porter les 10 valeurs de depart dans leurs tables », sans C++), 1re `[ ]` sans C++ du Socle commun (les D866, D860 [mixte C++], lot C++ groupe, recharges sont C++ ; D861 etc. sont [x] ou [~]). Son reste n'a plus de partie sans C++ libre.

APPELS_DECLARES: 20 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 15:54:35 / 15:57:45
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (aucun code). TEST_ROUGE_SEPARE: sans objet.
COMMITS: 1 sur zone-effect (a489422, journal AubeInProgress.md), merge_main_fait: oui (6 fichiers de docs, aucune DLL concernee)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; BUILDS_DEMANDES: 0 ; TYPES_A_EXPOSER: aucun nouveau
ESSAIS: aucun
QUESTIONS: Retours\Q-decide-jauge.md (defaut A : on garde le parametre Gauge de Decide) ; Retours\Q-chevalier-defi-ligne.md (defaut C : HOLDawnValues.as reste la source du Defi du Chevalier)

FAIT / RESTE:
- 1. Nettoyage HOLRpcPlayer.as l.1077 : NON FAIT, par constat. Grep : le `FHOLFactionResource Gauge` est passe a `HOLFactionPowerCast::Decide` (HOLFactionPowerCast.as:59), qui le lit (`Result.NewGaugeValue = Gauge.Value`, l.63 et 96). Le RPC ignore NewGaugeValue, mais 12 appels (HOLFactionPowerCast_Test.as x10, HOLFactionPowerCastEdge_Test.as x1, RPC x1) et ~25 assertions « jauge inchangee » (preuve D862, rouge 9b68d8a) dependent du parametre. Le retirer = changer la signature + perdre cette couverture : decision de design (A garder / B retirer), donc question, pas de modification.
- 2. D859 : verifie dans la file, AubeInProgress.md et CELEBORN-1119n2.md : n. 1a/1b (HALDIR-1120a), 1c (GLORFINDEL-1120a), 1d et 8 (CELEBORN-1120a/c), 6/7 (ligne LightSeal) faits ; n. 5 supplante par D862 ; n. 9 (XP du duel) = C++ (HOLHeroXPRules.h) ; n. 10 (Defi du Chevalier : ligne KnightChallenge dans UnitAbilities.csv) bloque : (a) l'unite VirtuousKnight n'existe que sur tool-known-reds (889a1cc), pas sur zone-effect ni main ; (b) la colonne Target n'a que Self (rayon 0 par convention) et Ground ; (c) HOLDawnValues.as est aujourd'hui la source lue par regex par ValueSheet.csv (migrer la ligne casserait la regex : une seule source D839 a decider) -> Q-chevalier-defi-ligne.md.
- Autres `[ ]` du Socle / Aube / Brouillard : D866 (C++ source unique), D860 (mixte C++), lot C++ groupe, recharges (C++), pose en ligne (a confirmer par l'utilisateur), talents_heal_cell (question de couche non tranchee, proprietaire du rendu des marques) : aucun n'est une tache sans C++ a prendre.
- Docs a jour : AubeInProgress.md (zone-effect, commit a489422), EtapeDefini.md (ligne FARAMIR-1120pc), Auto_Journey_File.md (ligne D859 completee), IN_USE.txt libre.
- RESTE : reponses aux 2 questions ; puis (si Q-decide-jauge = B) signature de Decide + tests ; (si Q-chevalier-defi-ligne = A ou B) ligne KnightChallenge apres l'integration de tool-known-reds.

NON VERIFIE: les constats (lecture de code et de docs uniquement, aucune execution). Je n'ai pas verifie que unit_abilities_test.py / les tests C++ de constantes d'aptitudes accepteraient une ligne dont l'unite n'existe pas dans UnitTypes.csv. DIAGNOSTICS_AS: sans objet (aucun .as modifie).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Faramir
ANOMALIES_PROCESSUS: numerotation de lignes de la file decalee de 1 entre Select-String et Get-Content (blocs lus par index) ; sans effet. EDITS_REFUSES: 0.
GIT_STATUS_PRINCIPALE: M AubeInProgress.md, Auto_Journey_File.md, Auto_Journey_Jour.md, ChatInProgress.md, EtapeDefini.md, FortificationsInProgress.md, Palier3InProgress.md, Placeholders.txt, SpecializationInProgress.md ; ?? Docs/*, Docs/Rapports/*, Retours/Q-decide-jauge.md, Retours/Q-chevalier-defi-ligne.md : docs seulement, mes ecritures = EtapeDefini.md, Auto_Journey_File.md, 2 Q ; aucun code modifie dans la copie principale : oui
