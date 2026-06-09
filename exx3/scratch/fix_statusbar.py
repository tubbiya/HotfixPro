import io
import re

path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path, 'r', encoding='utf-8') as f:
    text = f.read()

# 1. Update OnBnClickedBtnList 184 -> 198
text = re.sub(r"SetDialogHeight\(184\);(\s+CWnd\* pGroup = GetDlgItem\(IDC_GRP_LISTE\);)", r"SetDialogHeight(198);\1", text)

# 2. Update ToggleCompactMode
old_toggle = """        m_vecVisibleControls.clear();
        SetDialogHeight(m_bListOpen ? 232 : 184);
        GetDlgItem(IDC_BTN_simge)->SetWindowText(_T("-"));"""

new_toggle = """        m_vecVisibleControls.clear();
        if (m_bListOpen) {
            UpdateListLayout();
        } else {
            SetDialogHeight(198);
        }
        GetDlgItem(IDC_BTN_simge)->SetWindowText(_T("-"));"""

text = text.replace(old_toggle, new_toggle)

# 3. Update OnSize
old_onsize = """void CSearchDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);
    if (m_wndStatusBar.GetSafeHwnd()) {
        m_wndStatusBar.SendMessage(WM_SIZE, nType, MAKELPARAM(cx, cy));
    }
}"""

new_onsize = """void CSearchDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);
    if (m_wndStatusBar.GetSafeHwnd()) {
        RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
    }
}"""

text = text.replace(old_onsize, new_onsize)

with io.open(path, 'w', encoding='utf-8') as f:
    f.write(text)

print("Status bar layout issues fixed.")
