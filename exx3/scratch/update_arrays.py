import io

path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path, 'r', encoding='utf-8') as f:
    text = f.read()

# We need to replace the pTas->AddString block and pTip->AddString block.
old_block = """                CComboBox* pTas = new CComboBox();
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
                m_combosTip.push_back(pTip);"""

new_block = """                CComboBox* pTas = new CComboBox();
                pTas->Create(dwStyle, rcTas, this, 2000 + (int)i);
                pTas->SetFont(GetFont());
                const TCHAR* rsn[] = {
                    _T("Crystal"), _T("Amethyst"), _T("Black Diamond"), _T("Hyacinth"), _T("Aquamarine"), 
                    _T("Sapphire"), _T("Peridot"), _T("Light Sapphire"), _T("Sm. Topaz"), _T("Pink"), 
                    _T("Light Siam"), _T("Rainbow"), _T("Jet Black"), _T("Light Amethyst"), _T("Emerald"), 
                    _T("Jonquil"), _T("Siam"), _T("Topaz"), _T("CobaLt"), _T("Hematite"), 
                    _T("Gold Hematite"), _T("Ab Crystal"), _T("Olive"), _T("Rose Pink"), _T("Fuchsia"), 
                    _T("Blue Hematite"), _T("Light Brown"), _T("Rose Pink"), _T("Cosmojet"), _T("Light Topaz"), 
                    _T("Montana"), _T("Violet"), _T("Light Peach"), _T("Light Colorado Topaz"), _T("Silver Hematite"), 
                    _T("Citrine"), _T("Blue Zircon"), _T("Aquamarine Ab"), _T("Topaz Ab"), _T("Jet Ab"), 
                    _T("Sapphire Ab"), _T("Siam Ab"), _T("Hyacinth Ab"), _T("Peridot Ab"), _T("Nude"), 
                    _T("Light Black Diamond")
                };
                for (int j = 0; j < _countof(rsn); ++j) {
                    pTas->AddString(rsn[j]);
                }
                pTas->SetCurSel(0);
                m_combosTas.push_back(pTas);
                
                CComboBox* pTip = new CComboBox();
                pTip->Create(dwStyle, rcTip, this, 3000 + (int)i);
                pTip->SetFont(GetFont());
                const TCHAR* rst[] = {
                    _T("Dbl"), _T("Mc"), _T("Rzn"), _T("Oct"), _T("Drp"), _T("Düz"), _T("Özl")
                };
                for (int j = 0; j < _countof(rst); ++j) {
                    pTip->AddString(rst[j]);
                }
                pTip->SetCurSel(0);
                m_combosTip.push_back(pTip);"""

text = text.replace(old_block, new_block)

with io.open(path, 'w', encoding='utf-8') as f:
    f.write(text)

print("Combobox lists updated.")
