@echo off
setlocal

:: =========================================================
::  Ztrass Build Script
::  Kullanim: Developer Command Prompt'ta bu dosyayi calistirin
::  VEYA asagidaki MSVC yolunu sisteminizdeki gercek yola gore duzenleyin
:: =========================================================

set VCVARS="C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
if not exist %VCVARS% set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if not exist %VCVARS% set VCVARS="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if not exist %VCVARS% set VCVARS="C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
if not exist %VCVARS% set VCVARS="C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"

if not exist %VCVARS% (
    echo [HATA] Visual Studio Build Tools bulunamadi.
    echo Lutfen Visual Studio Build Tools kurun:
    echo   https://aka.ms/vs/17/release/vs_buildtools.exe
    echo Bitti.
    exit /b 1
)

call %VCVARS%

:: =========================================================
::  Otomatik Kaynak Kod Yedekleme (ZIP)
:: =========================================================
echo [YEDEK] Kaynak kodlari yedekleniyor...
if not exist backuptmp mkdir backuptmp
copy /Y *.cpp backuptmp\ >nul
copy /Y *.h backuptmp\ >nul
copy /Y *.rc backuptmp\ >nul
copy /Y *.def backuptmp\ >nul
copy /Y *.vcxproj backuptmp\ >nul
copy /Y *.sln backuptmp\ >nul

if not exist backups mkdir backups
powershell -Command "$dt = Get-Date -Format 'yyyyMMdd_HHmmss'; Compress-Archive -Path backuptmp\* -DestinationPath backups\Ztrass_Backup_$dt.zip -Force"
rmdir /s /q backuptmp
echo [YEDEK] Yedekleme tamamlandi.
:: =========================================================

:: Kodlama Duzeltici Python Scripti Olustur
echo import os, codecs > fix_encoding.py
echo def fix_encoding(filepath): >> fix_encoding.py
echo     try: >> fix_encoding.py
echo         with open(filepath, 'rb') as f: content = f.read() >> fix_encoding.py
echo         if content.startswith(codecs.BOM_UTF8): return >> fix_encoding.py
echo         try: text = content.decode('utf-8') >> fix_encoding.py
echo         except: >> fix_encoding.py
echo             try: text = content.decode('windows-1254') >> fix_encoding.py
echo             except: text = content.decode('latin1') >> fix_encoding.py
echo         with open(filepath, 'wb') as f: f.write(codecs.BOM_UTF8 + text.encode('utf-8')) >> fix_encoding.py
echo     except Exception as e: print("Encoding error in", filepath, e) >> fix_encoding.py
echo for f in os.listdir('.'): >> fix_encoding.py
echo     if f.endswith('.cpp') or f.endswith('.h') or f.endswith('.rc'): >> fix_encoding.py
echo         fix_encoding(f) >> fix_encoding.py
py fix_encoding.py
:: Cikti klasoru
if not exist out mkdir out

:: istek.md guncelle ve kodlamasini duzelt
if exist fix_istek.py (
    py fix_istek.py
)

:: Kaynak dosyalari derle
echo [1/3] RC kaynak derleniyor...
rc /fo out\Ztrass.res Ztrass.rc
if errorlevel 1 goto FAIL

echo [2/3] C++ kaynaklari derleniyor...
cl /c /EHsc /MT /W3 /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "_WINDLL" /D "_UNICODE" /D "UNICODE" /utf-8 ^
   SearchDlg.cpp Ztrass.cpp ^
   /Fo"out\\"
if errorlevel 1 goto FAIL

echo [3/3] DLL linkleniyor...
link /DLL /DEF:Ztrass.def /OUT:"out\Ztrass.dll" ^
   out\SearchDlg.obj out\Ztrass.obj out\Ztrass.res ^
   kernel32.lib user32.lib gdi32.lib ole32.lib oleaut32.lib
if errorlevel 1 goto FAIL

:: CPG olarak kopyala
copy /Y "out\Ztrass.dll" "out\Ztrass.cpg"

:: =========================================================

echo.
echo =========================================================
echo  BASARILI! Cikti: out\Ztrass.cpg
echo  Otomatik dagitim (deploy) baslatiliyor...
echo =========================================================

echo CorelDRAW kapatiliyor...
echo CorelDRAW kapatilmayacak.

echo Eski eklenti kalintilari siliniyor...
del /f /q "C:\Program Files\Corel\CorelDRAW Graphics Suite X8\Draw\Plugins\HotfixProCPG.cpg" >nul 2>&1
del /f /q "C:\Program Files\Corel\CorelDRAW Graphics Suite X8\Draw\Plugins\Ztrass.cpg" >nul 2>&1

echo Yeni eklenti dosyasi kopyalaniyor...
copy /Y "out\Ztrass.cpg" "C:\Program Files\Corel\CorelDRAW Graphics Suite X8\Draw\Plugins\Ztrass.cpg"

if %errorLevel% == 0 (
    echo Eklenti basariyla kopyalandi!
) else (
    echo [HATA] Kopyalama basarisiz oldu! CorelDRAW acik oldugu icin Ztrass.cpg kilitli olabilir.
    echo Lutfen CorelDRAW'i tamamen kapatip bu betigi yeniden calistirin.
)

goto END

:FAIL
echo.
echo [HATA] Derleme basarisiz! Yukaridaki hatalari kontrol edin.
echo Bitti.
exit /b 1

:END
echo Bitti.
