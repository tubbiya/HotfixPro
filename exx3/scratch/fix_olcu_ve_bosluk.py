# -*- coding: utf-8 -*-
import io
import re

def process_file(path, search_str, replace_str):
    with io.open(path, 'r', encoding='Windows-1254') as f:
        text = f.read()
    if search_str in text:
        text = text.replace(search_str, replace_str)
        with io.open(path, 'w', encoding='Windows-1254') as f:
            f.write(text)

# 1. resource.h
process_file(r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\resource.h', 
             '#define IDC_EDT_TOPLAM_IS               1059', 
             '#define IDC_EDT_TOPLAM_IS               1059\n#define IDC_EDT_EN                      1090\n#define IDC_EDT_BOY                     1091')

# 2. Ztrass.rc
with io.open(r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\Ztrass.rc', 'r', encoding='Windows-1254') as f:
    text = f.read()
text = text.replace('EDITTEXT        0,26,66,30,12,ES_AUTOHSCROLL', 'EDITTEXT        IDC_EDT_EN,26,66,30,12,ES_AUTOHSCROLL')
text = text.replace('EDITTEXT        1,79,66,30,12,ES_AUTOHSCROLL', 'EDITTEXT        IDC_EDT_BOY,79,66,30,12,ES_AUTOHSCROLL')
text = text.replace('CONTROL         "?",IDC_CHK_OLCU2,"Button"', 'CONTROL         "O",IDC_CHK_OLCU2,"Button"')
with io.open(r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\Ztrass.rc', 'w', encoding='Windows-1254') as f:
    f.write(text)

# 3. SearchDlg.h
process_file(r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.h',
             'afx_msg void OnBnClickedBtnAra();',
             'afx_msg void OnBnClickedBtnAra();\n    afx_msg void OnBnClickedBtnAra2();\n    afx_msg void OnBnClickedAra3Range(UINT nID);\n    void ResizeShapes(double w_mm, double h_mm);')


# 4. SearchDlg.cpp
path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path, 'r', encoding='Windows-1254') as f:
    text = f.read()

old_height = """void CSearchDlg::SetDialogHeight(int heightDU)
{
    CRect rc;
    GetWindowRect(&rc);
    
    CRect rcHeight(0, 0, 0, heightDU);
    MapDialogRect(&rcHeight);
    
    SetWindowPos(nullptr, 0, 0, rc.Width(), rcHeight.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}"""

new_height = """void CSearchDlg::SetDialogHeight(int heightDU)
{
    CRect rc;
    GetWindowRect(&rc);
    
    CRect rcHeight(0, 0, 0, heightDU);
    MapDialogRect(&rcHeight);
    CalcWindowRect(&rcHeight);
    
    SetWindowPos(nullptr, 0, 0, rc.Width(), rcHeight.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}"""

old_width = """void CSearchDlg::SetDialogWidth(int widthDU)
{
    CRect rc;
    GetWindowRect(&rc);
    
    CRect rcWidth(0, 0, widthDU, 0);
    MapDialogRect(&rcWidth);
    
    SetWindowPos(nullptr, 0, 0, rcWidth.right, rc.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}"""

new_width = """void CSearchDlg::SetDialogWidth(int widthDU)
{
    CRect rc;
    GetWindowRect(&rc);
    
    CRect rcWidth(0, 0, widthDU, 0);
    MapDialogRect(&rcWidth);
    CalcWindowRect(&rcWidth);
    
    SetWindowPos(nullptr, 0, 0, rcWidth.Width(), rc.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}"""

text = text.replace(old_height, new_height)
text = text.replace(old_width, new_width)

if 'ON_BN_CLICKED(IDC_BTN_ARA2' not in text:
    text = text.replace('ON_BN_CLICKED(IDC_BTN_ARA, &CSearchDlg::OnBnClickedBtnAra)', 'ON_BN_CLICKED(IDC_BTN_ARA, &CSearchDlg::OnBnClickedBtnAra)\n    ON_BN_CLICKED(IDC_BTN_ARA2, &CSearchDlg::OnBnClickedBtnAra2)\n    ON_COMMAND_RANGE(IDC_BTN_ARA3_1, IDC_BTN_ARA3_8, &CSearchDlg::OnBnClickedAra3Range)')

funcs = """
void CSearchDlg::OnBnClickedBtnAra2()
{
    CString strEn, strBoy;
    GetDlgItemText(IDC_EDT_EN, strEn);
    GetDlgItemText(IDC_EDT_BOY, strBoy);
    double w = _wtof(strEn);
    double h = _wtof(strBoy);
    if (w <= 0 || h <= 0) {
        AfxMessageBox(L"L\\xFCtfen ge\\xE7erli bir \\xF6l\\xE7\\xFC girin.");
        return;
    }
    ResizeShapes(w, h);
}

void CSearchDlg::OnBnClickedAra3Range(UINT nID)
{
    double sizes[] = {2.0, 2.1, 2.15, 3.0, 3.1, 3.15, 0.4, 0.6};
    int idx = nID - IDC_BTN_ARA3_1;
    if (idx >= 0 && idx < 8) {
        double s = sizes[idx];
        CString str;
        str.Format(L"%.2f", s);
        str.TrimRight(L"0");
        str.TrimRight(L".");
        SetDlgItemText(IDC_EDT_EN, str);
        SetDlgItemText(IDC_EDT_BOY, str);
        ResizeShapes(s, s);
    }
}

void CSearchDlg::ResizeShapes(double w_mm, double h_mm)
{
    if (!m_pApp) return;
    
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Boyutlandir");
    
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    
    SetLongProp(pDoc, L"Unit", 3); // cdrMillimeter
    SetLongProp(pDoc, L"ReferencePoint", 4); // cdrCenter
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pSel) {
        long count = GetLongProp(pSel, L"Count");
        if (count > 0) {
            bool bCircleOnly = (IsDlgButtonChecked(IDC_CHK_OLCU2) == BST_CHECKED);
            if (bCircleOnly) h_mm = w_mm;
            
            for (long i = 1; i <= count; ++i) {
                IDispatch* pShape = GetDispatchPropWithIntArg(pSel, L"Item", i);
                if (pShape) {
                    DISPID dispid;
                    OLECHAR* szSetSize = (OLECHAR*)L"SetSize";
                    if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szSetSize, 1, LOCALE_USER_DEFAULT, &dispid))) {
                        VARIANT args[2];
                        VariantInit(&args[0]); VariantInit(&args[1]);
                        args[1].vt = VT_R8; args[1].dblVal = w_mm;
                        args[0].vt = VT_R8; args[0].dblVal = h_mm;
                        
                        DISPPARAMS params = { args, NULL, 2, 0 };
                        pShape->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, NULL, NULL, NULL);
                    }
                    pShape->Release();
                }
            }
        }
        pSel->Release();
    }
    pDoc->Release();
}
"""
if 'CSearchDlg::OnBnClickedBtnAra2()' not in text:
    text += funcs

with io.open(path, 'w', encoding='Windows-1254') as f:
    f.write(text)

print("All fixes applied successfully.")
