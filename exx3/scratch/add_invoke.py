import io

path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path, 'r', encoding='utf-8') as f:
    text = f.read()

invoke_def = """static HRESULT InvokeMethod(IDispatch* pDisp, WCHAR* szMethod, VARIANT* pArgs, int cArgs)
{
    if (!pDisp) return E_POINTER;
    DISPID dispid;
    HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &szMethod, 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hr)) return hr;
    DISPPARAMS params = { pArgs, NULL, (UINT)cArgs, 0 };
    return pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, NULL, NULL, NULL);
}
"""

if 'static HRESULT InvokeMethod(IDispatch* pDisp' not in text:
    text = text.replace('static HRESULT InvokeMethodNoArgs(IDispatch* pDisp, WCHAR* szMethod)', invoke_def + '\nstatic HRESULT InvokeMethodNoArgs(IDispatch* pDisp, WCHAR* szMethod)')

with io.open(path, 'w', encoding='utf-8') as f:
    f.write(text)

print("InvokeMethod added.")
