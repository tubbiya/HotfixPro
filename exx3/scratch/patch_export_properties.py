import io

path_cpp = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'

with io.open(path_cpp, 'r', encoding='utf-8') as f:
    text_cpp = f.read()

start_idx = text_cpp.find("void CSearchDlg::OnBnClickedBtnExport()")
if start_idx != -1:
    end_idx = text_cpp.find("void CSearchDlg::OnBnClickedBtnSelectMatch()", start_idx)
    
    new_export_method = """void CSearchDlg::OnBnClickedBtnExport()
{
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (!pSel) { pDoc->Release(); return; }
    
    long count = GetLongProp(pSel, L"Count");
    if (count == 0) {
        MessageBox(L"Lutfen once disa aktarilacak objeleri secin.", L"Uyari", MB_ICONWARNING);
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
        
        IDispatch* pExpOpt = nullptr;
        DISPID dispCreate; OLECHAR* szCreate = (OLECHAR*)L"CreateStructExportOptions";
        if (SUCCEEDED(m_pApp->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dispCreate))) {
            DISPPARAMS params = { NULL, NULL, 0, 0 };
            VARIANT retOpt; VariantInit(&retOpt);
            if (SUCCEEDED(m_pApp->Invoke(dispCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retOpt, NULL, NULL))) {
                if (retOpt.vt == VT_DISPATCH) pExpOpt = retOpt.pdispVal;
            }
        }
        
        if (pExpOpt) SetBoolProp(pExpOpt, L"UseColorProfile", false);

        OLECHAR* szExportEx = (OLECHAR*)L"ExportEx";
        DISPID dispidExport;
        if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szExportEx, 1, LOCALE_USER_DEFAULT, &dispidExport))) {
            VARIANT args[4]; for(int i=0; i<4; i++) VariantInit(&args[i]);
            args[3].vt = VT_BSTR; args[3].bstrVal = SysAllocString(path);
            args[2].vt = VT_I4; args[2].lVal = filterIdx;
            args[1].vt = VT_I4; args[1].lVal = 1; // cdrSelection
            args[0].vt = VT_DISPATCH; args[0].pdispVal = pExpOpt;
            DISPPARAMS params = { args, NULL, 4, 0 };
            VARIANT retVal; VariantInit(&retVal);
            
            if (SUCCEEDED(pDoc->Invoke(dispidExport, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                    IDispatch* pFlt = retVal.pdispVal;
                    if (ext == L"plt") {
                        SetLongProp(pFlt, L"PenLibIndex", 0);
                        SetBoolProp(pFlt, L"FitToPage", false);
                        SetDoubleProp(pFlt, L"ScaleFactor", 100.0);
                        SetDoubleProp(pFlt, L"PageWidth", 8.5);
                        SetDoubleProp(pFlt, L"PageHeight", 11.0);
                        SetLongProp(pFlt, L"FillType", 0);
                        SetDoubleProp(pFlt, L"FillSpacing", 0.005);
                        SetDoubleProp(pFlt, L"FillAngle", 0.0);
                        SetDoubleProp(pFlt, L"HatchAngle", 90.0);
                    } else if (ext == L"dxf") {
                        SetLongProp(pFlt, L"BitmapType", 0);
                        SetBoolProp(pFlt, L"TextAsCurves", true);
                        SetLongProp(pFlt, L"Version", 1);
                        SetLongProp(pFlt, L"Units", 3);
                        SetBoolProp(pFlt, L"FillUnmapped", true);
                        SetLongProp(pFlt, L"FillColor", 0);
                    }
                    InvokeMethodNoArgs(pFlt, L"Finish");
                    pFlt->Release();
                }
            }
            SysFreeString(args[3].bstrVal);
        }
        
        if (IsDlgButtonChecked(IDC_CHK_OLCU3) == BST_CHECKED) {
            CString pathJpg = path.Left(path.ReverseFind(L'.')) + L".jpg";
            if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szExportEx, 1, LOCALE_USER_DEFAULT, &dispidExport))) {
                VARIANT args[4]; for(int i=0; i<4; i++) VariantInit(&args[i]);
                args[3].vt = VT_BSTR; args[3].bstrVal = SysAllocString(pathJpg);
                args[2].vt = VT_I4; args[2].lVal = 774; // cdrJPEG
                args[1].vt = VT_I4; args[1].lVal = 1; // cdrSelection
                args[0].vt = VT_DISPATCH; args[0].pdispVal = pExpOpt;
                DISPPARAMS params = { args, NULL, 4, 0 };
                VARIANT retVal; VariantInit(&retVal);
                if (SUCCEEDED(pDoc->Invoke(dispidExport, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                    if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                        IDispatch* pFlt = retVal.pdispVal;
                        SetLongProp(pFlt, L"Compression", 20);
                        SetBoolProp(pFlt, L"Optimized", true);
                        SetLongProp(pFlt, L"Smoothing", 0);
                        SetLongProp(pFlt, L"SubFormat", 1);
                        SetBoolProp(pFlt, L"Progressive", true);
                        InvokeMethodNoArgs(pFlt, L"Finish");
                        pFlt->Release();
                    }
                }
                SysFreeString(args[3].bstrVal);
            }
        }
        
        if (pExpOpt) pExpOpt->Release();
        
        MessageBox(L"Secilen formatta Export islemi basariyla gerceklestirildi.", L"Basarili", MB_ICONINFORMATION);
    }
    pSel->Release();
    pDoc->Release();
}
"""
    text_cpp = text_cpp[:start_idx] + new_export_method + "\n\n" + text_cpp[end_idx:]
    with io.open(path_cpp, 'w', encoding='utf-8') as f:
        f.write(text_cpp)
    print("Patch applied: proper filter properties added before Finish()")
