#include <windows.h>
#define ID_BUTTON_TOOLS 1
#define ID_BUTTON_MOVE 2
#define ID_BUTTON_CLOSE 3

int cpg_main();
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case ID_BUTTON_TOOLS: {
      MessageBox(hwnd, "Copy_1K_Objects!", "Info", MB_OK);
      cpg_main(); // 调用 app.cpp 文件中 main() 函数
    } break;
    case ID_BUTTON_MOVE: {
      MoveWindow(hwnd, GetSystemMetrics(SM_CXSCREEN) / 2, 0, 90, 30, TRUE);
      break;
    }
    case ID_BUTTON_CLOSE:
      PostMessage(hwnd, WM_CLOSE, 0, 0);
      break;
    }
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  default:
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
  return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
  const char CLASS_NAME[] = "NoTitleWindow";

  WNDCLASS wc = {};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.style = CS_HREDRAW | CS_VREDRAW;

  RegisterClass(&wc);
  int windowWidth = 90;
  int windowHeight = 30;
  int x = (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2;
  int y = (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 2;

  HWND hwnd = CreateWindowEx(WS_EX_TOPMOST, CLASS_NAME, NULL, WS_POPUP, x, y, windowWidth, windowHeight, NULL, NULL,
                             hInstance, NULL);
  if (hwnd == NULL) {
    return 0;
  }

  HWND hButtonTools = CreateWindow("BUTTON", "LYCPG", WS_VISIBLE | WS_CHILD | BS_FLAT, 0, 0, 60, 30, hwnd,
                                   (HMENU)ID_BUTTON_TOOLS, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
  HWND hButtonMove = CreateWindow("BUTTON", ">", WS_VISIBLE | WS_CHILD | BS_FLAT, 60, 15, 30, 15, hwnd,
                                  (HMENU)ID_BUTTON_MOVE, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
  HWND hButtonClose = CreateWindow("BUTTON", "X", WS_VISIBLE | WS_CHILD | BS_FLAT, 60, 0, 30, 15, hwnd,
                                   (HMENU)ID_BUTTON_CLOSE, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
  ShowWindow(hwnd, nShowCmd);
  UpdateWindow(hwnd);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return 0;
}