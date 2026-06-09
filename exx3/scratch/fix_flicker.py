import io

path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path, 'r', encoding='utf-8') as f:
    text = f.read()

old_start = """void CSearchDlg::OnBnClickedBtnList()
{
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Liste");
    UpdateData(TRUE);"""

new_start = """void CSearchDlg::OnBnClickedBtnList()
{
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Liste");
    SetRedraw(FALSE);
    UpdateData(TRUE);"""

text = text.replace(old_start, new_start)

old_list_close = """        CWnd* pGroup = GetDlgItem(IDC_GRP_LISTE);
        if (pGroup) pGroup->ShowWindow(SW_HIDE);
        m_listObjects.clear();
        Invalidate();
    }
    else {"""

new_list_close = """        CWnd* pGroup = GetDlgItem(IDC_GRP_LISTE);
        if (pGroup) pGroup->ShowWindow(SW_HIDE);
        m_listObjects.clear();
    }
    else {"""

text = text.replace(old_list_close, new_list_close)

old_list_open = """        // ComboBox'lari guncelle
        OnBnClickedChkListOption();
        Invalidate();
    }
}"""

new_list_open = """        // ComboBox'lari guncelle
        OnBnClickedChkListOption();
    }
    SetRedraw(TRUE);
    Invalidate();
    UpdateWindow();
}"""

text = text.replace(old_list_open, new_list_open)

with io.open(path, 'w', encoding='utf-8') as f:
    f.write(text)

print("SetRedraw logic added to prevent flickering.")
