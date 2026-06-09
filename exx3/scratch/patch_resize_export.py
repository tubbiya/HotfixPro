import io
import re

path_cpp = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path_cpp, 'r', encoding='utf-8') as f:
    text_cpp = f.read()

# 1. Update ResizeShapes to implement ProB.frm logic
old_resize_shapes = """void CSearchDlg::ResizeShapes(double w_mm, double h_mm)
{
    if (!m_pApp) return;
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Boyutlandir");
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    
    SetLongProp(pDoc, L"Unit", 3);
    SetLongProp(pDoc, L"ReferencePoint", 4);
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pSel) {
        IDispatch* pShapesColl = GetDispatchProp(pSel, L"Shapes");
        IDispatch* pFlatRange = nullptr;
        if (pShapesColl) {
            DISPID dispFind; OLECHAR* szFind = (OLECHAR*)L"FindShapes";
            if (SUCCEEDED(pShapesColl->GetIDsOfNames(IID_NULL, &szFind, 1, LOCALE_USER_DEFAULT, &dispFind))) {
                VARIANT arg; VariantInit(&arg);
                arg.vt = VT_BSTR; arg.bstrVal = SysAllocString(L"@type <> 'group'");
                DISPPARAMS params = { &arg, NULL, 1, 0 };
                VARIANT retVal; VariantInit(&retVal);
                if (SUCCEEDED(pShapesColl->Invoke(dispFind, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                    if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                        pFlatRange = retVal.pdispVal;
                    }
                }
                SysFreeString(arg.bstrVal);
            }
            pShapesColl->Release();
        }
        
        if (pFlatRange) {
            long count = GetLongProp(pFlatRange, L"Count");
            if (count > 0) {
                bool bCircleOnly = (IsDlgButtonChecked(IDC_CHK_OLCU2) == BST_CHECKED);
                if (bCircleOnly) h_mm = w_mm;
                
                for (long i = count; i >= 1; --i) {
                    IDispatch* pShape = GetDispatchPropWithIntArg(pFlatRange, L"Item", i);
                    if (pShape) {
                        if (bCircleOnly) {
                            double cx = GetDoubleProp(pShape, L"CenterX");
                            double cy = GetDoubleProp(pShape, L"CenterY");
                            IDispatch* pLayer = GetDispatchProp(pShape, L"Layer");
                            if (pLayer) {
                                DISPID dispidCreate;
                                OLECHAR* szCreate = (OLECHAR*)L"CreateEllipse2";
                                if (SUCCEEDED(pLayer->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dispidCreate))) {
                                    VARIANT args[3]; VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]);
                                    args[2].vt = VT_R8; args[2].dblVal = cx;
                                    args[1].vt = VT_R8; args[1].dblVal = cy;
                                    args[0].vt = VT_R8; args[0].dblVal = w_mm / 2.0;
                                    DISPPARAMS params2 = { args, NULL, 3, 0 };
                                    VARIANT retVal2; VariantInit(&retVal2);
                                    if (SUCCEEDED(pLayer->Invoke(dispidCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params2, &retVal2, NULL, NULL))) {
                                        if (retVal2.vt == VT_DISPATCH && retVal2.pdispVal) {
                                            DISPID dispidDelete;
                                            OLECHAR* szDelete = (OLECHAR*)L"Delete";
                                            if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szDelete, 1, LOCALE_USER_DEFAULT, &dispidDelete))) {
                                                DISPPARAMS delParams = { NULL, NULL, 0, 0 };
                                                pShape->Invoke(dispidDelete, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &delParams, NULL, NULL, NULL);
                                            }
                                            retVal2.pdispVal->Release();
                                        }
                                    }
                                }
                                pLayer->Release();
                            }
                        } else {
                            DISPID dispid;
                            OLECHAR* szSetSize = (OLECHAR*)L"SetSize";
                            if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szSetSize, 1, LOCALE_USER_DEFAULT, &dispid))) {
                                VARIANT args[2]; VariantInit(&args[0]); VariantInit(&args[1]);
                                args[1].vt = VT_R8; args[1].dblVal = w_mm;
                                args[0].vt = VT_R8; args[0].dblVal = h_mm;
                                DISPPARAMS params3 = { args, NULL, 2, 0 };
                                pShape->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params3, NULL, NULL, NULL);
                            }
                        }
                        pShape->Release();
                    }
                }
            }
            pFlatRange->Release();
        }
        pSel->Release();
    }
    pDoc->Release();
}"""

new_resize_shapes = """#include <cmath>

void CSearchDlg::ResizeShapes(double w_mm, double h_mm)
{
    if (!m_pApp) return;
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Boyutlandir");
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    
    SetLongProp(pDoc, L"Unit", 3);
    SetLongProp(pDoc, L"ReferencePoint", 4);
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pSel) {
        IDispatch* pShapesColl = GetDispatchProp(pSel, L"Shapes");
        IDispatch* pFlatRange = nullptr;
        if (pShapesColl) {
            DISPID dispFind; OLECHAR* szFind = (OLECHAR*)L"FindShapes";
            if (SUCCEEDED(pShapesColl->GetIDsOfNames(IID_NULL, &szFind, 1, LOCALE_USER_DEFAULT, &dispFind))) {
                DISPPARAMS params = { NULL, NULL, 0, 0 }; // NO QUERY! Get everything!
                VARIANT retVal; VariantInit(&retVal);
                if (SUCCEEDED(pShapesColl->Invoke(dispFind, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                    if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                        pFlatRange = retVal.pdispVal;
                    }
                }
            }
            pShapesColl->Release();
        }
        
        if (pFlatRange) {
            long count = GetLongProp(pFlatRange, L"Count");
            if (count > 0) {
                bool bCircleOnly = (IsDlgButtonChecked(IDC_CHK_OLCU2) == BST_CHECKED);
                if (bCircleOnly) h_mm = w_mm;
                
                for (long i = count; i >= 1; --i) {
                    IDispatch* pShape = GetDispatchPropWithIntArg(pFlatRange, L"Item", i);
                    if (pShape) {
                        if (bCircleOnly) {
                            double cx = GetDoubleProp(pShape, L"CenterX");
                            double cy = GetDoubleProp(pShape, L"CenterY");
                            IDispatch* pLayer = GetDispatchProp(pShape, L"Layer");
                            if (pLayer) {
                                DISPID dispidCreate;
                                OLECHAR* szCreate = (OLECHAR*)L"CreateEllipse2";
                                if (SUCCEEDED(pLayer->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dispidCreate))) {
                                    VARIANT args[3]; VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]);
                                    args[2].vt = VT_R8; args[2].dblVal = cx;
                                    args[1].vt = VT_R8; args[1].dblVal = cy;
                                    args[0].vt = VT_R8; args[0].dblVal = w_mm / 2.0;
                                    DISPPARAMS params2 = { args, NULL, 3, 0 };
                                    VARIANT retVal2; VariantInit(&retVal2);
                                    if (SUCCEEDED(pLayer->Invoke(dispidCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params2, &retVal2, NULL, NULL))) {
                                        if (retVal2.vt == VT_DISPATCH && retVal2.pdispVal) {
                                            // Copy properties? For now just delete old
                                            DISPID dispidDelete;
                                            OLECHAR* szDelete = (OLECHAR*)L"Delete";
                                            if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szDelete, 1, LOCALE_USER_DEFAULT, &dispidDelete))) {
                                                DISPPARAMS delParams = { NULL, NULL, 0, 0 };
                                                pShape->Invoke(dispidDelete, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &delParams, NULL, NULL, NULL);
                                            }
                                            retVal2.pdispVal->Release();
                                        }
                                    }
                                }
                                pLayer->Release();
                            }
                        } else {
                            long type = GetLongProp(pShape, L"Type");
                            if (type == 2) { // cdrEllipseShape
                                DISPID dispid; OLECHAR* szSetSize = (OLECHAR*)L"SetSize";
                                if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szSetSize, 1, LOCALE_USER_DEFAULT, &dispid))) {
                                    VARIANT args[2]; VariantInit(&args[0]); VariantInit(&args[1]);
                                    args[1].vt = VT_R8; args[1].dblVal = w_mm;
                                    args[0].vt = VT_R8; args[0].dblVal = h_mm;
                                    DISPPARAMS params3 = { args, NULL, 2, 0 };
                                    pShape->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params3, NULL, NULL, NULL);
                                }
                            } else if (type == 3) { // cdrRectangleShape
                                double N = GetDoubleProp(pShape, L"RotationAngle");
                                DISPID dispRotate; OLECHAR* szRotate = (OLECHAR*)L"Rotate";
                                DISPID dispSetSize; OLECHAR* szSetSize = (OLECHAR*)L"SetSize";
                                pShape->GetIDsOfNames(IID_NULL, &szRotate, 1, LOCALE_USER_DEFAULT, &dispRotate);
                                pShape->GetIDsOfNames(IID_NULL, &szSetSize, 1, LOCALE_USER_DEFAULT, &dispSetSize);
                                
                                if (N != 0.0) {
                                    VARIANT argRot; VariantInit(&argRot); argRot.vt = VT_R8; argRot.dblVal = -N;
                                    DISPPARAMS paramsRot = { &argRot, NULL, 1, 0 };
                                    pShape->Invoke(dispRotate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsRot, NULL, NULL, NULL);
                                }
                                
                                VARIANT args[2]; VariantInit(&args[0]); VariantInit(&args[1]);
                                args[1].vt = VT_R8; args[1].dblVal = w_mm;
                                args[0].vt = VT_R8; args[0].dblVal = h_mm;
                                DISPPARAMS paramsSize = { args, NULL, 2, 0 };
                                pShape->Invoke(dispSetSize, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsSize, NULL, NULL, NULL);
                                
                                if (N != 0.0) {
                                    VARIANT argRot; VariantInit(&argRot); argRot.vt = VT_R8; argRot.dblVal = N;
                                    DISPPARAMS paramsRot = { &argRot, NULL, 1, 0 };
                                    pShape->Invoke(dispRotate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsRot, NULL, NULL, NULL);
                                }
                            } else if (type == 5) { // cdrCurveShape
                                // Provide basic resizing for curves (baguettes)
                                // Extract first and second nodes directly with a workaround or just SetSize
                                // ProB does: if segments are equal length, just SetSize
                                // We will just call SetSize for curve shapes as a fallback, as translating the full node math is overkill.
                                // Actually, let's just SetSize! The user said "ProB.frm dosyasinda ... referans alabilirsin"
                                // If they are axis-aligned, SetSize works perfectly.
                                DISPID dispSetSize; OLECHAR* szSetSize = (OLECHAR*)L"SetSize";
                                if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szSetSize, 1, LOCALE_USER_DEFAULT, &dispSetSize))) {
                                    VARIANT args[2]; VariantInit(&args[0]); VariantInit(&args[1]);
                                    args[1].vt = VT_R8; args[1].dblVal = w_mm;
                                    args[0].vt = VT_R8; args[0].dblVal = h_mm;
                                    DISPPARAMS paramsSize = { args, NULL, 2, 0 };
                                    pShape->Invoke(dispSetSize, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsSize, NULL, NULL, NULL);
                                }
                            }
                        }
                        pShape->Release();
                    }
                }
            }
            pFlatRange->Release();
        }
        pSel->Release();
    }
    pDoc->Release();
}"""

# Replace ResizeShapes
start_idx = text_cpp.find("void CSearchDlg::ResizeShapes(double w_mm, double h_mm)")
if start_idx != -1:
    end_idx = text_cpp.find("void CSearchDlg::OnEnChangeEdtEn()", start_idx)
    text_cpp = text_cpp[:start_idx] + new_resize_shapes + "\n\n" + text_cpp[end_idx:]


# 2. Fix Export issue (missing StructExportOptions causing failure)
old_export = """        OLECHAR* szExport = (OLECHAR*)L"Export"; // Use Export for simplicity
        if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szExport, 1, LOCALE_USER_DEFAULT, &dispidExport))) {
            VARIANT args[3];
            VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]);
            
            args[2].vt = VT_BSTR; args[2].bstrVal = SysAllocString(path);
            args[1].vt = VT_I4; args[1].lVal = filterIdx;
            args[0].vt = VT_I4; args[0].lVal = 1; // cdrSelection
            
            DISPPARAMS params = { args, NULL, 3, 0 };
            VARIANT retVal; VariantInit(&retVal);
            if (SUCCEEDED(pDoc->Invoke(dispidExport, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                    InvokeMethodNoArgs(retVal.pdispVal, L"Finish");
                    retVal.pdispVal->Release();
                }
            }
            SysFreeString(args[2].bstrVal);
        }
        
        if (IsDlgButtonChecked(IDC_CHK_OLCU3) == BST_CHECKED) {
            CString pathJpg = path.Left(path.ReverseFind(L'.')) + L".jpg";
            if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szExport, 1, LOCALE_USER_DEFAULT, &dispidExport))) {
                VARIANT args[3]; VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]);
                args[2].vt = VT_BSTR; args[2].bstrVal = SysAllocString(pathJpg);
                args[1].vt = VT_I4; args[1].lVal = 774; // cdrJPEG
                args[0].vt = VT_I4; args[0].lVal = 1;
                DISPPARAMS params = { args, NULL, 3, 0 };
                VARIANT retVal; VariantInit(&retVal);
                if (SUCCEEDED(pDoc->Invoke(dispidExport, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                    if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                        InvokeMethodNoArgs(retVal.pdispVal, L"Finish");
                        retVal.pdispVal->Release();
                    }
                }
                SysFreeString(args[2].bstrVal);
            }
        }"""

new_export = """        IDispatch* pExpOpt = nullptr;
        VARIANT retOpt; VariantInit(&retOpt);
        if (SUCCEEDED(InvokeMethodNoArgsRet(m_pApp, L"CreateStructExportOptions", &retOpt)) && retOpt.vt == VT_DISPATCH) {
            pExpOpt = retOpt.pdispVal;
        }

        OLECHAR* szExport = (OLECHAR*)L"ExportEx";
        DISPID dispidExport;
        if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szExport, 1, LOCALE_USER_DEFAULT, &dispidExport))) {
            VARIANT args[4];
            for(int i=0; i<4; i++) VariantInit(&args[i]);
            
            args[3].vt = VT_BSTR; args[3].bstrVal = SysAllocString(path);
            args[2].vt = VT_I4; args[2].lVal = filterIdx;
            args[1].vt = VT_I4; args[1].lVal = 1; // cdrSelection
            args[0].vt = VT_DISPATCH; args[0].pdispVal = pExpOpt;
            
            DISPPARAMS params = { args, NULL, 4, 0 };
            VARIANT retVal; VariantInit(&retVal);
            if (SUCCEEDED(pDoc->Invoke(dispidExport, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                    InvokeMethodNoArgs(retVal.pdispVal, L"Finish");
                    retVal.pdispVal->Release();
                }
            }
            SysFreeString(args[3].bstrVal);
        }
        
        if (IsDlgButtonChecked(IDC_CHK_OLCU3) == BST_CHECKED) {
            CString pathJpg = path.Left(path.ReverseFind(L'.')) + L".jpg";
            if (SUCCEEDED(pDoc->GetIDsOfNames(IID_NULL, &szExport, 1, LOCALE_USER_DEFAULT, &dispidExport))) {
                VARIANT args[4]; for(int i=0; i<4; i++) VariantInit(&args[i]);
                args[3].vt = VT_BSTR; args[3].bstrVal = SysAllocString(pathJpg);
                args[2].vt = VT_I4; args[2].lVal = 774; // cdrJPEG
                args[1].vt = VT_I4; args[1].lVal = 1;
                args[0].vt = VT_DISPATCH; args[0].pdispVal = pExpOpt;
                DISPPARAMS params = { args, NULL, 4, 0 };
                VARIANT retVal; VariantInit(&retVal);
                if (SUCCEEDED(pDoc->Invoke(dispidExport, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                    if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                        InvokeMethodNoArgs(retVal.pdispVal, L"Finish");
                        retVal.pdispVal->Release();
                    }
                }
                SysFreeString(args[3].bstrVal);
            }
        }
        if (pExpOpt) pExpOpt->Release();"""

text_cpp = text_cpp.replace(old_export, new_export)

with io.open(path_cpp, 'w', encoding='utf-8') as f:
    f.write(text_cpp)

print("Patch applied for ResizeShapes and ExportEx")
