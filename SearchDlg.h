#pragma once
#include <afxcmn.h>
#include <afxwin.h>
#include "resource.h"
#include <vector>
#include <map>


struct SelectedObjectInfo {
    COLORREF color;
    CString text;
    double width;
    double height;
    int count;
};

class CSearchDlg : public CDialog
{
    DECLARE_DYNAMIC(CSearchDlg)

public:
    CSearchDlg(IDispatch* pApp, CWnd* pParent = nullptr);
    virtual ~CSearchDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_SEARCH_DLG };
#endif

protected:
    BOOL m_bAyarOpen;
    BOOL m_bCompactMode;
    std::vector<HWND> m_vecVisibleControls;
    CStatic m_wndAdBox;
    CFont m_fontAd;
    void SetCompactMode(BOOL bCompact);
    virtual void DoDataExchange(CDataExchange* pDX) override;
    virtual BOOL OnInitDialog() override;
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
    virtual void PostNcDestroy() override;
    virtual void OnCancel() override;
    virtual void OnOK() override;

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL PreTranslateMessage(MSG* pMsg) override;
    // UI Kontrol De�Yi�Ykenleri
    BOOL m_bOlcu;
    BOOL m_bIcRenk;
    BOOL m_bTur;
    BOOL m_bKonturOlcu;
    BOOL m_bKonturRenk;
    BOOL m_bGrupIci;
    BOOL m_bNodeCount;
    BOOL m_bSubpathCount;
    BOOL m_bActiveLayer;
    
    // Kesi�Yen Objeler Bölümü De�Yi�Ykenleri
    double m_dTolerance;
    double m_minX_desen;
    double m_maxX_desen;
    double m_minY_desen;
    double m_maxY_desen;
    int m_nIntersectAction; // 0=Seç, 1=Sil, 2=Ortala
    BOOL m_bTop;
    long m_lastSelCount;
    double m_lastSelW;
    double m_lastSelH;
    BOOL m_bBottom;
    BOOL m_bSame;
    
    // Liste Bölümü De�Yi�Ykenleri
    BOOL m_bKalip;
    BOOL m_bHesap;
    BOOL m_bZtrass;
    BOOL m_bListOption;
    CListCtrl m_listHesap;
    std::vector<SelectedObjectInfo> m_listObjects;
    std::map<COLORREF, std::pair<CString, CString>> m_parsedStoneDetails;
    
    // Geni�Yleme ve Dinamik ComboBox De�Yi�Ykenleri
    BOOL m_bListOpen;
    std::vector<CComboBox*> m_combosTas;
    std::vector<CComboBox*> m_combosTip;
    std::vector<class CDarkGroupBox*> m_darkBoxes;
    CStatusBarCtrl m_wndStatusBar;
    void SetStatus(LPCTSTR lpszText);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    
    
    // Renklendirme Fırçaları
    CBrush m_brushAra;
    CBrush m_brushOlcu;
    CBrush m_brushKesisen;
    CBrush m_brushExport;
    CBrush m_brushListe;
    CBrush m_brushKalipBilgi;
    CBrush m_brushKalipDuzenle;
    
    // Yardımcı Metotlar
    void ClearDynamicCombos();
    void UpdateListLayout();
    void SetDialogHeight(int heightDU);
    void SetDialogWidth(int widthDU);
    void UpdateDialogWidth();
    void ShowKalipPanel(BOOL bShow);
    
    // Kar�Yıla�Ytırma karakteri (=, <, >, <=, >=)
    CString m_strOlcuOp;

    // CorelDRAW Application COM Arayüzü
    IDispatch* m_pApp;

    // Buton ve Olay İ�Yleyiciler
    afx_msg void OnBnClickedBtnAra();
    afx_msg void OnBnClickedBtnAra2();
    afx_msg void OnBnClickedBtnAra5();
    afx_msg void OnBnClickedBtnAra6();
    afx_msg void OnBnClickedBtnAra7();
    afx_msg void OnBnClickedAra3Range(UINT nID);
    void ResizeShapes(double w_mm, double h_mm);
    afx_msg void OnBnClickedBtnOlcuOp();
    afx_msg void OnBnClickedBtnFind();
    afx_msg void OnBnClickedBtnExport();
    afx_msg void OnEnChangeEdtEn();
    afx_msg void OnBnClickedBtnSelectMatch();
    afx_msg void OnPaint();
    afx_msg void OnBnClickedBtnList();
    afx_msg void OnBnClickedChkListOption();
    afx_msg void OnBnClickedSimge();
    afx_msg void OnBnClickedAyarToggle();
    afx_msg void OnBnClickedChkKalip();
    afx_msg void OnBnClickedChkZtrass();
    afx_msg void OnBnClickedChkHesap();
    afx_msg void OnBnClickedBtnBilgiEkle();
    afx_msg void OnBnClickedBtnKalipEkle();
    afx_msg void OnBnClickedBtnIsaretTasi();
    afx_msg void OnBnClickedBtnImalat();
    afx_msg void OnBnClickedBtnKalipAyir();
    afx_msg void OnBnClickedBtnMakineKalibi();
    afx_msg void OnBnClickedBtnHesapla();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnCustomDrawList(NMHDR* pNMHDR, LRESULT* pResult);

    // Alternatif COM Tarama Arama Yöntemi
    IDispatch* SearchByCOMScan(IDispatch* pActiveShape);
};
