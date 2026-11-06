@echo off
rem Morning launcher for the rest measures (S1-S7) on the main copy. Double-click it, or run it from any folder.
rem Before: close Unreal, ProjectShiva, browsers, Discord, and let every other Claude session sit idle (~15 min).
rem The script itself refuses to start if Unreal / a build runs or the processor is above 15 %.
cd /d C:\Users\baudo\Documents\MyCrusader
powershell -NoProfile -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\Tests\rest_measures.ps1
echo.
echo Summary: C:\Users\baudo\Documents\MyCrusader\Saved_Tests\Logs\rest_measures_*.txt (the newest one)
pause
