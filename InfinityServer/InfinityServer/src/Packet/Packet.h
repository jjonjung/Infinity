#pragma once

#include <cstdint>

#pragma pack(push, 1)

struct PacketHeader
{
    uint16_t body_size;
    uint16_t opcode;
};

constexpr uint16_t OP_LOGIN_REQ = 0x0001;
constexpr uint16_t OP_LOGIN_RES = 0x0002;
constexpr uint16_t OP_SOCIAL_LOGIN_REQ = 0x0003;
constexpr uint16_t OP_VALIDATE_GAME_TOKEN_REQ = 0x0004;
constexpr uint16_t OP_VALIDATE_GAME_TOKEN_RES = 0x0005;
constexpr uint16_t OP_REGISTER_REQ = 0x0006;
constexpr uint16_t OP_REGISTER_RES = 0x0007;
constexpr uint16_t OP_MATCH_RESULT_REQ = 0x0008;
constexpr uint16_t OP_MATCH_RESULT_RES = 0x0009;
constexpr uint16_t OP_PLAYER_STATS_REQ = 0x000A;
constexpr uint16_t OP_PLAYER_STATS_RES = 0x000B;
constexpr uint16_t OP_ADMIN_MONITORING_REQ = 0x000C;
constexpr uint16_t OP_ADMIN_MONITORING_RES = 0x000D;

struct LoginReqBody
{
    char username[32];
    char password[32];
};

struct RegisterReqBody
{
    char email[32];
    char password[32];
    char nickname[32];
};

struct SocialLoginReqBody
{
    char provider[16];
    char provider_token[64];
};

struct LoginResBody
{
    uint8_t result;
    int64_t user_id;
    uint8_t provider;
    char message[64];
    char access_token[64];
    char refresh_token[64];
    char game_session_token[64];
};

struct ValidateGameTokenReqBody
{
    char game_session_token[64];
};

struct ValidateGameTokenResBody
{
    uint8_t result;
    int64_t user_id;
    char message[64];
};

struct RegisterResBody
{
    uint8_t result;
    int64_t user_id;
    char message[64];
    char game_session_token[64];
};

struct MatchPlayerBody
{
    int64_t user_id;
    char team[16];
    char character_name[32];
    int32_t kills;
    int32_t deaths;
    int32_t assists;
    int32_t damage_dealt;
    int32_t damage_taken;
    int32_t score;
    char result[8];
};

struct MatchResultReqBody
{
    char match_id[32];
    char winner_team[16];
    int32_t player_count;
    MatchPlayerBody players[4];
};

struct MatchResultResBody
{
    uint8_t result;
    char message[64];
};

struct PlayerStatsReqBody
{
    int64_t user_id;
};

struct PlayerStatsResBody
{
    uint8_t result;
    int64_t user_id;
    int32_t total_matches;
    int32_t total_wins;
    int32_t total_kills;
    int32_t total_deaths;
    int32_t total_assists;
    int32_t total_damage_dealt;
    char message[64];
};

struct MonitoringNodeBody
{
    char name[32];
    uint8_t healthy;
};

struct AdminMonitoringResBody
{
    uint8_t result;
    int32_t active_match_count;
    int32_t connected_session_count;
    int32_t cached_leaderboard_entry_count;
    int32_t node_count;
    MonitoringNodeBody nodes[4];
    char message[64];
};

#pragma pack(pop)

constexpr uint32_t HEADER_SIZE = sizeof(PacketHeader);
