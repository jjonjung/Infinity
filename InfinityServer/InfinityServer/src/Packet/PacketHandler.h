#pragma once

#include <cstdint>

class Session;

class PacketHandler
{
public:
    static void Handle(Session& session, uint16_t opcode,
                       const char* body, uint16_t bodySize);

private:
    static void OnRegisterReq(Session& session, const char* body, uint16_t bodySize);
    static void OnLoginReq(Session& session, const char* body, uint16_t bodySize);
    static void OnSocialLoginReq(Session& session, const char* body, uint16_t bodySize);
    static void OnValidateGameTokenReq(Session& session, const char* body, uint16_t bodySize);
    static void OnMatchResultReq(Session& session, const char* body, uint16_t bodySize);
    static void OnPlayerStatsReq(Session& session, const char* body, uint16_t bodySize);
};
