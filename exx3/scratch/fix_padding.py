import io

path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path, 'r', encoding='utf-8') as f:
    text = f.read()

old_func = """void CSearchDlg::UpdateListLayout()
{
    if (!m_bListOpen) return;
    
    int rowCount = (int)m_listObjects.size();
    if (rowCount == 0) return;
    
    int rowH = m_bListOption ? 24 : 18; // piksel
    int grupIcYukseklik = rowCount * rowH + 20;
    int minGrupH = 30;
    if (grupIcYukseklik < minGrupH) grupIcYukseklik = minGrupH;
    
    // Liste grup kutusunu yeniden boyutlandir
    CWnd* pGrp = GetDlgItem(IDC_GRP_LISTE);
    if (pGrp) {
        CRect rcGrp;
        pGrp->GetWindowRect(&rcGrp);
        ScreenToClient(&rcGrp);
        
        CRect rcBase(0, 0, 190, 0);
        CRect rcWide(0, 0, 208, 0);
        MapDialogRect(&rcBase);
        MapDialogRect(&rcWide);
        int baseWidth = rcBase.Width();
        int wideWidth = rcWide.Width();
        int newWidth = m_bListOption ? wideWidth : baseWidth; 
        
        pGrp->SetWindowPos(nullptr, rcGrp.left, rcGrp.top, newWidth, grupIcYukseklik, SWP_NOZORDER | SWP_NOACTIVATE);
    }
    
    // Diyalog yuksekligini liste altina gore ayarla
    CRect rcDlg;
    GetWindowRect(&rcDlg);
    CRect rcGrpFinal;
    if (pGrp) {
        pGrp->GetWindowRect(&rcGrpFinal);
        ScreenToClient(&rcGrpFinal);
        // Yukseklik = Liste kutu alti + 8px bosluk
        CRect rcThis;
        GetWindowRect(&rcThis);
        int newH = rcGrpFinal.bottom + 32; // + 24 for statusbar
        int screenH = GetSystemMetrics(SM_CYSCREEN) - 100;
        if (newH > screenH) newH = screenH; // Ekrana sigmama durumunda max yukseklik
        SetWindowPos(nullptr, 0, 0, rcThis.Width(), newH, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}"""

new_func = """void CSearchDlg::UpdateListLayout()
{
    if (!m_bListOpen) return;
    
    int rowCount = (int)m_listObjects.size();
    if (rowCount == 0) return;
    
    int rowH = m_bListOption ? 24 : 18; // piksel
    int grupIcYukseklik = rowCount * rowH + 16;
    int minGrupH = 30;
    if (grupIcYukseklik < minGrupH) grupIcYukseklik = minGrupH;
    
    // Liste grup kutusunu yeniden boyutlandir
    CWnd* pGrp = GetDlgItem(IDC_GRP_LISTE);
    if (pGrp) {
        CRect rcGrp;
        pGrp->GetWindowRect(&rcGrp);
        ScreenToClient(&rcGrp);
        
        CRect rcBase(0, 0, 160, 0);
        CRect rcWide(0, 0, 218, 0);
        MapDialogRect(&rcBase);
        MapDialogRect(&rcWide);
        int baseWidth = rcBase.Width();
        int wideWidth = rcWide.Width();
        int newWidth = m_bListOption ? wideWidth : baseWidth; 
        
        pGrp->SetWindowPos(nullptr, rcGrp.left, rcGrp.top, newWidth, grupIcYukseklik, SWP_NOZORDER | SWP_NOACTIVATE);
    }
    
    // Diyalog yuksekligini liste altina gore ayarla
    CRect rcGrpFinal;
    if (pGrp) {
        pGrp->GetWindowRect(&rcGrpFinal);
        ScreenToClient(&rcGrpFinal);
        
        // Dogru Client hesaplamasi: Grup kutusunun alti + 10px bosluk + 24px statusbar
        CRect rcClient(0, 0, 100, rcGrpFinal.bottom + 10 + 24);
        CalcWindowRect(&rcClient);
        int newH = rcClient.Height();
        
        int screenH = GetSystemMetrics(SM_CYSCREEN) - 100;
        if (newH > screenH) newH = screenH; // Ekrana sigmama durumunda max yukseklik
        
        CRect rcThis;
        GetWindowRect(&rcThis);
        SetWindowPos(nullptr, 0, 0, rcThis.Width(), newH, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}"""

if old_func in text:
    text = text.replace(old_func, new_func)
    with io.open(path, 'w', encoding='utf-8') as f:
        f.write(text)
    print("UpdateListLayout updated perfectly.")
else:
    print("old_func not found!")
