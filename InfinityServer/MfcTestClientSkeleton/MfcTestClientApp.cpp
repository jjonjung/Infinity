#include "MfcTestClientApp.h"
#include "MfcTestClientMainDialog.h"

BEGIN_MESSAGE_MAP(CMfcTestClientApp, CWinApp)
END_MESSAGE_MAP()

CMfcTestClientApp theApp;

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previousInstance, LPWSTR commandLine, int showCommand)
{
    if (!AfxWinInit(instance, previousInstance, commandLine, showCommand))
    {
        return 1;
    }

    theApp.InitApplication();
    theApp.InitInstance();
    return 0;
}

BOOL CMfcTestClientApp::InitInstance()
{
    CWinApp::InitInstance();

    CMfcTestClientMainDialog dialog;
    m_pMainWnd = &dialog;
    dialog.DoModal();
    return FALSE;
}
