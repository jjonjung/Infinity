# InfinityServer — 구현 전체 정리

> C++ 기반 게임 서버 포트폴리오  
> 로그인 서버 / 로비 서버 / 게임 서버 3-tier 구조  
> Thread / Lock / Atomic / Authoritative Server / Dead Reckoning / DB 트랜잭션 구현

---

## 목차

1. [프로젝트 개요](#1-프로젝트-개요)
2. [디렉터리 구조](#2-디렉터리-구조)
3. [서버 아키텍처](#3-서버-아키텍처)
4. [패킷 설계](#4-패킷-설계)
5. [멀티스레드 설계](#5-멀티스레드-설계)
6. [로그인 서버](#6-로그인-서버)
7. [로비 서버](#7-로비-서버)
8. [게임 서버](#8-게임-서버)
9. [인프라 계층](#9-인프라-계층)
10. [작동 방법](#10-작동-방법)
11. [트러블슈팅](#11-트러블슈팅)

---

## 1. 프로젝트 개요

| 항목 | 내용 |
|------|------|
| 언어 | C++17 |
| 플랫폼 | Windows (Winsock2, SRWLOCK) |
| 네트워크 | TCP, 고정 헤더(4 bytes) + 바이너리 패킷 |
| DB | MySQL (mysqlcppconn8) / 인메모리 폴백 |
| 캐시 | Redis 인메모리 시뮬레이션 |
| 포트 | 로그인 7000 / 로비 8000 / 게임 9000 |

### 구현된 핵심 기술 목록

- **멀티스레드**: ThreadPool, SpinLock, ReadWriteLock (SRWLOCK), condition_variable, atomic
- **Race Condition 대응**: lock_guard, 락 분리 브로드캐스트(Snapshot 전략)
- **Authoritative Server**: 클라이언트 위치 신뢰 ❌, 방향 입력만 수신 후 서버가 속도 제한 적용
- **패킷 직렬화**: `#pragma pack(push,1)` + PacketWriter/PacketReader 템플릿
- **Server Tick**: 20Hz 고정 틱레이트, 오버런 감지 + 누적 지연 리셋
- **Dead Reckoning**: 링 버퍼 기반 위치 예측 + Lag Compensation
- **상태 동기화**: 전체 스냅샷 + DirtyFlags 기반 델타 업데이트
- **DB 트랜잭션**: RAII Commit/Rollback 패턴
- **채팅**: 로비 채팅 + 인게임 채팅 (락 분리 브로드캐스트)

---

## 2. 디렉터리 구조

```
InfinityServer/
└── src/
    ├── main.cpp                        # 진입점
    ├── Bootstrap/
    │   ├── ServerRuntime.cpp/h         # 전역 서비스 컨테이너 (싱글턴)
    ├── Network/
    │   ├── Server.cpp/h                # 기본 TCP accept 서버
    │   └── Session.cpp/h              # 기본 TCP 세션 (수신 루프)
    ├── Packet/
    │   ├── Packet.h                   # 공용 패킷 헤더 + opcode 0x0001~
    │   └── PacketHandler.cpp/h        # 공용 패킷 디스패처
    ├── LoginServer/
    │   ├── LoginServer.cpp/h          # 포트 7000 accept
    │   ├── LoginSession.cpp/h         # 로그인 세션 수신 루프
    │   └── LoginPacketHandler.cpp/h   # 회원가입/로그인/소셜로그인
    ├── LobbyServer/
    │   ├── LobbyServer.cpp/h          # 포트 8000 accept + 방 관리
    │   ├── LobbySession.cpp/h         # 로비 세션
    │   ├── LobbyRoom.cpp/h            # 방 상태 + 브로드캐스트
    │   ├── LobbyPacketDef.h           # 로비 opcode + 패킷 구조체
    │   └── LobbyPacketHandler.cpp/h   # 방 생성/입장/준비/채팅
    ├── GameServer/
    │   ├── Connection/
    │   │   ├── GameServer.cpp/h       # 포트 9000 accept + 세션 레지스트리
    │   │   └── GameSession.cpp/h      # 게임 세션 수신 루프 + 채팅
    │   ├── Packet/
    │   │   ├── GamePacketOpcodes.h    # 게임 opcode 0x0200~
    │   │   ├── GamePacketDef.h        # 게임 패킷 구조체
    │   │   └── Serializer.h           # PacketWriter / PacketReader
    │   ├── State/
    │   │   ├── GameStateManager.cpp/h # 플레이어 상태 + Authoritative 처리
    │   │   └── PlayerState.h          # 플레이어 상태 구조체
    │   ├── Sync/
    │   │   ├── MovementSync.cpp/h     # 이동 입력 → 브로드캐스트
    │   │   └── StateSync.cpp/h        # 스냅샷 / 델타 전송
    │   ├── Prediction/
    │   │   └── DeadReckoning.cpp/h    # 위치 예측 + Lag Compensation
    │   ├── Thread/
    │   │   ├── ThreadPool.cpp/h       # 워커 스레드 풀
    │   │   ├── SpinLock.h             # 경량 스핀락
    │   │   └── ReadWriteLock.h        # SRWLOCK 래퍼
    │   ├── Tick/
    │   │   └── ServerTick.cpp/h       # 고정 틱레이트 루프
    │   └── DB/
    │       └── GameDBTransaction.cpp/h # RAII 트랜잭션
    ├── Auth/
    │   └── Application/
    │       └── AuthService.cpp/h      # 회원가입/로그인/토큰 검증
    ├── DB/
    │   └── DBConnector.cpp/h          # MySQL + 인메모리 폴백
    ├── Infrastructure/
    │   ├── Cache/
    │   │   └── RedisCache.cpp/h       # Redis 인메모리 시뮬레이션
    │   └── Repositories/
    │       ├── UserRepository.cpp/h   # 유저 CRUD
    │       └── MatchRepository.cpp/h  # 매치 결과 저장
    ├── Admin/
    │   └── Application/
    │       └── AdminMonitoringService.cpp/h  # 서버 상태 모니터링
    ├── Match/
    │   └── Application/
    │       └── MatchResultDispatcher.cpp/h   # 매치 결과 처리
    └── Shared/
        ├── Config/ServerConfig.cpp/h  # 환경변수 기반 설정
        ├── Logging/Logger.cpp/h       # 콘솔 로거
        ├── Network/ISession.h         # 세션 인터페이스
        ├── Security/
        │   ├── PasswordHasher.cpp/h   # 비밀번호 해시
        │   └── TokenService.cpp/h     # 토큰 발급
        └── Types/ServiceResult.h      # Result 패턴
```

---

## 3. 서버 아키텍처

### 3-Tier 흐름

```
클라이언트
    │
    ├── [1] 로그인 서버 (포트 7000)
    │       회원가입 / 로컬 로그인 / 소셜 로그인
    │       → AccessToken + RefreshToken + GameSessionToken 발급
    │       → GameSessionToken을 Redis에 캐싱
    │
    ├── [2] 로비 서버 (포트 8000)
    │       GameSessionToken 제시 → Redis 검증
    │       방 생성 / 방 입장 / 준비 / 채팅
    │       전원 준비 완료 → MatchToken 발급 → 게임 서버 IP:포트 브로드캐스트
    │
    └── [3] 게임 서버 (포트 9000)
            MatchToken 제시 → 룸 배정
            이동 입력 → 서버 위치 계산 (Authoritative)
            상태 동기화 (스냅샷 + 델타)
            Dead Reckoning / 채팅
            매치 종료 → DB 트랜잭션 기록
```

### ServerRuntime — 전역 서비스 컨테이너

```cpp
// Bootstrap/ServerRuntime.cpp
ServerRuntime::ServerRuntime(const ServerConfig& config)
    : m_authService(m_userRepository, m_redisCache, m_tokenService)
    , m_adminMonitoringService(m_redisCache)
    , m_matchResultDispatcher(m_matchRepository, m_redisCache)
{
    m_redisCache.Connect(config.Redis.Host, config.Redis.Port);
    DBConnector::Get().Connect(config.Mysql.Host, ...);
}
```

- `ServerRuntime::Get()` — 어느 계층에서나 서비스에 접근하는 단일 진입점
- `GetAuthService()` / `GetAdminMonitoringService()` / `GetMatchResultDispatcher()`

---

## 4. 패킷 설계

### 4-1. 헤더 구조

```cpp
// Packet/Packet.h
#pragma pack(push, 1)
struct PacketHeader
{
    uint16_t body_size;   // 바디 길이 (바이트)
    uint16_t opcode;      // 패킷 종류
};
#pragma pack(pop)

constexpr uint32_t HEADER_SIZE = 4;  // 고정 4 bytes
```

**수신 원칙: Short Read 방지**
```cpp
// Network/Session.cpp — RecvAll
bool Session::RecvAll(char* buf, int size)
{
    int received = 0;
    while (received < size)
    {
        int ret = recv(m_socket, buf + received, size - received, 0);
        if (ret <= 0) return false;  // 0=연결 종료, 음수=오류
        received += ret;
    }
    return true;
}
```
TCP는 스트림 프로토콜이므로 `recv` 한 번에 요청한 바이트가 전부 오지 않을 수 있다.  
`RecvAll`이 반드시 `size` 바이트가 될 때까지 루프를 돌린다.

### 4-2. Opcode 체계

| 범위 | 서버 | 예시 |
|------|------|------|
| `0x0001~0x00FF` | 공용 (Packet.h) | 로그인, 회원가입, 매치 결과, 통계 |
| `0x0101~0x01FF` | 로비 서버 | 방 생성/입장/준비/채팅 |
| `0x0201~0x02FF` | 게임 서버 | 이동/상태/전투/채팅 |

### 4-3. 직렬화 — Serializer.h

```cpp
// 직렬화
PacketWriter w;
w.Write<uint32_t>(serverTick);
w.Write<float>(pos.x);
w.WriteStruct(body);          // packed 구조체 한 번에
session.SendPacket(opcode, w.Data(), w.Size());

// 역직렬화
PacketReader r(body, bodySize);
uint32_t tick = r.Read<uint32_t>();
float px      = r.Read<float>();
```

**원칙:**
- `#pragma pack(push,1)` 구조체는 memcpy로 직렬화 (패딩 없는 바이너리 레이아웃)
- 수치형: Little-Endian (x86/x64 기본)
- 문자열: `null-terminated char[]` (동적 길이 없음)

### 4-4. 게임 패킷 구조체 주요 항목

```cpp
// GamePacketDef.h (일부)

// 이동 입력 — 클라이언트는 위치 직접 전송 ❌, 방향만 전송 ✅
struct GameMoveInputBody
{
    uint32_t tick;     // 클라이언트 로컬 틱 (Lag Compensation용)
    float    dir_x;    // 이동 방향 벡터 (-1.0 ~ 1.0)
    float    dir_y;
    float    yaw;
    uint8_t  jump;
    uint8_t  dash;
};

// 서버 권위 위치 브로드캐스트
struct GameMoveBroadcastBody
{
    uint32_t server_tick;
    int64_t  user_id;
    float    pos_x, pos_y, pos_z;   // 서버 계산 위치
    float    vel_x, vel_y, vel_z;   // Dead Reckoning용 속도 벡터
    float    yaw;
};

// 델타 업데이트 — 변경된 필드만 전송
struct PlayerDeltaEntry
{
    int64_t  user_id;
    uint16_t dirty_flags;  // 변경 필드 비트마스크
    int32_t  hp;           // DIRTY_HP 일 때만 유효
    float    pos_x, pos_y, pos_z;  // DIRTY_POS
    uint8_t  state;        // DIRTY_STATE
};
```

---

## 5. 멀티스레드 설계

### 5-1. ThreadPool

```cpp
// GameServer/Thread/ThreadPool.h
class ThreadPool
{
    std::vector<std::thread>          m_workers;
    std::queue<std::function<void()>> m_taskQueue;
    std::mutex                        m_queueMutex;
    std::condition_variable           m_condition;
    std::atomic<bool>                 m_stop;
};
```

```cpp
// ThreadPool.cpp — WorkerLoop
void ThreadPool::WorkerLoop()
{
    while (true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            // 작업이 생기거나 종료 신호가 올 때까지 sleep (busy-wait 없음)
            m_condition.wait(lock, [this] {
                return m_stop || !m_taskQueue.empty();
            });
            if (m_stop && m_taskQueue.empty()) return;
            task = std::move(m_taskQueue.front());
            m_taskQueue.pop();
        }
        task();  // 뮤텍스 밖에서 실행 → 다른 스레드가 계속 추가 가능
    }
}
```

**핵심 설계:**
- `condition_variable::wait` — 큐가 빌 때 스레드를 sleep 상태로 전환 → CPU 낭비 없음
- 작업 실행을 `lock_guard` 밖에서 수행 → 실행 중 다른 스레드가 Enqueue 가능
- 소멸자에서 `m_stop = true` → `notify_all` → 모든 워커 `join` 순서 보장

### 5-2. SpinLock

```cpp
// GameServer/Thread/SpinLock.h
class SpinLock
{
    std::atomic<bool> m_flag;
public:
    void lock()
    {
        while (m_flag.exchange(true, std::memory_order_acquire))
            std::this_thread::yield();  // CPU 양보 후 재시도
    }
    void unlock() { m_flag.store(false, std::memory_order_release); }
};
```

**사용 기준:**
| 상황 | 선택 |
|------|------|
| 락 보유 시간이 수십 나노초 이내 | SpinLock |
| 락 보유 시간이 길거나 불확실 | std::mutex |

- `memory_order_acquire` / `release` — 컴파일러/CPU 명령어 재배치 방지
- `RAII`: `SpinLockGuard`로 예외 안전 보장

### 5-3. ReadWriteLock (SRWLOCK)

```cpp
// GameServer/Thread/ReadWriteLock.h
class ReadWriteLock
{
    SRWLOCK m_srw;
public:
    ReadWriteLock()  { InitializeSRWLock(&m_srw); }
    void LockRead()   { AcquireSRWLockShared(&m_srw); }    // 다수 동시 읽기 허용
    void LockWrite()  { AcquireSRWLockExclusive(&m_srw); } // 단독 쓰기
};

// 사용 예
{ ReadGuard  r(lock); /* 읽기: 여러 스레드 동시 접근 가능 */ }
{ WriteGuard w(lock); /* 쓰기: 모든 읽기/쓰기 차단 */       }
```

**게임 서버 적용:**
- 플레이어 상태: 틱마다 쓰기, 브로드캐스트 시 읽기 → ReadWriteLock이 적합
- 방 목록: 방 생성/제거 시 쓰기, 패킷 처리 시 읽기

**Windows SRWLOCK vs std::shared_mutex:**
- SRWLOCK은 커널 오브젝트 없이 유저 모드에서 대부분 처리 → 더 경량

### 5-4. 락 분리 브로드캐스트 전략 (Race Condition 핵심 대응)

**문제:** `m_mutex` 보유 중 `send()` 호출 → TCP 버퍼 포화 시 send() 블로킹 → 다른 스레드의 방 조작 전체 차단

**해결: 포인터 스냅샷 후 락 해제 → 송신**

```cpp
// LobbyRoom.cpp — Broadcast
void LobbyRoom::Broadcast(uint16_t opcode, const char* body, uint16_t bodySize)
{
    // 1단계: 락 보유 중 세션 포인터만 복사 (빠르게 완료)
    LobbySession* snapshot[MAX_PLAYERS] = {};
    uint8_t count = 0;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (uint8_t i = 0; i < m_playerCount; ++i)
            if (m_slots[i].Session)
                snapshot[count++] = m_slots[i].Session;
    }  // ← 락 해제

    // 2단계: 락 해제 후 송신 — send() 블로킹이 룸 조작에 영향 없음
    for (uint8_t i = 0; i < count; ++i)
        snapshot[i]->SendPacket(opcode, body, bodySize);
}
```

**동일한 전략이 적용된 곳:**
- `LobbyRoom::Broadcast` — 로비 방 브로드캐스트
- `GameServer::BroadcastToRoom` — 인게임 채팅 브로드캐스트

### 5-5. Server Tick — 고정 틱레이트 루프

```cpp
// GameServer/Tick/ServerTick.cpp
void ServerTick::TickLoop()
{
    using Clock = std::chrono::steady_clock;
    const float deltaSeconds    = 1.f / static_cast<float>(m_ticksPerSecond);  // 0.05s at 20Hz
    const int   tickInterval_ms = 1000 / m_ticksPerSecond;                     // 50ms

    auto nextTick = Clock::now();

    while (m_running)
    {
        uint32_t tick = ++m_currentTick;
        if (m_callback) m_callback(tick, deltaSeconds);  // 게임 로직 실행

        nextTick += Ms(tickInterval_ms);
        auto now = Clock::now();

        if (now < nextTick)
        {
            std::this_thread::sleep_until(nextTick);  // 정확한 틱 간격 유지
        }
        else
        {
            // 틱 오버 처리 — 누적 지연 방지
            auto overrun = std::chrono::duration_cast<Ms>(now - nextTick).count();
            if (overrun > tickInterval_ms)
                nextTick = Clock::now();  // 리셋
        }
    }
}
```

**Start/Stop 스레드 안전:**
```cpp
void ServerTick::Start(TickCallback callback)
{
    if (m_running.exchange(true)) return;  // 중복 시작 방지 (atomic CAS)
    m_thread = std::thread(&ServerTick::TickLoop, this);
}
```

---

## 6. 로그인 서버

### 6-1. 구조

```
포트 7000
    ↓
LoginServer::Run() — accept 루프
    ↓ 클라이언트 접속마다
LoginServer::ClientThread (CreateThread)
    ↓
LoginSession::Run() — 수신 루프
    ↓
LoginPacketHandler::Handle()
    ├── OP_REGISTER_REQ (0x0006) → 회원가입
    ├── OP_LOGIN_REQ    (0x0001) → 로컬 로그인
    └── OP_SOCIAL_LOGIN_REQ (0x0003) → 소셜 로그인
```

**스레드 모델: per-client (로그인 서버는 동시 접속 수가 적어 적합)**

### 6-2. 로그인 처리 흐름

```cpp
// LoginPacketHandler.cpp — OnLoginReq
void LoginPacketHandler::OnLoginReq(LoginSession& session, const char* body, uint16_t bodySize)
{
    const auto* req = reinterpret_cast<const LoginReqBody*>(body);
    LocalLoginRequest svcReq;
    svcReq.Email    = req->username;
    svcReq.Password = req->password;

    // AuthService → UserRepository → DBConnector (MySQL 또는 인메모리)
    auto result = ServerRuntime::Get().GetAuthService().LoginWithLocalAccount(svcReq);

    LoginResBody res{};
    if (result.Success)
    {
        res.result = 1;
        res.user_id = result.Value.User.UserId;
        // AccessToken, RefreshToken, GameSessionToken 모두 발급
        strncpy(res.game_session_token, result.Value.Tokens.GameSessionToken.c_str(), ...);
    }
    session.SendPacket(OP_LOGIN_RES, reinterpret_cast<const char*>(&res), sizeof(res));
}
```

### 6-3. 토큰 발급 체계

```cpp
// TokenService.cpp
std::string IssueToken(const char* prefix, int64_t userId)
{
    static std::atomic<uint64_t> counter = 1;
    return std::string(prefix) + "-" + std::to_string(userId) + "-" + std::to_string(counter++);
    // 예: "gsk-1-42"
}

std::string TokenService::IssueAccessToken(int64_t userId)      { return IssueToken("atk", userId); }
std::string TokenService::IssueRefreshToken(int64_t userId)     { return IssueToken("rtk", userId); }
std::string TokenService::IssueGameSessionToken(int64_t userId) { return IssueToken("gsk", userId); }
```

- `atomic<uint64_t>` counter — 멀티스레드 환경에서 중복 없는 토큰 발급
- 발급 후 `RedisCache::CacheUserSession()` → 로비 서버 인증 시 조회

### 6-4. DB 연결 구조 (이중 백엔드)

```cpp
// DBConnector.cpp — Connect
bool DBConnector::Connect(...)
{
#if INFINITY_HAS_MYSQLCPP          // MySQL C++ Connector 8 설치된 경우
    try {
        m_driver = sql::mysql::get_mysql_driver_instance();
        auto connection = OpenConnection();
        EnsureSchema();             // CREATE TABLE IF NOT EXISTS
        SeedDevelopmentData();      // 개발용 시드 데이터
        m_useMysqlBackend = true;
        return true;
    } catch (...) { /* 폴백 */ }
#endif

    SeedDevelopmentData();          // 인메모리로 동작
    return false;
}
```

**자동 스키마 생성 테이블:**
- `users` — 유저 기본 정보
- `user_identities` — 로그인 제공자별 아이덴티티 (로컬/구글/소셜)
- `user_sessions` — refresh token 세션
- `matches` — 매치 헤더
- `match_players` — 매치별 플레이어 결과

---

## 7. 로비 서버

### 7-1. 구조

```
포트 8000
    ↓
LobbyServer::Run() — accept 루프
    ↓ 클라이언트 접속마다
LobbyServer::ClientThread (CreateThread)
    ↓
LobbySession::Run() — 수신 루프
    ↓
LobbyPacketHandler::Handle()
    ├── OP_LOBBY_CONNECT_REQ (0x0101) → GameSessionToken 검증
    ├── OP_ROOM_LIST_REQ     (0x0103) → 방 목록
    ├── OP_ROOM_CREATE_REQ   (0x0105) → 방 생성
    ├── OP_ROOM_JOIN_REQ     (0x0107) → 방 입장
    ├── OP_ROOM_LEAVE_REQ    (0x0109) → 방 퇴장
    ├── OP_PLAYER_READY_REQ  (0x010B) → 준비 토글 + 전원 준비 시 매치 시작
    └── OP_CHAT_SEND_REQ     (0x010E) → 채팅 메시지 → LobbyRoom::Broadcast
```

### 7-2. 패킷 핸들러 — OCP 설계 (switch → 함수 포인터 맵)

```cpp
// LobbyPacketHandler.cpp
using HandlerFn = void(*)(LobbySession&, const char*, uint16_t);
static const std::unordered_map<uint16_t, HandlerFn> s_table = {
    { OP_ROOM_LIST_REQ,    OnRoomListReq    },
    { OP_ROOM_CREATE_REQ,  OnRoomCreateReq  },
    { OP_ROOM_JOIN_REQ,    OnRoomJoinReq    },
    { OP_ROOM_LEAVE_REQ,   OnRoomLeaveReq   },
    { OP_PLAYER_READY_REQ, OnPlayerReadyReq },
    { OP_CHAT_SEND_REQ,    OnChatSendReq    },  // 새 opcode 추가 = 이 테이블에만 추가
};

auto it = s_table.find(opcode);
if (it != s_table.end())
    it->second(session, body, bodySize);
```

**이전 switch 방식:** 새 opcode 추가 = `Handle()` 함수 수정 (기존 코드 변경)  
**이후 맵 방식:** 새 opcode 추가 = `s_table`에 항목 추가만 (기존 코드 불변)

### 7-3. LobbyRoom — 방 상태 관리

```cpp
// LobbyRoom.cpp — 방 생성 시
bool LobbyRoom::AddPlayer(LobbySession* session)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_started || m_playerCount >= m_maxPlayers) return false;
    m_slots[m_playerCount++] = { session, false };
    return true;
}
```

**준비 완료 → 매치 시작:**
```cpp
void LobbyPacketHandler::OnPlayerReadyReq(LobbySession& session, ...)
{
    room->SetReady(session.GetUserId(), true);

    // 준비 상태 방 전체 브로드캐스트
    PlayerReadyNtfyBody ntfy{};
    room->Broadcast(OP_PLAYER_READY_NTFY, ...);

    // 전원 준비 완료 시 게임 서버 IP:포트 + MatchToken 전송
    if (room->AllReady())
        room->StartMatch(GAME_SERVER_IP, GAME_SERVER_PORT);
}

// LobbyRoom::StartMatch
void LobbyRoom::StartMatch(const char* gameServerIp, uint16_t gameServerPort)
{
    if (m_started.exchange(true)) return;  // 중복 시작 방지 (atomic CAS)

    std::string token = GenerateMatchToken();  // 64자 랜덤 hex
    // MatchStartNtfyBody (game_server_ip, port, match_token) 브로드캐스트
    Broadcast(OP_MATCH_START_NTFY, ...);
}
```

### 7-4. MatchToken 생성 — thread_local RNG

```cpp
// LobbyRoom.cpp
std::string LobbyRoom::GenerateMatchToken() const
{
    thread_local std::mt19937_64 s_rng(
        static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count())
        ^ reinterpret_cast<uint64_t>(&s_rng));  // 스레드 주소로 시드 추가 분산

    std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(16) << dist(s_rng)  // 64자 hex 토큰
        << std::setw(16) << dist(s_rng)
        << std::setw(16) << dist(s_rng)
        << std::setw(16) << dist(s_rng);
    return oss.str();
}
```

**이전 방식:** 호출마다 mt19937_64 객체 생성 + 시드 연산 → 불필요한 오버헤드  
**이후 방식:** `thread_local`로 스레드당 1회 초기화, 이후 `next()` 만 수행

### 7-5. 로비 채팅

```cpp
// LobbyPacketHandler.cpp — OnChatSendReq
void LobbyPacketHandler::OnChatSendReq(LobbySession& session, const char* body, uint16_t bodySize)
{
    if (bodySize < sizeof(ChatSendReqBody)) return;
    LobbyRoom* room = session.GetRoom();
    if (!room) return;  // 방 미입장 방어

    const auto* req = reinterpret_cast<const ChatSendReqBody*>(body);

    ChatNtfyBody ntfy{};
    ntfy.user_id = session.GetUserId();
    strncpy(ntfy.nickname, session.GetNickname(), sizeof(ntfy.nickname) - 1);
    ntfy.nickname[sizeof(ntfy.nickname) - 1] = '\0';   // null-termination 강제
    strncpy(ntfy.message, req->message, sizeof(ntfy.message) - 1);
    ntfy.message[sizeof(ntfy.message) - 1] = '\0';

    // 락 분리 브로드캐스트 → 방 전체 전송
    room->Broadcast(OP_CHAT_NTFY, reinterpret_cast<const char*>(&ntfy), sizeof(ntfy));
}
```

---

## 8. 게임 서버

### 8-1. 구조

```
포트 9000
    ↓
GameServer::Run() — accept 루프
    ↓ 클라이언트 접속마다
GameServer::ClientThread (CreateThread)
    ↓
GameSession::Run() — 수신 루프
    ├── OP_GAME_CONNECT_REQ (0x0201) → match_token → GameRoom 배정 + 세션 등록
    ├── OP_GAME_MOVE_INPUT  (0x0205) → 이동 입력 → GameStateManager
    ├── OP_GAME_SKILL_USE_REQ (0x0207) → 스킬 사용
    ├── OP_GAME_PING        (0x020D) → Pong 즉시 응답
    └── OP_GAME_CHAT_REQ    (0x020F) → 인게임 채팅 → GameServer::BroadcastToRoom
```

### 8-2. Authoritative Server — 핵심 원리

```
[잘못된 방식]  클라이언트 → 서버에 위치 직접 전송 (pos_x, pos_y)
               → 속도 치트, 워프 해킹에 취약

[올바른 방식]  클라이언트 → 서버에 방향 입력만 전송 (dir_x, dir_y)
               → 서버가 속도 제한 적용 후 위치를 직접 계산
               → 서버가 브로드캐스트 (서버 권위 위치)
```

```cpp
// GameStateManager.cpp — ApplyMoveInput
void GameStateManager::ApplyMoveInput(int64_t userId, float dirX, float dirY,
                                      float yaw, bool jump, bool dash)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    PlayerState* p = FindPlayer(userId);
    if (!p || p->IsDead() || p->IsStunned()) return;

    float speed = dash ? MAX_MOVE_SPEED * 2.f : MAX_MOVE_SPEED;

    // 방향 벡터 정규화 후 서버가 속도 적용 → 속도 치트 차단
    float len = std::sqrtf(dirX * dirX + dirY * dirY);
    if (len > 0.001f)
    {
        p->VelX = (dirX / len) * speed;
        p->VelY = (dirY / len) * speed;
        p->StateFlags |= STATE_MOVING;
    }

    if (jump && p->PosZ <= 0.01f)  // 지상에서만 점프
    {
        p->VelZ = 500.f;
        p->StateFlags |= STATE_JUMPING;
    }

    p->Yaw = yaw;
    p->MarkDirty(DIRTY_POS | DIRTY_VEL | DIRTY_STATE);
}
```

```cpp
// GameStateManager.cpp — TickPhysics (물리 시뮬레이션)
void GameStateManager::TickPhysics(PlayerState& p, float dt)
{
    p.PosX += p.VelX * dt;
    p.PosY += p.VelY * dt;
    p.PosZ += p.VelZ * dt;

    if (p.PosZ > 0.f) p.VelZ += GRAVITY * dt;  // 중력 적용
    else { p.PosZ = 0.f; p.VelZ = 0.f; p.StateFlags &= ~STATE_JUMPING; }

    if (p.VelX != 0.f || p.VelY != 0.f || p.VelZ != 0.f)
        p.MarkDirty(DIRTY_POS);
}
```

### 8-3. 상태 동기화 — 스냅샷 + 델타

**전체 스냅샷 (5초마다 또는 접속 직후):**
```cpp
// StateSync.cpp — SendSnapshot
void StateSync::SendSnapshot(uint32_t serverTick, const vector<GameSession*>& sessions)
{
    auto players = m_stateManager.GetSnapshot();
    GameStateSnapshotBody body{};
    body.server_tick  = serverTick;
    body.player_count = players.size();

    for (uint8_t i = 0; i < body.player_count; ++i)
    {
        const PlayerState& p = players[i];
        body.players[i] = { p.UserId, p.PosX, p.PosY, p.PosZ,
                            p.VelX, p.VelY, p.VelZ, p.Yaw,
                            p.Hp, p.MaxHp, p.StateFlags, ... };
    }

    for (GameSession* sess : sessions)
        sess->SendPacket(OP_GAME_STATE_SNAPSHOT, ...);
}
```

**델타 업데이트 (매 틱, 변경된 플레이어만):**
```cpp
// StateSync.cpp — SendDelta
void StateSync::SendDelta(uint32_t serverTick, const vector<GameSession*>& sessions)
{
    auto dirtyPlayers = m_stateManager.DrainDirty();  // dirty 플래그 초기화 포함
    if (dirtyPlayers.empty()) return;  // 변경 없으면 패킷 전송 없음

    GameStateDeltaBody body{};
    body.delta_count = dirtyPlayers.size();
    for (uint8_t i = 0; i < body.delta_count; ++i)
    {
        const PlayerState& p = dirtyPlayers[i];
        PlayerDeltaEntry& d  = body.deltas[i];
        d.dirty_flags = p.DirtyFlags;
        if (p.DirtyFlags & DIRTY_HP)    d.hp    = p.Hp;
        if (p.DirtyFlags & DIRTY_STATE) d.state = p.StateFlags;
        if (p.DirtyFlags & DIRTY_POS) { d.pos_x = p.PosX; d.pos_y = p.PosY; d.pos_z = p.PosZ; }
    }
    for (GameSession* sess : sessions)
        sess->SendPacket(OP_GAME_STATE_DELTA, ...);
}
```

**DirtyFlags 체계:**
```cpp
enum GameDirtyFlag : uint16_t
{
    DIRTY_HP       = 0x0001,  // HP 변경
    DIRTY_POS      = 0x0002,  // 위치 변경
    DIRTY_VEL      = 0x0004,  // 속도 변경
    DIRTY_STATE    = 0x0008,  // 상태 플래그 변경
    DIRTY_SKILL_CD = 0x0010,  // 스킬 쿨다운 변경
};
```

### 8-4. Dead Reckoning

```
목적: 서버 패킷이 오기 전까지 클라이언트가 움직임을 끊김 없이 예측

원리: 마지막 권위 위치 + 속도 × 경과 시간 = 현재 예측 위치
```

```cpp
// DeadReckoning.cpp — Predict
PredictedPosition DeadReckoning::Predict(uint32_t currentTick, float deltaSeconds) const
{
    const PositionSnapshot* latest = LatestSnapshot();
    if (!latest) return {};

    uint32_t tickDelta = (currentTick > latest->Tick) ? (currentTick - latest->Tick) : 0;
    float dt = static_cast<float>(tickDelta) * deltaSeconds;

    PredictedPosition pred;
    pred.PosX = latest->PosX + latest->VelX * dt;  // 위치 + 속도 × 시간
    pred.PosY = latest->PosY + latest->VelY * dt;
    pred.PosZ = latest->PosZ + latest->VelZ * dt;
    if (pred.PosZ < 0.f) pred.PosZ = 0.f;  // 지면 클램프

    return pred;
}
```

**보정 필요 여부 판단:**
```cpp
bool DeadReckoning::NeedsCorrection(float actualX, float actualY, float actualZ) const
{
    // 예측 위치와 서버 권위 위치 간 거리가 임계값 초과 시 강제 보정
    float distSq = dx*dx + dy*dy + dz*dz;
    return distSq > (CORRECTION_THRESHOLD * CORRECTION_THRESHOLD);
}
```

**Lag Compensation — 과거 위치 재현:**
```cpp
// 클라이언트 tick 기준으로 그 시점의 위치를 링 버퍼에서 복원
bool DeadReckoning::GetHistoricalPosition(uint32_t tick, PositionSnapshot& out) const
{
    for (int i = 0; i < m_count; ++i)
    {
        int idx = (m_head - i + HISTORY_SIZE) % HISTORY_SIZE;
        if (m_history[idx].Tick == tick) { out = m_history[idx]; return true; }
    }
    return false;
}
```

### 8-5. 인게임 채팅

```
클라이언트 → OP_GAME_CHAT_REQ → GameSession
    → GameServer::BroadcastToRoom (락 분리 전략)
        → 룸 내 모든 GameSession::SendPacket
            → OP_GAME_CHAT_NTFY (user_id, nickname, message, server_tick)
```

**GameServer 세션 레지스트리:**
```cpp
// GameServer.cpp — BroadcastToRoom
void GameServer::BroadcastToRoom(uint32_t roomId, uint16_t opcode,
                                 const char* body, uint16_t bodySize)
{
    // 1단계: 락 보유 → 세션 포인터만 스냅샷 복사
    std::vector<GameSession*> snapshot;
    {
        std::lock_guard<std::mutex> lock(m_roomMutex);
        auto it = m_roomSessions.find(roomId);
        if (it == m_roomSessions.end()) return;
        snapshot = it->second;
    }  // ← 락 해제

    // 2단계: 락 해제 후 송신
    for (GameSession* sess : snapshot)
        if (sess) sess->SendPacket(opcode, body, bodySize);
}
```

**세션 생명주기:**
```cpp
// GameSession.cpp — 접속 성공 시
GameServer::Get().RegisterSession(m_roomId, this);

// GameSession.cpp — 소멸자
if (m_roomId != 0)
    GameServer::Get().UnregisterSession(m_roomId, this);
```

### 8-6. DB 트랜잭션 — RAII 패턴

```cpp
// GameDBTransaction.cpp
class GameDBTransaction
{
public:
    GameDBTransaction(const std::string& matchId) : m_active(true), m_committed(false)
    {
        // BEGIN TRANSACTION (conn->setAutoCommit(false))
    }

    ~GameDBTransaction()
    {
        if (m_active && !m_committed)
        {
            // Commit() 없이 소멸되면 자동 ROLLBACK
            // → 예외, 조기 return 시에도 안전 보장
        }
    }

    bool Commit()
    {
        if (!m_active || m_committed) return false;
        m_committed = true;
        m_active    = false;
        // COMMIT
        return true;
    }
};

// 사용 예
{
    GameDBTransaction tx(matchId);
    if (!tx.RecordMatchResult(results, winnerTeam)) return;  // 실패 → 소멸자에서 롤백
    if (!tx.UpdatePlayerStats(results)) return;
    tx.Commit();  // 성공 → 커밋
}  // 블록 벗어남 → 소멸자 호출 (이미 커밋했으므로 롤백 없음)
```

**원자성 보장:**  
`RecordMatchResult` + `UpdatePlayerStats` 두 작업이 하나의 트랜잭션에 묶임  
→ 둘 다 성공하거나, 둘 다 실패 (통계 불일치 방지)

---

## 9. 인프라 계층

### 9-1. Redis 캐시 (인메모리 시뮬레이션)

```cpp
// RedisCache.cpp
void RedisCache::CacheUserSession(int64_t userId,
                                  const std::string& refreshToken,
                                  const std::string& gameSessionToken)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cachedSessions[BuildSessionKey(userId)] = refreshToken + "|" + gameSessionToken;
    m_gameSessions[gameSessionToken] = userId;  // 로비 서버 인증에 사용
}

std::optional<int64_t> RedisCache::FindUserIdByGameSessionToken(const std::string& token) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto it = m_gameSessions.find(token);
    if (it == m_gameSessions.end()) return std::nullopt;
    return it->second;
}
```

**캐싱 대상:**
- `game_session_token → user_id` (로비 서버 진입 인증)
- `session:user:{userId} → refresh_token|game_session_token`
- `match:snapshot:{matchId}` (매치 스냅샷)
- `leaderboard:season:{seasonId}` (리더보드)

### 9-2. 관리자 모니터링

```cpp
// AdminMonitoringService.cpp
MonitoringSnapshot AdminMonitoringService::BuildSnapshot() const
{
    MonitoringSnapshot snapshot;
    snapshot.Nodes.push_back({ "auth-api",             true });
    snapshot.Nodes.push_back({ "dedicated-game-server", true });
    snapshot.Nodes.push_back({ "redis", m_redisCache.IsConnected() });
    snapshot.Nodes.push_back({ "mysql", DBConnector::Get().IsConnected() });
    snapshot.ActiveMatchCount          = m_redisCache.GetCachedMatchCount();
    snapshot.ConnectedSessionCount     = Session::GetActiveSessionCount();
    snapshot.CachedLeaderboardEntryCount = m_redisCache.GetLeaderboardEntryCount("current");
    return snapshot;
}
```

**`Session::GetActiveSessionCount()`:** `std::atomic<int> s_activeSessions`를 생성자/소멸자에서 증감  
→ 락 없이 현재 접속 세션 수를 O(1)로 조회

### 9-3. 서버 설정 — 환경변수 기반

```cpp
// ServerConfig.cpp
ServerConfig ServerConfigLoader::LoadDefaults()
{
    ServerConfig config;
    config.Mysql.Host     = ReadEnvOrDefault("INFINITY_MYSQL_HOST",     "localhost");
    config.Mysql.Port     = ReadEnvPortOrDefault("INFINITY_MYSQL_PORT", 3306);
    config.Mysql.Database = ReadEnvOrDefault("INFINITY_MYSQL_DATABASE", "infinity");
    config.Mysql.User     = ReadEnvOrDefault("INFINITY_MYSQL_USER",     "root");
    config.Mysql.Password = ReadEnvOrDefault("INFINITY_MYSQL_PASSWORD", "");
    return config;
}
```

**환경변수 우선, 미설정 시 기본값 사용 → 코드 수정 없이 환경별 설정 변경 가능**

---

## 10. 작동 방법

### 10-1. 빌드 환경

| 항목 | 요구사항 |
|------|----------|
| 컴파일러 | Visual Studio 2022 (MSVC) |
| 표준 | C++17 |
| 플랫폼 | Windows x64 |
| 외부 라이브러리 | MySQL C++ Connector 8 (선택) |

**MySQL Connector 없이 빌드:**
- `INFINITY_HAS_MYSQLCPP = 0` 으로 자동 처리
- 인메모리 저장소로 동작, 기능 동일

### 10-2. 환경변수 설정 (선택)

```bash
# MySQL 사용 시
set INFINITY_MYSQL_HOST=localhost
set INFINITY_MYSQL_PORT=3306
set INFINITY_MYSQL_DATABASE=infinity
set INFINITY_MYSQL_USER=root
set INFINITY_MYSQL_PASSWORD=yourpassword
```

### 10-3. 실행

```bash
# Visual Studio에서 빌드 후
InfinityServer.exe
```

**시작 시 콘솔 출력 예:**
```
[INFO][bootstrap] InfinityServer starting
[INFO][bootstrap] redis cache initialized
[WARN][bootstrap] db connector is not available       # MySQL 미설치 시
[INFO][db] initialized in-memory persistence for mysql schema 'infinity'
[INFO][login-server] 인증 서버 준비 완료 — 포트 7000
[INFO][lobby-server] 로비 서버 준비 완료 — 포트 8000
[INFO][game-server] 게임 서버 준비 완료 — 포트 9000
[INFO][thread-pool] 워커 스레드 4개 시작
[INFO][server-tick] 틱 루프 시작 — 20 Hz (간격 50ms)
```

### 10-4. 클라이언트 접속 흐름

```
Step 1 — 로그인 서버 (포트 7000)
  C→S: OP_LOGIN_REQ { username, password }
  S→C: OP_LOGIN_RES { result=1, user_id, game_session_token="gsk-1-42" }

Step 2 — 로비 서버 (포트 8000)
  C→S: OP_LOBBY_CONNECT_REQ { game_session_token="gsk-1-42" }
  S→C: OP_LOBBY_CONNECT_RES { result=1, user_id, nickname }

  C→S: OP_ROOM_CREATE_REQ { room_name="방1", max_players=4 }
  S→C: OP_ROOM_CREATE_RES { result=1, room_id=1 }

  C→S: OP_PLAYER_READY_REQ
  S→C: OP_PLAYER_READY_NTFY { is_ready=1 }
  S→C: OP_MATCH_START_NTFY { game_server_ip="127.0.0.1", port=9000, match_token="..." }

Step 3 — 게임 서버 (포트 9000)
  C→S: OP_GAME_CONNECT_REQ { match_token="..." }
  S→C: OP_GAME_CONNECT_RES { result=1, spawn_x, spawn_y }

  C→S: OP_GAME_MOVE_INPUT { dir_x=1.0, dir_y=0.0, yaw=0.0 }
  S→C: OP_GAME_MOVE_BROADCAST { server_tick, user_id, pos_x, pos_y, vel_x, vel_y }
  S→C: OP_GAME_STATE_DELTA { dirty 플레이어만 포함 }

  C→S: OP_GAME_CHAT_REQ { message="안녕" }
  S→C: OP_GAME_CHAT_NTFY { user_id, nickname, message, server_tick }
```

### 10-5. 기능별 포트 정리

| 포트 | 서버 | 주요 기능 |
|------|------|-----------|
| 7000 | 로그인 | 회원가입, 로컬/소셜 로그인, 토큰 발급 |
| 8000 | 로비 | 방 목록/생성/입장/준비/채팅, 매치 시작 |
| 9000 | 게임 | 이동/상태/전투 동기화, 채팅, 매치 결과 DB 저장 |

---

## 11. 트러블슈팅

### T-1. Broadcast 중 다른 스레드의 AddPlayer가 차단되는 문제

**현상:** `m_mutex` 보유 중 `send()` 호출 → TCP 버퍼 포화나 느린 클라이언트로 인해 send() 블로킹 → `AddPlayer`, `RemovePlayer`를 호출하는 다른 스레드 전체 차단

**원인 분석:** 락 보유 시간이 I/O 시간에 비례 → 룸 조작 처리량 급감

**해결:**
```cpp
// 이전: 락 보유 중 send()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto* sess : m_slots)
        sess->SendPacket(...);  // send()가 블로킹될 수 있음 → 락 장시간 보유
}

// 이후: 락 내에서 포인터 복사만, 락 해제 후 send()
LobbySession* snapshot[MAX_PLAYERS] = {};
uint8_t count = 0;
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (uint8_t i = 0; i < m_playerCount; ++i)
        snapshot[count++] = m_slots[i].Session;  // 포인터 복사만 수행
}  // ← 락 해제
for (uint8_t i = 0; i < count; ++i)
    snapshot[i]->SendPacket(...);  // 락 없이 send() 수행
```

**적용 위치:** `LobbyRoom::Broadcast`, `GameServer::BroadcastToRoom`

---

### T-2. 틱 누적 지연 문제 (틱 오버)

**현상:** 게임 로직 처리 시간이 틱 간격(50ms)을 초과할 경우 다음 틱이 지연 없이 연속 실행 → 누적 지연으로 시간이 갈수록 틱이 몰려서 실행

**원인 분석:** `nextTick += 50ms` 방식에서 오버런 감지 없이 계속 더하면 다음 틱의 sleep 시간이 음수가 되어 즉시 실행 반복

**해결:**
```cpp
// ServerTick.cpp
nextTick += Ms(tickInterval_ms);
auto now = Clock::now();

if (now < nextTick)
{
    std::this_thread::sleep_until(nextTick);  // 정상: 다음 틱까지 대기
}
else
{
    auto overrun = duration_cast<Ms>(now - nextTick).count();
    if (overrun > tickInterval_ms)
    {
        Logger::Write(Warning, "server-tick", "틱 오버 " + to_string(overrun) + "ms");
        nextTick = Clock::now();  // 누적 지연 리셋 — 이후 틱 정상화
    }
}
```

---

### T-3. 스레드별 RNG 시드 충돌 (MatchToken 중복 위험)

**현상:** 여러 스레드가 같은 시각에 MatchToken 생성 시 `chrono::now()` 시드가 동일 → 동일한 토큰 생성 위험

**원인 분析:** 멀티스레드 환경에서 시각 기반 시드는 충돌 가능성 존재

**해결:**
```cpp
thread_local std::mt19937_64 s_rng(
    static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count())
    ^ reinterpret_cast<uint64_t>(&s_rng));  // 스레드 고유 주소값으로 XOR → 시드 분산
```

- `thread_local` → 스레드당 1개의 RNG 인스턴스 → 스레드 간 경합 없음
- 스레드 스택 주소 XOR → 같은 시각에 생성된 스레드도 다른 시드 보장

---

### T-4. 미인증 세션에서 게임 패킷이 처리되는 문제

**현상:** 클라이언트가 `OP_GAME_CONNECT_REQ` 없이 이동 패킷을 바로 보낼 경우 처리될 수 있음

**해결:**
```cpp
// GameSession.cpp — Run()
if (header.opcode == OP_GAME_CONNECT_REQ)
{
    // 인증 처리
    m_authenticated = true;
    continue;
}

// ← 인증 전 다른 패킷은 아래에서 차단
if (!m_authenticated)
{
    Logger::Write(Warning, "game-session", "미인증 opcode: " + to_string(header.opcode));
    break;  // 연결 강제 종료
}
```

---

### T-5. GameSession 소멸 후 BroadcastToRoom에서 댕글링 포인터

**현상:** 세션이 소멸된 후 `m_roomSessions` 벡터에 해당 포인터가 남아있으면 `BroadcastToRoom` 호출 시 댕글링 포인터 접근

**해결:**
```cpp
// GameSession.cpp — 소멸자
GameSession::~GameSession()
{
    // 소멸 전 반드시 레지스트리에서 제거
    if (m_roomId != 0)
        GameServer::Get().UnregisterSession(m_roomId, this);

    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
}

// GameServer.cpp — UnregisterSession
void GameServer::UnregisterSession(uint32_t roomId, GameSession* sess)
{
    std::lock_guard<std::mutex> lock(m_roomMutex);
    auto& vec = m_roomSessions[roomId];
    vec.erase(std::remove(vec.begin(), vec.end(), sess), vec.end());
}
```

---

### T-6. GameStateManager — O(N) 플레이어 탐색 → O(1) 최적화

**현상:** 틱마다 호출되는 `ApplyMoveInput`, `ApplyDamage`에서 플레이어 벡터를 순회하여 O(N) 탐색

**해결:** `userId → 벡터 인덱스` 인덱스 맵 추가

```cpp
// GameStateManager.h
std::vector<PlayerState>           m_players;
std::unordered_map<int64_t, size_t> m_playerIndex;  // userId → 인덱스

// PlayerState* FindPlayer(int64_t userId)
PlayerState* GameStateManager::FindPlayer(int64_t userId)
{
    auto it = m_playerIndex.find(userId);
    if (it == m_playerIndex.end()) return nullptr;
    return &m_players[it->second];  // O(1)
}
```

**RemovePlayer — swap-erase 패턴 (O(1)):**
```cpp
void GameStateManager::RemovePlayer(int64_t userId)
{
    size_t idx  = m_playerIndex[userId];
    size_t last = m_players.size() - 1;

    if (idx != last)
    {
        // 마지막 원소를 빈 자리로 이동 (O(1)) — 배열 중간 삭제 O(N) 회피
        m_players[idx] = std::move(m_players[last]);
        m_playerIndex[m_players[idx].UserId] = idx;  // 이동된 원소 인덱스 갱신
    }

    m_players.pop_back();
    m_playerIndex.erase(userId);
}
```

---

### T-7. LobbyPacketHandler switch → OCP 위반 문제

**현상:** 새 opcode 추가마다 `Handle()` 함수의 switch 문을 수정 → 기존 코드를 변경하는 OCP 위반

**해결:** 함수 포인터 맵으로 전환
```cpp
// 이전: switch (모든 추가마다 Handle() 수정)
switch (opcode) {
    case OP_ROOM_LIST_REQ:   OnRoomListReq(...);   break;
    // 새 opcode 추가 시 여기에 case 추가 → 기존 코드 변경
}

// 이후: s_table에 항목 추가만 (Handle() 코드 불변)
static const unordered_map<uint16_t, HandlerFn> s_table = {
    { OP_ROOM_LIST_REQ,    OnRoomListReq    },
    { OP_CHAT_SEND_REQ,    OnChatSendReq    },  // 추가 = 이 줄만 추가
};
auto it = s_table.find(opcode);
if (it != s_table.end())
    it->second(session, body, bodySize);
```

**`static local` 초기화:** C++11 이후 thread-safe 보장 → 첫 호출 시 1회 초기화, 이후 lock-free 읽기

---

## 부록 — 핵심 코드 위치 요약

| 기능 | 파일 | 핵심 함수/클래스 |
|------|------|-----------------|
| 서버 진입점 | `src/main.cpp` | `main()` |
| 서비스 컨테이너 | `Bootstrap/ServerRuntime.cpp` | `ServerRuntime::Initialize()` |
| 패킷 헤더 | `Packet/Packet.h` | `PacketHeader`, `HEADER_SIZE` |
| 패킷 직렬화 | `GameServer/Packet/Serializer.h` | `PacketWriter`, `PacketReader` |
| 게임 패킷 구조체 | `GameServer/Packet/GamePacketDef.h` | 전체 |
| 스레드 풀 | `GameServer/Thread/ThreadPool.cpp` | `WorkerLoop()` |
| 스핀락 | `GameServer/Thread/SpinLock.h` | `SpinLock::lock()` |
| R/W 락 | `GameServer/Thread/ReadWriteLock.h` | `ReadGuard`, `WriteGuard` |
| 틱 루프 | `GameServer/Tick/ServerTick.cpp` | `TickLoop()` |
| Authoritative | `GameServer/State/GameStateManager.cpp` | `ApplyMoveInput()`, `TickPhysics()` |
| 상태 동기화 | `GameServer/Sync/StateSync.cpp` | `SendSnapshot()`, `SendDelta()` |
| Dead Reckoning | `GameServer/Prediction/DeadReckoning.cpp` | `Predict()`, `GetHistoricalPosition()` |
| 채팅 (로비) | `LobbyServer/LobbyPacketHandler.cpp` | `OnChatSendReq()` |
| 채팅 (게임) | `GameServer/Connection/GameSession.cpp` | `OP_GAME_CHAT_REQ` 처리 |
| 채팅 브로드캐스트 | `GameServer/Connection/GameServer.cpp` | `BroadcastToRoom()` |
| DB 트랜잭션 | `GameServer/DB/GameDBTransaction.cpp` | `Commit()`, `~GameDBTransaction()` |
| DB 커넥터 | `DB/DBConnector.cpp` | `Connect()`, `PersistMatch()` |
| Redis 캐시 | `Infrastructure/Cache/RedisCache.cpp` | `CacheUserSession()` |
| 인증 서비스 | `Auth/Application/AuthService.cpp` | `LoginWithLocalAccount()` |
| 토큰 발급 | `Shared/Security/TokenService.cpp` | `IssueGameSessionToken()` |
| 모니터링 | `Admin/Application/AdminMonitoringService.cpp` | `BuildSnapshot()` |
| 서버 설정 | `Shared/Config/ServerConfig.cpp` | `LoadDefaults()` |
