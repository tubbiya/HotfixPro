import io
import re

path_cpp = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
path_h = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.h'

# 1. Update SearchDlg.h
with io.open(path_h, 'r', encoding='utf-8') as f:
    text_h = f.read()

if "afx_msg void OnEnChangeEdtEn();" not in text_h:
    text_h = text_h.replace("afx_msg void OnBnClickedBtnAra11();", 
                            "afx_msg void OnEnChangeEdtEn();\n    afx_msg void OnBnClickedBtnAra11();")
    with io.open(path_h, 'w', encoding='utf-8') as f:
        f.write(text_h)

# 2. Update SearchDlg.cpp
with io.open(path_cpp, 'r', encoding='utf-8') as f:
    text_cpp = f.read()

if "ON_EN_CHANGE(IDC_EDT_EN, &CSearchDlg::OnEnChangeEdtEn)" not in text_cpp:
    text_cpp = text_cpp.replace("ON_BN_CLICKED(IDC_BTN_ARA11, &CSearchDlg::OnBnClickedBtnAra11)",
                                "ON_BN_CLICKED(IDC_BTN_ARA11, &CSearchDlg::OnBnClickedBtnAra11)\n    ON_EN_CHANGE(IDC_EDT_EN, &CSearchDlg::OnEnChangeEdtEn)")

if "void CSearchDlg::OnEnChangeEdtEn()" not in text_cpp:
    en_change_impl = """void CSearchDlg::OnEnChangeEdtEn()
{
    CString strEn;
    GetDlgItemText(IDC_EDT_EN, strEn);
    SetDlgItemText(IDC_EDT_BOY, strEn);
}
"""
    text_cpp += "\n" + en_change_impl

# 3. Update ResizeShapes to use FindShapes("@type <> 'group'")
old_resize_loop = """    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pSel) {
        long count = GetLongProp(pSel, L"Count");
        if (count > 0) {
            bool bCircleOnly = (IsDlgButtonChecked(IDC_CHK_OLCU2) == BST_CHECKED);
            if (bCircleOnly) h_mm = w_mm;
            
            for (long i = count; i >= 1; --i) {
                IDispatch* pShape = GetDispatchPropWithIntArg(pSel, L"Item", i);"""

new_resize_loop = """    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
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
                    IDispatch* pShape = GetDispatchPropWithIntArg(pFlatRange, L"Item", i);"""

text_cpp = text_cpp.replace(old_resize_loop, new_resize_loop)

# close brace for pFlatRange
old_resize_end = """                            }
                            pShape->Release();
                        }
                    }
                }
            }
        }
        pSel->Release();"""

new_resize_end = """                            }
                            pShape->Release();
                        }
                    }
                }
            }
            pFlatRange->Release();
        }
        pSel->Release();"""
text_cpp = text_cpp.replace(old_resize_end, new_resize_end)

# 4. Update ExportEx to Export
text_cpp = text_cpp.replace('OLECHAR* szExport = (OLECHAR*)L"ExportEx"; // Use ExportEx for proper ExportFilter return', 'OLECHAR* szExport = (OLECHAR*)L"Export"; // Use Export for simplicity')

# 5. Update Isaret Tasi to find minimum object within bounds
old_isaret_loop = """            double minW = 9999.0;
            for(long i=1; i<=count; i++) {
                IDispatch* pShape = GetDispatchPropWithIntArg(pSel, L"Item", i);
                if(pShape) {
                    double w = GetDoubleProp(pShape, L"SizeWidth");
                    if (w < minW && w > 0.1) minW = w;
                    pShape->Release();
                }
            }"""

new_isaret_loop = """            double minW = 9999.0;
            IDispatch* pPage = GetDispatchProp(pDoc, L"ActivePage");
            if (pPage) {
                DISPID dispSelect; OLECHAR* szSelect = (OLECHAR*)L"SelectShapesFromRectangle";
                if (SUCCEEDED(pPage->GetIDsOfNames(IID_NULL, &szSelect, 1, LOCALE_USER_DEFAULT, &dispSelect))) {
                    VARIANT args[5]; for(int i=0; i<5; ++i) VariantInit(&args[i]);
                    args[4].vt = VT_R8; args[4].dblVal = left - 5.0; // Left
                    args[3].vt = VT_R8; args[3].dblVal = top + 5.0; // Top
                    args[2].vt = VT_R8; args[2].dblVal = right + 5.0; // Right
                    args[1].vt = VT_R8; args[1].dblVal = bottom - 5.0; // Bottom
                    args[0].vt = VT_BOOL; args[0].boolVal = VARIANT_TRUE; // Intersect
                    DISPPARAMS params = { args, NULL, 5, 0 };
                    VARIANT retRange; VariantInit(&retRange);
                    if (SUCCEEDED(pPage->Invoke(dispSelect, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retRange, NULL, NULL))) {
                        if (retRange.vt == VT_DISPATCH && retRange.pdispVal) {
                            IDispatch* pInnerRange = retRange.pdispVal;
                            IDispatch* pShapesColl = GetDispatchProp(pInnerRange, L"Shapes");
                            if (pShapesColl) {
                                DISPID dispFind; OLECHAR* szFind = (OLECHAR*)L"FindShapes";
                                if (SUCCEEDED(pShapesColl->GetIDsOfNames(IID_NULL, &szFind, 1, LOCALE_USER_DEFAULT, &dispFind))) {
                                    VARIANT qArg; VariantInit(&qArg);
                                    qArg.vt = VT_BSTR; qArg.bstrVal = SysAllocString(L"@type <> 'group'");
                                    DISPPARAMS qParams = { &qArg, NULL, 1, 0 };
                                    VARIANT qRet; VariantInit(&qRet);
                                    if (SUCCEEDED(pShapesColl->Invoke(dispFind, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &qParams, &qRet, NULL, NULL))) {
                                        if (qRet.vt == VT_DISPATCH && qRet.pdispVal) {
                                            IDispatch* pFlatInner = qRet.pdispVal;
                                            long innerCount = GetLongProp(pFlatInner, L"Count");
                                            for (long i = 1; i <= innerCount; i++) {
                                                IDispatch* pInnerShape = GetDispatchPropWithIntArg(pFlatInner, L"Item", i);
                                                if (pInnerShape) {
                                                    double w = GetDoubleProp(pInnerShape, L"SizeWidth");
                                                    if (w < minW && w > 0.1 && w < 15.0) minW = w;
                                                    pInnerShape->Release();
                                                }
                                            }
                                            pFlatInner->Release();
                                        }
                                    }
                                    SysFreeString(qArg.bstrVal);
                                }
                                pShapesColl->Release();
                            }
                            pInnerRange->Release();
                        }
                    }
                }
                pPage->Release();
            }"""
text_cpp = text_cpp.replace(old_isaret_loop, new_isaret_loop)


with io.open(path_cpp, 'w', encoding='utf-8') as f:
    f.write(text_cpp)

print("Patch applied to SearchDlg.cpp and SearchDlg.h successfully")
