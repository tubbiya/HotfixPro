Created At: 2026-06-02T18:57:53Z
Completed At: 2026-06-02T18:57:53Z
File Path: `file:///c:/Users/tubbi/Dropbox/Claude/HotfixPro/SearchDlg.cpp`
Total Lines: 3170
Total Bytes: 138287
Showing lines 2170 to 2250
The following code has been modified to include a line number before every line, in the format: <line_number>: <original_line>. Please note that any changes targeting the original code should remove the line number, colon, and leading space.
2170:     
2171:     if (m_bListOpen && !m_listObjects.empty()) {
2172:         UpdateListLayout();
2173:     }
2174:     SetRedraw(TRUE);
2175:     Invalidate();
2176:     UpdateWindow();
2177: }
2178: 
2179: void CSearchDlg::OnBnClickedChkKalip()
2180: {
2181:     UpdateData(TRUE);
2182:     ShowKalipPanel(m_bKalip);
2183: }
2184: 
2185: void CSearchDlg::OnBnClickedBtnBilgiEkle()
2186: {
2187:     if (!m_pApp) return;
2188:     CCorelOptimizer optimizer(m_pApp, L"Ztrass Bilgi Ekle");
2189:     
2190:     CString sFirma, sDosya, sKalip, sPres, sMontaj, sKalipIs, sToplamIs;
2191:     GetDlgItemText(IDC_EDT_FIRMA, sFirma);
2192:     GetDlgItemText(IDC_EDT_DOSYA, sDosya);
2193:     GetDlgItemText(IDC_EDT_KALIP_ADI, sKalip);
2194:     GetDlgItemText(IDC_EDT_PRES, sPres);
2195:     GetDlgItemText(IDC_EDT_MONTAJ, sMontaj);
2196:     GetDlgItemText(IDC_EDT_KALIPTA_IS, sKalipIs);
2197:     GetDlgItemText(IDC_EDT_TOPLAM_IS, sToplamIs);
2198:     
2199:     IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
2200:     if (!pDoc) return;
2201:     SetLongProp(pDoc, L"Unit", 3); // mm
2202:     
2203:     IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
2204:     if (pSel) {
2205:         long count = GetLongProp(pSel, L"Count");
2206:         if (count > 0) {
2207:             double left = GetDoubleProp(pSel, L"LeftX");
2208:             double top = GetDoubleProp(pSel, L"TopY");
2209:             
2210:             IDispatch* pLayer = GetDispatchProp(pDoc, L"ActiveLayer");
2211:             if (pLayer) {
2212:
<truncated 831 bytes>
irma.IsEmpty()) AddLine(_T("Firma: ") + sFirma, CLR_INVALID);
2227:                     if (!sDosya.IsEmpty()) AddLine(_T("Dosya: ") + sDosya, CLR_INVALID);
2228:                     if (!sKalip.IsEmpty()) AddLine(_T("Kalıp Adı: ") + sKalip, CLR_INVALID);
2229:                     if (!sPres.IsEmpty()) AddLine(_T("Pres: ") + sPres, CLR_INVALID);
2230:                     if (!sMontaj.IsEmpty()) AddLine(_T("Montaj: ") + sMontaj, CLR_INVALID);
2231:                     if (!sKalipIs.IsEmpty()) AddLine(_T("Kalıptaki İş: ") + sKalipIs, CLR_INVALID);
2232:                     if (!sToplamIs.IsEmpty()) AddLine(_T("Toplam İş: ") + sToplamIs, CLR_INVALID);
2233:                     
2234:                     if (m_bListOption && !m_listObjects.empty()) {
2235:                         // Benzersiz renk sayısını hesapla
2236:                         std::vector<COLORREF> uniqueClrs;
2237:                         for (size_t i = 0; i < m_listObjects.size(); i++) {
2238:                             COLORREF c = m_listObjects[i].color;
2239:                             if (std::find(uniqueClrs.begin(), uniqueClrs.end(), c) == uniqueClrs.end())
2240:                                 uniqueClrs.push_back(c);
2241:                         }
2242:                         CString header;
2243:                         header.Format(_T("%d Renk"), (int)uniqueClrs.size());
2244:                         AddLine(header, CLR_INVALID);
2245:                         
2246:                         for (size_t i = 0; i < m_listObjects.size(); i++) {
2247:                             CString tasAdi, tipAdi;
2248:                             if (i < m_combosTas.size() && ::IsWindow(m_combosTas[i]->GetSafeHwnd())) {
2249:                                 m_combosTas[i]->GetWindowText(tasAdi);
2250:                             }
The above content does NOT show the entire file contents. If you need to view any lines of the file which were not shown to complete your task, call this tool again to view those lines.
