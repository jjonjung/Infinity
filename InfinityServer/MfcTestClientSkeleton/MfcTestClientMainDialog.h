#pragma once

#include "../InfinityServer/src/TestClient/Application/TestClientService.h"
#include "../InfinityServer/src/TestClient/Application/TestScenarioRunner.h"

#include <afxwin.h>

class CMfcTestClientMainDialog : public CDialogEx
{
public:
    enum { IDD = 1000 };

    CMfcTestClientMainDialog();

protected:
    BOOL OnInitDialog() override;
    afx_msg void OnBnClickedConnect();
    afx_msg void OnBnClickedRegister();
    afx_msg void OnBnClickedLogin();
    afx_msg void OnBnClickedGoogleLogin();
    afx_msg void OnBnClickedSteamLogin();
    afx_msg void OnBnClickedValidateToken();
    afx_msg void OnBnClickedSubmitMatch();
    afx_msg void OnBnClickedFetchStats();
    afx_msg void OnBnClickedRunRegression();
    DECLARE_MESSAGE_MAP()

private:
    void AppendLog(const CString& line);
    CString ReadText(int controlId) const;
    void WriteText(int controlId, const CString& value);
    void RenderResult(const ClientOperationResult& result);

    TestClientService m_service;
};
