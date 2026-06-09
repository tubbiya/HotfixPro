# -*- coding: utf-8 -*-
import io

cpp_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(cpp_path, 'r', encoding='utf-8') as f:
    cpp_content = f.read()

# 1. Fix Unit
cpp_content = cpp_content.replace('SetLongProp(pDoc, L"Unit", 1);', 'SetLongProp(pDoc, L"Unit", 3); // cdrMillimeter')

# 2. Fix List box layout for Compact (m_bListOption == TRUE)
old_compact_draw = """                int startY = rcGroup.top + 14;
                int rowH = 15; // Sat?r yuksekligi
                int boxW = 52; // Renk kutusu genisligi
                
                for (size_t i = 0; i < m_listObjects.size(); ++i) {
                    const auto& obj = m_listObjects[i];
                    int rowY = startY + (int)i * rowH;
                    
                    CRect rcBox(rcGroup.left + 4, rowY, rcGroup.left + 4 + boxW, rowY + 12);"""

new_compact_draw = """                int startY = rcGroup.top + 14;
                int rowH = 18; // Sat?r yuksekligi (biraz daha ferah)
                int boxW = 80; // Renk kutusu genisligi artirildi
                
                for (size_t i = 0; i < m_listObjects.size(); ++i) {
                    const auto& obj = m_listObjects[i];
                    int rowY = startY + (int)i * rowH;
                    
                    CRect rcBox(rcGroup.left + 8, rowY, rcGroup.left + 8 + boxW, rowY + 14);"""
cpp_content = cpp_content.replace(old_compact_draw, new_compact_draw)

old_combos = """                CRect rcTas(rcGroup.left + 60, rowY, rcGroup.left + 148, rowY + 80);
                CRect rcTip(rcGroup.left + 152, rowY, rcGroup.left + 186, rowY + 60);"""

new_combos = """                CRect rcTas(rcGroup.left + 95, rowY, rcGroup.left + 195, rowY + 100);
                CRect rcTip(rcGroup.left + 200, rowY, rcGroup.left + 260, rowY + 80);"""
cpp_content = cpp_content.replace(old_combos, new_combos)

# Fix rowH in combo box creation
cpp_content = cpp_content.replace('int rowH = 15; // Kompakt satir yuksekligi', 'int rowH = 18; // Kompakt satir yuksekligi')

# 3. Fix List box layout for Horizontal (m_bListOption == FALSE)
old_horiz_draw = """                int startX = rcGroup.left + 8;
                int startY = rcGroup.top + 18;
                int boxW = 68;
                int boxH = 16;
                int spacing = 5;"""

new_horiz_draw = """                int startX = rcGroup.left + 8;
                int startY = rcGroup.top + 18;
                int boxW = 85;
                int boxH = 18;
                int spacing = 8;"""
cpp_content = cpp_content.replace(old_horiz_draw, new_horiz_draw)

# Note: The `GetDoubleProp` for `docW / 10.0` will now correctly return cm!
# E.g. Unit=3 (mm) -> docW=150mm -> docW / 10.0 = 15.0 cm!
# And the stones will say "2.0mm" correctly.

with io.open(cpp_path, 'w', encoding='utf-8') as f:
    f.write(cpp_content)

print("Layouts and units fixed!")
