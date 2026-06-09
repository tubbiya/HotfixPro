import io

path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path, 'r', encoding='utf-8') as f:
    text = f.read()

# The block to move
block = """    m_bAyarOpen = FALSE;
    m_bCompactMode = FALSE;
    GetDlgItem(IDC_BTN_MAKINE_KALIBI2)->SetWindowText(L"Ayar");
    GetDlgItem(IDC_BTN_simge)->SetWindowText(L"-");
    // Move Ayar elements overlapping to the Bilgi panel
    CRect rcBilgi, rcAyar2;
    GetDlgItem(IDC_GRP_KALIP_BILGI)->GetWindowRect(&rcBilgi);
    GetDlgItem(IDC_GRP_KALIP_DUZENLE_AYAR2)->GetWindowRect(&rcAyar2);
    ScreenToClient(&rcBilgi);
    ScreenToClient(&rcAyar2);
    int offsetX = rcAyar2.left - rcBilgi.left;
    int offsetY = rcAyar2.top - rcBilgi.top;

    int idsAyarMove[] = {
        IDC_GRP_KALIP_DUZENLE_AYAR2, IDC_EDT_2MM, IDC_EDT_2MM2, IDC_EDT_2MM3, IDC_EDT_2MM4, IDC_EDT_2MM5, IDC_EDT_2MM6, IDC_EDT_2MM7,
        IDC_BTN_MAKINE_KALIBI3, IDC_BTN_MAKINE_KALIBI4, IDC_BTN_MAKINE_KALIBI5
    };
    for (int id : idsAyarMove) {
        CWnd* p = GetDlgItem(id);
        if (p) {
            CRect rc;
            p->GetWindowRect(&rc);
            ScreenToClient(&rc);
            rc.OffsetRect(-offsetX, -offsetY);
            p->SetWindowPos(nullptr, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        }
    }
    CWnd* pWnd = GetWindow(GW_CHILD);
    while (pWnd) {
        TCHAR szClass[32];
        ::GetClassName(pWnd->GetSafeHwnd(), szClass, 32);
        if (_wcsicmp(szClass, L"Static") == 0) {
            CRect rc;
            pWnd->GetWindowRect(&rc);
            ScreenToClient(&rc);
            if (rcAyar2.PtInRect(rc.CenterPoint())) {
                CString strText;
                pWnd->GetWindowText(strText);
                if (strText.Find(_T("mm")) != -1 && strText.Find(_T("Firma")) == -1 && strText.Find(_T("Dosya")) == -1) {
                    rc.OffsetRect(-offsetX, -offsetY);
                    pWnd->SetWindowPos(nullptr, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                }
            }
        }
        pWnd = pWnd->GetWindow(GW_HWNDNEXT);
    }

    
    CRect rcAdBoxDU(6, 10, 192, 22);
    MapDialogRect(&rcAdBoxDU);
    m_wndAdBox.Create(_T("Hotfix Pro - 0532 709 96 81"), WS_CHILD | SS_CENTERIMAGE | SS_CENTER, rcAdBoxDU, this, 3000);
    m_fontAd.CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
    m_wndAdBox.SetFont(&m_fontAd);"""

# Remove the block from its current location
text = text.replace(block, "")

# Insert the block at the end of OnInitDialog, just before `return TRUE;`
insert_target = "    return TRUE;\n}\n\nvoid CSearchDlg::OnBnClickedBtnOlcuOp()"
if insert_target in text:
    text = text.replace(insert_target, block + "\n" + insert_target)

with io.open(path, 'w', encoding='utf-8') as f:
    f.write(text)

print("Moved Ayar shift logic to OnInitDialog.")
