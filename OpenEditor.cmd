@echo off
rem Ouvre l'editeur du jeu sur un poste d'artiste sans Visual Studio (D927) : mode d'emploi dans Docs\ARTISTES.md, section 0.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Tools\Art\open_editor.ps1" %*
if errorlevel 1 pause
