import io
import re

path_cpp = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path_cpp, 'r', encoding='utf-8', errors='ignore') as f:
    text_cpp = f.read()

# Make sure we have <thread> and <vector> included
if "#include <thread>" not in text_cpp:
    text_cpp = text_cpp.replace("#include <afxdlgs.h>", "#include <afxdlgs.h>\n#include <thread>\n#include <vector>\n#include <cmath>")

# 1. Update the OnBnClickedBtnFind to use std::thread with 8 threads
old_find_logic = """            struct CircleInfo {
                long index;
                double cx, cy, radius;
                bool markForDelete;
            };
            std::vector<CircleInfo> circles;
            
            for (long i = 1; i <= count; ++i) {
                IDispatch* pShape = GetDispatchPropWithIntArg(pSel, L"Item", i);
                if (pShape) {
                    double w = GetDoubleProp(pShape, L"SizeWidth");
                    double cx = GetDoubleProp(pShape, L"CenterX");
                    double cy = GetDoubleProp(pShape, L"CenterY");
                    circles.push_back({i, cx, cy, w / 2.0, false});
                    pShape->Release();
                }
            }
            
            for (size_t i = 0; i < circles.size(); ++i) {
                if (circles[i].markForDelete) continue;
                for (size_t j = i + 1; j < circles.size(); ++j) {
                    if (circles[j].markForDelete) continue;
                    
                    double dx = circles[i].cx - circles[j].cx;
                    double dy = circles[i].cy - circles[j].cy;
                    double dist = sqrt(dx*dx + dy*dy);
                    
                    double requiredDist = circles[i].radius + circles[j].radius - tol;
                    if (dist < requiredDist) {
                        circles[j].markForDelete = true;
                        if (bCenter) {
                            circles[i].cx = (circles[i].cx + circles[j].cx) / 2.0;
                            circles[i].cy = (circles[i].cy + circles[j].cy) / 2.0;
                        }
                    }
                }
            }"""

new_find_logic = """            struct CircleInfo {
                long index;
                double cx, cy, radius;
                volatile bool markForDelete;
            };
            std::vector<CircleInfo> circles(count);
            
            for (long i = 1; i <= count; ++i) {
                IDispatch* pShape = GetDispatchPropWithIntArg(pSel, L"Item", i);
                if (pShape) {
                    double w = GetDoubleProp(pShape, L"SizeWidth");
                    double cx = GetDoubleProp(pShape, L"CenterX");
                    double cy = GetDoubleProp(pShape, L"CenterY");
                    circles[i - 1] = {i, cx, cy, w / 2.0, false};
                    pShape->Release();
                }
            }
            
            // Multithreading ile hizlandirma (8 is parcacigi)
            int numThreads = 8;
            std::vector<std::thread> threads;
            size_t n = circles.size();
            
            for (int t = 0; t < numThreads; ++t) {
                threads.emplace_back([&circles, n, tol, bCenter, t, numThreads]() {
                    // Blok dagitimi (Block distribution) i=0..n-1
                    size_t start = (n * t) / numThreads;
                    size_t end = (n * (t + 1)) / numThreads;
                    
                    for (size_t i = start; i < end; ++i) {
                        if (circles[i].markForDelete) continue;
                        
                        for (size_t j = i + 1; j < n; ++j) {
                            if (circles[j].markForDelete) continue;
                            
                            double dx = circles[i].cx - circles[j].cx;
                            double dy = circles[i].cy - circles[j].cy;
                            
                            // Karekteristik optimizasyon: distance hesaplamadan once bounding box kontrolu (cok hizlandirir)
                            double requiredDist = circles[i].radius + circles[j].radius - tol;
                            if (abs(dx) > requiredDist || abs(dy) > requiredDist) continue;
                            
                            double dist = sqrt(dx*dx + dy*dy);
                            if (dist < requiredDist) {
                                circles[j].markForDelete = true;
                                if (bCenter) {
                                    circles[i].cx = (circles[i].cx + circles[j].cx) / 2.0;
                                    circles[i].cy = (circles[i].cy + circles[j].cy) / 2.0;
                                }
                            }
                        }
                    }
                });
            }
            
            for (auto& th : threads) {
                th.join();
            }"""

text_cpp = text_cpp.replace(old_find_logic, new_find_logic)

with io.open(path_cpp, 'w', encoding='utf-8') as f:
    f.write(text_cpp)

print("Multithreading added to OnBnClickedBtnFind.")
