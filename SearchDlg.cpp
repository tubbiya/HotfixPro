#include "SearchDlg.h"
#include <afxdlgs.h>
#include <thread>
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <stdarg.h>

#include <shlobj.h>

static CString GetSettingsFilePath()
{
    TCHAR szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath))) {
        CString folder = CString(szPath) + _T("\\ztrass");
        CreateDirectory(folder, NULL);
        return folder + _T("\\settings.ini");
    }
    return _T("");
}

// UTF-8 Formatında Loglama Yapan Yardımcı Fonksiyon
static HRESULT InvokeMethodNoArgsRet(IDispatch* pDisp, WCHAR* szMethod, VARIANT* pRetVal)
{
    if (!pDisp) return E_POINTER;
    DISPID dispid;
    HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &szMethod, 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hr)) return hr;
    DISPPARAMS params = { NULL, NULL, 0, 0 };
    return pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, pRetVal, NULL, NULL);
}

static void LogDebug(const WCHAR* szFormat, ...)
{
    return; // GECICI OLARAK DEVRE DISI
    
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    WCHAR szTime[64];
    swprintf_s(szTime, L"[%02d.%02d.%04d %02d:%02d:%02d.%03d] ", 
        st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    va_list args;
    va_start(args, szFormat);
    
    WCHAR szMessage[2048];
    vswprintf_s(szMessage, szFormat, args);
    va_end(args);

    FILE* f = nullptr;
    if (_wfopen_s(&f, L"c:\\Users\\tubbi\\Dropbox\\Claude\\Ztrass\\debug_log.txt", L"a, ccs=UTF-8") == 0 && f) {
        fwprintf_s(f, L"%s%s\n", szTime, szMessage);
        fclose(f);
    }
}

static void SetColorRGB(IDispatch* pColor, int r, int g, int b)
{
    if (!pColor) return;
    DISPID dispid;
    WCHAR* szMethod = L"RGBAssign";
    HRESULT hr = pColor->GetIDsOfNames(IID_NULL, &szMethod, 1, LOCALE_USER_DEFAULT, &dispid);
    if (SUCCEEDED(hr)) {
        VARIANT vars[3];
        VariantInit(&vars[0]);
        vars[0].lVal = b;
        vars[0].vt = VT_I4;

        VariantInit(&vars[1]);
        vars[1].lVal = g;
        vars[1].vt = VT_I4;

        VariantInit(&vars[2]);
        vars[2].lVal = r;
        vars[2].vt = VT_I4;

        DISPPARAMS params = { vars, NULL, 3, 0 };
        pColor->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, NULL, NULL, NULL);
    }
}

// COM Late Binding Yardımcı Fonksiyonları (Log Destekli)
static HRESULT InvokeMethod(IDispatch* pDisp, WCHAR* szMethod, VARIANT* pArgs, int cArgs)
{
    if (!pDisp) return E_POINTER;
    DISPID dispid;
    HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &szMethod, 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hr)) return hr;
    DISPPARAMS params = { pArgs, NULL, (UINT)cArgs, 0 };
    return pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, NULL, NULL, NULL);
}

static HRESULT InvokeMethodNoArgs(IDispatch* pDisp, WCHAR* szMethod)
{
    if (!pDisp) {
        LogDebug(L"[HATA] InvokeMethodNoArgs: pDisp NULL! (Metod: %s)", szMethod);
        return E_POINTER;
    }
    DISPID dispid;
    HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &szMethod, 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hr)) {
        LogDebug(L"[HATA] InvokeMethodNoArgs: GetIDsOfNames (Metod: %s) basarisiz! hr: 0x%08X", szMethod, hr);
        return hr;
    }

    DISPPARAMS params = { NULL, NULL, 0, 0 };
    hr = pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, NULL, NULL, NULL);
    if (FAILED(hr)) {
        LogDebug(L"[HATA] InvokeMethodNoArgs: Invoke (Metod: %s) basarisiz! hr: 0x%08X", szMethod, hr);
    }
    return hr;
}

static IDispatch* GetDispatchProp(IDispatch* pDisp, WCHAR* szProp)
{
    if (!pDisp) {
        LogDebug(L"[HATA] GetDispatchProp: Kaynak nesne (pDisp) NULL! (Prop: %s)", szProp);
        return nullptr;
    }
    DISPID dispid;
    HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &szProp, 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hr)) {
        LogDebug(L"[HATA] GetDispatchProp: GetIDsOfNames basarisiz! (Prop: %s, hr: 0x%08X)", szProp, hr);
        return nullptr;
    }

    DISPPARAMS params = { NULL, NULL, 0, 0 };
    VARIANT varResult;
    VariantInit(&varResult);
    hr = pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &varResult, NULL, NULL);
    if (FAILED(hr)) {
        LogDebug(L"[HATA] GetDispatchProp: Invoke (PROPERTYGET) basarisiz! (Prop: %s, hr: 0x%08X)", szProp, hr);
        return nullptr;
    }
    if (varResult.vt != VT_DISPATCH) {
        LogDebug(L"[HATA] GetDispatchProp: Donen deger IDispatch tipinde degil! (Prop: %s, vt: %d)", szProp, varResult.vt);
        VariantClear(&varResult);
        return nullptr;
    }

    return varResult.pdispVal;
}

static double GetDoubleProp(IDispatch* pDisp, WCHAR* szProp)
{
    if (!pDisp) {
        LogDebug(L"[HATA] GetDoubleProp: Kaynak nesne (pDisp) NULL! (Prop: %s)", szProp);
        return 0.0;
    }
    DISPID dispid;
    HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &szProp, 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hr)) {
        LogDebug(L"[HATA] GetDoubleProp: GetIDsOfNames basarisiz! (Prop: %s, hr: 0x%08X)", szProp, hr);
        return 0.0;
    }

    DISPPARAMS params = { NULL, NULL, 0, 0 };
    VARIANT varResult;
    VariantInit(&varResult);
    hr = pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &varResult, NULL, NULL);
    if (FAILED(hr)) {
        LogDebug(L"[HATA] GetDoubleProp: Invoke (PROPERTYGET) basarisiz! (Prop: %s, hr: 0x%08X)", szProp, hr);
        return 0.0;
    }

    VARIANT varDouble;
    VariantInit(&varDouble);
    hr = VariantChangeType(&varDouble, &varResult, 0, VT_R8);
    double val = 0.0;
    if (SUCCEEDED(hr)) {
        val = varDouble.dblVal;
    } else {
        LogDebug(L"[HATA] GetDoubleProp: VariantChangeType (VT_R8) basarisiz! (Prop: %s, hr: 0x%08X)", szProp, hr);
    }
    VariantClear(&varResult);
    VariantClear(&varDouble);
    return val;
}

static long GetLongProp(IDispatch* pDisp, WCHAR* szProp)
{
    if (!pDisp) {
        LogDebug(L"[HATA] GetLongProp: Kaynak nesne (pDisp) NULL! (Prop: %s)", szProp);
        return 0;
    }
    DISPID dispid;
    HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &szProp, 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hr)) {
        LogDebug(L"[HATA] GetLongProp: GetIDsOfNames basarisiz! (Prop: %s, hr: 0x%08X)", szProp, hr);
        return 0;
    }

    DISPPARAMS params = { NULL, NULL, 0, 0 };
    VARIANT varResult;
    VariantInit(&varResult);
    hr = pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &varResult, NULL, NULL);
    if (FAILED(hr)) {
        LogDebug(L"[HATA] GetLongProp: Invoke (PROPERTYGET) basarisiz! (Prop: %s, hr: 0x%08X)", szProp, hr);
        return 0;
    }

    VARIANT varLong;
    VariantInit(&varLong);
    hr = VariantChangeType(&varLong, &varResult, 0, VT_I4);
    long val = 0;
    if (SUCCEEDED(hr)) {
        val = varLong.lVal;
    } else {
        LogDebug(L"[HATA] GetLongProp: VariantChangeType (VT_I4) basarisiz! (Prop: %s, hr: 0x%08X)", szProp, hr);
    }
    VariantClear(&varResult);
    VariantClear(&varLong);
    return val;
}


static bool SetBoolProp(IDispatch* pDisp, WCHAR* szProp, bool bValue)
{
    if (!pDisp) return false;
    DISPID dispid;
    if (FAILED(pDisp->GetIDsOfNames(IID_NULL, &szProp, 1, LOCALE_USER_DEFAULT, &dispid))) return false;
    VARIANT arg;
    VariantInit(&arg);
    arg.vt = VT_BOOL;
    arg.boolVal = bValue ? VARIANT_TRUE : VARIANT_FALSE;
    DISPID mydispid = DISPID_PROPERTYPUT;
    DISPPARAMS params = { &arg, &mydispid, 1, 1 };
    HRESULT hr = pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &params, nullptr, nullptr, nullptr);
    return SUCCEEDED(hr);
}

static bool GetBoolProp(IDispatch* pDisp, WCHAR* szProp)
{
    if (!pDisp) return false;
    DISPID dispid;
    HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &szProp, 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hr)) return false;

    DISPPARAMS params = { NULL, NULL, 0, 0 };
    VARIANT varResult;
    VariantInit(&varResult);
    hr = pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &varResult, NULL, NULL);
    if (FAILED(hr)) return false;

    VARIANT varBool;
    VariantInit(&varBool);
    hr = VariantChangeType(&varBool, &varResult, 0, VT_BOOL);
    bool val = false;
    if (SUCCEEDED(hr)) {
        val = (varBool.boolVal != VARIANT_FALSE);
    }
    VariantClear(&varResult);
    VariantClear(&varBool);
    return val;
}

static CString GetStringProp(IDispatch* pDisp, WCHAR* szProp)
{
    CString result;
    if (!pDisp) return result;
    DISPID dispid;
    if (FAILED(pDisp->GetIDsOfNames(IID_NULL, &szProp, 1, LOCALE_USER_DEFAULT, &dispid))) return result;

    DISPPARAMS params = { NULL, NULL, 0, 0 };
    VARIANT varResult;
    VariantInit(&varResult);
    if (FAILED(pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &varResult, NULL, NULL))) return result;

    if (varResult.vt == VT_BSTR && varResult.bstrVal) {
        result = varResult.bstrVal;
    } else {
        VARIANT varDest;
        VariantInit(&varDest);
        if (SUCCEEDED(VariantChangeType(&varDest, &varResult, 0, VT_BSTR))) {
            result = varDest.bstrVal;
            VariantClear(&varDest);
        }
    }
    VariantClear(&varResult);
    return result;
}

static void OptimizeCorel(IDispatch* pApp, BOOL bUse, const WCHAR* szName)
{
    if (!pApp) return;
    
    IDispatch* pDoc = GetDispatchProp(pApp, L"ActiveDocument");
    if (!pDoc) return;
    
    if (bUse) {
        VARIANT arg;
        VariantInit(&arg);
        arg.vt = VT_BSTR;
        arg.bstrVal = SysAllocString(szName);
        InvokeMethod(pDoc, L"BeginCommandGroup", &arg, 1);
        SysFreeString(arg.bstrVal);
        
        SetBoolProp(pApp, L"Optimization", true);
        SetBoolProp(pApp, L"EventsEnabled", false);
    } else {
        SetBoolProp(pApp, L"Optimization", false);
        SetBoolProp(pApp, L"EventsEnabled", true);
        
        InvokeMethod(pDoc, L"EndCommandGroup", nullptr, 0);
        
        IDispatch* pWin = GetDispatchProp(pApp, L"ActiveWindow");
        if (pWin) {
            InvokeMethod(pWin, L"Refresh", nullptr, 0);
            pWin->Release();
        }
    }
    pDoc->Release();
}

class CCorelOptimizer {
    IDispatch* m_pApp;
public:
    CCorelOptimizer(IDispatch* pApp, const WCHAR* szName) : m_pApp(pApp) {
        OptimizeCorel(m_pApp, TRUE, szName);
    }
    ~CCorelOptimizer() {
        OptimizeCorel(m_pApp, FALSE, L"");
    }
};

static bool SetDoubleProp(IDispatch* pDisp, WCHAR* szProp, double value)
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

static bool SetStringProp(IDispatch* pDisp, WCHAR* szProp, WCHAR* value)
{
    if (!pDisp) return false;
    DISPID dispid;
    if (SUCCEEDED(pDisp->GetIDsOfNames(IID_NULL, &szProp, 1, LOCALE_USER_DEFAULT, &dispid))) {
        VARIANT arg;
        VariantInit(&arg);
        arg.vt = VT_BSTR;
        arg.bstrVal = SysAllocString(value);
        DISPID putid = DISPID_PROPERTYPUT;
        DISPPARAMS params = { &arg, &putid, 1, 1 };
        HRESULT hr = pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &params, NULL, NULL, NULL);
        SysFreeString(arg.bstrVal);
        return SUCCEEDED(hr);
    }
    return false;
}

static bool SetLongProp(IDispatch* pDisp, WCHAR* szProp, long value)
{
    if (!pDisp) {
        LogDebug(L"[HATA] SetLongProp: Kaynak nesne (pDisp) NULL! (Prop: %s)", szProp);
        return false;
    }
    DISPID dispid;
    HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &szProp, 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hr)) {
        LogDebug(L"[HATA] SetLongProp: GetIDsOfNames basarisiz! (Prop: %s, hr: 0x%08X)", szProp, hr);
        return false;
    }

    DISPID dispidNamed = DISPID_PROPERTYPUT;
    VARIANT varArg;
    VariantInit(&varArg);
    varArg.vt = VT_I4;
    varArg.lVal = value;

    DISPPARAMS params = { &varArg, &dispidNamed, 1, 1 };
    hr = pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &params, NULL, NULL, NULL);
    if (FAILED(hr)) {
        LogDebug(L"[HATA] SetLongProp: Invoke (PROPERTYPUT) basarisiz! (Prop: %s, hr: 0x%08X)", szProp, hr);
        return false;
    }
    return true;
}

static CString CallToStringMethod(IDispatch* pDisp)
{
    if (!pDisp) return _T("");
    DISPID dispid;
    WCHAR* szMethod = L"ToString";
    HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &szMethod, 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hr)) {
        LogDebug(L"[HATA] CallToStringMethod: GetIDsOfNames (ToString) basarisiz! hr: 0x%08X", hr);
        return _T("");
    }

    DISPPARAMS params = { NULL, NULL, 0, 0 };
    VARIANT varResult;
    VariantInit(&varResult);
    hr = pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &varResult, NULL, NULL);
    if (FAILED(hr)) {
        LogDebug(L"[HATA] CallToStringMethod: Invoke (ToString) basarisiz! hr: 0x%08X", hr);
        return _T("");
    }
    if (varResult.vt != VT_BSTR) {
        LogDebug(L"[HATA] CallToStringMethod: ToString donen degeri VT_BSTR degil! vt: %d", varResult.vt);
        VariantClear(&varResult);
        return _T("");
    }

    CString strResult(varResult.bstrVal);
    VariantClear(&varResult);
    return strResult;
}

static CString GetColorCQL(IDispatch* pColor)
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
    // Fallback: Eski dize formatı
    return _T("'") + CallToStringMethod(pColor) + _T("'");
}


static IDispatch* FindShapes(IDispatch* pDisp, CString strQuery, long shapeType, bool bRecursive)
{
    if (!pDisp) {
        LogDebug(L"[HATA] FindShapes: pDisp NULL!");
        return nullptr;
    }
    DISPID dispid;
    WCHAR* szMethod = L"FindShapes";
    HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &szMethod, 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hr)) {
        LogDebug(L"[HATA] FindShapes: GetIDsOfNames (FindShapes) basarisiz! hr: 0x%08X", hr);
        return nullptr;
    }

    VARIANT vars[3];
    VariantInit(&vars[0]);
    vars[0].boolVal = bRecursive ? VARIANT_TRUE : VARIANT_FALSE;
    vars[0].vt = VT_BOOL;

    VariantInit(&vars[1]);
    vars[1].lVal = shapeType;
    vars[1].vt = VT_I4;

    VariantInit(&vars[2]);
    vars[2].bstrVal = strQuery.AllocSysString();
    vars[2].vt = VT_BSTR;

    DISPPARAMS params = { vars, NULL, 3, 0 };
    VARIANT varResult;
    VariantInit(&varResult);
    
    LogDebug(L"[BILGI] FindShapes cagrisi yapiliyor. Sorgu: %s, Tip: %d, Recursive: %d", (const WCHAR*)strQuery, shapeType, bRecursive);
    hr = pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &varResult, NULL, NULL);
    
    SysFreeString(vars[2].bstrVal);
    
    if (FAILED(hr)) {
        LogDebug(L"[HATA] FindShapes: Invoke basarisiz! hr: 0x%08X", hr);
        return nullptr;
    }
    if (varResult.vt != VT_DISPATCH) {
        LogDebug(L"[HATA] FindShapes: Donen deger VT_DISPATCH degil! vt: %d", varResult.vt);
        VariantClear(&varResult);
        return nullptr;
    }
    return varResult.pdispVal;
}

IMPLEMENT_DYNAMIC(CSearchDlg, CDialog)

static IDispatch* GetDispatchPropWithIntArg(IDispatch* pDisp, WCHAR* szProp, int arg)
{
    if (!pDisp) return nullptr;
    DISPID dispid;
    HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &szProp, 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hr)) return nullptr;

    VARIANT varArg;
    VariantInit(&varArg);
    varArg.vt = VT_I4;
    varArg.lVal = arg;

    DISPPARAMS params = { &varArg, NULL, 1, 0 };
    VARIANT varResult;
    VariantInit(&varResult);
    hr = pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET | DISPATCH_METHOD, &params, &varResult, NULL, NULL);
    if (FAILED(hr)) return nullptr;
    if (varResult.vt != VT_DISPATCH) {
        VariantClear(&varResult);
        return nullptr;
    }
    return varResult.pdispVal;
}

static COLORREF GetColorRef(IDispatch* pColor)
{
    if (!pColor) return RGB(128,128,128);
    InvokeMethodNoArgs(pColor, L"ConvertToRGB");
    long r = GetLongProp(pColor, L"RGBRed");
    long g = GetLongProp(pColor, L"RGBGreen");
    long b = GetLongProp(pColor, L"RGBBlue");
    return RGB(r, g, b);
}

static bool IsColorBlack(COLORREF col)
{
    if (col == CLR_INVALID) return false;
    return (GetRValue(col) < 15 && GetGValue(col) < 15 && GetBValue(col) < 15);
}

static CString FormatCm(double val_mm)
{
    double val_cm = val_mm / 10.0;
    CString str;
    str.Format(_T("%.1f"), val_cm);
    str.Replace(_T(","), _T(".")); // Once noktaya esitleyelim
    if (str.Right(2) == _T(".0")) {
        str = str.Left(str.GetLength() - 2);
    }
    str.Replace(_T("."), _T(",")); // Turkce virgul ayraci yapalim
    return str;
}

class CDarkGroupBox : public CButton {
protected:
    afx_msg void OnPaint() {
        CPaintDC dc(this);
        CRect rc;
        GetClientRect(&rc);
        
        CString strText;
        GetWindowText(strText);
        
        CFont* pFont = GetFont();
        CFont* pOldFont = dc.SelectObject(pFont);
        CSize size = dc.GetTextExtent(strText);
        
        int topOffset = size.cy / 2;
        rc.top += topOffset;
        
        CPen pen(PS_SOLID, 1, RGB(76, 76, 76)); // %70 siyah
        CPen* pOldPen = dc.SelectObject(&pen);
        dc.SelectStockObject(NULL_BRUSH);
        
        dc.MoveTo(rc.left, rc.bottom - 1);
        dc.LineTo(rc.left, rc.top);
        dc.LineTo(rc.left + 8, rc.top);
        
        if (strText.GetLength() > 0) {
            dc.MoveTo(rc.left + 10 + size.cx, rc.top);
        } else {
            dc.MoveTo(rc.left + 8, rc.top);
        }
        
        dc.LineTo(rc.right - 1, rc.top);
        dc.LineTo(rc.right - 1, rc.bottom - 1);
        dc.LineTo(rc.left, rc.bottom - 1);
        
        dc.SelectObject(pOldPen);
        
        if (strText.GetLength() > 0) {
            CRect rcText(rc.left + 9, rc.top - topOffset, rc.left + 11 + size.cx, rc.top + topOffset);
            dc.SetBkMode(TRANSPARENT);
            dc.SetTextColor(RGB(0, 0, 0));
            dc.DrawText(strText, rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
        
        dc.SelectObject(pOldFont);
    }
    DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CDarkGroupBox, CButton)
    ON_WM_PAINT()
    ON_WM_SIZE()
END_MESSAGE_MAP()

CSearchDlg::CSearchDlg(IDispatch* pApp, CWnd* pParent /*=nullptr*/)
    : CDialog(IDD_SEARCH_DLG, pParent)
    , m_bOlcu(FALSE)
    , m_bIcRenk(TRUE) // Varsayılan i�Yaretli
    , m_bTur(FALSE)
    , m_bKonturOlcu(FALSE)
    , m_bKonturRenk(FALSE)
    , m_bGrupIci(FALSE)
    , m_bNodeCount(FALSE)
    , m_bSubpathCount(FALSE)
    , m_bActiveLayer(FALSE)
    , m_strOlcuOp(_T("="))
    , m_pApp(pApp)
    , m_dTolerance(0.0)
    , m_nIntersectAction(0)
    , m_bTop(FALSE)
    , m_bBottom(TRUE)
    , m_bSame(FALSE)
    , m_bKalip(FALSE)
    , m_bZtrass(FALSE)
    , m_bHesap(FALSE)
    , m_bListOption(FALSE)
    , m_bListOpen(FALSE)
{
    m_brushAra.CreateSolidBrush(RGB(255, 182, 193));     // LightPink
    m_brushOlcu.CreateSolidBrush(RGB(173, 216, 230));    // LightBlue
    m_brushKesisen.CreateSolidBrush(RGB(255, 204, 153)); // Peach
    m_brushExport.CreateSolidBrush(RGB(152, 251, 152));  // PaleGreen
    m_brushListe.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));   // LightYellow
    m_brushKalipBilgi.CreateSolidBrush(RGB(255, 153, 204)); // Vibrant Rose
    m_brushKalipDuzenle.CreateSolidBrush(RGB(204, 153, 255)); // Lavender
}

CSearchDlg::~CSearchDlg()
{
    extern CSearchDlg* g_pSearchDlg;
    if (g_pSearchDlg == this) {
        g_pSearchDlg = nullptr;
    }

    for (auto pBox : m_darkBoxes) {
        if (pBox && ::IsWindow(pBox->GetSafeHwnd())) {
            pBox->UnsubclassWindow();
        }
        delete pBox;
    }
    m_darkBoxes.clear();
}

void CSearchDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHK_OLCU, m_bOlcu);
    DDX_Check(pDX, IDC_CHK_IC_RENK, m_bIcRenk);
    DDX_Check(pDX, IDC_CHK_TUR, m_bTur);
    DDX_Check(pDX, IDC_CHK_KONTUR_OLCU, m_bKonturOlcu);
    DDX_Check(pDX, IDC_CHK_KONTUR_RENK, m_bKonturRenk);
    DDX_Check(pDX, IDC_CHK_GRUP_ICI, m_bGrupIci);
    DDX_Check(pDX, IDC_CHK_NODE_COUNT, m_bNodeCount);
    DDX_Check(pDX, IDC_CHK_SUBPATH_COUNT, m_bSubpathCount);
    DDX_Check(pDX, IDC_CHK_ACTIVE_LAYER, m_bActiveLayer);
    DDX_Text(pDX, IDC_EDT_TOLERANCE, m_dTolerance);
    DDX_Radio(pDX, IDC_RAD_SELECT, m_nIntersectAction);
    DDX_Check(pDX, IDC_CHK_TOP, m_bTop);
    DDX_Check(pDX, IDC_CHK_BOTTOM, m_bBottom);
    DDX_Check(pDX, IDC_CHK_SAME, m_bSame);
    DDX_Check(pDX, IDC_CHK_KALIP, m_bKalip);
    DDX_Check(pDX, IDC_CHK_ZTRASS, m_bZtrass);
    DDX_Check(pDX, IDC_CHK_HESAP, m_bHesap);
    DDX_Control(pDX, IDC_LIST_HESAP, m_listHesap);
    DDX_Check(pDX, IDC_CHK_LIST_OPTION, m_bListOption);
}

BEGIN_MESSAGE_MAP(CSearchDlg, CDialog)
    ON_BN_CLICKED(IDC_BTN_ARA, &CSearchDlg::OnBnClickedBtnAra)
    ON_BN_CLICKED(IDC_BTN_ARA2, &CSearchDlg::OnBnClickedBtnAra2)
    ON_BN_CLICKED(IDC_BTN_ARA5, &CSearchDlg::OnBnClickedBtnAra5)
    ON_BN_CLICKED(IDC_BTN_ARA6, &CSearchDlg::OnBnClickedBtnAra6)
    ON_BN_CLICKED(IDC_BTN_ARA7, &CSearchDlg::OnBnClickedBtnAra7)
    ON_COMMAND_RANGE(IDC_BTN_OLCU_1, IDC_BTN_OLCU_8, &CSearchDlg::OnBnClickedAra3Range)
    ON_BN_CLICKED(IDC_BTN_OLCU_OP, &CSearchDlg::OnBnClickedBtnOlcuOp)
    ON_BN_CLICKED(IDC_BTN_FIND, &CSearchDlg::OnBnClickedBtnFind)
    ON_BN_CLICKED(IDC_BTN_ARA11, &CSearchDlg::OnBnClickedBtnExport)
    ON_BN_CLICKED(IDC_BTN_SELECT_MATCH, &CSearchDlg::OnBnClickedBtnSelectMatch)
    ON_WM_PAINT()
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_BTN_LIST, &CSearchDlg::OnBnClickedBtnList)
    ON_BN_CLICKED(IDC_CHK_LIST_OPTION, &CSearchDlg::OnBnClickedChkListOption)
    ON_BN_CLICKED(IDC_CHK_KALIP, &CSearchDlg::OnBnClickedChkKalip)
    ON_BN_CLICKED(IDC_CHK_HESAP, &CSearchDlg::OnBnClickedChkHesap)
    ON_BN_CLICKED(IDC_BTN_HESAPLA, &CSearchDlg::OnBnClickedBtnHesapla)
    ON_BN_CLICKED(IDC_BTN_MAKINE_KALIBI2, &CSearchDlg::OnBnClickedAyarToggle)
    ON_BN_CLICKED(IDC_BTN_MAKINE_KALIBI3, &CSearchDlg::OnBnClickedAyarToggle)
    ON_BN_CLICKED(IDC_BTN_MAKINE_KALIBI4, &CSearchDlg::OnBnClickedAyarToggle)
    ON_BN_CLICKED(IDC_BTN_simge, &CSearchDlg::OnBnClickedSimge)
    ON_BN_CLICKED(IDC_CHK_ZTRASS, &CSearchDlg::OnBnClickedChkZtrass)
    ON_BN_CLICKED(IDC_BTN_BILGI_EKLE, &CSearchDlg::OnBnClickedBtnBilgiEkle)
    ON_BN_CLICKED(IDC_BTN_KALIP_EKLE, &CSearchDlg::OnBnClickedBtnKalipEkle)
    ON_BN_CLICKED(IDC_BTN_ISARET_TASI, &CSearchDlg::OnBnClickedBtnIsaretTasi)
    ON_BN_CLICKED(IDC_BTN_IMALAT, &CSearchDlg::OnBnClickedBtnImalat)
    ON_BN_CLICKED(IDC_BTN_KALIP_AYIR, &CSearchDlg::OnBnClickedBtnKalipAyir)
    ON_BN_CLICKED(IDC_BTN_MAKINE_KALIBI, &CSearchDlg::OnBnClickedBtnMakineKalibi)
    ON_EN_CHANGE(IDC_EDT_EN, &CSearchDlg::OnEnChangeEdtEn)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_HESAP, &CSearchDlg::OnCustomDrawList)
    ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CSearchDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) {
        TCHAR szClass[128];
        if (::GetClassName(pMsg->hwnd, szClass, 128)) {
            CString strClass(szClass);
            if (strClass.CompareNoCase(_T("Edit")) == 0 || strClass.CompareNoCase(_T("ComboBox")) == 0) {
                ::TranslateMessage(pMsg);
                ::DispatchMessage(pMsg);
                return TRUE; // handled
            }
        }
    }
    return CDialog::PreTranslateMessage(pMsg);
}

BOOL CSearchDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    SetWindowText(_T("Ztrass.com v1.0.18"));

    m_listHesap.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_listHesap.ModifyStyle(0, LVS_NOCOLUMNHEADER);
    m_listHesap.InsertColumn(0, _T("Col0"), LVCFMT_LEFT, 100);
    m_listHesap.InsertColumn(1, _T("Col1"), LVCFMT_LEFT, 80);
    m_listHesap.InsertColumn(2, _T("Col2"), LVCFMT_LEFT, 100);
    m_listHesap.InsertColumn(3, _T("Col3"), LVCFMT_LEFT, 50);
    m_listHesap.InsertColumn(4, _T("Col4"), LVCFMT_LEFT, 75);
    m_listHesap.InsertColumn(5, _T("Col5"), LVCFMT_LEFT, 60);
    m_listHesap.InsertColumn(6, _T("Col6"), LVCFMT_LEFT, 80);
    m_listHesap.InsertColumn(7, _T("Col7"), LVCFMT_LEFT, 55);
    m_listHesap.InsertColumn(8, _T("Col8"), LVCFMT_LEFT, 120);
    // �-lçü kar�Yıla�Ytırma buton metnini ayarla
    SetDlgItemText(IDC_BTN_OLCU_OP, m_strOlcuOp);
    
    // Makine kalıbı ölçüleri varsayılan de�Yerleri
    SetDlgItemText(IDC_EDT_2MM, _T("0.4"));
    SetDlgItemText(IDC_EDT_2MM2, _T("0.6"));
    SetDlgItemText(IDC_EDT_2MM3, _T("0.6"));
    SetDlgItemText(IDC_EDT_2MM4, _T("0.6"));
    SetDlgItemText(IDC_EDT_2MM5, _T("0.6"));
    SetDlgItemText(IDC_EDT_2MM6, _T("0.6"));
    SetDlgItemText(IDC_EDT_2MM7, _T("0.6"));

    // Pencere simgesini ayarla
    HINSTANCE hInst = AfxGetResourceHandle();
    HICON hIcon = ::LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_SIZE));
    if (hIcon) {
        SetIcon(hIcon, TRUE);   // Büyük simge
        SetIcon(hIcon, FALSE);  // Küçük simge
    }
    HICON hIconAyar = ::LoadIcon(hInst, MAKEINTRESOURCE(IDI_SETTINGS));
    if (hIconAyar) {
        ((CButton*)GetDlgItem(IDC_BTN_MAKINE_KALIBI2))->SetIcon(hIconAyar);
    }


    m_wndStatusBar.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, 10001);
    int parts[] = { -1 };
    m_wndStatusBar.SetParts(1, parts);
    m_wndStatusBar.SetText(_T("Hazır"), 0, 0);
        m_bListOpen = FALSE;
    SetDialogHeight(198);
    // SetTimer(1, 250, nullptr); // 184 + 14 for status bar
    CWnd* pGroup = GetDlgItem(IDC_GRP_LISTE);
    if (pGroup) {
        pGroup->ShowWindow(SW_HIDE);
    }
    // Kalip paneli: baslangicta gizli, varsayilan degerler
    ShowKalipPanel(FALSE);
    SetDlgItemText(IDC_EDT_PRES, _T("0"));
    SetDlgItemText(IDC_EDT_MONTAJ, _T("0"));
    SetDlgItemText(IDC_EDT_KALIPTA_IS, _T("1"));
    SetDlgItemText(IDC_EDT_TOPLAM_IS, _T("1"));
    SetDlgItemText(IDC_EDT_KALIP_OLCU, _T("10"));
    SetDlgItemText(IDC_EDT_TASI_OLCU, _T("10"));
    SetDlgItemText(IDC_EDT_2MM, _T("0,55"));
    SetDlgItemText(IDC_EDT_3MM, _T("0,8"));
    SetDlgItemText(IDC_EDT_4MM, _T("1,2"));
    SetDlgItemText(IDC_EDT_5MM, _T("1,5"));
    SetDlgItemText(IDC_EDT_6MM, _T("2"));
    SetDlgItemText(IDC_EDT_7MM, _T("3"));
    CWnd* pWnd = GetWindow(GW_CHILD);
    while (pWnd) {
        TCHAR szClass[32];
        ::GetClassName(pWnd->GetSafeHwnd(), szClass, 32);
        if (_wcsicmp(szClass, L"Button") == 0) {
            if ((pWnd->GetStyle() & BS_TYPEMASK) == BS_GROUPBOX) {
                CDarkGroupBox* pDark = new CDarkGroupBox();
                pDark->SubclassWindow(pWnd->GetSafeHwnd());
                m_darkBoxes.push_back(pDark);
            }
        }
        pWnd = pWnd->GetWindow(GW_HWNDNEXT);
    }
    m_bAyarOpen = FALSE;
    m_bCompactMode = FALSE;
    GetDlgItem(IDC_BTN_simge)->SetWindowText(L"-");
    // Move Ayar elements overlapping to the Bilgi panel
    CRect rcBilgi, rcAyar2;
    GetDlgItem(IDC_GRP_KALIP_BILGI)->GetWindowRect(&rcBilgi);
    GetDlgItem(IDC_GRP_KALIP_DUZENLE_AYAR2)->GetWindowRect(&rcAyar2);
    ScreenToClient(&rcBilgi);
    ScreenToClient(&rcAyar2);
    int offsetX = rcAyar2.left - rcBilgi.left;
    int offsetY = rcAyar2.top - rcBilgi.top;

    int idsAyarMove[] = {
        IDC_GRP_KALIP_DUZENLE_AYAR2, IDC_EDT_2MM, IDC_EDT_2MM2, IDC_EDT_2MM3, IDC_EDT_2MM4, IDC_EDT_2MM5, IDC_EDT_2MM6, IDC_EDT_2MM7,
        IDC_BTN_MAKINE_KALIBI3, IDC_BTN_MAKINE_KALIBI4, IDC_BTN_MAKINE_KALIBI5
    };
    for (int id : idsAyarMove) {
        CWnd* p = GetDlgItem(id);
        if (p) {
            CRect rc;
            p->GetWindowRect(&rc);
            ScreenToClient(&rc);
            rc.OffsetRect(-offsetX, -offsetY);
            p->SetWindowPos(nullptr, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        }
    }
    pWnd = GetWindow(GW_CHILD);
    while (pWnd) {
        TCHAR szClass[32];
        ::GetClassName(pWnd->GetSafeHwnd(), szClass, 32);
        if (_wcsicmp(szClass, L"Static") == 0) {
            CRect rc;
            pWnd->GetWindowRect(&rc);
            ScreenToClient(&rc);
            if (rcAyar2.PtInRect(rc.CenterPoint())) {
                CString strText;
                pWnd->GetWindowText(strText);
                if (strText.Find(_T("mm")) != -1 && strText.Find(_T("Firma")) == -1 && strText.Find(_T("Dosya")) == -1) {
                    rc.OffsetRect(-offsetX, -offsetY);
                    pWnd->SetWindowPos(nullptr, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                }
            }
        }
        pWnd = pWnd->GetWindow(GW_HWNDNEXT);
    }

    
    CRect rcAdBoxDU(6, 10, 192, 22);
    MapDialogRect(&rcAdBoxDU);
    m_wndAdBox.Create(_T("Hotfix Pro - 0532 709 96 81"), WS_CHILD | SS_CENTERIMAGE | SS_CENTER, rcAdBoxDU, this, 3000);
    m_fontAd.CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
    m_wndAdBox.SetFont(&m_fontAd);
    // Firma ComboBox'ini klasor isimleriyle doldur ve Dosya adini cek
    CComboBox* pComboFirma = (CComboBox*)GetDlgItem(IDC_EDT_FIRMA);
    if (m_pApp) {
        IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
        if (pDoc) {
            CString docName = GetStringProp(pDoc, L"Name");
            int extPos = docName.ReverseFind(_T('.')); if (extPos != -1) docName = docName.Left(extPos); SetDlgItemText(IDC_EDT_DOSYA, docName);
            
            if (pComboFirma) {
                CString fullPath = GetStringProp(pDoc, L"FullFileName");
                if (!fullPath.IsEmpty()) {
                    int start = 0;
                    CString token = fullPath.Tokenize(_T("\\/"), start);
                    while (start != -1) {
                        if (token.Find(_T(".")) == -1 && token.Find(_T(":")) == -1 && !token.IsEmpty()) {
                            if (pComboFirma->FindStringExact(-1, token) == CB_ERR) {
                                pComboFirma->AddString(token);
                            }
                        }
                        token = fullPath.Tokenize(_T("\\/"), start);
                    }
                }
            }
            pDoc->Release();
        }
    }
    
    // Kaydedilen Firma adini yukle
    CString iniPath = GetSettingsFilePath();
    if (!iniPath.IsEmpty()) {
        TCHAR szFirma[256] = {0};
        GetPrivateProfileString(_T("Settings"), _T("Firma"), _T(""), szFirma, 256, iniPath);
        if (_tcslen(szFirma) > 0) {
            if (pComboFirma) {
                int idx = pComboFirma->FindStringExact(-1, szFirma);
                if (idx != CB_ERR) pComboFirma->SetCurSel(idx);
                else pComboFirma->SetWindowText(szFirma);
            } else {
                SetDlgItemText(IDC_EDT_FIRMA, szFirma);
            }
        }
    }

    return TRUE;
}

void CSearchDlg::OnBnClickedBtnOlcuOp()
{
    // = -> > -> < döngüsü
    if (m_strOlcuOp == _T("="))
        m_strOlcuOp = _T(">");
    else if (m_strOlcuOp == _T(">"))
        m_strOlcuOp = _T("<");
    else
        m_strOlcuOp = _T("=");

    SetDlgItemText(IDC_BTN_OLCU_OP, m_strOlcuOp);
}

void CSearchDlg::OnBnClickedBtnAra()
{
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Arama");
    UpdateData(TRUE); // Arayüzdeki seçimleri de�Yi�Ykenlere aktar
    LogDebug(L"=========================================================");
    LogDebug(L"[BILGI] OnBnClickedBtnAra: Arama Islemi Baslatildi.");
    LogDebug(L"[BILGI] Secimler -> m_bOlcu: %d, m_bIcRenk: %d, m_bTur: %d, m_bKonturOlcu: %d, m_bKonturRenk: %d, m_bGrupIci: %d, Op: %s",
        m_bOlcu, m_bIcRenk, m_bTur, m_bKonturOlcu, m_bKonturRenk, m_bGrupIci, (const WCHAR*)m_strOlcuOp);

    if (!m_pApp) {
        LogDebug(L"[HATA] OnBnClickedBtnAra: m_pApp NULL!");
        AfxMessageBox(_T("CorelDRAW uygulama nesnesi bulunamadı!"));
        return;
    }

    // 1. Aktif Dökümanı Al
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) {
        LogDebug(L"[HATA] OnBnClickedBtnAra: ActiveDocument alinamadi.");
        AfxMessageBox(_T("Aktif bir döküman bulunamadı."));
        return;
    }

    // 2. Birimleri Milimetreye Ayarla (cdrMillimeter = 1)
    SetLongProp(pDoc, L"Unit", 3); // cdrMillimeter

    // 3. Seçili �ekli Al
    IDispatch* pActiveShape = GetDispatchProp(pDoc, L"ActiveShape");
    if (!pActiveShape) {
        LogDebug(L"[HATA] OnBnClickedBtnAra: ActiveShape NULL! Secili nesne yok.");
        AfxMessageBox(_T("Lütfen referans alaca�Yınız bir obje seçin."));
        pDoc->Release();
        return;
    }

    // 4. Seçim aralı�Yını kontrol et (Tek obje mi seçili yoksa çoklu mu?)
    IDispatch* pSelection = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    long selectionCount = 0;
    if (pSelection) {
        selectionCount = GetLongProp(pSelection, L"Count");
    }
    LogDebug(L"[BILGI] Secim araligindaki toplam nesne sayisi: %d", selectionCount);

    CString q = _T("");

    // A. İç Renk Kriteri
    if (m_bIcRenk) {
        IDispatch* pFill = GetDispatchProp(pActiveShape, L"Fill");
        if (pFill) {
            long fillType = GetLongProp(pFill, L"Type");
            if (fillType == 1) { // cdrUniformFill = 1
                IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                if (pColor) {
                    CString colorCQL = GetColorCQL(pColor);
                    q += _T("(@fill.color = ") + colorCQL + _T(")");
                    pColor->Release();
                }
            }
            else if (fillType == 0) { // cdrNoFill
                q += _T("(@fill.type = 'none')");
            }
            pFill->Release();
        }
    }

    // B. Kontür Rengi Kriteri
    if (m_bKonturRenk) {
        IDispatch* pOutline = GetDispatchProp(pActiveShape, L"Outline");
        if (pOutline) {
            long outlineType = GetLongProp(pOutline, L"Type");
            if (outlineType == 1) { // cdrOutline
                IDispatch* pColor = GetDispatchProp(pOutline, L"Color");
                if (pColor) {
                    CString colorCQL = GetColorCQL(pColor);
                    if (!q.IsEmpty()) q += _T(" and ");
                    q += _T("(@outline.color = ") + colorCQL + _T(")");
                    pColor->Release();
                }
            }
            else if (outlineType == 0) { // cdrNoOutline
                if (!q.IsEmpty()) q += _T(" and ");
                q += _T("(@outline.type = 'none')");
            }
            pOutline->Release();
        }
    }

    // C. Kontür �-lçüsü Kriteri
    if (m_bKonturOlcu) {
        IDispatch* pOutline = GetDispatchProp(pActiveShape, L"Outline");
        if (pOutline) {
            long outlineType = GetLongProp(pOutline, L"Type");
            if (outlineType == 1) { // cdrOutline
                double width = GetDoubleProp(pOutline, L"Width"); // inç cinsinden gelir
                CString strWidth;
                strWidth.Format(_T("%.6f"), width);
                strWidth.Replace(_T(","), _T(".")); // Ondalık ayracını nokta yap
                if (!q.IsEmpty()) q += _T(" and ");
                q += _T("(@outline.width = ") + strWidth + _T(")");
            }
            else if (outlineType == 0) { // cdrNoOutline
                if (!q.IsEmpty()) q += _T(" and ");
                q += _T("(@outline.type = 'none')");
            }
            pOutline->Release();
        }
    }

    // D. �-lçü (Geni�Ylik) Kriteri
    if (m_bOlcu) {
        double sizeWidth = GetDoubleProp(pActiveShape, L"SizeWidth"); // inç cinsinden gelir
        CString op = _T("");
        if (m_strOlcuOp == _T("=")) {
            CString strMin, strMax;
            strMin.Format(_T("%.6f"), sizeWidth - 0.00236); // 0.06 mm = 0.00236 inç tolerans
            strMin.Replace(_T(","), _T("."));
            strMax.Format(_T("%.6f"), sizeWidth + 0.00236);
            strMax.Replace(_T(","), _T("."));
            op.Format(_T("(@width > %s and @width < %s)"), strMin, strMax);
        }
        else if (m_strOlcuOp == _T(">")) {
            CString strVal;
            strVal.Format(_T("%.6f"), sizeWidth - 0.00039); // 0.01 mm = 0.00039 inç tolerans
            strVal.Replace(_T(","), _T("."));
            op.Format(_T("(@width >= %s)"), strVal);
        }
        else if (m_strOlcuOp == _T("<")) {
            CString strVal;
            strVal.Format(_T("%.6f"), sizeWidth + 0.00039);
            strVal.Replace(_T(","), _T("."));
            op.Format(_T("(@width <= %s)"), strVal);
        }

        if (!op.IsEmpty()) {
            if (!q.IsEmpty()) q += _T(" and ");
            q += op;
        }
    }

    // E. Dü�Yüm Sayısı Kriteri
    if (m_bNodeCount) {
        IDispatch* pCurve = GetDispatchProp(pActiveShape, L"Curve");
        if (pCurve) {
            IDispatch* pNodes = GetDispatchProp(pCurve, L"Nodes");
            if (pNodes) {
                long nodesCount = GetLongProp(pNodes, L"Count");
                if (!q.IsEmpty()) q += _T(" and ");
                CString strCount;
                strCount.Format(_T("%d"), nodesCount);
                q += _T("(@controlPoints.count = ") + strCount + _T(")");
                pNodes->Release();
            }
            pCurve->Release();
        }
    }

    // F. Alt Yol Sayısı Kriteri
    if (m_bSubpathCount) {
        IDispatch* pCurve = GetDispatchProp(pActiveShape, L"Curve");
        if (pCurve) {
            IDispatch* pSubpaths = GetDispatchProp(pCurve, L"Subpaths");
            if (pSubpaths) {
                long subpathCount = GetLongProp(pSubpaths, L"Count");
                if (!q.IsEmpty()) q += _T(" and ");
                CString strCount;
                strCount.Format(_T("%d"), subpathCount);
                q += _T("(@subpaths.count = ") + strCount + _T(")");
                pSubpaths->Release();
            }
            pCurve->Release();
        }
    }

    // Grup kısıtını do�Yrudan sorguya ekle
    if (!m_bGrupIci) {
        if (!q.IsEmpty()) q += _T(" and ");
        q += _T("(@type <> 'group')");
    }

    // Do�Yrulama: En az bir kriter seçilmelidir
    if (!m_bTur && q.IsEmpty()) {
        AfxMessageBox(_T("Lütfen en az bir kriter seçin."));
        pActiveShape->Release();
        if (pSelection) pSelection->Release();
        pDoc->Release();
        return;
    }

    long shapeType = 0; // cdrNoShape
    if (m_bTur) {
        shapeType = GetLongProp(pActiveShape, L"Type");
    }

    // Arama Kapsamı (Tek obje seçiliyse tüm sayfa/aktif katman, çoklu seçiliyse seçim içi)
    IDispatch* pResultRange = nullptr;
    if (selectionCount <= 1) {
        if (m_bActiveLayer) {
            // Sadece Aktif Katmanda aramak için ActiveLayer.Shapes.FindShapes kullanıyoruz
            LogDebug(L"[BILGI] ActiveLayer->Shapes->FindShapes cagrisi yapiliyor. Sorgu: %s", (const WCHAR*)q);
            IDispatch* pLayer = GetDispatchProp(pDoc, L"ActiveLayer");
            if (pLayer) {
                IDispatch* pShapes = GetDispatchProp(pLayer, L"Shapes");
                if (pShapes) {
                    pResultRange = FindShapes(pShapes, q, shapeType, true);
                    pShapes->Release();
                }
                pLayer->Release();
            }
        }
        else {
            // Tüm dökümanda (Aktif Sayfada) aramak için ActivePage.Shapes.FindShapes metodunu kullanıyoruz
            LogDebug(L"[BILGI] ActivePage->Shapes->FindShapes cagrisi yapiliyor. Sorgu: %s", (const WCHAR*)q);
            IDispatch* pPage = GetDispatchProp(pDoc, L"ActivePage");
            if (pPage) {
                IDispatch* pShapes = GetDispatchProp(pPage, L"Shapes");
                if (pShapes) {
                    pResultRange = FindShapes(pShapes, q, shapeType, true);
                    pShapes->Release();
                }
                pPage->Release();
            }
        }
    }
    else {
        // �?oklu seçim alanı içinde arama
        pResultRange = FindShapes(pSelection, q, shapeType, true);
    }

    // Arama sonuclarini logla
    long matchCount = 0;
    if (pResultRange) {
        matchCount = GetLongProp(pResultRange, L"Count");
    }
    LogDebug(L"[BILGI] Arama Tamamlandi! Sorgu: %s", (const WCHAR*)q);
    LogDebug(L"[BILGI] E�Yle�Yen Nesne Sayısı: %d", matchCount);

    // ALTERNATİF Y�-NTEM: E�Yer CQL araması 0 sonuç döndürdüyse, do�Yrudan COM tarama yapalım!
    if (matchCount == 0) {
        LogDebug(L"[BILGI] CQL Sonuc vermedi. COM Tarama yontemi (Alternatif) baslatiliyor...");
        if (pResultRange) pResultRange->Release();
        pResultRange = SearchByCOMScan(pActiveShape);
        if (pResultRange) {
            matchCount = GetLongProp(pResultRange, L"Count");
            LogDebug(L"[BILGI] COM Tarama tamamlandi. E�Yle�Yen Nesne Sayısı: %d", matchCount);
        }
    }

    // E�Yle�Yen �Yekilleri seç
    if (pResultRange && matchCount > 0) {
        LogDebug(L"[BILGI] CreateSelection cagriliyor...");
        InvokeMethodNoArgs(pResultRange, L"CreateSelection");
        
        // Kullanıcının ba�Yta seçti�Yi objenin seçimini kaybetmemesi için AddToSelection ça�Yırıyoruz
        DISPID dispidAdd;
        OLECHAR* szAdd = (OLECHAR*)L"AddToSelection";
        if (SUCCEEDED(pActiveShape->GetIDsOfNames(IID_NULL, &szAdd, 1, LOCALE_USER_DEFAULT, &dispidAdd))) {
            DISPPARAMS paramsAdd = { NULL, NULL, 0, 0 };
            pActiveShape->Invoke(dispidAdd, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsAdd, NULL, NULL, NULL);
        }
    }
    if (pResultRange) {
        pResultRange->Release();
    }

    // COM Kaynaklarını Serbest Bırak
    pActiveShape->Release();
    if (pSelection) pSelection->Release();
    pDoc->Release();
}

void CSearchDlg::PostNcDestroy()
{
    CDialog::PostNcDestroy();
    delete this;
}

void CSearchDlg::OnCancel()
{
    DestroyWindow();
}

void CSearchDlg::OnOK()
{
    OnBnClickedBtnAra();
}



static bool CompareColors(IDispatch* pCol1, IDispatch* pCol2)
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
}

IDispatch* CSearchDlg::SearchByCOMScan(IDispatch* pActiveShape)
{
    LogDebug(L"[BILGI] SearchByCOMScan: COM Tarama Islemi Baslatildi. AktifKatman: %d", m_bActiveLayer);
    
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return nullptr;

    IDispatch* pShapes = nullptr;
    if (m_bActiveLayer) {
        IDispatch* pLayer = GetDispatchProp(pDoc, L"ActiveLayer");
        if (pLayer) {
            pShapes = GetDispatchProp(pLayer, L"Shapes");
            pLayer->Release();
        }
    } else {
        IDispatch* pPage = GetDispatchProp(pDoc, L"ActivePage");
        if (pPage) {
            pShapes = GetDispatchProp(pPage, L"Shapes");
            pPage->Release();
        }
    }

    if (!pShapes) {
        LogDebug(L"[HATA] SearchByCOMScan: Shapes koleksiyonu alinamadi.");
        pDoc->Release();
        return nullptr;
    }

    // Seçilen kapsama göre tüm �Yekilleri tek bir düz ShapeRange olarak alalım
    IDispatch* pAllShapesRange = FindShapes(pShapes, _T(""), 0, true);
    pShapes->Release();

    if (!pAllShapesRange) {
        pDoc->Release();
        return nullptr;
    }

    long totalShapes = GetLongProp(pAllShapesRange, L"Count");
    LogDebug(L"[BILGI] SearchByCOMScan: Taranacak toplam sekil sayisi: %d", totalShapes);

    // Bo�Y bir ShapeRange olu�Yturalım
    IDispatch* pResultRange = nullptr;
    DISPID dispidCreateSR;
    WCHAR* szMethodCreate = L"CreateShapeRange";
    HRESULT hr = m_pApp->GetIDsOfNames(IID_NULL, &szMethodCreate, 1, LOCALE_USER_DEFAULT, &dispidCreateSR);
    if (SUCCEEDED(hr)) {
        DISPPARAMS params = { NULL, NULL, 0, 0 };
        VARIANT varRes;
        VariantInit(&varRes);
        hr = m_pApp->Invoke(dispidCreateSR, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &varRes, NULL, NULL);
        if (SUCCEEDED(hr) && varRes.vt == VT_DISPATCH) {
            pResultRange = varRes.pdispVal;
        }
    }

    if (!pResultRange) {
        LogDebug(L"[HATA] SearchByCOMScan: CreateShapeRange basarisiz!");
        pAllShapesRange->Release();
        pDoc->Release();
        return nullptr;
    }

    // Referans nesnenin özelliklerini alalım
    double refWidth = GetDoubleProp(pActiveShape, L"SizeWidth");
    long refType = GetLongProp(pActiveShape, L"Type");

    // Referans Dü�Yüm ve Alt Yol Sayısı
    long refNodeCount = 0;
    long refSubpathCount = 0;

    if (m_bNodeCount) {
        IDispatch* pCurve = GetDispatchProp(pActiveShape, L"Curve");
        if (pCurve) {
            IDispatch* pNodes = GetDispatchProp(pCurve, L"Nodes");
            if (pNodes) {
                refNodeCount = GetLongProp(pNodes, L"Count");
                pNodes->Release();
            }
            pCurve->Release();
        }
        LogDebug(L"[BILGI] SearchByCOMScan: Referans Dugum Sayisi: %d", refNodeCount);
    }

    if (m_bSubpathCount) {
        IDispatch* pCurve = GetDispatchProp(pActiveShape, L"Curve");
        if (pCurve) {
            IDispatch* pSubpaths = GetDispatchProp(pCurve, L"Subpaths");
            if (pSubpaths) {
                refSubpathCount = GetLongProp(pSubpaths, L"Count");
                pSubpaths->Release();
            }
            pCurve->Release();
        }
        LogDebug(L"[BILGI] SearchByCOMScan: Referans Alt Yol Sayisi: %d", refSubpathCount);
    }

    // Referans dolgu ve kontür
    long refFillType = 0;
    IDispatch* pRefFillColor = nullptr;
    IDispatch* pRefFill = GetDispatchProp(pActiveShape, L"Fill");
    if (pRefFill) {
        refFillType = GetLongProp(pRefFill, L"Type");
        if (refFillType == 1) { // Uniform
            pRefFillColor = GetDispatchProp(pRefFill, L"UniformColor");
        }
        pRefFill->Release();
    }

    long refOutlineType = 0;
    double refOutlineWidth = 0.0;
    IDispatch* pRefOutlineColor = nullptr;
    IDispatch* pRefOutline = GetDispatchProp(pActiveShape, L"Outline");
    if (pRefOutline) {
        refOutlineType = GetLongProp(pRefOutline, L"Type");
        if (refOutlineType == 1) {
            refOutlineWidth = GetDoubleProp(pRefOutline, L"Width");
            pRefOutlineColor = GetDispatchProp(pRefOutline, L"Color");
        }
        pRefOutline->Release();
    }

    // Toleranslar (inç cinsinden)
    double widthTol = 0.00236; // 0.06 mm

    DISPID dispidItem, dispidAdd;
    WCHAR* szItem = L"Item";
    WCHAR* szAdd = L"Add";
    pAllShapesRange->GetIDsOfNames(IID_NULL, &szItem, 1, LOCALE_USER_DEFAULT, &dispidItem);
    pResultRange->GetIDsOfNames(IID_NULL, &szAdd, 1, LOCALE_USER_DEFAULT, &dispidAdd);

    for (long i = 1; i <= totalShapes; i++) {
        // i. �Yekli al
        VARIANT varIndex;
        varIndex.vt = VT_I4;
        varIndex.lVal = i;
        DISPPARAMS paramsItem = { &varIndex, NULL, 1, 0 };
        VARIANT varShape;
        VariantInit(&varShape);
        hr = pAllShapesRange->Invoke(dispidItem, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &paramsItem, &varShape, NULL, NULL);
        if (FAILED(hr) || varShape.vt != VT_DISPATCH || !varShape.pdispVal) continue;

        IDispatch* pShape = varShape.pdispVal;

        // Referans nesnenin kendisini atlayalım
        long shapeID = GetLongProp(pShape, L"StaticID");
        long refID = GetLongProp(pActiveShape, L"StaticID");
        if (shapeID == refID) {
            pShape->Release();
            continue;
        }

        bool match = true;

        // 1. Tip kontrolü
        if (m_bTur) {
            long type = GetLongProp(pShape, L"Type");
            if (type != refType) match = false;
        }

        // 2. �-lçü kontrolü
        if (match && m_bOlcu) {
            double width = GetDoubleProp(pShape, L"SizeWidth");
            double diff = fabs(width - refWidth);
            if (m_strOlcuOp == _T("=")) {
                if (diff > widthTol) match = false;
            }
            else if (m_strOlcuOp == _T(">")) {
                if (width < (refWidth - 0.00039)) match = false;
            }
            else if (m_strOlcuOp == _T("<")) {
                if (width > (refWidth + 0.00039)) match = false;
            }
        }

        // 3. İç Renk kontrolü
        if (match && m_bIcRenk) {
            long fillType = 0;
            IDispatch* pFillColor = nullptr;
            IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
            if (pFill) {
                fillType = GetLongProp(pFill, L"Type");
                if (fillType == 1) {
                    pFillColor = GetDispatchProp(pFill, L"UniformColor");
                }
                pFill->Release();
            }

            if (fillType != refFillType) {
                match = false;
            }
            else if (fillType == 1) {
                if (!CompareColors(pFillColor, pRefFillColor)) match = false;
            }

            if (pFillColor) pFillColor->Release();
        }

        // 4. Kontür Rengi kontrolü
        if (match && m_bKonturRenk) {
            long outlineType = 0;
            IDispatch* pOutlineColor = nullptr;
            IDispatch* pOutline = GetDispatchProp(pShape, L"Outline");
            if (pOutline) {
                outlineType = GetLongProp(pOutline, L"Type");
                if (outlineType == 1) {
                    pOutlineColor = GetDispatchProp(pOutline, L"Color");
                }
                pOutline->Release();
            }

            if (outlineType != refOutlineType) {
                match = false;
            }
            else if (outlineType == 1) {
                if (!CompareColors(pOutlineColor, pRefOutlineColor)) match = false;
            }

            if (pOutlineColor) pOutlineColor->Release();
        }

        // 5. Kontür �-lçüsü kontrolü
        if (match && m_bKonturOlcu) {
            long outlineType = 0;
            double outlineWidth = 0.0;
            IDispatch* pOutline = GetDispatchProp(pShape, L"Outline");
            if (pOutline) {
                outlineType = GetLongProp(pOutline, L"Type");
                if (outlineType == 1) {
                    outlineWidth = GetDoubleProp(pOutline, L"Width");
                }
                pOutline->Release();
            }

            if (outlineType != refOutlineType) {
                match = false;
            }
            else if (outlineType == 1) {
                if (fabs(outlineWidth - refOutlineWidth) > 0.0001) match = false;
            }
        }

        // 6. Grup kontrolü
        if (match && !m_bGrupIci) {
            IDispatch* pParentGroup = GetDispatchProp(pShape, L"ParentGroup");
            if (pParentGroup) {
                match = false;
                pParentGroup->Release();
            }
        }

        // 7. Dü�Yüm Sayısı kontrolü
        if (match && m_bNodeCount) {
            long nodeCount = 0;
            IDispatch* pCurve = GetDispatchProp(pShape, L"Curve");
            if (pCurve) {
                IDispatch* pNodes = GetDispatchProp(pCurve, L"Nodes");
                if (pNodes) {
                    nodeCount = GetLongProp(pNodes, L"Count");
                    pNodes->Release();
                }
                pCurve->Release();
            }
            if (nodeCount != refNodeCount) match = false;
        }

        // 8. Alt Yol Sayısı kontrolü
        if (match && m_bSubpathCount) {
            long subpathCount = 0;
            IDispatch* pCurve = GetDispatchProp(pShape, L"Curve");
            if (pCurve) {
                IDispatch* pSubpaths = GetDispatchProp(pCurve, L"Subpaths");
                if (pSubpaths) {
                    subpathCount = GetLongProp(pSubpaths, L"Count");
                    pSubpaths->Release();
                }
                pCurve->Release();
            }
            if (subpathCount != refSubpathCount) match = false;
        }

        // E�Yer e�Yle�Yiyorsa sonuç listesine ekle
        if (match) {
            VARIANT varArg;
            varArg.vt = VT_DISPATCH;
            varArg.pdispVal = pShape;
            DISPPARAMS paramsAdd = { &varArg, NULL, 1, 0 };
            pResultRange->Invoke(dispidAdd, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsAdd, NULL, NULL, NULL);
        }

        pShape->Release();
    }

    // Temizlik
    if (pRefFillColor) pRefFillColor->Release();
    if (pRefOutlineColor) pRefOutlineColor->Release();
    pAllShapesRange->Release();
    pDoc->Release();

    return pResultRange;
}


void CSearchDlg::OnBnClickedBtnFind()
{
    if (!m_pApp) return;
    
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Lazer Kontrol");
    
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    
    SetLongProp(pDoc, L"Unit", 3);
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pSel) {
        long count = GetLongProp(pSel, L"Count");
        if (count > 0) {
            CString strTol;
            GetDlgItemText(IDC_EDT_TOLERANCE, strTol);
            strTol.Replace(L",", L".");
            double tol = _wtof(strTol);
            if (tol <= 0.0) tol = 0.1;
            
            bool bDelete = (IsDlgButtonChecked(IDC_RAD_DELETE) == BST_CHECKED);
            bool bCenter = (IsDlgButtonChecked(IDC_RAD_CENTER) == BST_CHECKED);
            
            struct CircleInfo {
                long index;
                double cx, cy, radius;
                volatile bool markForDelete;
            };
            std::vector<CircleInfo> circles(count);
            
            for (long i = 1; i <= count; ++i) {
                IDispatch* pShape = GetDispatchPropWithIntArg(pSel, L"Item", i);
                if (pShape) {
                    double w = GetDoubleProp(pShape, L"SizeWidth");
                    double cx = GetDoubleProp(pShape, L"CenterX");
                    double cy = GetDoubleProp(pShape, L"CenterY");
                    circles[i - 1] = {i, cx, cy, w / 2.0, false};
                    pShape->Release();
                }
            }
            
            // Multithreading ile hizlandirma (Dinamik is parcacigi)
            int hwThreads = std::thread::hardware_concurrency();
            if (hwThreads == 0) hwThreads = 8;
            
            // Cok az obje varsa gereksiz thread acmayalim
            int numThreads = hwThreads;
            if (count < 1000) {
                numThreads = 2;
            }
            if (count < 100) {
                numThreads = 1;
            }
            std::vector<std::thread> threads;
            size_t n = circles.size();
            
            for (int t = 0; t < numThreads; ++t) {
                threads.emplace_back([&circles, n, tol, bCenter, t, numThreads]() {
                    // Blok dagitimi (Block distribution) i=0..n-1
                    size_t start = (n * t) / numThreads;
                    size_t end = (n * (t + 1)) / numThreads;
                    
                    for (size_t i = start; i < end; ++i) {
                        if (circles[i].markForDelete) continue;
                        
                        for (size_t j = i + 1; j < n; ++j) {
                            if (circles[j].markForDelete) continue;
                            
                            double dx = circles[i].cx - circles[j].cx;
                            double dy = circles[i].cy - circles[j].cy;
                            
                            // Karekteristik optimizasyon: distance hesaplamadan once bounding box kontrolu (cok hizlandirir)
                            double requiredDist = circles[i].radius + circles[j].radius - tol;
                            if (abs(dx) > requiredDist || abs(dy) > requiredDist) continue;
                            
                            double dist = sqrt(dx*dx + dy*dy);
                            if (dist < requiredDist) {
                                circles[j].markForDelete = true;
                                if (bCenter) {
                                    circles[i].cx = (circles[i].cx + circles[j].cx) / 2.0;
                                    circles[i].cy = (circles[i].cy + circles[j].cy) / 2.0;
                                }
                            }
                        }
                    }
                });
            }
            
            for (auto& th : threads) {
                th.join();
            }
            
            for (long i = (long)circles.size() - 1; i >= 0; --i) {
                if (circles[i].markForDelete) {
                    IDispatch* pShape = GetDispatchPropWithIntArg(pSel, L"Item", circles[i].index);
                    if (pShape) {
                        if (bDelete || bCenter) {
                            DISPID dispid;
                            OLECHAR* szDelete = (OLECHAR*)L"Delete";
                            if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szDelete, 1, LOCALE_USER_DEFAULT, &dispid))) {
                                DISPPARAMS params = { NULL, NULL, 0, 0 };
                                pShape->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, NULL, NULL, NULL);
                            }
                        } else {
                            IDispatch* pOutline = GetDispatchProp(pShape, L"Outline");
                            if (pOutline) {
                                IDispatch* pColor = GetDispatchProp(pOutline, L"Color");
                                if (pColor) {
                                    DISPID dispid;
                                    OLECHAR* szAssign = (OLECHAR*)L"RGBAssign";
                                    if (SUCCEEDED(pColor->GetIDsOfNames(IID_NULL, &szAssign, 1, LOCALE_USER_DEFAULT, &dispid))) {
                                        VARIANT args[3]; VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]);
                                        args[2].vt = VT_I4; args[2].lVal = 255;
                                        args[1].vt = VT_I4; args[1].lVal = 0;
                                        args[0].vt = VT_I4; args[0].lVal = 0;
                                        DISPPARAMS params = { args, NULL, 3, 0 };
                                        pColor->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, NULL, NULL, NULL);
                                    }
                                    pColor->Release();
                                }
                                pOutline->Release();
                            }
                        }
                        pShape->Release();
                    }
                } else if (bCenter) {
                    IDispatch* pShape = GetDispatchPropWithIntArg(pSel, L"Item", circles[i].index);
                    if (pShape) {
                        SetDoubleProp(pShape, L"CenterX", circles[i].cx);
                        SetDoubleProp(pShape, L"CenterY", circles[i].cy);
                        pShape->Release();
                    }
                }
            }
        }
        pSel->Release();
    }
    pDoc->Release();
}

void CSearchDlg::OnBnClickedBtnExport()
{
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    
    CFileDialog dlg(FALSE, L"plt", L"LazerExport.plt", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        L"PLT Dosyalari (*.plt)|*.plt|DXF Dosyalari (*.dxf)|*.dxf||", this);
        
    if (dlg.DoModal() == IDOK) {
        CString path = dlg.GetPathName();
        CString ext = dlg.GetFileExt();
        ext.MakeLower();
        
        // Kesin Dogru Filtre ID'leri
        long filterIdx = (ext == L"dxf") ? 1296 : 1281; 
        
        // 1. Export Options Olustur (CreateStructExportOptions)
        IDispatch* pExpOpt = nullptr;
        DISPID dispidCreate; OLECHAR* szCreate = (OLECHAR*)L"CreateStructExportOptions";
        if (SUCCEEDED(m_pApp->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dispidCreate))) {
            DISPPARAMS params = { NULL, NULL, 0, 0 };
            VARIANT retOpt; VariantInit(&retOpt);
            if (SUCCEEDED(m_pApp->Invoke(dispidCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retOpt, NULL, NULL))) {
                if (retOpt.vt == VT_DISPATCH) pExpOpt = retOpt.pdispVal;
            }
        }
        
        if (pExpOpt) {
            SetBoolProp(pExpOpt, L"UseColorProfile", false);
        }

        // ===== STRATEJI: ExportEx metodu 5 arguman (Null Dispatch ile) =====
        DISPID dispidExportEx;
        OLECHAR* szExportEx = (OLECHAR*)L"ExportEx";
        HRESULT hrExp = E_FAIL;
        UINT argErr = 0;
        EXCEPINFO excep;
        memset(&excep, 0, sizeof(excep));
        
        if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szExportEx, 1, LOCALE_USER_DEFAULT, &dispidExportEx))) {
            VARIANT args[5];
            for(int i=0; i<5; i++) VariantInit(&args[i]);
            args[4].vt = VT_BSTR; args[4].bstrVal = SysAllocString(path);
            args[3].vt = VT_I4;   args[3].lVal = filterIdx;
            args[2].vt = VT_I4;   args[2].lVal = 1; // cdrSelection
            
            args[1].vt = VT_DISPATCH; args[1].pdispVal = pExpOpt ? pExpOpt : NULL; // Options
            args[0].vt = VT_DISPATCH; args[0].pdispVal = NULL; // PaletteOptions
            
            DISPPARAMS params = { args, NULL, 5, 0 };
            VARIANT retVal; VariantInit(&retVal);
            
            hrExp = pDoc->Invoke(dispidExportEx, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD | DISPATCH_PROPERTYGET, &params, &retVal, &excep, &argErr);
            
            if (SUCCEEDED(hrExp) && retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                IDispatch* pFlt = retVal.pdispVal;
                
                // ExportFilter Ayarlari
                if (ext == L"plt") {
                    SetLongProp(pFlt, L"PenLibIndex", 0);
                    SetBoolProp(pFlt, L"FitToPage", false);
                    SetDoubleProp(pFlt, L"ScaleFactor", 100.0);
                    SetDoubleProp(pFlt, L"PageWidth", 8.5);
                    SetDoubleProp(pFlt, L"PageHeight", 11.0);
                    SetLongProp(pFlt, L"FillType", 0); // hpglNoFill
                    SetDoubleProp(pFlt, L"FillSpacing", 0.005);
                    SetDoubleProp(pFlt, L"FillAngle", 0.0);
                    SetDoubleProp(pFlt, L"HatchAngle", 90.0);
                } else if (ext == L"dxf") {
                    SetLongProp(pFlt, L"BitmapType", 0);
                    SetBoolProp(pFlt, L"TextAsCurves", true);
                    SetLongProp(pFlt, L"Version", 1); // dxfVersion2000
                    SetLongProp(pFlt, L"Units", 3);   // dxfMillimeters
                    SetBoolProp(pFlt, L"FillUnmapped", true);
                    SetLongProp(pFlt, L"FillColor", 0);
                }
                
                InvokeMethodNoArgs(pFlt, L"Finish");
                pFlt->Release();
            } else {
                CString errMsg;
                errMsg.Format(L"ExportEx basarisiz.\nHR: 0x%08X\nArgErrIndex: %u", hrExp, argErr);
                if (excep.bstrDescription) {
                    errMsg += L"\nAciklama: ";
                    errMsg += excep.bstrDescription;
                    SysFreeString(excep.bstrDescription);
                }
                MessageBox(errMsg, L"Export Hata Detayi", MB_ICONERROR);
            }
            SysFreeString(args[4].bstrVal);
        } else {
            MessageBox(L"ExportEx metodu bulunamadi!", L"Hata", MB_ICONERROR);
        }
        
        // JPG Export
        if (SUCCEEDED(hrExp) && IsDlgButtonChecked(IDC_CHK_OLCU3) == BST_CHECKED) {
            CString pathJpg = path.Left(path.ReverseFind(L'.')) + L".jpg";
            DISPID dispExpJpg;
            if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szExportEx, 1, LOCALE_USER_DEFAULT, &dispExpJpg))) {
                VARIANT argsJpg[5];
                for(int i=0; i<5; i++) VariantInit(&argsJpg[i]);
                argsJpg[4].vt = VT_BSTR; argsJpg[4].bstrVal = SysAllocString(pathJpg);
                argsJpg[3].vt = VT_I4;   argsJpg[3].lVal = 774; // cdrJPEG
                argsJpg[2].vt = VT_I4;   argsJpg[2].lVal = 1;   // cdrSelection
                
                argsJpg[1].vt = VT_DISPATCH; argsJpg[1].pdispVal = pExpOpt ? pExpOpt : NULL; // Options
                argsJpg[0].vt = VT_DISPATCH; argsJpg[0].pdispVal = NULL; // PaletteOptions
                
                DISPPARAMS paramsJpg = { argsJpg, NULL, 5, 0 };
                VARIANT retJpg; VariantInit(&retJpg);
                HRESULT hrJpg = pDoc->Invoke(dispExpJpg, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD | DISPATCH_PROPERTYGET, &paramsJpg, &retJpg, NULL, NULL);
                if (SUCCEEDED(hrJpg) && retJpg.vt == VT_DISPATCH && retJpg.pdispVal) {
                    IDispatch* pFltJpg = retJpg.pdispVal;
                    SetLongProp(pFltJpg, L"Compression", 20);
                    SetBoolProp(pFltJpg, L"Optimized", true);
                    SetLongProp(pFltJpg, L"Smoothing", 0);
                    SetLongProp(pFltJpg, L"SubFormat", 1);
                    SetBoolProp(pFltJpg, L"Progressive", true);
                    InvokeMethodNoArgs(pFltJpg, L"Finish");
                    pFltJpg->Release();
                }
                SysFreeString(argsJpg[4].bstrVal);
            }
        }
        
        if (pExpOpt) pExpOpt->Release();
        
        if (SUCCEEDED(hrExp)) {
            MessageBox(L"Export islemi tamamlandi. Dosyalari kontrol edin.", L"Basari", MB_ICONINFORMATION);
        }
    }
    pDoc->Release();
}


void CSearchDlg::OnBnClickedBtnSelectMatch()
{
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Benzerleri Sec");
    UpdateData(TRUE);
    LogDebug(L"[BILGI] OnBnClickedBtnSelectMatch tetiklendi. Ust: %d, Alt: %d, Ayni: %d", m_bTop, m_bBottom, m_bSame);
}


void CSearchDlg::SetDialogHeight(int heightDU)
{
    CRect rc;
    GetWindowRect(&rc);
    
    CRect rcHeight(0, 0, 0, heightDU);
    MapDialogRect(&rcHeight);
    
    if (m_wndStatusBar.GetSafeHwnd()) {
        CRect rcStatus;
        m_wndStatusBar.GetWindowRect(&rcStatus);
        rcHeight.bottom += rcStatus.Height();
    }
    
    CalcWindowRect(&rcHeight);
    
    SetWindowPos(nullptr, 0, 0, rc.Width(), rcHeight.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    
    if (m_wndStatusBar.GetSafeHwnd()) {
        m_wndStatusBar.SendMessage(WM_SIZE, 0, 0);
    }
}

void CSearchDlg::SetDialogWidth(int widthDU)
{
    CRect rc;
    GetWindowRect(&rc);
    
    CRect rcWidth(0, 0, widthDU, 0);
    MapDialogRect(&rcWidth);
    CalcWindowRect(&rcWidth);
    
    SetWindowPos(nullptr, 0, 0, rcWidth.Width(), rc.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CSearchDlg::UpdateDialogWidth()
{
    int wDU = 230;
    if (m_bKalip) wDU = 350;
    
    CRect rcWidth(0, 0, wDU, 0);
    MapDialogRect(&rcWidth);
    int targetClientWidth = rcWidth.Width();
    
    if (m_bHesap) {
        int totalListWidth = 0;
        int nCols = m_listHesap.GetHeaderCtrl()->GetItemCount();
        for (int i = 0; i < nCols; i++) {
            totalListWidth += m_listHesap.GetColumnWidth(i);
        }
        int listW = totalListWidth + 25; 
        
        CRect rcGrp;
        CWnd* pGrp = GetDlgItem(IDC_GRP_KALIP_BILGI);
        int listX = 0;
        int listY = 0;
        
        if (pGrp && pGrp->IsWindowVisible()) {
            pGrp->GetWindowRect(&rcGrp);
            ScreenToClient(&rcGrp);
            listX = rcGrp.right + 15;
            listY = rcGrp.top;
        } else {
            CRect rcList;
            m_listHesap.GetWindowRect(&rcList);
            ScreenToClient(&rcList);
            listX = targetClientWidth + 10;
            listY = rcList.top;
        }
        
        CRect rcList;
        m_listHesap.GetWindowRect(&rcList);
        ScreenToClient(&rcList);
        m_listHesap.SetWindowPos(NULL, listX, listY, listW, rcList.Height(), SWP_NOZORDER);
        
        targetClientWidth = listX + listW + 15;
        
        CWnd* pBtn = GetDlgItem(IDC_BTN_HESAPLA);
        if (pBtn) {
            CRect rcBtn;
            pBtn->GetWindowRect(&rcBtn);
            ScreenToClient(&rcBtn);
            int btnX = targetClientWidth - rcBtn.Width() - 15;
            pBtn->SetWindowPos(NULL, btnX, rcBtn.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
    }
    
    CRect rcDlg;
    GetWindowRect(&rcDlg);
    CRect rcClient;
    GetClientRect(&rcClient);
    int nonClientW = rcDlg.Width() - rcClient.Width();
    
    SetWindowPos(NULL, 0, 0, targetClientWidth + nonClientW, rcDlg.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    Invalidate();
}
void CSearchDlg::ShowKalipPanel(BOOL bShow)
{
    int nShow = bShow ? SW_SHOW : SW_HIDE;
    
    int idsDuzenle[] = {
        IDC_BTN_KALIP_EKLE, IDC_EDT_KALIP_OLCU,
        IDC_BTN_ISARET_TASI, IDC_BTN_IMALAT, IDC_EDT_TASI_OLCU, IDC_BTN_KALIP_AYIR, IDC_BTN_MAKINE_KALIBI,
        IDC_BTN_MAKINE_KALIBI2
    };
    for (int id : idsDuzenle) {
        CWnd* p = GetDlgItem(id);
        if (p) p->ShowWindow(bShow ? (m_bAyarOpen ? SW_HIDE : SW_SHOW) : SW_HIDE);
    }
    
    int idsBilgi[] = {
        IDC_GRP_KALIP_BILGI, IDC_EDT_FIRMA, IDC_EDT_DOSYA, IDC_EDT_KALIP_ADI,
        IDC_EDT_PRES, IDC_EDT_MONTAJ, IDC_BTN_BILGI_EKLE, IDC_EDT_KALIPTA_IS, IDC_EDT_TOPLAM_IS
    };
    for (int id : idsBilgi) {
        CWnd* p = GetDlgItem(id);
        if (p) p->ShowWindow(bShow ? (m_bAyarOpen ? SW_HIDE : SW_SHOW) : SW_HIDE);
    }
    
    int idsAyar[] = {
        IDC_GRP_KALIP_DUZENLE_AYAR2, IDC_EDT_2MM, IDC_EDT_2MM2, IDC_EDT_2MM3, IDC_EDT_2MM4, IDC_EDT_2MM5, IDC_EDT_2MM6, IDC_EDT_2MM7,
        IDC_BTN_MAKINE_KALIBI3, IDC_BTN_MAKINE_KALIBI4, IDC_BTN_MAKINE_KALIBI5
    };
    for (int id : idsAyar) {
        CWnd* p = GetDlgItem(id);
        if (p) p->ShowWindow(bShow ? (m_bAyarOpen ? SW_SHOW : SW_HIDE) : SW_HIDE);
    }
    
    // Show static texts inside the Ayar groupbox. Since they are IDC_STATIC, we hide them by their layout position
    // We can also just hide all static controls that overlap with Ayar.
    CWnd* pWnd = GetWindow(GW_CHILD);
    while (pWnd) {
        TCHAR szClass[32];
        ::GetClassName(pWnd->GetSafeHwnd(), szClass, 32);
        if (_wcsicmp(szClass, L"Static") == 0) {
            CString strText;
            pWnd->GetWindowText(strText);
            
            if (strText == _T("Firma") || strText == _T("Dosya") || strText == _T("Kalıp") ||
                strText == _T("Pres") || strText == _T("Montaj") || strText == _T("K. İ�Y") || strText == _T("Toplam") ||
                strText == _T("Kalıp Adı")) 
            {
                pWnd->ShowWindow(bShow ? (m_bAyarOpen ? SW_HIDE : SW_SHOW) : SW_HIDE);
            }
            else if (strText.Find(_T("mm")) != -1 && strText.Find(_T("Firma")) == -1 && strText.Find(_T("Dosya")) == -1) {
                pWnd->ShowWindow(bShow ? (m_bAyarOpen ? SW_SHOW : SW_HIDE) : SW_HIDE);
            }
        }
        pWnd = pWnd->GetWindow(GW_HWNDNEXT);
    }

    UpdateDialogWidth();
}

void CSearchDlg::OnBnClickedAyarToggle()
{
    m_bAyarOpen = !m_bAyarOpen;
    ShowKalipPanel(m_bKalip);
}


void CSearchDlg::UpdateListLayout()
{
    if (!m_bListOpen) return;
    
    int rowCount = (int)m_listObjects.size();
    if (rowCount == 0) return;
    
    int rowH = m_bListOption ? 26 : 18 + 8; // combo boxlar icin 26 piksel ayiralim (biraz daha rahat)
    int gridRows = m_bListOption ? rowCount : (rowCount + 2) / 3;
    
    // Baslik alani (14px) + satirlar + alt bosluk (combo box tasmamasi icin 30 px, normalde 16 px)
    int grupIcYukseklik = 14 + gridRows * rowH + (m_bListOption ? 16 : 8);
    int minGrupH = 30;
    if (grupIcYukseklik < minGrupH) grupIcYukseklik = minGrupH;
    
    // Liste grup kutusunu yeniden boyutlandir
    CWnd* pGrp = GetDlgItem(IDC_GRP_LISTE);
    if (pGrp) {
        CRect rcGrp;
        pGrp->GetWindowRect(&rcGrp);
        ScreenToClient(&rcGrp);
        
        CRect rcWide(0, 0, 218, 0); // Always wide
        MapDialogRect(&rcWide);
        int newWidth = rcWide.Width(); 
        
        pGrp->SetWindowPos(nullptr, rcGrp.left, rcGrp.top, newWidth, grupIcYukseklik, SWP_NOZORDER | SWP_NOACTIVATE);
    }
    
    // Diyalog yuksekligini liste altina gore ayarla
    if (pGrp) {
        CRect rcGrpFinal;
        pGrp->GetWindowRect(&rcGrpFinal);
        ScreenToClient(&rcGrpFinal);
        
        int newBottom = rcGrpFinal.bottom + 10;
        
        if (m_wndStatusBar.GetSafeHwnd()) {
            CRect rcStatus;
            m_wndStatusBar.GetWindowRect(&rcStatus);
            ScreenToClient(&rcStatus);
            m_wndStatusBar.SetWindowPos(nullptr, rcStatus.left, newBottom, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            newBottom += rcStatus.Height() + 10;
        }
        
        CRect rcClient(0, 0, 100, newBottom);
        CalcWindowRect(&rcClient);
        int newH = rcClient.Height();
        
        int screenH = GetSystemMetrics(SM_CYSCREEN) - 100;
        if (newH > screenH) newH = screenH; // Ekrana sigmama durumunda max yukseklik
        
        CRect rcThis;
        GetWindowRect(&rcThis);
        SetWindowPos(nullptr, 0, 0, rcThis.Width(), newH, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void CSearchDlg::ClearDynamicCombos()
{
    for (auto p : m_combosTas) {
        if (::IsWindow(p->GetSafeHwnd())) {
            p->DestroyWindow();
        }
        delete p;
    }
    m_combosTas.clear();
    
    for (auto p : m_combosTip) {
        if (::IsWindow(p->GetSafeHwnd())) {
            p->DestroyWindow();
        }
        delete p;
    }
    m_combosTip.clear();
}

HBRUSH CSearchDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    
    if (nCtlColor == CTLCOLOR_STATIC || nCtlColor == CTLCOLOR_BTN) {
        int ctrlID = pWnd->GetDlgCtrlID();
        if (ctrlID == 3000) {
            pDC->SetTextColor(RGB(255, 128, 0));
            pDC->SetBkColor(RGB(0, 0, 0));
            return (HBRUSH)GetStockObject(BLACK_BRUSH);
        }

        if (nCtlColor == CTLCOLOR_BTN) {
            TCHAR szClassName[256];
            GetClassName(pWnd->GetSafeHwnd(), szClassName, 255);
            if (_tcsicmp(szClassName, _T("Button")) == 0) {
                LONG_PTR style = GetWindowLongPtr(pWnd->GetSafeHwnd(), GWL_STYLE);
                int btnType = style & 0xF;
                if (btnType != BS_CHECKBOX && btnType != BS_AUTOCHECKBOX && 
                    btnType != BS_RADIOBUTTON && btnType != BS_AUTORADIOBUTTON) {
                    return hbr;
                }
            } else {
                return hbr;
            }
        }

        CRect rc;
        pWnd->GetWindowRect(&rc);
        CPoint pt = rc.CenterPoint();
        ScreenToClient(&pt);
        
        pDC->SetBkMode(TRANSPARENT);
        
        struct ColorGroup { int id; CBrush* pBrush; };
        ColorGroup groups[] = {
            { IDC_GRP_ARA, &m_brushAra },
            { IDC_GRP_OLCU, &m_brushOlcu },
            { IDC_GRP_KESISEN, &m_brushKesisen },
            { IDC_GRP_EXPORT, &m_brushExport },
            { IDC_GRP_LISTE, &m_brushListe },
            { IDC_GRP_KALIP_BILGI, &m_brushKalipBilgi },
            { IDC_GRP_KALIP_DUZENLE_AYAR2, &m_brushKalipBilgi }
        };
        
        for (int i = 0; i < sizeof(groups)/sizeof(groups[0]); i++) {
            if (!m_bListOpen && groups[i].id == IDC_GRP_LISTE) continue;
            if (!m_bKalip && (groups[i].id == IDC_GRP_KALIP_BILGI || groups[i].id == IDC_GRP_KALIP_DUZENLE_AYAR2)) continue;
            
            CWnd* pGrp = GetDlgItem(groups[i].id);
            if (pGrp && pGrp->IsWindowVisible()) {
                CRect rcGrp;
                pGrp->GetWindowRect(&rcGrp);
                ScreenToClient(&rcGrp);
                if (rcGrp.PtInRect(pt)) {
                    pDC->SetTextColor(RGB(0, 0, 0));
                    return (HBRUSH)groups[i].pBrush->GetSafeHandle();
                }
            }
        }
    }
    return hbr;
}

void CSearchDlg::OnBnClickedChkHesap()
{
    UpdateData(TRUE);
    int nShow = m_bHesap ? SW_SHOW : SW_HIDE;
    if (::IsWindow(m_listHesap.GetSafeHwnd())) m_listHesap.ShowWindow(nShow);
    if (CWnd* pBtn = GetDlgItem(IDC_BTN_HESAPLA)) pBtn->ShowWindow(nShow);
    // if (CWnd* pTxt = GetDlgItem(IDC_STATIC_TAS_TOPLAM)) pTxt->ShowWindow(nShow);
    UpdateDialogWidth();
}

void CSearchDlg::OnBnClickedBtnHesapla()
{
    if (!m_pApp) return;
    
    m_listHesap.DeleteAllItems();
    long totalStones = 0;
    
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    
    IDispatch* pPage = GetDispatchProp(pDoc, L"ActivePage");
    if (!pPage) return;
    
    IDispatch* pShapes = GetDispatchProp(pPage, L"Shapes");
    if (!pShapes) return;
    
    IDispatch* pShapeRange = FindShapes(pShapes, _T(""), 0, true);
    if (pShapeRange) {
        long count = GetLongProp(pShapeRange, L"Count");
        
        int listIndex = 0;
        for (long i = 1; i <= count; ++i) {
                    VARIANTARG itemArgs[1];
                    itemArgs[0].vt = VT_I4; itemArgs[0].lVal = i;
                    DISPPARAMS itemParams = { itemArgs, NULL, 1, 0 };
                    VARIANT itemResult; VariantInit(&itemResult);
                    
                    if (SUCCEEDED(pShapeRange->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &itemParams, &itemResult, NULL, NULL))) {
                        if (itemResult.vt == VT_DISPATCH && itemResult.pdispVal) {
                            IDispatch* pShape = itemResult.pdispVal;
                            IDispatch* pText = GetDispatchProp(pShape, L"Text");
                            if (pText) {
                                IDispatch* pStory = GetDispatchProp(pText, L"Story");
                                if (pStory) {
                                    CString textContent = GetStringProp(pStory, L"Text");
                                    CString checkText = textContent;
                                    checkText.Trim();
                                    checkText.MakeLower();
                                    if (checkText.Find(_T("ztrass.com")) == 0 || checkText.Find(_T("ztrass")) >= 0) {
                                        AfxMessageBox(textContent);
                                        CString firma, dosya, presStr, montajStr, kalip, enStr, boyStr, renkStr, kalipIs, topIs, tarih, tur; CStringArray renkListesi;
                                        double en = 0, boy = 0;
                                        int renk = 0, kIs = 1, tIs = 1;
                                        
                                        int pos = 0;
                                        CString line = textContent.Tokenize(_T("\r\n"), pos);
                                        while (!line.IsEmpty()) {
                                            line.Trim();
                                            CString lowerLine = line;
                                            lowerLine.MakeLower();

                                            if (line.Find(_T(".")) > 0 && line.Find(_T(":")) > 0 && line.GetLength() <= 30 && lowerLine.Find(_T("dosya")) < 0 && lowerLine.Find(_T("firma")) < 0 && lowerLine.Find(_T("toplam")) < 0 && lowerLine.Find(_T("kal")) < 0) {
                                                tarih = line;
                                            }
                                            int colonPos = line.Find(_T(":"));
                                            if (colonPos > 0) {
                                                CString val = line.Mid(colonPos + 1).Trim();
                                                
                                                if (lowerLine.Find(_T("firma")) >= 0 || lowerLine.Find(_T("company")) >= 0) {
                                                    firma = val;
                                                    SetDlgItemText(IDC_EDT_FIRMA, val);
                                                }
                                                else if (lowerLine.Find(_T("dosya")) >= 0 || lowerLine.Find(_T("file")) >= 0) {
                                                    dosya = val;
                                                    SetDlgItemText(IDC_EDT_DOSYA, val);
                                                }
                                                else if ((lowerLine.Find(_T("kal")) >= 0 || lowerLine.Find(_T("pat")) >= 0) && lowerLine.Find(_T("ad")) >= 0 && lowerLine.Find(_T("taki")) < 0) {
                                                    kalip = val;
                                                    SetDlgItemText(IDC_EDT_KALIP_ADI, val);
                                                }
                                                else if ((lowerLine.Find(_T("kal")) >= 0) && (lowerLine.Find(_T("l\xE7\xFC")) >= 0 || lowerLine.Find(_T("lcu")) >= 0 || lowerLine.Find(_T("size")) >= 0)) {
                                                      CString olcuStr = val;
                                                      olcuStr.MakeLower();
                                                      olcuStr.Replace(_T("cm"), _T(""));
                                                      olcuStr.Replace(_T(" "), _T(""));
                                                      olcuStr.Replace(_T(","), _T("."));
                                                      int xPos = olcuStr.Find(_T("x"));
                                                      if (xPos > 0) {
                                                          enStr = olcuStr.Left(xPos);
                                                          boyStr = olcuStr.Mid(xPos + 1);
                                                          enStr.Trim(); boyStr.Trim();
                                                          en = _tstof(enStr);
                                                          boy = _tstof(boyStr);
                                                          SetDlgItemText(IDC_EDT_EN, enStr);
                                                          SetDlgItemText(IDC_EDT_BOY, boyStr);
                                                      }
                                                  }
                                                  else if (lowerLine.Find(_T("kal")) >= 0 && lowerLine.Find(_T("taki")) >= 0) {
                                                      kIs = _tstoi(val);
                                                      SetDlgItemText(IDC_EDT_KALIPTA_IS, val);
                                                  }
                                                  else if (lowerLine.Find(_T("toplam")) >= 0) {
                                                      tIs = _tstoi(val);
                                                      SetDlgItemText(IDC_EDT_TOPLAM_IS, val);
                                                  }
                                                  else if (lowerLine.Find(_T("renk")) >= 0) {
                                                      renk = _tstoi(val);
                                                  }
                                              } else {
                                                  if (line.Find(_T("-")) > 0) {
                                                      renkListesi.Add(line);
                                                  }
                                              }
                                              pos = textContent.Find(_T("\r\n"), pos);
                                              if (pos >= 0) { pos += 2; line = textContent.Tokenize(_T("\r\n"), pos); } else { line = _T(""); }
                                          }
                                          
                                          m_listHesap.DeleteAllItems();
                                          double m1Bant = (renk * en) / 100.0;
                                          int dizim = (kIs > 0) ? (tIs / kIs) : 0;
                                          int topBant = (int)(m1Bant * tIs + 0.5);

                                          CString sRenk, sKIs, sDizim, sCombinedOlcu, sCombinedBant, sTopAdet;
                                          sRenk.Format(_T("%d"), renk);
                                          sKIs.Format(_T("%d"), kIs);
                                          sDizim.Format(_T("%d"), dizim);
                                          sTopAdet.Format(_T("%d"), tIs);
                                          sCombinedOlcu.Format(_T("%.1f X %.1f"), en, boy);
                                          sCombinedBant.Format(_T("%.2f / %d"), m1Bant, topBant);
                                          
                                          int listIndex = 0;
                                          m_listHesap.InsertItem(listIndex, _T("Kal\x0131p"));
                                          m_listHesap.SetItemData(listIndex, 888);
                                          m_listHesap.SetItemText(listIndex, 1, _T("Renk Say."));
                                          m_listHesap.SetItemText(listIndex, 2, _T("Kal\x0131p \xD6l\xE7\xFCs\xFC"));
                                          m_listHesap.SetItemText(listIndex, 3, _T("K.is"));
                                          m_listHesap.SetItemText(listIndex, 4, _T("Bant(m)"));
                                          m_listHesap.SetItemText(listIndex, 5, _T("Plaka"));
                                          m_listHesap.SetItemText(listIndex, 6, _T("Toplam Adet"));
                                          m_listHesap.SetItemText(listIndex, 7, _T("T\xFCr"));
                                          m_listHesap.SetItemText(listIndex, 8, _T("Tarih"));
                                          listIndex++;
                                          
                                          m_listHesap.InsertItem(listIndex, kalip);
                                          m_listHesap.SetItemText(listIndex, 1, sRenk);
                                          m_listHesap.SetItemText(listIndex, 2, sCombinedOlcu);
                                          m_listHesap.SetItemText(listIndex, 3, sKIs);
                                          m_listHesap.SetItemText(listIndex, 4, sCombinedBant);
                                          m_listHesap.SetItemText(listIndex, 5, sDizim);
                                          m_listHesap.SetItemText(listIndex, 6, sTopAdet);
                                          m_listHesap.SetItemText(listIndex, 7, tur);
                                          m_listHesap.SetItemText(listIndex, 8, tarih);
                                          listIndex++;

                                          if (renkListesi.GetSize() > 0) {
                                              m_listHesap.InsertItem(listIndex, _T("Ta\x015F Rengi"));
                                              m_listHesap.SetItemData(listIndex, 888);
                                              m_listHesap.SetItemText(listIndex, 1, _T("Ta\x015F \xD6l\xE7\xFCs\xFC"));
                                              m_listHesap.SetItemText(listIndex, 2, _T("Adet"));
                                              m_listHesap.SetItemText(listIndex, 3, _T("Tip"));
                                              m_listHesap.SetItemText(listIndex, 4, _T(""));
                                              m_listHesap.SetItemText(listIndex, 5, _T(""));
                                              m_listHesap.SetItemText(listIndex, 6, _T(""));
                                              m_listHesap.SetItemText(listIndex, 7, _T(""));
                                              m_listHesap.SetItemText(listIndex, 8, _T(""));
                                              listIndex++;

                                              for (int i = 0; i < renkListesi.GetSize(); i++) {
                                                  CString rLine = renkListesi.GetAt(i);
                                                  CString rOlcu, rAdet, rRenk, rTur;
                                                  int p1 = rLine.Find(_T("-"));
                                                  if (p1 > 0) {
                                                      rOlcu = rLine.Left(p1).Trim();
                                                      int p2 = rLine.Find(_T("-"), p1 + 1);
                                                      if (p2 > 0) {
                                                          rAdet = rLine.Mid(p1 + 1, p2 - p1 - 1).Trim();
                                                          int p3 = rLine.Find(_T("-"), p2 + 1);
                                                          if (p3 > 0) {
                                                              rRenk = rLine.Mid(p2 + 1, p3 - p2 - 1).Trim();
                                                              rTur = rLine.Mid(p3 + 1).Trim();
                                                              rTur.Replace(_T("-zl"), _T("\xD6zel"));
                                                          } else {
                                                              rRenk = rLine.Mid(p2 + 1).Trim();
                                                          }
                                                      }
                                                  }
                                                  m_listHesap.InsertItem(listIndex, rRenk);
                                                  m_listHesap.SetItemData(listIndex, 999);
                                                  m_listHesap.SetItemText(listIndex, 1, rOlcu);
                                                  m_listHesap.SetItemText(listIndex, 2, rAdet);
                                                  m_listHesap.SetItemText(listIndex, 3, rTur);
                                                  m_listHesap.SetItemText(listIndex, 4, _T(""));
                                                  m_listHesap.SetItemText(listIndex, 5, _T(""));
                                                  m_listHesap.SetItemText(listIndex, 6, _T(""));
                                                  m_listHesap.SetItemText(listIndex, 7, _T(""));
                                                  m_listHesap.SetItemText(listIndex, 8, _T(""));
                                                  listIndex++;
                                              }
                                          }
                                      }
                                  }
}

                }
            }
        }
    }
}
void CSearchDlg::OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
    *pResult = CDRF_DODEFAULT;

    if (pLVCD->nmcd.dwDrawStage == CDDS_PREPAINT) {
        *pResult = CDRF_NOTIFYITEMDRAW;
    }
    // ====================================================================
    // 1. ASAMA: SATIRIN TAMAMINI (BOSLUKLAR DAHIL) BOYADIGIMIZ YER
    // ====================================================================
    else if (pLVCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
        int nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);
        CString col0Text = m_listHesap.GetItemText(nItem, 0);
        CString lowerCol0 = col0Text;
        lowerCol0.MakeLower();
        
        if (m_listHesap.GetItemData(nItem) == 888 || lowerCol0.Find(_T("kal")) >= 0 || lowerCol0.Find(_T("tas")) >= 0 || lowerCol0.Find(_T("ta\x015F")) >= 0) {
            pLVCD->clrTextBk = RGB(210, 235, 255);
            pLVCD->clrText = RGB(0, 0, 0);
            *pResult = CDRF_NEWFONT;
        } else {
            *pResult = CDRF_NOTIFYITEMDRAW;
        }
    }
    else if (pLVCD->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM)) {
        int nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);
        int nSubItem = pLVCD->iSubItem;

        CString col0Text = m_listHesap.GetItemText(nItem, 0);
        if (m_listHesap.GetItemData(nItem) == 999) { // Eger renk satiri ise
            if (nSubItem == 0) {
                CString sRenk = col0Text; // 0 nolu Sutun = Renk Adi
            sRenk.MakeLower();
            
            if (!sRenk.IsEmpty()) {
                COLORREF bgColor = RGB(255, 255, 255);
                BOOL bFound = FALSE;
                
                if (sRenk.Find(_T("rose pink")) >= 0) { bgColor = RGB(255, 153, 204); bFound = TRUE; }
                else if (sRenk.Find(_T("siam ab")) >= 0) { bgColor = RGB(220, 20, 60); bFound = TRUE; }
                else if (sRenk.Find(_T("siam")) >= 0) { bgColor = RGB(255, 0, 0); bFound = TRUE; }
                else if (sRenk.Find(_T("ab crystal")) >= 0 || sRenk.Find(_T("crystal ab")) >= 0) { bgColor = RGB(224, 255, 255); bFound = TRUE; }
                else if (sRenk.Find(_T("crystal")) >= 0) { bgColor = RGB(245, 245, 245); bFound = TRUE; }
                else if (sRenk.Find(_T("light brown")) >= 0 || sRenk.Find(_T("brown")) >= 0) { bgColor = RGB(165, 42, 42); bFound = TRUE; }
                else if (sRenk.Find(_T("jet black")) >= 0 || sRenk.Find(_T("black")) >= 0) { bgColor = RGB(30, 30, 30); bFound = TRUE; }
                else if (sRenk.Find(_T("rainbow")) >= 0) { bgColor = RGB(255, 140, 0); bFound = TRUE; }
                else if (sRenk.Find(_T("emerald")) >= 0 || sRenk.Find(_T("green")) >= 0) { bgColor = RGB(0, 128, 0); bFound = TRUE; }
                else if (sRenk.Find(_T("sapphire")) >= 0 || sRenk.Find(_T("blue")) >= 0) { bgColor = RGB(0, 0, 255); bFound = TRUE; }
                else if (sRenk.Find(_T("topaz")) >= 0 || sRenk.Find(_T("yellow")) >= 0) { bgColor = RGB(255, 215, 0); bFound = TRUE; }
                else if (sRenk.Find(_T("amethyst")) >= 0 || sRenk.Find(_T("purple")) >= 0) { bgColor = RGB(128, 0, 128); bFound = TRUE; }
                else { bgColor = RGB(200, 200, 200); bFound = TRUE; } // Digerleri icin gri
                
                if (bFound) {
                    pLVCD->clrTextBk = bgColor;
                    int luminance = (299 * GetRValue(bgColor) + 587 * GetGValue(bgColor) + 114 * GetBValue(bgColor)) / 1000;
                    if (luminance < 128) pLVCD->clrText = RGB(255, 255, 255);
                    else pLVCD->clrText = RGB(0, 0, 0);
                    *pResult = CDRF_NEWFONT;
                }
            }
        }
        else {
            CString lowerCol0 = col0Text;
            lowerCol0.MakeLower();

            if (m_listHesap.GetItemData(nItem) == 888 || lowerCol0.Find(_T("kal")) >= 0 || lowerCol0.Find(_T("tas")) >= 0 || lowerCol0.Find(_T("ta\x015F")) >= 0) {
            pLVCD->clrTextBk = RGB(210, 235, 255); // Acik pastel mavi
                pLVCD->clrText = RGB(0, 0, 0); // Siyah yazi
            *pResult = CDRF_NEWFONT;
        } else {
                pLVCD->clrTextBk = GetSysColor(COLOR_WINDOW);
                pLVCD->clrText = GetSysColor(COLOR_WINDOWTEXT);
                *pResult = CDRF_NEWFONT;
            }
        }
    }
}

}

// ==============================================================================
// SILINEN 17 FONKSIYONUN GECICI SABLONLARI
// (Buradaki MessageBox uyarilarini silip orijinal kodlarinizi iclerine yaziniz)
// ==============================================================================

void CSearchDlg::OnBnClickedBtnAra2()
{
    CString strEn, strBoy;
    GetDlgItemText(IDC_EDT_EN, strEn);
    GetDlgItemText(IDC_EDT_BOY, strBoy);
    double w = _wtof(strEn);
    double h = _wtof(strBoy);
    if (w <= 0 || h <= 0) {
        return;
    }
    ResizeShapes(w, h);
}

void CSearchDlg::OnBnClickedAra3Range(UINT nID)
{
    double sizes[] = {2.0, 2.1, 2.15, 3.0, 3.1, 3.15, 0.4, 0.6};
    int idx = nID - IDC_BTN_OLCU_1;
    if (idx >= 0 && idx < 8) {
        double s = sizes[idx];
        CString str;
        str.Format(L"%.2f", s);
        str.TrimRight(L"0");
        str.TrimRight(L".");
        SetDlgItemText(IDC_EDT_EN, str);
        SetDlgItemText(IDC_EDT_BOY, str);
        ResizeShapes(s, s);
    }
}

#include <cmath>

void CSearchDlg::ResizeShapes(double w_mm, double h_mm)
{
    if (!m_pApp) return;
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Boyutlandir");
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    
    SetLongProp(pDoc, L"Unit", 3);
    SetLongProp(pDoc, L"ReferencePoint", 5);
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pSel) {
        IDispatch* pShapesColl = GetDispatchProp(pSel, L"Shapes");
        IDispatch* pFlatRange = nullptr;
        if (pShapesColl) {
            DISPID dispFind; OLECHAR* szFind = (OLECHAR*)L"FindShapes";
            if (SUCCEEDED(pShapesColl->GetIDsOfNames(IID_NULL, &szFind, 1, LOCALE_USER_DEFAULT, &dispFind))) {
                DISPPARAMS params = { NULL, NULL, 0, 0 };
                VARIANT retVal; VariantInit(&retVal);
                if (SUCCEEDED(pShapesColl->Invoke(dispFind, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                    if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                        pFlatRange = retVal.pdispVal;
                    }
                }
            }
            pShapesColl->Release();
        }
        
        if (pFlatRange) {
            long count = GetLongProp(pFlatRange, L"Count");
            if (count > 0) {
                bool bCircleOnly = (IsDlgButtonChecked(IDC_CHK_OLCU2) == BST_CHECKED);
                if (bCircleOnly) h_mm = w_mm;
                
                for (long i = count; i >= 1; --i) {
                    IDispatch* pShape = GetDispatchPropWithIntArg(pFlatRange, L"Item", i);
                    if (pShape) {
                        long type = GetLongProp(pShape, L"Type");
                        bool isRectOrCurve = (type == 3 || type == 5);
                        
                        if (bCircleOnly && !isRectOrCurve) {
                            double cx = GetDoubleProp(pShape, L"CenterX");
                            double cy = GetDoubleProp(pShape, L"CenterY");
                            IDispatch* pLayer = GetDispatchProp(pShape, L"Layer");
                            if (pLayer) {
                                DISPID dispidCreate;
                                OLECHAR* szCreate = (OLECHAR*)L"CreateEllipse2";
                                if (SUCCEEDED(pLayer->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dispidCreate))) {
                                    VARIANT args[3]; VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]);
                                    args[2].vt = VT_R8; args[2].dblVal = cx;
                                    args[1].vt = VT_R8; args[1].dblVal = cy;
                                    args[0].vt = VT_R8; args[0].dblVal = w_mm / 2.0;
                                    DISPPARAMS params2 = { args, NULL, 3, 0 };
                                    VARIANT retVal2; VariantInit(&retVal2);
                                    if (SUCCEEDED(pLayer->Invoke(dispidCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params2, &retVal2, NULL, NULL))) {
                                        if (retVal2.vt == VT_DISPATCH && retVal2.pdispVal) {
                                            IDispatch* pNewShape = retVal2.pdispVal;
                                            
                                            // Orijinal dolgu rengini kopyala
                                            IDispatch* pOrigFill = GetDispatchProp(pShape, L"Fill");
                                            if (pOrigFill) {
                                                long fillType = GetLongProp(pOrigFill, L"Type");
                                                if (fillType == 1) {
                                                    IDispatch* pOrigColor = GetDispatchProp(pOrigFill, L"UniformColor");
                                                    if (pOrigColor) {
                                                        IDispatch* pNewFill = GetDispatchProp(pNewShape, L"Fill");
                                                        if (pNewFill) {
                                                            DISPID dApply; OLECHAR* szApply = (OLECHAR*)L"ApplyUniformFill";
                                                            if (SUCCEEDED(pNewFill->GetIDsOfNames(IID_NULL, &szApply, 1, LOCALE_USER_DEFAULT, &dApply))) {
                                                                VARIANT argC; VariantInit(&argC); argC.vt = VT_DISPATCH; argC.pdispVal = pOrigColor;
                                                                DISPPARAMS prmsC = { &argC, NULL, 1, 0 };
                                                                pNewFill->Invoke(dApply, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsC, NULL, NULL, NULL);
                                                            }
                                                            pNewFill->Release();
                                                        }
                                                        pOrigColor->Release();
                                                    }
                                                }
                                                pOrigFill->Release();
                                            }
                                            
                                            // Orijinal �izgi rengini kopyala
                                            IDispatch* pOrigOutline = GetDispatchProp(pShape, L"Outline");
                                            if (pOrigOutline) {
                                                long outType = GetLongProp(pOrigOutline, L"Type");
                                                if (outType != 0) {
                                                    IDispatch* pOrigColor = GetDispatchProp(pOrigOutline, L"Color");
                                                    if (pOrigColor) {
                                                        IDispatch* pNewOutline = GetDispatchProp(pNewShape, L"Outline");
                                                        if (pNewOutline) {
                                                            SetDoubleProp(pNewOutline, L"Width", GetDoubleProp(pOrigOutline, L"Width"));
                                                            IDispatch* pNewColor = GetDispatchProp(pNewOutline, L"Color");
                                                            if (pNewColor) {
                                                                DISPID dCopy; OLECHAR* szCopy = (OLECHAR*)L"CopyAssign";
                                                                if (SUCCEEDED(pNewColor->GetIDsOfNames(IID_NULL, &szCopy, 1, LOCALE_USER_DEFAULT, &dCopy))) {
                                                                    VARIANT argC; VariantInit(&argC); argC.vt = VT_DISPATCH; argC.pdispVal = pOrigColor;
                                                                    DISPPARAMS prmsC = { &argC, NULL, 1, 0 };
                                                                    pNewColor->Invoke(dCopy, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsC, NULL, NULL, NULL);
                                                                }
                                                                pNewColor->Release();
                                                            }
                                                            pNewOutline->Release();
                                                        }
                                                        pOrigColor->Release();
                                                    }
                                                }
                                                pOrigOutline->Release();
                                            }

                                            DISPID dispidDelete;
                                            OLECHAR* szDelete = (OLECHAR*)L"Delete";
                                            if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szDelete, 1, LOCALE_USER_DEFAULT, &dispidDelete))) {
                                                DISPPARAMS delParams = { NULL, NULL, 0, 0 };
                                                pShape->Invoke(dispidDelete, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &delParams, NULL, NULL, NULL);
                                            }
                                            pNewShape->Release();
                                        }
                                    }
                                }
                                pLayer->Release();
                            }
                        } else {
                            if (type == 2) { // cdrEllipseShape
                                DISPID dispid; OLECHAR* szSetSize = (OLECHAR*)L"SetSize";
                                if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szSetSize, 1, LOCALE_USER_DEFAULT, &dispid))) {
                                    VARIANT args[2]; VariantInit(&args[0]); VariantInit(&args[1]);
                                    args[1].vt = VT_R8; args[1].dblVal = w_mm;
                                    args[0].vt = VT_R8; args[0].dblVal = bCircleOnly ? w_mm : h_mm;
                                    DISPPARAMS params3 = { args, NULL, 2, 0 };
                                    pShape->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params3, NULL, NULL, NULL);
                                }
                            } else if (type == 3 || type == 5) { // cdrRectangleShape veya cdrCurveShape
                                double N = GetDoubleProp(pShape, L"RotationAngle");
                                double cx = GetDoubleProp(pShape, L"CenterX");
                                double cy = GetDoubleProp(pShape, L"CenterY");
                                
                                DISPID dispRotate = 0; OLECHAR* szRotate = (OLECHAR*)L"Rotate";
                                DISPID dispSetSize = 0; OLECHAR* szSetSize = (OLECHAR*)L"SetSize";
                                pShape->GetIDsOfNames(IID_NULL, &szRotate, 1, LOCALE_USER_DEFAULT, &dispRotate);
                                pShape->GetIDsOfNames(IID_NULL, &szSetSize, 1, LOCALE_USER_DEFAULT, &dispSetSize);
                                
                                // 1. Açıyı düzelt (Kendi merkezinden döndür)
                                if (N != 0.0 && dispRotate) {
                                    VARIANT argsRot[3]; VariantInit(&argsRot[0]); VariantInit(&argsRot[1]); VariantInit(&argsRot[2]);
                                    argsRot[2].vt = VT_R8; argsRot[2].dblVal = -N;
                                    argsRot[1].vt = VT_R8; argsRot[1].dblVal = cx;
                                    argsRot[0].vt = VT_R8; argsRot[0].dblVal = cy;
                                    DISPPARAMS paramsRot = { argsRot, NULL, 3, 0 };
                                    pShape->Invoke(dispRotate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsRot, NULL, NULL, NULL);
                                }
                                
                                // 2. Boyutlandır
                                if (dispSetSize) {
                                    VARIANT args[2]; VariantInit(&args[0]); VariantInit(&args[1]);
                                    args[1].vt = VT_R8; args[1].dblVal = w_mm;
                                    args[0].vt = VT_R8; args[0].dblVal = bCircleOnly ? w_mm : h_mm;
                                    DISPPARAMS paramsSize = { args, NULL, 2, 0 };
                                    pShape->Invoke(dispSetSize, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsSize, NULL, NULL, NULL);
                                }
                                
                                // 3. Eski açısına döndür (Kendi merkezinden döndür)
                                if (N != 0.0 && dispRotate) {
                                    VARIANT argsRot[3]; VariantInit(&argsRot[0]); VariantInit(&argsRot[1]); VariantInit(&argsRot[2]);
                                    argsRot[2].vt = VT_R8; argsRot[2].dblVal = N;
                                    argsRot[1].vt = VT_R8; argsRot[1].dblVal = cx;
                                    argsRot[0].vt = VT_R8; argsRot[0].dblVal = cy;
                                    DISPPARAMS paramsRot = { argsRot, NULL, 3, 0 };
                                    pShape->Invoke(dispRotate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsRot, NULL, NULL, NULL);
                                }
                                
                                // 4. Merkez koordinatlarn koru
                                SetDoubleProp(pShape, L"CenterX", cx);
                                SetDoubleProp(pShape, L"CenterY", cy);
                            }
                        }
                        pShape->Release();
                    }
                }
            }
            pFlatRange->Release();
        }
        pSel->Release();
    }
    pDoc->Release();
}

void CSearchDlg::OnBnClickedBtnAra5()
{
    if (!m_pApp) return;
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (!pSel) return;
    long count = GetLongProp(pSel, L"Count");
    if (count == 0) { pSel->Release(); return; }

    for (long i = 1; i <= count; i++) {
        VARIANT arg; VariantInit(&arg); arg.vt = VT_I4; arg.lVal = i;
        DISPPARAMS prms = { &arg, NULL, 1, 0 };
        VARIANT res; VariantInit(&res);
        if (SUCCEEDED(pSel->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &prms, &res, NULL, NULL)) && res.vt == VT_DISPATCH) {
            IDispatch* pShape = res.pdispVal;
            if (pShape) {
                IDispatch* pOutline = GetDispatchProp(pShape, L"Outline");
                IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
                if (pOutline && pFill) {
                    IDispatch* pOutlineColor = GetDispatchProp(pOutline, L"Color");
                    if (pOutlineColor) {
                        DISPID dApply; OLECHAR* szApply = (OLECHAR*)L"ApplyUniformFill";
                        if (SUCCEEDED(pFill->GetIDsOfNames(IID_NULL, &szApply, 1, LOCALE_USER_DEFAULT, &dApply))) {
                            VARIANT argF; VariantInit(&argF); argF.vt = VT_DISPATCH; argF.pdispVal = pOutlineColor;
                            DISPPARAMS params = { &argF, NULL, 1, 0 };
                            pFill->Invoke(dApply, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, NULL, NULL, NULL);
                        }
                        pOutlineColor->Release();
                    }
                    pFill->Release();
                    pOutline->Release();
                }
                pShape->Release();
            }
        }
    }
    pSel->Release();
}

void CSearchDlg::OnBnClickedBtnAra6()
{
    if (!m_pApp) return;
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (!pSel) return;
    long count = GetLongProp(pSel, L"Count");
    if (count == 0) { pSel->Release(); return; }

    for (long i = 1; i <= count; i++) {
        VARIANT arg; VariantInit(&arg); arg.vt = VT_I4; arg.lVal = i;
        DISPPARAMS prms = { &arg, NULL, 1, 0 };
        VARIANT res; VariantInit(&res);
        if (SUCCEEDED(pSel->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &prms, &res, NULL, NULL)) && res.vt == VT_DISPATCH) {
            IDispatch* pShape = res.pdispVal;
            if (pShape) {
                IDispatch* pOutline = GetDispatchProp(pShape, L"Outline");
                IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
                if (pOutline && pFill) {
                    long fillType = GetLongProp(pFill, L"Type");
                    if (fillType == 1) { // cdrUniformFill
                        IDispatch* pFillColor = GetDispatchProp(pFill, L"UniformColor");
                        if (pFillColor) {
                            IDispatch* pOutlineColor = GetDispatchProp(pOutline, L"Color");
                            if (pOutlineColor) {
                                DISPID dCopy; OLECHAR* szCopy = (OLECHAR*)L"CopyAssign";
                                if (SUCCEEDED(pOutlineColor->GetIDsOfNames(IID_NULL, &szCopy, 1, LOCALE_USER_DEFAULT, &dCopy))) {
                                    VARIANT argC; VariantInit(&argC); argC.vt = VT_DISPATCH; argC.pdispVal = pFillColor;
                                    DISPPARAMS prmsC = { &argC, NULL, 1, 0 };
                                    pOutlineColor->Invoke(dCopy, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsC, NULL, NULL, NULL);
                                }
                                pOutlineColor->Release();
                            }
                            
                            long outType = GetLongProp(pOutline, L"Type");
                            if (outType == 0) {
                                SetDoubleProp(pOutline, L"Width", 0.5);
                            }
                            pFillColor->Release();
                        }
                    }
                    pFill->Release();
                    pOutline->Release();
                }
                pShape->Release();
            }
        }
    }
    pSel->Release();
}

void CSearchDlg::OnBnClickedBtnAra7()
{
    if (!m_pApp) return;
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (!pSel) return;
    long count = GetLongProp(pSel, L"Count");
    if (count < 1) { pSel->Release(); return; }

    IDispatch* pLine = NULL;
    for (long i = 1; i <= count; i++) {
        VARIANT arg; VariantInit(&arg); arg.vt = VT_I4; arg.lVal = i;
        DISPPARAMS prms = { &arg, NULL, 1, 0 };
        VARIANT res; VariantInit(&res);
        if (SUCCEEDED(pSel->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &prms, &res, NULL, NULL)) && res.vt == VT_DISPATCH) {
            IDispatch* pShape = res.pdispVal;
            IDispatch* pCurve = GetDispatchProp(pShape, L"Curve");
            if (pCurve) {
                IDispatch* pNodes = GetDispatchProp(pCurve, L"Nodes");
                if (pNodes) {
                    long nodeCount = GetLongProp(pNodes, L"Count");
                    if (nodeCount == 2) {
                        pLine = pShape;
                        pLine->AddRef();
                        pNodes->Release();
                        pCurve->Release();
                        break;
                    }
                    pNodes->Release();
                }
                pCurve->Release();
            }
            pShape->Release();
        }
    }

    if (!pLine) { pSel->Release(); return; }

    double rotationAngle = 0.0;
    IDispatch* pCurve = GetDispatchProp(pLine, L"Curve");
    if (pCurve) {
        IDispatch* pNodes = GetDispatchProp(pCurve, L"Nodes");
        if (pNodes) {
            IDispatch* pNode1 = GetDispatchPropWithIntArg(pNodes, L"Item", 1);
            IDispatch* pNode2 = GetDispatchPropWithIntArg(pNodes, L"Item", 2);
            if (pNode1 && pNode2) {
                double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
                DISPID dGetPos; OLECHAR* szGetPos = (OLECHAR*)L"GetPosition";
                if (SUCCEEDED(pNode1->GetIDsOfNames(IID_NULL, &szGetPos, 1, LOCALE_USER_DEFAULT, &dGetPos))) {
                    VARIANT args1[2]; VariantInit(&args1[0]); VariantInit(&args1[1]);
                    args1[1].vt = VT_BYREF | VT_R8; args1[1].pdblVal = &x1;
                    args1[0].vt = VT_BYREF | VT_R8; args1[0].pdblVal = &y1;
                    DISPPARAMS params1 = { args1, NULL, 2, 0 };
                    pNode1->Invoke(dGetPos, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params1, NULL, NULL, NULL);

                    VARIANT args2[2]; VariantInit(&args2[0]); VariantInit(&args2[1]);
                    args2[1].vt = VT_BYREF | VT_R8; args2[1].pdblVal = &x2;
                    args2[0].vt = VT_BYREF | VT_R8; args2[0].pdblVal = &y2;
                    DISPPARAMS params2 = { args2, NULL, 2, 0 };
                    pNode2->Invoke(dGetPos, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params2, NULL, NULL, NULL);
                }
                pNode1->Release();
                pNode2->Release();

                double dx = x2 - x1;
                double dy = y2 - y1;
                double angle = atan2(dy, dx) * 180.0 / 3.141592653589793;
                double normAngle = angle;
                if (fabs(normAngle) <= 45.0) {
                    rotationAngle = -normAngle;
                } else {
                    if (normAngle > 0)
                        rotationAngle = -(normAngle - 90.0);
                    else
                        rotationAngle = -(normAngle + 90.0);
                }
            }
            pNodes->Release();
        }
        pCurve->Release();
    }

    if (fabs(rotationAngle) < 0.001) {
        pLine->Release(); pSel->Release();
        return;
    }

    OptimizeCorel(m_pApp, TRUE, L"A�� D�zelt");

    VARIANT argRot; VariantInit(&argRot);
    argRot.vt = VT_R8; argRot.dblVal = rotationAngle;

    for (long i = 1; i <= count; i++) {
        VARIANT arg; VariantInit(&arg); arg.vt = VT_I4; arg.lVal = i;
        DISPPARAMS prms = { &arg, NULL, 1, 0 };
        VARIANT res; VariantInit(&res);
        if (SUCCEEDED(pSel->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &prms, &res, NULL, NULL)) && res.vt == VT_DISPATCH) {
            IDispatch* pShape = res.pdispVal;
            DISPID dRotate; OLECHAR* szRotate = (OLECHAR*)L"Rotate";
            if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szRotate, 1, LOCALE_USER_DEFAULT, &dRotate))) {
                DISPPARAMS prmsR = { &argRot, NULL, 1, 0 };
                pShape->Invoke(dRotate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsR, NULL, NULL, NULL);
            }
            pShape->Release();
        }
    }

    OptimizeCorel(m_pApp, FALSE, L"A�� D�zelt");
    pLine->Release();
    pSel->Release();
}

void CSearchDlg::OnEnChangeEdtEn()
{
    CString strEn;
    GetDlgItemText(IDC_EDT_EN, strEn);
    SetDlgItemText(IDC_EDT_BOY, strEn);
}

void CSearchDlg::OnPaint()
{
    CPaintDC dc(this);
    
    struct ColorGroup { int id; CBrush* pBrush; };
    ColorGroup groups[] = {
        { IDC_GRP_ARA, &m_brushAra },
        { IDC_GRP_OLCU, &m_brushOlcu },
        { IDC_GRP_KESISEN, &m_brushKesisen },
        { IDC_GRP_EXPORT, &m_brushExport },
        { IDC_GRP_LISTE, &m_brushListe },
        { IDC_GRP_KALIP_BILGI, &m_brushKalipBilgi },
        { IDC_GRP_KALIP_DUZENLE_AYAR2, &m_brushKalipBilgi }
    };
    
    for (int i = 0; i < sizeof(groups)/sizeof(groups[0]); i++) {
        if (!m_bListOpen && groups[i].id == IDC_GRP_LISTE) continue;
        if (!m_bKalip && (groups[i].id == IDC_GRP_KALIP_BILGI || groups[i].id == IDC_GRP_KALIP_DUZENLE_AYAR2)) continue;
        
        CWnd* pGrp = GetDlgItem(groups[i].id);
        if (pGrp && pGrp->IsWindowVisible()) {
            CRect rcGrp;
            pGrp->GetWindowRect(&rcGrp);
            ScreenToClient(&rcGrp);
            
            CRect rcFill = rcGrp;
            rcFill.DeflateRect(2, 2);
            rcFill.top += 8;
            if (rcFill.bottom > rcFill.top && rcFill.right > rcFill.left) {
                dc.FillRect(&rcFill, groups[i].pBrush);
            }
        }
    }

    if (m_bListOpen && !m_listObjects.empty()) {
        CWnd* pGrp = GetDlgItem(IDC_GRP_LISTE);
        if (pGrp && pGrp->IsWindowVisible()) {
            CRect rcGrp;
            pGrp->GetWindowRect(&rcGrp);
            ScreenToClient(&rcGrp);
            
            int startY = rcGrp.top + 14;
            
            if (m_bListOption) {
                CDC* pGrpDC = pGrp->GetDC();
                if (pGrpDC) {
                    int rowH = 24;
                    CFont fontSmall;
                    fontSmall.CreatePointFont(70, _T("MS Shell Dlg"));
                    CFont* pOldFont = pGrpDC->SelectObject(&fontSmall);
                    int oldBkMode = pGrpDC->SetBkMode(TRANSPARENT);
                    COLORREF oldTextColor = pGrpDC->SetTextColor(RGB(0, 0, 0));
                    int localStartY = 14;
                    
                    for (size_t i = 0; i < m_listObjects.size(); ++i) {
                        int rowY = localStartY + (int)i * rowH;
                        CRect rcBox(8, rowY + 3, 8 + 70, rowY + 19);
                        
                        CBrush brushBg(m_listObjects[i].color);
                        CPen penBorder(PS_SOLID, 1, RGB(40, 40, 40));
                        CBrush* pOldBrush = pGrpDC->SelectObject(&brushBg);
                        CPen* pOldPen = pGrpDC->SelectObject(&penBorder);
                        
                        pGrpDC->RoundRect(&rcBox, CPoint(8, 8));
                        
                        pGrpDC->SelectObject(pOldBrush);
                        pGrpDC->SelectObject(pOldPen);
                        
                        COLORREF bgCol = m_listObjects[i].color;
                        int r = GetRValue(bgCol);
                        int g = GetGValue(bgCol);
                        int b = GetBValue(bgCol);
                        double luminance = 0.299 * r + 0.587 * g + 0.114 * b;
                        COLORREF textCol = (luminance > 150) ? RGB(0, 0, 0) : RGB(255, 255, 255);
                        
                        pGrpDC->SetTextColor(textCol);
                        
                        CRect rcText = rcBox;
                        rcText.top += 1;
                        pGrpDC->DrawText(m_listObjects[i].text, &rcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                    }
                    
                    pGrpDC->SetTextColor(oldTextColor);
                    pGrpDC->SetBkMode(oldBkMode);
                    pGrpDC->SelectObject(pOldFont);
                    pGrp->ReleaseDC(pGrpDC);
                }
            } else {
                int rowH = 24;
                int gap = 6;
                int grpW = rcGrp.Width();
                int colW = (grpW - 16 - (2 * gap)) / 3;
                
                CFont fontSmall;
                fontSmall.CreatePointFont(70, _T("MS Shell Dlg"));
                CFont* pOldFont = dc.SelectObject(&fontSmall);
                int oldBkMode = dc.SetBkMode(TRANSPARENT);
                COLORREF oldTextColor = dc.SetTextColor(RGB(0, 0, 0));
                
                for (size_t i = 0; i < m_listObjects.size(); ++i) {
                    int col = (int)i % 3;
                    int row = (int)i / 3;
                    int rowY = startY + row * rowH;
                    int x = rcGrp.left + 8 + col * (colW + gap);
                    
                    CRect rcBox(x, rowY + 3, x + colW, rowY + 19);
                    
                    CBrush brushBg(m_listObjects[i].color);
                    CPen penBorder(PS_SOLID, 1, RGB(40, 40, 40));
                    CBrush* pOldBrush = dc.SelectObject(&brushBg);
                    CPen* pOldPen = dc.SelectObject(&penBorder);
                    
                    dc.RoundRect(&rcBox, CPoint(8, 8));
                    
                    dc.SelectObject(pOldBrush);
                    dc.SelectObject(pOldPen);
                    
                    COLORREF bgCol = m_listObjects[i].color;
                    int r = GetRValue(bgCol);
                    int g = GetGValue(bgCol);
                    int b = GetBValue(bgCol);
                    double luminance = 0.299 * r + 0.587 * g + 0.114 * b;
                    COLORREF textCol = (luminance > 150) ? RGB(0, 0, 0) : RGB(255, 255, 255);
                    
                    dc.SetTextColor(textCol);
                    
                    CRect rcText = rcBox;
                    rcText.top += 1;
                    dc.DrawText(m_listObjects[i].text, &rcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                }
                
                dc.SetTextColor(oldTextColor);
                dc.SetBkMode(oldBkMode);
                dc.SelectObject(pOldFont);
            }
        }
    }
}

void CSearchDlg::OnBnClickedBtnList()
{
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Liste");
    SetRedraw(FALSE);
    UpdateData(TRUE);

    if (m_bListOpen) {
        m_bListOpen = FALSE;
        ClearDynamicCombos();
        m_bListOption = FALSE;
        UpdateData(FALSE);
        CheckDlgButton(IDC_CHK_LIST_OPTION, BST_UNCHECKED);
        CWnd* pGroup = GetDlgItem(IDC_GRP_LISTE);
        if (pGroup) {
            pGroup->ShowWindow(SW_HIDE);
            pGroup->SetWindowText(_T("Liste"));
        }
        m_listObjects.clear();
    }
    else {
        m_bListOpen = TRUE;
        m_bListOption = FALSE;
        CheckDlgButton(IDC_CHK_LIST_OPTION, BST_UNCHECKED);
        CWnd* pGroup = GetDlgItem(IDC_GRP_LISTE);
        if (pGroup) {
            pGroup->ShowWindow(SW_SHOW);
        }

        CString sMetin = _T("");
        m_parsedStoneDetails.clear(); // color -> (tasAdi, tipAdi)

        if (!m_pApp) {
            SetRedraw(TRUE);
            return;
        }
        IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
        if (!pDoc) {
            SetRedraw(TRUE);
            return;
        }
        
        SetLongProp(pDoc, L"Unit", 3); // cdrMillimeter

        IDispatch* pSelection = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
        if (pSelection) {
            long count = GetLongProp(pSelection, L"Count");
            
            // E�er se�ilen sadece 1 nesne varsa ve bu nesne kal�p �er�evesiyse (�rne�in geni�li�i > 20 mm ise):
            if (count == 1) {
                IDispatch* pSingleShape = GetDispatchPropWithIntArg(pSelection, L"Item", 1);
                if (pSingleShape) {
                    double left = GetDoubleProp(pSingleShape, L"LeftX");
                    double right = GetDoubleProp(pSingleShape, L"RightX");
                    double bottom = GetDoubleProp(pSingleShape, L"BottomY");
                    double top = GetDoubleProp(pSingleShape, L"TopY");
                    
                    // E�er bu nesne geni� veya y�ksekse (kal�p �er�evesi olabilir), i�indeki ta�lar� se�elim
                    double w = GetDoubleProp(pSingleShape, L"SizeWidth");
                    double h = GetDoubleProp(pSingleShape, L"SizeHeight");
                    if (w > 20.0 || h > 20.0) {
                        IDispatch* pPage = GetDispatchProp(pDoc, L"ActivePage");
                        if (pPage) {
                            DISPID dispSelect; OLECHAR* szSelect = (OLECHAR*)L"SelectShapesFromRectangle";
                            if (SUCCEEDED(pPage->GetIDsOfNames(IID_NULL, &szSelect, 1, LOCALE_USER_DEFAULT, &dispSelect))) {
                                VARIANT args[5]; for(int i=0; i<5; ++i) VariantInit(&args[i]);
                                args[4].vt = VT_R8; args[4].dblVal = left - 5.0;
                                args[3].vt = VT_R8; args[3].dblVal = top + 5.0;
                                args[2].vt = VT_R8; args[2].dblVal = right + 5.0;
                                args[1].vt = VT_R8; args[1].dblVal = bottom - 5.0;
                                args[0].vt = VT_BOOL; args[0].boolVal = VARIANT_TRUE; // deselectOld = True
                                
                                DISPPARAMS params = { args, NULL, 5, 0 };
                                VARIANT retSel; VariantInit(&retSel);
                                if (SUCCEEDED(pPage->Invoke(dispSelect, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retSel, NULL, NULL))) {
                                    pSelection->Release();
                                    pSelection = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
                                    if (pSelection) {
                                        count = GetLongProp(pSelection, L"Count");
                                    }
                                }
                            }
                            pPage->Release();
                        }
                    }
                    pSingleShape->Release();
                }
            }

            if (!pSelection) { 
                pDoc->Release(); 
                SetRedraw(TRUE);
                return; 
            }

            // Kal�p/se�im s�n�rlar�n� belirleyelim
            double selW = GetDoubleProp(pSelection, L"SizeWidth");
            double selH = GetDoubleProp(pSelection, L"SizeHeight");
            double selLeft = GetDoubleProp(pSelection, L"LeftX");
            double selTop = GetDoubleProp(pSelection, L"TopY");

            // --- ESK� METN� BULMA VE PARSE ETME (Parser) ---
            IDispatch* pPage = GetDispatchProp(pDoc, L"ActivePage");
            if (pPage) {
                IDispatch* pShapes = GetDispatchProp(pPage, L"Shapes");
                if (pShapes) {
                    DISPID dFind; OLECHAR* szFind = (OLECHAR*)L"FindShapes";
                    if (SUCCEEDED(pShapes->GetIDsOfNames(IID_NULL, &szFind, 1, LOCALE_USER_DEFAULT, &dFind))) {
                        // Query: artistic text nesnelerini arayal�m
                        VARIANT argQ; VariantInit(&argQ);
                        argQ.vt = VT_BSTR; argQ.bstrVal = SysAllocString(L"@type = 'text:artistic'");
                        DISPPARAMS prmsFind = { &argQ, NULL, 1, 0 };
                        VARIANT retFind; VariantInit(&retFind);
                        if (SUCCEEDED(pShapes->Invoke(dFind, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsFind, &retFind, NULL, NULL)) && retFind.vt == VT_DISPATCH) {
                            IDispatch* pTextRange = retFind.pdispVal;
                            if (pTextRange) {
                                long txtCount = GetLongProp(pTextRange, L"Count");
                                double minDistance = 999999.0;
                                IDispatch* pNearestText = nullptr;
                                
                                for (long t = 1; t <= txtCount; ++t) {
                                    IDispatch* pTShape = GetDispatchPropWithIntArg(pTextRange, L"Item", t);
                                    if (pTShape) {
                                        IDispatch* pTextObj = GetDispatchProp(pTShape, L"Text");
                                        if (pTextObj) {
                                            IDispatch* pStory = GetDispatchProp(pTextObj, L"Story");
                                            if (pStory) {
                                                double fontSize = GetDoubleProp(pStory, L"Size");
                                                // Boyutu 8.01pt olanlar� arayal�m
                                                if (fabs(fontSize - 8.01) < 0.05) {
                                                    double tLeft = GetDoubleProp(pTShape, L"LeftX");
                                                    double tTop = GetDoubleProp(pTShape, L"TopY");
                                                    
                                                    double dx = tLeft - selLeft;
                                                    double dy = tTop - selTop;
                                                    double dist = sqrt(dx*dx + dy*dy);
                                                    if (dist < minDistance) {
                                                        minDistance = dist;
                                                        if (pNearestText) pNearestText->Release();
                                                        pNearestText = pTShape;
                                                        pNearestText->AddRef();
                                                    }
                                                }
                                                pStory->Release();
                                            }
                                            pTextObj->Release();
                                        }
                                        pTShape->Release();
                                    }
                                }
                                
                                if (pNearestText) {
                                    IDispatch* pTextObj = GetDispatchProp(pNearestText, L"Text");
                                    if (pTextObj) {
                                        IDispatch* pStory = GetDispatchProp(pTextObj, L"Story");
                                        if (pStory) {
                                            sMetin = GetStringProp(pStory, L"Text");
                                            pStory->Release();
                                        }
                                        pTextObj->Release();
                                    }
                                    pNearestText->Release();
                                }
                                
                                // Renge g�re ta� detaylar�n� sayfadaki t�m 8.01pt boyutundaki textlerden �ekme:
                                for (long t = 1; t <= txtCount; ++t) {
                                    IDispatch* pTShape = GetDispatchPropWithIntArg(pTextRange, L"Item", t);
                                    if (pTShape) {
                                        IDispatch* pTextObj = GetDispatchProp(pTShape, L"Text");
                                        if (pTextObj) {
                                            IDispatch* pStory = GetDispatchProp(pTextObj, L"Story");
                                            if (pStory) {
                                                double fontSize = GetDoubleProp(pStory, L"Size");
                                                if (fabs(fontSize - 8.01) < 0.05) {
                                                    CString txt = GetStringProp(pStory, L"Text");
                                                    
                                                    COLORREF textCol = RGB(128, 128, 128);
                                                    IDispatch* pFill = GetDispatchProp(pTShape, L"Fill");
                                                    if (pFill) {
                                                        long fType = GetLongProp(pFill, L"Type");
                                                        if (fType == 1) {
                                                            IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                                                            if (pColor) {
                                                                textCol = GetColorRef(pColor);
                                                                pColor->Release();
                                                            }
                                                        }
                                                        pFill->Release();
                                                    }
                                                    
                                                    if (txt.Find(_T(" - ")) > 0) {
                                                        int pos = 0;
                                                        CString ssVal = txt.Tokenize(_T(" - "), pos);
                                                        CString tasVal = txt.Tokenize(_T(" - "), pos);
                                                        CString tipVal = txt.Tokenize(_T(" - "), pos);
                                                        ssVal.Trim(); tasVal.Trim(); tipVal.Trim();
                                                        if (!tasVal.IsEmpty() && !tipVal.IsEmpty()) {
                                                            m_parsedStoneDetails[textCol] = std::make_pair(tasVal, tipVal);
                                                        }
                                                    }
                                                }
                                                pStory->Release();
                                            }
                                            pTextObj->Release();
                                        }
                                        pTShape->Release();
                                    }
                                }
                                
                                pTextRange->Release();
                            }
                        }
                        SysFreeString(argQ.bstrVal);
                    }
                    pShapes->Release();
                }
                pPage->Release();
            }

            // Metin parse etme ve UI g�ncelleme
            if (!sMetin.IsEmpty()) {
                sMetin.Replace(_T("\r\n"), _T("\n"));
                sMetin.Replace(_T("\r"), _T("\n"));
                
                int curPos = 0;
                CString line = sMetin.Tokenize(_T("\n"), curPos);
                while (!line.IsEmpty()) {
                    line.Trim();
                    if (line.Find(_T(":")) > 0) {
                        int idxColon = line.Find(_T(":"));
                        CString key = line.Left(idxColon);
                        CString val = line.Mid(idxColon + 1);
                        key.Trim(); val.Trim();
                        
                        key.MakeLower();
                        if (key.Find(_T("firma")) >= 0) {
                            CComboBox* pComboFirma = (CComboBox*)GetDlgItem(IDC_EDT_FIRMA);
                            if (pComboFirma) {
                                int idxFind = pComboFirma->FindStringExact(-1, val);
                                if (idxFind == CB_ERR) {
                                    idxFind = pComboFirma->AddString(val);
                                }
                                pComboFirma->SetCurSel(idxFind);
                            } else {
                                SetDlgItemText(IDC_EDT_FIRMA, val);
                            }
                        }
                        else if (key.Find(_T("dosya")) >= 0) SetDlgItemText(IDC_EDT_DOSYA, val);
                        else if (key.Find(_T("kal�p ad�")) >= 0) SetDlgItemText(IDC_EDT_KALIP_ADI, val);
                        else if (key.Find(_T("pres")) >= 0) SetDlgItemText(IDC_EDT_PRES, val);
                        else if (key.Find(_T("montaj")) >= 0) SetDlgItemText(IDC_EDT_MONTAJ, val);
                        else if (key.Find(_T("kal�ptaki")) >= 0) SetDlgItemText(IDC_EDT_KALIPTA_IS, val);
                        else if (key.Find(_T("toplam")) >= 0) SetDlgItemText(IDC_EDT_TOPLAM_IS, val);
                    }
                    line = sMetin.Tokenize(_T("\n"), curPos);
                }
            }

            // --- TA�LARI GRUPLAMA VE F�LTRELEME ---
            struct GroupedStone {
                COLORREF color;
                double width;
                double height;
                int count;
            };
            std::vector<GroupedStone> groups;
            
            for (long i = 1; i <= count; ++i) {
                IDispatch* pShape = GetDispatchPropWithIntArg(pSelection, L"Item", i);
                if (pShape) {
                    long type = GetLongProp(pShape, L"Type");
                    if (type != 2 && type != 3 && type != 5) {
                        pShape->Release();
                        continue;
                    }
                    
                    bool isBlackOutline = false;
                    bool isBlackFill = false;
                    bool isNoFill = false;
                    
                    IDispatch* pOutline = GetDispatchProp(pShape, L"Outline");
                    if (pOutline) {
                        long outType = GetLongProp(pOutline, L"Type");
                        if (outType != 0) {
                            IDispatch* pColor = GetDispatchProp(pOutline, L"Color");
                            if (pColor) {
                                CString hexVal = GetStringProp(pColor, L"HexValue");
                                hexVal.Replace(L"#", L"");
                                if (hexVal == L"000000" || hexVal.IsEmpty()) {
                                    isBlackOutline = true;
                                }
                                pColor->Release();
                            }
                        } else {
                            isBlackOutline = true;
                        }
                        pOutline->Release();
                    }
                    
                    IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
                    if (pFill) {
                        long fillType = GetLongProp(pFill, L"Type");
                        if (fillType == 0) {
                            isNoFill = true;
                        } else if (fillType == 1) {
                            IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                            if (pColor) {
                                CString hexVal = GetStringProp(pColor, L"HexValue");
                                hexVal.Replace(L"#", L"");
                                if (hexVal == L"000000" || hexVal.IsEmpty()) {
                                    isBlackFill = true;
                                }
                                pColor->Release();
                            }
                        }
                        pFill->Release();
                    }

                    CString shapeName = GetStringProp(pShape, L"Name");
                    double w = GetDoubleProp(pShape, L"SizeWidth");
                    double h = GetDoubleProp(pShape, L"SizeHeight");
                    
                    if (shapeName == _T("kalip ztrass.com") || shapeName == _T("Kesim") || (isNoFill && isBlackOutline) || (isBlackFill && isBlackOutline)) {
                        pShape->Release();
                        continue;
                    }
                    
                    if (type == 3 && (fabs(w - selW) < 2.0 || fabs(h - selH) < 2.0)) {
                        pShape->Release();
                        continue;
                    }
                    
                    COLORREF col = RGB(128, 128, 128);
                    pFill = GetDispatchProp(pShape, L"Fill");
                    long fillType = 0;
                    if (pFill) {
                        fillType = GetLongProp(pFill, L"Type");
                        if (fillType == 1) {
                            IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                            if (pColor) { col = GetColorRef(pColor); pColor->Release(); }
                        }
                        pFill->Release();
                    }
                    if (fillType != 1) {
                        pOutline = GetDispatchProp(pShape, L"Outline");
                        if (pOutline) {
                            IDispatch* pColor = GetDispatchProp(pOutline, L"Color");
                            if (pColor) { col = GetColorRef(pColor); pColor->Release(); }
                            pOutline->Release();
                        }
                    }

                    bool found = false;
                    for (auto& g : groups) {
                        if (g.color == col && fabs(g.width - w) < 0.05 && fabs(g.height - h) < 0.05) {
                            g.count++;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        GroupedStone gs;
                        gs.color = col;
                        gs.width = w;
                        gs.height = h;
                        gs.count = 1;
                        groups.push_back(gs);
                    }
                    
                    pShape->Release();
                }
            }
            
            m_listObjects.clear();
            std::vector<COLORREF> uniqueColors;
            auto GetSsSize = [](double size) -> CString {
                if (size <= 2.2) return _T("Ss6");
                if (size <= 2.6) return _T("Ss8");
                if (size <= 3.2) return _T("Ss10");
                if (size <= 3.6) return _T("Ss12");
                if (size <= 4.2) return _T("Ss16");
                if (size <= 5.2) return _T("Ss20");
                if (size <= 6.6) return _T("Ss30");
                if (size <= 7.6) return _T("Ss34");
                return _T("Ss40");
            };
            for (const auto& g : groups) {
                SelectedObjectInfo info;
                info.color = g.color;
                info.width = g.width;
                info.height = g.height;
                info.count = g.count;
                
                CString selType = _T("");
                if (m_parsedStoneDetails.find(g.color) != m_parsedStoneDetails.end()) {
                    selType = m_parsedStoneDetails[g.color].second;
                }
                
                CString wStr;
                if (selType == _T("Dbl") || selType == _T("Mc")) {
                    wStr = GetSsSize(g.width);
                } else {
                    if (fabs(g.width - (int)g.width) < 0.05) {
                        wStr.Format(_T("%dmm"), (int)g.width);
                    } else {
                        wStr.Format(_T("%.1fmm"), g.width);
                        wStr.Replace(_T("."), _T(","));
                    }
                }
                
                CString hStr;
                if (selType == _T("Dbl") || selType == _T("Mc")) {
                    hStr = GetSsSize(g.height);
                } else {
                    if (fabs(g.height - (int)g.height) < 0.05) {
                        hStr.Format(_T("%dmm"), (int)g.height);
                    } else {
                        hStr.Format(_T("%.1fmm"), g.height);
                        hStr.Replace(_T("."), _T(","));
                    }
                }
                
                CString objText;
                if (fabs(g.width - g.height) < 0.05) {
                    objText.Format(_T("%s-%d"), wStr, g.count);
                } else {
                    objText.Format(_T("%s/%s-%d"), wStr, hStr, g.count);
                }
                info.text = objText;
                m_listObjects.push_back(info);
                
                if (std::find(uniqueColors.begin(), uniqueColors.end(), g.color) == uniqueColors.end()) {
                    uniqueColors.push_back(g.color);
                }
            }
            
            CWnd* pGroup = GetDlgItem(IDC_GRP_LISTE);
            if (pGroup) {
                CString strTitle;
                strTitle.Format(_T("En: %.1f Cm Boy: %.1f Cm | %d Renk"), selW / 10.0, selH / 10.0, (int)uniqueColors.size());
                strTitle.Replace(_T("."), _T(","));
                pGroup->SetWindowText(strTitle);
            }
            
            pSelection->Release();
        }

        UpdateListLayout();
        
        ClearDynamicCombos();
        CWnd* pGroup2 = GetDlgItem(IDC_GRP_LISTE);
        if (pGroup2 && !m_listObjects.empty()) {
            CRect rcGroup;
            pGroup2->GetWindowRect(&rcGroup);
            ScreenToClient(&rcGroup);
            int startY = rcGroup.top + 14;
            int rowH = 24;
            
            DWORD dwStyle = WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL;
            if (m_bListOption) dwStyle |= WS_VISIBLE;
            
            std::vector<CString> RSN = {
                _T("Crystal"), _T("Amethyst"), _T("Black Diamond"), _T("Hyacinth"), _T("Aquamarine"), _T("Sapphire"), 
                _T("Peridot"), _T("Light Sapphire"), _T("Sm. Topaz"), _T("Pink"), _T("Light Siam"), _T("Rainbow"), 
                _T("Jet Black"), _T("Light Amethyst"), _T("Emerald"), _T("Jonquil"), _T("Siam"), _T("Topaz"), 
                _T("CobaLt"), _T("Hematite"), _T("Gold Hematite"), _T("Ab Crystal"), _T("Olive"), _T("Rose Pink"), 
                _T("Fuchsia"), _T("Blue Hematite"), _T("Light Brown"), _T("Rose Pink"), _T("Cosmojet"), _T("Light Topaz"), 
                _T("Montana"), _T("Violet"), _T("Light Peach"), _T("Light Colorado Topaz"), _T("Silver Hematite"), 
                _T("Citrine"), _T("Blue Zircon"), _T("Aquamarine Ab"), _T("Topaz Ab"), _T("Jet Ab"), _T("Sapphire Ab"), 
                _T("Siam Ab"), _T("Hyacinth Ab"), _T("Peridot Ab"), _T("Nude"), _T("Light Black Diamond")
            };
            std::vector<CString> RST = {_T("Dbl"), _T("Mc"), _T("Rzn"), _T("Oct"), _T("Drp"), _T("D\u00fcz"), _T("\u00d6zl")};

            for (size_t i = 0; i < m_listObjects.size(); ++i) {
                int rowY = startY + (int)i * rowH;
                int startX = rcGroup.left + 84;
                int endX = rcGroup.right - 8;
                int totalW = endX - startX;
                int tipW = 45;
                int gap = 4;
                int tasW = totalW - tipW - gap;
                
                CRect rcTas(startX, rowY, startX + tasW, rowY + 100);
                CRect rcTip(startX + tasW + gap, rowY, endX, rowY + 80);
                
                CComboBox* pTas = new CComboBox();
                pTas->Create(dwStyle, rcTas, this, 2000 + (int)i);
                pTas->SetFont(GetFont());
                
                for (const auto& name : RSN) {
                    pTas->AddString(name);
                }
                
                COLORREF rowCol = m_listObjects[i].color;
                CString selName = _T("");
                CString selType = _T("");
                if (m_parsedStoneDetails.find(rowCol) != m_parsedStoneDetails.end()) {
                    selName = m_parsedStoneDetails[rowCol].first;
                    selType = m_parsedStoneDetails[rowCol].second;
                }
                
                // Varsayılan taş renkleri ve tipleri otomatik eşleştirmesi
                if (selName.IsEmpty()) {
                    if (rowCol == RGB(255, 153, 204)) { // Rose Pink
                        selName = _T("Rose Pink");
                        selType = _T("Mc");
                    }
                    else if (rowCol == RGB(220, 20, 60)) { // Siam Ab
                        selName = _T("Siam Ab");
                        selType = _T("Rzn");
                    }
                    else if (rowCol == RGB(224, 255, 255)) { // Ab Crystal
                        selName = _T("Ab Crystal");
                        selType = _T("Oct");
                    }
                    else if (rowCol == RGB(165, 42, 42)) { // Light Brown
                        selName = _T("Light Brown");
                        selType = _T("Drp");
                    }
                    else if (rowCol == RGB(30, 30, 30) || rowCol == RGB(0, 0, 0)) { // Jet Black
                        selName = _T("Jet Black");
                        selType = _T("D\u00fcz");
                    }
                    else if (rowCol == RGB(255, 140, 0)) { // Rainbow
                        selName = _T("Rainbow");
                        selType = _T("Dbl");
                    }
                    else if (rowCol == RGB(245, 245, 245) || rowCol == RGB(255, 255, 255)) { // Crystal
                        selName = _T("Crystal");
                        selType = _T("\u00d6zl");
                    }
                    
                    if (!selName.IsEmpty()) {
                        m_parsedStoneDetails[rowCol] = std::make_pair(selName, selType);
                    }
                }
                
                int nameIdx = pTas->FindStringExact(-1, selName);
                if (nameIdx != CB_ERR) {
                    pTas->SetCurSel(nameIdx);
                } else {
                    pTas->SetCurSel(0);
                }
                m_combosTas.push_back(pTas);
                
                CComboBox* pTip = new CComboBox();
                pTip->Create(dwStyle, rcTip, this, 3000 + (int)i);
                pTip->SetFont(GetFont());
                
                for (const auto& type : RST) {
                    pTip->AddString(type);
                }
                
                int typeIdx = pTip->FindStringExact(-1, selType);
                if (typeIdx != CB_ERR) {
                    pTip->SetCurSel(typeIdx);
                } else {
                    pTip->SetCurSel(0);
                }
                m_combosTip.push_back(pTip);
            }
        }
        pDoc->Release();
    }
    SetRedraw(TRUE);
    Invalidate();
    UpdateWindow();
}

void CSearchDlg::OnBnClickedChkListOption()
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

void CSearchDlg::OnBnClickedSimge()
{
    SetCompactMode(!m_bCompactMode);
}

void CSearchDlg::SetCompactMode(BOOL bCompact)
{
    m_bCompactMode = bCompact;
    if (bCompact) {
        m_vecVisibleControls.clear();
        CWnd* pWnd = GetWindow(GW_CHILD);
        while (pWnd) {
            if (pWnd->IsWindowVisible()) {
                m_vecVisibleControls.push_back(pWnd->GetSafeHwnd());
                int id = pWnd->GetDlgCtrlID();
                if (id != IDC_BTN_simge && id != 3000 && id != 10001) {
                    pWnd->ShowWindow(SW_HIDE);
                }
            }
            pWnd = pWnd->GetWindow(GW_HWNDNEXT);
        }
        m_wndAdBox.ShowWindow(SW_SHOW);
        SetDialogHeight(32);
        GetDlgItem(IDC_BTN_simge)->SetWindowText(_T("+"));
    } else {
        m_wndAdBox.ShowWindow(SW_HIDE);
        for (HWND hWnd : m_vecVisibleControls) {
            if (::IsWindow(hWnd)) {
                ::ShowWindow(hWnd, SW_SHOW);
            }
        }
        m_vecVisibleControls.clear();
        if (m_bListOpen) {
            UpdateListLayout();
        } else {
            SetDialogHeight(198);
        }
        GetDlgItem(IDC_BTN_simge)->SetWindowText(_T("-"));
    }
    Invalidate();
}

void CSearchDlg::OnBnClickedChkKalip()
{
    UpdateData(TRUE);
    ShowKalipPanel(m_bKalip);
}

void CSearchDlg::OnBnClickedChkZtrass()
{
    UpdateData(TRUE);
}

void CSearchDlg::OnBnClickedBtnBilgiEkle()
{
    if (!m_pApp) return;
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Bilgi Ekle");
    
    CString sFirma, sDosya, sKalip, sPres, sMontaj, sKalipIs, sToplamIs;
    GetDlgItemText(IDC_EDT_FIRMA, sFirma);
    GetDlgItemText(IDC_EDT_DOSYA, sDosya);
    GetDlgItemText(IDC_EDT_KALIP_ADI, sKalip);
    GetDlgItemText(IDC_EDT_PRES, sPres);
    GetDlgItemText(IDC_EDT_MONTAJ, sMontaj);
    GetDlgItemText(IDC_EDT_KALIPTA_IS, sKalipIs);
    GetDlgItemText(IDC_EDT_TOPLAM_IS, sToplamIs);
    
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    SetLongProp(pDoc, L"Unit", 3); // mm
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pSel) {
        long count = GetLongProp(pSel, L"Count");
        if (count > 0) {
            double left = GetDoubleProp(pSel, L"LeftX");
            double top = GetDoubleProp(pSel, L"TopY");
            double right = GetDoubleProp(pSel, L"RightX");
            double bottom = GetDoubleProp(pSel, L"BottomY");
            
            // 1 mm disindan yeniden secelim
            IDispatch* pPage = GetDispatchProp(pDoc, L"ActivePage");
            if (pPage) {
                DISPID dispSelect; OLECHAR* szSelect = (OLECHAR*)L"SelectShapesFromRectangle";
                if (SUCCEEDED(pPage->GetIDsOfNames(IID_NULL, &szSelect, 1, LOCALE_USER_DEFAULT, &dispSelect))) {
                    VARIANT args[5]; for(int i=0; i<5; ++i) VariantInit(&args[i]);
                    args[4].vt = VT_R8; args[4].dblVal = left - 1.0;
                    args[3].vt = VT_R8; args[3].dblVal = top + 1.0;
                    args[2].vt = VT_R8; args[2].dblVal = right + 1.0;
                    args[1].vt = VT_R8; args[1].dblVal = bottom - 1.0;
                    args[0].vt = VT_BOOL; args[0].boolVal = VARIANT_TRUE;
                    
                    DISPPARAMS params = { args, NULL, 5, 0 };
                    VARIANT retSel; VariantInit(&retSel);
                    if (SUCCEEDED(pPage->Invoke(dispSelect, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retSel, NULL, NULL))) {
                        IDispatch* pNewSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
                        if (pNewSel) {
                            pSel->Release();
                            pSel = pNewSel;
                            left = GetDoubleProp(pSel, L"LeftX");
                            top = GetDoubleProp(pSel, L"TopY");
                            right = GetDoubleProp(pSel, L"RightX");
                            bottom = GetDoubleProp(pSel, L"BottomY");
                        }
                    }
                }
                pPage->Release();
            }
            
            IDispatch* pLayer = GetDispatchProp(pDoc, L"ActiveLayer");
            if (pLayer) {
                DISPID dispidCreate; OLECHAR* szCreate = (OLECHAR*)L"CreateArtisticText";
                if (SUCCEEDED(pLayer->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dispidCreate))) {
                    
                    std::vector<CString> linesToDraw;
                    std::vector<COLORREF> colorsToDraw;
                    
                    auto AddLineData = [&](const CString& txt, COLORREF col) {
                        linesToDraw.push_back(txt);
                        colorsToDraw.push_back(col);
                    };
                    
                    auto IsValidValue = [](const CString& val) -> bool {
                        CString temp = val;
                        temp.Trim();
                        return !temp.IsEmpty() && temp != _T("0");
                    };
                    
                    // Tarih ve saat bilgisini olusturalim
                    SYSTEMTIME st;
                    GetLocalTime(&st);
                    CString strTarih;
                    strTarih.Format(_T("%02d.%02d.%04d %02d:%02d"), st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);
                    
                    // Kalip ve Desen olculerini hesaplayalim
                    double kalipW = GetDoubleProp(pSel, L"SizeWidth");
                    double kalipH = GetDoubleProp(pSel, L"SizeHeight");
                    
                    double desenMinX = 999999.0, desenMaxX = -999999.0;
                    double desenMinY = 999999.0, desenMaxY = -999999.0;
                    bool hasStones = false;
                    double kalipTop = -999999.0;
                    double kalipLeft = 999999.0;
                    
                    IDispatch* pFlatRange = nullptr;
                    IDispatch* pShapes = GetDispatchProp(pSel, L"Shapes");
                    DISPID dispFind; OLECHAR* szFind = (OLECHAR*)L"FindShapes";
                    if (pShapes && SUCCEEDED(pShapes->GetIDsOfNames(IID_NULL, &szFind, 1, LOCALE_USER_DEFAULT, &dispFind))) {
                        DISPPARAMS params = { NULL, NULL, 0, 0 };
                        VARIANT retVal; VariantInit(&retVal);
                        if (SUCCEEDED(pShapes->Invoke(dispFind, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                            if (retVal.vt == VT_DISPATCH) pFlatRange = retVal.pdispVal;
                        }
                    }
                    if (pShapes) pShapes->Release();
                    
                    if (pFlatRange) {
                        long flatCount = GetLongProp(pFlatRange, L"Count");
                        for (long i = 1; i <= flatCount; ++i) {
                            IDispatch* pShape = GetDispatchPropWithIntArg(pFlatRange, L"Item", i);
                            if (pShape) {
                                long type = GetLongProp(pShape, L"Type");
                                CString shapeName = GetStringProp(pShape, L"Name");
                                
                                CString nameLower = shapeName;
                                nameLower.MakeLower();
                                bool isCutObject = (nameLower.Find(_T("kesim")) != -1 || 
                                                    nameLower.Find(_T("kalip")) != -1 || 
                                                    nameLower.Find(_T("ztrass")) != -1);
                                
                                if (type != 8 && type != 6 && !isCutObject) {
                                    double w = GetDoubleProp(pShape, L"SizeWidth");
                                    double h = GetDoubleProp(pShape, L"SizeHeight");
                                    
                                    // Dolgu rengini ve tipini alalim
                                    long fillType = 0;
                                    COLORREF fillCol = CLR_INVALID;
                                    IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
                                    if (pFill) {
                                        fillType = GetLongProp(pFill, L"Type");
                                        if (fillType == 1) {
                                            IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                                            if (pColor) {
                                                fillCol = GetColorRef(pColor);
                                                pColor->Release();
                                            }
                                        }
                                        pFill->Release();
                                    }
                                    
                                    // Sadece gercek desen taslarini (boyutu < 10mm, dolgulu, beyaz olmayan ve siyah olmayan) hesaba katalim
                                    bool isRealStone = (w < 10.0 && h < 10.0 && fillType == 1 && fillCol != RGB(255, 255, 255) && !IsColorBlack(fillCol));
                                    
                                    if (isRealStone) {
                                        double l = GetDoubleProp(pShape, L"LeftX");
                                        double r = GetDoubleProp(pShape, L"RightX");
                                        double b = GetDoubleProp(pShape, L"BottomY");
                                        double t = GetDoubleProp(pShape, L"TopY");
                                        
                                        if (l < desenMinX) desenMinX = l;
                                        if (r > desenMaxX) desenMaxX = r;
                                        if (b < desenMinY) desenMinY = b;
                                        if (t > desenMaxY) desenMaxY = t;
                                        hasStones = true;
                                    }
                                }
                                pShape->Release();
                            }
                        }
                        pFlatRange->Release();
                    }
                    
                    if (!hasStones) {
                        desenMinX = left;
                        desenMaxX = right;
                        desenMinY = bottom;
                        desenMaxY = top;
                    }
                    
                    double maxArea = 0.0;
                    IDispatch* pPage = GetDispatchProp(pDoc, L"ActivePage");
                    if (pPage) {
                        IDispatch* pPageShapes = GetDispatchProp(pPage, L"Shapes");
                        if (pPageShapes) {
                            DISPID dFindAll; OLECHAR* szFindAll = (OLECHAR*)L"FindShapes";
                            if (SUCCEEDED(pPageShapes->GetIDsOfNames(IID_NULL, &szFindAll, 1, LOCALE_USER_DEFAULT, &dFindAll))) {
                                DISPPARAMS paramsAll = { NULL, NULL, 0, 0 };
                                VARIANT retValAll; VariantInit(&retValAll);
                                if (SUCCEEDED(pPageShapes->Invoke(dFindAll, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsAll, &retValAll, NULL, NULL))) {
                                    if (retValAll.vt == VT_DISPATCH && retValAll.pdispVal) {
                                        IDispatch* pAllFlatRange = retValAll.pdispVal;
                                        long allCount = GetLongProp(pAllFlatRange, L"Count");
                                        for (long i = 1; i <= allCount; ++i) {
                                            IDispatch* pShape = GetDispatchPropWithIntArg(pAllFlatRange, L"Item", i);
                                            if (pShape) {
                                                long type = GetLongProp(pShape, L"Type");
                                                CString shapeName = GetStringProp(pShape, L"Name");
                                                
                                                CString nameLower = shapeName;
                                                nameLower.MakeLower();
                                                bool isKalipObj = (nameLower.Find(_T("kesim")) != -1 || 
                                                                    nameLower.Find(_T("kalip")) != -1 || 
                                                                    nameLower.Find(_T("ztrass")) != -1);
                                                
                                                // Adi bos olsa bile dolgusuz ve buyuk bir dikdortgense kalip kabul edelim
                                                if (!isKalipObj && type == 3) { // 3 = cdrRectangleShape
                                                    double w = GetDoubleProp(pShape, L"SizeWidth");
                                                    double h = GetDoubleProp(pShape, L"SizeHeight");
                                                    if (w > 20.0 && h > 20.0) {
                                                        long fillType = 0;
                                                        IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
                                                        if (pFill) {
                                                            fillType = GetLongProp(pFill, L"Type");
                                                            pFill->Release();
                                                        }
                                                        if (fillType == 0) { // 0 = cdrNoFill
                                                            isKalipObj = true;
                                                        }
                                                    }
                                                }
                                                
                                                if (isKalipObj) {
                                                    double sLeft = GetDoubleProp(pShape, L"LeftX");
                                                    double sRight = GetDoubleProp(pShape, L"RightX");
                                                    double sBottom = GetDoubleProp(pShape, L"BottomY");
                                                    double sTop = GetDoubleProp(pShape, L"TopY");
                                                    
                                                    // Desen sinirlarini kapsiyor mu (geometrik enclosing)?
                                                    if (sLeft <= desenMinX + 1.0 && sRight >= desenMaxX - 1.0 &&
                                                        sBottom <= desenMinY + 1.0 && sTop >= desenMaxY - 1.0) {
                                                        
                                                        double w = GetDoubleProp(pShape, L"SizeWidth");
                                                        double h = GetDoubleProp(pShape, L"SizeHeight");
                                                        double area = w * h;
                                                        if (area > maxArea) {
                                                            maxArea = area;
                                                            kalipTop = sTop;
                                                            kalipLeft = sLeft;
                                                        }
                                                    }
                                                }
                                                pShape->Release();
                                            }
                                        }
                                        pAllFlatRange->Release();
                                    }
                                }
                            }
                            pPageShapes->Release();
                        }
                        pPage->Release();
                    }
                    
                    CString strKalipOlcu = FormatCm(kalipW) + _T(" X ") + FormatCm(kalipH) + _T(" cm");
                    CString strDesenOlcu;
                    if (hasStones) {
                        strDesenOlcu = FormatCm(desenMaxX - desenMinX) + _T(" X ") + FormatCm(desenMaxY - desenMinY) + _T(" cm");
                    } else {
                        strDesenOlcu = FormatCm(kalipW) + _T(" X ") + FormatCm(kalipH) + _T(" cm"); // tas yoksa kalip boyutu
                    }
                    
                    AddLineData(_T("ztrass.com"), CLR_INVALID);
                    AddLineData(_T("Dosya: ") + sDosya, CLR_INVALID);
                    AddLineData(_T("Firma: ") + sFirma, CLR_INVALID);
                    AddLineData(_T("Kalıp Adı: ") + sKalip, CLR_INVALID);
                    AddLineData(_T("Kalıp Ölçü: ") + strKalipOlcu, CLR_INVALID);
                    AddLineData(_T("Desen Ölçü: ") + strDesenOlcu, CLR_INVALID);
                    AddLineData(_T("Kalıptaki İş: ") + sKalipIs, CLR_INVALID);
                    AddLineData(_T("Toplam İş: ") + sToplamIs, CLR_INVALID);
                    AddLineData(strTarih, CLR_INVALID);
                    
                    if (m_bListOption && !m_listObjects.empty()) {
                        std::vector<COLORREF> uniqueClrs;
                        for (size_t i = 0; i < m_listObjects.size(); i++) {
                            COLORREF c = m_listObjects[i].color;
                            if (std::find(uniqueClrs.begin(), uniqueClrs.end(), c) == uniqueClrs.end())
                                uniqueClrs.push_back(c);
                        }
                        CString header;
                        header.Format(_T("%d Renk"), (int)uniqueClrs.size());
                        AddLineData(header, CLR_INVALID);
                        
                        for (size_t i = 0; i < m_listObjects.size(); i++) {
                            CString tasAdi, tipAdi;
                            if (i < m_combosTas.size() && ::IsWindow(m_combosTas[i]->GetSafeHwnd())) {
                                m_combosTas[i]->GetWindowText(tasAdi);
                            }
                            if (i < m_combosTip.size() && ::IsWindow(m_combosTip[i]->GetSafeHwnd())) {
                                m_combosTip[i]->GetWindowText(tipAdi);
                            }
                            CString lineStr = m_listObjects[i].text + _T(" - ") + tasAdi + _T(" - ") + tipAdi;
                            AddLineData(lineStr, m_listObjects[i].color);
                        }
                    }
                    
                    CString fullText = _T("");
                    for (size_t idx = 0; idx < linesToDraw.size(); ++idx) {
                        if (!fullText.IsEmpty()) fullText += _T("\r\n");
                        fullText += linesToDraw[idx];
                    }
                    
                    double refTop = (kalipTop > -900000.0) ? kalipTop : top;
                    double refLeft = (kalipLeft < 900000.0) ? kalipLeft : left;
                    
                    double startY = refTop + 5.0 + (linesToDraw.size() * 4.0);
                    
                    if (!fullText.IsEmpty()) {
                        VARIANT args[3]; VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]);
                        args[2].vt = VT_R8; args[2].dblVal = refLeft;
                        args[1].vt = VT_R8; args[1].dblVal = startY;
                        args[0].vt = VT_BSTR; args[0].bstrVal = SysAllocString(fullText);
                        
                        DISPPARAMS params = { args, NULL, 3, 0 };
                        VARIANT retVal; VariantInit(&retVal);
                        if (SUCCEEDED(pLayer->Invoke(dispidCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                            if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                                IDispatch* pTextShape = retVal.pdispVal;
                                IDispatch* pText = GetDispatchProp(pTextShape, L"Text");
                                if (pText) {
                                    IDispatch* pStory = GetDispatchProp(pText, L"Story");
                                    if (pStory) {
                                        SetDoubleProp(pStory, L"Size", 8.0);
                                        
                                        IDispatch* pParagraphs = GetDispatchProp(pStory, L"Paragraphs");
                                        if (pParagraphs) {
                                            long paraCount = GetLongProp(pParagraphs, L"Count");
                                            for (long pIdx = 1; pIdx <= paraCount && pIdx <= (long)linesToDraw.size(); ++pIdx) {
                                                COLORREF col = colorsToDraw[pIdx - 1];
                                                if (col != CLR_INVALID) {
                                                    IDispatch* pPara = GetDispatchPropWithIntArg(pParagraphs, L"Item", pIdx);
                                                    if (pPara) {
                                                        IDispatch* pFill = GetDispatchProp(pPara, L"Fill");
                                                        if (pFill) {
                                                            IDispatch* pColor = nullptr;
                                                            VARIANT rCol; VariantInit(&rCol);
                                                            if (SUCCEEDED(InvokeMethodNoArgsRet(m_pApp, L"CreateColor", &rCol)) && rCol.vt == VT_DISPATCH) {
                                                                pColor = rCol.pdispVal;
                                                                SetColorRGB(pColor, GetRValue(col), GetGValue(col), GetBValue(col));
                                                                
                                                                DISPID dApply; OLECHAR* szApply = (OLECHAR*)L"ApplyUniformFill";
                                                                if (SUCCEEDED(pFill->GetIDsOfNames(IID_NULL, &szApply, 1, LOCALE_USER_DEFAULT, &dApply))) {
                                                                    VARIANT argC; VariantInit(&argC); argC.vt = VT_DISPATCH; argC.pdispVal = pColor;
                                                                    DISPPARAMS prmsC = { &argC, NULL, 1, 0 };
                                                                    pFill->Invoke(dApply, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsC, NULL, NULL, NULL);
                                                                }
                                                                pColor->Release();
                                                            }
                                                            pFill->Release();
                                                        }
                                                        pPara->Release();
                                                    }
                                                }
                                            }
                                            pParagraphs->Release();
                                        }
                                        pStory->Release();
                                    }
                                    pText->Release();
                                }
                                
                                // Bilgi metninin toplam yuksekligini alalim ve PositionX/PositionY ile kalibin sol ustunun 5 mm uzerine yerlestirelim
                                double textHeight = GetDoubleProp(pTextShape, L"SizeHeight");
                                
                                SetDoubleProp(pTextShape, L"PositionX", refLeft);
                                SetDoubleProp(pTextShape, L"PositionY", refTop + 5.0 + textHeight);
                                
                                pTextShape->Release();
                            }
                        }
                        SysFreeString(args[0].bstrVal);
                    }
                }
                pLayer->Release();
            }
        }
        pSel->Release();
    }
    pDoc->Release();
}
void CSearchDlg::OnBnClickedBtnKalipEkle()
{
    if (!m_pApp) return;
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Isaret Tasi");
    
    CString strTasMM;
    GetDlgItemText(IDC_EDT_KALIP_OLCU, strTasMM);
    strTasMM.Replace(L",", L".");
    double tasMM = _wtof(strTasMM);
    if (tasMM <= 0) tasMM = 2.0;
    
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    SetLongProp(pDoc, L"Unit", 3);
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pSel) {
        long count = GetLongProp(pSel, L"Count");
        if (count > 0) {
            double left = GetDoubleProp(pSel, L"LeftX");
            double right = GetDoubleProp(pSel, L"RightX");
            double bottom = GetDoubleProp(pSel, L"BottomY");
            double top = GetDoubleProp(pSel, L"TopY");
            
            double minW = 9999.0;
            IDispatch* pPage = GetDispatchProp(pDoc, L"ActivePage");
            if (pPage) {
                DISPID dispSelect; OLECHAR* szSelect = (OLECHAR*)L"SelectShapesFromRectangle";
                if (SUCCEEDED(pPage->GetIDsOfNames(IID_NULL, &szSelect, 1, LOCALE_USER_DEFAULT, &dispSelect))) {
                    VARIANT args[5]; for(int i=0; i<5; ++i) VariantInit(&args[i]);
                    args[4].vt = VT_R8; args[4].dblVal = left - 5.0;
                    args[3].vt = VT_R8; args[3].dblVal = top + 5.0;
                    args[2].vt = VT_R8; args[2].dblVal = right + 5.0;
                    args[1].vt = VT_R8; args[1].dblVal = bottom - 5.0;
                    args[0].vt = VT_BOOL; args[0].boolVal = VARIANT_TRUE;
                    
                    DISPPARAMS params = { args, NULL, 5, 0 };
                    VARIANT retSel; VariantInit(&retSel);
                    if (SUCCEEDED(pPage->Invoke(dispSelect, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retSel, NULL, NULL))) {
                        IDispatch* pNewSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
                        if (pNewSel) {
                            IDispatch* pShapes = GetDispatchProp(pNewSel, L"Shapes");
                            if (pShapes) {
                                DISPID dFind; OLECHAR* szFind = (OLECHAR*)L"FindShapes";
                                if (SUCCEEDED(pShapes->GetIDsOfNames(IID_NULL, &szFind, 1, LOCALE_USER_DEFAULT, &dFind))) {
                                    DISPPARAMS paramsFind = { NULL, NULL, 0, 0 };
                                    VARIANT retFind; VariantInit(&retFind);
                                    if (SUCCEEDED(pShapes->Invoke(dFind, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsFind, &retFind, NULL, NULL))) {
                                        if (retFind.vt == VT_DISPATCH && retFind.pdispVal) {
                                            IDispatch* pFlat = retFind.pdispVal;
                                            long flatCount = GetLongProp(pFlat, L"Count");
                                            for (long i = 1; i <= flatCount; i++) {
                                                IDispatch* pShape = GetDispatchPropWithIntArg(pFlat, L"Item", i);
                                                if (pShape) {
                                                    long type = GetLongProp(pShape, L"Type");
                                                    if (type == 2) {
                                                        double w = GetDoubleProp(pShape, L"SizeWidth");
                                                        if (w < minW) minW = w;
                                                    }
                                                    pShape->Release();
                                                }
                                            }
                                            pFlat->Release();
                                        }
                                    }
                                }
                                pShapes->Release();
                            }
                            pNewSel->Release();
                        }
                    }
                }
                pPage->Release();
            }
            
            if (minW > 100.0) minW = tasMM;
            
            double cy = bottom + tasMM;
            double radius = minW / 2.0;
            double cx1 = left + tasMM;
            double cx2 = right - tasMM;
            double centers[] = { cx1, cx2 };
            
            IDispatch* pActiveLayer = GetDispatchProp(pDoc, L"ActiveLayer");
            if (pActiveLayer) {
                DISPID dCreate; OLECHAR* szCreate = (OLECHAR*)L"CreateEllipse2";
                if (SUCCEEDED(pActiveLayer->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dCreate))) {
                    for (int i = 0; i < 2; i++) {
                        VARIANT args[3]; VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]);
                        args[2].vt = VT_R8; args[2].dblVal = centers[i];
                        args[1].vt = VT_R8; args[1].dblVal = cy;
                        args[0].vt = VT_R8; args[0].dblVal = radius;
                        
                        DISPPARAMS params = { args, NULL, 3, 0 };
                        VARIANT retVal; VariantInit(&retVal);
                        if (SUCCEEDED(pActiveLayer->Invoke(dCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                            if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                                IDispatch* pNewShape = retVal.pdispVal;
                                
                                IDispatch* pFill = GetDispatchProp(pNewShape, L"Fill");
                                if (pFill) {
                                    InvokeMethodNoArgs(pFill, L"ApplyNoFill");
                                    pFill->Release();
                                }
                                
                                IDispatch* pOutline = GetDispatchProp(pNewShape, L"Outline");
                                if (pOutline) {
                                    SetDoubleProp(pOutline, L"Width", 0.5);
                                    IDispatch* pColor = nullptr;
                                    VARIANT rCol; VariantInit(&rCol);
                                    if (SUCCEEDED(InvokeMethodNoArgsRet(m_pApp, L"CreateColor", &rCol)) && rCol.vt == VT_DISPATCH) {
                                        pColor = rCol.pdispVal;
                                        SetColorRGB(pColor, 0, 0, 0); // Siyah
                                        
                                        DISPID dCopy; OLECHAR* szCopy = (OLECHAR*)L"CopyAssign";
                                        IDispatch* pOutColor = GetDispatchProp(pOutline, L"Color");
                                        if (pOutColor && SUCCEEDED(pOutColor->GetIDsOfNames(IID_NULL, &szCopy, 1, LOCALE_USER_DEFAULT, &dCopy))) {
                                            VARIANT argC; VariantInit(&argC); argC.vt = VT_DISPATCH; argC.pdispVal = pColor;
                                            DISPPARAMS prmsC = { &argC, NULL, 1, 0 };
                                            pOutColor->Invoke(dCopy, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsC, NULL, NULL, NULL);
                                        }
                                        if (pOutColor) pOutColor->Release();
                                        pColor->Release();
                                    }
                                    pOutline->Release();
                                }
                                
                                SetStringProp(pNewShape, L"Name", L"kalip ztrass.com");
                                pNewShape->Release();
                            }
                        }
                    }
                }
                pActiveLayer->Release();
            }
        }
        pSel->Release();
    }
    pDoc->Release();
}

void CSearchDlg::OnBnClickedBtnIsaretTasi()
{
    // ProB.frm KalipKareM mantigi: desen etrafina padding ekle, yuksekligi snap et, alti sabit tut
    if (!m_pApp) return;
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Kalip Ekle");
    
    CString strPadding;
    GetDlgItemText(IDC_EDT_TASI_OLCU, strPadding);
    
    strPadding.Replace(L",", L".");
    double padding = _wtof(strPadding);
    if (padding <= 0) padding = 10.0;
    
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    SetLongProp(pDoc, L"Unit", 3);
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pSel) {
        long count = GetLongProp(pSel, L"Count");
        if (count > 0) {
            double left = GetDoubleProp(pSel, L"LeftX");
            double right = GetDoubleProp(pSel, L"RightX");
            double bottom = GetDoubleProp(pSel, L"BottomY");
            double top = GetDoubleProp(pSel, L"TopY");
            
            // 1 mm disindan yeniden secelim
            IDispatch* pPage = GetDispatchProp(pDoc, L"ActivePage");
            if (pPage) {
                DISPID dispSelect; OLECHAR* szSelect = (OLECHAR*)L"SelectShapesFromRectangle";
                if (SUCCEEDED(pPage->GetIDsOfNames(IID_NULL, &szSelect, 1, LOCALE_USER_DEFAULT, &dispSelect))) {
                    VARIANT args[5]; for(int i=0; i<5; ++i) VariantInit(&args[i]);
                    args[4].vt = VT_R8; args[4].dblVal = left - 1.0;
                    args[3].vt = VT_R8; args[3].dblVal = top + 1.0;
                    args[2].vt = VT_R8; args[2].dblVal = right + 1.0;
                    args[1].vt = VT_R8; args[1].dblVal = bottom - 1.0;
                    args[0].vt = VT_BOOL; args[0].boolVal = VARIANT_TRUE;
                    
                    DISPPARAMS params = { args, NULL, 5, 0 };
                    VARIANT retSel; VariantInit(&retSel);
                    if (SUCCEEDED(pPage->Invoke(dispSelect, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retSel, NULL, NULL))) {
                        IDispatch* pNewSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
                        if (pNewSel) {
                            pSel->Release();
                            pSel = pNewSel;
                            left = GetDoubleProp(pSel, L"LeftX");
                            right = GetDoubleProp(pSel, L"RightX");
                            bottom = GetDoubleProp(pSel, L"BottomY");
                            top = GetDoubleProp(pSel, L"TopY");
                        }
                    }
                }
                pPage->Release();
            }
            
            double kalipW = GetDoubleProp(pSel, L"SizeWidth");
            double kalipH = GetDoubleProp(pSel, L"SizeHeight");
            
            double desenMinX = 999999.0, desenMaxX = -999999.0;
            double desenMinY = 999999.0, desenMaxY = -999999.0;
            bool hasStones = false;
            
            IDispatch* pFlatRange = nullptr;
            IDispatch* pShapes = GetDispatchProp(pSel, L"Shapes");
            DISPID dispFind; OLECHAR* szFind = (OLECHAR*)L"FindShapes";
            if (pShapes && SUCCEEDED(pShapes->GetIDsOfNames(IID_NULL, &szFind, 1, LOCALE_USER_DEFAULT, &dispFind))) {
                DISPPARAMS params = { NULL, NULL, 0, 0 };
                VARIANT retVal; VariantInit(&retVal);
                if (SUCCEEDED(pShapes->Invoke(dispFind, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                    if (retVal.vt == VT_DISPATCH) pFlatRange = retVal.pdispVal;
                }
            }
            if (pShapes) pShapes->Release();
            
            if (pFlatRange) {
                long flatCount = GetLongProp(pFlatRange, L"Count");
                for (long i = 1; i <= flatCount; ++i) {
                    IDispatch* pShape = GetDispatchPropWithIntArg(pFlatRange, L"Item", i);
                    if (pShape) {
                        long type = GetLongProp(pShape, L"Type");
                        CString shapeName = GetStringProp(pShape, L"Name");
                        
                        CString nameLower = shapeName;
                        nameLower.MakeLower();
                        bool isCutObject = (nameLower.Find(_T("kesim")) != -1 || 
                                            nameLower.Find(_T("kalip")) != -1 || 
                                            nameLower.Find(_T("ztrass")) != -1);
                        
                        if (type != 8 && type != 6 && !isCutObject) {
                            double w = GetDoubleProp(pShape, L"SizeWidth");
                            double h = GetDoubleProp(pShape, L"SizeHeight");
                            
                            // Dolgu rengini ve tipini alalim
                            long fillType = 0;
                            COLORREF fillCol = CLR_INVALID;
                            IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
                            if (pFill) {
                                fillType = GetLongProp(pFill, L"Type");
                                if (fillType == 1) {
                                    IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                                    if (pColor) {
                                        fillCol = GetColorRef(pColor);
                                        pColor->Release();
                                    }
                                }
                                pFill->Release();
                            }
                            
                            // Sadece gercek desen taslarini (boyutu < 10mm, dolgulu, beyaz olmayan ve siyah olmayan) hesaba katalim
                            bool isRealStone = (w < 10.0 && h < 10.0 && fillType == 1 && fillCol != RGB(255, 255, 255) && !IsColorBlack(fillCol));
                            
                            if (isRealStone) {
                                double l = GetDoubleProp(pShape, L"LeftX");
                                double r = GetDoubleProp(pShape, L"RightX");
                                double b = GetDoubleProp(pShape, L"BottomY");
                                double t = GetDoubleProp(pShape, L"TopY");
                                
                                if (l < desenMinX) desenMinX = l;
                                if (r > desenMaxX) desenMaxX = r;
                                if (b < desenMinY) desenMinY = b;
                                if (t > desenMaxY) desenMaxY = t;
                                hasStones = true;
                            }
                        }
                        pShape->Release();
                    }
                }
                pFlatRange->Release();
            }
            
            if (hasStones) {
                left = desenMinX;
                right = desenMaxX;
                bottom = desenMinY;
                top = desenMaxY;
            }
            
            // ProB.frm: CreateRectangle(sr.LeftX - padding, sr.BottomY - padding, sr.RightX + padding, sr.TopY + padding)
            double x1 = left - padding;
            double y2 = bottom - padding;
            double x2 = right + padding;
            double y1 = top + padding;
            
            IDispatch* pActiveLayer = GetDispatchProp(pDoc, L"ActiveLayer");
            if (pActiveLayer) {
                DISPID dCreate; OLECHAR* szCreate = (OLECHAR*)L"CreateRectangle";
                if (SUCCEEDED(pActiveLayer->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dCreate))) {
                    
                    VARIANT args[4]; for(int i=0; i<4; i++) VariantInit(&args[i]);
                    args[3].vt = VT_R8; args[3].dblVal = x1;
                    args[2].vt = VT_R8; args[2].dblVal = y1;
                    args[1].vt = VT_R8; args[1].dblVal = x2;
                    args[0].vt = VT_R8; args[0].dblVal = y2;
                    
                    DISPPARAMS params = { args, NULL, 4, 0 };
                    VARIANT retVal; VariantInit(&retVal);
                    if (SUCCEEDED(pActiveLayer->Invoke(dCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                        if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                            IDispatch* pRect = retVal.pdispVal;
                            
                            IDispatch* pOutline = GetDispatchProp(pRect, L"Outline");
                            if (pOutline) {
                                SetDoubleProp(pOutline, L"Width", 0.5);
                                IDispatch* pColor = nullptr;
                                VARIANT rCol; VariantInit(&rCol);
                                if (SUCCEEDED(InvokeMethodNoArgsRet(m_pApp, L"CreateColor", &rCol)) && rCol.vt == VT_DISPATCH) {
                                    pColor = rCol.pdispVal;
                                    SetColorRGB(pColor, 0, 0, 0); // Siyah outline
                                    
                                    DISPID dCopy; OLECHAR* szCopy = (OLECHAR*)L"CopyAssign";
                                    IDispatch* pOutColor = GetDispatchProp(pOutline, L"Color");
                                    if (pOutColor && SUCCEEDED(pOutColor->GetIDsOfNames(IID_NULL, &szCopy, 1, LOCALE_USER_DEFAULT, &dCopy))) {
                                        VARIANT argC; VariantInit(&argC); argC.vt = VT_DISPATCH; argC.pdispVal = pColor;
                                        DISPPARAMS prmsC = { &argC, NULL, 1, 0 };
                                        pOutColor->Invoke(dCopy, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsC, NULL, NULL, NULL);
                                    }
                                    if (pOutColor) pOutColor->Release();
                                    pColor->Release();
                                }
                                pOutline->Release();
                            }
                            
                            IDispatch* pFill = GetDispatchProp(pRect, L"Fill");
                            if (pFill) {
                                InvokeMethodNoArgs(pFill, L"ApplyNoFill");
                                pFill->Release();
                            }
                            InvokeMethodNoArgs(pRect, L"OrderToBack");
                            SetStringProp(pRect, L"Name", L"kalip ztrass.com");
                            pRect->Release();
                        }
                    }
                }
                pActiveLayer->Release();
            }
        }
        pSel->Release();
    }
    pDoc->Release();
}

void CSearchDlg::OnBnClickedBtnImalat()
{
    // ProB.frm KalipKareM mantigi: desen etrafina padding ekle, yuksekligi snap et, alti sabit tut
    if (!m_pApp) return;
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Kalip Ekle");
    
    CString strPadding;
    GetDlgItemText(IDC_EDT_TASI_OLCU, strPadding);
    
    strPadding.Replace(L",", L".");
    double padding = _wtof(strPadding);
    if (padding <= 0) padding = 10.0;
    
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    SetLongProp(pDoc, L"Unit", 3);
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pSel) {
        long count = GetLongProp(pSel, L"Count");
        if (count > 0) {
            double left = GetDoubleProp(pSel, L"LeftX");
            double right = GetDoubleProp(pSel, L"RightX");
            double bottom = GetDoubleProp(pSel, L"BottomY");
            double top = GetDoubleProp(pSel, L"TopY");
            
            // 1 mm disindan yeniden secelim
            IDispatch* pPage = GetDispatchProp(pDoc, L"ActivePage");
            if (pPage) {
                DISPID dispSelect; OLECHAR* szSelect = (OLECHAR*)L"SelectShapesFromRectangle";
                if (SUCCEEDED(pPage->GetIDsOfNames(IID_NULL, &szSelect, 1, LOCALE_USER_DEFAULT, &dispSelect))) {
                    VARIANT args[5]; for(int i=0; i<5; ++i) VariantInit(&args[i]);
                    args[4].vt = VT_R8; args[4].dblVal = left - 1.0;
                    args[3].vt = VT_R8; args[3].dblVal = top + 1.0;
                    args[2].vt = VT_R8; args[2].dblVal = right + 1.0;
                    args[1].vt = VT_R8; args[1].dblVal = bottom - 1.0;
                    args[0].vt = VT_BOOL; args[0].boolVal = VARIANT_TRUE;
                    
                    DISPPARAMS params = { args, NULL, 5, 0 };
                    VARIANT retSel; VariantInit(&retSel);
                    if (SUCCEEDED(pPage->Invoke(dispSelect, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retSel, NULL, NULL))) {
                        IDispatch* pNewSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
                        if (pNewSel) {
                            pSel->Release();
                            pSel = pNewSel;
                            left = GetDoubleProp(pSel, L"LeftX");
                            right = GetDoubleProp(pSel, L"RightX");
                            bottom = GetDoubleProp(pSel, L"BottomY");
                            top = GetDoubleProp(pSel, L"TopY");
                        }
                    }
                }
                pPage->Release();
            }
            
            double kalipW = GetDoubleProp(pSel, L"SizeWidth");
            double kalipH = GetDoubleProp(pSel, L"SizeHeight");
            
            double desenMinX = 999999.0, desenMaxX = -999999.0;
            double desenMinY = 999999.0, desenMaxY = -999999.0;
            bool hasStones = false;
            
            IDispatch* pFlatRange = nullptr;
            IDispatch* pShapes = GetDispatchProp(pSel, L"Shapes");
            DISPID dispFind; OLECHAR* szFind = (OLECHAR*)L"FindShapes";
            if (pShapes && SUCCEEDED(pShapes->GetIDsOfNames(IID_NULL, &szFind, 1, LOCALE_USER_DEFAULT, &dispFind))) {
                DISPPARAMS params = { NULL, NULL, 0, 0 };
                VARIANT retVal; VariantInit(&retVal);
                if (SUCCEEDED(pShapes->Invoke(dispFind, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                    if (retVal.vt == VT_DISPATCH) pFlatRange = retVal.pdispVal;
                }
            }
            if (pShapes) pShapes->Release();
            
            if (pFlatRange) {
                long flatCount = GetLongProp(pFlatRange, L"Count");
                for (long i = 1; i <= flatCount; ++i) {
                    IDispatch* pShape = GetDispatchPropWithIntArg(pFlatRange, L"Item", i);
                    if (pShape) {
                        long type = GetLongProp(pShape, L"Type");
                        CString shapeName = GetStringProp(pShape, L"Name");
                        
                        CString nameLower = shapeName;
                        nameLower.MakeLower();
                        bool isCutObject = (nameLower.Find(_T("kesim")) != -1 || 
                                            nameLower.Find(_T("kalip")) != -1 || 
                                            nameLower.Find(_T("ztrass")) != -1);
                        
                        if (type != 8 && type != 6 && !isCutObject) {
                            double w = GetDoubleProp(pShape, L"SizeWidth");
                            double h = GetDoubleProp(pShape, L"SizeHeight");
                            
                            // Dolgu rengini ve tipini alalim
                            long fillType = 0;
                            COLORREF fillCol = CLR_INVALID;
                            IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
                            if (pFill) {
                                fillType = GetLongProp(pFill, L"Type");
                                if (fillType == 1) {
                                    IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                                    if (pColor) {
                                        fillCol = GetColorRef(pColor);
                                        pColor->Release();
                                    }
                                }
                                pFill->Release();
                            }
                            
                            // Sadece gercek desen taslarini (boyutu < 10mm, dolgulu, beyaz olmayan ve siyah olmayan) hesaba katalim
                            bool isRealStone = (w < 10.0 && h < 10.0 && fillType == 1 && fillCol != RGB(255, 255, 255) && !IsColorBlack(fillCol));
                            
                            if (isRealStone) {
                                double l = GetDoubleProp(pShape, L"LeftX");
                                double r = GetDoubleProp(pShape, L"RightX");
                                double b = GetDoubleProp(pShape, L"BottomY");
                                double t = GetDoubleProp(pShape, L"TopY");
                                
                                if (l < desenMinX) desenMinX = l;
                                if (r > desenMaxX) desenMaxX = r;
                                if (b < desenMinY) desenMinY = b;
                                if (t > desenMaxY) desenMaxY = t;
                                hasStones = true;
                            }
                        }
                        pShape->Release();
                    }
                }
                pFlatRange->Release();
            }
            
            if (hasStones) {
                left = desenMinX;
                right = desenMaxX;
                bottom = desenMinY;
                top = desenMaxY;
            }
            
            // ProB.frm: CreateRectangle(sr.LeftX - padding, sr.BottomY - padding, sr.RightX + padding, sr.TopY + padding)
            double x1 = left - padding;
            double y2 = bottom - padding;   // altY = kare.BottomY
            double x2 = right + padding;
            double y1 = top + padding;
            
            double altY = y2;  // alt siniri sakla
            
            // Yukseklik kontrolu: ProB.frm Select Case .SizeHeight
            double sizeH = y1 - y2;
            if (sizeH < 240.0) {
                sizeH = 240.0;
            } else if (sizeH < 320.0) {
                sizeH = 320.0;
            } else if (sizeH < 400.0) {
                sizeH = 400.0;
            }
            // Alt kenar sabit, ust kenar yukseklige gore: VBA .BottomY = altY
            y1 = altY + sizeH;
            
            IDispatch* pActiveLayer = GetDispatchProp(pDoc, L"ActiveLayer");
            if (pActiveLayer) {
                DISPID dCreate; OLECHAR* szCreate = (OLECHAR*)L"CreateRectangle";
                if (SUCCEEDED(pActiveLayer->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dCreate))) {
                    
                    VARIANT args[4]; for(int i=0; i<4; i++) VariantInit(&args[i]);
                    args[3].vt = VT_R8; args[3].dblVal = x1;
                    args[2].vt = VT_R8; args[2].dblVal = y1;
                    args[1].vt = VT_R8; args[1].dblVal = x2;
                    args[0].vt = VT_R8; args[0].dblVal = y2;
                    
                    DISPPARAMS params = { args, NULL, 4, 0 };
                    VARIANT retVal; VariantInit(&retVal);
                    if (SUCCEEDED(pActiveLayer->Invoke(dCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                        if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                            IDispatch* pRect = retVal.pdispVal;
                            
                            IDispatch* pOutline = GetDispatchProp(pRect, L"Outline");
                            if (pOutline) {
                                SetDoubleProp(pOutline, L"Width", 0.5);
                                IDispatch* pColor = nullptr;
                                VARIANT rCol; VariantInit(&rCol);
                                if (SUCCEEDED(InvokeMethodNoArgsRet(m_pApp, L"CreateColor", &rCol)) && rCol.vt == VT_DISPATCH) {
                                    pColor = rCol.pdispVal;
                                    SetColorRGB(pColor, 0, 0, 0); // Siyah outline
                                    
                                    DISPID dCopy; OLECHAR* szCopy = (OLECHAR*)L"CopyAssign";
                                    IDispatch* pOutColor = GetDispatchProp(pOutline, L"Color");
                                    if (pOutColor && SUCCEEDED(pOutColor->GetIDsOfNames(IID_NULL, &szCopy, 1, LOCALE_USER_DEFAULT, &dCopy))) {
                                        VARIANT argC; VariantInit(&argC); argC.vt = VT_DISPATCH; argC.pdispVal = pColor;
                                        DISPPARAMS prmsC = { &argC, NULL, 1, 0 };
                                        pOutColor->Invoke(dCopy, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsC, NULL, NULL, NULL);
                                    }
                                    if (pOutColor) pOutColor->Release();
                                    pColor->Release();
                                }
                                pOutline->Release();
                            }
                            
                            IDispatch* pFill = GetDispatchProp(pRect, L"Fill");
                            if (pFill) {
                                InvokeMethodNoArgs(pFill, L"ApplyNoFill");
                                pFill->Release();
                            }
                            InvokeMethodNoArgs(pRect, L"OrderToBack");
                            SetStringProp(pRect, L"Name", L"kalip ztrass.com");
                            pRect->Release();
                        }
                    }
                }
                pActiveLayer->Release();
            }
        }
        pSel->Release();
    }
    pDoc->Release();
}

void CSearchDlg::OnBnClickedBtnKalipAyir()
{
    if (!m_pApp) return;
    
    IDispatch* pTempSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pTempSel) {
        long c = GetLongProp(pTempSel, L"Count");
        pTempSel->Release();
        if (c == 1) OnBnClickedBtnList();
    }
    
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Kalip Ayir");
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    
    SetLongProp(pDoc, L"Unit", 3); // mm
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (!pSel) { pDoc->Release(); return; }
    
    IDispatch* pPage = GetDispatchProp(pDoc, L"ActivePage");
    
    IDispatch* pFlatRange = nullptr;
    IDispatch* pSelShapes = GetDispatchProp(pSel, L"Shapes");
    if (pSelShapes) {
        pFlatRange = FindShapes(pSelShapes, _T(""), 0, true);
        pSelShapes->Release();
    }
    if (!pFlatRange) { 
        if (pPage) pPage->Release();
        pSel->Release(); 
        pDoc->Release(); 
        return; 
    }
    long flatCount = GetLongProp(pFlatRange, L"Count");
    
    std::set<long> selectedIDs;
    for (long i = 1; i <= flatCount; i++) {
        IDispatch* pShape = GetDispatchPropWithIntArg(pFlatRange, L"Item", i);
        if (pShape) {
            selectedIDs.insert(GetLongProp(pShape, L"StaticID"));
            pShape->Release();
        }
    }
    
    double desenMinX = 999999.0, desenMaxX = -999999.0;
    double desenMinY = 999999.0, desenMaxY = -999999.0;
    bool hasStones = false;
    
    for (long i = 1; i <= flatCount; i++) {
        IDispatch* pShape = GetDispatchPropWithIntArg(pFlatRange, L"Item", i);
        if (!pShape) continue;
        long type = GetLongProp(pShape, L"Type");
        if (type != 8 && type != 6) {
            double w = GetDoubleProp(pShape, L"SizeWidth");
            double h = GetDoubleProp(pShape, L"SizeHeight");
            long fillType = 0;
            COLORREF fillCol = CLR_INVALID;
            IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
            if (pFill) {
                fillType = GetLongProp(pFill, L"Type");
                if (fillType == 1) {
                    IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                    if (pColor) {
                        fillCol = GetColorRef(pColor);
                        pColor->Release();
                    }
                }
                pFill->Release();
            }
            bool isRealStone = (w < 10.0 && h < 10.0 && fillType == 1 && fillCol != RGB(255, 255, 255) && !IsColorBlack(fillCol));
            if (isRealStone) {
                double l = GetDoubleProp(pShape, L"LeftX");
                double r = GetDoubleProp(pShape, L"RightX");
                double b = GetDoubleProp(pShape, L"BottomY");
                double t = GetDoubleProp(pShape, L"TopY");
                if (l < desenMinX) desenMinX = l;
                if (r > desenMaxX) desenMaxX = r;
                if (b < desenMinY) desenMinY = b;
                if (t > desenMaxY) desenMaxY = t;
                hasStones = true;
            }
        }
        pShape->Release();
    }
    
    if (!hasStones) {
        desenMinX = GetDoubleProp(pSel, L"LeftX");
        desenMaxX = GetDoubleProp(pSel, L"RightX");
        desenMinY = GetDoubleProp(pSel, L"BottomY");
        desenMaxY = GetDoubleProp(pSel, L"TopY");
    }
    
    // Find pKalipKare (largest shape on page enclosing selection, prioritizing "kalip" in name, max size 320x520 mm)
    IDispatch* pKalipKare = nullptr;
    int bestScore = -1;
    double bestArea = 0.0;
    double kalipLeft = 0.0, kalipRight = 0.0, kalipBottom = 0.0, kalipTop = 0.0;
    
    if (pPage) {
        IDispatch* pPageShapes = GetDispatchProp(pPage, L"Shapes");
        if (pPageShapes) {
            IDispatch* pAllFlatRange = FindShapes(pPageShapes, _T(""), 0, true);
            if (pAllFlatRange) {
                long allCount = GetLongProp(pAllFlatRange, L"Count");
                for (long i = 1; i <= allCount; ++i) {
                    IDispatch* pShape = GetDispatchPropWithIntArg(pAllFlatRange, L"Item", i);
                    if (!pShape) continue;
                    long type = GetLongProp(pShape, L"Type");
                    if (type != 8 && type != 6) {
                        double sLeft = GetDoubleProp(pShape, L"LeftX");
                        double sRight = GetDoubleProp(pShape, L"RightX");
                        double sBottom = GetDoubleProp(pShape, L"BottomY");
                        double sTop = GetDoubleProp(pShape, L"TopY");
                        double w = GetDoubleProp(pShape, L"SizeWidth");
                        double h = GetDoubleProp(pShape, L"SizeHeight");
                        
                        if (w >= 15.0 && h >= 15.0 && w <= 320.0 && h <= 520.0) {
                            if (sLeft <= desenMinX + 2.0 && sRight >= desenMaxX - 2.0 &&
                                sBottom <= desenMinY + 2.0 && sTop >= desenMaxY - 2.0) {
                                double area = w * h;
                                CString name = GetStringProp(pShape, L"Name");
                                CString nameLower = name;
                                nameLower.MakeLower();
                                bool hasKalipName = (nameLower.Find(L"kalip") != -1 || nameLower.Find(L"ztrass") != -1);
                                
                                int currentScore = hasKalipName ? 2 : 1;
                                
                                bool replace = false;
                                if (currentScore > bestScore) {
                                    replace = true;
                                } else if (currentScore == bestScore) {
                                    if (area > bestArea) {
                                        replace = true;
                                    }
                                }
                                
                                if (replace) {
                                    bestScore = currentScore;
                                    bestArea = area;
                                    if (pKalipKare) pKalipKare->Release();
                                    pKalipKare = pShape;
                                    pKalipKare->AddRef();
                                    kalipLeft = sLeft;
                                    kalipRight = sRight;
                                    kalipBottom = sBottom;
                                    kalipTop = sTop;
                                }
                            }
                        }
                    }
                    pShape->Release();
                }
                pAllFlatRange->Release();
            }
            pPageShapes->Release();
        }
    }
    
    // Create backup mold rectangle if none found
    if (!pKalipKare) {
        double padding = 10.0;
        double x1 = desenMinX - padding;
        double y2 = desenMinY - padding;
        double x2 = desenMaxX + padding;
        double y1 = desenMaxY + padding;
        
        double sizeH = y1 - y2;
        if (sizeH < 240.0) {
            sizeH = 240.0;
        } else if (sizeH < 320.0) {
            sizeH = 320.0;
        } else if (sizeH < 400.0) {
            sizeH = 400.0;
        }
        y1 = y2 + sizeH;
        
        IDispatch* pActiveLayer = GetDispatchProp(pDoc, L"ActiveLayer");
        if (pActiveLayer) {
            DISPID dCreate; OLECHAR* szCreate = (OLECHAR*)L"CreateRectangle";
            if (SUCCEEDED(pActiveLayer->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dCreate))) {
                VARIANT args[4]; for(int i=0; i<4; i++) VariantInit(&args[i]);
                args[3].vt = VT_R8; args[3].dblVal = x1;
                args[2].vt = VT_R8; args[2].dblVal = y1;
                args[1].vt = VT_R8; args[1].dblVal = x2;
                args[0].vt = VT_R8; args[0].dblVal = y2;
                
                DISPPARAMS params = { args, NULL, 4, 0 };
                VARIANT retVal; VariantInit(&retVal);
                if (SUCCEEDED(pActiveLayer->Invoke(dCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                    if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                        pKalipKare = retVal.pdispVal;
                        
                        IDispatch* pOutline = GetDispatchProp(pKalipKare, L"Outline");
                        if (pOutline) {
                            SetDoubleProp(pOutline, L"Width", 0.5);
                            IDispatch* pColor = nullptr;
                            VARIANT rCol; VariantInit(&rCol);
                            if (SUCCEEDED(InvokeMethodNoArgsRet(m_pApp, L"CreateColor", &rCol)) && rCol.vt == VT_DISPATCH) {
                                pColor = rCol.pdispVal;
                                SetColorRGB(pColor, 0, 0, 0);
                                
                                DISPID dCopy; OLECHAR* szCopy = (OLECHAR*)L"CopyAssign";
                                IDispatch* pOutColor = GetDispatchProp(pOutline, L"Color");
                                if (pOutColor && SUCCEEDED(pOutColor->GetIDsOfNames(IID_NULL, &szCopy, 1, LOCALE_USER_DEFAULT, &dCopy))) {
                                    VARIANT argC; VariantInit(&argC); argC.vt = VT_DISPATCH; argC.pdispVal = pColor;
                                    DISPPARAMS prmsC = { &argC, NULL, 1, 0 };
                                    pOutColor->Invoke(dCopy, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsC, NULL, NULL, NULL);
                                }
                                if (pOutColor) pOutColor->Release();
                                pColor->Release();
                            }
                            pOutline->Release();
                        }
                        
                        IDispatch* pFill = GetDispatchProp(pKalipKare, L"Fill");
                        if (pFill) {
                            InvokeMethodNoArgs(pFill, L"ApplyNoFill");
                            pFill->Release();
                        }
                        InvokeMethodNoArgs(pKalipKare, L"OrderToBack");
                        SetStringProp(pKalipKare, L"Name", L"kalip ztrass.com");
                        
                        kalipLeft = x1;
                        kalipRight = x2;
                        kalipBottom = y2;
                        kalipTop = y1;
                    }
                }
            }
            pActiveLayer->Release();
        }
    }
    
    double kalipW = 0.0;
    if (pKalipKare) {
        kalipW = GetDoubleProp(pKalipKare, L"SizeWidth");
    } else {
        kalipLeft = GetDoubleProp(pSel, L"LeftX");
        kalipRight = GetDoubleProp(pSel, L"RightX");
        kalipBottom = GetDoubleProp(pSel, L"BottomY");
        kalipTop = GetDoubleProp(pSel, L"TopY");
        kalipW = GetDoubleProp(pSel, L"SizeWidth");
    }
    
    // Find info text shape
    IDispatch* pInfoTextShape = nullptr;
    if (pPage) {
        IDispatch* pPageShapes = GetDispatchProp(pPage, L"Shapes");
        if (pPageShapes) {
            IDispatch* pTextRange = FindShapes(pPageShapes, _T("@type = 'text:artistic'"), 0, true);
            if (pTextRange) {
                long txtCount = GetLongProp(pTextRange, L"Count");
                double minTextDistance = 999999.0;
                for (long t = 1; t <= txtCount; ++t) {
                    IDispatch* pTShape = GetDispatchPropWithIntArg(pTextRange, L"Item", t);
                    if (pTShape) {
                        IDispatch* pTextObj = GetDispatchProp(pTShape, L"Text");
                        if (pTextObj) {
                            IDispatch* pStory = GetDispatchProp(pTextObj, L"Story");
                            if (pStory) {
                                CString textVal = GetStringProp(pStory, L"Text");
                                CString textValLower = textVal;
                                textValLower.MakeLower();
                                if (textValLower.Find(L"ztrass.com") != -1) {
                                    double tLeft = GetDoubleProp(pTShape, L"LeftX");
                                    double tTop = GetDoubleProp(pTShape, L"TopY");
                                    double dx = tLeft - kalipLeft;
                                    double dy = tTop - kalipTop;
                                    double dist = sqrt(dx*dx + dy*dy);
                                    if (dist < minTextDistance) {
                                        minTextDistance = dist;
                                        if (pInfoTextShape) pInfoTextShape->Release();
                                        pInfoTextShape = pTShape;
                                        pInfoTextShape->AddRef();
                                    }
                                }
                                pStory->Release();
                            }
                            pTextObj->Release();
                        }
                        pTShape->Release();
                    }
                }
                pTextRange->Release();
            }
            pPageShapes->Release();
        }
    }
    
    std::vector<IDispatch*> commonShapes;
    std::vector<IDispatch*> textShapes;
    std::vector<IDispatch*> markerShapesOnPage;
    std::map<COLORREF, std::vector<IDispatch*>> colorGroups;
    
    auto AddToCommonUnique = [&](IDispatch* pNewShape) {
        if (!pNewShape) return;
        long newID = GetLongProp(pNewShape, L"StaticID");
        for (auto s : commonShapes) {
            if (GetLongProp(s, L"StaticID") == newID) return;
        }
        pNewShape->AddRef();
        commonShapes.push_back(pNewShape);
    };
    
    if (pPage) {
        IDispatch* pPageShapes = GetDispatchProp(pPage, L"Shapes");
        if (pPageShapes) {
            IDispatch* pAllFlatRange = FindShapes(pPageShapes, _T(""), 0, true);
            if (pAllFlatRange) {
                long allCount = GetLongProp(pAllFlatRange, L"Count");
                long kkStaticID = pKalipKare ? GetLongProp(pKalipKare, L"StaticID") : 0;
                long infoStaticID = pInfoTextShape ? GetLongProp(pInfoTextShape, L"StaticID") : 0;
                
                for (long i = 1; i <= allCount; ++i) {
                    IDispatch* pShape = GetDispatchPropWithIntArg(pAllFlatRange, L"Item", i);
                    if (!pShape) continue;
                    long shapeID = GetLongProp(pShape, L"StaticID");
                    if (shapeID != kkStaticID && shapeID != infoStaticID) {
                        double sLeft = GetDoubleProp(pShape, L"LeftX");
                        double sRight = GetDoubleProp(pShape, L"RightX");
                        double sBottom = GetDoubleProp(pShape, L"BottomY");
                        double sTop = GetDoubleProp(pShape, L"TopY");
                        
                        bool isInside = (sLeft >= kalipLeft - 10.0 && sRight <= kalipRight + 10.0 &&
                                         sBottom >= kalipBottom - 10.0 && sTop <= kalipTop + 10.0);
                        if (isInside) {
                            long type = GetLongProp(pShape, L"Type");
                            if (type == 8 || type == 6) {
                                if (type == 8) {
                                    pShape->AddRef();
                                    textShapes.push_back(pShape);
                                }
                            } else {
                                double w = GetDoubleProp(pShape, L"SizeWidth");
                                double h = GetDoubleProp(pShape, L"SizeHeight");
                                long fillType = 0;
                                COLORREF fillCol = CLR_INVALID;
                                IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
                                if (pFill) {
                                    fillType = GetLongProp(pFill, L"Type");
                                    if (fillType == 1) {
                                        IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                                        if (pColor) {
                                            fillCol = GetColorRef(pColor);
                                            pColor->Release();
                                        }
                                    }
                                    pFill->Release();
                                }
                                bool isRealStone = (w < 10.0 && h < 10.0 && fillType == 1 && fillCol != RGB(255, 255, 255) && !IsColorBlack(fillCol));
                                if (isRealStone) {
                                    pShape->AddRef();
                                    colorGroups[fillCol].push_back(pShape);
                                } else {
                                    pShape->AddRef();
                                    markerShapesOnPage.push_back(pShape);
                                }
                            }
                        }
                    }
                    pShape->Release();
                }
                pAllFlatRange->Release();
            }
            pPageShapes->Release();
        }
    }
    
    if (pKalipKare) {
        AddToCommonUnique(pKalipKare);
    }
    for (auto m : markerShapesOnPage) {
        AddToCommonUnique(m);
    }
    
    double xOffset = kalipW + 10.0;
    int index = 1;
    long kkID = pKalipKare ? GetLongProp(pKalipKare, L"StaticID") : 0;
    
    for (auto const& it : colorGroups) {
        COLORREF col = it.first;
        const auto& shapes = it.second;
        double currentOffset = xOffset * index;
        
        CString selType = _T("");
        if (m_parsedStoneDetails.find(col) != m_parsedStoneDetails.end()) {
            selType = m_parsedStoneDetails[col].second;
        }
        
        int r = GetRValue(col);
        int g = GetGValue(col);
        int b = GetBValue(col);
        
        // Duplicate common shapes (mold & marker stones) - keep original colors
        for (auto s : commonShapes) {
            VARIANT args[2]; VariantInit(&args[0]); VariantInit(&args[1]);
            args[1].vt = VT_R8; args[1].dblVal = currentOffset;
            args[0].vt = VT_R8; args[0].dblVal = 0.0;
            DISPPARAMS prms = { args, NULL, 2, 0 };
            VARIANT retDup; VariantInit(&retDup);
            DISPID dDup; OLECHAR* szDup = (OLECHAR*)L"Duplicate";
            if (SUCCEEDED(s->GetIDsOfNames(IID_NULL, &szDup, 1, LOCALE_USER_DEFAULT, &dDup))) {
                if (SUCCEEDED(s->Invoke(dDup, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prms, &retDup, NULL, NULL))) {
                    if (retDup.vt == VT_DISPATCH && retDup.pdispVal) {
                        IDispatch* pDupShape = retDup.pdispVal;
                        long sID = GetLongProp(s, L"StaticID");
                        if (sID == kkID) {
                            // Mold rectangle: keep black outline, ensure no fill, send to back
                            IDispatch* pFill = GetDispatchProp(pDupShape, L"Fill");
                            if (pFill) {
                                InvokeMethodNoArgs(pFill, L"ApplyNoFill");
                                pFill->Release();
                            }
                            InvokeMethodNoArgs(pDupShape, L"OrderToBack");
                        }
                        // Marker stones: keep original appearance (no color change)
                        pDupShape->Release();
                    }
                }
            }
        }
        
        // Duplicate design stones
        for (auto s : shapes) {
            VARIANT args[2]; VariantInit(&args[0]); VariantInit(&args[1]);
            args[1].vt = VT_R8; args[1].dblVal = currentOffset;
            args[0].vt = VT_R8; args[0].dblVal = 0.0;
            DISPPARAMS prms = { args, NULL, 2, 0 };
            VARIANT retDup; VariantInit(&retDup);
            DISPID dDup; OLECHAR* szDup = (OLECHAR*)L"Duplicate";
            if (SUCCEEDED(s->GetIDsOfNames(IID_NULL, &szDup, 1, LOCALE_USER_DEFAULT, &dDup))) {
                if (SUCCEEDED(s->Invoke(dDup, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prms, &retDup, NULL, NULL))) {
                    if (retDup.vt == VT_DISPATCH && retDup.pdispVal) {
                        IDispatch* pDupShape = retDup.pdispVal;
                        IDispatch* pFill = GetDispatchProp(pDupShape, L"Fill");
                        if (pFill) {
                            IDispatch* pColor = nullptr;
                            VARIANT rCol; VariantInit(&rCol);
                            if (SUCCEEDED(InvokeMethodNoArgsRet(m_pApp, L"CreateColor", &rCol)) && rCol.vt == VT_DISPATCH) {
                                pColor = rCol.pdispVal;
                                SetColorRGB(pColor, r, g, b);
                                DISPID dApply; OLECHAR* szApply = (OLECHAR*)L"ApplyUniformFill";
                                if (SUCCEEDED(pFill->GetIDsOfNames(IID_NULL, &szApply, 1, LOCALE_USER_DEFAULT, &dApply))) {
                                    VARIANT argC; VariantInit(&argC); argC.vt = VT_DISPATCH; argC.pdispVal = pColor;
                                    DISPPARAMS prmsC = { &argC, NULL, 1, 0 };
                                    pFill->Invoke(dApply, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsC, NULL, NULL, NULL);
                                }
                                pColor->Release();
                            }
                            pFill->Release();
                        }
                        pDupShape->Release();
                    }
                }
            }
        }
        
        // Duplicate text shapes
        for (auto s : textShapes) {
            COLORREF textCol = RGB(128, 128, 128);
            IDispatch* pFill = GetDispatchProp(s, L"Fill");
            if (pFill) {
                long ft = GetLongProp(pFill, L"Type");
                if (ft == 1) {
                    IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                    if (pColor) { textCol = GetColorRef(pColor); pColor->Release(); }
                }
                pFill->Release();
            }
            
            if (textCol == col) {
                VARIANT args[2]; VariantInit(&args[0]); VariantInit(&args[1]);
                args[1].vt = VT_R8; args[1].dblVal = currentOffset;
                args[0].vt = VT_R8; args[0].dblVal = 0.0;
                DISPPARAMS prms = { args, NULL, 2, 0 };
                VARIANT retDup; VariantInit(&retDup);
                DISPID dDup; OLECHAR* szDup = (OLECHAR*)L"Duplicate";
                if (SUCCEEDED(s->GetIDsOfNames(IID_NULL, &szDup, 1, LOCALE_USER_DEFAULT, &dDup))) {
                    if (SUCCEEDED(s->Invoke(dDup, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prms, &retDup, NULL, NULL))) {
                        if (retDup.vt == VT_DISPATCH && retDup.pdispVal) {
                            IDispatch* pDupShape = retDup.pdispVal;
                            IDispatch* pDupFill = GetDispatchProp(pDupShape, L"Fill");
                            if (pDupFill) {
                                IDispatch* pColor = nullptr;
                                VARIANT rCol; VariantInit(&rCol);
                                if (SUCCEEDED(InvokeMethodNoArgsRet(m_pApp, L"CreateColor", &rCol)) && rCol.vt == VT_DISPATCH) {
                                    pColor = rCol.pdispVal;
                                    SetColorRGB(pColor, r, g, b);
                                    DISPID dApply; OLECHAR* szApply = (OLECHAR*)L"ApplyUniformFill";
                                    if (SUCCEEDED(pDupFill->GetIDsOfNames(IID_NULL, &szApply, 1, LOCALE_USER_DEFAULT, &dApply))) {
                                        VARIANT argC; VariantInit(&argC); argC.vt = VT_DISPATCH; argC.pdispVal = pColor;
                                        DISPPARAMS prmsC = { &argC, NULL, 1, 0 };
                                        pDupFill->Invoke(dApply, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsC, NULL, NULL, NULL);
                                    }
                                    pColor->Release();
                                }
                                pDupFill->Release();
                            }
                            pDupShape->Release();
                        }
                    }
                }
            }
        }
        
        // Duplicate & filter info text
        if (pInfoTextShape) {
            VARIANT args[2]; VariantInit(&args[0]); VariantInit(&args[1]);
            args[1].vt = VT_R8; args[1].dblVal = currentOffset;
            args[0].vt = VT_R8; args[0].dblVal = 0.0;
            DISPPARAMS prms = { args, NULL, 2, 0 };
            VARIANT retDup; VariantInit(&retDup);
            DISPID dDup; OLECHAR* szDup = (OLECHAR*)L"Duplicate";
            if (SUCCEEDED(pInfoTextShape->GetIDsOfNames(IID_NULL, &szDup, 1, LOCALE_USER_DEFAULT, &dDup))) {
                if (SUCCEEDED(pInfoTextShape->Invoke(dDup, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prms, &retDup, NULL, NULL))) {
                    if (retDup.vt == VT_DISPATCH && retDup.pdispVal) {
                        IDispatch* pDupText = retDup.pdispVal;
                        
                        IDispatch* pText = GetDispatchProp(pDupText, L"Text");
                        if (pText) {
                            IDispatch* pStory = GetDispatchProp(pText, L"Story");
                            if (pStory) {
                                CString newText = _T("");
                                IDispatch* pParagraphs = GetDispatchProp(pStory, L"Paragraphs");
                                if (pParagraphs) {
                                    long paraCount = GetLongProp(pParagraphs, L"Count");
                                    for (long ln = 1; ln <= paraCount; ln++) {
                                        IDispatch* pPara = GetDispatchPropWithIntArg(pParagraphs, L"Item", ln);
                                        if (pPara) {
                                            CString paraText = GetStringProp(pPara, L"Text");
                                            paraText.Replace(_T("\r"), _T(""));
                                            paraText.Replace(_T("\n"), _T(""));
                                            
                                            bool keepLine = false;
                                            if (paraText.Find(_T("ztrass.com")) != -1) {
                                                keepLine = true;
                                            } else {
                                                IDispatch* pParaFill = GetDispatchProp(pPara, L"Fill");
                                                COLORREF lineCol = CLR_INVALID;
                                                if (pParaFill) {
                                                    long ft = GetLongProp(pParaFill, L"Type");
                                                    if (ft == 1) {
                                                        IDispatch* pColor = GetDispatchProp(pParaFill, L"UniformColor");
                                                        if (pColor) {
                                                            lineCol = GetColorRef(pColor);
                                                            pColor->Release();
                                                        }
                                                    }
                                                    pParaFill->Release();
                                                }
                                                
                                                if (lineCol != CLR_INVALID) {
                                                    if (abs(GetRValue(lineCol) - GetRValue(col)) < 15 &&
                                                        abs(GetGValue(lineCol) - GetGValue(col)) < 15 &&
                                                        abs(GetBValue(lineCol) - GetBValue(col)) < 15) {
                                                        keepLine = true;
                                                    }
                                                }
                                            }
                                            
                                            if (keepLine) {
                                                if (paraText.Find(_T("Renk")) != -1) {
                                                    paraText = _T("1 Renk");
                                                }
                                                if (!newText.IsEmpty()) newText += _T("\r");
                                                newText += paraText;
                                            }
                                            pPara->Release();
                                        }
                                    }
                                    pParagraphs->Release();
                                }
                                SetStringProp(pStory, L"Text", (WCHAR*)(const WCHAR*)newText);
                                pStory->Release();
                            }
                            pText->Release();
                        }
                        
                        // Set text color to stone's color (instead of green)
                        IDispatch* pDupFill = GetDispatchProp(pDupText, L"Fill");
                        if (pDupFill) {
                            IDispatch* pColor = nullptr;
                            VARIANT rCol; VariantInit(&rCol);
                            if (SUCCEEDED(InvokeMethodNoArgsRet(m_pApp, L"CreateColor", &rCol)) && rCol.vt == VT_DISPATCH) {
                                pColor = rCol.pdispVal;
                                SetColorRGB(pColor, r, g, b); // The stone's color!
                                DISPID dApply; OLECHAR* szApply = (OLECHAR*)L"ApplyUniformFill";
                                if (SUCCEEDED(pDupFill->GetIDsOfNames(IID_NULL, &szApply, 1, LOCALE_USER_DEFAULT, &dApply))) {
                                    VARIANT argC; VariantInit(&argC); argC.vt = VT_DISPATCH; argC.pdispVal = pColor;
                                    DISPPARAMS prmsC = { &argC, NULL, 1, 0 };
                                    pDupFill->Invoke(dApply, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsC, NULL, NULL, NULL);
                                }
                                pColor->Release();
                            }
                            pDupFill->Release();
                        }
                        
                        // Align 3 mm inside top-left corner of the mold
                        SetDoubleProp(pDupText, L"PositionX", kalipLeft + currentOffset + 3.0);
                        SetDoubleProp(pDupText, L"PositionY", kalipTop - 3.0);
                        pDupText->Release();
                    }
                }
            }
        }
        
        index++;
    }
    
    if (pKalipKare) pKalipKare->Release();
    if (pInfoTextShape) pInfoTextShape->Release();
    if (pPage) pPage->Release();
    for (auto m : markerShapesOnPage) {
        m->Release();
    }
    for (auto s : commonShapes) {
        s->Release();
    }
    for (auto s : textShapes) {
        s->Release();
    }
    for (auto const& it : colorGroups) {
        for (auto s : it.second) {
            s->Release();
        }
    }
    
    pFlatRange->Release();
    pSel->Release();
    pDoc->Release();
}

void CSearchDlg::OnBnClickedBtnMakineKalibi()
{
    if (!m_pApp) return;
    
    IDispatch* pTempSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pTempSel) {
        long c = GetLongProp(pTempSel, L"Count");
        pTempSel->Release();
        if (c == 1) OnBnClickedBtnList();
    }
    
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Makine Kalıbı");
    
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    SetLongProp(pDoc, L"Unit", 3); // mm
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (!pSel) { pDoc->Release(); return; }
    long selCount = GetLongProp(pSel, L"Count");
    if (selCount == 0) {
        AfxMessageBox(_T("Lütfen makine kalıbı hazırlanacak kalıbı seçin."));
        pSel->Release(); pDoc->Release(); return;
    }
    
    IDispatch* pSelFlat = nullptr;
    IDispatch* pSelShapes = GetDispatchProp(pSel, L"Shapes");
    DISPID dispSelFind; OLECHAR* szSelFind = (OLECHAR*)L"FindShapes";
    if (pSelShapes && SUCCEEDED(pSelShapes->GetIDsOfNames(IID_NULL, &szSelFind, 1, LOCALE_USER_DEFAULT, &dispSelFind))) {
        DISPPARAMS params = { NULL, NULL, 0, 0 };
        VARIANT retVal; VariantInit(&retVal);
        if (SUCCEEDED(pSelShapes->Invoke(dispSelFind, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
            if (retVal.vt == VT_DISPATCH) pSelFlat = retVal.pdispVal;
        }
    }
    if (pSelShapes) pSelShapes->Release();
    if (!pSelFlat) { pSel->Release(); pDoc->Release(); return; }
    long flatCount = GetLongProp(pSelFlat, L"Count");
    
    std::set<long> selectedIDs;
    for (long i = 1; i <= flatCount; i++) {
        IDispatch* pShape = GetDispatchPropWithIntArg(pSelFlat, L"Item", i);
        if (pShape) {
            selectedIDs.insert(GetLongProp(pShape, L"StaticID"));
            pShape->Release();
        }
    }
    
    double desenMinX = 999999.0, desenMaxX = -999999.0;
    double desenMinY = 999999.0, desenMaxY = -999999.0;
    bool hasStones = false;
    
    for (long i = 1; i <= flatCount; i++) {
        IDispatch* pShape = GetDispatchPropWithIntArg(pSelFlat, L"Item", i);
        if (!pShape) continue;
        long type = GetLongProp(pShape, L"Type");
        if (type != 8 && type != 6) {
            double w = GetDoubleProp(pShape, L"SizeWidth");
            double h = GetDoubleProp(pShape, L"SizeHeight");
            long fillType = 0;
            COLORREF fillCol = CLR_INVALID;
            IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
            if (pFill) {
                fillType = GetLongProp(pFill, L"Type");
                if (fillType == 1) {
                    IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                    if (pColor) {
                        fillCol = GetColorRef(pColor);
                        pColor->Release();
                    }
                }
                pFill->Release();
            }
            bool isRealStone = (w < 10.0 && h < 10.0 && fillType == 1 && fillCol != RGB(255, 255, 255) && !IsColorBlack(fillCol));
            if (isRealStone) {
                double l = GetDoubleProp(pShape, L"LeftX");
                double r = GetDoubleProp(pShape, L"RightX");
                double b = GetDoubleProp(pShape, L"BottomY");
                double t = GetDoubleProp(pShape, L"TopY");
                if (l < desenMinX) desenMinX = l;
                if (r > desenMaxX) desenMaxX = r;
                if (b < desenMinY) desenMinY = b;
                if (t > desenMaxY) desenMaxY = t;
                hasStones = true;
            }
        }
        pShape->Release();
    }
    
    if (!hasStones) {
        desenMinX = GetDoubleProp(pSel, L"LeftX");
        desenMaxX = GetDoubleProp(pSel, L"RightX");
        desenMinY = GetDoubleProp(pSel, L"BottomY");
        desenMaxY = GetDoubleProp(pSel, L"TopY");
    }
    
    // Find pKalipKare (largest shape in selection with no area limit)
    IDispatch* pKalipKare = nullptr;
    double maxArea = 0.0;
    double kalipLeft = 0.0, kalipRight = 0.0, kalipBottom = 0.0, kalipTop = 0.0;
    
    for (long i = 1; i <= flatCount; i++) {
        IDispatch* pShape = GetDispatchPropWithIntArg(pSelFlat, L"Item", i);
        if (!pShape) continue;
        long type = GetLongProp(pShape, L"Type");
        if (type != 8 && type != 6) {
            double w = GetDoubleProp(pShape, L"SizeWidth");
            double h = GetDoubleProp(pShape, L"SizeHeight");
            double area = w * h;
            
            long fillType = 0;
            COLORREF fillCol = CLR_INVALID;
            IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
            if (pFill) {
                fillType = GetLongProp(pFill, L"Type");
                if (fillType == 1) {
                    IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                    if (pColor) {
                        fillCol = GetColorRef(pColor);
                        pColor->Release();
                    }
                }
                pFill->Release();
            }
            bool isRealStone = (w < 10.0 && h < 10.0 && fillType == 1 && fillCol != RGB(255, 255, 255) && !IsColorBlack(fillCol));
            
            if (!isRealStone && area > maxArea) {
                maxArea = area;
                if (pKalipKare) pKalipKare->Release();
                pKalipKare = pShape;
                pKalipKare->AddRef();
                kalipLeft = GetDoubleProp(pShape, L"LeftX");
                kalipRight = GetDoubleProp(pShape, L"RightX");
                kalipBottom = GetDoubleProp(pShape, L"BottomY");
                kalipTop = GetDoubleProp(pShape, L"TopY");
            }
        }
        pShape->Release();
    }
    
    IDispatch* pPage = GetDispatchProp(pDoc, L"ActivePage");
    std::vector<IDispatch*> markerShapesOnPage;
    
    if (!pKalipKare && pPage) {
        // Fallback to page shapes enclosing selection with 15mm tolerance
        IDispatch* pPageShapes = GetDispatchProp(pPage, L"Shapes");
        if (pPageShapes) {
            DISPID dFindAll; OLECHAR* szFindAll = (OLECHAR*)L"FindShapes";
            if (SUCCEEDED(pPageShapes->GetIDsOfNames(IID_NULL, &szFindAll, 1, LOCALE_USER_DEFAULT, &dFindAll))) {
                DISPPARAMS paramsAll = { NULL, NULL, 0, 0 };
                VARIANT retValAll; VariantInit(&retValAll);
                if (SUCCEEDED(pPageShapes->Invoke(dFindAll, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsAll, &retValAll, NULL, NULL))) {
                    if (retValAll.vt == VT_DISPATCH && retValAll.pdispVal) {
                        IDispatch* pAllFlatRange = retValAll.pdispVal;
                        long allCount = GetLongProp(pAllFlatRange, L"Count");
                        for (long i = 1; i <= allCount; ++i) {
                            IDispatch* pShape = GetDispatchPropWithIntArg(pAllFlatRange, L"Item", i);
                            if (!pShape) continue;
                            long type = GetLongProp(pShape, L"Type");
                            if (type != 8 && type != 6) {
                                double sLeft = GetDoubleProp(pShape, L"LeftX");
                                double sRight = GetDoubleProp(pShape, L"RightX");
                                double sBottom = GetDoubleProp(pShape, L"BottomY");
                                double sTop = GetDoubleProp(pShape, L"TopY");
                                if (sLeft <= desenMinX + 15.0 && sRight >= desenMaxX - 15.0 &&
                                    sBottom <= desenMinY + 15.0 && sTop >= desenMaxY - 15.0) {
                                    double w = GetDoubleProp(pShape, L"SizeWidth");
                                    double h = GetDoubleProp(pShape, L"SizeHeight");
                                    double area = w * h;
                                    
                                    long fillType = 0;
                                    COLORREF fillCol = CLR_INVALID;
                                    IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
                                    if (pFill) {
                                        fillType = GetLongProp(pFill, L"Type");
                                        if (fillType == 1) {
                                            IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                                            if (pColor) {
                                                fillCol = GetColorRef(pColor);
                                                pColor->Release();
                                            }
                                        }
                                        pFill->Release();
                                    }
                                    bool isRealStone = (w < 10.0 && h < 10.0 && fillType == 1 && fillCol != RGB(255, 255, 255) && !IsColorBlack(fillCol));
                                    
                                    if (!isRealStone && area > maxArea) {
                                        maxArea = area;
                                        if (pKalipKare) pKalipKare->Release();
                                        pKalipKare = pShape;
                                        pKalipKare->AddRef();
                                        kalipLeft = sLeft;
                                        kalipRight = sRight;
                                        kalipBottom = sBottom;
                                        kalipTop = sTop;
                                    }
                                }
                            }
                            pShape->Release();
                        }
                        pAllFlatRange->Release();
                    }
                }
            }
            pPageShapes->Release();
        }
    }
    
    if (pKalipKare && pPage) {
        // Find non-selected marker shapes on the page inside the mold rectangle
        IDispatch* pPageShapes = GetDispatchProp(pPage, L"Shapes");
        if (pPageShapes) {
            DISPID dFindAll; OLECHAR* szFindAll = (OLECHAR*)L"FindShapes";
            if (SUCCEEDED(pPageShapes->GetIDsOfNames(IID_NULL, &szFindAll, 1, LOCALE_USER_DEFAULT, &dFindAll))) {
                DISPPARAMS paramsAll = { NULL, NULL, 0, 0 };
                VARIANT retValAll; VariantInit(&retValAll);
                if (SUCCEEDED(pPageShapes->Invoke(dFindAll, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsAll, &retValAll, NULL, NULL))) {
                    if (retValAll.vt == VT_DISPATCH && retValAll.pdispVal) {
                        IDispatch* pAllFlatRange = retValAll.pdispVal;
                        long allCount = GetLongProp(pAllFlatRange, L"Count");
                        long kkStaticID = GetLongProp(pKalipKare, L"StaticID");
                        
                        for (long i = 1; i <= allCount; ++i) {
                            IDispatch* pShape = GetDispatchPropWithIntArg(pAllFlatRange, L"Item", i);
                            if (!pShape) continue;
                            long shapeID = GetLongProp(pShape, L"StaticID");
                            if (shapeID != kkStaticID) {
                                long type = GetLongProp(pShape, L"Type");
                                if (type != 8 && type != 6) {
                                    double sLeft = GetDoubleProp(pShape, L"LeftX");
                                    double sRight = GetDoubleProp(pShape, L"RightX");
                                    double sBottom = GetDoubleProp(pShape, L"BottomY");
                                    double sTop = GetDoubleProp(pShape, L"TopY");
                                    bool isInside = (sLeft >= kalipLeft - 10.0 && sRight <= kalipRight + 10.0 &&
                                                     sBottom >= kalipBottom - 10.0 && sTop <= kalipTop + 10.0);
                                    if (isInside) {
                                        // Check if not a real stone
                                        double w = GetDoubleProp(pShape, L"SizeWidth");
                                        double h = GetDoubleProp(pShape, L"SizeHeight");
                                        long fillType = 0;
                                        COLORREF fillCol = CLR_INVALID;
                                        IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
                                        if (pFill) {
                                            fillType = GetLongProp(pFill, L"Type");
                                            if (fillType == 1) {
                                                IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                                                if (pColor) {
                                                    fillCol = GetColorRef(pColor);
                                                    pColor->Release();
                                                }
                                            }
                                            pFill->Release();
                                        }
                                        bool isRealStone = (w < 10.0 && h < 10.0 && fillType == 1 && fillCol != RGB(255, 255, 255) && !IsColorBlack(fillCol));
                                        if (!isRealStone && selectedIDs.find(shapeID) == selectedIDs.end()) {
                                            pShape->AddRef();
                                            markerShapesOnPage.push_back(pShape);
                                        }
                                    }
                                }
                            }
                            pShape->Release();
                        }
                        pAllFlatRange->Release();
                    }
                }
            }
            pPageShapes->Release();
        }
    }
    
    // Create new ShapeRange to combine selection, mold and marker shapes
    IDispatch* pMyRange = nullptr;
    VARIANT rRange; VariantInit(&rRange);
    if (SUCCEEDED(InvokeMethodNoArgsRet(m_pApp, L"CreateShapeRange", &rRange)) && rRange.vt == VT_DISPATCH) {
        pMyRange = rRange.pdispVal;
    }
    
    if (pMyRange) {
        DISPID dAdd; OLECHAR* szAdd = (OLECHAR*)L"Add";
        if (SUCCEEDED(pMyRange->GetIDsOfNames(IID_NULL, &szAdd, 1, LOCALE_USER_DEFAULT, &dAdd))) {
            // Add selected shapes
            for (long i = 1; i <= flatCount; i++) {
                IDispatch* pItem = GetDispatchPropWithIntArg(pSelFlat, L"Item", i);
                if (pItem) {
                    VARIANT arg; VariantInit(&arg); arg.vt = VT_DISPATCH; arg.pdispVal = pItem;
                    DISPPARAMS prms = { &arg, NULL, 1, 0 };
                    pMyRange->Invoke(dAdd, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prms, NULL, NULL, NULL);
                    pItem->Release();
                }
            }
            // Add pKalipKare
            if (pKalipKare) {
                bool found = false;
                long targetID = GetLongProp(pKalipKare, L"StaticID");
                long myCount = GetLongProp(pMyRange, L"Count");
                for (long i = 1; i <= myCount; i++) {
                    IDispatch* pItem = GetDispatchPropWithIntArg(pMyRange, L"Item", i);
                    if (pItem) {
                        long itemID = GetLongProp(pItem, L"StaticID");
                        pItem->Release();
                        if (itemID == targetID) { found = true; break; }
                    }
                }
                if (!found) {
                    VARIANT arg; VariantInit(&arg); arg.vt = VT_DISPATCH; arg.pdispVal = pKalipKare;
                    DISPPARAMS prms = { &arg, NULL, 1, 0 };
                    pMyRange->Invoke(dAdd, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prms, NULL, NULL, NULL);
                }
            }
            // Add marker shapes
            for (auto m : markerShapesOnPage) {
                bool found = false;
                long targetID = GetLongProp(m, L"StaticID");
                long myCount = GetLongProp(pMyRange, L"Count");
                for (long i = 1; i <= myCount; i++) {
                    IDispatch* pItem = GetDispatchPropWithIntArg(pMyRange, L"Item", i);
                    if (pItem) {
                        long itemID = GetLongProp(pItem, L"StaticID");
                        pItem->Release();
                        if (itemID == targetID) { found = true; break; }
                    }
                }
                if (!found) {
                    VARIANT arg; VariantInit(&arg); arg.vt = VT_DISPATCH; arg.pdispVal = m;
                    DISPPARAMS prms = { &arg, NULL, 1, 0 };
                    pMyRange->Invoke(dAdd, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prms, NULL, NULL, NULL);
                }
            }
        }
    }
    
    struct SizeMap { double origMM; double machineMM; int editID; };
    SizeMap sizeTable[] = {
        {2.0, 0.0, IDC_EDT_2MM},
        {3.0, 0.0, IDC_EDT_2MM2},
        {4.0, 0.0, IDC_EDT_2MM3},
        {5.0, 0.0, IDC_EDT_2MM4},
        {6.0, 0.0, IDC_EDT_2MM5},
        {7.0, 0.0, IDC_EDT_2MM6},
        {8.0, 0.0, IDC_EDT_2MM7}
    };
    int tableSize = sizeof(sizeTable) / sizeof(sizeTable[0]);
    
    for (int i = 0; i < tableSize; i++) {
        CString strVal;
        GetDlgItemText(sizeTable[i].editID, strVal);
        strVal.Replace(L",", L".");
        double val = _wtof(strVal);
        sizeTable[i].machineMM = val;
    }
    
    double minSize = 0.6;
    double selH = pMyRange ? GetDoubleProp(pMyRange, L"SizeHeight") : GetDoubleProp(pSel, L"SizeHeight");
    
    double offset = -selH - 10.0;
    VARIANT args[2]; VariantInit(&args[0]); VariantInit(&args[1]);
    args[1].vt = VT_R8; args[1].dblVal = 0.0;
    args[0].vt = VT_R8; args[0].dblVal = offset;
    DISPPARAMS paramsDup = { args, NULL, 2, 0 };
    VARIANT retDup; VariantInit(&retDup);
    IDispatch* pDupRange = nullptr;
    DISPID dispDup; OLECHAR* szDup = (OLECHAR*)L"Duplicate";
    
    IDispatch* pSourceRange = pMyRange ? pMyRange : pSel;
    if (pSourceRange->GetIDsOfNames(IID_NULL, &szDup, 1, LOCALE_USER_DEFAULT, &dispDup) == S_OK) {
        if (pSourceRange->Invoke(dispDup, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsDup, &retDup, NULL, NULL) == S_OK) {
            if (retDup.vt == VT_DISPATCH) {
                pDupRange = retDup.pdispVal;
                // UngroupAll returns a new ShapeRange containing all ungrouped shapes.
                // We update pDupRange with this new pointer.
                VARIANT rUngroup; VariantInit(&rUngroup);
                if (SUCCEEDED(InvokeMethodNoArgsRet(pDupRange, L"UngroupAll", &rUngroup)) && rUngroup.vt == VT_DISPATCH) {
                    pDupRange->Release();
                    pDupRange = rUngroup.pdispVal;
                }
            }
        }
    }
    
    if (!pDupRange) { 
        if (pMyRange) pMyRange->Release();
        if (pKalipKare) pKalipKare->Release();
        for (auto m : markerShapesOnPage) m->Release();
        pSelFlat->Release(); pSel->Release(); pDoc->Release(); return; 
    }
    
    IDispatch* pDupFlatRange = nullptr;
    IDispatch* pDupShapes = GetDispatchProp(pDupRange, L"Shapes");
    DISPID dispFind; OLECHAR* szFind = (OLECHAR*)L"FindShapes";
    if (pDupShapes && SUCCEEDED(pDupShapes->GetIDsOfNames(IID_NULL, &szFind, 1, LOCALE_USER_DEFAULT, &dispFind))) {
        DISPPARAMS params = { NULL, NULL, 0, 0 };
        VARIANT retVal; VariantInit(&retVal);
        if (SUCCEEDED(pDupShapes->Invoke(dispFind, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
            if (retVal.vt == VT_DISPATCH) pDupFlatRange = retVal.pdispVal;
        }
    }
    if (pDupShapes) pDupShapes->Release();
    if (!pDupFlatRange) { 
        pDupRange->Release(); 
        if (pMyRange) pMyRange->Release();
        if (pKalipKare) pKalipKare->Release();
        for (auto m : markerShapesOnPage) m->Release();
        pSelFlat->Release(); pSel->Release(); pDoc->Release(); return; 
    }
    long dupFlatCount = GetLongProp(pDupFlatRange, L"Count");
    
    // Find pDupKalipKare (largest shape in dupFlatRange of any type except text)
    IDispatch* pDupKalipKare = nullptr;
    double maxDupArea = 0.0;
    for (long j = 1; j <= dupFlatCount; j++) {
        IDispatch* pShape = GetDispatchPropWithIntArg(pDupFlatRange, L"Item", j);
        if (pShape) {
            long type = GetLongProp(pShape, L"Type");
            if (type != 8 && type != 6) {
                double w = GetDoubleProp(pShape, L"SizeWidth");
                double h = GetDoubleProp(pShape, L"SizeHeight");
                double area = w * h;
                if (area > maxDupArea) {
                    maxDupArea = area;
                    if (pDupKalipKare) pDupKalipKare->Release();
                    pDupKalipKare = pShape;
                    pDupKalipKare->AddRef();
                }
            }
            pShape->Release();
        }
    }
    
    for (long i = 1; i <= dupFlatCount; i++) {
        IDispatch* pShape = GetDispatchPropWithIntArg(pDupFlatRange, L"Item", i);
        if (!pShape) continue;
        
        long staticID = GetLongProp(pShape, L"StaticID");
        bool isProtected = false;
        if (pDupKalipKare && staticID == GetLongProp(pDupKalipKare, L"StaticID")) isProtected = true;
        
        if (isProtected) {
            pShape->Release();
            continue;
        }
        
        long type = GetLongProp(pShape, L"Type");
        if (type == 6) {
            // Do not delete group container, just skip it as its sub-shapes are already processed.
            pShape->Release();
            continue;
        }
        if (type == 8) {
            DISPID dispidDelete; OLECHAR* szDelete = (OLECHAR*)L"Delete";
            if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szDelete, 1, LOCALE_USER_DEFAULT, &dispidDelete))) {
                DISPPARAMS delParams = { NULL, NULL, 0, 0 };
                pShape->Invoke(dispidDelete, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &delParams, NULL, NULL, NULL);
            }
            pShape->Release();
            continue;
        }
        
        // Convert and resize to machine size (all other shapes including stones and markers)
        double origW = GetDoubleProp(pShape, L"SizeWidth");
        double cx = GetDoubleProp(pShape, L"CenterX");
        double cy = GetDoubleProp(pShape, L"CenterY");
        
        double newSize = minSize;
        double bestDiff = 9999.0;
        for (int t = 0; t < tableSize; t++) {
            double diff = fabs(origW - sizeTable[t].origMM);
            if (diff < bestDiff) {
                bestDiff = diff;
                if (sizeTable[t].machineMM > 0.01) {
                    newSize = sizeTable[t].machineMM;
                }
            }
        }
        
        if (newSize < minSize) newSize = minSize;
        
        if (type != 2) {
            // Delete original
            DISPID dispidDelete; OLECHAR* szDelete = (OLECHAR*)L"Delete";
            if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szDelete, 1, LOCALE_USER_DEFAULT, &dispidDelete))) {
                DISPPARAMS delParams = { NULL, NULL, 0, 0 };
                pShape->Invoke(dispidDelete, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &delParams, NULL, NULL, NULL);
            }
            
            // Draw circle (ellipse)
            IDispatch* pActiveLayer = GetDispatchProp(pDoc, L"ActiveLayer");
            if (pActiveLayer) {
                VARIANT args[4];
                args[3].vt = VT_R8; args[3].dblVal = cx - newSize / 2.0; // Left
                args[2].vt = VT_R8; args[2].dblVal = cy + newSize / 2.0; // Top
                args[1].vt = VT_R8; args[1].dblVal = cx + newSize / 2.0; // Right
                args[0].vt = VT_R8; args[0].dblVal = cy - newSize / 2.0; // Bottom
                DISPPARAMS prms = { args, NULL, 4, 0 };
                VARIANT rEllipse; VariantInit(&rEllipse);
                DISPID dCreate; OLECHAR* szCreate = (OLECHAR*)L"CreateEllipse";
                if (SUCCEEDED(pActiveLayer->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dCreate))) {
                    if (SUCCEEDED(pActiveLayer->Invoke(dCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prms, &rEllipse, NULL, NULL))) {
                        if (rEllipse.vt == VT_DISPATCH && rEllipse.pdispVal) {
                            IDispatch* pNewEllipse = rEllipse.pdispVal;
                            
                            // Fill with black
                            IDispatch* pFillNew = GetDispatchProp(pNewEllipse, L"Fill");
                            if (pFillNew) {
                                IDispatch* pColor = nullptr;
                                VARIANT rCol; VariantInit(&rCol);
                                if (SUCCEEDED(InvokeMethodNoArgsRet(m_pApp, L"CreateColor", &rCol)) && rCol.vt == VT_DISPATCH) {
                                    pColor = rCol.pdispVal;
                                    SetColorRGB(pColor, 0, 0, 0); // Black
                                    DISPID dApply; OLECHAR* szApply = (OLECHAR*)L"ApplyUniformFill";
                                    if (SUCCEEDED(pFillNew->GetIDsOfNames(IID_NULL, &szApply, 1, LOCALE_USER_DEFAULT, &dApply))) {
                                        VARIANT argC; VariantInit(&argC); argC.vt = VT_DISPATCH; argC.pdispVal = pColor;
                                        DISPPARAMS prmsC = { &argC, NULL, 1, 0 };
                                        pFillNew->Invoke(dApply, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsC, NULL, NULL, NULL);
                                    }
                                    pColor->Release();
                                }
                                pFillNew->Release();
                            }
                            pNewEllipse->Release();
                        }
                    }
                }
                pActiveLayer->Release();
            }
        } else {
            // Already a circle, just resize and reposition
            SetDoubleProp(pShape, L"SizeWidth", newSize);
            SetDoubleProp(pShape, L"SizeHeight", newSize);
            SetDoubleProp(pShape, L"CenterX", cx);
            SetDoubleProp(pShape, L"CenterY", cy);
        }
        
        pShape->Release();
    }
    
    InvokeMethodNoArgs(pDupRange, L"CreateSelection");
    
    pDupFlatRange->Release();
    pDupRange->Release();
    if (pMyRange) pMyRange->Release();
    
    if (pDupKalipKare) pDupKalipKare->Release();
    if (pKalipKare) pKalipKare->Release();
    for (auto m : markerShapesOnPage) m->Release();
    
    pSelFlat->Release();
    pSel->Release();
    pDoc->Release();
}

void CSearchDlg::SetStatus(LPCTSTR lpszText)
{
    if (m_wndStatusBar.GetSafeHwnd()) {
        m_wndStatusBar.SetText(lpszText, 0, 0);
    }
}

void CSearchDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);
    if (m_wndStatusBar.GetSafeHwnd()) {
        RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
    }
}




BOOL CSearchDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
    WORD notification = HIWORD(wParam);
    WORD id = LOWORD(wParam);
    
    if (notification == CBN_SELCHANGE) {
        if (id >= 2000 && id < 3000) { // Tas Adi Combo
            int index = id - 2000;
            if (index >= 0 && index < (int)m_listObjects.size() && index < (int)m_combosTas.size()) {
                CComboBox* pTas = m_combosTas[index];
                if (::IsWindow(pTas->GetSafeHwnd())) {
                    CString tasAdi;
                    pTas->GetLBText(pTas->GetCurSel(), tasAdi);
                    COLORREF col = m_listObjects[index].color;
                    m_parsedStoneDetails[col].first = tasAdi;
                }
            }
        }
        else if (id >= 3000 && id < 4000) { // Tas Tipi Combo
            int index = id - 3000;
            if (index >= 0 && index < (int)m_listObjects.size() && index < (int)m_combosTip.size()) {
                CComboBox* pTip = m_combosTip[index];
                if (::IsWindow(pTip->GetSafeHwnd())) {
                    CString tipAdi;
                    pTip->GetLBText(pTip->GetCurSel(), tipAdi);
                    
                    COLORREF col = m_listObjects[index].color;
                    m_parsedStoneDetails[col].second = tipAdi;
                    
                    // Recalculate button label
                    auto GetSsSize = [](double size) -> CString {
                        if (size <= 2.2) return _T("Ss6");
                        if (size <= 2.6) return _T("Ss8");
                        if (size <= 3.2) return _T("Ss10");
                        if (size <= 3.6) return _T("Ss12");
                        if (size <= 4.2) return _T("Ss16");
                        if (size <= 5.2) return _T("Ss20");
                        if (size <= 6.6) return _T("Ss30");
                        if (size <= 7.6) return _T("Ss34");
                        return _T("Ss40");
                    };
                    
                    double w = m_listObjects[index].width;
                    double h = m_listObjects[index].height;
                    int cnt = m_listObjects[index].count;
                    
                    CString wStr;
                    if (tipAdi == _T("Dbl") || tipAdi == _T("Mc")) {
                        wStr = GetSsSize(w);
                    } else {
                        if (fabs(w - (int)w) < 0.05) {
                            wStr.Format(_T("%dmm"), (int)w);
                        } else {
                            wStr.Format(_T("%.1fmm"), w);
                            wStr.Replace(_T("."), _T(","));
                        }
                    }
                    
                    CString hStr;
                    if (tipAdi == _T("Dbl") || tipAdi == _T("Mc")) {
                        hStr = GetSsSize(h);
                    } else {
                        if (fabs(h - (int)h) < 0.05) {
                            hStr.Format(_T("%dmm"), (int)h);
                        } else {
                            hStr.Format(_T("%.1fmm"), h);
                            hStr.Replace(_T("."), _T(","));
                        }
                    }
                    
                    CString objText;
                    if (fabs(w - h) < 0.05) {
                        objText.Format(_T("%s-%d"), wStr, cnt);
                    } else {
                        objText.Format(_T("%s/%s-%d"), wStr, hStr, cnt);
                    }
                    
                    m_listObjects[index].text = objText;
                    
                    // Repaint to reflect changes instantly
                    Invalidate();
                }
            }
        }
    }
    
    return CDialog::OnCommand(wParam, lParam);
}
