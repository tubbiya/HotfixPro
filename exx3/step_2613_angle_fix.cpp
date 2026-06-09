Created At: 2026-06-04T01:32:26Z
Completed At: 2026-06-04T01:32:26Z
File Path: `file:///c:/Users/tubbi/Dropbox/Claude/HotfixPro/SearchDlg.cpp`
Total Lines: 4631
Total Bytes: 213408
Showing lines 4495 to 4631
The following code has been modified to include a line number before every line, in the format: <line_number>: <original_line>. Please note that any changes targeting the original code should remove the line number, colon, and leading space.
// A3 - AÃ§Ä± dÃ¼zeltme: 2 noktalÄ± Ã§izginin aÃ§Ä±sÄ±nÄ± bulup yatay/dikeye dÃ¼zleÅŸtir, diÄŸer objelere de uygula
void CSearchDlg::OnBnClickedBtnAra7()
{
    if (!m_pApp) return;
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (!pSel) return;
    long count = GetLongProp(pSel, L"Count");
    if (count < 1) { pSel->Release(); return; }

    // Ã‡izgiyi bul (2 noktalÄ± curve)
    IDispatch* pLine = NULL;
    long lineIndex = 0;
    for (long i = 1; i <= count; i++) {
        VARIANT arg; VariantInit(&arg); arg.vt = VT_I4; arg.lVal = i;
        DISPPARAMS prms = { &arg, NULL, 1, 0 };
        VARIANT res; VariantInit(&res);
        if (SUCCEEDED(pSel->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &prms, &res, NULL, NULL)) && res.vt == VT_DISPATCH) {
            IDispatch* pShape = res.pdispVal;
            long shapeType = GetLongProp(pShape, L"Type");
            // cdrCurveShape = 6 veya kontrol et
            IDispatch* pCurve = GetDispatchProp(pShape, L"Curve");
            if (pCurve) {
                IDispatch* pNodes = GetDispatchProp(pCurve, L"Nodes");
                if (pNodes) {
                    long nodeCount = GetLongProp(pNodes, L"Count");
                    if (nodeCount == 2) {
                        pLine = pShape;
                        pLine->AddRef();
                        lineIndex = i;
                       
<truncated 2737 bytes>
 dikey
    if (fabs(normAngle) <= 45.0) {
        // Yataya yakÄ±n, dÃ¼zelt
        rotationAngle = -normAngle;
    } else {
        // Dikeye yakÄ±n
        if (normAngle > 0)
            rotationAngle = -(normAngle - 90.0);
        else
            rotationAngle = -(normAngle + 90.0);
    }

    if (fabs(rotationAngle) < 0.001) {
        pLine->Release(); pSel->Release();
        return; // Zaten dÃ¼z
    }

    // TÃ¼m seÃ§ili objelere rotasyon uygula
    OptimizeCorel(m_pApp, TRUE, L"AÃ§Ä± DÃ¼zelt");

    // Rotate argÃ¼manlarÄ±: Angle (derece)
    VARIANT argRot; VariantInit(&argRot);
    argRot.vt = VT_R8; argRot.dblVal = rotationAngle;

    for (long i = 1; i <= count; i++) {
        VARIANT arg; VariantInit(&arg); arg.vt = VT_I4; arg.lVal = i;
        DISPPARAMS prms = { &arg, NULL, 1, 0 };
        VARIANT res; VariantInit(&res);
        if (SUCCEEDED(pSel->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &prms, &res, NULL, NULL)) && res.vt == VT_DISPATCH) {
            IDispatch* pShape = res.pdispVal;
            // Shape.Rotate(angle)
            DISPID dRotate; OLECHAR* szRotate = (OLECHAR*)L"Rotate";
            if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szRotate, 1, LOCALE_USER_DEFAULT, &dRotate))) {
                DISPPARAMS prmsR = { &argRot, NULL, 1, 0 };
                pShape->Invoke(dRotate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsR, NULL, NULL, NULL);
            }
            pShape->Release();
        }
    }

    OptimizeCorel(m_pApp, FALSE, L"AÃ§Ä± DÃ¼zelt");
    pLine->Release();
    pSel->Release();
}

The above content does NOT show the entire file contents. If you need to view any lines of the file which were not shown to complete your task, call this tool again to view those lines.
