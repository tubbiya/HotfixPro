# -*- coding: utf-8 -*-
import io

cpp_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(cpp_path, 'r', encoding='utf-8') as f:
    cpp_content = f.read()

old_begin = """        InvokeMethod(pDoc, L"BeginCommandGroup", &arg, 1);"""
new_begin = """        WCHAR* szCmd = (WCHAR*)L"BeginCommandGroup";
        DISPID dispid;
        if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szCmd, 1, LOCALE_USER_DEFAULT, &dispid))) {
            DISPPARAMS params = { &arg, nullptr, 1, 0 };
            pDoc->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, nullptr, nullptr, nullptr);
        }"""
cpp_content = cpp_content.replace(old_begin, new_begin)

old_end = """        InvokeMethod(pDoc, L"EndCommandGroup", nullptr, 0);"""
new_end = """        InvokeMethodNoArgs(pDoc, (WCHAR*)L"EndCommandGroup");"""
cpp_content = cpp_content.replace(old_end, new_end)

old_refresh = """            InvokeMethod(pWin, L"Refresh", nullptr, 0);"""
new_refresh = """            InvokeMethodNoArgs(pWin, (WCHAR*)L"Refresh");"""
cpp_content = cpp_content.replace(old_refresh, new_refresh)

with io.open(cpp_path, 'w', encoding='utf-8') as f:
    f.write(cpp_content)

print("Invoke logic fixed!")
