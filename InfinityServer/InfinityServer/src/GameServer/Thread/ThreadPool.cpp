#include "GameServer/Thread/ThreadPool.h"
#include "Shared/Logging/Logger.h"

#include <string>

ThreadPool::ThreadPool(int workerCount)
    : m_stop(false)
{
    for (int i = 0; i < workerCount; ++i)
        m_workers.emplace_back(&ThreadPool::WorkerLoop, this);

    Logger::Write(LogLevel::Info, "thread-pool",
                  "워커 스레드 " + std::to_string(workerCount) + "개 시작");
}

// ─────────────────────────────────────────────────────
//  소멸자 — 큐에 남은 작업 모두 처리 후 join
// ─────────────────────────────────────────────────────
ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_stop = true;
    }
    m_condition.notify_all();  // 대기 중인 모든 워커 깨우기

    for (auto& t : m_workers)
        if (t.joinable()) t.join();
}

// ─────────────────────────────────────────────────────
//  작업 추가 — 임의 스레드에서 호출 가능
// ─────────────────────────────────────────────────────
void ThreadPool::Enqueue(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        if (m_stop) return;  // 종료 중에는 새 작업 거부
        m_taskQueue.push(std::move(task));
    }
    m_condition.notify_one();  // 대기 중인 워커 하나 깨우기
}

int ThreadPool::GetQueueSize() const
{
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_queueMutex));
    return static_cast<int>(m_taskQueue.size());
}

// ─────────────────────────────────────────────────────
//  워커 루프 — 큐에서 작업을 꺼내 실행
//
//  condition_variable 사용으로 빈 큐에서 바쁜 대기(busy-wait) 방지
//  → 작업이 없으면 스레드가 sleep 상태로 CPU 낭비 없음
// ─────────────────────────────────────────────────────
void ThreadPool::WorkerLoop()
{
    while (true)
    {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            // 작업이 생기거나 종료 신호가 올 때까지 대기
            m_condition.wait(lock, [this] {
                return m_stop || !m_taskQueue.empty();
            });

            if (m_stop && m_taskQueue.empty())
                return;  // 종료 + 큐 비어있으면 스레드 종료

            task = std::move(m_taskQueue.front());
            m_taskQueue.pop();
        }

        task();  // 뮤텍스 밖에서 실행 — 다른 스레드가 계속 큐에 추가 가능
    }
}
