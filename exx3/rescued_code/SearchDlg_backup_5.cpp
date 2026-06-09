Created At: 2026-06-03T19:35:20Z
Completed At: 2026-06-03T19:35:20Z
File Path: `file:///c:/Users/tubbi/Dropbox/Claude/HotfixPro/SearchDlg.cpp`
Total Lines: 4058
Total Bytes: 187356
Showing lines 2250 to 2350
The following code has been modified to include a line number before every line, in the format: <line_number>: <original_line>. Please note that any changes targeting the original code should remove the line number, colon, and leading space.
2250:                 }
2251:                 pTip->SetCurSel(0);
2252:                 m_combosTip.push_back(pTip);
2253:             }
2254:         }
2255:     }
2256:     SetRedraw(TRUE);
2257:     Invalidate();
2258:     UpdateWindow();
2259: }
2260: 
2261: void CSearchDlg::OnBnClickedChkListOption()
2262: {
2263:     SetRedraw(FALSE);
2264:     UpdateData(TRUE);
2265:     
2266:     int nShow = m_bListOption ? SW_SHOW : SW_HIDE;
2267:     for (auto p : m_combosTas) if (::IsWindow(p->GetSafeHwnd())) p->ShowWindow(nShow);
2268:     for (auto p : m_combosTip) if (::IsWindow(p->GetSafeHwnd())) p->ShowWindow(nShow);
2269:     
2270:     if (m_bListOpen && !m_listObjects.empty()) {
2271:         UpdateListLayout();
2272:     }
2273:     SetRedraw(TRUE);
2274:     Invalidate();
2275:     UpdateWindow();
2276: }
2277: 
2278: void CSearchDlg::OnBnClickedChkKalip()
2279: {
2280:     UpdateData(TRUE);
2281:     ShowKalipPanel(m_bKalip);
2282: }
2283: 
2284: void CSearchDlg::OnBnClickedBtnBilgiEkle()
2285: {
2286:     if (!m_pApp) return;
2287:     CCorelOptimizer optimizer(m_pApp, L"Ztrass Bilgi Ekle");
2288:     
2289:     CString sFirma, sDosya, sKalip, sPres, sMontaj, sKalipIs, sToplamIs;
2290:     GetDlgItemText(IDC_EDT_FIRMA, sFirma);
2291:     GetDlgItemText(IDC_EDT_DOSYA, sDosya);
2292:     GetDlgItemText(IDC_EDT_KALIP_ADI, sKalip);
2293:     GetDlgItemText(IDC_EDT_PRES, sPres);
2294:     GetDlgItemText(IDC_EDT_MONTAJ, sMontaj);
2295:     GetDlgItemText(IDC_EDT_KALIPTA_IS, sKalipIs);
2296:     GetDlgItemText(IDC_EDT_TOPLAM_I
<truncated 1272 bytes>
;
2321:                         fullText += text;
2322:                         lineColors.push_back(color);
2323:                     };
2324:                     
2325:                     auto SafeParseInt = [](const CString& str) -> int {
2326:                         CString clean = str;
2327:                         clean.Trim();
2328:                         if (clean.IsEmpty()) return 0;
2329:                         int val = _ttoi(clean);
2330:                         if (val < 0) return 0;
2331:                         return val;
2332:                     };
2333: 
2334:                     int nPres = SafeParseInt(sPres);
2335:                     int nMontaj = SafeParseInt(sMontaj);
2336:                     int nKalipIs = SafeParseInt(sKalipIs);
2337:                     int nToplamIs = SafeParseInt(sToplamIs);
2338: 
2339:                     CString tmp;
2340:                     tmp.Format(_T("%d"), nPres); SetDlgItemText(IDC_EDT_PRES, tmp);
2341:                     tmp.Format(_T("%d"), nMontaj); SetDlgItemText(IDC_EDT_MONTAJ, tmp);
2342:                     tmp.Format(_T("%d"), nKalipIs); SetDlgItemText(IDC_EDT_KALIPTA_IS, tmp);
2343:                     tmp.Format(_T("%d"), nToplamIs); SetDlgItemText(IDC_EDT_TOPLAM_IS, tmp);
2344:                     
2345:                     if (!sDosya.IsEmpty()) AddLine(_T("Dosya: ") + sDosya, CLR_INVALID);
2346:                     if (!sFirma.IsEmpty()) AddLine(_T("Firma: ") + sFirma, CLR_INVALID);
2347:                     if (!sKalip.IsEmpty()) AddLine(_T("Kalıp Adı: ") + sKalip, CLR_INVALID);
2348:                     
2349:                     if (nPres > 0) { tmp.Format(_T("Pres: %d"), nPres); AddLine(tmp, CLR_INVALID); }
2350:                     if (nMontaj > 0) { tmp.Format(_T("Montaj: %d"), nMontaj); AddLine(tmp, CLR_INVALID); }
The above content does NOT show the entire file contents. If you need to view any lines of the file which were not shown to complete your task, call this tool again to view those lines.
