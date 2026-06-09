import io

path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path, 'r', encoding='utf-8') as f:
    text = f.read()

# Replace the OnBnClickedChkListOption call in OnBnClickedBtnList with combo creation
old_btn_list_end = """        UpdateListLayout();
        
        // ComboBox'lari guncelle
        OnBnClickedChkListOption();
    }
    SetRedraw(TRUE);
    Invalidate();
    UpdateWindow();
}"""

new_btn_list_end = """        UpdateListLayout();
        
        ClearDynamicCombos();
        CWnd* pGroup2 = GetDlgItem(IDC_GRP_LISTE);
        if (pGroup2 && !m_listObjects.empty()) {
            CRect rcGroup;
            pGroup2->GetWindowRect(&rcGroup);
            ScreenToClient(&rcGroup);
            int startY = rcGroup.top + 14;
            int rowH = 24; // Kompakt satir yuksekligi
            
            DWORD dwStyle = WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL;
            if (m_bListOption) dwStyle |= WS_VISIBLE;
            
            for (size_t i = 0; i < m_listObjects.size(); ++i) {
                int rowY = startY + (int)i * rowH;
                CRect rcTas(rcGroup.left + 93, rowY, rcGroup.left + 203, rowY + 100);
                CRect rcTip(rcGroup.left + 208, rowY, rcGroup.left + 248, rowY + 80);
                
                CComboBox* pTas = new CComboBox();
                pTas->Create(dwStyle, rcTas, this, 2000 + (int)i);
                pTas->SetFont(GetFont());
                pTas->AddString(_T("Black Diamond"));
                pTas->AddString(_T("Hyacinth"));
                pTas->AddString(_T("Crystal"));
                pTas->AddString(_T("Siam"));
                pTas->SetCurSel(i % 2);
                m_combosTas.push_back(pTas);
                
                CComboBox* pTip = new CComboBox();
                pTip->Create(dwStyle, rcTip, this, 3000 + (int)i);
                pTip->SetFont(GetFont());
                pTip->AddString(_T("Dbl"));
                pTip->AddString(_T("Sgl"));
                pTip->SetCurSel(0);
                m_combosTip.push_back(pTip);
            }
        }
    }
    SetRedraw(TRUE);
    Invalidate();
    UpdateWindow();
}"""

text = text.replace(old_btn_list_end, new_btn_list_end)

# Replace the entire OnBnClickedChkListOption
import re
# We'll use regex to replace from void CSearchDlg::OnBnClickedChkListOption() to just before void CSearchDlg::OnBnClickedChkKalip()
old_chk_func_pattern = re.compile(r"void CSearchDlg::OnBnClickedChkListOption\(\)[\s\S]*?(?=void CSearchDlg::OnBnClickedChkKalip\(\))")

new_chk_func = """void CSearchDlg::OnBnClickedChkListOption()
{
    SetRedraw(FALSE);
    UpdateData(TRUE);
    
    int nShow = m_bListOption ? SW_SHOW : SW_HIDE;
    for (auto p : m_combosTas) if (::IsWindow(p->GetSafeHwnd())) p->ShowWindow(nShow);
    for (auto p : m_combosTip) if (::IsWindow(p->GetSafeHwnd())) p->ShowWindow(nShow);
    
    if (m_bListOpen && !m_listObjects.empty()) {
        UpdateListLayout();
    }
    SetRedraw(TRUE);
    Invalidate();
    UpdateWindow();
}

"""

text = old_chk_func_pattern.sub(new_chk_func, text)

with io.open(path, 'w', encoding='utf-8') as f:
    f.write(text)

print("Updated OnBnClickedChkListOption to toggle visibility without recreating.")
