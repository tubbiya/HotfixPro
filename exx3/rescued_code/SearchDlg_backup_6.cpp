Created At: 2026-06-03T20:26:58Z
Completed At: 2026-06-03T20:26:58Z
File Path: `file:///c:/Users/tubbi/Dropbox/Claude/HotfixPro/SearchDlg.cpp`
Total Lines: 4190
Total Bytes: 195351
Showing lines 2400 to 2500
The following code has been modified to include a line number before every line, in the format: <line_number>: <original_line>. Please note that any changes targeting the original code should remove the line number, colon, and leading space.
2400:     SetRedraw(TRUE);
2401:     Invalidate();
2402:     UpdateWindow();
2403: }
2404: 
2405: void CSearchDlg::OnBnClickedChkKalip()
2406: {
2407:     UpdateData(TRUE);
2408:     ShowKalipPanel(m_bKalip);
2409: }
2410: 
2411: void CSearchDlg::OnBnClickedBtnBilgiEkle()
2412: {
2413:     if (!m_pApp) return;
2414:     CCorelOptimizer optimizer(m_pApp, L"Ztrass Bilgi Ekle");
2415:     
2416:     CString sFirma, sDosya, sKalip, sPres, sMontaj, sKalipIs, sToplamIs;
2417:     GetDlgItemText(IDC_EDT_FIRMA, sFirma);
2418:     GetDlgItemText(IDC_EDT_DOSYA, sDosya);
2419:     GetDlgItemText(IDC_EDT_KALIP_ADI, sKalip);
2420:     GetDlgItemText(IDC_EDT_PRES, sPres);
2421:     GetDlgItemText(IDC_EDT_MONTAJ, sMontaj);
2422:     GetDlgItemText(IDC_EDT_KALIPTA_IS, sKalipIs);
2423:     GetDlgItemText(IDC_EDT_TOPLAM_IS, sToplamIs);
2424:     
2425:     IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
2426:     if (!pDoc) return;
2427:     SetLongProp(pDoc, L"Unit", 3); // mm
2428:     
2429:     IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
2430:     if (pSel) {
2431:         long count = GetLongProp(pSel, L"Count");
2432:         if (count > 0) {
2433:             double left = GetDoubleProp(pSel, L"LeftX");
2434:             double top = GetDoubleProp(pSel, L"TopY");
2435:             
2436:             IDispatch* pLayer = GetDispatchProp(pDoc, L"ActiveLayer");
2437:             if (pLayer) {
2438:                 DISPID dispCreate; OLECHAR* szCreate = (OLECHAR*)L"CreateArtisticText";
2439:                 if (
<truncated 2144 bytes>
      
2476:                     if (nPres > 0) { tmp.Format(_T("Pres: %d"), nPres); AddLine(tmp, CLR_INVALID); }
2477:                     if (nMontaj > 0) { tmp.Format(_T("Montaj: %d"), nMontaj); AddLine(tmp, CLR_INVALID); }
2478:                     if (nKalipIs > 0) { tmp.Format(_T("Kalıptaki İş: %d"), nKalipIs); AddLine(tmp, CLR_INVALID); }
2479:                     if (nToplamIs > 0) { tmp.Format(_T("Toplam İş: %d"), nToplamIs); AddLine(tmp, CLR_INVALID); }
2480:                     
2481:                     CTime t = CTime::GetCurrentTime();
2482:                     CString sTime = t.Format(_T("%d.%m.%Y %H:%M"));
2483:                     AddLine(sTime, CLR_INVALID);
2484:                     if (m_bListOption && !m_listObjects.empty()) {
2485:                         // Benzersiz renk sayısını hesapla
2486:                         std::vector<COLORREF> uniqueClrs;
2487:                         for (size_t i = 0; i < m_listObjects.size(); i++) {
2488:                             COLORREF c = m_listObjects[i].color;
2489:                             if (std::find(uniqueClrs.begin(), uniqueClrs.end(), c) == uniqueClrs.end())
2490:                                 uniqueClrs.push_back(c);
2491:                         }
2492:                         CString header;
2493:                         header.Format(_T("%d Renk"), (int)uniqueClrs.size());
2494:                         AddLine(header, CLR_INVALID);
2495:                         
2496:                         for (size_t i = 0; i < m_listObjects.size(); i++) {
2497:                             CString tasAdi, tipAdi;
2498:                             if (i < m_combosTas.size() && ::IsWindow(m_combosTas[i]->GetSafeHwnd())) {
2499:                                 m_combosTas[i]->GetWindowText(tasAdi);
2500:                             }
The above content does NOT show the entire file contents. If you need to view any lines of the file which were not shown to complete your task, call this tool again to view those lines.
