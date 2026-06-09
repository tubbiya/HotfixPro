#include "pch.h"
#include "PluginDialog.h"

IMPLEMENT_DYNAMIC(CPluginDialog, CDialogEx)

CPluginDialog::CPluginDialog(CWnd* pParent)
    : CDialogEx(IDD_PLUGIN_DIALOG, pParent)
{
}

CPluginDialog::~CPluginDialog()
{
}

void CPluginDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_TEXT, m_editText);
    DDX_Text(pDX, IDC_EDIT_TEXT, m_strText);
}

BEGIN_MESSAGE_MAP(CPluginDialog, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_OK, &CPluginDialog::OnBnClickedButtonOk)
    ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CPluginDialog::OnBnClickedButtonCancel)
END_MESSAGE_MAP()

BOOL CPluginDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    
    // Dialog başlangıç ayarları
    m_strText = _T("CorelDRAW eklentisine hoş geldiniz!");
    UpdateData(FALSE);
    
    return TRUE;
}

void CPluginDialog::OnBnClickedButtonOk()
{
    UpdateData(TRUE);
    
    // Burada kullanıcının girdiği metni işleyebilirsiniz
    // Örneğin: MessageBox ile gösterelim
    CString msg;
    msg.Format(_T("Girdiğiniz metin:\n%s"), m_strText);
    MessageBox(msg, _T("Bilgi"), MB_OK | MB_ICONINFORMATION);
    
    CDialogEx::OnOK();
}

void CPluginDialog::OnBnClickedButtonCancel()
{
    CDialogEx::OnCancel();
}
