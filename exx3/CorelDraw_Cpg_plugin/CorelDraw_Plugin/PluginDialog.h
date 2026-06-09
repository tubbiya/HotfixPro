#pragma once
#include "Resource.h"

class CPluginDialog : public CDialogEx
{
    DECLARE_DYNAMIC(CPluginDialog)

public:
    CPluginDialog(CWnd* pParent = nullptr);
    virtual ~CPluginDialog();

    enum { IDD = IDD_PLUGIN_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    
    afx_msg void OnBnClickedButtonOk();
    afx_msg void OnBnClickedButtonCancel();
    
    DECLARE_MESSAGE_MAP()

private:
    CString m_strText;
    CEdit m_editText;
};
