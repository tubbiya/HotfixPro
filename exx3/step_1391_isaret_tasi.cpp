Created At: 2026-06-03T18:04:24Z
Completed At: 2026-06-03T18:04:24Z
File Path: `file:///c:/Users/tubbi/Dropbox/Claude/HotfixPro/SearchDlg.cpp`
Total Lines: 3982
Total Bytes: 182292
Showing lines 2430 to 2520
The following code has been modified to include a line number before every line, in the format: <line_number>: <original_line>. Please note that any changes targeting the original code should remove the line number, colon, and leading space.
                                            DISPID dispItem;
                                            OLECHAR* szItem = (OLECHAR*)L"Item";
                                            if (SUCCEEDED(pLines->GetIDsOfNames(IID_NULL, &szItem, 1, LOCALE_USER_DEFAULT, &dispItem))) {
                                                for (size_t i = 0; i < lineColors.size(); ++i) {
                                                    if (lineColors[i] != CLR_INVALID && lineColors[i] != RGB(0,0,0)) {
                                                        VARIANT vIdx; vIdx.vt = VT_I4; vIdx.lVal = (long)(i + 1);
                                                        DISPPARAMS dpItem = { &vIdx, NULL, 1, 0 };
                                                        VARIANT vLine; VariantInit(&vLine);
                                                        if (SUCCEEDED(pLines->Invoke(dispItem, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET | DISPATCH_METHOD, &dpItem, &vLine, NULL, NULL)) && vLine.vt == VT_DISPATCH) {
                                                            IDispatch* pLine = vLine.pdispVal;
                                                            if (pLine) {
                                                                IDispatch* pFill = GetDispatchProp(pLine, L"Fill");
                                                                if (pFill) {
                                                                    IDispatch* pColor = GetDispatchProp(pFill, L"
<truncated 3108 bytes>
 }

void CSearchDlg::OnBnClickedBtnKalipEkle() // Isaret Tasi (KalipTasM)
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
            IDispatch* pPage = GetDispatchProp(pDoc, L"ActivePage");
            if (pPage) {
                DISPID dispSelect; OLECHAR* szSelect = (OLECHAR*)L"SelectShapesFromRectangle";
                if (SUCCEEDED(pPage->GetIDsOfNames(IID_NULL, &szSelect, 1, LOCALE_USER_DEFAULT, &dispSelect))) {
                    VARIANT args[5]; for(int i=0; i<5; ++i) VariantInit(&args[i]);
                    args[4].vt = VT_R8; args[4].dblVal = left - 5.0; // Left
                    args[3].vt = VT_R8; args[3].dblVal = top + 5.0; // Top
                    args[2].vt = VT_R8; args[2].dblVal = right + 5.0; // Right
                    args[1].vt = VT_R8; args[1].dblVal = bottom - 5.0; // Bottom
The above content does NOT show the entire file contents. If you need to view any lines of the file which were not shown to complete your task, call this tool again to view those lines.
