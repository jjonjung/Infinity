#pragma once

#include <atomic>
#include <thread>

// ─────────────────────────────────────────────────────
//  [게임 서버] SpinLock — 경량 스핀락
//
//  언제 사용:
//    - 락 보유 시간이 극히 짧은 경우 (수십 나노초)
//    - OS 컨텍스트 전환 비용을 피하고 싶을 때
//    - 예) 이동 입력 큐 push/pop, 틱 카운터 갱신
//
//  주의:
//    - 락 보유 시간이 길면 CPU를 낭비 (다른 스레드 차단)
//    - 재진입 불가 (동일 스레드가 두 번 lock() 하면 데드락)
//    - 우선순위 역전 문제가 있는 환경에서는 std::mutex 선호
//
//  PAUSE 명령어 (_mm_pause / yield):
//    스핀 대기 중 파이프라인 힌트를 주어 전력/열 절감
// ─────────────────────────────────────────────────────
class SpinLock
{
public:
    SpinLock() : m_flag(false) {}

    void lock()
    {
        while (m_flag.exchange(true, std::memory_order_acquire))
            std::this_thread::yield();  // 다른 스레드에 양보 후 재시도
    }

    bool try_lock()
    {
        return !m_flag.load(std::memory_order_relaxed) &&
               !m_flag.exchange(true, std::memory_order_acquire);
    }

    void unlock()
    {
        m_flag.store(false, std::memory_order_release);
    }

private:
    std::atomic<bool> m_flag;
};

// RAII 래퍼 — std::lock_guard<SpinLock> 과 동일하게 사용 가능
class SpinLockGuard
{
public:
    explicit SpinLockGuard(SpinLock& lock) : m_lock(lock) { m_lock.lock(); }
    ~SpinLockGuard() { m_lock.unlock(); }

    SpinLockGuard(const SpinLockGuard&) = delete;
    SpinLockGuard& operator=(const SpinLockGuard&) = delete;

private:
    SpinLock& m_lock;
};
