@echo off
title MQTT Broker - BugBuddys

:: -------------------------------------------------------
:: Pfade anpassen falls nötig
set MOSQUITTO_EXE="C:\Program Files\Mosquitto\mosquitto.exe"
set MOSQUITTO_CONF="C:\Program Files\Mosquitto\mosquitto.conf"
:: -------------------------------------------------------

echo  ╔══════════════════════════════════════╗
echo  ║      MQTT Broker - BugBuddys         ║
echo  ╚══════════════════════════════════════╝
echo.
echo  Broker:  test.mosquitto.org (lokal)
echo  Port:    1883
echo  Config:  %MOSQUITTO_CONF%
echo.
echo  Fenster offen lassen - hier erscheinen alle MQTT-Nachrichten.
echo  Zum Beenden: Strg+C oder Fenster schliessen.
echo.
echo ════════════════════════════════════════
echo.

%MOSQUITTO_EXE% -v -c %MOSQUITTO_CONF%

:: Falls Mosquitto sich beendet (z.B. Fehler), Fenster offen lassen
echo.
echo [BROKER BEENDET] Taste druecken zum Schliessen...
pause > nul
