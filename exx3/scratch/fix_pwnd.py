import io

path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path, 'r', encoding='utf-8') as f:
    text = f.read()

# Replace CWnd* pWnd = GetWindow(GW_CHILD); with pWnd = GetWindow(GW_CHILD); in the second occurrence
# Wait, let's just find the exact block:
block_to_find = """    }
    CWnd* pWnd = GetWindow(GW_CHILD);
    while (pWnd) {"""

replacement = """    }
    pWnd = GetWindow(GW_CHILD);
    while (pWnd) {"""

# If it exists, replace only the last occurrence or just replace all because the first one is different?
# The first one is at line 609:
# CWnd* pWnd = GetWindow(GW_CHILD);
# while (pWnd) {
#    TCHAR szClass[32]; ...

# So I can just do:
text = text.replace(block_to_find, replacement)

with io.open(path, 'w', encoding='utf-8') as f:
    f.write(text)

print("Fixed pWnd redeclaration.")
