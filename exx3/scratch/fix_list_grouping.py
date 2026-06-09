# -*- coding: utf-8 -*-
import io

cpp_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(cpp_path, 'r', encoding='utf-8') as f:
    cpp_content = f.read()

old_loop_code = """        for (long i = 1; i <= count; ++i) {
            IDispatch* pShape = GetDispatchPropWithIntArg(pSelection, L"Item", i);
            if (pShape) {
                COLORREF col = RGB(128, 128, 128);
                IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
                long fillType = 0;
                if (pFill) {
                    fillType = GetLongProp(pFill, L"Type");
                    if (fillType == 1) {
                        IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                        if (pColor) {
                            col = GetColorRef(pColor);
                            pColor->Release();
                        }
                    }
                    pFill->Release();
                }
                if (fillType != 1) {
                    IDispatch* pOutline = GetDispatchProp(pShape, L"Outline");
                    if (pOutline) {
                        IDispatch* pColor = GetDispatchProp(pOutline, L"Color");
                        if (pColor) {
                            col = GetColorRef(pColor);
                            pColor->Release();
                        }
                        pOutline->Release();
                    }
                }
                
                double w = GetDoubleProp(pShape, L"SizeWidth");
                CString objText;
                objText.Format(_T("%.1fmm-%d"), w, i);
                objText.Replace(_T("."), _T(","));
                
                SelectedObjectInfo info;
                info.color = col;
                info.text = objText;
                m_listObjects.push_back(info);
                
                pShape->Release();
            }
        }
        
        std::vector<COLORREF> uniqueColors;
        for (const auto& obj : m_listObjects) {
            if (std::find(uniqueColors.begin(), uniqueColors.end(), obj.color) == uniqueColors.end()) {
                uniqueColors.push_back(obj.color);
            }
        }
        long numColors = uniqueColors.size();"""

new_loop_code = """        struct GroupedStone {
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
        long numColors = uniqueColors.size();"""

cpp_content = cpp_content.replace(old_loop_code, new_loop_code)

with io.open(cpp_path, 'w', encoding='utf-8') as f:
    f.write(cpp_content)

print("Grouping logic added to OnBnClickedBtnList.")
