# -*- coding: utf-8 -*-
import io

# 1. Restore Ztrass.rc
rc_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\Ztrass.rc'
with io.open(rc_path, 'r', encoding='cp1254') as f:
    rc_content = f.read()

replacements = {
    '243,18,16,8': '348,18,16,8',
    '263,16,24,12': '368,16,24,12',
    '243,32,16,8': '348,32,16,8',
    '263,30,24,12': '368,30,24,12',
    '243,46,16,8': '348,46,16,8',
    '263,44,24,12': '368,44,24,12',
    '243,60,16,8': '348,60,16,8',
    '263,58,24,12': '368,58,24,12',
    '243,74,16,8': '348,74,16,8',
    '263,72,24,12': '368,72,24,12',
    '243,88,16,8': '348,88,16,8',
    '263,86,24,12': '368,86,24,12',
    '263,100,24,12': '368,100,24,12',
    '243,102,16,8': '348,102,16,8',
    '237,6,99,113': '342,6,99,113',
    '291,100,40,12': '396,100,40,12',
    '291,86,40,12': '396,86,40,12',
    '291,72,40,12': '396,72,40,12'
}
for k, v in replacements.items():
    rc_content = rc_content.replace(k, v)

with io.open(rc_path, 'w', encoding='cp1254') as f:
    f.write(rc_content)


cpp_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(cpp_path, 'r', encoding='utf-8') as f:
    cpp_content = f.read()

# 2. Fix GetColorRef to support Outline Color when Type != 1
# And use ConvertToRGB!
color_func_old = """static COLORREF GetColorRef(IDispatch* pColor)
{
    if (!pColor) return RGB(128,128,128);
    long colorType = GetLongProp(pColor, L"Type");
    if (colorType == 2) { // RGB
        long r = GetLongProp(pColor, L"RGBRed");
        long g = GetLongProp(pColor, L"RGBGreen");
        long b = GetLongProp(pColor, L"RGBBlue");
        return RGB(r, g, b);
    }
    else if (colorType == 3) { // CMYK
        long c = GetLongProp(pColor, L"CMYKCyan");
        long m = GetLongProp(pColor, L"CMYKMagenta");
        long y = GetLongProp(pColor, L"CMYKYellow");
        long k = GetLongProp(pColor, L"CMYKBlack");
        int r = (int)(255 * (1 - c/100.0) * (1 - k/100.0));
        int g = (int)(255 * (1 - m/100.0) * (1 - k/100.0));
        int b = (int)(255 * (1 - y/100.0) * (1 - k/100.0));
        return RGB(r, g, b);
    }
    return RGB(128,128,128);
}"""

color_func_new = """static COLORREF GetColorRef(IDispatch* pColor)
{
    if (!pColor) return RGB(128,128,128);
    InvokeMethodNoArgs(pColor, L"ConvertToRGB");
    long r = GetLongProp(pColor, L"RGBRed");
    long g = GetLongProp(pColor, L"RGBGreen");
    long b = GetLongProp(pColor, L"RGBBlue");
    return RGB(r, g, b);
}"""
cpp_content = cpp_content.replace(color_func_old, color_func_new)

# Modify color fetching in list objects
color_fetch_old = """                COLORREF col = RGB(128, 128, 128);
                IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
                if (pFill) {
                    long fillType = GetLongProp(pFill, L"Type");
                    if (fillType == 1) {
                        IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                        if (pColor) {
                            col = GetColorRef(pColor);
                            pColor->Release();
                        }
                    }
                    pFill->Release();
                }"""

color_fetch_new = """                COLORREF col = RGB(128, 128, 128);
                IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
                long fillType = 0;
                if (pFill) {
                    fillType = GetLongProp(pFill, L"Type");
                    if (fillType == 1) {
                        IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                        if (pColor) {
                            col = GetColorRef(pColor);
                            pColor->Release();
                        }
                    }
                    pFill->Release();
                }
                if (fillType != 1) {
                    IDispatch* pOutline = GetDispatchProp(pShape, L"Outline");
                    if (pOutline) {
                        IDispatch* pColor = GetDispatchProp(pOutline, L"Color");
                        if (pColor) {
                            col = GetColorRef(pColor);
                            pColor->Release();
                        }
                        pOutline->Release();
                    }
                }"""
cpp_content = cpp_content.replace(color_fetch_old, color_fetch_new)

# 3. Change icon to text
cpp_content = cpp_content.replace('GetDlgItem(IDC_BTN_MAKINE_KALIBI2)->SetWindowText(L"\\x2699");', 'GetDlgItem(IDC_BTN_MAKINE_KALIBI2)->SetWindowText(L"Ayar");')

# 4. Hide Düzenle & Bilgi when Ayar is open, and dynamically move Ayar panel at init
show_panel_old = """void CSearchDlg::ShowKalipPanel(BOOL bShow)
{
    int nShow = bShow ? SW_SHOW : SW_HIDE;
    int ids[] = {
        IDC_GRP_KALIP_DUZENLE, IDC_BTN_KALIP_EKLE, IDC_EDT_KALIP_OLCU,
        IDC_BTN_ISARET_TASI, IDC_EDT_TASI_OLCU, IDC_BTN_KALIP_AYIR, IDC_BTN_MAKINE_KALIBI,
        IDC_BTN_MAKINE_KALIBI2
    };
    for (int id : ids) {
        CWnd* p = GetDlgItem(id);
        if (p) p->ShowWindow(nShow);
    }
    
    // Default to showing Bilgi, hiding Ayar
    int idsBilgi[] = {
        IDC_GRP_KALIP_BILGI, IDC_EDT_FIRMA, IDC_EDT_DOSYA, IDC_EDT_KALIP_ADI,
        IDC_EDT_PRES, IDC_EDT_MONTAJ, IDC_BTN_BILGI_EKLE, IDC_EDT_KALIPTA_IS, IDC_EDT_TOPLAM_IS
    };
    for (int id : idsBilgi) {
        CWnd* p = GetDlgItem(id);
        if (p) p->ShowWindow(bShow ? (m_bAyarOpen ? SW_HIDE : SW_SHOW) : SW_HIDE);
    }
    
    int idsAyar[] = {
        IDC_GRP_KALIP_DUZENLE_AYAR2, IDC_EDT_2MM, IDC_EDT_2MM2, IDC_EDT_2MM3, IDC_EDT_2MM4, IDC_EDT_2MM5, IDC_EDT_2MM6, IDC_EDT_2MM7,
        IDC_BTN_MAKINE_KALIBI3, IDC_BTN_MAKINE_KALIBI4, IDC_BTN_MAKINE_KALIBI5
    };
    for (int id : idsAyar) {
        CWnd* p = GetDlgItem(id);
        if (p) p->ShowWindow(bShow ? (m_bAyarOpen ? SW_SHOW : SW_HIDE) : SW_HIDE);
    }"""

show_panel_new = """void CSearchDlg::ShowKalipPanel(BOOL bShow)
{
    int nShow = bShow ? SW_SHOW : SW_HIDE;
    
    int idsDuzenle[] = {
        IDC_GRP_KALIP_DUZENLE, IDC_BTN_KALIP_EKLE, IDC_EDT_KALIP_OLCU,
        IDC_BTN_ISARET_TASI, IDC_EDT_TASI_OLCU, IDC_BTN_KALIP_AYIR, IDC_BTN_MAKINE_KALIBI,
        IDC_BTN_MAKINE_KALIBI2
    };
    for (int id : idsDuzenle) {
        CWnd* p = GetDlgItem(id);
        if (p) p->ShowWindow(bShow ? (m_bAyarOpen ? SW_HIDE : SW_SHOW) : SW_HIDE);
    }
    
    int idsBilgi[] = {
        IDC_GRP_KALIP_BILGI, IDC_EDT_FIRMA, IDC_EDT_DOSYA, IDC_EDT_KALIP_ADI,
        IDC_EDT_PRES, IDC_EDT_MONTAJ, IDC_BTN_BILGI_EKLE, IDC_EDT_KALIPTA_IS, IDC_EDT_TOPLAM_IS
    };
    for (int id : idsBilgi) {
        CWnd* p = GetDlgItem(id);
        if (p) p->ShowWindow(bShow ? (m_bAyarOpen ? SW_HIDE : SW_SHOW) : SW_HIDE);
    }
    
    int idsAyar[] = {
        IDC_GRP_KALIP_DUZENLE_AYAR2, IDC_EDT_2MM, IDC_EDT_2MM2, IDC_EDT_2MM3, IDC_EDT_2MM4, IDC_EDT_2MM5, IDC_EDT_2MM6, IDC_EDT_2MM7,
        IDC_BTN_MAKINE_KALIBI3, IDC_BTN_MAKINE_KALIBI4, IDC_BTN_MAKINE_KALIBI5
    };
    for (int id : idsAyar) {
        CWnd* p = GetDlgItem(id);
        if (p) p->ShowWindow(bShow ? (m_bAyarOpen ? SW_SHOW : SW_HIDE) : SW_HIDE);
    }"""
cpp_content = cpp_content.replace(show_panel_old, show_panel_new)

# Add event mapping for İptal and Kaydet buttons
cpp_content = cpp_content.replace('ON_BN_CLICKED(IDC_BTN_MAKINE_KALIBI2, &CSearchDlg::OnBnClickedAyarToggle)',
                                  'ON_BN_CLICKED(IDC_BTN_MAKINE_KALIBI2, &CSearchDlg::OnBnClickedAyarToggle)\n    ON_BN_CLICKED(IDC_BTN_MAKINE_KALIBI3, &CSearchDlg::OnBnClickedAyarToggle)\n    ON_BN_CLICKED(IDC_BTN_MAKINE_KALIBI4, &CSearchDlg::OnBnClickedAyarToggle)')

# Move elements in OnInitDialog
init_inject = """
    // Move Ayar elements overlapping to the Bilgi panel
    CRect rShift(0, 0, 105, 0);
    MapDialogRect(&rShift);
    int idsAyarMove[] = {
        IDC_GRP_KALIP_DUZENLE_AYAR2, IDC_EDT_2MM, IDC_EDT_2MM2, IDC_EDT_2MM3, IDC_EDT_2MM4, IDC_EDT_2MM5, IDC_EDT_2MM6, IDC_EDT_2MM7,
        IDC_BTN_MAKINE_KALIBI3, IDC_BTN_MAKINE_KALIBI4, IDC_BTN_MAKINE_KALIBI5
    };
    for (int id : idsAyarMove) {
        CWnd* p = GetDlgItem(id);
        if (p) {
            CRect rc;
            p->GetWindowRect(&rc);
            ScreenToClient(&rc);
            rc.OffsetRect(-rShift.right, 0);
            p->SetWindowPos(nullptr, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        }
    }
    CWnd* pWnd = GetWindow(GW_CHILD);
    while (pWnd) {
        TCHAR szClass[32];
        ::GetClassName(pWnd->GetSafeHwnd(), szClass, 32);
        if (_wcsicmp(szClass, L"Static") == 0) {
            CRect rc;
            pWnd->GetWindowRect(&rc);
            ScreenToClient(&rc);
            CRect rcAyar(342, 6, 441, 119); 
            MapDialogRect(&rcAyar);
            if (rcAyar.PtInRect(rc.CenterPoint())) {
                CString strText;
                pWnd->GetWindowText(strText);
                if (strText.Find(_T("mm")) != -1 && strText.Find(_T("Firma")) == -1 && strText.Find(_T("Dosya")) == -1) {
                    rc.OffsetRect(-rShift.right, 0);
                    pWnd->SetWindowPos(nullptr, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                }
            }
        }
        pWnd = pWnd->GetWindow(GW_HWNDNEXT);
    }
"""

cpp_content = cpp_content.replace('GetDlgItem(IDC_BTN_simge)->SetWindowText(L"-");', 'GetDlgItem(IDC_BTN_simge)->SetWindowText(L"-");' + init_inject)

# Also remove the hardcoded RC offset static check in ShowKalipPanel (we already moved them to overlapping pos)
show_static_old = """    CWnd* pWnd = GetWindow(GW_CHILD);
    while (pWnd) {
        TCHAR szClass[32];
        ::GetClassName(pWnd->GetSafeHwnd(), szClass, 32);
        if (_wcsicmp(szClass, L"Static") == 0) {
            CRect rc;
            pWnd->GetWindowRect(&rc);
            ScreenToClient(&rc);
            // static controls for 2mm, 3mm etc are around x=243
            CRect rcAyar(237, 6, 336, 119); MapDialogRect(&rcAyar);
            if (rcAyar.PtInRect(rc.CenterPoint())) {
                CString strText;
                pWnd->GetWindowText(strText);
                if (strText.Find(_T("mm")) != -1 && strText.Find(_T("Firma")) == -1 && strText.Find(_T("Dosya")) == -1) {
                    pWnd->ShowWindow(bShow ? (m_bAyarOpen ? SW_SHOW : SW_HIDE) : SW_HIDE);
                }
            }
        }
        pWnd = pWnd->GetWindow(GW_HWNDNEXT);
    }"""

show_static_new = """    CWnd* pWnd = GetWindow(GW_CHILD);
    while (pWnd) {
        TCHAR szClass[32];
        ::GetClassName(pWnd->GetSafeHwnd(), szClass, 32);
        if (_wcsicmp(szClass, L"Static") == 0) {
            CRect rc;
            pWnd->GetWindowRect(&rc);
            ScreenToClient(&rc);
            // Since they were offset by -105 DU in OnInitDialog, their X is now around 237 DU
            CRect rcAyar(237, 6, 336, 119); MapDialogRect(&rcAyar);
            if (rcAyar.PtInRect(rc.CenterPoint())) {
                CString strText;
                pWnd->GetWindowText(strText);
                if (strText.Find(_T("mm")) != -1 && strText.Find(_T("Firma")) == -1 && strText.Find(_T("Dosya")) == -1) {
                    pWnd->ShowWindow(bShow ? (m_bAyarOpen ? SW_SHOW : SW_HIDE) : SW_HIDE);
                }
            }
        }
        pWnd = pWnd->GetWindow(GW_HWNDNEXT);
    }"""
cpp_content = cpp_content.replace(show_static_old, show_static_new)

with io.open(cpp_path, 'w', encoding='utf-8') as f:
    f.write(cpp_content)

print("Ayar paneli ve renk cekme duzeltildi.")
