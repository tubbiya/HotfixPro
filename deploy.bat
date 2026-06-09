@echo off
echo =========================================================
echo Ztrass.cpg Eklenti Dagitim (Deploy) Araci
echo =========================================================
echo.
echo CorelDRAW arka planda kapali olmalidir.
echo.

echo Eski eklentiler temizleniyor...
del /f /q "C:\Program Files\Corel\CorelDRAW Graphics Suite X8\Draw\Plugins\HotfixProCPG.cpg" >nul 2>&1
del /f /q "C:\Program Files\Corel\CorelDRAW Graphics Suite X8\Draw\Plugins\Ztrass.cpg" >nul 2>&1

echo Yeni eklenti dosyasi kopyalaniyor...
copy /Y "C:\Users\tubbi\Dropbox\Claude\HotfixPro\out\Ztrass.cpg" "C:\Program Files\Corel\CorelDRAW Graphics Suite X8\Draw\Plugins\Ztrass.cpg"

if %errorLevel% == 0 (
    echo Eklenti basariyla kopyalandi!
) else (
    echo [HATA] Kopyalama basarisiz oldu. Lutfen deploy.bat dosyasina sag tiklayip YONETICI OLARAK CALISTIR diyerek acin.
)

pause