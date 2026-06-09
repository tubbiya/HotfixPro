import os

path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\istek.md'
if os.path.exists(path):
    try:
        # Önce latin-1 olarak okuyup byte byte koruyoruz
        with open(path, 'rb') as f:
            raw = f.read()
        
        # UTF-8 veya Windows-1254/CP1254 decode denemeleri
        content = ""
        for enc in ['utf-8', 'cp1254', 'latin-1']:
            try:
                content = raw.decode(enc)
                break
            except Exception:
                continue
                
        if not content:
            content = raw.decode('latin-1', errors='ignore')
            
        # Satır sonlarını temizle ve \r\n olarak normalize et
        content = content.replace('\r\n', '\n').replace('\r', '\n').replace('\n', '\r\n')
            
        new_request = "\n\n### Ek Talep 35\n- Numune ve İmalat kalıplama pay mantığı optimize edildi. İmalat kalıbı standart limitlerine 500 mm (50) eklendi.\n"
        if "Ek Talep 35" not in content:
            content += new_request
            
        new_request2 = "\n\n### Ek Talep 36\n- Numune ve İmalat kalıp karesi oluşturulurken dolgunun kaldırılması (ApplyNoFill) ve nesnelerin en arkaya gönderilmesi (OrderToBack) eklendi.\n"
        if "Ek Talep 36" not in content:
            content += new_request2
            
        new_request3 = "\n\n### Ek Talep 37\n- İmalat kalıbı yükseklik payı hesaplanırken üst pay çıkarılarak sadece alt pay bırakıldı. Böylece gereksiz bir üst banta yuvarlanma engellendi.\n"
        if "Ek Talep 37" not in content:
            content += new_request3
            
        with open(path, 'w', encoding='utf-8-sig') as f:
            f.write(content)
        print("[ISTEK] istek.md basariyla guncellendi, temizlendi ve UTF-8-BOM yapildi.")
    except Exception as e:
        print(f"[HATA] istek.md guncellenemedi: {e}")
else:
    print("[HATA] istek.md bulunamadi.")
