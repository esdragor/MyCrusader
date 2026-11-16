# CELEBORN-1116s : S1e, exposer les constantes et fiches Diplomacy, les temps de la rage

Branche hero-j8-close (worktree HeroesOfLiberty-Celeborn), commit unique 94966ee "S1e: expose Diplomacy constants and change fiches, rage times". Motif C++ : exposer au script des regles qui restent en C++ (D836, D839). Aucun changement de comportement.

AJOUTS
- HOLDiplomacyReadLibrary : GetStanceEnemy/Neutral/Ally/Default (0/1/2/Enemy), GetResultOk/NoSuchSlot/Self/BadStance/SenderIsAI/Locked/NotAlly (0..6), USTRUCT FHOLDiplomacyStanceChange (Result, bAnnounce, From, To, Stance, Before, After, bBroke, bFormed) et FHOLDiplomacyVisionChange (Result, bAnnounce, From, To, bShare), ChangeStance / ChangeVision (retournent la fiche ; SetStance / SetVision gardent leur int32).
- HOLUnitAbilityReadLibrary : GetRageCooldownEndMs(StartMs), GetRageEffectEndMs(StartMs), IsRageEffectEnded(StartMs, NowMs, Health).
- HOLUnitAbilityRules.h : 3 fonctions inline neuves (le calcul de temps n'existait PAS en C++, seulement dans l'ancien HOLUnitAbilityRules.as supprime par ELROND-1116p ; reprise a l'identique : Start+Cooldown, Start+Duree, Now >= fin ou Health <= StopHp).
- Script\Tests\HOLS1eLibraries_Test.as : 3 tests de fumee (Test_DiplomacyConstantsReadLibrarySmoke, Test_DiplomacyChangeFicheReadLibrarySmoke, Test_UnitAbilityRageTimesReadLibrarySmoke). Ils ne compilent qu'apres le build (attendu).

ECART A L'ENONCE (a connaitre pour la redirection)
- Les fiches s'appellent FHOLDiplomacyStanceChange / FHOLDiplomacyVisionChange et non FHOLStanceChange / FHOLVisionChange : ces deux noms existent deja dans Script\Diplomacy\HOLDiplomacyRules.as (qu'on ne devait pas toucher) ; un USTRUCT de meme nom aurait casse la compilation des scripts apres le build (meme precaution que FHOLDiplomacyStances / FHOLDiplomacyBook). Le lot suivant renomme les usages en retirant HOLDiplomacyRules.as.
- Les constantes sont des fonctions GetStance*/GetResult* (int32), comme GetRage* : le script ecrit HOLDiplomacyRead::GetStanceAlly() la ou il ecrivait HOLDiplomacy::Ally. Le champ Result des fiches est un int32 (pas un enum).
- Le fichier de regles C++ est Match\HOLDiplomacyRules.h (pas HOLDiplomacy.h).

INCIDENT
- Une premiere ecriture a vide les 5 fichiers C++ (alias PowerShell `rd` = Remove-Item) ; restaures par git checkout -- Source avant tout commit, puis reecrits : le diff du commit ne contient que des ajouts (208 insertions, 0 suppression).

BUILD A LANCER (session principale) : <worktree>\Tools\build.ps1 -LiveCoding=0 -Wait, puis quick_check -Parts tests (les 3 tests S1e et les scripts doivent compiler).

CHAMPS
BUILDS_DEMANDES: 1 (a lancer, non fait ici) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 3 (Test_DiplomacyConstantsReadLibrarySmoke, Test_DiplomacyChangeFicheReadLibrarySmoke, Test_UnitAbilityRageTimesReadLibrarySmoke) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 (ajouts 208 lignes C++, 52 lignes script de test) ; TYPES_A_EXPOSER: restants pour (1) : InMatch (cite par ARWEN-1116q) non traite, a verifier au lot de redirection ; TEST_ROUGE: non fait (aucune execution possible avant build) ; EDITS_REFUSES: 0 ; PROCESSUS_DETACHES: voir fin.
NON VERIFIE : aucune compilation C++ ni script ; la justesse des valeurs des 3 tests (Before/After = 0/2, Result Ok au 2e ChangeVision entre allies) repose sur la lecture de HOLDiplomacyRules.h, pas sur une execution.