import io
import re

path_cpp = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'

with io.open(path_cpp, 'r', encoding='utf-8') as f:
    text_cpp = f.read()

# 1. Update CompareColors
old_compare = """static bool CompareColors(IDispatch* pCol1, IDispatch* pCol2)
{
    if (!pCol1 || !pCol2) return false;
    long type1 = GetLongProp(pCol1, L"Type");
    long type2 = GetLongProp(pCol2, L"Type");
    if (type1 != type2) return false;

    if (type1 == 2) { // RGB
        long r1 = GetLongProp(pCol1, L"RGBRed");
        long g1 = GetLongProp(pCol1, L"RGBGreen");
        long b1 = GetLongProp(pCol1, L"RGBBlue");
        long r2 = GetLongProp(pCol2, L"RGBRed");
        long g2 = GetLongProp(pCol2, L"RGBGreen");
        long b2 = GetLongProp(pCol2, L"RGBBlue");
        return (r1 == r2 && g1 == g2 && b1 == b2);
    }
    else if (type1 == 3) { // CMYK
        long c1 = GetLongProp(pCol1, L"CMYKCyan");
        long m1 = GetLongProp(pCol1, L"CMYKMagenta");
        long y1 = GetLongProp(pCol1, L"CMYKYellow");
        long k1 = GetLongProp(pCol1, L"CMYKBlack");
        long c2 = GetLongProp(pCol2, L"CMYKCyan");
        long m2 = GetLongProp(pCol2, L"CMYKMagenta");
        long y2 = GetLongProp(pCol2, L"CMYKYellow");
        long k2 = GetLongProp(pCol2, L"CMYKBlack");
        return (c1 == c2 && m1 == m2 && y1 == y2 && k1 == k2);
    }
    return false;
}"""

new_compare = """static bool CompareColors(IDispatch* pCol1, IDispatch* pCol2)
{
    if (!pCol1 || !pCol2) return false;
    DISPID dispIsSame; OLECHAR* szIsSame = (OLECHAR*)L"IsSame";
    if (SUCCEEDED(pCol1->GetIDsOfNames(IID_NULL, &szIsSame, 1, LOCALE_USER_DEFAULT, &dispIsSame))) {
        VARIANT arg; VariantInit(&arg);
        arg.vt = VT_DISPATCH; arg.pdispVal = pCol2;
        DISPPARAMS params = { &arg, NULL, 1, 0 };
        VARIANT retVal; VariantInit(&retVal);
        if (SUCCEEDED(pCol1->Invoke(dispIsSame, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
            return (retVal.vt == VT_BOOL && retVal.boolVal == VARIANT_TRUE);
        }
    }
    return false;
}"""

text_cpp = text_cpp.replace(old_compare, new_compare)


# 2. Update GetColorCQL
old_get_color = """static CString GetColorCQL(IDispatch* pColor)
{
    if (!pColor) return _T("");
    long colorType = GetLongProp(pColor, L"Type");
    if (colorType == 2) { // RGB
        long r = GetLongProp(pColor, L"RGBRed");
        long g = GetLongProp(pColor, L"RGBGreen");
        long b = GetLongProp(pColor, L"RGBBlue");
        CString str;
        str.Format(_T("RGB(%d, %d, %d)"), r, g, b);
        return str;
    }
    else if (colorType == 3) { // CMYK
        long c = GetLongProp(pColor, L"CMYKCyan");
        long m = GetLongProp(pColor, L"CMYKMagenta");
        long y = GetLongProp(pColor, L"CMYKYellow");
        long k = GetLongProp(pColor, L"CMYKBlack");
        CString str;
        str.Format(_T("CMYK(%d, %d, %d, %d)"), c, m, y, k);
        return str;
    }
    return _T("");
}"""

new_get_color = """static CString GetColorCQL(IDispatch* pColor)
{
    if (!pColor) return _T("");
    DISPID dispToString; OLECHAR* szToString = (OLECHAR*)L"ToString";
    if (SUCCEEDED(pColor->GetIDsOfNames(IID_NULL, &szToString, 1, LOCALE_USER_DEFAULT, &dispToString))) {
        DISPPARAMS params = { NULL, NULL, 0, 0 };
        VARIANT retVal; VariantInit(&retVal);
        if (SUCCEEDED(pColor->Invoke(dispToString, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
            if (retVal.vt == VT_BSTR) {
                CString str(retVal.bstrVal);
                VariantClear(&retVal);
                return _T("'") + str + _T("'");
            }
        }
    }
    return _T("");
}"""

text_cpp = text_cpp.replace(old_get_color, new_get_color)

with io.open(path_cpp, 'w', encoding='utf-8') as f:
    f.write(text_cpp)

print("Patch applied to SearchDlg.cpp for Color operations")
