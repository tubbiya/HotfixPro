# -*- coding: utf-8 -*-
import io

cpp_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(cpp_path, 'r', encoding='utf-8') as f:
    cpp_content = f.read()

# 1. Combobox widths
old_combos = """                CRect rcTas(rcGroup.left + 95, rowY, rcGroup.left + 225, rowY + 100);
                CRect rcTip(rcGroup.left + 235, rowY, rcGroup.left + 285, rowY + 80);"""

new_combos = """                CRect rcTas(rcGroup.left + 93, rowY, rcGroup.left + 203, rowY + 100);
                CRect rcTip(rcGroup.left + 208, rowY, rcGroup.left + 248, rowY + 80);"""
cpp_content = cpp_content.replace(old_combos, new_combos)

# 2. GroupBox width limit
old_grp_resize = """            // Liste grup kutusunu yeniden boyutlandir
            CWnd* pGrp = GetDlgItem(IDC_GRP_LISTE);
            if (pGrp) {
                CRect rcGrp;
                pGrp->GetWindowRect(&rcGrp);
                ScreenToClient(&rcGrp);
                pGrp->SetWindowPos(nullptr, rcGrp.left, rcGrp.top, rcGrp.Width(), grupIcYukseklik, SWP_NOZORDER | SWP_NOACTIVATE);
            }"""

new_grp_resize = """            // Liste grup kutusunu yeniden boyutlandir
            CWnd* pGrp = GetDlgItem(IDC_GRP_LISTE);
            if (pGrp) {
                CRect rcGrp;
                pGrp->GetWindowRect(&rcGrp);
                ScreenToClient(&rcGrp);
                int newWidth = m_bListOption ? 255 : rcGrp.Width();
                pGrp->SetWindowPos(nullptr, rcGrp.left, rcGrp.top, newWidth, grupIcYukseklik, SWP_NOZORDER | SWP_NOACTIVATE);
            }"""
cpp_content = cpp_content.replace(old_grp_resize, new_grp_resize)

# 3. Limit dialog height to screen height
old_dlg_resize = """                // Yukseklik = Liste kutu alti + 8px bosluk
                CRect rcThis;
                GetWindowRect(&rcThis);
                int newH = rcGrpFinal.bottom + 32; // + 24 for statusbar
                SetWindowPos(nullptr, 0, 0, rcThis.Width(), newH, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);"""

new_dlg_resize = """                // Yukseklik = Liste kutu alti + 8px bosluk
                CRect rcThis;
                GetWindowRect(&rcThis);
                int newH = rcGrpFinal.bottom + 32; // + 24 for statusbar
                int screenH = GetSystemMetrics(SM_CYSCREEN) - 100;
                if (newH > screenH) newH = screenH; // Ekrana sigmama durumunda max yukseklik
                SetWindowPos(nullptr, 0, 0, rcThis.Width(), newH, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);"""
cpp_content = cpp_content.replace(old_dlg_resize, new_dlg_resize)

with io.open(cpp_path, 'w', encoding='utf-8') as f:
    f.write(cpp_content)

print("List dimension fixes applied!")
