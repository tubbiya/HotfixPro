# -*- coding: utf-8 -*-
import io
import re

rc_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\Ztrass.rc'
with io.open(rc_path, 'r', encoding='cp1254') as f:
    rc_content = f.read()

# Shift coordinates by -105 in X
replacements = {
    '348,18,16,8': '243,18,16,8',
    '368,16,24,12': '263,16,24,12',
    '348,32,16,8': '243,32,16,8',
    '368,30,24,12': '263,30,24,12',
    '348,46,16,8': '243,46,16,8',
    '368,44,24,12': '263,44,24,12',
    '348,60,16,8': '243,60,16,8',
    '368,58,24,12': '263,58,24,12',
    '348,74,16,8': '243,74,16,8',
    '368,72,24,12': '263,72,24,12',
    '348,88,16,8': '243,88,16,8',
    '368,86,24,12': '263,86,24,12',
    '368,100,24,12': '263,100,24,12',
    '348,102,16,8': '243,102,16,8',
    '342,6,99,113': '237,6,99,113',
    '396,100,40,12': '291,100,40,12',
    '396,86,40,12': '291,86,40,12',
    '396,72,40,12': '291,72,40,12'
}

for k, v in replacements.items():
    rc_content = rc_content.replace(k, v)

with io.open(rc_path, 'w', encoding='cp1254') as f:
    f.write(rc_content)


h_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.h'
with io.open(h_path, 'r', encoding='utf-8') as f:
    h_lines = f.readlines()

for i, line in enumerate(h_lines):
    if 'afx_msg void OnBnClickedChkKalip();' in line:
        h_lines.insert(i, '    afx_msg void OnBnClickedAyarToggle();\n')
        break

with io.open(h_path, 'w', encoding='utf-8') as f:
    f.writelines(h_lines)


cpp_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(cpp_path, 'r', encoding='utf-8') as f:
    cpp_content = f.read()

# Add message map
cpp_content = cpp_content.replace('ON_BN_CLICKED(IDC_CHK_KALIP, &CSearchDlg::OnBnClickedChkKalip)',
                                  'ON_BN_CLICKED(IDC_CHK_KALIP, &CSearchDlg::OnBnClickedChkKalip)\n    ON_BN_CLICKED(IDC_BTN_MAKINE_KALIBI2, &CSearchDlg::OnBnClickedAyarToggle)')

# Set icon in OnInitDialog
init_str = 'SetDialogHeight(184);'
cpp_content = cpp_content.replace(init_str, init_str + '\n    GetDlgItem(IDC_BTN_MAKINE_KALIBI2)->SetWindowText(L"\\x2699");\n    m_bAyarOpen = FALSE;')

# Modify ShowKalipPanel
# Initially, ShowKalipPanel showed both. Now it should hide Ayar when showing Bilgi.
show_panel_old = """void CSearchDlg::ShowKalipPanel(BOOL bShow)
{
    int nShow = bShow ? SW_SHOW : SW_HIDE;
    int ids[] = {
        IDC_GRP_KALIP_BILGI, IDC_EDT_FIRMA, IDC_EDT_DOSYA, IDC_EDT_KALIP_ADI,
        IDC_EDT_PRES, IDC_EDT_MONTAJ, IDC_BTN_BILGI_EKLE, IDC_EDT_KALIPTA_IS, IDC_EDT_TOPLAM_IS,
        IDC_GRP_KALIP_DUZENLE, IDC_BTN_KALIP_EKLE, IDC_EDT_KALIP_OLCU,
        IDC_BTN_ISARET_TASI, IDC_EDT_TASI_OLCU, IDC_BTN_KALIP_AYIR, IDC_BTN_MAKINE_KALIBI,
        IDC_EDT_2MM, IDC_EDT_3MM, IDC_EDT_4MM, IDC_EDT_5MM, IDC_EDT_6MM, IDC_EDT_7MM
    };
    for (int id : ids) {
        CWnd* p = GetDlgItem(id);
        if (p) p->ShowWindow(nShow);
    }
    // LTEXT (IDC_STATIC) tipleri tek tek handle edilemez; grup kutulari OnCtlColor'da renk alacak
    // Dialog genisligini ayarla
    if (bShow) {
        SetDialogWidth(450);
    } else {
        SetDialogWidth(230);
    }
    Invalidate();
}"""

show_panel_new = """void CSearchDlg::ShowKalipPanel(BOOL bShow)
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
    }
    
    // Show static texts inside the Ayar groupbox. Since they are IDC_STATIC, we hide them by their layout position
    // We can also just hide all static controls that overlap with Ayar.
    CWnd* pWnd = GetWindow(GW_CHILD);
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
    }

    if (bShow) {
        SetDialogWidth(350);
    } else {
        SetDialogWidth(230);
    }
    Invalidate();
}

void CSearchDlg::OnBnClickedAyarToggle()
{
    m_bAyarOpen = !m_bAyarOpen;
    ShowKalipPanel(m_bKalip);
}
"""

cpp_content = cpp_content.replace(show_panel_old, show_panel_new)

with io.open(cpp_path, 'w', encoding='utf-8') as f:
    f.write(cpp_content)

print("Done")
