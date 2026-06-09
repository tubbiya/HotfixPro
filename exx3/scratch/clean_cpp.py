# -*- coding: utf-8 -*-
import io

cpp_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(cpp_path, 'r', encoding='utf-8') as f:
    cpp_content = f.read()

# 1. Remove the bad block from OnBnClickedBtnList
bad_block = """    GetDlgItem(IDC_BTN_MAKINE_KALIBI2)->SetWindowText(L"\\x2699");
    m_bAyarOpen = FALSE;
    m_bCompactMode = FALSE;
    GetDlgItem(IDC_BTN_simge)->SetWindowText(L"-");
    
    CRect rcAdBoxDU(6, 10, 192, 22);
    MapDialogRect(&rcAdBoxDU);
    m_wndAdBox.Create(_T("Hotfix Pro - 0532 709 96 81"), WS_CHILD | SS_CENTERIMAGE | SS_CENTER, rcAdBoxDU, this, 3000);
    m_fontAd.CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
    m_wndAdBox.SetFont(&m_fontAd);"""
cpp_content = cpp_content.replace(bad_block, "")

# Ensure it's in OnInitDialog
if 'm_wndAdBox.Create' not in cpp_content:
    init_str = 'SetDialogHeight(184);'
    init_new = init_str + """
    m_bAyarOpen = FALSE;
    m_bCompactMode = FALSE;
    GetDlgItem(IDC_BTN_MAKINE_KALIBI2)->SetWindowText(L"\\x2699");
    GetDlgItem(IDC_BTN_simge)->SetWindowText(L"-");
    
    CRect rcAdBoxDU(6, 10, 192, 22);
    MapDialogRect(&rcAdBoxDU);
    m_wndAdBox.Create(_T("Hotfix Pro - 0532 709 96 81"), WS_CHILD | SS_CENTERIMAGE | SS_CENTER, rcAdBoxDU, this, 3000);
    m_fontAd.CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
    m_wndAdBox.SetFont(&m_fontAd);
"""
    cpp_content = cpp_content.replace(init_str, init_new)

# 2. Fix height calculation in OnBnClickedBtnList
# Replace the current height calc:
old_height_calc = """        // Yuksekligi nesne sayisina gore ayarla
        // Her satir yaklasik 15px, minimum 48px (Liste kutu)
        // IDC_GRP_LISTE'nin Y pozisyonu 179 DU
        // Kompakt: 15px * count + 20px (header) = dinamik liste yuksekligi
        {
            int rowCount = (int)m_listObjects.size();
            int rowH = 15; // piksel
            int grupIcYukseklik = rowCount * rowH + 20;
            int minGrupH = 30;
            if (grupIcYukseklik < minGrupH) grupIcYukseklik = minGrupH;
            
            // Liste grup kutusunu yeniden boyutlandir
            CWnd* pGrp = GetDlgItem(IDC_GRP_LISTE);
            if (pGrp) {
                CRect rcGrp;
                pGrp->GetWindowRect(&rcGrp);
                ScreenToClient(&rcGrp);
                pGrp->SetWindowPos(nullptr, rcGrp.left, rcGrp.top, rcGrp.Width(), grupIcYukseklik, SWP_NOZORDER | SWP_NOACTIVATE);
            }
            
            // Diyalog yuksekligini liste altina gore ayarla
            CRect rcDlg;
            GetWindowRect(&rcDlg);
            ScreenToClient(&rcDlg);
            CRect rcGrpFinal;
            if (pGroup) {
                pGroup->GetWindowRect(&rcGrpFinal);
                ScreenToClient(&rcGrpFinal);
                SetDialogHeight(rcGrpFinal.bottom + 5);
            }
        }"""

new_height_calc = """        {
            int grupIcYukseklik = 48;
            CWnd* pGrp = GetDlgItem(IDC_GRP_LISTE);
            if (pGrp) {
                CRect rcGrp;
                pGrp->GetWindowRect(&rcGrp);
                ScreenToClient(&rcGrp);
                
                if (m_bListOption) {
                    int rowCount = (int)m_listObjects.size();
                    int rowH = 15;
                    grupIcYukseklik = rowCount * rowH + 20;
                } else {
                    int boxW = 68;
                    int boxH = 16;
                    int spacing = 5;
                    int availableW = rcGrp.Width() - 16;
                    int columns = max(1, availableW / (boxW + spacing));
                    int rows = (int)m_listObjects.size() / columns;
                    if (m_listObjects.size() % columns != 0) rows++;
                    grupIcYukseklik = 18 + rows * (boxH + spacing) + 10;
                }
                
                int minGrupH = 48;
                if (grupIcYukseklik < minGrupH) grupIcYukseklik = minGrupH;
                
                pGrp->SetWindowPos(nullptr, rcGrp.left, rcGrp.top, rcGrp.Width(), grupIcYukseklik, SWP_NOZORDER | SWP_NOACTIVATE);
                SetDialogHeight(rcGrp.top + grupIcYukseklik + 5);
            }
        }"""

cpp_content = cpp_content.replace(old_height_calc, new_height_calc)

# Fix IDC_STATUSBAR to 10001
cpp_content = cpp_content.replace('id != IDC_STATUSBAR', 'id != 10001')

with io.open(cpp_path, 'w', encoding='utf-8') as f:
    f.write(cpp_content)

print("SearchDlg.cpp cleaned up.")
