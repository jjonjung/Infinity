#pragma once

#include <string>
#include <vector>

struct TestScenario
{
    std::string Name;
    std::string Description;
};

class TestScenarioCatalog
{
public:
    static std::vector<TestScenario> BuildDefaultScenarios();
};
