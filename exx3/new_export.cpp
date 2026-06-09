        IDispatch* pExpOpt = nullptr;
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
        if (pExpOpt) pExpOpt->Release();