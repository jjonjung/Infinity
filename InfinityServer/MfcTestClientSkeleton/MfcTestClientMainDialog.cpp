#include "MfcTestClientMainDialog.h"
#include "resource.h"

#include <sstream>

BEGIN_MESSAGE_MAP(CMfcTestClientMainDialog, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CMfcTestClientMainDialog::OnBnClickedConnect)
    ON_BN_CLICKED(IDC_BUTTON_REGISTER, &CMfcTestClientMainDialog::OnBnClickedRegister)
    ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CMfcTestClientMainDialog::OnBnClickedLogin)
    ON_BN_CLICKED(IDC_BUTTON_GOOGLE_LOGIN, &CMfcTestClientMainDialog::OnBnClickedGoogleLogin)
    ON_BN_CLICKED(IDC_BUTTON_STEAM_LOGIN, &CMfcTestClientMainDialog::OnBnClickedSteamLogin)
    ON_BN_CLICKED(IDC_BUTTON_VALIDATE_TOKEN, &CMfcTestClientMainDialog::OnBnClickedValidateToken)
    ON_BN_CLICKED(IDC_BUTTON_SUBMIT_MATCH, &CMfcTestClientMainDialog::OnBnClickedSubmitMatch)
    ON_BN_CLICKED(IDC_BUTTON_FETCH_STATS, &CMfcTestClientMainDialog::OnBnClickedFetchStats)
    ON_BN_CLICKED(IDC_BUTTON_RUN_REGRESSION, &CMfcTestClientMainDialog::OnBnClickedRunRegression)
END_MESSAGE_MAP()

CMfcTestClientMainDialog::CMfcTestClientMainDialog()
    : CDialogEx(IDD)
{
}

BOOL CMfcTestClientMainDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    WriteText(IDC_EDIT_HOST, _T("127.0.0.1"));
    WriteText(IDC_EDIT_PORT, _T("9000"));
    WriteText(IDC_EDIT_EMAIL, _T("tester@infinity.local"));
    WriteText(IDC_EDIT_PASSWORD, _T("pass1234"));
    WriteText(IDC_EDIT_NICKNAME, _T("LocalTester"));
    WriteText(IDC_EDIT_PROVIDER_TOKEN, _T("google-dev-token"));
    AppendLog(_T("MFC test client initialized"));
    return TRUE;
}

void CMfcTestClientMainDialog::OnBnClickedConnect()
{
    CString host = ReadText(IDC_EDIT_HOST);
    CString portText = ReadText(IDC_EDIT_PORT);
    std::string errorMessage;

    const bool connected = m_service.Connect(CT2A(host), static_cast<uint16_t>(_ttoi(portText)), errorMessage);
    AppendLog(connected ? _T("Connected to server") : CString(errorMessage.c_str()));
}

void CMfcTestClientMainDialog::OnBnClickedRegister()
{
    ClientRegisterRequest request;
    request.Email = CT2A(ReadText(IDC_EDIT_EMAIL));
    request.Password = CT2A(ReadText(IDC_EDIT_PASSWORD));
    request.Nickname = CT2A(ReadText(IDC_EDIT_NICKNAME));

    RenderResult(m_service.RegisterLocal(request));
}

void CMfcTestClientMainDialog::OnBnClickedLogin()
{
    ClientLoginRequest request;
    request.Email = CT2A(ReadText(IDC_EDIT_EMAIL));
    request.Password = CT2A(ReadText(IDC_EDIT_PASSWORD));

    RenderResult(m_service.LoginLocal(request));
}

void CMfcTestClientMainDialog::OnBnClickedGoogleLogin()
{
    ClientSocialLoginRequest request;
    request.Provider = "google";
    request.ProviderToken = CT2A(ReadText(IDC_EDIT_PROVIDER_TOKEN));

    RenderResult(m_service.LoginSocial(request));
}

void CMfcTestClientMainDialog::OnBnClickedSteamLogin()
{
    ClientSocialLoginRequest request;
    request.Provider = "steam";
    request.ProviderToken = "steam-dev-ticket";

    RenderResult(m_service.LoginSocial(request));
}

void CMfcTestClientMainDialog::OnBnClickedValidateToken()
{
    RenderResult(m_service.ValidateGameSession(CT2A(ReadText(IDC_EDIT_GAME_TOKEN))));
}

void CMfcTestClientMainDialog::OnBnClickedSubmitMatch()
{
    ClientMatchRequest request;
    request.MatchId = "mfc-manual-001";
    request.WinnerTeam = "Blue";
    request.Players.push_back({
        _ttoi64(ReadText(IDC_EDIT_USER_ID)),
        "Blue",
        "DoctorStrange",
        6,
        2,
        4,
        5400,
        2100,
        1100,
        "WIN"
    });

    RenderResult(m_service.SubmitMatchResult(request));
}

void CMfcTestClientMainDialog::OnBnClickedFetchStats()
{
    RenderResult(m_service.QueryPlayerStats(_ttoi64(ReadText(IDC_EDIT_USER_ID))));
}

void CMfcTestClientMainDialog::OnBnClickedRunRegression()
{
    TestScenarioRunner runner(m_service);
    const auto executions = runner.RunDefaultRegression("mfc");

    for (const TestScenarioExecution& execution : executions)
    {
        CString line;
        line.Format(_T("[%s] %s - %s"),
                    CString(execution.Name.c_str()),
                    execution.Success ? _T("OK") : _T("FAIL"),
                    CString(execution.Message.c_str()));
        AppendLog(line);
    }
}

void CMfcTestClientMainDialog::AppendLog(const CString& line)
{
    CString current;
    GetDlgItemText(IDC_EDIT_LOG, current);
    current += line + _T("\r\n");
    SetDlgItemText(IDC_EDIT_LOG, current);
}

CString CMfcTestClientMainDialog::ReadText(int controlId) const
{
    CString value;
    GetDlgItemText(controlId, value);
    return value;
}

void CMfcTestClientMainDialog::WriteText(int controlId, const CString& value)
{
    SetDlgItemText(controlId, value);
}

void CMfcTestClientMainDialog::RenderResult(const ClientOperationResult& result)
{
    CString line;
    line.Format(_T("success=%d userId=%lld message=%s"),
                result.Success ? 1 : 0,
                result.UserId,
                CString(result.Message.c_str()));
    AppendLog(line);

    if (result.UserId > 0)
    {
        CString userIdText;
        userIdText.Format(_T("%lld"), result.UserId);
        WriteText(IDC_EDIT_USER_ID, userIdText);
    }

    if (!result.Tokens.GameSessionToken.empty())
    {
        WriteText(IDC_EDIT_GAME_TOKEN, CString(result.Tokens.GameSessionToken.c_str()));
    }

    if (result.Stats.UserId > 0)
    {
        CString statsLine;
        statsLine.Format(_T("stats matches=%d wins=%d kills=%d deaths=%d assists=%d damage=%d"),
                         result.Stats.TotalMatches,
                         result.Stats.TotalWins,
                         result.Stats.TotalKills,
                         result.Stats.TotalDeaths,
                         result.Stats.TotalAssists,
                         result.Stats.TotalDamageDealt);
        AppendLog(statsLine);
    }
}
