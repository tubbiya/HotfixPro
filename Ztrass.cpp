#include "Ztrass.h"

// MFC DLL İlklendirmesi için Uygulama Nesnesi
class CZtrassApp : public CWinApp
{
public:
    CZtrassApp() {}
    virtual BOOL InitInstance() override { 
        AfxInitRichEdit2(); // RichEdit2.0 kontrollerinin yüklenmesini sağlar
        return CWinApp::InitInstance(); 
    }
};
CZtrassApp theApp;

// Global Değişken Tanımlamaları
HHOOK g_hKeyboardHook = nullptr;
HHOOK g_hGetMsgHook = nullptr;
CSearchDlg* g_pSearchDlg = nullptr;
IDispatch* g_pApp = nullptr;

LRESULT CALLBACK GetMessageHookProc(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK KeyboardHookProc(int code, WPARAM wParam, LPARAM lParam);

// 1. Eklenti Sürüm Numarasını Döner
extern "C" __declspec(dllexport) DWORD APIENTRY GetPluginVersion()
{
    return 1;
}

// 2. Eklenti İsmini Döner
extern "C" __declspec(dllexport) const char* APIENTRY GetPluginName()
{
    return "Ztrass High-Performance Search CPG";
}

// 3. CorelDRAW Tarafından Yüklenme Anı (Giriş Noktası)
extern "C" __declspec(dllexport) BOOL APIENTRY AttachPlugin(IDispatch* pApp)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    if (!pApp) return FALSE;

    g_pApp = pApp;
    g_pApp->AddRef(); // COM Referans Sayısını Arttır

    // CorelDRAW'ın kendi thread'i üzerinde klavye kancalarını kuruyoruz
    DWORD threadId = ::GetCurrentThreadId();
    g_hKeyboardHook = ::SetWindowsHookEx(WH_KEYBOARD, KeyboardHookProc, NULL, threadId);
    g_hGetMsgHook = ::SetWindowsHookEx(WH_GETMESSAGE, GetMessageHookProc, NULL, threadId);

    return TRUE;
}

// 4. CorelDRAW Kapatılırken veya Eklenti Kaldırılırken (Çıkış Noktası)
extern "C" __declspec(dllexport) void APIENTRY DetachPlugin()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    // Klavye kancasını kaldır
    if (g_hKeyboardHook) {
        ::UnhookWindowsHookEx(g_hKeyboardHook);
        g_hKeyboardHook = nullptr;
    }
    if (g_hGetMsgHook) {
        ::UnhookWindowsHookEx(g_hGetMsgHook);
        g_hGetMsgHook = nullptr;
    }

    // Arama panelini kapat ve belleği serbest bırak
    if (g_pSearchDlg) {
        if (::IsWindow(g_pSearchDlg->GetSafeHwnd())) {
            g_pSearchDlg->DestroyWindow();
        }
        g_pSearchDlg = nullptr;
    }

    // COM referansını serbest bırak
    if (g_pApp) {
        g_pApp->Release();
        g_pApp = nullptr;
    }
}

// Klavye Kancası Geri Çağırım Metodu (Ctrl+F ve Edit kontrolleri için)
LRESULT CALLBACK KeyboardHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    if (code >= 0) {
        // Ctrl+F Yakalama
        bool bKeyDown = !(lParam & (1 << 31));
        if (bKeyDown && wParam == 'F') {
            bool bCtrlDown = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
            bool bAltDown = (::GetKeyState(VK_MENU) & 0x8000) != 0;
            bool bShiftDown = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;

            if (bCtrlDown && !bAltDown && !bShiftDown) {
                TriggerSearchDialog(g_pApp);
                return 1;
            }
        }
    }
    return ::CallNextHookEx(g_hKeyboardHook, code, wParam, lParam);
}

// Modeless pencerede klavye mesajları için (örn. TAB ile gezinme)
LRESULT CALLBACK GetMessageHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    if (code >= 0 && wParam == PM_REMOVE) {
        MSG* pMsg = (MSG*)lParam;
        if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) {
            if (g_pSearchDlg && ::IsWindow(g_pSearchDlg->GetSafeHwnd())) {
                HWND hwndTarget = pMsg->hwnd;
                if (hwndTarget && (hwndTarget == g_pSearchDlg->GetSafeHwnd() || ::IsChild(g_pSearchDlg->GetSafeHwnd(), hwndTarget))) {
                    if (g_pSearchDlg->PreTranslateMessage(pMsg)) {
                        pMsg->message = WM_NULL;
                        pMsg->lParam = 0;
                        pMsg->wParam = 0;
                    } else if (::IsDialogMessage(g_pSearchDlg->GetSafeHwnd(), pMsg)) {
                        pMsg->message = WM_NULL;
                        pMsg->lParam = 0;
                        pMsg->wParam = 0;
                    }
                }
            }
        }
    }
    return ::CallNextHookEx(g_hGetMsgHook, code, wParam, lParam);
}

// Arama Dialogunu Ekrana Getirme veya Ön Plana Çıkarma Fonksiyonu
extern "C" __declspec(dllexport) void APIENTRY TriggerSearchDialog(IDispatch* pApp)
{
    // VBA veya CorelDRAW tarafından gönderilen uygulama nesnesini doğrudan bağla
    if (pApp) {
        if (g_pApp) {
            g_pApp->Release();
        }
        g_pApp = pApp;
        g_pApp->AddRef();
    }

    // Eğer hala g_pApp atanmadıysa (parametre geçilmediyse), COM tablosundan çekmeye çalış
    if (!g_pApp) {
        ::CoInitialize(NULL);
        CLSID clsid;
        // Farklı versiyon ProgID'lerini deniyoruz (Sırasıyla en güncelden geriye doğru)
        const WCHAR* progIds[] = {
            L"CorelDRAW.Application",
            L"CorelDRAW.Application.18", // X8
            L"CorelDRAW.Application.17", // X7
            L"CorelDRAW.Application.19", // 2017
            L"CorelDRAW.Application.20", // 2018
            L"CorelDRAW.Application.21", // 2019
            L"CorelDRAW.Application.22", // 2020
            L"CorelDRAW.Application.23"  // 2021
        };

        for (int i = 0; i < sizeof(progIds)/sizeof(progIds[0]); ++i) {
            HRESULT hr = CLSIDFromProgID(progIds[i], &clsid);
            if (SUCCEEDED(hr)) {
                IUnknown* pUnk = nullptr;
                hr = GetActiveObject(clsid, NULL, &pUnk);
                if (SUCCEEDED(hr) && pUnk) {
                    hr = pUnk->QueryInterface(IID_IDispatch, (void**)&g_pApp);
                    pUnk->Release();
                    if (SUCCEEDED(hr) && g_pApp) break;
                }
            }
        }
    }

    // Arayüz zaten açıksa ön plana getir
    if (g_pSearchDlg != nullptr && ::IsWindow(g_pSearchDlg->GetSafeHwnd())) {
        g_pSearchDlg->SetForegroundWindow();
        return;
    }

    // CorelDRAW ana penceresini ebeveyn (parent) pencere olarak belirle
    HWND hWndCorel = ::GetActiveWindow();
    CWnd* pParent = CWnd::FromHandle(hWndCorel);

    // MFC modül durumunu bu DLL'e kaydır (Resource dosyasından IDD_SEARCH_DLG şablonunu okuyabilmek için şarttır)
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    // Modeless (Bagimsiz/Kayan) pencereyi olustur
    g_pSearchDlg = new CSearchDlg(g_pApp, nullptr);
    if (g_pSearchDlg->Create(IDD_SEARCH_DLG, CWnd::GetDesktopWindow())) {
        g_pSearchDlg->SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        g_pSearchDlg->ShowWindow(SW_SHOW);
    } else {
        delete g_pSearchDlg;
        g_pSearchDlg = nullptr;
    }
}
