import os, codecs 
def fix_encoding(filepath): 
    try: 
        with open(filepath, 'rb') as f: content = f.read() 
        if content.startswith(codecs.BOM_UTF8): return 
        try: text = content.decode('utf-8') 
        except: 
            try: text = content.decode('windows-1254') 
            except: text = content.decode('latin1') 
        with open(filepath, 'wb') as f: f.write(codecs.BOM_UTF8 + text.encode('utf-8')) 
    except Exception as e: print("Encoding error in", filepath, e) 
for f in os.listdir('.'): 
    if f.endswith('.cpp') or f.endswith('.h') or f.endswith('.rc'): 
        fix_encoding(f) 
