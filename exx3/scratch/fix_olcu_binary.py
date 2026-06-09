import os

def replace_in_file(path, search_bytes, replace_bytes):
    with open(path, 'rb') as f:
        data = f.read()
    if search_bytes in data:
        data = data.replace(search_bytes, replace_bytes)
        with open(path, 'wb') as f:
            f.write(data)

# resource.h
replace_in_file(
    r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\resource.h',
    b'#define IDC_EDT_TOPLAM_IS               1059',
    b'#define IDC_EDT_TOPLAM_IS               1059\r\n#define IDC_EDT_EN                      1090\r\n#define IDC_EDT_BOY                     1091'
)

# Ztrass.rc
replace_in_file(
    r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\Ztrass.rc',
    b'EDITTEXT        0,26,66,30,12,ES_AUTOHSCROLL',
    b'EDITTEXT        IDC_EDT_EN,26,66,30,12,ES_AUTOHSCROLL'
)
replace_in_file(
    r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\Ztrass.rc',
    b'EDITTEXT        1,79,66,30,12,ES_AUTOHSCROLL',
    b'EDITTEXT        IDC_EDT_BOY,79,66,30,12,ES_AUTOHSCROLL'
)
replace_in_file(
    r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\Ztrass.rc',
    b'CONTROL         "?",IDC_CHK_OLCU2,"Button"',
    b'CONTROL         "O",IDC_CHK_OLCU2,"Button"'
)

# SearchDlg.h
replace_in_file(
    r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.h',
    b'afx_msg void OnBnClickedBtnAra();',
    b'afx_msg void OnBnClickedBtnAra();\r\n    afx_msg void OnBnClickedBtnAra2();\r\n    afx_msg void OnBnClickedAra3Range(UINT nID);\r\n    void ResizeShapes(double w_mm, double h_mm);'
)

# SearchDlg.cpp
old_height = b"""void CSearchDlg::SetDialogHeight(int heightDU)\r
{\r
    CRect rc;\r
    GetWindowRect(&rc);\r
    \r
    CRect rcHeight(0, 0, 0, heightDU);\r
    MapDialogRect(&rcHeight);\r
    \r
    SetWindowPos(nullptr, 0, 0, rc.Width(), rcHeight.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);\r
}"""

new_height = b"""void CSearchDlg::SetDialogHeight(int heightDU)\r
{\r
    CRect rc;\r
    GetWindowRect(&rc);\r
    \r
    CRect rcHeight(0, 0, 0, heightDU);\r
    MapDialogRect(&rcHeight);\r
    CalcWindowRect(&rcHeight);\r
    \r
    SetWindowPos(nullptr, 0, 0, rc.Width(), rcHeight.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);\r
}"""

old_width = b"""void CSearchDlg::SetDialogWidth(int widthDU)\r
{\r
    CRect rc;\r
    GetWindowRect(&rc);\r
    \r
    CRect rcWidth(0, 0, widthDU, 0);\r
    MapDialogRect(&rcWidth);\r
    \r
    SetWindowPos(nullptr, 0, 0, rcWidth.right, rc.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);\r
}"""

new_width = b"""void CSearchDlg::SetDialogWidth(int widthDU)\r
{\r
    CRect rc;\r
    GetWindowRect(&rc);\r
    \r
    CRect rcWidth(0, 0, widthDU, 0);\r
    MapDialogRect(&rcWidth);\r
    CalcWindowRect(&rcWidth);\r
    \r
    SetWindowPos(nullptr, 0, 0, rcWidth.Width(), rc.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);\r
}"""

replace_in_file(r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp', old_height, new_height)
replace_in_file(r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp', old_width, new_width)

replace_in_file(
    r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp',
    b'ON_BN_CLICKED(IDC_BTN_ARA, &CSearchDlg::OnBnClickedBtnAra)',
    b'ON_BN_CLICKED(IDC_BTN_ARA, &CSearchDlg::OnBnClickedBtnAra)\r\n    ON_BN_CLICKED(IDC_BTN_ARA2, &CSearchDlg::OnBnClickedBtnAra2)\r\n    ON_COMMAND_RANGE(IDC_BTN_ARA3_1, IDC_BTN_ARA3_8, &CSearchDlg::OnBnClickedAra3Range)'
)

funcs = b"""\r
void CSearchDlg::OnBnClickedBtnAra2()\r
{\r
    CString strEn, strBoy;\r
    GetDlgItemText(IDC_EDT_EN, strEn);\r
    GetDlgItemText(IDC_EDT_BOY, strBoy);\r
    double w = _wtof(strEn);\r
    double h = _wtof(strBoy);\r
    if (w <= 0 || h <= 0) {\r
        return;\r
    }\r
    ResizeShapes(w, h);\r
}\r
\r
void CSearchDlg::OnBnClickedAra3Range(UINT nID)\r
{\r
    double sizes[] = {2.0, 2.1, 2.15, 3.0, 3.1, 3.15, 0.4, 0.6};\r
    int idx = nID - IDC_BTN_ARA3_1;\r
    if (idx >= 0 && idx < 8) {\r
        double s = sizes[idx];\r
        CString str;\r
        str.Format(L"%.2f", s);\r
        str.TrimRight(L"0");\r
        str.TrimRight(L".");\r
        SetDlgItemText(IDC_EDT_EN, str);\r
        SetDlgItemText(IDC_EDT_BOY, str);\r
        ResizeShapes(s, s);\r
    }\r
}\r
\r
void CSearchDlg::ResizeShapes(double w_mm, double h_mm)\r
{\r
    if (!m_pApp) return;\r
    \r
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Boyutlandir");\r
    \r
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");\r
    if (!pDoc) return;\r
    \r
    SetLongProp(pDoc, L"Unit", 3);\r
    SetLongProp(pDoc, L"ReferencePoint", 4);\r
    \r
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");\r
    if (pSel) {\r
        long count = GetLongProp(pSel, L"Count");\r
        if (count > 0) {\r
            bool bCircleOnly = (IsDlgButtonChecked(IDC_CHK_OLCU2) == BST_CHECKED);\r
            if (bCircleOnly) h_mm = w_mm;\r
            \r
            for (long i = 1; i <= count; ++i) {\r
                IDispatch* pShape = GetDispatchPropWithIntArg(pSel, L"Item", i);\r
                if (pShape) {\r
                    DISPID dispid;\r
                    OLECHAR* szSetSize = (OLECHAR*)L"SetSize";\r
                    if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szSetSize, 1, LOCALE_USER_DEFAULT, &dispid))) {\r
                        VARIANT args[2];\r
                        VariantInit(&args[0]); VariantInit(&args[1]);\r
                        args[1].vt = VT_R8; args[1].dblVal = w_mm;\r
                        args[0].vt = VT_R8; args[0].dblVal = h_mm;\r
                        \r
                        DISPPARAMS params = { args, NULL, 2, 0 };\r
                        pShape->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, NULL, NULL, NULL);\r
                    }\r
                    pShape->Release();\r
                }\r
            }\r
        }\r
        pSel->Release();\r
    }\r
    pDoc->Release();\r
}\r
"""

with open(r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp', 'rb') as f:
    data = f.read()
if b'CSearchDlg::OnBnClickedBtnAra2()' not in data:
    with open(r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp', 'ab') as f:
        f.write(funcs)
print("Fixes applied successfully in binary mode.")
