import io

path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path, 'r', encoding='utf-8') as f:
    text = f.read()

old_logic = """        CRect rcBase(0, 0, 190, 0);
        MapDialogRect(&rcBase);
        int baseWidth = rcBase.Width();
        int newWidth = m_bListOption ? 260 : baseWidth;"""

new_logic = """        CRect rcBase(0, 0, 190, 0);
        CRect rcWide(0, 0, 208, 0);
        MapDialogRect(&rcBase);
        MapDialogRect(&rcWide);
        int baseWidth = rcBase.Width();
        int wideWidth = rcWide.Width();
        int newWidth = m_bListOption ? wideWidth : baseWidth;"""

text = text.replace(old_logic, new_logic)

with io.open(path, 'w', encoding='utf-8') as f:
    f.write(text)

print("Updated list resize logic to be DPI-aware using MapDialogRect.")
