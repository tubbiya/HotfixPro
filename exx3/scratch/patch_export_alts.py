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
        
        CString path1 = path.Left(path.ReverseFind(L'.')) + L"_alt1." + ext;
        CString path2 = path.Left(path.ReverseFind(L'.')) + L"_alt2." + ext;
        CString path3 = path.Left(path.ReverseFind(L'.')) + L"_alt3." + ext;
        
        // --- ALTERNATIF 1: Export (Sadece 3 arguman: FileName, Filter, Range) ---
        DISPID dispidExport;
        OLECHAR* szExport = (OLECHAR*)L"Export";
        if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szExport, 1, LOCALE_USER_DEFAULT, &dispidExport))) {
            VARIANT args[3]; for(int i=0; i<3; i++) VariantInit(&args[i]);
            args[2].vt = VT_BSTR; args[2].bstrVal = SysAllocString(path1); // FileName (arg 1)
            args[1].vt = VT_I4; args[1].lVal = filterIdx; // Filter (arg 2)
            args[0].vt = VT_I4; args[0].lVal = 1; // Range = cdrSelection (arg 3)
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

        // --- ALTERNATIF 2: ExportEx (4 arguman, StructExportOptions ile) ---
        IDispatch* pExpOpt = nullptr;
        DISPID dispCreate; OLECHAR* szCreate = (OLECHAR*)L"CreateStructExportOptions";
        if (SUCCEEDED(m_pApp->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dispCreate))) {
            DISPPARAMS params = { NULL, NULL, 0, 0 };
            VARIANT retOpt; VariantInit(&retOpt);
            if (SUCCEEDED(m_pApp->Invoke(dispCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retOpt, NULL, NULL))) {
                if (retOpt.vt == VT_DISPATCH) pExpOpt = retOpt.pdispVal;
            }
        }
        OLECHAR* szExportEx = (OLECHAR*)L"ExportEx";
        if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szExportEx, 1, LOCALE_USER_DEFAULT, &dispidExport))) {
            VARIANT args[4]; for(int i=0; i<4; i++) VariantInit(&args[i]);
            args[3].vt = VT_BSTR; args[3].bstrVal = SysAllocString(path2);
            args[2].vt = VT_I4; args[2].lVal = filterIdx;
            args[1].vt = VT_I4; args[1].lVal = 1; // cdrSelection
            args[0].vt = VT_DISPATCH; args[0].pdispVal = pExpOpt;
            DISPPARAMS params = { args, NULL, 4, 0 };
            VARIANT retVal; VariantInit(&retVal);
            if (SUCCEEDED(pDoc->Invoke(dispidExport, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                    InvokeMethodNoArgs(retVal.pdispVal, L"Finish");
                    retVal.pdispVal->Release();
                }
            }
            SysFreeString(args[3].bstrVal);
        }
        if (pExpOpt) pExpOpt->Release();
        
        // --- ALTERNATIF 3: CorelScript (Eski script motoru ile) ---
        IDispatch* pCorelScript = GetDispatchProp(m_pApp, L"CorelScript");
        if (pCorelScript) {
            DISPID dispFileExport; OLECHAR* szFileExport = (OLECHAR*)L"FileExport";
            if (SUCCEEDED(pCorelScript->GetIDsOfNames(IID_NULL, &szFileExport, 1, LOCALE_USER_DEFAULT, &dispFileExport))) {
                // FileExport(FileName, Filter, Width, Height, DPI, SelectionOnly)
                VARIANT args[6]; for(int i=0; i<6; i++) VariantInit(&args[i]);
                args[5].vt = VT_BSTR; args[5].bstrVal = SysAllocString(path3); // FileName
                args[4].vt = VT_I4; args[4].lVal = filterIdx; // Filter
                args[3].vt = VT_I4; args[3].lVal = 0; // Width
                args[2].vt = VT_I4; args[2].lVal = 0; // Height
                args[1].vt = VT_I4; args[1].lVal = 0; // DPI
                args[0].vt = VT_BOOL; args[0].boolVal = VARIANT_TRUE; // SelectionOnly
                DISPPARAMS params = { args, NULL, 6, 0 };
                VARIANT retVal; VariantInit(&retVal);
                pCorelScript->Invoke(dispFileExport, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL);
                SysFreeString(args[5].bstrVal);
            }
            pCorelScript->Release();
        }
        
        MessageBox(L"3 alternatif Export islemi denendi! Lutfen sectiginiz klasoru kontrol edin:\\n\\n_alt1." + ext + L" (Export)\\n_alt2." + ext + L" (ExportEx)\\n_alt3." + ext + L" (CorelScript)\\n\\nHangisi olustuysa soyleyin.", L"Export Test", MB_ICONINFORMATION);
    }
    pSel->Release();
    pDoc->Release();
}
"""
    text_cpp = text_cpp[:start_idx] + new_export_method + "\n\n" + text_cpp[end_idx:]
    with io.open(path_cpp, 'w', encoding='utf-8') as f:
        f.write(text_cpp)
    print("Patch applied for 3 export alternatives")
