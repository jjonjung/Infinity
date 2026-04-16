#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <atomic>
#include <cstdint>

// ─────────────────────────────────────────────────────
//  [로그인 서버] 인증 전담 TCP 서버 (포트 7000)
//
//  역할:
//    - 회원가입 / 로컬 로그인 / 소셜 로그인 처리
//    - 성공 시 GameSessionToken 발급 → 클라이언트가 로비 서버 진입에 사용
//
//  아키텍처 특성:
//    - Stateless: 단일 요청-응답 후 연결 즉시 해제
//    - 동시 인증 부하가 낮으므로 스레드-per-클라이언트 모델 사용
//    - 게임/로비 서버와 포트를 분리해 서비스별 스케일링 독립성 확보
//    - 기존 AuthService / DBConnector 레이어 재사용
// ─────────────────────────────────────────────────────
class LoginServer
{
public:
    explicit LoginServer(uint16_t port);
    ~LoginServer();

    bool Init();
    void Run();
    void Stop();

private:
    static DWORD WINAPI ClientThread(LPVOID param);

    uint16_t          m_port;
    SOCKET            m_listenSocket;
    std::atomic<bool> m_running;
};
