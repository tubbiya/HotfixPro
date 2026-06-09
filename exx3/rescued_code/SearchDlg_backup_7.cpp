Created At: 2026-06-04T01:45:11Z
Completed At: 2026-06-04T01:45:11Z
File Path: `file:///c:/Users/tubbi/Dropbox/Claude/HotfixPro/SearchDlg.cpp`
Total Lines: 4703
Total Bytes: 216875
Showing lines 2500 to 2560
The following code has been modified to include a line number before every line, in the format: <line_number>: <original_line>. Please note that any changes targeting the original code should remove the line number, colon, and leading space.
2500: {
2501:     SetRedraw(FALSE);
2502:     UpdateData(TRUE);
2503:     
2504:     int nShow = m_bListOption ? SW_SHOW : SW_HIDE;
2505:     for (auto p : m_combosTas) if (::IsWindow(p->GetSafeHwnd())) p->ShowWindow(nShow);
2506:     for (auto p : m_combosTip) if (::IsWindow(p->GetSafeHwnd())) p->ShowWindow(nShow);
2507:     
2508:     if (m_bListOpen && !m_listObjects.empty()) {
2509:         UpdateListLayout();
2510:     }
2511:     SetRedraw(TRUE);
2512:     Invalidate();
2513:     UpdateWindow();
2514: }
2515: 
2516: void CSearchDlg::OnBnClickedChkKalip()
2517: {
2518:     UpdateData(TRUE);
2519:     ShowKalipPanel(m_bKalip);
2520: }
2521: 
2522: void CSearchDlg::OnBnClickedBtnBilgiEkle()
2523: {
2524:     if (!m_pApp) return;
2525:     CCorelOptimizer optimizer(m_pApp, L"Ztrass Bilgi Ekle");
2526:     
2527:     CString sFirma, sDosya, sKalip, sPres, sMontaj, sKalipIs, sToplamIs;
2528:     GetDlgItemText(IDC_EDT_FIRMA, sFirma);
2529:     
2530:     // Firma adini ayarlara kaydet
2531:     CString iniPath = GetSettingsFilePath();
2532:     if (!iniPath.IsEmpty() && !sFirma.IsEmpty()) {
2533:         WritePrivateProfileString(_T("Settings"), _T("Firma"), sFirma, iniPath);
2534:     }
2535:     
2536:     GetDlgItemText(IDC_EDT_DOSYA, sDosya);
2537:     GetDlgItemText(IDC_EDT_KALIP_ADI, sKalip);
2538:     GetDlgItemText(IDC_EDT_PRES, sPres);
2539:     GetDlgItemText(IDC_EDT_MONTAJ, sMontaj);
2540:     GetDlgItemText(IDC_EDT_KALIPTA_IS, sKalipIs);
2541:     GetDlgItemText(IDC_EDT_TOPLAM_IS, sToplamIs);
2542:     
2543:     IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
2544:     if (!pDoc) return;
2545:     SetLongProp(pDoc, L"Unit", 3); // mm
2546:     
2547:     IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
2548:     if (pSel) {
2549:         long count = GetLongProp(pSel, L"Count");
2550:         if (count > 0) {
2551:             double left = GetDoubleProp(pSel, L"LeftX");
2552:             double top = GetDoubleProp(pSel, L"TopY");
2553:             
2554:             IDispatch* pLayer = GetDispatchProp(pDoc, L"ActiveLayer");
2555:             if (pLayer) {
2556:                 DISPID dispCreate; OLECHAR* szCreate = (OLECHAR*)L"CreateArtisticText";
2557:                 if (SUCCEEDED(pLayer->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dispCreate))) {
2558:                     double currentY = top - 2.0;
2559:                     
2560:                     CString fullText;
The above content does NOT show the entire file contents. If you need to view any lines of the file which were not shown to complete your task, call this tool again to view those lines.
