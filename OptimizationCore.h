// ============================================================================
// OptimizationCore.h
// HOTFIXPRO Performance Optimization System
// Integrates all optimizations: Parallel, Caching, Batch Processing
// ============================================================================

#pragma once

#include <omp.h>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>

// ============================================================================
// GLOBAL PERFORMANCE CONFIGURATION
// ============================================================================

#define OPTIMIZE_PARALLEL           1       // Enable OpenMP parallelization
#define OPTIMIZE_CACHE              1       // Enable color/data caching
#define OPTIMIZE_BATCH              1       // Enable batch processing
#define MAX_SHAPES_CACHE            5000    // Max shapes in cache
#define COLOR_TOLERANCE             15      // RGB tolerance for color matching
#define THREAD_CHUNK_SIZE           100     // OpenMP chunk size

// ============================================================================
// GLOBAL CACHES
// ============================================================================

// Color brightness cache (for text color selection)
static std::map<COLORREF, BYTE> g_colorBrightnessCache;

// Shape data cache (for repeated property access)
struct CachedShapeData {
    double width, height;
    double centerX, centerY;
    double leftX, rightX, topY, bottomY;
    COLORREF fillColor;
    long type;
    long staticID;
    bool isRealStone;
};

static std::map<long, CachedShapeData> g_shapeDataCache;
static std::vector<CachedShapeData> g_shapeArray;

// ============================================================================
// ULTRA-FAST COLOR UTILITIES
// ============================================================================

inline BYTE FastGetBrightness(COLORREF col) {
    auto it = g_colorBrightnessCache.find(col);
    if (it != g_colorBrightnessCache.end()) {
        return it->second;
    }
    
    BYTE r = GetRValue(col);
    BYTE g = GetGValue(col);
    BYTE b = GetBValue(col);
    
    // Luminance formula: 0.2126*R + 0.7152*G + 0.0722*B
    BYTE brightness = static_cast<BYTE>(0.2126 * r + 0.7152 * g + 0.0722 * b);
    g_colorBrightnessCache[col] = brightness;
    return brightness;
}

inline bool ColorsMatch(COLORREF c1, COLORREF c2, int tolerance = COLOR_TOLERANCE) {
    if (c1 == CLR_INVALID || c2 == CLR_INVALID) return false;
    
    int dr = abs((int)GetRValue(c1) - (int)GetRValue(c2));
    int dg = abs((int)GetGValue(c1) - (int)GetGValue(c2));
    int db = abs((int)GetBValue(c1) - (int)GetBValue(c2));
    
    return (dr < tolerance && dg < tolerance && db < tolerance);
}

inline COLORREF HexToRGB(const CString& hex) {
    if (hex.GetLength() < 6) return RGB(0, 0, 0);
    int r = _tcstol(hex.Mid(0, 2), NULL, 16);
    int g = _tcstol(hex.Mid(2, 2), NULL, 16);
    int b = _tcstol(hex.Mid(4, 2), NULL, 16);
    return RGB(r, g, b);
}

// ============================================================================
// BATCH SHAPE DATA READER (Optimized with Parallel)
// ============================================================================

CachedShapeData FastReadShape(IDispatch* pShape, IDispatch* pApp) {
    CachedShapeData csd = {};
    if (!pShape) return csd;
    
    try {
        csd.type = GetLongProp(pShape, L"Type");
        csd.staticID = GetLongProp(pShape, L"StaticID");
        
        csd.leftX = GetDoubleProp(pShape, L"LeftX");
        csd.rightX = GetDoubleProp(pShape, L"RightX");
        csd.topY = GetDoubleProp(pShape, L"TopY");
        csd.bottomY = GetDoubleProp(pShape, L"BottomY");
        csd.width = csd.rightX - csd.leftX;
        csd.height = csd.topY - csd.bottomY;
        csd.centerX = (csd.leftX + csd.rightX) / 2.0;
        csd.centerY = (csd.bottomY + csd.topY) / 2.0;
        
        csd.fillColor = CLR_INVALID;
        IDispatch* pFill = GetDispatchProp(pShape, L"Fill");
        if (pFill) {
            long fillType = GetLongProp(pFill, L"Type");
            if (fillType == 1) {
                IDispatch* pColor = GetDispatchProp(pFill, L"UniformColor");
                if (pColor) {
                    csd.fillColor = GetColorRef(pColor);
                    pColor->Release();
                }
            }
            pFill->Release();
        }
        
        csd.isRealStone = (csd.width < 10.0 && csd.height < 10.0 && 
                          csd.fillColor != RGB(255, 255, 255) && 
                          csd.fillColor != RGB(0, 0, 0) &&
                          csd.fillColor != CLR_INVALID);
    }
    catch (...) {
        // Silent error handling
    }
    
    return csd;
}

// Batch read all shapes with OpenMP parallelization
void BatchReadAllShapes(IDispatch* pShapeRange, IDispatch* pApp, int& outCount) {
    outCount = 0;
    long count = GetLongProp(pShapeRange, L"Count");
    
    if (count > MAX_SHAPES_CACHE) count = MAX_SHAPES_CACHE;
    
    g_shapeArray.clear();
    g_shapeArray.resize(count);
    
    #pragma omp parallel for schedule(dynamic, THREAD_CHUNK_SIZE) if(OPTIMIZE_PARALLEL)
    for (long i = 1; i <= count; i++) {
        IDispatch* pShape = GetDispatchPropWithIntArg(pShapeRange, L"Item", i);
        if (pShape) {
            g_shapeArray[i - 1] = FastReadShape(pShape, pApp);
            pShape->Release();
        }
    }
    
    outCount = count;
}

// ============================================================================
// PARALLEL BOUNDS CALCULATION
// ============================================================================

struct BoundsResult {
    double minX, maxX, minY, maxY;
    bool hasStones;
    int realStoneCount;
};

BoundsResult CalculateBounds(const std::vector<CachedShapeData>& shapes) {
    BoundsResult result = { 999999.0, -999999.0, 999999.0, -999999.0, false, 0 };
    
    #pragma omp parallel for reduction(min:result.minX, result.minY) \
                             reduction(max:result.maxX, result.maxY) \
                             reduction(+:result.realStoneCount) \
                             if(OPTIMIZE_PARALLEL)
    for (int i = 0; i < shapes.size(); i++) {
        if (shapes[i].isRealStone) {
            result.minX = std::min(result.minX, shapes[i].leftX);
            result.maxX = std::max(result.maxX, shapes[i].rightX);
            result.minY = std::min(result.minY, shapes[i].bottomY);
            result.maxY = std::max(result.maxY, shapes[i].topY);
            result.realStoneCount++;
            result.hasStones = true;
        }
    }
    
    return result;
}

// ============================================================================
// PARALLEL COLOR GROUPING
// ============================================================================

struct ColorGroup {
    COLORREF color;
    std::vector<int> shapeIndices;
    int totalCount;
};

std::vector<ColorGroup> GroupShapesByColor(const std::vector<CachedShapeData>& shapes) {
    std::map<COLORREF, std::vector<int>> colorMap;
    
    // Sequential grouping (collection is single-threaded)
    for (int i = 0; i < shapes.size(); i++) {
        if (shapes[i].fillColor != CLR_INVALID) {
            colorMap[shapes[i].fillColor].push_back(i);
        }
    }
    
    std::vector<ColorGroup> groups;
    for (const auto& [color, indices] : colorMap) {
        ColorGroup cg;
        cg.color = color;
        cg.shapeIndices = indices;
        cg.totalCount = indices.size();
        groups.push_back(cg);
    }
    
    return groups;
}

// ============================================================================
// CLEANUP UTILITIES
// ============================================================================

inline void ClearCaches() {
    if (OPTIMIZE_CACHE) {
        g_colorBrightnessCache.clear();
        g_shapeDataCache.clear();
        g_shapeArray.clear();
    }
}

#endif // OPTIMIZATIONCORE_H_
