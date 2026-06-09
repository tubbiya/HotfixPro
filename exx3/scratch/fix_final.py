# -*- coding: utf-8 -*-
import io

cpp_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(cpp_path, 'r', encoding='utf-8') as f:
    cpp_content = f.read()

# 1. Fix SetCompactMode (CalcWindowRect and height)
old_compact = """    if (bCompact) {
        m_vecVisibleControls.clear();
        
        CWnd* pWnd = GetWindow(GW_CHILD);
        while (pWnd) {
            if (pWnd->IsWindowVisible() && pWnd->GetDlgCtrlID() != IDC_BTN_simge) {
                m_vecVisibleControls.push_back(pWnd->GetSafeHwnd());
                pWnd->ShowWindow(SW_HIDE);
            }
            pWnd = pWnd->GetWindow(GW_HWNDNEXT);
        }
        
        CRect rc(0, 0, 195, 30);
        MapDialogRect(&rc);
        SetWindowPos(nullptr, 0, 0, rc.Width(), rc.Height(), SWP_NOMOVE | SWP_NOZORDER);
        
        m_wndAdBox.ShowWindow(SW_SHOW);
    }"""

new_compact = """    if (bCompact) {
        m_vecVisibleControls.clear();
        
        CWnd* pWnd = GetWindow(GW_CHILD);
        while (pWnd) {
            if (pWnd->IsWindowVisible() && pWnd->GetDlgCtrlID() != IDC_BTN_simge) {
                m_vecVisibleControls.push_back(pWnd->GetSafeHwnd());
                pWnd->ShowWindow(SW_HIDE);
            }
            pWnd = pWnd->GetWindow(GW_HWNDNEXT);
        }
        
        CRect rc(0, 0, 210, 40);
        MapDialogRect(&rc);
        CalcWindowRect(&rc); // Adjust for title bar and borders!
        SetWindowPos(nullptr, 0, 0, rc.Width(), rc.Height(), SWP_NOMOVE | SWP_NOZORDER);
        
        m_wndAdBox.ShowWindow(SW_SHOW);
    }"""

cpp_content = cpp_content.replace(old_compact, new_compact)

# 2. Fix ShowKalipPanel (Hide static labels)
old_show_static = """    CWnd* pWnd = GetWindow(GW_CHILD);
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

new_show_static = """    CWnd* pWnd = GetWindow(GW_CHILD);
    while (pWnd) {
        TCHAR szClass[32];
        ::GetClassName(pWnd->GetSafeHwnd(), szClass, 32);
        if (_wcsicmp(szClass, L"Static") == 0) {
            CString strText;
            pWnd->GetWindowText(strText);
            
            if (strText == _T("Firma") || strText == _T("Dosya") || strText == _T("Kalıp") ||
                strText == _T("Pres") || strText == _T("Montaj") || strText == _T("K. İş") || strText == _T("Toplam") ||
                strText == _T("Kalip Adi")) 
            {
                pWnd->ShowWindow(bShow ? (m_bAyarOpen ? SW_HIDE : SW_SHOW) : SW_HIDE);
            }
            else if (strText.Find(_T("mm")) != -1 && strText.Find(_T("Firma")) == -1 && strText.Find(_T("Dosya")) == -1) {
                pWnd->ShowWindow(bShow ? (m_bAyarOpen ? SW_SHOW : SW_HIDE) : SW_HIDE);
            }
        }
        pWnd = pWnd->GetWindow(GW_HWNDNEXT);
    }"""

cpp_content = cpp_content.replace(old_show_static, new_show_static)

# Replace "Kalip Adi" if missing in resources
cpp_content = cpp_content.replace('_T("Kalip Adi")', '_T("Kalıp Adı")')


with io.open(cpp_path, 'w', encoding='utf-8') as f:
    f.write(cpp_content)

print("Final fixes applied!")
