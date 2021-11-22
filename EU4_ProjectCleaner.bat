@echo on

REM CLEAR UNEEDED FILES

RD /s /q Intermediate
RD /s /q Saved
RD /s /q Binaries
RD /s /q .vs
DEL *.sln
Rem DEL *.bin