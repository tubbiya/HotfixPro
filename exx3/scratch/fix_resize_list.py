import io

path_h = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.h'
with io.open(path_h, 'r', encoding='utf-8') as f:
    text_h = f.read()

text_h = text_h.replace('void ClearDynamicCombos();', 'void ClearDynamicCombos();\n    void UpdateListLayout();')

with io.open(path_h, 'w', encoding='utf-8') as f:
    f.write(text_h)


path_cpp = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path_cpp, 'r', encoding='utf-8') as f:
    text_cpp = f.read()

update_layout_func = """void CSearchDlg::UpdateListLayout()
{
    if (!m_bListOpen) return;
    
    int rowCount = (int)m_listObjects.size();
    if (rowCount == 0) return;
    
    int rowH = m_bListOption ? 24 : 18; // piksel
    int grupIcYukseklik = rowCount * rowH + 20;
    int minGrupH = 30;
    if (grupIcYukseklik < minGrupH) grupIcYukseklik = minGrupH;
    
    // Liste grup kutusunu yeniden boyutlandir
    CWnd* pGrp = GetDlgItem(IDC_GRP_LISTE);
    if (pGrp) {
        CRect rcGrp;
        pGrp->GetWindowRect(&rcGrp);
        ScreenToClient(&rcGrp);
        
        CRect rcBase(0, 0, 190, 0);
        MapDialogRect(&rcBase);
        int baseWidth = rcBase.Width();
        int newWidth = m_bListOption ? 260 : baseWidth; 
        
        pGrp->SetWindowPos(nullptr, rcGrp.left, rcGrp.top, newWidth, grupIcYukseklik, SWP_NOZORDER | SWP_NOACTIVATE);
    }
    
    // Diyalog yuksekligini liste altina gore ayarla
    CRect rcDlg;
    GetWindowRect(&rcDlg);
    CRect rcGrpFinal;
    if (pGrp) {
        pGrp->GetWindowRect(&rcGrpFinal);
        ScreenToClient(&rcGrpFinal);
        // Yukseklik = Liste kutu alti + 8px bosluk
        CRect rcThis;
        GetWindowRect(&rcThis);
        int newH = rcGrpFinal.bottom + 32; // + 24 for statusbar
        int screenH = GetSystemMetrics(SM_CYSCREEN) - 100;
        if (newH > screenH) newH = screenH; // Ekrana sigmama durumunda max yukseklik
        SetWindowPos(nullptr, 0, 0, rcThis.Width(), newH, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}
"""

# Insert the function just before ClearDynamicCombos in cpp
target_insert = "void CSearchDlg::ClearDynamicCombos()"
if target_insert in text_cpp:
    text_cpp = text_cpp.replace(target_insert, update_layout_func + "\n" + target_insert)

# Now remove the old block from OnBnClickedBtnList
old_block = """        // Yuksekligi nesne sayisina gore ayarla
        // Her satir yaklasik 15px, minimum 48px (Liste kutu)
        // IDC_GRP_LISTE'nin Y pozisyonu 179 DU
        // Kompakt: 15px * count + 20px (header) = dinamik liste yuksekligi
        {
            int rowCount = (int)m_listObjects.size();
            int rowH = m_bListOption ? 24 : 18; // piksel
            int grupIcYukseklik = rowCount * rowH + 20;
            int minGrupH = 30;
            if (grupIcYukseklik < minGrupH) grupIcYukseklik = minGrupH;
            
            // Liste grup kutusunu yeniden boyutlandir
            CWnd* pGrp = GetDlgItem(IDC_GRP_LISTE);
            if (pGrp) {
                CRect rcGrp;
                pGrp->GetWindowRect(&rcGrp);
                ScreenToClient(&rcGrp);
                int newWidth = m_bListOption ? 255 : rcGrp.Width();
                pGrp->SetWindowPos(nullptr, rcGrp.left, rcGrp.top, newWidth, grupIcYukseklik, SWP_NOZORDER | SWP_NOACTIVATE);
            }
            
            // Diyalog yuksekligini liste altina gore ayarla
            CRect rcDlg;
            GetWindowRect(&rcDlg);
            ScreenToClient(&rcDlg);
            CRect rcGrpFinal;
            if (pGroup) {
                pGroup->GetWindowRect(&rcGrpFinal);
                ScreenToClient(&rcGrpFinal);
                // Yukseklik = Liste kutu alti + 8px bosluk
                CRect rcThis;
                GetWindowRect(&rcThis);
                int newH = rcGrpFinal.bottom + 32; // + 24 for statusbar
                int screenH = GetSystemMetrics(SM_CYSCREEN) - 100;
                if (newH > screenH) newH = screenH; // Ekrana sigmama durumunda max yukseklik
                SetWindowPos(nullptr, 0, 0, rcThis.Width(), newH, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }"""

text_cpp = text_cpp.replace(old_block, "        UpdateListLayout();")

# Now add UpdateListLayout() at the end of OnBnClickedChkListOption
chk_list_option_end = """        }
    }
    SetRedraw(TRUE);
    Invalidate();"""

new_chk_list_option_end = """        }
        UpdateListLayout();
    }
    else if (m_bListOpen && !m_listObjects.empty()) {
        UpdateListLayout();
    }
    SetRedraw(TRUE);
    Invalidate();"""

text_cpp = text_cpp.replace(chk_list_option_end, new_chk_list_option_end)

with io.open(path_cpp, 'w', encoding='utf-8') as f:
    f.write(text_cpp)

print("UpdateListLayout implemented.")
