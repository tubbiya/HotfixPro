import io

path_cpp = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'

with io.open(path_cpp, 'r', encoding='utf-8') as f:
    text_cpp = f.read()

# Find the start of ResizeShapes
start_idx = text_cpp.find("void CSearchDlg::ResizeShapes(double w_mm, double h_mm)")
if start_idx != -1:
    # Find the end of ResizeShapes (next function is OnEnChangeEdtEn)
    end_idx = text_cpp.find("void CSearchDlg::OnEnChangeEdtEn()", start_idx)
    
    clean_resize_shapes = """void CSearchDlg::ResizeShapes(double w_mm, double h_mm)
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
                    IDispatch* pShape = GetDispatchPropWithIntArg(pFlatRange, L"Item", i);
                    if (pShape) {
                        if (bCircleOnly) {
                            double cx = GetDoubleProp(pShape, L"CenterX");
                            double cy = GetDoubleProp(pShape, L"CenterY");
                            IDispatch* pLayer = GetDispatchProp(pShape, L"Layer");
                            if (pLayer) {
                                DISPID dispidCreate;
                                OLECHAR* szCreate = (OLECHAR*)L"CreateEllipse2";
                                if (SUCCEEDED(pLayer->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dispidCreate))) {
                                    VARIANT args[3]; VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]);
                                    args[2].vt = VT_R8; args[2].dblVal = cx;
                                    args[1].vt = VT_R8; args[1].dblVal = cy;
                                    args[0].vt = VT_R8; args[0].dblVal = w_mm / 2.0;
                                    DISPPARAMS params2 = { args, NULL, 3, 0 };
                                    VARIANT retVal2; VariantInit(&retVal2);
                                    if (SUCCEEDED(pLayer->Invoke(dispidCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params2, &retVal2, NULL, NULL))) {
                                        if (retVal2.vt == VT_DISPATCH && retVal2.pdispVal) {
                                            DISPID dispidDelete;
                                            OLECHAR* szDelete = (OLECHAR*)L"Delete";
                                            if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szDelete, 1, LOCALE_USER_DEFAULT, &dispidDelete))) {
                                                DISPPARAMS delParams = { NULL, NULL, 0, 0 };
                                                pShape->Invoke(dispidDelete, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &delParams, NULL, NULL, NULL);
                                            }
                                            retVal2.pdispVal->Release();
                                        }
                                    }
                                }
                                pLayer->Release();
                            }
                        } else {
                            DISPID dispid;
                            OLECHAR* szSetSize = (OLECHAR*)L"SetSize";
                            if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szSetSize, 1, LOCALE_USER_DEFAULT, &dispid))) {
                                VARIANT args[2]; VariantInit(&args[0]); VariantInit(&args[1]);
                                args[1].vt = VT_R8; args[1].dblVal = w_mm;
                                args[0].vt = VT_R8; args[0].dblVal = h_mm;
                                DISPPARAMS params3 = { args, NULL, 2, 0 };
                                pShape->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params3, NULL, NULL, NULL);
                            }
                        }
                        pShape->Release();
                    }
                }
            }
            pFlatRange->Release();
        }
        pSel->Release();
    }
    pDoc->Release();
}

"""
    text_cpp = text_cpp[:start_idx] + clean_resize_shapes + text_cpp[end_idx:]
    with io.open(path_cpp, 'w', encoding='utf-8') as f:
        f.write(text_cpp)
    print("Fixed braces")
