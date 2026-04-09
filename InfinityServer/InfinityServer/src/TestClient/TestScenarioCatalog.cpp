#include "TestClient/TestScenarioCatalog.h"

std::vector<TestScenario> TestScenarioCatalog::BuildDefaultScenarios()
{
    return {
        { "local-login", "email/password login and token issue flow" },
        { "google-login", "google token exchange and linked account flow" },
        { "steam-login", "steam identity verification and linked account flow" },
        { "dedicated-handshake", "game session token validation against dedicated server" },
        { "match-result-post", "dedicated server to match api result submission flow" },
        { "admin-monitoring", "monitoring snapshot for redis/mysql/session and leaderboard state" },
        { "reconnect", "session restore using refresh token and renewed game token" }
    };
}
