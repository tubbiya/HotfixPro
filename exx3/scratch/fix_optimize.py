# -*- coding: utf-8 -*-
import io

cpp_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(cpp_path, 'r', encoding='utf-8') as f:
    cpp_content = f.read()

helpers = """
static bool SetBoolProp(IDispatch* pDisp, WCHAR* szProp, bool bValue)
{
    if (!pDisp) return false;
    DISPID dispid;
    if (FAILED(pDisp->GetIDsOfNames(IID_NULL, &szProp, 1, LOCALE_USER_DEFAULT, &dispid))) return false;
    VARIANT arg;
    VariantInit(&arg);
    arg.vt = VT_BOOL;
    arg.boolVal = bValue ? VARIANT_TRUE : VARIANT_FALSE;
    DISPID mydispid = DISPID_PROPERTYPUT;
    DISPPARAMS params = { &arg, &mydispid, 1, 1 };
    HRESULT hr = pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &params, nullptr, nullptr, nullptr);
    return SUCCEEDED(hr);
}

static void OptimizeCorel(IDispatch* pApp, BOOL bUse, const WCHAR* szName)
{
    if (!pApp) return;
    
    IDispatch* pDoc = GetDispatchProp(pApp, L"ActiveDocument");
    if (!pDoc) return;
    
    if (bUse) {
        VARIANT arg;
        VariantInit(&arg);
        arg.vt = VT_BSTR;
        arg.bstrVal = SysAllocString(szName);
        InvokeMethod(pDoc, L"BeginCommandGroup", &arg, 1);
        SysFreeString(arg.bstrVal);
        
        SetBoolProp(pApp, L"Optimization", true);
        SetBoolProp(pApp, L"EventsEnabled", false);
    } else {
        SetBoolProp(pApp, L"Optimization", false);
        SetBoolProp(pApp, L"EventsEnabled", true);
        
        InvokeMethod(pDoc, L"EndCommandGroup", nullptr, 0);
        
        IDispatch* pWin = GetDispatchProp(pApp, L"ActiveWindow");
        if (pWin) {
            InvokeMethod(pWin, L"Refresh", nullptr, 0);
            pWin->Release();
        }
    }
    pDoc->Release();
}

class CCorelOptimizer {
    IDispatch* m_pApp;
public:
    CCorelOptimizer(IDispatch* pApp, const WCHAR* szName) : m_pApp(pApp) {
        OptimizeCorel(m_pApp, TRUE, szName);
    }
    ~CCorelOptimizer() {
        OptimizeCorel(m_pApp, FALSE, L"");
    }
};
"""

target_insertion = "static bool SetLongProp(IDispatch* pDisp, WCHAR* szProp, long value)"
if helpers not in cpp_content:
    cpp_content = cpp_content.replace(target_insertion, helpers + "\n" + target_insertion)

replacements = [
    ("void CSearchDlg::OnBnClickedBtnAra()\n{", 
     "void CSearchDlg::OnBnClickedBtnAra()\n{\n    CCorelOptimizer optimizer(m_pApp, L\"Ztrass Arama\");"),
     
    ("void CSearchDlg::OnBnClickedBtnSelectMatch()\n{", 
     "void CSearchDlg::OnBnClickedBtnSelectMatch()\n{\n    CCorelOptimizer optimizer(m_pApp, L\"Ztrass Benzerleri Sec\");"),
     
    ("void CSearchDlg::OnBnClickedBtnList()\n{", 
     "void CSearchDlg::OnBnClickedBtnList()\n{\n    CCorelOptimizer optimizer(m_pApp, L\"Ztrass Liste\");"),
     
    ("void CSearchDlg::OnBnClickedBtnKalipAyir()\n{", 
     "void CSearchDlg::OnBnClickedBtnKalipAyir()\n{\n    CCorelOptimizer optimizer(m_pApp, L\"Ztrass Kalip Ayir\");"),
     
    ("void CSearchDlg::OnBnClickedBtnMakineKalibi()\n{", 
     "void CSearchDlg::OnBnClickedBtnMakineKalibi()\n{\n    CCorelOptimizer optimizer(m_pApp, L\"Ztrass Makine Kalibi\");"),
     
    ("void CSearchDlg::OnBnClickedBtnBilgiEkle()\n{", 
     "void CSearchDlg::OnBnClickedBtnBilgiEkle()\n{\n    CCorelOptimizer optimizer(m_pApp, L\"Ztrass Bilgi Ekle\");"),
     
    ("void CSearchDlg::OnBnClickedBtnKalipEkle()\n{", 
     "void CSearchDlg::OnBnClickedBtnKalipEkle()\n{\n    CCorelOptimizer optimizer(m_pApp, L\"Ztrass Kalip Ekle\");"),
     
    ("void CSearchDlg::OnBnClickedBtnIsaretTasi()\n{", 
     "void CSearchDlg::OnBnClickedBtnIsaretTasi()\n{\n    CCorelOptimizer optimizer(m_pApp, L\"Ztrass Isaret Tasi\");")
]

for old, new in replacements:
    if new not in cpp_content:
        cpp_content = cpp_content.replace(old, new)

with io.open(cpp_path, 'w', encoding='utf-8') as f:
    f.write(cpp_content)

print("Optimization logic added!")
