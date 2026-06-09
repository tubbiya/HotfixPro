# -*- coding: utf-8 -*-
import os, codecs

def search_200():
    filepath = "SearchDlg.cpp"
    if not os.path.exists(filepath):
        return "SearchDlg.cpp not found"
        
    results = []
    # Read as UTF-8
    try:
        with open(filepath, 'r', encoding='utf-8-sig') as f:
            content = f.read()
    except Exception as e:
        return f"Error reading file: {e}"
        
    lines = content.splitlines()
    for i, line in enumerate(lines, 1):
        if "200" in line:
            results.append(f"Line {i}: {line}")
            
    with open("search_results.txt", "w", encoding="utf-8") as f:
        f.write("\n".join(results))
    return f"Done: {len(results)} occurrences of 200 found"

search_200()
