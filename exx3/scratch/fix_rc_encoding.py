import io
import re

path_rc = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\Ztrass.rc'
with io.open(path_rc, 'r', encoding='utf-8') as f:
    text_rc = f.read()

replacements = {
    '"l"': '"Ölçü"',
    '"Kontr l"': '"Kontür Ölçü"',
    '"Tr"': '"Tür"',
    '" Renk"': '"İç Renk"',
    '"Kontr Renk"': '"Kontür Renk"',
    '"Grup i"': '"Grup İçi"',
    '"Dm Says"': '"Düğüm Sayısı"',
    '"Alt Yol Says"': '"Alt Yol Sayısı"',
    '"Se"': '"Seç"',
    '"st"': '"Üst"',
    '"Ayn"': '"Aynı"',
    '"Ayr"': '"Ayır"',
    '"Kalp"': '"Kalıp"',
    '"Kalp Bilgi"': '"Kalıp Bilgi"',
    '"Kalp Dzenle"': '"Kalıp Düzenle"',
    '"aret Ta"': '"İşaret Taşı"',
    '"Kalpla"': '"Kalıpla"',
    '"Kalp Ayr"': '"Kalıp Ayır"',
    '"Makine Kalb Hazrla"': '"Makine Kalıbı Hazırla"',
    '"Makine Kalb lleri"': '"Makine Kalıbı Ölçüleri"',
    '"Sfrla"': '"Sıfırla"',
    '"Kalp Ad"': '"Kalıp Adı"',
    '"Kalptaki"': '"Kalıptaki"',
    '"Ta mm"': '"Taş mm"',
    # I and l issues (from previous manual check if any left)
    '"Kontr I"': '"Kontür Ölçü"',
    '"Tr"': '"Tür"',
    '"Dm Says"': '"Düğüm Sayısı"',
    '"st"': '"Üst"',
    '"Ayn"': '"Aynı"',
    '"Kalp"': '"Kalıp"',
    '"Ayr"': '"Ayır"',
    '"Se"': '"Seç"',
    '"I"': '"Ölçü"'
}

for k, v in replacements.items():
    text_rc = text_rc.replace(k, v)

# Re-save with windows-1254 encoding
with io.open(path_rc, 'w', encoding='windows-1254', errors='replace') as f:
    f.write(text_rc)

print("Fixed Ztrass.rc encoding")
