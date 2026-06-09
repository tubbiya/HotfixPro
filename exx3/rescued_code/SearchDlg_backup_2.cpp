Created At: 2026-06-02T15:19:36Z
Completed At: 2026-06-02T15:19:36Z
File Path: `file:///c:/Users/tubbi/Dropbox/Claude/HotfixPro/SearchDlg.cpp`
Total Lines: 2719
Total Bytes: 111969
Showing lines 2160 to 2220
The following code has been modified to include a line number before every line, in the format: <line_number>: <original_line>. Please note that any changes targeting the original code should remove the line number, colon, and leading space.
2160: {
2161:     SetRedraw(FALSE);
2162:     UpdateData(TRUE);
2163:     
2164:     int nShow = m_bListOption ? SW_SHOW : SW_HIDE;
2165:     for (auto p : m_combosTas) if (::IsWindow(p->GetSafeHwnd())) p->ShowWindow(nShow);
2166:     for (auto p : m_combosTip) if (::IsWindow(p->GetSafeHwnd())) p->ShowWindow(nShow);
2167:     
2168:     if (m_bListOpen && !m_listObjects.empty()) {
2169:         UpdateListLayout();
2170:     }
2171:     SetRedraw(TRUE);
2172:     Invalidate();
2173:     UpdateWindow();
2174: }
2175: 
2176: void CSearchDlg::OnBnClickedChkKalip()
2177: {
2178:     UpdateData(TRUE);
2179:     ShowKalipPanel(m_bKalip);
2180: }
2181: 
2182: void CSearchDlg::OnBnClickedBtnBilgiEkle()
2183: {
2184:     CCorelOptimizer optimizer(m_pApp, L"Ztrass Bilgi Ekle");
2185:     // Kalip bilgi ekleme islemi - ileride implement edilecek
2186:     LogDebug(L"[BILGI] Bilgi Ekle clicked.");
2187: }
2188: 
2189: void CSearchDlg::OnBnClickedBtnKalipEkle() // Isaret Tasi (KalipTasM)
2190: {
2191:     if (!m_pApp) return;
2192:     CCorelOptimizer optimizer(m_pApp, L"Ztrass Isaret Tasi");
2193:     
2194:     CString strTasMM;
2195:     GetDlgItemText(IDC_EDT_KALIP_OLCU, strTasMM);
2196:     strTasMM.Replace(L",", L".");
2197:     double tasMM = _wtof(strTasMM);
2198:     if (tasMM <= 0) tasMM = 2.0;
2199:     
2200:     IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
2201:     if (!pDoc) return;
2202:     SetLongProp(pDoc, L"Unit", 3);
2203:     
2204:     IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
2205:     if (pSel) {
2206:         long count = GetLongProp(pSel, L"Count");
2207:         if (count > 0) {
2208:             double left = GetDoubleProp(pSel, L"LeftX");
2209:             double right = GetDoubleProp(pSel, L"RightX");
2210:             double bottom = GetDoubleProp(pSel, L"BottomY");
2211:             double top = GetDoubleProp(pSel, L"TopY");
2212:             
2213:             double minW = 9999.0;
2214:             IDispatch* pPage = GetDispatchProp(pDoc, L"ActivePage");
2215:             if (pPage) {
2216:                 DISPID dispSelect; OLECHAR* szSelect = (OLECHAR*)L"SelectShapesFromRectangle";
2217:                 if (SUCCEEDED(pPage->GetIDsOfNames(IID_NULL, &szSelect, 1, LOCALE_USER_DEFAULT, &dispSelect))) {
2218:                     VARIANT args[5]; for(int i=0; i<5; ++i) VariantInit(&args[i]);
2219:                     args[4].vt = VT_R8; args[4].dblVal = left - 5.0; // Left
2220:                     args[3].vt = VT_R8; args[3].dblVal = top + 5.0; // Top
The above content does NOT show the entire file contents. If you need to view any lines of the file which were not shown to complete your task, call this tool again to view those lines.
