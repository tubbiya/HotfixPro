# -*- coding: utf-8 -*-
import io
import re

rc_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\Ztrass.rc'
with io.open(rc_path, 'r', encoding='cp1254') as f:
    rc_content = f.read()

rc_content = re.sub(r'GROUPBOX\s+"Ara \(Ctrl\+F\)",IDC_STATIC,', 'GROUPBOX        "Ara (Ctrl+F)",IDC_GRP_ARA,', rc_content)
# "Ölçü" with ascii/ansi encoding may appear as "\xd6l\xe7\xfc" or similar
rc_content = re.sub(r'GROUPBOX\s+"[^"]*",IDC_STATIC,(\d+),(56),', r'GROUPBOX        "Ölçü",IDC_GRP_OLCU,\1,\2,', rc_content)
rc_content = re.sub(r'GROUPBOX\s+"Kesi[^"]*Objeler",IDC_STATIC,', 'GROUPBOX        "Kesişen Objeler",IDC_GRP_KESISEN,', rc_content)
rc_content = re.sub(r'GROUPBOX\s+"PLt Dxf Export",IDC_STATIC,', 'GROUPBOX        "PLt Dxf Export",IDC_GRP_EXPORT,', rc_content)

# Fix the actual text replacing if regex failed due to encoding
rc_content = rc_content.replace('GROUPBOX        "Ölçü",IDC_STATIC,', 'GROUPBOX        "Ölçü",IDC_GRP_OLCU,')
rc_content = rc_content.replace('GROUPBOX        "Ölçü",IDC_STATIC,6,56', 'GROUPBOX        "Ölçü",IDC_GRP_OLCU,6,56')

# Just use direct replacements based on coordinates since IDC_STATIC is there
rc_content = rc_content.replace('IDC_STATIC,6,6,190,48', 'IDC_GRP_ARA,6,6,190,48')
rc_content = rc_content.replace('IDC_STATIC,6,56,190,42', 'IDC_GRP_OLCU,6,56,190,42')
rc_content = rc_content.replace('IDC_STATIC,6,102,190,42', 'IDC_GRP_KESISEN,6,102,190,42')
rc_content = rc_content.replace('IDC_STATIC,6,148,95,27', 'IDC_GRP_EXPORT,6,148,95,27')

with io.open(rc_path, 'w', encoding='cp1254') as f:
    f.write(rc_content)

h_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\resource.h'
with io.open(h_path, 'r', encoding='cp1254') as f:
    h_content = f.read()

if 'IDC_GRP_ARA' not in h_content:
    h_content = h_content.replace('#define _APS_NEXT_RESOURCE_VALUE', 
"""#define IDC_GRP_ARA                     1600
#define IDC_GRP_OLCU                    1601
#define IDC_GRP_KESISEN                 1602
#define IDC_GRP_EXPORT                  1603
#define _APS_NEXT_RESOURCE_VALUE""")

with io.open(h_path, 'w', encoding='cp1254') as f:
    f.write(h_content)

print("IDs assigned.")
