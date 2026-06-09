import io

path_cpp = "SearchDlg.cpp"
content = io.open(path_cpp, "r", encoding="cp1254", errors="ignore").read()

# 1. InvokeMethodNoArgsRet ekleyelim (eğer henüz eklenmemişse)
if "static HRESULT InvokeMethodNoArgsRet" not in content:
    idx_log = content.find("static void LogDebug")
    if idx_log != -1:
        ret_func = """static HRESULT InvokeMethodNoArgsRet(IDispatch* pDisp, WCHAR* szMethod, VARIANT* pRetVal)
{
    if (!pDisp) return E_POINTER;
    DISPID dispid;
    HRESULT hr = pDisp->GetIDsOfNames(IID_NULL, &szMethod, 1, LOCALE_USER_DEFAULT, &dispid);
    if (FAILED(hr)) return hr;
    DISPPARAMS params = { NULL, NULL, 0, 0 };
    return pDisp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, pRetVal, NULL, NULL);
}

"""
        content = content[:idx_log] + ret_func + content[idx_log:]
        print("Inserted InvokeMethodNoArgsRet before LogDebug")
    else:
        print("Error: static void LogDebug not found!")

# 2. Şablonların yerini alacak yeni kodları uygulayalım
target_start = "void CSearchDlg::OnBnClickedBtnAra2() {"
idx = content.find(target_start)
if idx == -1:
    target_start = "void CSearchDlg::OnBnClickedBtnAra2()"
    idx = content.find(target_start)

if idx == -1:
    print("Error: OnBnClickedBtnAra2 not found!")
else:
    header = content[:idx]
    
    new_code = """void CSearchDlg::OnBnClickedBtnAra2()
{
    CString strEn, strBoy;
    GetDlgItemText(IDC_EDT_EN, strEn);
    GetDlgItemText(IDC_EDT_BOY, strBoy);
    double w = _wtof(strEn);
    double h = _wtof(strBoy);
    if (w <= 0 || h <= 0) {
        return;
    }
    ResizeShapes(w, h);
}

void CSearchDlg::OnBnClickedAra3Range(UINT nID)
{
    double sizes[] = {2.0, 2.1, 2.15, 3.0, 3.1, 3.15, 0.4, 0.6};
    int idx = nID - IDC_BTN_OLCU_1;
    if (idx >= 0 && idx < 8) {
        double s = sizes[idx];
        CString str;
        str.Format(L"%.2f", s);
        str.TrimRight(L"0");
        str.TrimRight(L".");
        SetDlgItemText(IDC_EDT_EN, str);
        SetDlgItemText(IDC_EDT_BOY, str);
        ResizeShapes(s, s);
    }
}

#include <cmath>

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
                DISPPARAMS params = { NULL, NULL, 0, 0 };
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
}

void CSearchDlg::OnBnClickedBtnAra5()
{
    if (!m_pApp) return;
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (!pSel) return;
    long count = GetLongProp(pSel, L"Count");
    if (count == 0) { pSel->Release(); return; }

    for (long i = 1; i <= count; i++) {
        VARIANT arg; VariantInit(&arg); arg.vt = VT_I4; arg.lVal = i;
        DISPPARAMS prms = { &arg, NULL, 1, 0 };
        VARIANT res; VariantInit(&res);
        if (SUCCEEDED(pSel->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &prms, &res, NULL, NULL)) && res.vt == VT_DISPATCH) {
            IDispatch* pShape = res.pdispVal;
            if (pShape) {
                IDispatch* pOutline = GetDispatchProp(pShape, L"Outline");
                IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
                if (pOutline && pFill) {
                    IDispatch* pOutlineColor = GetDispatchProp(pOutline, L"Color");
                    if (pOutlineColor) {
                        DISPID dApply; OLECHAR* szApply = (OLECHAR*)L"ApplyUniformFill";
                        if (SUCCEEDED(pFill->GetIDsOfNames(IID_NULL, &szApply, 1, LOCALE_USER_DEFAULT, &dApply))) {
                            VARIANT argF; VariantInit(&argF); argF.vt = VT_DISPATCH; argF.pdispVal = pOutlineColor;
                            DISPPARAMS params = { &argF, NULL, 1, 0 };
                            pFill->Invoke(dApply, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, NULL, NULL, NULL);
                        }
                        pOutlineColor->Release();
                    }
                    pFill->Release();
                    pOutline->Release();
                }
                pShape->Release();
            }
        }
    }
    pSel->Release();
}

void CSearchDlg::OnBnClickedBtnAra6()
{
    if (!m_pApp) return;
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (!pSel) return;
    long count = GetLongProp(pSel, L"Count");
    if (count == 0) { pSel->Release(); return; }

    for (long i = 1; i <= count; i++) {
        VARIANT arg; VariantInit(&arg); arg.vt = VT_I4; arg.lVal = i;
        DISPPARAMS prms = { &arg, NULL, 1, 0 };
        VARIANT res; VariantInit(&res);
        if (SUCCEEDED(pSel->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &prms, &res, NULL, NULL)) && res.vt == VT_DISPATCH) {
            IDispatch* pShape = res.pdispVal;
            if (pShape) {
                IDispatch* pOutline = GetDispatchProp(pShape, L"Outline");
                IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
                if (pOutline && pFill) {
                    long fillType = GetLongProp(pFill, L"Type");
                    if (fillType == 1) { // cdrUniformFill
                        IDispatch* pFillColor = GetDispatchProp(pFill, L"UniformColor");
                        if (pFillColor) {
                            IDispatch* pOutlineColor = GetDispatchProp(pOutline, L"Color");
                            if (pOutlineColor) {
                                DISPID dCopy; OLECHAR* szCopy = (OLECHAR*)L"CopyAssign";
                                if (SUCCEEDED(pOutlineColor->GetIDsOfNames(IID_NULL, &szCopy, 1, LOCALE_USER_DEFAULT, &dCopy))) {
                                    VARIANT argC; VariantInit(&argC); argC.vt = VT_DISPATCH; argC.pdispVal = pFillColor;
                                    DISPPARAMS prmsC = { &argC, NULL, 1, 0 };
                                    pOutlineColor->Invoke(dCopy, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsC, NULL, NULL, NULL);
                                }
                                pOutlineColor->Release();
                            }
                            
                            long outType = GetLongProp(pOutline, L"Type");
                            if (outType == 0) {
                                SetDoubleProp(pOutline, L"Width", 0.5);
                            }
                            pFillColor->Release();
                        }
                    }
                    pFill->Release();
                    pOutline->Release();
                }
                pShape->Release();
            }
        }
    }
    pSel->Release();
}

void CSearchDlg::OnBnClickedBtnAra7()
{
    if (!m_pApp) return;
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (!pSel) return;
    long count = GetLongProp(pSel, L"Count");
    if (count < 1) { pSel->Release(); return; }

    IDispatch* pLine = NULL;
    for (long i = 1; i <= count; i++) {
        VARIANT arg; VariantInit(&arg); arg.vt = VT_I4; arg.lVal = i;
        DISPPARAMS prms = { &arg, NULL, 1, 0 };
        VARIANT res; VariantInit(&res);
        if (SUCCEEDED(pSel->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &prms, &res, NULL, NULL)) && res.vt == VT_DISPATCH) {
            IDispatch* pShape = res.pdispVal;
            IDispatch* pCurve = GetDispatchProp(pShape, L"Curve");
            if (pCurve) {
                IDispatch* pNodes = GetDispatchProp(pCurve, L"Nodes");
                if (pNodes) {
                    long nodeCount = GetLongProp(pNodes, L"Count");
                    if (nodeCount == 2) {
                        pLine = pShape;
                        pLine->AddRef();
                        pNodes->Release();
                        pCurve->Release();
                        break;
                    }
                    pNodes->Release();
                }
                pCurve->Release();
            }
            pShape->Release();
        }
    }

    if (!pLine) { pSel->Release(); return; }

    double rotationAngle = 0.0;
    IDispatch* pCurve = GetDispatchProp(pLine, L"Curve");
    if (pCurve) {
        IDispatch* pNodes = GetDispatchProp(pCurve, L"Nodes");
        if (pNodes) {
            IDispatch* pNode1 = GetDispatchPropWithIntArg(pNodes, L"Item", 1);
            IDispatch* pNode2 = GetDispatchPropWithIntArg(pNodes, L"Item", 2);
            if (pNode1 && pNode2) {
                double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
                DISPID dGetPos; OLECHAR* szGetPos = (OLECHAR*)L"GetPosition";
                if (SUCCEEDED(pNode1->GetIDsOfNames(IID_NULL, &szGetPos, 1, LOCALE_USER_DEFAULT, &dGetPos))) {
                    VARIANT args1[2]; VariantInit(&args1[0]); VariantInit(&args1[1]);
                    args1[1].vt = VT_BYREF | VT_R8; args1[1].pdblVal = &x1;
                    args1[0].vt = VT_BYREF | VT_R8; args1[0].pdblVal = &y1;
                    DISPPARAMS params1 = { args1, NULL, 2, 0 };
                    pNode1->Invoke(dGetPos, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params1, NULL, NULL, NULL);

                    VARIANT args2[2]; VariantInit(&args2[0]); VariantInit(&args2[1]);
                    args2[1].vt = VT_BYREF | VT_R8; args2[1].pdblVal = &x2;
                    args2[0].vt = VT_BYREF | VT_R8; args2[0].pdblVal = &y2;
                    DISPPARAMS params2 = { args2, NULL, 2, 0 };
                    pNode2->Invoke(dGetPos, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params2, NULL, NULL, NULL);
                }
                pNode1->Release();
                pNode2->Release();

                double dx = x2 - x1;
                double dy = y2 - y1;
                double angle = atan2(dy, dx) * 180.0 / 3.141592653589793;
                double normAngle = angle;
                if (fabs(normAngle) <= 45.0) {
                    rotationAngle = -normAngle;
                } else {
                    if (normAngle > 0)
                        rotationAngle = -(normAngle - 90.0);
                    else
                        rotationAngle = -(normAngle + 90.0);
                }
            }
            pNodes->Release();
        }
        pCurve->Release();
    }

    if (fabs(rotationAngle) < 0.001) {
        pLine->Release(); pSel->Release();
        return;
    }

    OptimizeCorel(m_pApp, TRUE, L"Açı Düzelt");

    VARIANT argRot; VariantInit(&argRot);
    argRot.vt = VT_R8; argRot.dblVal = rotationAngle;

    for (long i = 1; i <= count; i++) {
        VARIANT arg; VariantInit(&arg); arg.vt = VT_I4; arg.lVal = i;
        DISPPARAMS prms = { &arg, NULL, 1, 0 };
        VARIANT res; VariantInit(&res);
        if (SUCCEEDED(pSel->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &prms, &res, NULL, NULL)) && res.vt == VT_DISPATCH) {
            IDispatch* pShape = res.pdispVal;
            DISPID dRotate; OLECHAR* szRotate = (OLECHAR*)L"Rotate";
            if (SUCCEEDED(pShape->GetIDsOfNames(IID_NULL, &szRotate, 1, LOCALE_USER_DEFAULT, &dRotate))) {
                DISPPARAMS prmsR = { &argRot, NULL, 1, 0 };
                pShape->Invoke(dRotate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsR, NULL, NULL, NULL);
            }
            pShape->Release();
        }
    }

    OptimizeCorel(m_pApp, FALSE, L"Açı Düzelt");
    pLine->Release();
    pSel->Release();
}

void CSearchDlg::OnEnChangeEdtEn()
{
}

void CSearchDlg::OnPaint()
{
    CPaintDC dc(this);
}

void CSearchDlg::OnBnClickedBtnList()
{
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Liste");
    SetRedraw(FALSE);
    UpdateData(TRUE);

    if (m_bListOpen) {
        m_bListOpen = FALSE;
        ClearDynamicCombos();
        m_bListOption = FALSE;
        UpdateData(FALSE);
        CheckDlgButton(IDC_CHK_LIST_OPTION, BST_UNCHECKED);
        CWnd* pGroup = GetDlgItem(IDC_GRP_LISTE);
        if (pGroup) pGroup->ShowWindow(SW_HIDE);
        m_listObjects.clear();
    }
    else {
        m_bListOpen = TRUE;
        m_bListOption = TRUE;
        CheckDlgButton(IDC_CHK_LIST_OPTION, BST_CHECKED);

        IDispatch* pSelection = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
        if (pSelection) {
            long count = GetLongProp(pSelection, L"Count");
            
            struct GroupedStone {
                COLORREF color;
                double width;
                int count;
            };
            std::vector<GroupedStone> groups;
            
            for (long i = 1; i <= count; ++i) {
                IDispatch* pShape = GetDispatchPropWithIntArg(pSelection, L"Item", i);
                if (pShape) {
                    COLORREF col = RGB(128, 128, 128);
                    IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
                    long fillType = 0;
                    if (pFill) {
                        fillType = GetLongProp(pFill, L"Type");
                        if (fillType == 1) {
                            IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                            if (pColor) { col = GetColorRef(pColor); pColor->Release(); }
                        }
                        pFill->Release();
                    }
                    if (fillType != 1) {
                        IDispatch* pOutline = GetDispatchProp(pShape, L"Outline");
                        if (pOutline) {
                            IDispatch* pColor = GetDispatchProp(pOutline, L"Color");
                            if (pColor) { col = GetColorRef(pColor); pColor->Release(); }
                            pOutline->Release();
                        }
                    }
                    
                    double w = GetDoubleProp(pShape, L"SizeWidth");
                    
                    bool found = false;
                    for (auto& g : groups) {
                        if (g.color == col && fabs(g.width - w) < 0.05) {
                            g.count++;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        GroupedStone gs;
                        gs.color = col;
                        gs.width = w;
                        gs.count = 1;
                        groups.push_back(gs);
                    }
                    
                    pShape->Release();
                }
            }
            
            m_listObjects.clear();
            std::vector<COLORREF> uniqueColors;
            for (const auto& g : groups) {
                SelectedObjectInfo info;
                info.color = g.color;
                CString objText;
                objText.Format(_T("%.1fmm-%d"), g.width, g.count);
                objText.Replace(_T("."), _T(","));
                info.text = objText;
                m_listObjects.push_back(info);
                
                if (std::find(uniqueColors.begin(), uniqueColors.end(), g.color) == uniqueColors.end()) {
                    uniqueColors.push_back(g.color);
                }
            }
            pSelection->Release();
        }

        UpdateListLayout();
        
        ClearDynamicCombos();
        CWnd* pGroup2 = GetDlgItem(IDC_GRP_LISTE);
        if (pGroup2 && !m_listObjects.empty()) {
            CRect rcGroup;
            pGroup2->GetWindowRect(&rcGroup);
            ScreenToClient(&rcGroup);
            int startY = rcGroup.top + 14;
            int rowH = 24;
            
            DWORD dwStyle = WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL;
            if (m_bListOption) dwStyle |= WS_VISIBLE;
            
            for (size_t i = 0; i < m_listObjects.size(); ++i) {
                int rowY = startY + (int)i * rowH;
                CRect rcTas(rcGroup.left + 93, rowY, rcGroup.left + 203, rowY + 100);
                CRect rcTip(rcGroup.left + 208, rowY, rcGroup.left + 248, rowY + 80);
                
                CComboBox* pTas = new CComboBox();
                pTas->Create(dwStyle, rcTas, this, 2000 + (int)i);
                pTas->SetFont(GetFont());
                pTas->AddString(_T("Black Diamond"));
                pTas->AddString(_T("Hyacinth"));
                pTas->AddString(_T("Crystal"));
                pTas->AddString(_T("Siam"));
                pTas->SetCurSel(i % 2);
                m_combosTas.push_back(pTas);
                
                CComboBox* pTip = new CComboBox();
                pTip->Create(dwStyle, rcTip, this, 3000 + (int)i);
                pTip->SetFont(GetFont());
                pTip->AddString(_T("Dbl"));
                pTip->AddString(_T("Sgl"));
                pTip->SetCurSel(0);
                m_combosTip.push_back(pTip);
            }
        }
        if (pGroup2) pGroup2->ShowWindow(SW_SHOW);
    }
    SetRedraw(TRUE);
    Invalidate();
    UpdateWindow();
}

void CSearchDlg::OnBnClickedChkListOption()
{
    SetRedraw(FALSE);
    UpdateData(TRUE);
    
    int nShow = m_bListOption ? SW_SHOW : SW_HIDE;
    for (auto p : m_combosTas) if (::IsWindow(p->GetSafeHwnd())) p->ShowWindow(nShow);
    for (auto p : m_combosTip) if (::IsWindow(p->GetSafeHwnd())) p->ShowWindow(nShow);
    
    if (m_bListOpen && !m_listObjects.empty()) {
        UpdateListLayout();
    }
    SetRedraw(TRUE);
    Invalidate();
    UpdateWindow();
}

void CSearchDlg::OnBnClickedSimge()
{
    SetCompactMode(!m_bCompactMode);
}

void CSearchDlg::SetCompactMode(BOOL bCompact)
{
    m_bCompactMode = bCompact;
    if (bCompact) {
        m_vecVisibleControls.clear();
        CWnd* pWnd = GetWindow(GW_CHILD);
        while (pWnd) {
            if (pWnd->IsWindowVisible()) {
                m_vecVisibleControls.push_back(pWnd->GetSafeHwnd());
                int id = pWnd->GetDlgCtrlID();
                if (id != IDC_BTN_simge && id != 3000 && id != 10001) {
                    pWnd->ShowWindow(SW_HIDE);
                }
            }
            pWnd = pWnd->GetWindow(GW_HWNDNEXT);
        }
        m_wndAdBox.ShowWindow(SW_SHOW);
        SetDialogHeight(32);
        GetDlgItem(IDC_BTN_simge)->SetWindowText(_T("+"));
    } else {
        m_wndAdBox.ShowWindow(SW_HIDE);
        for (HWND hWnd : m_vecVisibleControls) {
            if (::IsWindow(hWnd)) {
                ::ShowWindow(hWnd, SW_SHOW);
            }
        }
        m_vecVisibleControls.clear();
        if (m_bListOpen) {
            UpdateListLayout();
        } else {
            SetDialogHeight(198);
        }
        GetDlgItem(IDC_BTN_simge)->SetWindowText(_T("-"));
    }
    Invalidate();
}

void CSearchDlg::OnBnClickedChkKalip()
{
    UpdateData(TRUE);
    ShowKalipPanel(m_bKalip);
}

void CSearchDlg::OnBnClickedChkZtrass()
{
    UpdateData(TRUE);
}

void CSearchDlg::OnBnClickedBtnBilgiEkle()
{
    if (!m_pApp) return;
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Bilgi Ekle");
    
    CString sFirma, sDosya, sKalip, sPres, sMontaj, sKalipIs, sToplamIs;
    GetDlgItemText(IDC_EDT_FIRMA, sFirma);
    GetDlgItemText(IDC_EDT_DOSYA, sDosya);
    GetDlgItemText(IDC_EDT_KALIP_ADI, sKalip);
    GetDlgItemText(IDC_EDT_PRES, sPres);
    GetDlgItemText(IDC_EDT_MONTAJ, sMontaj);
    GetDlgItemText(IDC_EDT_KALIPTA_IS, sKalipIs);
    GetDlgItemText(IDC_EDT_TOPLAM_IS, sToplamIs);
    
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    SetLongProp(pDoc, L"Unit", 3); // mm
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pSel) {
        long count = GetLongProp(pSel, L"Count");
        if (count > 0) {
            double left = GetDoubleProp(pSel, L"LeftX");
            double top = GetDoubleProp(pSel, L"TopY");
            
            IDispatch* pLayer = GetDispatchProp(pDoc, L"ActiveLayer");
            if (pLayer) {
                DISPID dispCreate; OLECHAR* szCreate = (OLECHAR*)L"CreateArtisticText";
                if (SUCCEEDED(pLayer->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dispCreate))) {
                    double curY = top + 10.0;
                    auto AddLine = [&](const CString& txt, COLORREF col) {
                        VARIANT args[3]; VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]);
                        args[2].vt = VT_R8; args[2].dblVal = left;
                        args[1].vt = VT_R8; args[1].dblVal = curY;
                        args[0].vt = VT_BSTR; args[0].bstrVal = SysAllocString(txt);
                        
                        DISPPARAMS params = { args, NULL, 3, 0 };
                        VARIANT retVal; VariantInit(&retVal);
                        if (SUCCEEDED(pLayer->Invoke(dispCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                            if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                                IDispatch* pTextShape = retVal.pdispVal;
                                if (col != CLR_INVALID) {
                                    IDispatch* pFill = GetDispatchProp(pTextShape, L"Fill");
                                    if (pFill) {
                                        IDispatch* pColor = nullptr;
                                        VARIANT rCol; VariantInit(&rCol);
                                        if (SUCCEEDED(InvokeMethodNoArgsRet(m_pApp, L"CreateColor", &rCol)) && rCol.vt == VT_DISPATCH) {
                                            pColor = rCol.pdispVal;
                                            SetColorRGB(pColor, GetRValue(col), GetGValue(col), GetBValue(col));
                                            DISPID dApply; OLECHAR* szApply = (OLECHAR*)L"ApplyUniformFill";
                                            if (SUCCEEDED(pFill->GetIDsOfNames(IID_NULL, &szApply, 1, LOCALE_USER_DEFAULT, &dApply))) {
                                                VARIANT argC; VariantInit(&argC); argC.vt = VT_DISPATCH; argC.pdispVal = pColor;
                                                DISPPARAMS prmsC = { &argC, NULL, 1, 0 };
                                                pFill->Invoke(dApply, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsC, NULL, NULL, NULL);
                                            }
                                            pColor->Release();
                                        }
                                        pFill->Release();
                                    }
                                }
                                pTextShape->Release();
                            }
                        }
                        SysFreeString(args[0].bstrVal);
                        curY -= 6.0;
                    };

                    AddLine(_T("ztrass.com"), CLR_INVALID);
                    if (!sFirma.IsEmpty()) AddLine(_T("Firma: ") + sFirma, CLR_INVALID);
                    if (!sDosya.IsEmpty()) AddLine(_T("Dosya: ") + sDosya, CLR_INVALID);
                    if (!sKalip.IsEmpty()) AddLine(_T("Kalıp Adı: ") + sKalip, CLR_INVALID);
                    if (!sPres.IsEmpty()) AddLine(_T("Pres: ") + sPres, CLR_INVALID);
                    if (!sMontaj.IsEmpty()) AddLine(_T("Montaj: ") + sMontaj, CLR_INVALID);
                    if (!sKalipIs.IsEmpty()) AddLine(_T("Kalıptaki İş: ") + sKalipIs, CLR_INVALID);
                    if (!sToplamIs.IsEmpty()) AddLine(_T("Toplam İş: ") + sToplamIs, CLR_INVALID);
                    
                    if (m_bListOption && !m_listObjects.empty()) {
                        std::vector<COLORREF> uniqueClrs;
                        for (size_t i = 0; i < m_listObjects.size(); i++) {
                            COLORREF c = m_listObjects[i].color;
                            if (std::find(uniqueClrs.begin(), uniqueClrs.end(), c) == uniqueClrs.end())
                                uniqueClrs.push_back(c);
                        }
                        CString header;
                        header.Format(_T("%d Renk"), (int)uniqueClrs.size());
                        AddLine(header, CLR_INVALID);
                        
                        for (size_t i = 0; i < m_listObjects.size(); i++) {
                            CString tasAdi, tipAdi;
                            if (i < m_combosTas.size() && ::IsWindow(m_combosTas[i]->GetSafeHwnd())) {
                                m_combosTas[i]->GetWindowText(tasAdi);
                            }
                            if (i < m_combosTip.size() && ::IsWindow(m_combosTip[i]->GetSafeHwnd())) {
                                m_combosTip[i]->GetWindowText(tipAdi);
                            }
                            CString lineStr = tasAdi + _T(" - ") + tipAdi + _T(" - ") + m_listObjects[i].text;
                            AddLine(lineStr, m_listObjects[i].color);
                        }
                    }
                }
                pLayer->Release();
            }
        }
        pSel->Release();
    }
    pDoc->Release();
}

void CSearchDlg::OnBnClickedBtnKalipEkle()
{
    if (!m_pApp) return;
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Isaret Tasi");
    
    CString strTasMM;
    GetDlgItemText(IDC_EDT_KALIP_OLCU, strTasMM);
    strTasMM.Replace(L",", L".");
    double tasMM = _wtof(strTasMM);
    if (tasMM <= 0) tasMM = 2.0;
    
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    SetLongProp(pDoc, L"Unit", 3);
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pSel) {
        long count = GetLongProp(pSel, L"Count");
        if (count > 0) {
            double left = GetDoubleProp(pSel, L"LeftX");
            double right = GetDoubleProp(pSel, L"RightX");
            double bottom = GetDoubleProp(pSel, L"BottomY");
            double top = GetDoubleProp(pSel, L"TopY");
            
            double minW = 9999.0;
            IDispatch* pPage = GetDispatchProp(pDoc, L"ActivePage");
            if (pPage) {
                DISPID dispSelect; OLECHAR* szSelect = (OLECHAR*)L"SelectShapesFromRectangle";
                if (SUCCEEDED(pPage->GetIDsOfNames(IID_NULL, &szSelect, 1, LOCALE_USER_DEFAULT, &dispSelect))) {
                    VARIANT args[5]; for(int i=0; i<5; ++i) VariantInit(&args[i]);
                    args[4].vt = VT_R8; args[4].dblVal = left - 5.0;
                    args[3].vt = VT_R8; args[3].dblVal = top + 5.0;
                    args[2].vt = VT_R8; args[2].dblVal = right + 5.0;
                    args[1].vt = VT_R8; args[1].dblVal = bottom - 5.0;
                    args[0].vt = VT_BOOL; args[0].boolVal = VARIANT_TRUE;
                    
                    DISPPARAMS params = { args, NULL, 5, 0 };
                    VARIANT retSel; VariantInit(&retSel);
                    if (SUCCEEDED(pPage->Invoke(dispSelect, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retSel, NULL, NULL))) {
                        IDispatch* pNewSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
                        if (pNewSel) {
                            IDispatch* pShapes = GetDispatchProp(pNewSel, L"Shapes");
                            if (pShapes) {
                                DISPID dFind; OLECHAR* szFind = (OLECHAR*)L"FindShapes";
                                if (SUCCEEDED(pShapes->GetIDsOfNames(IID_NULL, &szFind, 1, LOCALE_USER_DEFAULT, &dFind))) {
                                    DISPPARAMS paramsFind = { NULL, NULL, 0, 0 };
                                    VARIANT retFind; VariantInit(&retFind);
                                    if (SUCCEEDED(pShapes->Invoke(dFind, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsFind, &retFind, NULL, NULL))) {
                                        if (retFind.vt == VT_DISPATCH && retFind.pdispVal) {
                                            IDispatch* pFlat = retFind.pdispVal;
                                            long flatCount = GetLongProp(pFlat, L"Count");
                                            for (long i = 1; i <= flatCount; i++) {
                                                IDispatch* pShape = GetDispatchPropWithIntArg(pFlat, L"Item", i);
                                                if (pShape) {
                                                    long type = GetLongProp(pShape, L"Type");
                                                    if (type == 2) {
                                                        double w = GetDoubleProp(pShape, L"SizeWidth");
                                                        if (w < minW) minW = w;
                                                    }
                                                    pShape->Release();
                                                }
                                            }
                                            pFlat->Release();
                                        }
                                    }
                                }
                                pShapes->Release();
                            }
                            pNewSel->Release();
                        }
                    }
                }
                pPage->Release();
            }
            
            if (minW > 100.0) minW = tasMM;
            
            IDispatch* pActiveLayer = GetDispatchProp(pDoc, L"ActiveLayer");
            if (pActiveLayer) {
                DISPID dCreate; OLECHAR* szCreate = (OLECHAR*)L"CreateEllipse2";
                if (SUCCEEDED(pActiveLayer->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dCreate))) {
                    double cx = left - 2.5;
                    double cy = top + 2.5;
                    double radius = minW / 2.0;
                    
                    VARIANT args[3]; VariantInit(&args[0]); VariantInit(&args[1]); VariantInit(&args[2]);
                    args[2].vt = VT_R8; args[2].dblVal = cx;
                    args[1].vt = VT_R8; args[1].dblVal = cy;
                    args[0].vt = VT_R8; args[0].dblVal = radius;
                    
                    DISPPARAMS params = { args, NULL, 3, 0 };
                    VARIANT retVal; VariantInit(&retVal);
                    if (SUCCEEDED(pActiveLayer->Invoke(dCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                        if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                            IDispatch* pNewShape = retVal.pdispVal;
                            
                            IDispatch* pFill = GetDispatchProp(pNewShape, L"Fill");
                            if (pFill) {
                                IDispatch* pColor = nullptr;
                                VARIANT rCol; VariantInit(&rCol);
                                if (SUCCEEDED(InvokeMethodNoArgsRet(m_pApp, L"CreateColor", &rCol)) && rCol.vt == VT_DISPATCH) {
                                    pColor = rCol.pdispVal;
                                    SetColorRGB(pColor, 0, 255, 0);
                                    
                                    DISPID dApply; OLECHAR* szApply = (OLECHAR*)L"ApplyUniformFill";
                                    if (SUCCEEDED(pFill->GetIDsOfNames(IID_NULL, &szApply, 1, LOCALE_USER_DEFAULT, &dApply))) {
                                        VARIANT argC; VariantInit(&argC); argC.vt = VT_DISPATCH; argC.pdispVal = pColor;
                                        DISPPARAMS prmsC = { &argC, NULL, 1, 0 };
                                        pFill->Invoke(dApply, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsC, NULL, NULL, NULL);
                                    }
                                    pColor->Release();
                                }
                                pFill->Release();
                            }
                            
                            IDispatch* pOutline = GetDispatchProp(pNewShape, L"Outline");
                            if (pOutline) {
                                SetDoubleProp(pOutline, L"Width", 0.0);
                                pOutline->Release();
                            }
                            
                            pNewShape->Release();
                        }
                    }
                }
                pActiveLayer->Release();
            }
        }
        pSel->Release();
    }
    pDoc->Release();
}

void CSearchDlg::OnBnClickedBtnIsaretTasi()
{
    if (!m_pApp) return;
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Kalip Ekle");
    
    CString strPadding;
    GetDlgItemText(IDC_EDT_TASI_OLCU, strPadding);
    strPadding.Replace(L",", L".");
    double padding = _wtof(strPadding);
    if (padding <= 0) padding = 10.0;
    
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    SetLongProp(pDoc, L"Unit", 3);
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pSel) {
        long count = GetLongProp(pSel, L"Count");
        if (count > 0) {
            double left = GetDoubleProp(pSel, L"LeftX");
            double right = GetDoubleProp(pSel, L"RightX");
            double bottom = GetDoubleProp(pSel, L"BottomY");
            double top = GetDoubleProp(pSel, L"TopY");
            
            IDispatch* pActiveLayer = GetDispatchProp(pDoc, L"ActiveLayer");
            if (pActiveLayer) {
                DISPID dCreate; OLECHAR* szCreate = (OLECHAR*)L"CreateRectangle";
                if (SUCCEEDED(pActiveLayer->GetIDsOfNames(IID_NULL, &szCreate, 1, LOCALE_USER_DEFAULT, &dCreate))) {
                    double x1 = left - padding;
                    double y1 = top + padding;
                    double x2 = right + padding;
                    double y2 = bottom - padding;
                    
                    VARIANT args[4]; for(int i=0; i<4; i++) VariantInit(&args[i]);
                    args[3].vt = VT_R8; args[3].dblVal = x1;
                    args[2].vt = VT_R8; args[2].dblVal = y1;
                    args[1].vt = VT_R8; args[1].dblVal = x2;
                    args[0].vt = VT_R8; args[0].dblVal = y2;
                    
                    DISPPARAMS params = { args, NULL, 4, 0 };
                    VARIANT retVal; VariantInit(&retVal);
                    if (SUCCEEDED(pActiveLayer->Invoke(dCreate, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
                        if (retVal.vt == VT_DISPATCH && retVal.pdispVal) {
                            IDispatch* pRect = retVal.pdispVal;
                            
                            IDispatch* pOutline = GetDispatchProp(pRect, L"Outline");
                            if (pOutline) {
                                SetDoubleProp(pOutline, L"Width", 0.5);
                                IDispatch* pColor = nullptr;
                                VARIANT rCol; VariantInit(&rCol);
                                if (SUCCEEDED(InvokeMethodNoArgsRet(m_pApp, L"CreateColor", &rCol)) && rCol.vt == VT_DISPATCH) {
                                    pColor = rCol.pdispVal;
                                    SetColorRGB(pColor, 0, 0, 0);
                                    
                                    DISPID dCopy; OLECHAR* szCopy = (OLECHAR*)L"CopyAssign";
                                    IDispatch* pOutColor = GetDispatchProp(pOutline, L"Color");
                                    if (pOutColor && SUCCEEDED(pOutColor->GetIDsOfNames(IID_NULL, &szCopy, 1, LOCALE_USER_DEFAULT, &dCopy))) {
                                        VARIANT argC; VariantInit(&argC); argC.vt = VT_DISPATCH; argC.pdispVal = pColor;
                                        DISPPARAMS prmsC = { &argC, NULL, 1, 0 };
                                        pOutColor->Invoke(dCopy, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prmsC, NULL, NULL, NULL);
                                    }
                                    if (pOutColor) pOutColor->Release();
                                    pColor->Release();
                                }
                                pOutline->Release();
                            }
                            pRect->Release();
                        }
                    }
                }
                pActiveLayer->Release();
            }
        }
        pSel->Release();
    }
    pDoc->Release();
}

void CSearchDlg::OnBnClickedBtnKalipAyir()
{
    if (!m_pApp) return;
    
    IDispatch* pTempSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pTempSel) {
        long c = GetLongProp(pTempSel, L"Count");
        pTempSel->Release();
        if (c == 1) OnBnClickedBtnList();
    }
    
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Kalip Ayir");
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    
    SetLongProp(pDoc, L"Unit", 3);
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (!pSel) { pDoc->Release(); return; }
    
    double kalipW = GetDoubleProp(pSel, L"SizeWidth");
    
    IDispatch* pFlatRange = nullptr;
    IDispatch* pShapes = GetDispatchProp(pSel, L"Shapes");
    DISPID dispFind; OLECHAR* szFind = (OLECHAR*)L"FindShapes";
    if (pShapes && SUCCEEDED(pShapes->GetIDsOfNames(IID_NULL, &szFind, 1, LOCALE_USER_DEFAULT, &dispFind))) {
        DISPPARAMS params = { NULL, NULL, 0, 0 };
        VARIANT retVal; VariantInit(&retVal);
        if (SUCCEEDED(pShapes->Invoke(dispFind, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
            if (retVal.vt == VT_DISPATCH) pFlatRange = retVal.pdispVal;
        }
    }
    if (pShapes) pShapes->Release();
    if (!pFlatRange) { pSel->Release(); pDoc->Release(); return; }
    long flatCount = GetLongProp(pFlatRange, L"Count");
    
    std::vector<IDispatch*> commonShapes;
    std::vector<IDispatch*> textShapes;
    std::map<COLORREF, std::vector<IDispatch*>> colorGroups;
    
    for (long i = 1; i <= flatCount; i++) {
        IDispatch* pShape = GetDispatchPropWithIntArg(pFlatRange, L"Item", i);
        if (!pShape) continue;
        
        long type = GetLongProp(pShape, L"Type");
        if (type == 8) {
            textShapes.push_back(pShape);
            continue;
        }
        
        bool isBlackOutline = false;
        bool isBlackFill = false;
        bool isNoFill = false;
        
        IDispatch* pOutline = GetDispatchProp(pShape, L"Outline");
        if (pOutline) {
            long outType = GetLongProp(pOutline, L"Type");
            if (outType != 0) {
                IDispatch* pColor = GetDispatchProp(pOutline, L"Color");
                if (pColor) {
                    COLORREF cr = GetColorRef(pColor);
                    if (cr == RGB(0,0,0)) isBlackOutline = true;
                    pColor->Release();
                }
            }
            pOutline->Release();
        }
        
        IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
        if (pFill) {
            long fillType = GetLongProp(pFill, L"Type");
            if (fillType == 0) {
                isNoFill = true;
            } else if (fillType == 1) {
                IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                if (pColor) {
                    COLORREF cr = GetColorRef(pColor);
                    if (cr == RGB(0,0,0)) isBlackFill = true;
                    pColor->Release();
                }
            }
            pFill->Release();
        }
        
        bool isCommon = (isBlackOutline && (isBlackFill || isNoFill)) || (!isBlackOutline && isBlackFill);
        
        if (isCommon) {
            commonShapes.push_back(pShape);
        } else {
            COLORREF col = RGB(128, 128, 128);
            IDispatch* pFill2 = GetDispatchProp(pShape, L"Fill");
            if (pFill2) {
                long ft = GetLongProp(pFill2, L"Type");
                if (ft == 1) {
                    IDispatch* pColor2 = GetDispatchProp(pFill2, L"UniformColor");
                    if (pColor2) { col = GetColorRef(pColor2); pColor2->Release(); }
                }
                pFill2->Release();
            }
            colorGroups[col].push_back(pShape);
        }
    }
    
    double xOffset = kalipW + 10.0;
    int index = 1;
    for (auto const& it : colorGroups) {
        COLORREF col = it.first;
        const auto& shapes = it.second;
        double currentOffset = xOffset * index;
        
        for (auto s : commonShapes) {
            VARIANT args[2]; VariantInit(&args[0]); VariantInit(&args[1]);
            args[1].vt = VT_R8; args[1].dblVal = currentOffset;
            args[0].vt = VT_R8; args[0].dblVal = 0.0;
            DISPPARAMS prms = { args, NULL, 2, 0 };
            VARIANT retDup; VariantInit(&retDup);
            DISPID dDup; OLECHAR* szDup = (OLECHAR*)L"Duplicate";
            if (SUCCEEDED(s->GetIDsOfNames(IID_NULL, &szDup, 1, LOCALE_USER_DEFAULT, &dDup))) {
                s->Invoke(dDup, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prms, &retDup, NULL, NULL);
                if (retDup.vt == VT_DISPATCH && retDup.pdispVal) {
                    retDup.pdispVal->Release();
                }
            }
        }
        
        for (auto s : shapes) {
            VARIANT args[2]; VariantInit(&args[0]); VariantInit(&args[1]);
            args[1].vt = VT_R8; args[1].dblVal = currentOffset;
            args[0].vt = VT_R8; args[0].dblVal = 0.0;
            DISPPARAMS prms = { args, NULL, 2, 0 };
            VARIANT retDup; VariantInit(&retDup);
            DISPID dDup; OLECHAR* szDup = (OLECHAR*)L"Duplicate";
            if (SUCCEEDED(s->GetIDsOfNames(IID_NULL, &szDup, 1, LOCALE_USER_DEFAULT, &dDup))) {
                s->Invoke(dDup, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prms, &retDup, NULL, NULL);
                if (retDup.vt == VT_DISPATCH && retDup.pdispVal) {
                    retDup.pdispVal->Release();
                }
            }
        }
        
        for (auto s : textShapes) {
            VARIANT args[2]; VariantInit(&args[0]); VariantInit(&args[1]);
            args[1].vt = VT_R8; args[1].dblVal = currentOffset;
            args[0].vt = VT_R8; args[0].dblVal = 0.0;
            DISPPARAMS prms = { args, NULL, 2, 0 };
            VARIANT retDup; VariantInit(&retDup);
            DISPID dDup; OLECHAR* szDup = (OLECHAR*)L"Duplicate";
            if (SUCCEEDED(s->GetIDsOfNames(IID_NULL, &szDup, 1, LOCALE_USER_DEFAULT, &dDup))) {
                if (SUCCEEDED(s->Invoke(dDup, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &prms, &retDup, NULL, NULL))) {
                    if (retDup.vt == VT_DISPATCH && retDup.pdispVal) {
                        IDispatch* pDupText = retDup.pdispVal;
                        pDupText->Release();
                    }
                }
            }
        }
        index++;
    }
    
    for (auto s : commonShapes) s->Release();
    for (auto s : textShapes) s->Release();
    for (auto const& it : colorGroups) {
        for (auto s : it.second) s->Release();
    }
    pFlatRange->Release();
    pSel->Release();
    pDoc->Release();
}

void CSearchDlg::OnBnClickedBtnMakineKalibi()
{
    if (!m_pApp) return;
    
    IDispatch* pTempSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (pTempSel) {
        long c = GetLongProp(pTempSel, L"Count");
        pTempSel->Release();
        if (c == 1) OnBnClickedBtnList();
    }
    
    CCorelOptimizer optimizer(m_pApp, L"Ztrass Makine Kalıbı");
    
    IDispatch* pDoc = GetDispatchProp(m_pApp, L"ActiveDocument");
    if (!pDoc) return;
    SetLongProp(pDoc, L"Unit", 3);
    
    IDispatch* pSel = GetDispatchProp(m_pApp, L"ActiveSelectionRange");
    if (!pSel) { pDoc->Release(); return; }
    long selCount = GetLongProp(pSel, L"Count");
    if (selCount == 0) {
        AfxMessageBox(_T("Lütfen makine kalıbı hazırlanacak kalıbı seçin."));
        pSel->Release(); pDoc->Release(); return;
    }
    
    struct SizeMap { double origMM; double machineMM; int editID; };
    SizeMap sizeTable[] = {
        {2.0, 0.0, IDC_EDT_2MM},
        {3.0, 0.0, IDC_EDT_2MM2},
        {4.0, 0.0, IDC_EDT_2MM3},
        {5.0, 0.0, IDC_EDT_2MM4},
        {6.0, 0.0, IDC_EDT_2MM5},
        {7.0, 0.0, IDC_EDT_2MM6},
        {8.0, 0.0, IDC_EDT_2MM7}
    };
    int tableSize = sizeof(sizeTable) / sizeof(sizeTable[0]);
    
    for (int i = 0; i < tableSize; i++) {
        CString strVal;
        GetDlgItemText(sizeTable[i].editID, strVal);
        strVal.Replace(L",", L".");
        double val = _wtof(strVal);
        sizeTable[i].machineMM = val;
    }
    
    double minSize = 0.6;
    double selH = GetDoubleProp(pSel, L"SizeHeight");
    
    double offset = -selH - 10.0;
    VARIANT args[2]; VariantInit(&args[0]); VariantInit(&args[1]);
    args[1].vt = VT_R8; args[1].dblVal = 0.0;
    args[0].vt = VT_R8; args[0].dblVal = offset;
    DISPPARAMS paramsDup = { args, NULL, 2, 0 };
    VARIANT retDup; VariantInit(&retDup);
    IDispatch* pDupRange = nullptr;
    DISPID dispDup; OLECHAR* szDup = (OLECHAR*)L"Duplicate";
    if (pSel->GetIDsOfNames(IID_NULL, &szDup, 1, LOCALE_USER_DEFAULT, &dispDup) == S_OK) {
        if (pSel->Invoke(dispDup, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &paramsDup, &retDup, NULL, NULL) == S_OK) {
            if (retDup.vt == VT_DISPATCH) {
                pDupRange = retDup.pdispVal;
            }
        }
    }
    
    if (!pDupRange) { pSel->Release(); pDoc->Release(); return; }
    
    IDispatch* pFlatRange = nullptr;
    IDispatch* pShapes = GetDispatchProp(pDupRange, L"Shapes");
    DISPID dispFind; OLECHAR* szFind = (OLECHAR*)L"FindShapes";
    if (pShapes && SUCCEEDED(pShapes->GetIDsOfNames(IID_NULL, &szFind, 1, LOCALE_USER_DEFAULT, &dispFind))) {
        DISPPARAMS params = { NULL, NULL, 0, 0 };
        VARIANT retVal; VariantInit(&retVal);
        if (SUCCEEDED(pShapes->Invoke(dispFind, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &retVal, NULL, NULL))) {
            if (retVal.vt == VT_DISPATCH) pFlatRange = retVal.pdispVal;
        }
    }
    if (pShapes) pShapes->Release();
    if (!pFlatRange) { pDupRange->Release(); pSel->Release(); pDoc->Release(); return; }
    long flatCount = GetLongProp(pFlatRange, L"Count");
    
    for (long i = 1; i <= flatCount; i++) {
        IDispatch* pShape = GetDispatchPropWithIntArg(pFlatRange, L"Item", i);
        if (!pShape) continue;
        
        bool isBlackOutline = false;
        bool isBlackFill = false;
        bool isNoFill = false;
        
        IDispatch* pOutline = GetDispatchProp(pShape, L"Outline");
        if (pOutline) {
            long outType = GetLongProp(pOutline, L"Type");
            if (outType != 0) {
                IDispatch* pColor = GetDispatchProp(pOutline, L"Color");
                if (pColor) {
                    long colorType = GetLongProp(pColor, L"Type");
                    if (colorType == 2) {
                        long r = GetLongProp(pColor, L"RGBRed");
                        long g = GetLongProp(pColor, L"RGBGreen");
                        long b = GetLongProp(pColor, L"RGBBlue");
                        if (r <= 10 && g <= 10 && b <= 10) isBlackOutline = true;
                    } else if (colorType == 3) {
                        long k = GetLongProp(pColor, L"CMYKBlack");
                        if (k >= 95) isBlackOutline = true;
                    }
                    pColor->Release();
                }
            }
            pOutline->Release();
        }
        
        IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
        if (pFill) {
            long fillType = GetLongProp(pFill, L"Type");
            if (fillType == 0) {
                isNoFill = true;
            } else if (fillType == 1) {
                IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                if (pColor) {
                    long colorType = GetLongProp(pColor, L"Type");
                    if (colorType == 2) {
                        long r = GetLongProp(pColor, L"RGBRed");
                        long g = GetLongProp(pColor, L"RGBGreen");
                        long b = GetLongProp(pColor, L"RGBBlue");
                        if (r <= 10 && g <= 10 && b <= 10) isBlackFill = true;
                    } else if (colorType == 3) {
                        long k = GetLongProp(pColor, L"CMYKBlack");
                        if (k >= 95) isBlackFill = true;
                    }
                    pColor->Release();
                }
            }
            pFill->Release();
        }
        
        bool isCommon = (isBlackOutline && (isBlackFill || isNoFill)) || (!isBlackOutline && isBlackFill);
        
        if (isCommon) {
            pShape->Release();
            continue;
        }
        
        double origW = GetDoubleProp(pShape, L"SizeWidth");
        double cx = GetDoubleProp(pShape, L"CenterX");
        double cy = GetDoubleProp(pShape, L"CenterY");
        
        double newSize = minSize;
        double bestDiff = 9999.0;
        for (int t = 0; t < tableSize; t++) {
            double diff = fabs(origW - sizeTable[t].origMM);
            if (diff < bestDiff) {
                bestDiff = diff;
                if (sizeTable[t].machineMM > 0.01) {
                    newSize = sizeTable[t].machineMM;
                }
            }
        }
        
        if (newSize < minSize) newSize = minSize;
        
        SetDoubleProp(pShape, L"SizeWidth", newSize);
        SetDoubleProp(pShape, L"SizeHeight", newSize);
        SetDoubleProp(pShape, L"CenterX", cx);
        SetDoubleProp(pShape, L"CenterY", cy);
        
        pShape->Release();
    }
    
    InvokeMethodNoArgs(pDupRange, L"CreateSelection");
    
    pFlatRange->Release();
    pDupRange->Release();
    pSel->Release();
    pDoc->Release();
}"""

    io.open(path_cpp, "w", encoding="cp1254").write(header + new_code)
    print("SearchDlg.cpp patched successfully!")
