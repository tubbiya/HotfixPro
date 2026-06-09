import io

path_cpp = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
path_h = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.h'

# 1. Fix SearchDlg.h
with io.open(path_h, 'r', encoding='utf-8') as f:
    text_h = f.read()

if "afx_msg void OnEnChangeEdtEn();" not in text_h:
    text_h = text_h.replace("afx_msg void OnBnClickedBtnExport();", 
                            "afx_msg void OnBnClickedBtnExport();\n    afx_msg void OnEnChangeEdtEn();")
    with io.open(path_h, 'w', encoding='utf-8') as f:
        f.write(text_h)

# 2. Fix braces in SearchDlg.cpp ResizeShapes
with io.open(path_cpp, 'r', encoding='utf-8') as f:
    text_cpp = f.read()

old_braces = """                    pShape->Release();
                }
            }
        }
        pSel->Release();
    }
    pDoc->Release();
}"""

new_braces = """                    pShape->Release();
                }
            }
            pFlatRange->Release();
        }
        pSel->Release();
    }
    pDoc->Release();
}"""

text_cpp = text_cpp.replace(old_braces, new_braces)

with io.open(path_cpp, 'w', encoding='utf-8') as f:
    f.write(text_cpp)

print("Compilation fixes applied")
