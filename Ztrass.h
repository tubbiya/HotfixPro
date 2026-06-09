#pragma once
#include "SearchDlg.h"

// CorelDRAW CPG eklenti ihraç fonksiyonları (Dışa Aktarılanlar)
extern "C" {
    __declspec(dllexport) DWORD APIENTRY GetPluginVersion();
    __declspec(dllexport) const char* APIENTRY GetPluginName();
    __declspec(dllexport) BOOL APIENTRY AttachPlugin(IDispatch* pApp);
    __declspec(dllexport) void APIENTRY DetachPlugin();
    __declspec(dllexport) void APIENTRY TriggerSearchDialog(IDispatch* pApp);
}

// Global Durum Değişkenleri
extern HHOOK g_hKeyboardHook;
extern CSearchDlg* g_pSearchDlg;
extern IDispatch* g_pApp;

// Klavye Kanca ve Dialog Yönetim Fonksiyonları
LRESULT CALLBACK KeyboardHookProc(int code, WPARAM wParam, LPARAM lParam);
