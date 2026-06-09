import io
bat = """@echo off
setlocal

echo Yonetici yetkileri aliniyor...
net session >nul 2>&1
if %errorLevel% == 0 (
    goto admin
) else (
    powershell -Command "Start-Process '%~f0' -Verb RunAs"
    exit /b
)

:admin
echo CorelDRAW kapatiliyor...
taskkill /F /IM CorelDRW.exe /T >nul 2>&1
timeout /t 2 /nobreak >nul

echo Eski eklenti kalintilari siliniyor...
del /f /q "C:\\Program Files\\Corel\\CorelDRAW Graphics Suite X8\\Draw\\Plugins\\HotfixProCPG.cpg" >nul 2>&1

echo Yeni eklenti dosyasi kopyalaniyor...
copy /Y "%~dp0Ztrass.cpg" "C:\\Program Files\\Corel\\CorelDRAW Graphics Suite X8\\Draw\\Plugins\\" >nul 2>&1

if %errorLevel% == 0 (
    echo Eklenti basariyla kopyalandi!
    echo CorelDRAW yeniden baslatiliyor...
    start "" "C:\\Program Files\\Corel\\CorelDRAW Graphics Suite X8\\Programs64\\CorelDRW.exe"
) else (
    echo [HATA] Kopyalama basarisiz oldu.
)

timeout /t 3
"""

with io.open(r"C:\Users\tubbi\Dropbox\Claude\HotfixPro\out\deploy.bat", "w", encoding="utf-8") as f:
    f.write(bat)

print("Written deploy.bat")
