#import "VGCoreAuto.tlb" // 导入VGCoreAuto类型库: 这个文件在CorelDRAW 软件目录下可以找到
#define corel VGCore::IVGApplication
using namespace VGCore;

bool Copy_1K_Objects(corel *cdr);

int cpg_main()
{
  // 初始化COM库，使用多线程公寓模型
  HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

  if (SUCCEEDED(hr)) {
    // 创建一个指向CorelDRAW应用程序的指针  CorelDRAW 2022 版本号是 24
    VGCore::IVGApplicationPtr app(L"CorelDRAW.Application.24");

    // 设置应用程序为可见状态
    app->Visible = VARIANT_TRUE;
    // 获取当前活动的文档，如果没有则创建一个新文档
    auto doc = app->ActiveDocument;
    if (!doc)
      doc = app->CreateDocument();

    Copy_1K_Objects(app);

    // 清理COM库的初始化
    CoUninitialize();
  }

  return 0;
}

//  1- A program that will copy a selected object 1000 times
bool Copy_1K_Objects(corel *cdr)
{
  auto sr = cdr->ActiveSelectionRange; // Get the selection range
  double sw = sr->SizeWidth;
  double sh = sr->SizeHeight;
  int n = 10;
  auto dup = sr->StepAndRepeat(n - 1, sw, 0.0, cdrModeOffset, cdrRight, cdrModeOffset, cdrDown);
  sr->AddRange(dup);
  auto dup_all = sr->StepAndRepeat(n - 1, 0.0, sh, cdrModeOffset, cdrRight, cdrModeOffset, cdrDown);
  sr->AddRange(dup_all);

  return true;
}

/***********   MSVC2022 compiles CorelDRAW CPG.exe command line syntax

cl.exe /nologo /utf-8 /w /EHsc /Ox /DNDEBUG /MD app.cpp cpg.cpp  /link  rpcrt4.lib msvcrt.lib shell32.lib user32.lib gdi32.lib

***********/