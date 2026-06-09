# -*- coding: utf-8 -*-
import io
import re

rc_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\Ztrass.rc'
with io.open(rc_path, 'r', encoding='Windows-1254') as f:
    rc_content = f.read()

# LTEXT for mm labels
rc_content = re.sub(r'LTEXT\s+"2mm",IDC_STATIC,348,', r'LTEXT           "2mm",IDC_STATIC,243,', rc_content)
rc_content = re.sub(r'LTEXT\s+"3mm",IDC_STATIC,348,', r'LTEXT           "3mm",IDC_STATIC,243,', rc_content)
rc_content = re.sub(r'LTEXT\s+"4mm",IDC_STATIC,348,', r'LTEXT           "4mm",IDC_STATIC,243,', rc_content)
rc_content = re.sub(r'LTEXT\s+"5mm",IDC_STATIC,348,', r'LTEXT           "5mm",IDC_STATIC,243,', rc_content)
rc_content = re.sub(r'LTEXT\s+"6mm",IDC_STATIC,348,', r'LTEXT           "6mm",IDC_STATIC,243,', rc_content)
rc_content = re.sub(r'LTEXT\s+"7mm",IDC_STATIC,348,', r'LTEXT           "7mm",IDC_STATIC,243,', rc_content)
rc_content = re.sub(r'LTEXT\s+"8mm",IDC_STATIC,348,', r'LTEXT           "8mm",IDC_STATIC,243,', rc_content)

# EDITTEXT for mm values
rc_content = re.sub(r'EDITTEXT\s+IDC_EDT_2MM,368,', r'EDITTEXT        IDC_EDT_2MM,263,', rc_content)
rc_content = re.sub(r'EDITTEXT\s+IDC_EDT_2MM2,368,', r'EDITTEXT        IDC_EDT_2MM2,263,', rc_content)
rc_content = re.sub(r'EDITTEXT\s+IDC_EDT_2MM3,368,', r'EDITTEXT        IDC_EDT_2MM3,263,', rc_content)
rc_content = re.sub(r'EDITTEXT\s+IDC_EDT_2MM4,368,', r'EDITTEXT        IDC_EDT_2MM4,263,', rc_content)
rc_content = re.sub(r'EDITTEXT\s+IDC_EDT_2MM5,368,', r'EDITTEXT        IDC_EDT_2MM5,263,', rc_content)
rc_content = re.sub(r'EDITTEXT\s+IDC_EDT_2MM6,368,', r'EDITTEXT        IDC_EDT_2MM6,263,', rc_content)
rc_content = re.sub(r'EDITTEXT\s+IDC_EDT_2MM7,368,', r'EDITTEXT        IDC_EDT_2MM7,263,', rc_content)

# Groupbox and buttons (using capture groups to preserve Turkish characters safely)
rc_content = re.sub(r'GROUPBOX\s+"([^"]*)",IDC_GRP_KALIP_DUZENLE_AYAR2,342,', r'GROUPBOX        "\1",IDC_GRP_KALIP_DUZENLE_AYAR2,237,', rc_content)
rc_content = re.sub(r'PUSHBUTTON\s+"([^"]*)",IDC_BTN_MAKINE_KALIBI3,396,', r'PUSHBUTTON      "\1",IDC_BTN_MAKINE_KALIBI3,291,', rc_content)
rc_content = re.sub(r'PUSHBUTTON\s+"([^"]*)",IDC_BTN_MAKINE_KALIBI4,396,', r'PUSHBUTTON      "\1",IDC_BTN_MAKINE_KALIBI4,291,', rc_content)
rc_content = re.sub(r'PUSHBUTTON\s+"([^"]*)",IDC_BTN_MAKINE_KALIBI5,396,', r'PUSHBUTTON      "\1",IDC_BTN_MAKINE_KALIBI5,291,', rc_content)

with io.open(rc_path, 'w', encoding='Windows-1254') as f:
    f.write(rc_content)

print("Ztrass.rc shifted.")
