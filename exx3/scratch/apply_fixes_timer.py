import io

# 1. Update resource.h
path_res = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\resource.h'
with io.open(path_res, 'r', encoding='mbcs', errors='ignore') as f:
    text_res = f.read()

for i in range(1, 9):
    text_res = text_res.replace(f"IDC_BTN_ARA3_{i}", f"IDC_BTN_OLCU_{i}")

with io.open(path_res, 'w', encoding='mbcs') as f:
    f.write(text_res)


# 2. Update Ztrass.rc
path_rc = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\Ztrass.rc'
with io.open(path_rc, 'r', encoding='mbcs', errors='ignore') as f:
    text_rc = f.read()

for i in range(1, 9):
    text_rc = text_rc.replace(f"IDC_BTN_ARA3_{i}", f"IDC_BTN_OLCU_{i}")

text_rc = text_rc.replace("WS_MAXIMIZEBOX | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME", "WS_POPUP | WS_CAPTION | WS_SYSMENU")

with io.open(path_rc, 'w', encoding='mbcs') as f:
    f.write(text_rc)


# 3. Update SearchDlg.h
path_h = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.h'
with io.open(path_h, 'r', encoding='utf-8', errors='ignore') as f:
    text_h = f.read()

if "void OnTimer" not in text_h:
    text_h = text_h.replace("afx_msg void OnSize(UINT nType, int cx, int cy);", "afx_msg void OnSize(UINT nType, int cx, int cy);\n    afx_msg void OnTimer(UINT_PTR nIDEvent);")

if "double m_lastSelW;" not in text_h:
    text_h = text_h.replace("BOOL m_bTop;", "BOOL m_bTop;\n    long m_lastSelCount;\n    double m_lastSelW;\n    double m_lastSelH;")

with io.open(path_h, 'w', encoding='utf-8') as f:
    f.write(text_h)


# 4. Update SearchDlg.cpp
path_cpp = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path_cpp, 'r', encoding='utf-8', errors='ignore') as f:
    text_cpp = f.read()

for i in range(1, 9):
    text_cpp = text_cpp.replace(f"IDC_BTN_ARA3_{i}", f"IDC_BTN_OLCU_{i}")

if "ON_WM_TIMER()" not in text_cpp:
    text_cpp = text_cpp.replace("ON_WM_SIZE()", "ON_WM_SIZE()\n    ON_WM_TIMER()")

text_cpp = text_cpp.replace("m_wndStatusBar.Create(WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP", "m_wndStatusBar.Create(WS_CHILD | WS_VISIBLE")

if "m_lastSelCount = 0;" not in text_cpp:
    text_cpp = text_cpp.replace("m_bTop = FALSE;", "m_bTop = FALSE;\n    m_lastSelCount = 0;\n    m_lastSelW = 0;\n    m_lastSelH = 0;")

if "SetTimer(1, 250, nullptr);" not in text_cpp:
    text_cpp = text_cpp.replace("SetDialogHeight(198);", "SetDialogHeight(198);\n    SetTimer(1, 250, nullptr);", 1)

text_cpp = text_cpp.replace("int grupIcYukseklik = gridRows * rowH + (m_bListOption ? 16 : 14);", "int grupIcYukseklik = gridRows * rowH + (m_bListOption ? 10 : 6);")

if "void CSearchDlg::OnTimer" not in text_cpp:
    ontimer_impl = """void CSearchDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 1 && m_pApp) {
        IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
        if (pSel) {
            long count = GetLongProp(pSel, L"Count");
            if (count > 0) {
                double w = GetDoubleProp(pSel, L"SizeWidth");
                double h = GetDoubleProp(pSel, L"SizeHeight");
                
                if (abs(w - m_lastSelW) > 0.001 || abs(h - m_lastSelH) > 0.001 || count != m_lastSelCount) {
                    m_lastSelW = w;
                    m_lastSelH = h;
                    m_lastSelCount = count;
                    
                    CString strEn, strBoy;
                    strEn.Format(L"%.2f", w);
                    strBoy.Format(L"%.2f", h);
                    strEn.Replace(L".", L",");
                    strBoy.Replace(L".", L",");
                    
                    SetDlgItemText(IDC_EDT_EN, strEn);
                    SetDlgItemText(IDC_EDT_BOY, strBoy);
                }
            } else {
                m_lastSelCount = 0;
            }
            pSel->Release();
        }
    }
    CDialog::OnTimer(nIDEvent);
}

"""
    text_cpp = text_cpp.replace("void CSearchDlg::OnSize", ontimer_impl + "void CSearchDlg::OnSize")

with io.open(path_cpp, 'w', encoding='utf-8') as f:
    f.write(text_cpp)

print("Applied fixes with proper encoding handling.")
