#include <windows.h>
#include "resource.h"

int cpg_main();

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        {
            HWND hCombo = GetDlgItem(hwndDlg, IDC_COMBO_OPTIONS);
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"Option 1");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"Option 2");
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)"Option 3");
            SendMessage(hCombo, CB_SETCURSEL, 0, 0);
        }
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BUTTON_TOOLS:
            cpg_main();
            return TRUE;
        case IDC_BUTTON_MOVE:
            MessageBox(hwndDlg, "Move clicked", "Info", MB_OK);
            return TRUE;
        case IDC_BUTTON_CLOSE:
            EndDialog(hwndDlg, 0);
            return TRUE;
        case IDC_COMBO_OPTIONS:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                HWND hCombo = GetDlgItem(hwndDlg, IDC_COMBO_OPTIONS);
                int idx = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0);
                char buf[256];
                SendMessage(hCombo, CB_GETLBTEXT, idx, (LPARAM)buf);
                MessageBox(hwndDlg, buf, "Selected", MB_OK);
            }
            return TRUE;
        }
        break;
    case WM_CLOSE:
        EndDialog(hwndDlg, 0);
        return TRUE;
    }
    return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    return DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL, DialogProc);
}
