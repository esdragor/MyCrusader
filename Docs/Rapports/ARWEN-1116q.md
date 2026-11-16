# ARWEN-1116q : MIG-S1c-Diplomacy (partiel)

FAIT : commit 0240d17 sur d751-root (branche prete, merge main non fait). HOLSignalRules.as, HOLSignalBoardScript.as (ToBook), HOLSignalRules_Test.as, HOLDiplomacyPanel_Test.as lisent HOLDiplomacyRead (FHOLDiplomacyStances) : NewBook, NewTeamBook, SetStance, SetVision, StanceOf, SharesVision, IsAlly, Relation.

ROUGE VU a 22:34 : Test_SignalSeenByMutualAlliesOnly, attendu 0 -> 1 : tests 1345/1348. Remis : vert 1346/1348 (seuls rouges : Nomad D831).

A EXPOSER AU PROCHAIN LOT C++ (HOLDiplomacyRules.as et HOLDiplomacyRules_Test.as RESTENT) :
- constantes Enemy/Neutral/Ally/DefaultStance : HOLDiplomacyPanel.as:40-44,62-63,69-70,90-92,222-224 ; HOLGameScreenScript.as:454-456,551-563,750,767,805,885-889 ; Panel_Test, Rules_Test
- codes de resultat (Ok, NoSuchSlot, Self, BadStance, SenderIsAI, Locked, NotAlly) : HOLDiplomacyRules_Test.as
- fiches de changement FHOLStanceChange/FHOLVisionChange (bFormed, bAnnounce, bBroke, Before/After) : HOLDiplomacyRules_Test.as:40-141
- InMatch : HOLSignalRules.as (copie locale d'une ligne, Slot>=0 && Slot<Slots)
- DoorsOpenFor, CanGarrison, AlliedVictory : aucun appelant script hors tests

BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 (tests existants redirigés) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 supprimees (54+/48- redirigees) ; TYPES_A_EXPOSER: constantes de posture, EResult, FHOLStanceChange, FHOLVisionChange, InMatch.
NOTE : 2 quick_check -Parts tests (rouge puis vert) ; l'EtapeDefini/file/IN_USE mis a jour (IN_USE.txt est ignoré par git).
