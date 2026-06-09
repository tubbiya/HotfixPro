import codecs

path = 'HotfixProCPG.rc'
try:
    try:
        with codecs.open(path, 'r', 'utf-8-sig') as f:
            content = f.read()
            encoding = 'utf-8-sig'
    except Exception:
        with codecs.open(path, 'r', 'mbcs') as f:
            content = f.read()
            encoding = 'mbcs'

    content = content.replace('#include "winresrc.h"', '')
    content = content.replace('#include "afxres.h"', '')
    content = content.replace('#include "winres.h"', '')

    styles = """#ifndef IDC_STATIC
#define IDC_STATIC          -1
#endif
#define DS_SETFONT          0x40L
#define DS_FIXEDSYS         0x0008L
#define WS_POPUP            0x80000000L
#define WS_CAPTION          0x00C00000L
#define WS_SYSMENU          0x00080000L
#define WS_THICKFRAME       0x00040000L
#define WS_TABSTOP          0x00010000L
#define WS_GROUP            0x00020000L
#define BS_AUTOCHECKBOX     0x00000003L
#define SS_ICON             0x00000003L
#define WS_BORDER           0x00800000L
#define ES_AUTOHSCROLL      0x0080L
"""

    if '#define DS_SETFONT' not in content:
        content = styles + "\r\n" + content

    with codecs.open(path, 'w', encoding) as f:
        f.write(content)
    print("RC dosyasi basariyla inline stillerle basina yazildi.")
except Exception as e:
    print(f"Hata: {e}")
