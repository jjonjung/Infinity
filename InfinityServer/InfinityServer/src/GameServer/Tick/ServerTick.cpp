#include "GameServer/Tick/ServerTick.h"
#include "Shared/Logging/Logger.h"

#include <chrono>
#include <string>
#include <thread>

ServerTick::ServerTick(int ticksPerSecond)
    : m_ticksPerSecond(ticksPerSecond)
    , m_running(false)
    , m_currentTick(0)
{}

ServerTick::~ServerTick()
{
    Stop();
}

void ServerTick::Start(TickCallback callback)
{
    if (m_running.exchange(true)) return;  // 중복 시작 방지
    m_callback = std::move(callback);
    m_thread   = std::thread(&ServerTick::TickLoop, this);
}

void ServerTick::Stop()
{
    if (!m_running.exchange(false)) return;
    if (m_thread.joinable())
        m_thread.join();
}

// ─────────────────────────────────────────────────────
//  고정 틱레이트 루프
//
//  목표: 매 틱이 정확히 tickInterval_ms 마다 실행
//
//  구현:
//    1. 틱 시작 시각 기록
//    2. 콜백(게임 로직) 실행
//    3. 경과 시간 계산 → (tickInterval - 경과) 만큼 sleep
//    4. 처리 시간이 인터벌 초과 → 즉시 다음 틱 실행 + 경고
// ─────────────────────────────────────────────────────
void ServerTick::TickLoop()
{
    using Clock    = std::chrono::steady_clock;
    using Ms       = std::chrono::milliseconds;
    using Duration = std::chrono::duration<float>;

    const float deltaSeconds   = 1.f / static_cast<float>(m_ticksPerSecond);
    const int   tickInterval_ms = 1000 / m_ticksPerSecond;  // 50ms at 20Hz

    Logger::Write(LogLevel::Info, "server-tick",
                  "틱 루프 시작 — " + std::to_string(m_ticksPerSecond) +
                  " Hz (간격 " + std::to_string(tickInterval_ms) + "ms)");

    auto nextTick = Clock::now();

    while (m_running)
    {
        auto tickStart = Clock::now();
        uint32_t tick  = ++m_currentTick;

        // ── 게임 로직 실행 ──────────────────────────
        if (m_callback)
            m_callback(tick, deltaSeconds);

        // ── 다음 틱 시각까지 sleep ──────────────────
        nextTick += Ms(tickInterval_ms);
        auto now = Clock::now();

        if (now < nextTick)
        {
            std::this_thread::sleep_until(nextTick);
        }
        else
        {
            // 틱 오버 — 처리 시간이 인터벌을 초과
            auto overrun = std::chrono::duration_cast<Ms>(now - nextTick).count();
            if (overrun > tickInterval_ms)
            {
                Logger::Write(LogLevel::Warning, "server-tick",
                              "틱 오버 " + std::to_string(overrun) + "ms (tick=" +
                              std::to_string(tick) + ")");
                // 누적 지연 리셋 — 이후 틱이 연속으로 밀리는 것 방지
                nextTick = Clock::now();
            }
        }
    }

    Logger::Write(LogLevel::Info, "server-tick", "틱 루프 종료");
}
