# -*- coding: utf-8 -*-
import io

# 1. Edit SearchDlg.h
h_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.h'
with io.open(h_path, 'r', encoding='utf-8') as f:
    h_lines = f.readlines()

for i, line in enumerate(h_lines):
    if 'afx_msg void OnBnClickedAyarToggle();' in line:
        h_lines.insert(i, '    afx_msg void OnBnClickedSimge();\n')
        break

for i, line in enumerate(h_lines):
    if 'BOOL m_bAyarOpen;' in line:
        h_lines.insert(i, '    BOOL m_bCompactMode;\n    std::vector<HWND> m_vecVisibleControls;\n    CStatic m_wndAdBox;\n    CFont m_fontAd;\n    void SetCompactMode(BOOL bCompact);\n')
        break

with io.open(h_path, 'w', encoding='utf-8') as f:
    f.writelines(h_lines)

# 2. Edit SearchDlg.cpp
cpp_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(cpp_path, 'r', encoding='utf-8') as f:
    cpp_content = f.read()

# Add message map
cpp_content = cpp_content.replace('ON_BN_CLICKED(IDC_BTN_MAKINE_KALIBI2, &CSearchDlg::OnBnClickedAyarToggle)',
                                  'ON_BN_CLICKED(IDC_BTN_MAKINE_KALIBI2, &CSearchDlg::OnBnClickedAyarToggle)\n    ON_BN_CLICKED(IDC_BTN_simge, &CSearchDlg::OnBnClickedSimge)')

# Add to OnInitDialog
init_str = 'm_bAyarOpen = FALSE;'
init_new = init_str + """
    m_bCompactMode = FALSE;
    GetDlgItem(IDC_BTN_simge)->SetWindowText(L"-");
    
    CRect rcAdBoxDU(6, 10, 192, 22);
    MapDialogRect(&rcAdBoxDU);
    m_wndAdBox.Create(_T("Hotfix Pro - 0532 709 96 81"), WS_CHILD | SS_CENTERIMAGE | SS_CENTER, rcAdBoxDU, this, 3000);
    m_fontAd.CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
    m_wndAdBox.SetFont(&m_fontAd);
"""
cpp_content = cpp_content.replace(init_str, init_new)

# Add to OnCtlColor
ctl_str = 'if (nCtlColor == CTLCOLOR_STATIC) {'
ctl_new = ctl_str + """
        if (pWnd->GetDlgCtrlID() == 3000) {
            pDC->SetTextColor(RGB(255, 128, 0));
            pDC->SetBkColor(RGB(0, 0, 0));
            return (HBRUSH)GetStockObject(BLACK_BRUSH);
        }
"""
cpp_content = cpp_content.replace(ctl_str, ctl_new)

# Add functions at the end
funcs = """
void CSearchDlg::OnBnClickedSimge()
{
    SetCompactMode(!m_bCompactMode);
}

void CSearchDlg::SetCompactMode(BOOL bCompact)
{
    m_bCompactMode = bCompact;
    if (bCompact) {
        m_vecVisibleControls.clear();
        CWnd* pWnd = GetWindow(GW_CHILD);
        while (pWnd) {
            if (pWnd->IsWindowVisible()) {
                m_vecVisibleControls.push_back(pWnd->GetSafeHwnd());
                int id = pWnd->GetDlgCtrlID();
                if (id != IDC_BTN_simge && id != 3000 && id != IDC_STATUSBAR) {
                    pWnd->ShowWindow(SW_HIDE);
                }
            }
            pWnd = pWnd->GetWindow(GW_HWNDNEXT);
        }
        m_wndAdBox.ShowWindow(SW_SHOW);
        SetDialogHeight(32);
        GetDlgItem(IDC_BTN_simge)->SetWindowText(_T("+"));
    } else {
        m_wndAdBox.ShowWindow(SW_HIDE);
        for (HWND hWnd : m_vecVisibleControls) {
            if (::IsWindow(hWnd)) {
                ::ShowWindow(hWnd, SW_SHOW);
            }
        }
        m_vecVisibleControls.clear();
        SetDialogHeight(m_bListOpen ? 232 : 184);
        GetDlgItem(IDC_BTN_simge)->SetWindowText(_T("-"));
    }
    Invalidate();
}
"""
cpp_content += funcs

with io.open(cpp_path, 'w', encoding='utf-8') as f:
    f.write(cpp_content)

print("Compact mode added.")
