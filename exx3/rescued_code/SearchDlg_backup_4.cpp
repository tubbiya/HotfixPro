Created At: 2026-06-03T18:04:40Z
Completed At: 2026-06-03T18:04:40Z
File Path: `file:///c:/Users/tubbi/Dropbox/Claude/HotfixPro/SearchDlg.cpp`
Total Lines: 3982
Total Bytes: 182292
Showing lines 2250 to 2350
The following code has been modified to include a line number before every line, in the format: <line_number>: <original_line>. Please note that any changes targeting the original code should remove the line number, colon, and leading space.
2250:     Invalidate();
2251:     UpdateWindow();
2252: }
2253: 
2254: void CSearchDlg::OnBnClickedChkKalip()
2255: {
2256:     UpdateData(TRUE);
2257:     ShowKalipPanel(m_bKalip);
2258: }
2259: 
2260: void CSearchDlg::OnBnClickedBtnBilgiEkle()
2261: {
2262:     if (!m_pApp) return;
2263:     CCorelOptimizer optimizer(m_pApp, L"Ztrass Bilgi Ekle");
2264:     
2265:     CString sFirma, sDosya, sKalip, sPres, sMontaj, sKalipIs, sToplamIs;
2266:     GetDlgItemText(IDC_EDT_FIRMA, sFirma);
2267:     GetDlgItemText(IDC_EDT_DOSYA, sDosya);
2268:     GetDlgItemText(IDC_EDT_KALIP_ADI, sKalip);
2269:     GetDlgItemText(IDC_EDT_PRES, sPres);
2270:     GetDlgItemText(IDC_EDT_MONTAJ, sMontaj);
2271:     GetDlgItemText(IDC_EDT_KALIPTA_IS, sKalipIs);
2272:     GetDlgItemText(IDC_EDT_TOPLAM_IS, sToplamIs);
2273:     
2274:     IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
2275:     if (!pDoc) return;
2276:     SetLongProp(pDoc, L"Unit", 3); // mm
2277:     
2278:     IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
2279:     if (pSel) {
2280:         long count = GetLongProp(pSel, L"Count");
2281:         if (count > 0) {
2282:             double left = GetDoubleProp(pSel, L"LeftX");
2283:             double top = GetDoubleProp(pSel, L"TopY");
2284:             
2285:             IDispatch* pLayer = GetDispatchProp(pDoc, L"ActiveLayer");
2286:             if (pLayer) {
2287:                 DISPID dispCreate; OLECHAR* szCreate = (OLECHAR*)L"CreateArtisticText";
2288:                 if (SUCCEEDED(pLayer->GetIDsOfNa
<truncated 2184 bytes>
es: %d"), nPres); AddLine(tmp, CLR_INVALID); }
2326:                     if (nMontaj > 0) { tmp.Format(_T("Montaj: %d"), nMontaj); AddLine(tmp, CLR_INVALID); }
2327:                     if (nKalipIs > 0) { tmp.Format(_T("Kalıptaki İş: %d"), nKalipIs); AddLine(tmp, CLR_INVALID); }
2328:                     if (nToplamIs > 0) { tmp.Format(_T("Toplam İş: %d"), nToplamIs); AddLine(tmp, CLR_INVALID); }
2329:                     
2330:                     if (m_bListOption && !m_listObjects.empty()) {
2331:                         // Benzersiz renk sayısını hesapla
2332:                         std::vector<COLORREF> uniqueClrs;
2333:                         for (size_t i = 0; i < m_listObjects.size(); i++) {
2334:                             COLORREF c = m_listObjects[i].color;
2335:                             if (std::find(uniqueClrs.begin(), uniqueClrs.end(), c) == uniqueClrs.end())
2336:                                 uniqueClrs.push_back(c);
2337:                         }
2338:                         CString header;
2339:                         header.Format(_T("%d Renk"), (int)uniqueClrs.size());
2340:                         AddLine(header, CLR_INVALID);
2341:                         
2342:                         for (size_t i = 0; i < m_listObjects.size(); i++) {
2343:                             CString tasAdi, tipAdi;
2344:                             if (i < m_combosTas.size() && ::IsWindow(m_combosTas[i]->GetSafeHwnd())) {
2345:                                 m_combosTas[i]->GetWindowText(tasAdi);
2346:                             }
2347:                             if (i < m_combosTip.size() && ::IsWindow(m_combosTip[i]->GetSafeHwnd())) {
2348:                                 m_combosTip[i]->GetWindowText(tipAdi);
2349:                             }
2350:                             
The above content does NOT show the entire file contents. If you need to view any lines of the file which were not shown to complete your task, call this tool again to view those lines.
