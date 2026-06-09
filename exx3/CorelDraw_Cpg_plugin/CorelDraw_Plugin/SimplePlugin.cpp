#include "pch.h"
#include "PluginDialog.h"

// DLL instance handle
HINSTANCE g_hInstance = NULL;

// CorelDRAW eklenti fonksiyonları
extern "C" __declspec(dllexport) BOOL WINAPI CPGPlugInInfo(LPSTR lpszName, int nMaxLen)
{
    strcpy_s(lpszName, nMaxLen, "Basit Eklenti");
    return TRUE;
}

extern "C" __declspec(dllexport) BOOL WINAPI CPGPlugInVersion(LPSTR lpszVersion, int nMaxLen)
{
    strcpy_s(lpszVersion, nMaxLen, "1.0");
    return TRUE;
}

extern "C" __declspec(dllexport) BOOL WINAPI CPGPlugInCommand()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    CPluginDialog dlg;
    dlg.DoModal();
    
    return TRUE;
}

extern "C" __declspec(dllexport) BOOL WINAPI CPGPlugInHelp()
{
    MessageBox(NULL, 
        _T("Bu basit bir CorelDRAW eklentisidir.\n\n")
        _T("Kullanım:\n")
        _T("1. Eklentiyi CorelDRAW'dan çalıştırın\n")
        _T("2. Metin kutusuna istediğiniz metni girin\n")
        _T("3. Tamam'a tıklayın"),
        _T("Yardım"), 
        MB_OK | MB_ICONINFORMATION);
    return TRUE;
}
