"import io
import re

path_cpp = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path_cpp, 'r', encoding='utf-8') as f:
    text_cpp = f.read()

# 1. Update ResizeShapes to implement ProB.frm logic
old_resize_shapes = """void CSearchDlg::ResizeShapes(double w_mm, double h_mm)
{
    if (!m_pApp) return;
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Boyutlandir");
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    
    SetLongProp(pDoc, L"Unit", 3);
    SetLongProp(pDoc, L"ReferencePoint", 4);
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pSel) {
        IDispatch* pShapesColl = GetDispatchProp(pSel, L"Shapes");
        IDispatch* pFlatRange = nullptr;
        if (pShapesColl) {
            DISPID dispFind; OLECHAR* szFind = (OLECHAR*)L"FindShapes";
            if (SUCCEEDED(pShapesColl->GetIDsOfNames(IID_NULL, &szFind, 1, LOCALE_USER_DEFAULT, &dispFind))) {
                VARIANT arg; VariantInit(&arg);
                arg.vt = VT_BSTR; arg.bstrVal = SysAllocString(L"@type <> 'group'");
                DISPPARAMS params = { &arg, NULL, 1, 0 };
                VARIANT retVal; VariantInit(&retVal);
                if (SUCCEEDED(pShapesColl->Invoke(dispFind, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                    if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                        pFlatRange = retVal.pdispVal;
                    }
                }
                SysFreeString(arg.bstrVal);
            }
            pShapesColl->Release();
        }
        
        if (pFlatRange) {
            long count = GetLongProp(pFlatRange, L"Count");
            if (count > 0) {
                bool bCircleOnly = (IsDlgButtonChecked(IDC_CHK_OLCU2) == BST_CHECKED);
                if (bCircleOnly) h_mm = w_mm;
                
                for (long i = count; i >= 1; --i) {
                    
<truncated 17547 bytes>