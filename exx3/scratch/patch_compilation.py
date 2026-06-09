import io
import re

path_cpp = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path_cpp, 'r', encoding='utf-8', errors='ignore') as f:
    text_cpp = f.read()

# 1. Include afxdlgs.h
if "#include <afxdlgs.h>" not in text_cpp:
    text_cpp = text_cpp.replace('#include "SearchDlg.h"', '#include "SearchDlg.h"\n#include <afxdlgs.h>')

# 2. Add SetDoubleProp
set_double_prop = """static bool SetDoubleProp(IDispatch* pDisp, WCHAR* szProp, double value)
{
    if (!pDisp) return false;
    DISPID dispid;
    if (SUCCEEDED(pDisp->GetIDsOfNames(IID_NULL, &szProp, 1, LOCALE_USER_DEFAULT, &dispid))) {
        VARIANT arg;
        VariantInit(&arg);
        arg.vt = VT_R8;
        arg.dblVal = value;
        DISPID putid = DISPID_PROPERTYPUT;
        DISPPARAMS params = { &arg, &putid, 1, 1 };
        HRESULT hr = pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &params, NULL, NULL, NULL);
        return SUCCEEDED(hr);
    }
    return false;
}

"""
if "static bool SetDoubleProp" not in text_cpp:
    text_cpp = text_cpp.replace("static bool SetLongProp", set_double_prop + "static bool SetLongProp")

with io.open(path_cpp, 'w', encoding='utf-8') as f:
    f.write(text_cpp)

print("Compilation errors patched.")
