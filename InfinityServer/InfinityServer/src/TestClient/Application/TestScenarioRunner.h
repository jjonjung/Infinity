#pragma once

#include "TestClient/Application/TestClientService.h"

#include <string>
#include <vector>

class TestScenarioRunner
{
public:
    explicit TestScenarioRunner(TestClientService& service);

    std::vector<TestScenarioExecution> RunDefaultRegression(const std::string& uniqueSuffix);

private:
    TestScenarioExecution RunRegisterAndLogin(const std::string& uniqueSuffix, int64_t& outUserId, std::string& outGameToken);
    TestScenarioExecution RunValidateGameSession(const std::string& gameToken);
    TestScenarioExecution RunSocialLogin(const std::string& provider, const std::string& token);
    TestScenarioExecution RunMatchAndStats(int64_t userId);

    TestClientService& m_service;
};
