import os

def replace_in_file(path, search_bytes, replace_bytes):
    with open(path, 'rb') as f:
        data = f.read()
    if search_bytes in data:
        data = data.replace(search_bytes, replace_bytes)
        with open(path, 'wb') as f:
            f.write(data)

rc_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\Ztrass.rc'

# Labels (X: 348 -> 347, Y: +1)
replace_in_file(rc_path, b'LTEXT           "2mm",IDC_STATIC,348,18', b'LTEXT           "2mm",IDC_STATIC,347,19')
replace_in_file(rc_path, b'LTEXT           "3mm",IDC_STATIC,348,32', b'LTEXT           "3mm",IDC_STATIC,347,33')
replace_in_file(rc_path, b'LTEXT           "4mm",IDC_STATIC,348,46', b'LTEXT           "4mm",IDC_STATIC,347,47')
replace_in_file(rc_path, b'LTEXT           "5mm",IDC_STATIC,348,60', b'LTEXT           "5mm",IDC_STATIC,347,61')
replace_in_file(rc_path, b'LTEXT           "6mm",IDC_STATIC,348,74', b'LTEXT           "6mm",IDC_STATIC,347,75')
replace_in_file(rc_path, b'LTEXT           "7mm",IDC_STATIC,348,88', b'LTEXT           "7mm",IDC_STATIC,347,89')
replace_in_file(rc_path, b'LTEXT           "8mm",IDC_STATIC,348,102', b'LTEXT           "8mm",IDC_STATIC,347,103')

# Edits (X: 368 -> 371, Y: +1)
replace_in_file(rc_path, b'EDITTEXT        IDC_EDT_2MM,368,16', b'EDITTEXT        IDC_EDT_2MM,371,17')
replace_in_file(rc_path, b'EDITTEXT        IDC_EDT_2MM2,368,30', b'EDITTEXT        IDC_EDT_2MM2,371,31')
replace_in_file(rc_path, b'EDITTEXT        IDC_EDT_2MM3,368,44', b'EDITTEXT        IDC_EDT_2MM3,371,45')
replace_in_file(rc_path, b'EDITTEXT        IDC_EDT_2MM4,368,58', b'EDITTEXT        IDC_EDT_2MM4,371,59')
replace_in_file(rc_path, b'EDITTEXT        IDC_EDT_2MM5,368,72', b'EDITTEXT        IDC_EDT_2MM5,371,73')
replace_in_file(rc_path, b'EDITTEXT        IDC_EDT_2MM6,368,86', b'EDITTEXT        IDC_EDT_2MM6,371,87')
replace_in_file(rc_path, b'EDITTEXT        IDC_EDT_2MM7,368,100', b'EDITTEXT        IDC_EDT_2MM7,371,101')

print("Aligned Ayar2 inner elements to match Bilgi offsets perfectly.")
