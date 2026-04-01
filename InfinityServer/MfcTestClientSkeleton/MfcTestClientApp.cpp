#include "MfcTestClientApp.h"
#include "MfcTestClientMainDialog.h"

BEGIN_MESSAGE_MAP(CMfcTestClientApp, CWinApp)
END_MESSAGE_MAP()

CMfcTestClientApp theApp;

BOOL CMfcTestClientApp::InitInstance()
{
    CWinApp::InitInstance();

    CMfcTestClientMainDialog dialog;
    m_pMainWnd = &dialog;
    dialog.DoModal();
    return FALSE;
}
