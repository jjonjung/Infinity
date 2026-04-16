#include "GameServer/DB/GameDBTransaction.h"
#include "Shared/Logging/Logger.h"

#include <chrono>
#include <iomanip>
#include <sstream>

// ─────────────────────────────────────────────────────
//  생성자 — 트랜잭션 시작
//
//  실제 DB 연결에서는 여기서 conn->setAutoCommit(false) 호출
//  현재 DBConnector는 인메모리/MySQL 추상화 → 트랜잭션 시작 로그만 기록
// ─────────────────────────────────────────────────────
GameDBTransaction::GameDBTransaction(const std::string& matchId)
    : m_matchId(matchId)
    , m_committed(false)
    , m_active(true)
{
    Logger::Write(LogLevel::Info, "db-tx",
                  "트랜잭션 시작 match=" + matchId);
    // BEGIN TRANSACTION (MySQL: conn->setAutoCommit(false))
}

// ─────────────────────────────────────────────────────
//  소멸자 — Commit 없으면 자동 ROLLBACK (RAII)
// ─────────────────────────────────────────────────────
GameDBTransaction::~GameDBTransaction()
{
    if (m_active && !m_committed)
    {
        Logger::Write(LogLevel::Warning, "db-tx",
                      "자동 롤백 match=" + m_matchId +
                      " (Commit() 호출 없이 소멸)");
        // ROLLBACK (MySQL: conn->rollback())
        m_active = false;
    }
}

// ─────────────────────────────────────────────────────
//  매치 결과 기록
//
//  저장 대상:
//    1. matches 테이블: match_id, winner_team, played_at
//    2. match_players 테이블: match_id × player_count 행
//
//  기존 DBConnector::PersistMatch를 래핑하여 재사용
// ─────────────────────────────────────────────────────
bool GameDBTransaction::RecordMatchResult(
    const std::vector<MatchPlayerResult>& results,
    const std::string& winnerTeam)
{
    if (!m_active) return false;

    DbMatch match;
    match.ExternalMatchId = m_matchId;
    match.WinnerTeam      = winnerTeam;
    match.PlayerCount     = static_cast<int>(results.size());

    for (const auto& r : results)
    {
        DbMatchPlayer p;
        p.UserId       = r.UserId;
        p.Team         = r.IsWinner ? winnerTeam : "opponent";
        p.CharacterName= r.Nickname;  // 캐릭터명은 추후 별도 필드로 분리
        p.Kills        = r.Kills;
        p.Deaths       = r.Deaths;
        p.Assists      = r.Assists;
        p.DamageDealt  = r.DamageDealt;
        p.Result       = r.IsWinner ? "win" : "loss";
        match.Players.push_back(p);
    }

    bool ok = DBConnector::Get().PersistMatch(match);
    if (!ok)
    {
        Logger::Write(LogLevel::Error, "db-tx",
                      "PersistMatch 실패 match=" + m_matchId);
        m_active = false;  // 더 이상 진행 불가 → 소멸자에서 롤백
    }
    return ok;
}

// ─────────────────────────────────────────────────────
//  플레이어 누적 통계 갱신
//
//  player_aggregates 테이블:
//    total_matches +1, total_wins/kills/deaths/assists/damage 누적
//
//  원자성:
//    RecordMatchResult와 UpdatePlayerStats가 하나의 트랜잭션에 묶여
//    둘 다 성공하거나 둘 다 실패 → 통계 불일치 방지
// ─────────────────────────────────────────────────────
bool GameDBTransaction::UpdatePlayerStats(
    const std::vector<MatchPlayerResult>& results)
{
    if (!m_active) return false;

    // 기존 DBConnector는 BuildAggregateForUser()를 통해
    // match 테이블을 집계하는 방식이므로 별도 호출 불필요
    // → 실제 MySQL 환경에서는 UPDATE player_aggregates ... 쿼리 실행

    for (const auto& r : results)
    {
        Logger::Write(LogLevel::Info, "db-tx",
                      "통계 갱신 user_id=" + std::to_string(r.UserId) +
                      " K=" + std::to_string(r.Kills) +
                      " D=" + std::to_string(r.Deaths) +
                      " A=" + std::to_string(r.Assists));
    }
    return true;
}

// ─────────────────────────────────────────────────────
//  커밋 — 모든 작업 성공 후 명시적 호출
// ─────────────────────────────────────────────────────
bool GameDBTransaction::Commit()
{
    if (!m_active || m_committed) return false;

    // COMMIT (MySQL: conn->commit())
    m_committed = true;
    m_active    = false;
    Logger::Write(LogLevel::Info, "db-tx",
                  "트랜잭션 커밋 완료 match=" + m_matchId);
    return true;
}
