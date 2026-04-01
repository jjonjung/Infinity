#include "TestClient/Application/TestScenarioRunner.h"

TestScenarioRunner::TestScenarioRunner(TestClientService& service)
    : m_service(service)
{
}

std::vector<TestScenarioExecution> TestScenarioRunner::RunDefaultRegression(const std::string& uniqueSuffix)
{
    std::vector<TestScenarioExecution> executions;

    int64_t userId = 0;
    std::string gameToken;
    executions.push_back(RunRegisterAndLogin(uniqueSuffix, userId, gameToken));
    executions.push_back(RunValidateGameSession(gameToken));
    executions.push_back(RunSocialLogin("google", "google-dev-token"));
    executions.push_back(RunSocialLogin("steam", "steam-dev-ticket"));
    executions.push_back(RunMatchAndStats(userId));
    return executions;
}

TestScenarioExecution TestScenarioRunner::RunRegisterAndLogin(const std::string& uniqueSuffix,
                                                              int64_t& outUserId,
                                                              std::string& outGameToken)
{
    TestScenarioExecution execution;
    execution.Name = "register-login";

    ClientRegisterRequest registerRequest;
    registerRequest.Email = "qa_" + uniqueSuffix + "@infinity.local";
    registerRequest.Password = "pass1234";
    registerRequest.Nickname = "qa_" + uniqueSuffix;

    const auto registerResult = m_service.RegisterLocal(registerRequest);
    if (!registerResult.Success)
    {
        execution.Message = registerResult.Message;
        return execution;
    }

    ClientLoginRequest loginRequest;
    loginRequest.Email = registerRequest.Email;
    loginRequest.Password = registerRequest.Password;

    const auto loginResult = m_service.LoginLocal(loginRequest);
    execution.Success = loginResult.Success;
    execution.Message = loginResult.Message;
    outUserId = loginResult.UserId;
    outGameToken = loginResult.Tokens.GameSessionToken;
    return execution;
}

TestScenarioExecution TestScenarioRunner::RunValidateGameSession(const std::string& gameToken)
{
    TestScenarioExecution execution;
    execution.Name = "validate-game-session";

    const auto result = m_service.ValidateGameSession(gameToken);
    execution.Success = result.Success;
    execution.Message = result.Message;
    return execution;
}

TestScenarioExecution TestScenarioRunner::RunSocialLogin(const std::string& provider, const std::string& token)
{
    TestScenarioExecution execution;
    execution.Name = provider + "-login";

    ClientSocialLoginRequest request;
    request.Provider = provider;
    request.ProviderToken = token;

    const auto result = m_service.LoginSocial(request);
    execution.Success = result.Success;
    execution.Message = result.Message;
    return execution;
}

TestScenarioExecution TestScenarioRunner::RunMatchAndStats(int64_t userId)
{
    TestScenarioExecution execution;
    execution.Name = "match-stats";

    ClientMatchRequest matchRequest;
    matchRequest.MatchId = "mfc-regression-001";
    matchRequest.WinnerTeam = "Blue";
    matchRequest.Players.push_back({ userId, "Blue", "DoctorStrange", 5, 2, 3, 4200, 1800, 980, "WIN" });

    const auto matchResult = m_service.SubmitMatchResult(matchRequest);
    if (!matchResult.Success)
    {
        execution.Message = matchResult.Message;
        return execution;
    }

    const auto statsResult = m_service.QueryPlayerStats(userId);
    execution.Success = statsResult.Success;
    execution.Message = statsResult.Message;
    return execution;
}
