#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <thread>

// ─────────────────────────────────────────────────────
//  [게임 서버] Server Tick — 고정 틱레이트 루프
//
//  게임 로직은 "언제 실행되느냐"와 독립적이어야 함
//  → 고정 틱(20Hz = 50ms) 으로 시뮬레이션을 일정하게 유지
//
//  설계:
//    - 틱 시작 시각을 기록, 틱 처리 후 남은 시간만큼 sleep
//    - 틱 처리가 50ms를 초과하면 "틱 오버" 경고 로그
//    - m_tickCallback: 매 틱마다 호출할 게임 로직 람다
//
//  사용 예:
//    ServerTick tick(20);  // 20 Hz
//    tick.Start([&](uint32_t t, float dt) {
//        stateManager.Update(t, dt);
//        stateSync.SendDelta(t, sessions);
//    });
//    // ... 게임 종료 시
//    tick.Stop();
// ─────────────────────────────────────────────────────
class ServerTick
{
public:
    using TickCallback = std::function<void(uint32_t tick, float deltaSeconds)>;

    explicit ServerTick(int ticksPerSecond = 20);
    ~ServerTick();

    void Start(TickCallback callback);
    void Stop();

    uint32_t GetCurrentTick() const { return m_currentTick.load(); }
    bool     IsRunning()      const { return m_running.load(); }

private:
    void TickLoop();

    int               m_ticksPerSecond;
    std::atomic<bool> m_running;
    std::atomic<uint32_t> m_currentTick;
    std::thread       m_thread;
    TickCallback      m_callback;
};
