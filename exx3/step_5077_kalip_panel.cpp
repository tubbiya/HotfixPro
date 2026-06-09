Created At: 2026-06-02T18:57:53Z
Completed At: 2026-06-02T18:57:53Z
File Path: `file:///c:/Users/tubbi/Dropbox/Claude/HotfixPro/SearchDlg.cpp`
Total Lines: 3170
Total Bytes: 138287
Showing lines 2170 to 2250
The following code has been modified to include a line number before every line, in the format: <line_number>: <original_line>. Please note that any changes targeting the original code should remove the line number, colon, and leading space.
    
    if (m_bListOpen && !m_listObjects.empty()) {
        UpdateListLayout();
    }
    SetRedraw(TRUE);
    Invalidate();
    UpdateWindow();
}

void CSearchDlg::OnBnClickedChkKalip()
{
    UpdateData(TRUE);
    ShowKalipPanel(m_bKalip);
}

void CSearchDlg::OnBnClickedBtnBilgiEkle()
{
    if (!m_pApp) return;
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Bilgi Ekle");
    
    CString sFirma, sDosya, sKalip, sPres, sMontaj, sKalipIs, sToplamIs;
    GetDlgItemText(IDC_EDT_FIRMA, sFirma);
    GetDlgItemText(IDC_EDT_DOSYA, sDosya);
    GetDlgItemText(IDC_EDT_KALIP_ADI, sKalip);
    GetDlgItemText(IDC_EDT_PRES, sPres);
    GetDlgItemText(IDC_EDT_MONTAJ, sMontaj);
    GetDlgItemText(IDC_EDT_KALIPTA_IS, sKalipIs);
    GetDlgItemText(IDC_EDT_TOPLAM_IS, sToplamIs);
    
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    SetLongProp(pDoc, L"Unit", 3); // mm
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pSel) {
        long count = GetLongProp(pSel, L"Count");
        if (count > 0) {
            double left = GetDoubleProp(pSel, L"LeftX");
            double top = GetDoubleProp(pSel, L"TopY");
            
            IDispatch* pLayer = GetDispatchProp(pDoc, L"ActiveLayer");
            if (pLayer) {

<truncated 831 bytes>
irma.IsEmpty()) AddLine(_T("Firma: ") + sFirma, CLR_INVALID);
                    if (!sDosya.IsEmpty()) AddLine(_T("Dosya: ") + sDosya, CLR_INVALID);
                    if (!sKalip.IsEmpty()) AddLine(_T("KalÄ±p AdÄ±: ") + sKalip, CLR_INVALID);
                    if (!sPres.IsEmpty()) AddLine(_T("Pres: ") + sPres, CLR_INVALID);
                    if (!sMontaj.IsEmpty()) AddLine(_T("Montaj: ") + sMontaj, CLR_INVALID);
                    if (!sKalipIs.IsEmpty()) AddLine(_T("KalÄ±ptaki Ä°ÅŸ: ") + sKalipIs, CLR_INVALID);
                    if (!sToplamIs.IsEmpty()) AddLine(_T("Toplam Ä°ÅŸ: ") + sToplamIs, CLR_INVALID);
                    
                    if (m_bListOption && !m_listObjects.empty()) {
                        // Benzersiz renk sayÄ±sÄ±nÄ± hesapla
                        std::vector<COLORREF> uniqueClrs;
                        for (size_t i = 0; i < m_listObjects.size(); i++) {
                            COLORREF c = m_listObjects[i].color;
                            if (std::find(uniqueClrs.begin(), uniqueClrs.end(), c) == uniqueClrs.end())
                                uniqueClrs.push_back(c);
                        }
                        CString header;
                        header.Format(_T("%d Renk"), (int)uniqueClrs.size());
                        AddLine(header, CLR_INVALID);
                        
                        for (size_t i = 0; i < m_listObjects.size(); i++) {
                            CString tasAdi, tipAdi;
                            if (i < m_combosTas.size() && ::IsWindow(m_combosTas[i]->GetSafeHwnd())) {
                                m_combosTas[i]->GetWindowText(tasAdi);
                            }
The above content does NOT show the entire file contents. If you need to view any lines of the file which were not shown to complete your task, call this tool again to view those lines.
