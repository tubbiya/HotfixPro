import io
import re

path_h = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.h'
with io.open(path_h, 'r', encoding='utf-8', errors='ignore') as f:
    text_h = f.read()

if "void OnBnClickedBtnExport()" not in text_h:
    text_h = text_h.replace("afx_msg void OnBnClickedBtnFind();", "afx_msg void OnBnClickedBtnFind();\n    afx_msg void OnBnClickedBtnExport();")

with io.open(path_h, 'w', encoding='utf-8') as f:
    f.write(text_h)


path_cpp = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path_cpp, 'r', encoding='utf-8', errors='ignore') as f:
    text_cpp = f.read()

if "ON_BN_CLICKED(IDC_BTN_ARA11" not in text_cpp:
    text_cpp = text_cpp.replace("ON_BN_CLICKED(IDC_BTN_FIND, &CSearchDlg::OnBnClickedBtnFind)", "ON_BN_CLICKED(IDC_BTN_FIND, &CSearchDlg::OnBnClickedBtnFind)\n    ON_BN_CLICKED(IDC_BTN_ARA11, &CSearchDlg::OnBnClickedBtnExport)")

# 1. Fix Checkbox List
old_list = """    if (m_bListOpen) {
        m_bListOpen = FALSE;
        ClearDynamicCombos();
        m_bListOption = FALSE;
        UpdateData(FALSE);
        SetDialogHeight(198);
        CWnd* pGroup = GetDlgItem(IDC_GRP_LISTE);
        if (pGroup) pGroup->ShowWindow(SW_HIDE);
        m_listObjects.clear();
    }
    else {
        m_bListOpen = TRUE;"""
new_list = """    if (m_bListOpen) {
        m_bListOpen = FALSE;
        ClearDynamicCombos();
        m_bListOption = FALSE;
        UpdateData(FALSE);
        CheckDlgButton(IDC_CHK_LIST_OPTION, BST_UNCHECKED);
        SetDialogHeight(198);
        CWnd* pGroup = GetDlgItem(IDC_GRP_LISTE);
        if (pGroup) pGroup->ShowWindow(SW_HIDE);
        m_listObjects.clear();
    }
    else {
        m_bListOpen = TRUE;
        m_bListOption = TRUE;
        CheckDlgButton(IDC_CHK_LIST_OPTION, BST_CHECKED);"""
text_cpp = text_cpp.replace(old_list, new_list)

# 2. Fix ResizeShapes for Circles (Iterating backwards!)
old_resize = """            bool bCircleOnly = (IsDlgButtonChecked(IDC_CHK_OLCU2) == BST_CHECKED);
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
            }"""
new_resize = """            bool bCircleOnly = (IsDlgButtonChecked(IDC_CHK_OLCU2) == BST_CHECKED);
            if (bCircleOnly) h_mm = w_mm;
            
            for (long i = count; i >= 1; --i) {
                IDispatch* pShape = GetDispatchPropWithIntArg(pSel, L"Item", i);
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
                                DISPPARAMS params = { args, NULL, 3, 0 };
                                VARIANT retVal; VariantInit(&retVal);
                                if (SUCCEEDED(pLayer->Invoke(dispidCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                                    if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                                        DISPID dispidDelete;
                                        OLECHAR* szDelete = (OLECHAR*)L"Delete";
                                        if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szDelete, 1, LOCALE_USER_DEFAULT, &dispidDelete))) {
                                            DISPPARAMS delParams = { NULL, NULL, 0, 0 };
                                            pShape->Invoke(dispidDelete, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &delParams, NULL, NULL, NULL);
                                        }
                                        retVal.pdispVal->Release();
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
                            DISPPARAMS params = { args, NULL, 2, 0 };
                            pShape->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, NULL, NULL, NULL);
                        }
                    }
                    pShape->Release();
                }
            }"""
text_cpp = text_cpp.replace(old_resize, new_resize)

# 3. Fix Lazer / Intersecting Objects
find_impl = """
void CSearchDlg::OnBnClickedBtnFind()
{
    if (!m_pApp) return;
    
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Lazer Kontrol");
    
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    
    SetLongProp(pDoc, L"Unit", 3);
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pSel) {
        long count = GetLongProp(pSel, L"Count");
        if (count > 0) {
            CString strTol;
            GetDlgItemText(IDC_EDT_TOLERANCE, strTol);
            strTol.Replace(L",", L".");
            double tol = _wtof(strTol);
            if (tol <= 0.0) tol = 0.1;
            
            bool bDelete = (IsDlgButtonChecked(IDC_RAD_DELETE) == BST_CHECKED);
            bool bCenter = (IsDlgButtonChecked(IDC_RAD_CENTER) == BST_CHECKED);
            
            struct CircleInfo {
                long index;
                double cx, cy, radius;
                bool markForDelete;
            };
            std::vector<CircleInfo> circles;
            
            for (long i = 1; i <= count; ++i) {
                IDispatch* pShape = GetDispatchPropWithIntArg(pSel, L"Item", i);
                if (pShape) {
                    double w = GetDoubleProp(pShape, L"SizeWidth");
                    double cx = GetDoubleProp(pShape, L"CenterX");
                    double cy = GetDoubleProp(pShape, L"CenterY");
                    circles.push_back({i, cx, cy, w / 2.0, false});
                    pShape->Release();
                }
            }
            
            for (size_t i = 0; i < circles.size(); ++i) {
                if (circles[i].markForDelete) continue;
                for (size_t j = i + 1; j < circles.size(); ++j) {
                    if (circles[j].markForDelete) continue;
                    
                    double dx = circles[i].cx - circles[j].cx;
                    double dy = circles[i].cy - circles[j].cy;
                    double dist = sqrt(dx*dx + dy*dy);
                    
                    double requiredDist = circles[i].radius + circles[j].radius - tol;
                    if (dist < requiredDist) {
                        circles[j].markForDelete = true;
                        if (bCenter) {
                            circles[i].cx = (circles[i].cx + circles[j].cx) / 2.0;
                            circles[i].cy = (circles[i].cy + circles[j].cy) / 2.0;
                        }
                    }
                }
            }
            
            for (long i = (long)circles.size() - 1; i >= 0; --i) {
                if (circles[i].markForDelete) {
                    IDispatch* pShape = GetDispatchPropWithIntArg(pSel, L"Item", circles[i].index);
                    if (pShape) {
                        if (bDelete || bCenter) {
                            DISPID dispid;
                            OLECHAR* szDelete = (OLECHAR*)L"Delete";
                            if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szDelete, 1, LOCALE_USER_DEFAULT, &dispid))) {
                                DISPPARAMS params = { NULL, NULL, 0, 0 };
                                pShape->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, NULL, NULL, NULL);
                            }
                        } else {
                            IDispatch* pOutline = GetDispatchProp(pShape, L"Outline");
                            if (pOutline) {
                                IDispatch* pColor = GetDispatchProp(pOutline, L"Color");
                                if (pColor) {
                                    DISPID dispid;
                                    OLECHAR* szAssign = (OLECHAR*)L"RGBAssign";
                                    if (SUCCEEDED(pColor->GetIDsOfNames(IID_NULL, &szAssign, 1, LOCALE_USER_DEFAULT, &dispid))) {
                                        VARIANT args[3]; VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]);
                                        args[2].vt = VT_I4; args[2].lVal = 255;
                                        args[1].vt = VT_I4; args[1].lVal = 0;
                                        args[0].vt = VT_I4; args[0].lVal = 0;
                                        DISPPARAMS params = { args, NULL, 3, 0 };
                                        pColor->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, NULL, NULL, NULL);
                                    }
                                    pColor->Release();
                                }
                                pOutline->Release();
                            }
                        }
                        pShape->Release();
                    }
                } else if (bCenter) {
                    IDispatch* pShape = GetDispatchPropWithIntArg(pSel, L"Item", circles[i].index);
                    if (pShape) {
                        SetDoubleProp(pShape, L"CenterX", circles[i].cx);
                        SetDoubleProp(pShape, L"CenterY", circles[i].cy);
                        pShape->Release();
                    }
                }
            }
        }
        pSel->Release();
    }
    pDoc->Release();
}

void CSearchDlg::OnBnClickedBtnExport()
{
    if (!m_pApp) return;
    
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) {
        MessageBox(L"Lutfen once bir belge acin.", L"Hata", MB_ICONERROR);
        return;
    }
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (!pSel) {
        pDoc->Release();
        return;
    }
    long count = GetLongProp(pSel, L"Count");
    if (count == 0) {
        MessageBox(L"Lutfen export edilecek objeleri secin.", L"Hata", MB_ICONWARNING);
        pSel->Release();
        pDoc->Release();
        return;
    }
    
    CFileDialog dlg(FALSE, L"plt", L"LazerExport.plt", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        L"PLT Dosyalari (*.plt)|*.plt|DXF Dosyalari (*.dxf)|*.dxf||", this);
        
    if (dlg.DoModal() == IDOK) {
        CString path = dlg.GetPathName();
        CString ext = dlg.GetFileExt();
        ext.MakeLower();
        
        long filterIdx = (ext == L"dxf") ? 1313 : 1303; // 1313 = cdrDXF, 1303 = cdrHPGL
        
        DISPID dispidExport;
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
        }
        
        MessageBox(L"Export islemi tamamlandi!", L"Basarili", MB_ICONINFORMATION);
    }
    pSel->Release();
    pDoc->Release();
}
"""

if "void CSearchDlg::OnBnClickedBtnFind()" in text_cpp:
    # already there, just replace it
    text_cpp = re.sub(r"void CSearchDlg::OnBnClickedBtnFind\(\)\s*\{[^\}]*\}\s*(?=\nvoid)", find_impl, text_cpp)
else:
    text_cpp += "\n" + find_impl

with io.open(path_cpp, 'w', encoding='utf-8') as f:
    f.write(text_cpp)

print("Features added.")
