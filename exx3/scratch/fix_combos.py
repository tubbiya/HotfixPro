# -*- coding: utf-8 -*-
import io

cpp_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(cpp_path, 'r', encoding='utf-8') as f:
    cpp_content = f.read()

# 1. Fix Group Box Height calculation
old_calc_h = """            int rowCount = (int)m_listObjects.size();
            int rowH = 15; // piksel"""
new_calc_h = """            int rowCount = (int)m_listObjects.size();
            int rowH = m_bListOption ? 24 : 18; // piksel"""
cpp_content = cpp_content.replace(old_calc_h, new_calc_h)

# 2. Fix rowH in OnBnClickedChkListOption
old_chk_opt = """            int startY = rcGroup.top + 14;
            int rowH = 26; // Kompakt satir yuksekligi"""
new_chk_opt = """            int startY = rcGroup.top + 14;
            int rowH = 24; // Kompakt satir yuksekligi"""
cpp_content = cpp_content.replace(old_chk_opt, new_chk_opt)

# 3. Fix rcTas and rcTip and SetFont
old_combos = """                CRect rcTas(rcGroup.left + 95, rowY, rcGroup.left + 195, rowY + 100);
                CRect rcTip(rcGroup.left + 200, rowY, rcGroup.left + 260, rowY + 80);
                
                CComboBox* pTas = new CComboBox();
                pTas->Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL, rcTas, this, 2000 + i);"""
new_combos = """                CRect rcTas(rcGroup.left + 95, rowY, rcGroup.left + 225, rowY + 100);
                CRect rcTip(rcGroup.left + 235, rowY, rcGroup.left + 285, rowY + 80);
                
                CComboBox* pTas = new CComboBox();
                pTas->Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL, rcTas, this, 2000 + i);
                pTas->SetFont(GetFont());"""
cpp_content = cpp_content.replace(old_combos, new_combos)

old_combo_tip = """                CComboBox* pTip = new CComboBox();
                pTip->Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL, rcTip, this, 3000 + i);"""
new_combo_tip = """                CComboBox* pTip = new CComboBox();
                pTip->Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL, rcTip, this, 3000 + i);
                pTip->SetFont(GetFont());"""
cpp_content = cpp_content.replace(old_combo_tip, new_combo_tip)

# 4. Fix rowH in OnPaint
old_paint_vert = """                int startY = rcGroup.top + 14;
                int rowH = 26; // Sat?r yuksekligi (dropdownlar icin genisletildi)"""
new_paint_vert = """                int startY = rcGroup.top + 14;
                int rowH = 24; // Sat?r yuksekligi (dropdownlar icin genisletildi)"""
cpp_content = cpp_content.replace(old_paint_vert, new_paint_vert)

with io.open(cpp_path, 'w', encoding='utf-8') as f:
    f.write(cpp_content)

print("ComboBox fixes applied!")
