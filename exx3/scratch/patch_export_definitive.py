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
    
    CFileDialog dlg(FALSE, L"plt", L"LazerExport.plt", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        L"PLT Dosyalari (*.plt)|*.plt|DXF Dosyalari (*.dxf)|*.dxf||", this);
        
    if (dlg.DoModal() == IDOK) {
        CString path = dlg.GetPathName();
        CString ext = dlg.GetFileExt();
        ext.MakeLower();
        
        // Kesin Dogru Filtre ID'leri (export.txt ve CorelDRAW Object Model uzerinden dogrulandi)
        long filterIdx = (ext == L"dxf") ? 1296 : 1281; 
        
        // 1. Export Options Olustur (CreateStructExportOptions)
        IDispatch* pExpOpt = nullptr;
        DISPID dispCreate; OLECHAR* szCreate = (OLECHAR*)L"CreateStructExportOptions";
        if (SUCCEEDED(m_pApp->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dispCreate))) {
            DISPPARAMS params = { NULL, NULL, 0, 0 };
            VARIANT retOpt; VariantInit(&retOpt);
            if (SUCCEEDED(m_pApp->Invoke(dispCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retOpt, NULL, NULL))) {
                if (retOpt.vt == VT_DISPATCH) pExpOpt = retOpt.pdispVal;
            }
        }
        
        if (pExpOpt) {
            SetBoolProp(pExpOpt, L"UseColorProfile", false);
        }

        // 2. ExportEx Metodu (4 parametreli)
        DISPID dispidExportEx;
        OLECHAR* szExportEx = (OLECHAR*)L"ExportEx";
        if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szExportEx, 1, LOCALE_USER_DEFAULT, &dispidExportEx))) {
            VARIANT args[4];
            for(int i=0; i<4; i++) VariantInit(&args[i]);
            args[3].vt = VT_BSTR; args[3].bstrVal = SysAllocString(path);     // FileName
            args[2].vt = VT_I4;   args[2].lVal = filterIdx;                   // Filter
            args[1].vt = VT_I4;   args[1].lVal = 1;                           // Range (cdrSelection)
            args[0].vt = VT_DISPATCH; args[0].pdispVal = pExpOpt;             // Options
            
            DISPPARAMS params = { args, NULL, 4, 0 };
            VARIANT retVal; VariantInit(&retVal);
            
            HRESULT hrExp = pDoc->Invoke(dispidExportEx, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL);
            if (SUCCEEDED(hrExp) && retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                IDispatch* pFlt = retVal.pdispVal;
                
                // 3. ExportFilter Ayarlari (ProB.frm icindeki gibi)
                if (ext == L"plt") {
                    SetLongProp(pFlt, L"PenLibIndex", 0);
                    SetBoolProp(pFlt, L"FitToPage", false);
                    SetDoubleProp(pFlt, L"ScaleFactor", 100.0);
                    SetDoubleProp(pFlt, L"PageWidth", 8.5);
                    SetDoubleProp(pFlt, L"PageHeight", 11.0);
                    SetLongProp(pFlt, L"FillType", 0); // hpglNoFill
                    SetDoubleProp(pFlt, L"FillSpacing", 0.005);
                    SetDoubleProp(pFlt, L"FillAngle", 0.0);
                    SetDoubleProp(pFlt, L"HatchAngle", 90.0);
                } else if (ext == L"dxf") {
                    SetLongProp(pFlt, L"BitmapType", 0);
                    SetBoolProp(pFlt, L"TextAsCurves", true);
                    SetLongProp(pFlt, L"Version", 1); // dxfVersion2000
                    SetLongProp(pFlt, L"Units", 3);   // dxfMillimeters
                    SetBoolProp(pFlt, L"FillUnmapped", true);
                    SetLongProp(pFlt, L"FillColor", 0);
                }
                
                // 4. Finish ile islemi bitir ve diske yaz!
                InvokeMethodNoArgs(pFlt, L"Finish");
                pFlt->Release();
            } else {
                LogDebug(L"[HATA] ExportEx basarisiz. HR: 0x%08X", hrExp);
            }
            SysFreeString(args[3].bstrVal);
        }
        
        // 5. Eger JPG Seciliyse JPG Export (ExportBitmap kullan)
        if (IsDlgButtonChecked(IDC_CHK_OLCU3) == BST_CHECKED) {
            CString pathJpg = path.Left(path.ReverseFind(L'.')) + L".jpg";
            DISPID dispExpBmp;
            OLECHAR* szExpBmp = (OLECHAR*)L"ExportBitmap";
            if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szExpBmp, 1, LOCALE_USER_DEFAULT, &dispExpBmp))) {
                // ExportBitmap 14 arguman alir. Hepsini VT_ERROR (DISP_E_PARAMNOTFOUND) ile gecebiliriz sadece ilk 3'u yeterli.
                // Ya da ExportEx kullanarak JPG cikarabiliriz.
                DISPID dispExportExJpg;
                if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szExportEx, 1, LOCALE_USER_DEFAULT, &dispExportExJpg))) {
                    VARIANT argsJpg[4];
                    for(int i=0; i<4; i++) VariantInit(&argsJpg[i]);
                    argsJpg[3].vt = VT_BSTR; argsJpg[3].bstrVal = SysAllocString(pathJpg); // FileName
                    argsJpg[2].vt = VT_I4;   argsJpg[2].lVal = 774;                        // Filter (cdrJPEG)
                    argsJpg[1].vt = VT_I4;   argsJpg[1].lVal = 1;                          // Range (cdrSelection)
                    argsJpg[0].vt = VT_DISPATCH; argsJpg[0].pdispVal = pExpOpt;            // Options
                    
                    DISPPARAMS paramsJpg = { argsJpg, NULL, 4, 0 };
                    VARIANT retJpg; VariantInit(&retJpg);
                    
                    if (SUCCEEDED(pDoc->Invoke(dispExportExJpg, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsJpg, &retJpg, NULL, NULL))) {
                        if (retJpg.vt == VT_DISPATCH && retJpg.pdispVal) {
                            IDispatch* pFltJpg = retJpg.pdispVal;
                            SetLongProp(pFltJpg, L"Compression", 20);
                            SetBoolProp(pFltJpg, L"Optimized", true);
                            SetLongProp(pFltJpg, L"Smoothing", 0);
                            SetLongProp(pFltJpg, L"SubFormat", 1);
                            SetBoolProp(pFltJpg, L"Progressive", true);
                            InvokeMethodNoArgs(pFltJpg, L"Finish");
                            pFltJpg->Release();
                        }
                    }
                    SysFreeString(argsJpg[3].bstrVal);
                }
            }
        }
        
        if (pExpOpt) pExpOpt->Release();
        
        MessageBox(L"Export islemi tamamlandi. Dosyalari kontrol edin.", L"Basari", MB_ICONINFORMATION);
    }
    pDoc->Release();
}
"""
    text_cpp = text_cpp[:start_idx] + new_export_method + "\n\n" + text_cpp[end_idx:]
    with io.open(path_cpp, 'w', encoding='utf-8') as f:
        f.write(text_cpp)
    print("Patch applied for definitive ExportEx with Finish")
