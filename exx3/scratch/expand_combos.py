import io
import re

path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path, 'r', encoding='utf-8') as f:
    text = f.read()

# 1. Update UpdateListLayout
old_layout = """void CSearchDlg::UpdateListLayout()
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
    }"""

new_layout = """void CSearchDlg::UpdateListLayout()
{
    if (!m_bListOpen) return;
    
    int rowCount = (int)m_listObjects.size();
    if (rowCount == 0) return;
    
    int rowH = m_bListOption ? 24 : 18 + 8; // piksel (boxH 18 + spacing 8)
    int gridRows = m_bListOption ? rowCount : (rowCount + 2) / 3;
    
    int grupIcYukseklik = gridRows * rowH + (m_bListOption ? 16 : 14);
    int minGrupH = 30;
    if (grupIcYukseklik < minGrupH) grupIcYukseklik = minGrupH;
    
    // Liste grup kutusunu yeniden boyutlandir
    CWnd* pGrp = GetDlgItem(IDC_GRP_LISTE);
    if (pGrp) {
        CRect rcGrp;
        pGrp->GetWindowRect(&rcGrp);
        ScreenToClient(&rcGrp);
        
        CRect rcWide(0, 0, 218, 0); // Always wide
        MapDialogRect(&rcWide);
        int newWidth = rcWide.Width(); 
        
        pGrp->SetWindowPos(nullptr, rcGrp.left, rcGrp.top, newWidth, grupIcYukseklik, SWP_NOZORDER | SWP_NOACTIVATE);
    }"""

text = text.replace(old_layout, new_layout)

# 2. Update OnPaint
old_paint1 = """                int startY = rcGroup.top + 14;
                int rowH = 24; // Sat?r yuksekligi (dropdownlar icin genisletildi)
                int boxW = 80; // Renk kutusu genisligi artirildi
                
                for (size_t i = 0; i < m_listObjects.size(); ++i) {
                    const auto& obj = m_listObjects[i];
                    int rowY = startY + (int)i * rowH;
                    
                    CRect rcBox(rcGroup.left + 8, rowY + 4, rcGroup.left + 8 + boxW, rowY + 20);"""

new_paint1 = """                int startY = rcGroup.top + 14;
                int rowH = 24; 
                double scale = rcGroup.Width() / 327.0;
                int boxW = (int)(90 * scale);
                
                for (size_t i = 0; i < m_listObjects.size(); ++i) {
                    const auto& obj = m_listObjects[i];
                    int rowY = startY + (int)i * rowH;
                    
                    CRect rcBox(rcGroup.left + 8, rowY + 4, rcGroup.left + 8 + boxW, rowY + 20);"""

text = text.replace(old_paint1, new_paint1)

old_paint2 = """                // Eski duzendeki gibi yan yana cizim
                int startX = rcGroup.left + 8;
                int startY = rcGroup.top + 18;
                int boxW = 110;
                int boxH = 18;
                int spacing = 8;"""

new_paint2 = """                // Yan yana 3 lu cizim
                int startX = rcGroup.left + 8;
                int startY = rcGroup.top + 18;
                double scale = rcGroup.Width() / 327.0;
                int spacing = (int)(6 * scale);
                int boxW = (rcGroup.Width() - 16 - 2 * spacing) / 3;
                int boxH = 18;
                int ySpacing = 8;"""

text = text.replace(old_paint2, new_paint2)

# Fix ySpacing in OnPaint loop
text = text.replace("startY += boxH + spacing;", "startY += boxH + ySpacing;")

# 3. Update Combobox positioning
old_combo = """            int startY = rcGroup.top + 14;
            int rowH = 24; // Kompakt satir yuksekligi
            
            DWORD dwStyle = WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL;
            if (m_bListOption) dwStyle |= WS_VISIBLE;
            
            for (size_t i = 0; i < m_listObjects.size(); ++i) {
                int rowY = startY + (int)i * rowH;
                CRect rcTas(rcGroup.left + 93, rowY, rcGroup.left + 203, rowY + 100);
                CRect rcTip(rcGroup.left + 208, rowY, rcGroup.left + 248, rowY + 80);"""

new_combo = """            int startY = rcGroup.top + 14;
            int rowH = 24; 
            
            double scale = rcGroup.Width() / 327.0;
            int wBox = (int)(90 * scale);
            int pad = (int)(6 * scale);
            int wTas = (int)(150 * scale);
            int wTip = (int)(55 * scale);
            
            DWORD dwStyle = WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL;
            if (m_bListOption) dwStyle |= WS_VISIBLE;
            
            for (size_t i = 0; i < m_listObjects.size(); ++i) {
                int rowY = startY + (int)i * rowH;
                int tasX = rcGroup.left + 8 + wBox + pad;
                int tipX = tasX + wTas + pad;
                CRect rcTas(tasX, rowY, tasX + wTas, rowY + 100);
                CRect rcTip(tipX, rowY, tipX + wTip, rowY + 80);"""

text = text.replace(old_combo, new_combo)

with io.open(path, 'w', encoding='utf-8') as f:
    f.write(text)

print("List elements dynamically widened.")
