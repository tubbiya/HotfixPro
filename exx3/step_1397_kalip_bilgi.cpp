Created At: 2026-06-03T18:04:40Z
Completed At: 2026-06-03T18:04:40Z
File Path: `file:///c:/Users/tubbi/Dropbox/Claude/HotfixPro/SearchDlg.cpp`
Total Lines: 3982
Total Bytes: 182292
Showing lines 2250 to 2350
The following code has been modified to include a line number before every line, in the format: <line_number>: <original_line>. Please note that any changes targeting the original code should remove the line number, colon, and leading space.
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
                DISPID dispCreate; OLECHAR* szCreate = (OLECHAR*)L"CreateArtisticText";
                if (SUCCEEDED(pLayer->GetIDsOfNa
<truncated 2184 bytes>
es: %d"), nPres); AddLine(tmp, CLR_INVALID); }
                    if (nMontaj > 0) { tmp.Format(_T("Montaj: %d"), nMontaj); AddLine(tmp, CLR_INVALID); }
                    if (nKalipIs > 0) { tmp.Format(_T("KalÄ±ptaki Ä°ÅŸ: %d"), nKalipIs); AddLine(tmp, CLR_INVALID); }
                    if (nToplamIs > 0) { tmp.Format(_T("Toplam Ä°ÅŸ: %d"), nToplamIs); AddLine(tmp, CLR_INVALID); }
                    
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
                            if (i < m_combosTip.size() && ::IsWindow(m_combosTip[i]->GetSafeHwnd())) {
                                m_combosTip[i]->GetWindowText(tipAdi);
                            }
                            
The above content does NOT show the entire file contents. If you need to view any lines of the file which were not shown to complete your task, call this tool again to view those lines.
