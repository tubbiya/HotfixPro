#include <windows.h>
#include "resource.h"

// Example plugin entry point called by CorelDRAW
extern "C" __declspec(dllexport) int cpg_main()
{
    MessageBox(NULL, "CPG Plugin Loaded", "CorelCPG", MB_OK);
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL, DialogProc);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BUTTON1:
            MessageBox(hwndDlg, "Button clicked!", "Info", MB_OK);
            return TRUE;
        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            return TRUE;
        }
        break;
    case WM_CLOSE:
        EndDialog(hwndDlg, 0);
        return TRUE;
    }
    return FALSE;
}
