#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

// ─────────────────────────────────────────────────────
//  [게임 서버] ThreadPool — 워커 스레드 풀
//
//  목적:
//    - 게임 룸별 틱 처리를 전용 스레드 없이 풀에서 분산
//    - 단발성 작업(DB 저장, 통계 집계 등)을 비동기로 처리
//
//  설계:
//    - 생성자에서 workerCount개의 스레드를 미리 생성
//    - Enqueue()로 작업을 큐에 추가
//    - 워커 스레드가 큐에서 꺼내 실행
//    - 소멸자에서 모든 작업 완료 후 스레드 join
//
//  스레드-per-클라이언트 vs 스레드 풀:
//    - 스레드-per-클라이언트: 구현 간단, 접속 수 증가 시 스레드 폭증
//    - 스레드 풀: 스레드 수 제한, 컨텍스트 전환 감소, 고부하에 유리
//    - 로그인 서버는 부하 낮아 per-client, 게임 서버는 풀 사용
// ─────────────────────────────────────────────────────
class ThreadPool
{
public:
    explicit ThreadPool(int workerCount);
    ~ThreadPool();

    // 작업을 큐에 추가 — 스레드 안전
    void Enqueue(std::function<void()> task);

    int  GetWorkerCount() const { return static_cast<int>(m_workers.size()); }
    int  GetQueueSize()   const;

    // 복사/이동 금지
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    void WorkerLoop();

    std::vector<std::thread>          m_workers;
    std::queue<std::function<void()>> m_taskQueue;
    std::mutex                        m_queueMutex;
    std::condition_variable           m_condition;
    std::atomic<bool>                 m_stop;
};
