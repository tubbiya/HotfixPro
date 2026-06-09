Created At: 2026-05-31T04:00:41Z
Completed At: 2026-05-31T04:00:41Z
File Path: `file:///c:/Users/tubbi/Dropbox/Claude/HotfixPro/SearchDlg.cpp`
Total Lines: 1539
Total Bytes: 55762
Showing lines 1500 to 1539
The following code has been modified to include a line number before every line, in the format: <line_number>: <original_line>. Please note that any changes targeting the original code should remove the line number, colon, and leading space.
1500:     }
1501:     Invalidate();
1502: }
1503: 
1504: void CSearchDlg::OnBnClickedChkKalip()
1505: {
1506:     UpdateData(TRUE);
1507:     ShowKalipPanel(m_bKalip);
1508: }
1509: 
1510: void CSearchDlg::OnBnClickedBtnBilgiEkle()
1511: {
1512:     // Kalip bilgi ekleme islemi - ileride implement edilecek
1513:     LogDebug(L"[BILGI] Bilgi Ekle clicked.");
1514: }
1515: 
1516: void CSearchDlg::OnBnClickedBtnKalipEkle()
1517: {
1518:     CString strOlcu;
1519:     GetDlgItemText(IDC_EDT_KALIP_OLCU, strOlcu);
1520:     LogDebug(L"[BILGI] Kalip Ekle: olcu=%s", (const WCHAR*)strOlcu);
1521: }
1522: 
1523: void CSearchDlg::OnBnClickedBtnIsaretTasi()
1524: {
1525:     CString strOlcu;
1526:     GetDlgItemText(IDC_EDT_TASI_OLCU, strOlcu);
1527:     LogDebug(L"[BILGI] Isaret Tasi Ekle: olcu=%s", (const WCHAR*)strOlcu);
1528: }
1529: 
1530: void CSearchDlg::OnBnClickedBtnKalipAyir()
1531: {
1532:     LogDebug(L"[BILGI] Kalip Ayir clicked.");
1533: }
1534: 
1535: void CSearchDlg::OnBnClickedBtnMakineKalibi()
1536: {
1537:     LogDebug(L"[BILGI] Makine Kalibi Hazirla clicked.");
1538: }
1539: 
The above content does NOT show the entire file contents. If you need to view any lines of the file which were not shown to complete your task, call this tool again to view those lines.
