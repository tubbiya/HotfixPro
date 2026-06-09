# -*- coding: utf-8 -*-
import io

cpp_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(cpp_path, 'r', encoding='utf-8') as f:
    cpp_lines = f.readlines()

ctl_start = -1
ctl_end = -1
paint_start = -1
paint_end = -1

for i, line in enumerate(cpp_lines):
    if 'HBRUSH CSearchDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)' in line:
        ctl_start = i
    elif ctl_start != -1 and ctl_end == -1 and 'return hbr;' in line:
        ctl_end = i + 1
    
    if 'void CSearchDlg::OnPaint()' in line:
        paint_start = i
    elif paint_start != -1 and paint_end == -1 and '// 2. Renkli kutulari ciz' in line:
        paint_end = i

onctl_new = """HBRUSH CSearchDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    
    if (nCtlColor == CTLCOLOR_STATIC) {
        CRect rc;
        pWnd->GetWindowRect(&rc);
        CPoint pt = rc.CenterPoint();
        ScreenToClient(&pt);
        
        pDC->SetBkMode(TRANSPARENT);
        
        struct ColorGroup { int id; CBrush* pBrush; };
        ColorGroup groups[] = {
            { IDC_GRP_ARA, &m_brushAra },
            { IDC_GRP_OLCU, &m_brushOlcu },
            { IDC_GRP_KESISEN, &m_brushKesisen },
            { IDC_GRP_EXPORT, &m_brushExport },
            { IDC_GRP_LISTE, &m_brushListe },
            { IDC_GRP_KALIP_BILGI, &m_brushKalipBilgi },
            { IDC_GRP_KALIP_DUZENLE, &m_brushKalipDuzenle },
            { IDC_GRP_KALIP_DUZENLE_AYAR2, &m_brushKalipBilgi }
        };
        
        for (int i = 0; i < sizeof(groups)/sizeof(groups[0]); i++) {
            if (!m_bListOpen && groups[i].id == IDC_GRP_LISTE) continue;
            if (!m_bKalip && (groups[i].id == IDC_GRP_KALIP_BILGI || groups[i].id == IDC_GRP_KALIP_DUZENLE || groups[i].id == IDC_GRP_KALIP_DUZENLE_AYAR2)) continue;
            
            CWnd* pGrp = GetDlgItem(groups[i].id);
            if (pGrp && pGrp->IsWindowVisible()) {
                CRect rcGrp;
                pGrp->GetWindowRect(&rcGrp);
                ScreenToClient(&rcGrp);
                if (rcGrp.PtInRect(pt)) {
                    return (HBRUSH)groups[i].pBrush->GetSafeHandle();
                }
            }
        }
    }
    return hbr;
}

"""

onpaint_new = """void CSearchDlg::OnPaint()
{
    CPaintDC dc(this);
    
    // 1. Bolumleri pastel renklerle boya
    struct ColorGroup { int id; CBrush* pBrush; };
    ColorGroup groups[] = {
        { IDC_GRP_ARA, &m_brushAra },
        { IDC_GRP_OLCU, &m_brushOlcu },
        { IDC_GRP_KESISEN, &m_brushKesisen },
        { IDC_GRP_EXPORT, &m_brushExport },
        { IDC_GRP_LISTE, &m_brushListe },
        { IDC_GRP_KALIP_BILGI, &m_brushKalipBilgi },
        { IDC_GRP_KALIP_DUZENLE, &m_brushKalipDuzenle },
        { IDC_GRP_KALIP_DUZENLE_AYAR2, &m_brushKalipBilgi }
    };
    
    for (int i = 0; i < sizeof(groups)/sizeof(groups[0]); i++) {
        if (!m_bListOpen && groups[i].id == IDC_GRP_LISTE) continue;
        if (!m_bKalip && (groups[i].id == IDC_GRP_KALIP_BILGI || groups[i].id == IDC_GRP_KALIP_DUZENLE || groups[i].id == IDC_GRP_KALIP_DUZENLE_AYAR2)) continue;
        
        CWnd* pGrp = GetDlgItem(groups[i].id);
        if (pGrp && pGrp->IsWindowVisible()) {
            CRect rc;
            pGrp->GetWindowRect(&rc);
            ScreenToClient(&rc);
            dc.FillRect(&rc, groups[i].pBrush);
        }
    }
    
    """

new_lines = cpp_lines[:ctl_start] + [onctl_new] + cpp_lines[ctl_end:paint_start] + [onpaint_new] + cpp_lines[paint_end:]

with io.open(cpp_path, 'w', encoding='utf-8') as f:
    f.writelines(new_lines)

print("Dynamic background coloring implemented.")
