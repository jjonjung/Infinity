#include "LoginServer/LoginPacketHandler.h"
#include "LoginServer/LoginSession.h"
#include "Bootstrap/ServerRuntime.h"
#include "Packet/Packet.h"
#include "Shared/Logging/Logger.h"

#include <cstring>

void LoginPacketHandler::Handle(LoginSession& session, uint16_t opcode,
                                const char* body, uint16_t bodySize)
{
    switch (opcode)
    {
    case OP_REGISTER_REQ:     OnRegisterReq    (session, body, bodySize); break;
    case OP_LOGIN_REQ:        OnLoginReq       (session, body, bodySize); break;
    case OP_SOCIAL_LOGIN_REQ: OnSocialLoginReq (session, body, bodySize); break;
    default:
        Logger::Write(LogLevel::Warning, "login-handler",
                      "알 수 없는 opcode: " + std::to_string(opcode));
        break;
    }
}

// ─────────────────────────────────────────────────────
//  회원가입
// ─────────────────────────────────────────────────────
void LoginPacketHandler::OnRegisterReq(LoginSession& session,
                                       const char* body, uint16_t bodySize)
{
    if (bodySize < sizeof(RegisterReqBody)) return;
    const auto* req = reinterpret_cast<const RegisterReqBody*>(body);

    RegisterRequest svcReq;
    svcReq.Email    = req->email;
    svcReq.Password = req->password;
    svcReq.Nickname = req->nickname;

    auto result = ServerRuntime::Get().GetAuthService().RegisterLocalAccount(svcReq);

    RegisterResBody res{};
    if (result.Success)
    {
        res.result  = 1;
        res.user_id = result.Value.User.UserId;
        std::strncpy(res.game_session_token,
                     result.Value.Tokens.GameSessionToken.c_str(),
                     sizeof(res.game_session_token) - 1);
        std::strncpy(res.message, "회원가입 성공", sizeof(res.message) - 1);
        Logger::Write(LogLevel::Info, "login-handler",
                      "회원가입 성공 user_id=" + std::to_string(res.user_id));
    }
    else
    {
        res.result = 0;
        std::strncpy(res.message, result.Error.Message.c_str(), sizeof(res.message) - 1);
    }

    session.SendPacket(OP_REGISTER_RES,
                       reinterpret_cast<const char*>(&res), sizeof(res));
}

// ─────────────────────────────────────────────────────
//  로컬 로그인 (이메일 + 비밀번호)
// ─────────────────────────────────────────────────────
void LoginPacketHandler::OnLoginReq(LoginSession& session,
                                    const char* body, uint16_t bodySize)
{
    if (bodySize < sizeof(LoginReqBody)) return;
    const auto* req = reinterpret_cast<const LoginReqBody*>(body);

    LocalLoginRequest svcReq;
    svcReq.Email    = req->username;  // username 필드 = email
    svcReq.Password = req->password;

    auto result = ServerRuntime::Get().GetAuthService().LoginWithLocalAccount(svcReq);

    LoginResBody res{};
    if (result.Success)
    {
        res.result  = 1;
        res.user_id = result.Value.User.UserId;
        std::strncpy(res.access_token,
                     result.Value.Tokens.AccessToken.c_str(),
                     sizeof(res.access_token) - 1);
        std::strncpy(res.refresh_token,
                     result.Value.Tokens.RefreshToken.c_str(),
                     sizeof(res.refresh_token) - 1);
        std::strncpy(res.game_session_token,
                     result.Value.Tokens.GameSessionToken.c_str(),
                     sizeof(res.game_session_token) - 1);
        std::strncpy(res.message, "로그인 성공", sizeof(res.message) - 1);
        Logger::Write(LogLevel::Info, "login-handler",
                      "로그인 성공 user_id=" + std::to_string(res.user_id));
    }
    else
    {
        res.result = 0;
        std::strncpy(res.message, result.Error.Message.c_str(), sizeof(res.message) - 1);
    }

    session.SendPacket(OP_LOGIN_RES,
                       reinterpret_cast<const char*>(&res), sizeof(res));
}

// ─────────────────────────────────────────────────────
//  소셜 로그인 (구글 / 애플 OAuth 토큰)
// ─────────────────────────────────────────────────────
void LoginPacketHandler::OnSocialLoginReq(LoginSession& session,
                                          const char* body, uint16_t bodySize)
{
    if (bodySize < sizeof(SocialLoginReqBody)) return;
    const auto* req = reinterpret_cast<const SocialLoginReqBody*>(body);

    SocialLoginRequest svcReq;
    svcReq.ProviderName        = req->provider;
    svcReq.ProviderAccessToken = req->provider_token;

    auto result = ServerRuntime::Get().GetAuthService().LoginWithSocialProvider(svcReq);

    LoginResBody res{};
    if (result.Success)
    {
        res.result  = 1;
        res.user_id = result.Value.User.UserId;
        std::strncpy(res.game_session_token,
                     result.Value.Tokens.GameSessionToken.c_str(),
                     sizeof(res.game_session_token) - 1);
        std::strncpy(res.message, "소셜 로그인 성공", sizeof(res.message) - 1);
        Logger::Write(LogLevel::Info, "login-handler",
                      "소셜 로그인 성공 provider=" + std::string(req->provider) +
                      " user_id=" + std::to_string(res.user_id));
    }
    else
    {
        res.result = 0;
        std::strncpy(res.message, result.Error.Message.c_str(), sizeof(res.message) - 1);
    }

    session.SendPacket(OP_LOGIN_RES,
                       reinterpret_cast<const char*>(&res), sizeof(res));
}
