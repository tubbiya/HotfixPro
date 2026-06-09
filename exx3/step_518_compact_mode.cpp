Created At: 2026-06-03T07:47:37Z
Completed At: 2026-06-03T07:47:37Z
File Path: `file:///c:/Users/tubbi/Dropbox/Claude/HotfixPro/SearchDlg.cpp`
Total Lines: 3364
Total Bytes: 152023
Showing lines 2940 to 2990
The following code has been modified to include a line number before every line, in the format: <line_number>: <original_line>. Please note that any changes targeting the original code should remove the line number, colon, and leading space.

void CSearchDlg::SetStatus(LPCTSTR lpszText)
{
    if (m_wndStatusBar.GetSafeHwnd()) {
        m_wndStatusBar.SetText(lpszText, 0, 0);
    }
}

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
                if (id != IDC_BTN_simge && id != 3000 && id != 10001) {
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
        if (m_bListOpen) {
            UpdateListLayout();
        } else {
            SetDialogHeight(198);
        }
        GetDlgItem(IDC_BTN_simge)->SetWindowText(_T("-"));
    }
    Invalidate();
}

void CSearchDlg::OnBnClickedBtnAra2()
The above content does NOT show the entire file contents. If you need to view any lines of the file which were not shown to complete your task, call this tool again to view those lines.
