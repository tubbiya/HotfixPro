import io
import os
import re

files = [
    "rescued_code/SearchDlg_backup_1.cpp",
    "rescued_code/SearchDlg_backup_2.cpp",
    "rescued_code/SearchDlg_backup_3.cpp",
    "rescued_code/SearchDlg_backup_4.cpp",
    "rescued_code/SearchDlg_backup_5.cpp",
    "rescued_code/SearchDlg_backup_6.cpp",
    "rescued_code/SearchDlg_backup_7.cpp"
]

for fpath in files:
    if os.path.exists(fpath):
        print("FILE:", fpath)
        content = io.open(fpath, "r", encoding="utf-8", errors="ignore").read()
        lines = content.splitlines()
        cleaned_lines = []
        for line in lines:
            m = re.match(r"^\s*\d+:\s?(.*)$", line)
            if m:
                cleaned_lines.append(m.group(1))
            else:
                cleaned_lines.append(line)
        cleaned_content = "\n".join(cleaned_lines)
        
        funcs = re.findall(r"void CSearchDlg::(\w+)\((.*?)\)", cleaned_content)
        for name, args in funcs:
            print("  Found:", name, args)
