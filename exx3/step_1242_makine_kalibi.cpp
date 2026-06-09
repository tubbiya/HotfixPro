Created At: 2026-06-03T14:02:56Z
Completed At: 2026-06-03T14:03:01Z
The following changes were made by the replace_file_content tool to: c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp. If relevant, proactively run terminal commands to execute this code for the USER. Don't ask for permission.
[diff_block_start]
@@ -3164,8 +3164,195 @@
 
 void CSearchDlg::OnBnClickedBtnMakineKalibi()
 {
+    if (!m_pApp) return;
     CCorelOptimizer optimizer(m_pApp, L"Ztrass Makine Kalibi");
-    LogDebug(L"[BILGI] Makine Kalibi Hazirla clicked.");
+    
+    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
+    if (!pDoc) return;
+    SetLongProp(pDoc, L"Unit", 3); // mm
+    
+    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
+    if (!pSel) { pDoc->Release(); return; }
+    long selCount = GetLongProp(pSel, L"Count");
+    if (selCount == 0) {
+        AfxMessageBox(_T("LÃ¼tfen makine kalÄ±bÄ± hazÄ±rlanacak kalÄ±bÄ± seÃ§in."));
+        pSel->Release(); pDoc->Release(); return;
+    }
+    
+    // UI'dan makine kalibi olculerini oku (mm cinsinden)
+    // originalMM -> machineMM eslemesi
+    struct SizeMap { double origMM; double machineMM; int editID; };
+    SizeMap sizeTable[] = {
+        {2.0, 0.0, IDC_EDT_2MM},
+        {3.0, 0.0, IDC_EDT_2MM2},
+        {4.0, 0.0, IDC_EDT_2MM3},
+        {5.0, 0.0, IDC_EDT_2MM4},
+        {6.0, 0.0, IDC_EDT_2MM5},
+        {7.0, 0.0, IDC_EDT_2MM6},
+        {8.0, 0.0, IDC_EDT_2MM7}
+    };
+    int tableSize = sizeof(sizeTable) / sizeof(sizeTable[0]);
+    
+    for (int i = 0; i < tableSize; i++) {
+        CString strVal;
+        GetDlgItemText(sizeTable[i].editID, strVal);
+        strVal.Replace(L",", L".");
+        double val = _wtof(strVal);
+        sizeTable[i].machineMM = val;
+    }
+    
+    // Minimum varyasyon boyutu
+    double minSize = 0.6;
+    
+    // Secimin boyutlarini al
+    double selH = GetDoubleProp(pSel, L"SizeHeight");
+    double selBottom = GetDoubleProp(pSel, L"Bott
<truncated 3931 bytes>
              }
+            }
+            pFill->Release();
+        }
+        
+        bool isCommon = (isBlackOutline && (isBlackFill || isNoFill)) || (!isBlackOutline && isBlackFill);
+        
+        if (isCommon) {
+            // Kalip cerceve / isaret tasi -> dokunma
+            pShape->Release();
+            continue;
+        }
+        
+        // Renkli tas -> boyutunu makine kalibi olcusune cevir
+        double origW = GetDoubleProp(pShape, L"SizeWidth");
+        double origH = GetDoubleProp(pShape, L"SizeHeight");
+        double cx = GetDoubleProp(pShape, L"CenterX");
+        double cy = GetDoubleProp(pShape, L"CenterY");
+        
+        // En yakin eslemeyi bul
+        double newSize = minSize; // varsayilan minimum
+        double bestDiff = 9999.0;
+        for (int t = 0; t < tableSize; t++) {
+            double diff = fabs(origW - sizeTable[t].origMM);
+            if (diff < bestDiff) {
+                bestDiff = diff;
+                if (sizeTable[t].machineMM > 0.01) {
+                    newSize = sizeTable[t].machineMM;
+                }
+            }
+        }
+        
+        // Minimum boyut kontrolu
+        if (newSize < minSize) newSize = minSize;
+        
+        // Yeniden boyutlandir (merkezi koruyarak)
+        SetDoubleProp(pShape, L"SizeWidth", newSize);
+        SetDoubleProp(pShape, L"SizeHeight", newSize);
+        SetDoubleProp(pShape, L"CenterX", cx);
+        SetDoubleProp(pShape, L"CenterY", cy);
+        
+        pShape->Release();
+    }
+    
+    // Kopyalanan range'i sec
+    InvokeMethodNoArgs(pDupRange, L"CreateSelection");
+    
+    pDupRange->Release();
+    pSel->Release();
+    pDoc->Release();
 }
 
 void CSearchDlg::OnBnClickedChkZtrass()
[diff_block_end]

Please note that the above snippet only shows the MODIFIED lines from the last change. It shows up to 3 lines of unchanged lines before and after the modified lines. The actual file contents may have many more lines not shown.