#include "TestClient/Application/TestClientService.h"

#include "Packet/Packet.h"

#include <cstring>

namespace
{
void CopyToFixed(char* target, size_t size, const std::string& value)
{
    if (size == 0)
    {
        return;
    }

    strncpy_s(target, size, value.c_str(), _TRUNCATE);
}

ClientOperationResult ToLoginResult(const LoginResBody& response)
{
    ClientOperationResult result;
    result.Success = response.result == 0;
    result.UserId = response.user_id;
    result.Provider = response.provider;
    result.Message = response.message;
    result.Tokens.AccessToken = response.access_token;
    result.Tokens.RefreshToken = response.refresh_token;
    result.Tokens.GameSessionToken = response.game_session_token;
    return result;
}
}

bool TestClientService::Connect(const std::string& host, uint16_t port, std::string& errorMessage)
{
    return m_connection.Connect(host, port, errorMessage);
}

void TestClientService::Disconnect()
{
    m_connection.Disconnect();
}

bool TestClientService::IsConnected() const
{
    return m_connection.IsConnected();
}

ClientOperationResult TestClientService::RegisterLocal(const ClientRegisterRequest& request)
{
    ClientOperationResult result;
    RegisterReqBody body{};
    CopyToFixed(body.email, sizeof(body.email), request.Email);
    CopyToFixed(body.password, sizeof(body.password), request.Password);
    CopyToFixed(body.nickname, sizeof(body.nickname), request.Nickname);

    std::string errorMessage;
    if (!m_connection.SendPacket(OP_REGISTER_REQ, &body, sizeof(body), errorMessage))
    {
        result.Message = errorMessage;
        return result;
    }

    RegisterResBody response{};
    if (!ReceiveTypedPacket(OP_REGISTER_RES, response, errorMessage))
    {
        result.Message = errorMessage;
        return result;
    }

    result.Success = response.result == 0;
    result.UserId = response.user_id;
    result.Message = response.message;
    result.Tokens.GameSessionToken = response.game_session_token;
    return result;
}

ClientOperationResult TestClientService::LoginLocal(const ClientLoginRequest& request)
{
    ClientOperationResult result;
    LoginReqBody body{};
    CopyToFixed(body.username, sizeof(body.username), request.Email);
    CopyToFixed(body.password, sizeof(body.password), request.Password);

    std::string errorMessage;
    if (!m_connection.SendPacket(OP_LOGIN_REQ, &body, sizeof(body), errorMessage))
    {
        result.Message = errorMessage;
        return result;
    }

    LoginResBody response{};
    if (!ReceiveTypedPacket(OP_LOGIN_RES, response, errorMessage))
    {
        result.Message = errorMessage;
        return result;
    }

    return ToLoginResult(response);
}

ClientOperationResult TestClientService::LoginSocial(const ClientSocialLoginRequest& request)
{
    ClientOperationResult result;
    SocialLoginReqBody body{};
    CopyToFixed(body.provider, sizeof(body.provider), request.Provider);
    CopyToFixed(body.provider_token, sizeof(body.provider_token), request.ProviderToken);

    std::string errorMessage;
    if (!m_connection.SendPacket(OP_SOCIAL_LOGIN_REQ, &body, sizeof(body), errorMessage))
    {
        result.Message = errorMessage;
        return result;
    }

    LoginResBody response{};
    if (!ReceiveTypedPacket(OP_LOGIN_RES, response, errorMessage))
    {
        result.Message = errorMessage;
        return result;
    }

    return ToLoginResult(response);
}

ClientOperationResult TestClientService::ValidateGameSession(const std::string& gameSessionToken)
{
    ClientOperationResult result;
    ValidateGameTokenReqBody body{};
    CopyToFixed(body.game_session_token, sizeof(body.game_session_token), gameSessionToken);

    std::string errorMessage;
    if (!m_connection.SendPacket(OP_VALIDATE_GAME_TOKEN_REQ, &body, sizeof(body), errorMessage))
    {
        result.Message = errorMessage;
        return result;
    }

    ValidateGameTokenResBody response{};
    if (!ReceiveTypedPacket(OP_VALIDATE_GAME_TOKEN_RES, response, errorMessage))
    {
        result.Message = errorMessage;
        return result;
    }

    result.Success = response.result == 0;
    result.UserId = response.user_id;
    result.Message = response.message;
    return result;
}

ClientOperationResult TestClientService::SubmitMatchResult(const ClientMatchRequest& request)
{
    ClientOperationResult result;
    MatchResultReqBody body{};
    CopyToFixed(body.match_id, sizeof(body.match_id), request.MatchId);
    CopyToFixed(body.winner_team, sizeof(body.winner_team), request.WinnerTeam);
    body.player_count = static_cast<int32_t>(request.Players.size() > 4 ? 4 : request.Players.size());

    for (int index = 0; index < body.player_count; ++index)
    {
        const ClientMatchPlayer& player = request.Players[index];
        body.players[index].user_id = player.UserId;
        CopyToFixed(body.players[index].team, sizeof(body.players[index].team), player.Team);
        CopyToFixed(body.players[index].character_name, sizeof(body.players[index].character_name), player.CharacterName);
        body.players[index].kills = player.Kills;
        body.players[index].deaths = player.Deaths;
        body.players[index].assists = player.Assists;
        body.players[index].damage_dealt = player.DamageDealt;
        body.players[index].damage_taken = player.DamageTaken;
        body.players[index].score = player.Score;
        CopyToFixed(body.players[index].result, sizeof(body.players[index].result), player.Result);
    }

    std::string errorMessage;
    if (!m_connection.SendPacket(OP_MATCH_RESULT_REQ, &body, sizeof(body), errorMessage))
    {
        result.Message = errorMessage;
        return result;
    }

    MatchResultResBody response{};
    if (!ReceiveTypedPacket(OP_MATCH_RESULT_RES, response, errorMessage))
    {
        result.Message = errorMessage;
        return result;
    }

    result.Success = response.result == 0;
    result.Message = response.message;
    return result;
}

ClientOperationResult TestClientService::QueryPlayerStats(int64_t userId)
{
    ClientOperationResult result;
    PlayerStatsReqBody body{};
    body.user_id = userId;

    std::string errorMessage;
    if (!m_connection.SendPacket(OP_PLAYER_STATS_REQ, &body, sizeof(body), errorMessage))
    {
        result.Message = errorMessage;
        return result;
    }

    PlayerStatsResBody response{};
    if (!ReceiveTypedPacket(OP_PLAYER_STATS_RES, response, errorMessage))
    {
        result.Message = errorMessage;
        return result;
    }

    result.Success = response.result == 0;
    result.UserId = response.user_id;
    result.Message = response.message;
    result.Stats.UserId = response.user_id;
    result.Stats.TotalMatches = response.total_matches;
    result.Stats.TotalWins = response.total_wins;
    result.Stats.TotalKills = response.total_kills;
    result.Stats.TotalDeaths = response.total_deaths;
    result.Stats.TotalAssists = response.total_assists;
    result.Stats.TotalDamageDealt = response.total_damage_dealt;
    return result;
}

ClientOperationResult TestClientService::QueryMonitoringSnapshot()
{
    ClientOperationResult result;
    std::string errorMessage;
    if (!m_connection.SendPacket(OP_ADMIN_MONITORING_REQ, nullptr, 0, errorMessage))
    {
        result.Message = errorMessage;
        return result;
    }

    AdminMonitoringResBody response{};
    if (!ReceiveTypedPacket(OP_ADMIN_MONITORING_RES, response, errorMessage))
    {
        result.Message = errorMessage;
        return result;
    }

    result.Success = response.result == 0;
    result.Message = response.message;
    result.Monitoring.ActiveMatchCount = response.active_match_count;
    result.Monitoring.ConnectedSessionCount = response.connected_session_count;
    result.Monitoring.CachedLeaderboardEntryCount = response.cached_leaderboard_entry_count;

    for (int index = 0; index < response.node_count && index < 4; ++index)
    {
        result.Monitoring.Nodes.push_back({
            response.nodes[index].name,
            response.nodes[index].healthy != 0
        });
    }

    return result;
}

template <typename TResponse>
bool TestClientService::ReceiveTypedPacket(uint16_t expectedOpcode, TResponse& response, std::string& errorMessage)
{
    PacketEnvelope envelope;
    if (!m_connection.ReceivePacket(envelope, errorMessage))
    {
        return false;
    }

    if (envelope.Opcode != expectedOpcode)
    {
        errorMessage = "unexpected opcode received";
        return false;
    }

    if (envelope.Body.size() < sizeof(TResponse))
    {
        errorMessage = "response body too small";
        return false;
    }

    std::memcpy(&response, envelope.Body.data(), sizeof(TResponse));
    return true;
}

template bool TestClientService::ReceiveTypedPacket<LoginResBody>(uint16_t, LoginResBody&, std::string&);
template bool TestClientService::ReceiveTypedPacket<RegisterResBody>(uint16_t, RegisterResBody&, std::string&);
template bool TestClientService::ReceiveTypedPacket<ValidateGameTokenResBody>(uint16_t, ValidateGameTokenResBody&, std::string&);
template bool TestClientService::ReceiveTypedPacket<MatchResultResBody>(uint16_t, MatchResultResBody&, std::string&);
template bool TestClientService::ReceiveTypedPacket<PlayerStatsResBody>(uint16_t, PlayerStatsResBody&, std::string&);
template bool TestClientService::ReceiveTypedPacket<AdminMonitoringResBody>(uint16_t, AdminMonitoringResBody&, std::string&);
