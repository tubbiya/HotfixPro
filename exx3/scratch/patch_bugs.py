import io
import re

# 1. Update Ztrass.rc
path_rc = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\Ztrass.rc'
with io.open(path_rc, 'r', encoding='utf-8', errors='ignore') as f:
    text_rc = f.read()

text_rc = text_rc.replace('"Üç Renk"', '"İç Renk"')
text_rc = text_rc.replace('"Grup Üçi"', '"Grup İçi"')

with io.open(path_rc, 'w', encoding='utf-8') as f:
    f.write(text_rc)

# 2. Update SearchDlg.cpp
path_cpp = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path_cpp, 'r', encoding='utf-8', errors='ignore') as f:
    text_cpp = f.read()

# Fix Selection logic in OnBnClickedBtnAra
old_sel = """    if (pResultRange) {
        LogDebug(L"[BILGI] CreateSelection cagriliyor...");
        InvokeMethodNoArgs(pResultRange, L"CreateSelection");
    }"""

new_sel = """    if (pResultRange) {
        LogDebug(L"[BILGI] CreateSelection cagriliyor...");
        if (pSelection) {
            VARIANT arg; VariantInit(&arg);
            arg.vt = VT_DISPATCH; arg.pdispVal = pSelection;
            InvokeMethod(pResultRange, L"AddRange", &arg, 1);
        } else if (pActiveShape) {
            VARIANT arg; VariantInit(&arg);
            arg.vt = VT_DISPATCH; arg.pdispVal = pActiveShape;
            InvokeMethod(pResultRange, L"Add", &arg, 1);
        }
        InvokeMethodNoArgs(pResultRange, L"CreateSelection");
    }"""
text_cpp = text_cpp.replace(old_sel, new_sel)

# Fix Export logic
old_exp = """        DISPID dispidExport;
        OLECHAR* szExport = (OLECHAR*)L"Export";
        if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szExport, 1, LOCALE_USER_DEFAULT, &dispidExport))) {
            VARIANT args[3];
            VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]);
            
            args[2].vt = VT_BSTR; args[2].bstrVal = SysAllocString(path);
            args[1].vt = VT_I4; args[1].lVal = filterIdx;
            args[0].vt = VT_I4; args[0].lVal = 1; // cdrSelection
            
            DISPPARAMS params = { args, NULL, 3, 0 };
            pDoc->Invoke(dispidExport, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, NULL, NULL, NULL);
            SysFreeString(args[2].bstrVal);
        }
        
        if (IsDlgButtonChecked(IDC_CHK_OLCU3) == BST_CHECKED) {
            CString pathJpg = path.Left(path.ReverseFind(L'.')) + L".jpg";
            if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szExport, 1, LOCALE_USER_DEFAULT, &dispidExport))) {
                VARIANT args[3]; VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]);
                args[2].vt = VT_BSTR; args[2].bstrVal = SysAllocString(pathJpg);
                args[1].vt = VT_I4; args[1].lVal = 774; // cdrJPEG
                args[0].vt = VT_I4; args[0].lVal = 1;
                DISPPARAMS params = { args, NULL, 3, 0 };
                pDoc->Invoke(dispidExport, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, NULL, NULL, NULL);
                SysFreeString(args[2].bstrVal);
            }
        }"""

new_exp = """        DISPID dispidExport;
        OLECHAR* szExport = (OLECHAR*)L"ExportEx"; // Use ExportEx for proper ExportFilter return
        if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szExport, 1, LOCALE_USER_DEFAULT, &dispidExport))) {
            VARIANT args[3];
            VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]);
            
            args[2].vt = VT_BSTR; args[2].bstrVal = SysAllocString(path);
            args[1].vt = VT_I4; args[1].lVal = filterIdx;
            args[0].vt = VT_I4; args[0].lVal = 1; // cdrSelection
            
            DISPPARAMS params = { args, NULL, 3, 0 };
            VARIANT retVal; VariantInit(&retVal);
            if (SUCCEEDED(pDoc->Invoke(dispidExport, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                    InvokeMethodNoArgs(retVal.pdispVal, L"Finish");
                    retVal.pdispVal->Release();
                }
            }
            SysFreeString(args[2].bstrVal);
        }
        
        if (IsDlgButtonChecked(IDC_CHK_OLCU3) == BST_CHECKED) {
            CString pathJpg = path.Left(path.ReverseFind(L'.')) + L".jpg";
            if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szExport, 1, LOCALE_USER_DEFAULT, &dispidExport))) {
                VARIANT args[3]; VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]);
                args[2].vt = VT_BSTR; args[2].bstrVal = SysAllocString(pathJpg);
                args[1].vt = VT_I4; args[1].lVal = 774; // cdrJPEG
                args[0].vt = VT_I4; args[0].lVal = 1;
                DISPPARAMS params = { args, NULL, 3, 0 };
                VARIANT retVal; VariantInit(&retVal);
                if (SUCCEEDED(pDoc->Invoke(dispidExport, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                    if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                        InvokeMethodNoArgs(retVal.pdispVal, L"Finish");
                        retVal.pdispVal->Release();
                    }
                }
                SysFreeString(args[2].bstrVal);
            }
        }"""
text_cpp = text_cpp.replace(old_exp, new_exp)

# Fix Isaret Tasi and Kalipla
old_kalip_ekle = """void CSearchDlg::OnBnClickedBtnKalipEkle()
{
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Kalip Ekle");
    CString strOlcu;
    GetDlgItemText(IDC_EDT_KALIP_OLCU, strOlcu);
    LogDebug(L"[BILGI] Kalip Ekle: olcu=%s", (const WCHAR*)strOlcu);
}

void CSearchDlg::OnBnClickedBtnIsaretTasi()
{
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Isaret Tasi");
    CString strOlcu;
    GetDlgItemText(IDC_EDT_TASI_OLCU, strOlcu);
    LogDebug(L"[BILGI] Isaret Tasi Ekle: olcu=%s", (const WCHAR*)strOlcu);
}"""

new_kalip_ekle = """void CSearchDlg::OnBnClickedBtnKalipEkle() // Isaret Tasi (KalipTasM)
{
    if (!m_pApp) return;
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Isaret Tasi");
    
    CString strTasMM;
    GetDlgItemText(IDC_EDT_KALIP_OLCU, strTasMM);
    strTasMM.Replace(L",", L".");
    double tasMM = _wtof(strTasMM);
    if (tasMM <= 0) tasMM = 2.0;
    
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    SetLongProp(pDoc, L"Unit", 3);
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pSel) {
        long count = GetLongProp(pSel, L"Count");
        if (count > 0) {
            double left = GetDoubleProp(pSel, L"LeftX");
            double right = GetDoubleProp(pSel, L"RightX");
            double bottom = GetDoubleProp(pSel, L"BottomY");
            double top = GetDoubleProp(pSel, L"TopY");
            
            double minW = 9999.0;
            for(long i=1; i<=count; i++) {
                IDispatch* pShape = GetDispatchPropWithIntArg(pSel, L"Item", i);
                if(pShape) {
                    double w = GetDoubleProp(pShape, L"SizeWidth");
                    if (w < minW && w > 0.1) minW = w;
                    pShape->Release();
                }
            }
            if (minW == 9999.0) minW = 2.0;
            double Sw = minW / 2.0;
            
            IDispatch* pLayer = GetDispatchProp(pDoc, L"ActiveLayer");
            if (pLayer) {
                DISPID dispidCreate;
                OLECHAR* szCreate = (OLECHAR*)L"CreateEllipse2";
                if (SUCCEEDED(pLayer->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dispidCreate))) {
                    auto CreateCircle = [&](double cx, double cy) {
                        VARIANT args[3]; VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]);
                        args[2].vt = VT_R8; args[2].dblVal = cx;
                        args[1].vt = VT_R8; args[1].dblVal = cy;
                        args[0].vt = VT_R8; args[0].dblVal = Sw;
                        DISPPARAMS params = { args, NULL, 3, 0 };
                        VARIANT retVal; VariantInit(&retVal);
                        if (SUCCEEDED(pLayer->Invoke(dispidCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                            if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                                IDispatch* pD = retVal.pdispVal;
                                SetStringProp(pD, L"Name", L"Kesim");
                                
                                IDispatch* pFill = GetDispatchProp(pD, L"Fill");
                                if (pFill) {
                                    InvokeMethodNoArgs(pFill, L"ApplyNoFill");
                                    pFill->Release();
                                }
                                
                                IDispatch* pOutline = GetDispatchProp(pD, L"Outline");
                                if (pOutline) {
                                    IDispatch* pColor = GetDispatchProp(pOutline, L"Color");
                                    if (pColor) {
                                        DISPID dispCmyk; OLECHAR* szCmyk = (OLECHAR*)L"CMYKAssign";
                                        if (SUCCEEDED(pColor->GetIDsOfNames(IID_NULL, &szCmyk, 1, LOCALE_USER_DEFAULT, &dispCmyk))) {
                                            VARIANT cargs[4];
                                            for(int i=0; i<4; i++) VariantInit(&cargs[i]);
                                            cargs[3].vt = VT_I4; cargs[3].lVal = 0; // C
                                            cargs[2].vt = VT_I4; cargs[2].lVal = 0; // M
                                            cargs[1].vt = VT_I4; cargs[1].lVal = 0; // Y
                                            cargs[0].vt = VT_I4; cargs[0].lVal = 100; // K
                                            DISPPARAMS cparams = { cargs, NULL, 4, 0 };
                                            pColor->Invoke(dispCmyk, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &cparams, NULL, NULL, NULL);
                                        }
                                        pColor->Release();
                                    }
                                    pOutline->Release();
                                }
                                pD->Release();
                            }
                        }
                    };
                    
                    CreateCircle(left + tasMM, bottom + tasMM);
                    CreateCircle(right - tasMM, bottom + tasMM);
                }
                pLayer->Release();
            }
        }
        pSel->Release();
    }
    pDoc->Release();
}

void CSearchDlg::OnBnClickedBtnIsaretTasi() // Kalipla (KalipKareM)
{
    if (!m_pApp) return;
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Kalipla");
    
    CString strPadding;
    GetDlgItemText(IDC_EDT_TASI_OLCU, strPadding);
    strPadding.Replace(L",", L".");
    double padding = _wtof(strPadding);
    if (padding <= 0) padding = 20.0;
    
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    SetLongProp(pDoc, L"Unit", 3);
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pSel) {
        long count = GetLongProp(pSel, L"Count");
        if (count > 0) {
            double left = GetDoubleProp(pSel, L"LeftX");
            double right = GetDoubleProp(pSel, L"RightX");
            double top = GetDoubleProp(pSel, L"TopY");
            double bottom = GetDoubleProp(pSel, L"BottomY");
            
            IDispatch* pLayer = GetDispatchProp(pDoc, L"ActiveLayer");
            if (pLayer) {
                DISPID dispidCreate;
                OLECHAR* szCreate = (OLECHAR*)L"CreateRectangle";
                if (SUCCEEDED(pLayer->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dispidCreate))) {
                    VARIANT args[4]; VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]); VariantInit(&args[3]);
                    args[3].vt = VT_R8; args[3].dblVal = left - padding;
                    args[2].vt = VT_R8; args[2].dblVal = bottom - padding;
                    args[1].vt = VT_R8; args[1].dblVal = right + padding;
                    args[0].vt = VT_R8; args[0].dblVal = top + padding;
                    DISPPARAMS params = { args, NULL, 4, 0 };
                    VARIANT retVal; VariantInit(&retVal);
                    if (SUCCEEDED(pLayer->Invoke(dispidCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                        if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                            IDispatch* pKare = retVal.pdispVal;
                            
                            IDispatch* pFill = GetDispatchProp(pKare, L"Fill");
                            if (pFill) {
                                InvokeMethodNoArgs(pFill, L"ApplyNoFill");
                                pFill->Release();
                            }
                            
                            IDispatch* pOutline = GetDispatchProp(pKare, L"Outline");
                            if (pOutline) {
                                IDispatch* pColor = GetDispatchProp(pOutline, L"Color");
                                if (pColor) {
                                    DISPID dispCmyk; OLECHAR* szCmyk = (OLECHAR*)L"CMYKAssign";
                                    if (SUCCEEDED(pColor->GetIDsOfNames(IID_NULL, &szCmyk, 1, LOCALE_USER_DEFAULT, &dispCmyk))) {
                                        VARIANT cargs[4];
                                        for(int i=0; i<4; i++) VariantInit(&cargs[i]);
                                        cargs[3].vt = VT_I4; cargs[3].lVal = 0; // C
                                        cargs[2].vt = VT_I4; cargs[2].lVal = 0; // M
                                        cargs[1].vt = VT_I4; cargs[1].lVal = 0; // Y
                                        cargs[0].vt = VT_I4; cargs[0].lVal = 100; // K
                                        DISPPARAMS cparams = { cargs, NULL, 4, 0 };
                                        pColor->Invoke(dispCmyk, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &cparams, NULL, NULL, NULL);
                                    }
                                    pColor->Release();
                                }
                                pOutline->Release();
                            }
                            
                            InvokeMethodNoArgs(pKare, L"OrderToBack");
                            SetStringProp(pKare, L"Name", L"Kesim");
                            
                            double h = GetDoubleProp(pKare, L"SizeHeight");
                            if (h < 240) SetDoubleProp(pKare, L"SizeHeight", 240);
                            else if (h < 320) SetDoubleProp(pKare, L"SizeHeight", 320);
                            else if (h < 400) SetDoubleProp(pKare, L"SizeHeight", 400);
                            
                            SetDoubleProp(pKare, L"BottomY", bottom - padding);
                            InvokeMethodNoArgs(pKare, L"CreateSelection");
                            
                            pKare->Release();
                        }
                    }
                }
                pLayer->Release();
            }
        }
        pSel->Release();
    }
    pDoc->Release();
}"""

text_cpp = text_cpp.replace(old_kalip_ekle, new_kalip_ekle)

with io.open(path_cpp, 'w', encoding='utf-8') as f:
    f.write(text_cpp)

print("Bugfixes and Kalip methods implemented.")
