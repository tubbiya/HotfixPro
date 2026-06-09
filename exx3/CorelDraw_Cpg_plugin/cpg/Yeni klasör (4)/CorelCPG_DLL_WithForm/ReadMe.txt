CorelCPG DLL Plugin with Form (Resource Editor)

How to use:
1. Open CorelCPG_DLL_WithForm.sln in Visual Studio 2022.
2. In Solution Explorer, double click "CorelCPG.rc" → this opens the Resource Editor.
   - You can drag & drop buttons, combo boxes, labels, etc.
3. Build (F7). Output is a DLL (CorelCPG_DLL_WithForm.dll).
4. Copy the DLL to CorelDRAW's Plugins/CPG folder.
5. Start CorelDRAW → the plugin will be loaded and you can trigger cpg_main.

Notes:
- You can add more UI controls via Resource Editor (like Qt Designer but for Win32).
- Events are handled in app.cpp, inside DialogProc.
