Created At: 2026-06-03T23:11:17Z
Completed At: 2026-06-03T23:11:17Z
File Path: `file:///c:/Users/tubbi/Dropbox/Claude/HotfixPro/SearchDlg.cpp`
Total Lines: 4441
Total Bytes: 206410
Showing lines 4350 to 4440
The following code has been modified to include a line number before every line, in the format: <line_number>: <original_line>. Please note that any changes targeting the original code should remove the line number, colon, and leading space.
    GetDlgItemText(IDC_EDT_EN, strEn);
    SetDlgItemText(IDC_EDT_BOY, strEn);
}

void CSearchDlg::OnBnClickedBtnAra5()
{
    if (!m_pApp) return;
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (!pSel) return;
    long count = GetLongProp(pSel, L"Count");
    if (count == 0) { pSel->Release(); return; }

    for (long i = 1; i <= count; i++) {
        VARIANT arg; VariantInit(&arg); arg.vt = VT_I4; arg.lVal = i;
        DISPPARAMS prms = { &arg, NULL, 1, 0 };
        VARIANT res; VariantInit(&res);
        if (SUCCEEDED(pSel->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &prms, &res, NULL, NULL)) && res.vt == VT_DISPATCH) {
            IDispatch* pShape = res.pdispVal;
            if (pShape) {
                IDispatch* pOutline = GetDispatchProp(pShape, L"Outline");
                IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
                if (pOutline && pFill) {
                    IDispatch* pOutlineColor = GetDispatchProp(pOutline, L"Color");
                    if (pOutlineColor) {
                        DISPID dApply; OLECHAR* szApply = (OLECHAR*)L"ApplyUniformFill";
                        if (SUCCEEDED(pFill->GetIDsOfNames(IID_NULL, &szApply, 1, LOCALE_USER_DEFAULT, &dApply))) {
                            VARIANT argF; VariantInit(&argF); argF.vt = VT_DISPATCH; argF.pdispVal = pOutlineColor;
                            DISP
<truncated 1445 bytes>
pe, L"Fill");
                if (pOutline && pFill) {
                    long fillType = GetLongProp(pFill, L"Type");
                    if (fillType == 1) { // cdrUniformFill
                        IDispatch* pFillColor = GetDispatchProp(pFill, L"UniformColor");
                        if (pFillColor) {
                            IDispatch* pOutlineColor = GetDispatchProp(pOutline, L"Color");
                            if (pOutlineColor) {
                                DISPID dCopy; OLECHAR* szCopy = (OLECHAR*)L"CopyAssign";
                                if (SUCCEEDED(pOutlineColor->GetIDsOfNames(IID_NULL, &szCopy, 1, LOCALE_USER_DEFAULT, &dCopy))) {
                                    VARIANT argC; VariantInit(&argC); argC.vt = VT_DISPATCH; argC.pdispVal = pFillColor;
                                    DISPPARAMS prmsC = { &argC, NULL, 1, 0 };
                                    pOutlineColor->Invoke(dCopy, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsC, NULL, NULL, NULL);
                                }
                                pOutlineColor->Release();
                            }
                            
                            long outType = GetLongProp(pOutline, L"Type");
                            if (outType == 0) {
                                SetDoubleProp(pOutline, L"Width", 0.5);
                            }
                            pFillColor->Release();
                        }
                    }
                    pFill->Release();
                    pOutline->Release();
                }
                pShape->Release();
            }
        }
    }
    pSel->Release();
}
The above content does NOT show the entire file contents. If you need to view any lines of the file which were not shown to complete your task, call this tool again to view those lines.
