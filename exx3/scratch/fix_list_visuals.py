# -*- coding: utf-8 -*-
import io

cpp_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(cpp_path, 'r', encoding='utf-8') as f:
    cpp_content = f.read()

# 1. Background color
cpp_content = cpp_content.replace('m_brushListe.CreateSolidBrush(RGB(255, 255, 153));', 'm_brushListe.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));')

# 2. Text formatting
old_text_format = """            CString objText;
            objText.Format(_T("%.1fmm-%d"), g.width, g.count);
            objText.Replace(_T("."), _T(","));"""

new_text_format = """            CString objText;
            double w = round(g.width * 10.0) / 10.0;
            if (w == floor(w)) {
                objText.Format(_T("%.0fmm-%d"), w, g.count);
            } else {
                objText.Format(_T("%.1fmm-%d"), w, g.count);
            }
            objText.Replace(_T("."), _T(","));"""

cpp_content = cpp_content.replace(old_text_format, new_text_format)

# 3. Horizontal layout & font size
old_horiz = """                int startX = rcGroup.left + 8;
                int startY = rcGroup.top + 18;
                int boxW = 85;
                int boxH = 18;
                int spacing = 8;"""

new_horiz = """                int startX = rcGroup.left + 8;
                int startY = rcGroup.top + 18;
                int boxW = 110;
                int boxH = 18;
                int spacing = 8;"""
cpp_content = cpp_content.replace(old_horiz, new_horiz)

old_draw_text_horiz = """                    dc.SetBkMode(TRANSPARENT);
                    dc.DrawText(obj.text, &rcBox, DT_CENTER | DT_VCENTER | DT_SINGLELINE);"""

new_draw_text_horiz = """                    dc.SetBkMode(TRANSPARENT);
                    CFont smallFont;
                    smallFont.CreatePointFont(80, _T("MS Shell Dlg"));
                    CFont* pOldFont = dc.SelectObject(&smallFont);
                    dc.DrawText(obj.text, &rcBox, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    dc.SelectObject(pOldFont);"""
cpp_content = cpp_content.replace(old_draw_text_horiz, new_draw_text_horiz)

# 4. Vertical layout (rowH = 26)
old_vert_draw = """                int startY = rcGroup.top + 14;
                int rowH = 18; // Sat?r yuksekligi (biraz daha ferah)
                int boxW = 80; // Renk kutusu genisligi artirildi
                
                for (size_t i = 0; i < m_listObjects.size(); ++i) {
                    const auto& obj = m_listObjects[i];
                    int rowY = startY + (int)i * rowH;
                    
                    CRect rcBox(rcGroup.left + 8, rowY, rcGroup.left + 8 + boxW, rowY + 14);"""

new_vert_draw = """                int startY = rcGroup.top + 14;
                int rowH = 26; // Sat?r yuksekligi (dropdownlar icin genisletildi)
                int boxW = 80; // Renk kutusu genisligi artirildi
                
                for (size_t i = 0; i < m_listObjects.size(); ++i) {
                    const auto& obj = m_listObjects[i];
                    int rowY = startY + (int)i * rowH;
                    
                    CRect rcBox(rcGroup.left + 8, rowY + 4, rcGroup.left + 8 + boxW, rowY + 20);"""
cpp_content = cpp_content.replace(old_vert_draw, new_vert_draw)

# 5. Fix rowH in OnBnClickedChkListOption
cpp_content = cpp_content.replace('int rowH = 18; // Kompakt satir yuksekligi', 'int rowH = 26; // Kompakt satir yuksekligi')

with io.open(cpp_path, 'w', encoding='utf-8') as f:
    f.write(cpp_content)

print("List updates applied!")
