import io

path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path, 'r', encoding='utf-8') as f:
    text = f.read()

old_code = """    // Move Ayar elements overlapping to the Bilgi panel
    CRect rShift(0, 0, 105, 0);
    MapDialogRect(&rShift);
    int idsAyarMove[] = {"""

new_code = """    // Move Ayar elements overlapping to the Bilgi panel
    CRect rcBilgi, rcAyar2;
    GetDlgItem(IDC_GRP_KALIP_BILGI)->GetWindowRect(&rcBilgi);
    GetDlgItem(IDC_GRP_KALIP_DUZENLE_AYAR2)->GetWindowRect(&rcAyar2);
    ScreenToClient(&rcBilgi);
    ScreenToClient(&rcAyar2);
    int offsetX = rcAyar2.left - rcBilgi.left;
    int offsetY = rcAyar2.top - rcBilgi.top;

    int idsAyarMove[] = {"""

text = text.replace(old_code, new_code)

old_shift1 = """            rc.OffsetRect(-rShift.right, 0);
            p->SetWindowPos(nullptr, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);"""

new_shift1 = """            rc.OffsetRect(-offsetX, -offsetY);
            p->SetWindowPos(nullptr, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);"""

text = text.replace(old_shift1, new_shift1)

old_static_check = """            CRect rcAyar(342, 6, 441, 119); 
            MapDialogRect(&rcAyar);
            if (rcAyar.PtInRect(rc.CenterPoint())) {
                CString strText;
                pWnd->GetWindowText(strText);
                if (strText.Find(_T("mm")) != -1 && strText.Find(_T("Firma")) == -1 && strText.Find(_T("Dosya")) == -1) {
                    rc.OffsetRect(-rShift.right, 0);
                    pWnd->SetWindowPos(nullptr, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                }
            }"""

new_static_check = """            if (rcAyar2.PtInRect(rc.CenterPoint())) {
                CString strText;
                pWnd->GetWindowText(strText);
                if (strText.Find(_T("mm")) != -1 && strText.Find(_T("Firma")) == -1 && strText.Find(_T("Dosya")) == -1) {
                    rc.OffsetRect(-offsetX, -offsetY);
                    pWnd->SetWindowPos(nullptr, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                }
            }"""

text = text.replace(old_static_check, new_static_check)

with io.open(path, 'w', encoding='utf-8') as f:
    f.write(text)

print("Dynamic offset implemented.")
