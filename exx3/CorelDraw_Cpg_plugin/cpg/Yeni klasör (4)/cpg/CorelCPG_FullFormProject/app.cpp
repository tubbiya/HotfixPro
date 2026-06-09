#import "C:\\Program Files\\Corel\\CorelDRAW Graphics Suite X8\\Draw\\VGCoreAuto.tlb" \
    rename_namespace("VGCore") named_guids

#define corel VGCore::IVGApplication
using namespace VGCore;

bool Copy_1K_Objects(corel *cdr);

int cpg_main()
{
  HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  if (SUCCEEDED(hr)) {
    VGCore::IVGApplicationPtr app(L"CorelDRAW.Application.18"); // CorelDRAW X8
    app->Visible = VARIANT_TRUE;
    auto doc = app->ActiveDocument;
    if (!doc)
      doc = app->CreateDocument();
    Copy_1K_Objects(app);
    CoUninitialize();
  }
  return 0;
}

bool Copy_1K_Objects(corel *cdr)
{
  auto sr = cdr->ActiveSelectionRange;
  double sw = sr->SizeWidth;
  double sh = sr->SizeHeight;
  int n = 10;
  auto dup = sr->StepAndRepeat(n - 1, sw, 0.0, cdrModeOffset, cdrRight, cdrModeOffset, cdrDown);
  sr->AddRange(dup);
  auto dup_all = sr->StepAndRepeat(n - 1, 0.0, sh, cdrModeOffset, cdrRight, cdrModeOffset, cdrDown);
  sr->AddRange(dup_all);
  return true;
}
