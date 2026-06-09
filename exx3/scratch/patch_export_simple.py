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
        
        long filterIdx = (ext == L"dxf") ? 1313 : 1303; // 1313=cdrDXF, 1303=cdrHPGL
        
        // Cok basit Export: ActiveDocument.Export(FileName) 
        // Sadece FileName verelim, gerisini CorelDRAW halletsin.
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
        
        // JPG Export (Eger seciliyse)
        if (IsDlgButtonChecked(IDC_CHK_OLCU3) == BST_CHECKED) {
            CString pathJpg = path.Left(path.ReverseFind(L'.')) + L".jpg";
            if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szExport, 1, LOCALE_USER_DEFAULT, &dispidExport))) {
                VARIANT args[3];
                VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]);
                args[2].vt = VT_BSTR; args[2].bstrVal = SysAllocString(pathJpg);
                args[1].vt = VT_I4; args[1].lVal = 774; // cdrJPEG
                args[0].vt = VT_I4; args[0].lVal = 1; // cdrSelection
                
                DISPPARAMS params = { args, NULL, 3, 0 };
                pDoc->Invoke(dispidExport, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, NULL, NULL, NULL);
                SysFreeString(args[2].bstrVal);
            }
        }
        
        MessageBox(L"Cok basit Export islemi tamamlandi. Dosyalari kontrol edin.", L"Basarili", MB_ICONINFORMATION);
    }
    pDoc->Release();
}
"""
    text_cpp = text_cpp[:start_idx] + new_export_method + "\n\n" + text_cpp[end_idx:]
    with io.open(path_cpp, 'w', encoding='utf-8') as f:
        f.write(text_cpp)
    print("Patch applied for simple export")
