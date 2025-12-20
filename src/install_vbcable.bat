@echo off
echo VB-CABLE Installation Check and Setup
echo =====================================

REM Check if VB-CABLE is already installed
echo Checking for VB-CABLE installation...
set VB_CABLE_FOUND=0

REM Check for 32-bit VB-CABLE
if exist "C:\Windows\System32\drivers\vbaudio_cable64.sys" (
    set VB_CABLE_FOUND=1
    echo VB-CABLE 64-bit driver found
)

REM Check for VB-CABLE in Program Files
if exist "C:\Program Files\VB-CABLE" (
    set VB_CABLE_FOUND=1
    echo VB-CABLE installation folder found
)

REM Check in Program Files (x86)
if exist "C:\Program Files (x86)\VB-CABLE" (
    set VB_CABLE_FOUND=1
    echo VB-CABLE (x86) installation folder found
)

if %VB_CABLE_FOUND%==1 (
    echo.
    echo VB-CABLE is already installed!
    echo Please restart the VoiceChanger application.
    pause
    exit /b 0
)

echo.
echo VB-CABLE not found on system.
echo Starting VB-CABLE installation...

REM Check if installer exists in current directory
if exist "VBCABLE_Setup_x64.exe" (
    echo Found VB-CABLE installer in current directory
    echo Running installer...
    VBCABLE_Setup_x64.exe
    goto :check_restart
)

REM Try to download VB-CABLE (basic download attempt)
echo VB-CABLE installer not found.
echo Please download VB-CABLE from: https://vb-audio.com/Cable/
echo.
echo Steps:
echo 1. Go to https://vb-audio.com/Cable/
echo 2. Download VB-CABLE A+B (free version)
echo 3. Run this script again from the same folder as the installer
echo.
pause
exit /b 1

:check_restart
echo.
echo VB-CABLE installation completed!
echo IMPORTANT: You must restart your computer for the audio drivers to load properly.
echo.
echo After restart:
echo 1. Run VoiceChanger application
echo 2. Click "Virtual Device" button
echo 3. Select CABLE Output from the dropdown
echo.
echo Would you like to restart now? (Y/N)
set /p restart_choice=

if /i "%restart_choice%"=="Y" (
    echo Restarting computer...
    shutdown /r /t 10
) else (
    echo Please restart your computer manually to complete installation.
)

pause