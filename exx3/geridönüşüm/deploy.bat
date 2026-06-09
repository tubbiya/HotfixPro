@echo off
:: Yonetici yetkisi kontrolu
NET SESSION >nul 2>&1
if %errorLevel% == 0 (
    goto :ADMIN
) else (
    echo Yonetici yetkileri aliniyor...
    powershell -Command "Start-Process '%~f0' -Verb RunAs"
    exit /b
)

:ADMIN
echo CorelDRAW kapatiliyor...
taskkill /f /im CorelDRW.exe >nul 2>&1
timeout /t 1 /nobreak >nul

echo Eski eklenti kalintilari siliniyor...
del /q "C:\Program Files\Corel\CorelDRAW Graphics Suite X8\Draw\Plugins\HotfixProCPG.cpg" >nul 2>&1

echo Yeni eklenti dosyasi kopyalaniyor...
copy /Y "%~dp0Ztrass.cpg" "C:\Program Files\Corel\CorelDRAW Graphics Suite X8\Draw\Plugins\"
if %errorLevel% == 0 (
    echo Eklenti basariyla kopyalandi!
) else (
    echo [HATA] Kopyalama basarisiz! CorelDRAW'in kapandigindan emin olun.
    pause
    exit /b 1
)

echo CorelDRAW yeniden baslatiliyor...
start "" "C:\Program Files\Corel\CorelDRAW Graphics Suite X8\Programs64\CorelDRW.exe"
exit
