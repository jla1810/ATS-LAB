@echo off
echo Nettoyage ATS LAB avant recompilation...
del /q *.dcu 2>nul
del /q *.exe 2>nul
del /q *.identcache 2>nul
del /q *.dproj.local 2>nul
if exist Win32 rmdir /s /q Win32
echo.
echo Nettoyage termine.
echo Ouvre maintenant ATSLab_v0070.dproj dans Delphi et fais:
echo Projet ^> Construire ATSLab_v0070
pause
