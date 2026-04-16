#pragma once

#include "DB/DBConnector.h"
#include "GameServer/State/PlayerState.h"
#include <cstdint>
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────
//  [게임 서버] DB 트랜잭션 — 매치 결과 원자적 저장
//
//  트랜잭션이 필요한 이유:
//    매치 결과 저장 = match 레코드 + N명의 match_player + 통계 갱신
//    → 일부만 성공하면 데이터 불일치 발생
//    → 전부 성공하거나 전부 실패해야 함 (ACID 원자성)
//
//  RAII 설계:
//    - 생성자에서 BEGIN TRANSACTION
//    - Commit() 호출 시 COMMIT
//    - 소멸자에서 Commit() 없으면 자동 ROLLBACK
//    → 예외나 early return 시에도 롤백 보장
//
//  사용 예:
//    {
//        GameDBTransaction tx("match-123");
//        tx.RecordMatchResult(results);  // 여러 INSERT
//        tx.UpdatePlayerStats(results);  // UPDATE ×N
//        tx.Commit();                    // 여기서 성공 → COMMIT
//    }   // Commit() 없이 스코프 벗어나면 → ROLLBACK
// ─────────────────────────────────────────────────────

struct MatchPlayerResult
{
    int64_t     UserId       = 0;
    std::string Nickname;
    int32_t     Kills        = 0;
    int32_t     Deaths       = 0;
    int32_t     Assists      = 0;
    int32_t     DamageDealt  = 0;
    bool        IsWinner     = false;
};

class GameDBTransaction
{
public:
    explicit GameDBTransaction(const std::string& matchId);
    ~GameDBTransaction();

    // 매치 결과 기록 (match + match_player 테이블)
    bool RecordMatchResult(const std::vector<MatchPlayerResult>& results,
                           const std::string& winnerTeam);

    // 플레이어 누적 통계 갱신 (kills/deaths/etc.)
    bool UpdatePlayerStats(const std::vector<MatchPlayerResult>& results);

    // 명시적 커밋 — 호출하지 않으면 소멸자에서 롤백
    bool Commit();

    bool IsCommitted() const { return m_committed; }

private:
    std::string m_matchId;
    bool        m_committed = false;
    bool        m_active    = false;
};
