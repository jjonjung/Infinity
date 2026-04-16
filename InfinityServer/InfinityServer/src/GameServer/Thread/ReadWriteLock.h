#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

// ─────────────────────────────────────────────────────
//  [게임 서버] ReadWriteLock — SRWLOCK 래퍼
//
//  언제 사용:
//    읽기 → 다수의 스레드가 동시에 접근 가능
//    쓰기 → 단독 접근, 모든 읽기/쓰기 차단
//
//  게임 서버 적용 예:
//    - 플레이어 상태: 틱마다 쓰기, 브로드캐스트 시 읽기
//    - 방 목록: 방 생성/제거 시 쓰기, 패킷 처리 시 읽기
//
//  성능:
//    - std::shared_mutex 대비 Windows SRWLOCK이 더 경량
//    - 커널 오브젝트 없이 유저 모드에서 대부분 처리
//
//  사용 예:
//    ReadWriteLock lock;
//    { ReadGuard  r(lock); /* 읽기 */ }
//    { WriteGuard w(lock); /* 쓰기 */ }
// ─────────────────────────────────────────────────────
class ReadWriteLock
{
public:
    ReadWriteLock()  { InitializeSRWLock(&m_srw); }
    ~ReadWriteLock() = default;

    void LockRead()    { AcquireSRWLockShared(&m_srw);    }
    void UnlockRead()  { ReleaseSRWLockShared(&m_srw);    }
    void LockWrite()   { AcquireSRWLockExclusive(&m_srw); }
    void UnlockWrite() { ReleaseSRWLockExclusive(&m_srw); }

    // std::lock_guard 호환을 위한 Shared/Exclusive 래퍼
    ReadWriteLock(const ReadWriteLock&) = delete;
    ReadWriteLock& operator=(const ReadWriteLock&) = delete;

private:
    SRWLOCK m_srw;
};

// RAII 읽기 가드
class ReadGuard
{
public:
    explicit ReadGuard(ReadWriteLock& lock) : m_lock(lock) { m_lock.LockRead(); }
    ~ReadGuard() { m_lock.UnlockRead(); }

    ReadGuard(const ReadGuard&) = delete;
    ReadGuard& operator=(const ReadGuard&) = delete;

private:
    ReadWriteLock& m_lock;
};

// RAII 쓰기 가드
class WriteGuard
{
public:
    explicit WriteGuard(ReadWriteLock& lock) : m_lock(lock) { m_lock.LockWrite(); }
    ~WriteGuard() { m_lock.UnlockWrite(); }

    WriteGuard(const WriteGuard&) = delete;
    WriteGuard& operator=(const WriteGuard&) = delete;

private:
    ReadWriteLock& m_lock;
};
