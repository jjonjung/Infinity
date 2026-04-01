#include "Packet/PacketHandler.h"

#include "Bootstrap/ServerRuntime.h"
#include "Network/Session.h"
#include "Packet/Packet.h"
#include "Infrastructure/Repositories/MatchRepository.h"
#include "Shared/Logging/Logger.h"

#include <cstring>
#include <string>

namespace
{
void CopyString(char* target, size_t targetSize, const std::string& source)
{
    if (targetSize == 0)
    {
        return;
    }

    std::strncpy(target, source.c_str(), targetSize - 1);
    target[targetSize - 1] = '\0';
}

void SendLoginFailure(Session& session, const std::string& message)
{
    LoginResBody res{};
    res.result = 1;
    CopyString(res.message, sizeof(res.message), message);
    session.SendPacket(OP_LOGIN_RES,
                       reinterpret_cast<const char*>(&res),
                       static_cast<uint16_t>(sizeof(LoginResBody)));
}

void SendRegisterFailure(Session& session, const std::string& message)
{
    RegisterResBody res{};
    res.result = 1;
    CopyString(res.message, sizeof(res.message), message);
    session.SendPacket(OP_REGISTER_RES,
                       reinterpret_cast<const char*>(&res),
                       static_cast<uint16_t>(sizeof(RegisterResBody)));
}
}

void PacketHandler::Handle(Session& session, uint16_t opcode,
                           const char* body, uint16_t bodySize)
{
    switch (opcode)
    {
    case OP_REGISTER_REQ:
        OnRegisterReq(session, body, bodySize);
        break;
    case OP_LOGIN_REQ:
        OnLoginReq(session, body, bodySize);
        break;
    case OP_SOCIAL_LOGIN_REQ:
        OnSocialLoginReq(session, body, bodySize);
        break;
    case OP_VALIDATE_GAME_TOKEN_REQ:
        OnValidateGameTokenReq(session, body, bodySize);
        break;
    case OP_MATCH_RESULT_REQ:
        OnMatchResultReq(session, body, bodySize);
        break;
    case OP_PLAYER_STATS_REQ:
        OnPlayerStatsReq(session, body, bodySize);
        break;
    default:
        Logger::Write(LogLevel::Warning, "packet", "unknown opcode received");
        break;
    }
}

void PacketHandler::OnRegisterReq(Session& session, const char* body, uint16_t bodySize)
{
    if (bodySize < sizeof(RegisterReqBody))
    {
        SendRegisterFailure(session, "invalid register request size");
        return;
    }

    RegisterReqBody req{};
    std::memcpy(&req, body, sizeof(RegisterReqBody));
    req.email[31] = '\0';
    req.password[31] = '\0';
    req.nickname[31] = '\0';

    RegisterRequest registerRequest;
    registerRequest.Email = req.email;
    registerRequest.Password = req.password;
    registerRequest.Nickname = req.nickname;

    auto result = ServerRuntime::Get().GetAuthService().RegisterLocalAccount(registerRequest);
    if (!result.Success)
    {
        SendRegisterFailure(session, result.Error.Message);
        return;
    }

    RegisterResBody res{};
    res.result = 0;
    res.user_id = result.Value.User.UserId;
    CopyString(res.message, sizeof(res.message), "register ok");
    CopyString(res.game_session_token, sizeof(res.game_session_token), result.Value.Tokens.GameSessionToken);
    session.SendPacket(OP_REGISTER_RES,
                       reinterpret_cast<const char*>(&res),
                       static_cast<uint16_t>(sizeof(RegisterResBody)));
}

void PacketHandler::OnLoginReq(Session& session, const char* body, uint16_t bodySize)
{
    if (bodySize < sizeof(LoginReqBody))
    {
        SendLoginFailure(session, "invalid login request size");
        return;
    }

    LoginReqBody req{};
    std::memcpy(&req, body, sizeof(LoginReqBody));
    req.username[31] = '\0';
    req.password[31] = '\0';

    LocalLoginRequest loginRequest;
    loginRequest.Email = req.username;
    loginRequest.Password = req.password;

    auto result = ServerRuntime::Get().GetAuthService().LoginWithLocalAccount(loginRequest);
    if (!result.Success)
    {
        SendLoginFailure(session, result.Error.Message);
        return;
    }

    LoginResBody res{};
    res.result = 0;
    res.user_id = result.Value.User.UserId;
    res.provider = static_cast<uint8_t>(result.Value.User.Provider);
    CopyString(res.message, sizeof(res.message), "local login ok");
    CopyString(res.access_token, sizeof(res.access_token), result.Value.Tokens.AccessToken);
    CopyString(res.refresh_token, sizeof(res.refresh_token), result.Value.Tokens.RefreshToken);
    CopyString(res.game_session_token, sizeof(res.game_session_token), result.Value.Tokens.GameSessionToken);

    session.SendPacket(OP_LOGIN_RES,
                       reinterpret_cast<const char*>(&res),
                       static_cast<uint16_t>(sizeof(LoginResBody)));
}

void PacketHandler::OnSocialLoginReq(Session& session, const char* body, uint16_t bodySize)
{
    if (bodySize < sizeof(SocialLoginReqBody))
    {
        SendLoginFailure(session, "invalid social login request size");
        return;
    }

    SocialLoginReqBody req{};
    std::memcpy(&req, body, sizeof(SocialLoginReqBody));
    req.provider[15] = '\0';
    req.provider_token[63] = '\0';

    SocialLoginRequest loginRequest;
    loginRequest.ProviderName = req.provider;
    loginRequest.ProviderAccessToken = req.provider_token;

    auto result = ServerRuntime::Get().GetAuthService().LoginWithSocialProvider(loginRequest);
    if (!result.Success)
    {
        SendLoginFailure(session, result.Error.Message);
        return;
    }

    LoginResBody res{};
    res.result = 0;
    res.user_id = result.Value.User.UserId;
    res.provider = static_cast<uint8_t>(result.Value.User.Provider);
    CopyString(res.message, sizeof(res.message), "social login ok");
    CopyString(res.access_token, sizeof(res.access_token), result.Value.Tokens.AccessToken);
    CopyString(res.refresh_token, sizeof(res.refresh_token), result.Value.Tokens.RefreshToken);
    CopyString(res.game_session_token, sizeof(res.game_session_token), result.Value.Tokens.GameSessionToken);

    session.SendPacket(OP_LOGIN_RES,
                       reinterpret_cast<const char*>(&res),
                       static_cast<uint16_t>(sizeof(LoginResBody)));
}

void PacketHandler::OnValidateGameTokenReq(Session& session, const char* body, uint16_t bodySize)
{
    ValidateGameTokenResBody res{};

    if (bodySize < sizeof(ValidateGameTokenReqBody))
    {
        res.result = 1;
        CopyString(res.message, sizeof(res.message), "invalid validation request size");
    }
    else
    {
        ValidateGameTokenReqBody req{};
        std::memcpy(&req, body, sizeof(ValidateGameTokenReqBody));
        req.game_session_token[63] = '\0';

        auto result = ServerRuntime::Get().GetAuthService().ValidateGameSessionToken(req.game_session_token);
        if (!result.Success)
        {
            res.result = 1;
            CopyString(res.message, sizeof(res.message), result.Error.Message);
        }
        else
        {
            res.result = 0;
            res.user_id = result.Value.UserId;
            CopyString(res.message, sizeof(res.message), "game session token valid");
        }
    }

    session.SendPacket(OP_VALIDATE_GAME_TOKEN_RES,
                       reinterpret_cast<const char*>(&res),
                       static_cast<uint16_t>(sizeof(ValidateGameTokenResBody)));
}

void PacketHandler::OnMatchResultReq(Session& session, const char* body, uint16_t bodySize)
{
    MatchResultResBody res{};

    if (bodySize < sizeof(MatchResultReqBody))
    {
        res.result = 1;
        CopyString(res.message, sizeof(res.message), "invalid match result request size");
    }
    else
    {
        MatchResultReqBody req{};
        std::memcpy(&req, body, sizeof(MatchResultReqBody));
        req.match_id[31] = '\0';
        req.winner_team[15] = '\0';

        PersistMatchResultRequest persistRequest;
        persistRequest.MatchId = req.match_id;
        persistRequest.WinnerTeam = req.winner_team;
        persistRequest.PlayerCount = req.player_count;

        for (int index = 0; index < req.player_count && index < 4; ++index)
        {
            req.players[index].team[15] = '\0';
            req.players[index].character_name[31] = '\0';
            req.players[index].result[7] = '\0';

            MatchPlayerResult player;
            player.UserId = req.players[index].user_id;
            player.Team = req.players[index].team;
            player.CharacterName = req.players[index].character_name;
            player.Kills = req.players[index].kills;
            player.Deaths = req.players[index].deaths;
            player.Assists = req.players[index].assists;
            player.DamageDealt = req.players[index].damage_dealt;
            player.DamageTaken = req.players[index].damage_taken;
            player.Score = req.players[index].score;
            player.Result = req.players[index].result;
            persistRequest.Players.push_back(player);
        }

        auto persistResult = ServerRuntime::Get().GetMatchResultDispatcher().PersistDedicatedMatchResult(persistRequest);
        if (!persistResult.Success)
        {
            res.result = 1;
            CopyString(res.message, sizeof(res.message), persistResult.Error.Message);
        }
        else
        {
            res.result = 0;
            CopyString(res.message, sizeof(res.message), "match persisted");
        }
    }

    session.SendPacket(OP_MATCH_RESULT_RES,
                       reinterpret_cast<const char*>(&res),
                       static_cast<uint16_t>(sizeof(MatchResultResBody)));
}

void PacketHandler::OnPlayerStatsReq(Session& session, const char* body, uint16_t bodySize)
{
    PlayerStatsResBody res{};

    if (bodySize < sizeof(PlayerStatsReqBody))
    {
        res.result = 1;
        CopyString(res.message, sizeof(res.message), "invalid player stats request size");
    }
    else
    {
        PlayerStatsReqBody req{};
        std::memcpy(&req, body, sizeof(PlayerStatsReqBody));

        auto statsResult = MatchRepository{}.GetAggregateStats(req.user_id);
        if (!statsResult.Success)
        {
            res.result = 1;
            CopyString(res.message, sizeof(res.message), statsResult.Error.Message);
        }
        else
        {
            res.result = 0;
            res.user_id = statsResult.Value.UserId;
            res.total_matches = statsResult.Value.TotalMatches;
            res.total_wins = statsResult.Value.TotalWins;
            res.total_kills = statsResult.Value.TotalKills;
            res.total_deaths = statsResult.Value.TotalDeaths;
            res.total_assists = statsResult.Value.TotalAssists;
            res.total_damage_dealt = statsResult.Value.TotalDamageDealt;
            CopyString(res.message, sizeof(res.message), "player stats ok");
        }
    }

    session.SendPacket(OP_PLAYER_STATS_RES,
                       reinterpret_cast<const char*>(&res),
                       static_cast<uint16_t>(sizeof(PlayerStatsResBody)));
}
