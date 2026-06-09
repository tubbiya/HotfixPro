import os

def replace_in_file(path, search_bytes, replace_bytes):
    with open(path, 'rb') as f:
        data = f.read()
    if search_bytes in data:
        data = data.replace(search_bytes, replace_bytes)
        with open(path, 'wb') as f:
            f.write(data)

rc_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\Ztrass.rc'

replace_in_file(rc_path, b'LTEXT           "2mm",IDC_STATIC,243,', b'LTEXT           "2mm",IDC_STATIC,348,')
replace_in_file(rc_path, b'LTEXT           "3mm",IDC_STATIC,243,', b'LTEXT           "3mm",IDC_STATIC,348,')
replace_in_file(rc_path, b'LTEXT           "4mm",IDC_STATIC,243,', b'LTEXT           "4mm",IDC_STATIC,348,')
replace_in_file(rc_path, b'LTEXT           "5mm",IDC_STATIC,243,', b'LTEXT           "5mm",IDC_STATIC,348,')
replace_in_file(rc_path, b'LTEXT           "6mm",IDC_STATIC,243,', b'LTEXT           "6mm",IDC_STATIC,348,')
replace_in_file(rc_path, b'LTEXT           "7mm",IDC_STATIC,243,', b'LTEXT           "7mm",IDC_STATIC,348,')
replace_in_file(rc_path, b'LTEXT           "8mm",IDC_STATIC,243,', b'LTEXT           "8mm",IDC_STATIC,348,')

replace_in_file(rc_path, b'EDITTEXT        IDC_EDT_2MM,263,', b'EDITTEXT        IDC_EDT_2MM,368,')
replace_in_file(rc_path, b'EDITTEXT        IDC_EDT_2MM2,263,', b'EDITTEXT        IDC_EDT_2MM2,368,')
replace_in_file(rc_path, b'EDITTEXT        IDC_EDT_2MM3,263,', b'EDITTEXT        IDC_EDT_2MM3,368,')
replace_in_file(rc_path, b'EDITTEXT        IDC_EDT_2MM4,263,', b'EDITTEXT        IDC_EDT_2MM4,368,')
replace_in_file(rc_path, b'EDITTEXT        IDC_EDT_2MM5,263,', b'EDITTEXT        IDC_EDT_2MM5,368,')
replace_in_file(rc_path, b'EDITTEXT        IDC_EDT_2MM6,263,', b'EDITTEXT        IDC_EDT_2MM6,368,')
replace_in_file(rc_path, b'EDITTEXT        IDC_EDT_2MM7,263,', b'EDITTEXT        IDC_EDT_2MM7,368,')

import io, re
with io.open(rc_path, 'r', encoding='Windows-1254') as f:
    rc_content = f.read()

rc_content = re.sub(r'GROUPBOX\s+"([^"]*)",IDC_GRP_KALIP_DUZENLE_AYAR2,237,', r'GROUPBOX        "\1",IDC_GRP_KALIP_DUZENLE_AYAR2,342,', rc_content)
rc_content = re.sub(r'PUSHBUTTON\s+"([^"]*)",IDC_BTN_MAKINE_KALIBI3,291,', r'PUSHBUTTON      "\1",IDC_BTN_MAKINE_KALIBI3,396,', rc_content)
rc_content = re.sub(r'PUSHBUTTON\s+"([^"]*)",IDC_BTN_MAKINE_KALIBI4,291,', r'PUSHBUTTON      "\1",IDC_BTN_MAKINE_KALIBI4,396,', rc_content)
rc_content = re.sub(r'PUSHBUTTON\s+"([^"]*)",IDC_BTN_MAKINE_KALIBI5,291,', r'PUSHBUTTON      "\1",IDC_BTN_MAKINE_KALIBI5,396,', rc_content)

with io.open(rc_path, 'w', encoding='Windows-1254') as f:
    f.write(rc_content)

print("Ayar elements shifted back to X=342 in Ztrass.rc")
