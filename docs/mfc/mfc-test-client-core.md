# MFC 테스트 클라이언트 코어

## 목적
실제 UI는 MFC 다이얼로그 프로젝트로 만들 수 있지만, 소켓 코드를 버튼 핸들러에 직접 넣기보다는 재사용 가능한 C++ 코어를 호출하는 구조가 더 적절합니다.

## 구현된 핵심 클래스
- `TestClientConnection`
  - Raw TCP 연결, 송신, 수신 담당
- `TestClientService`
  - 회원가입
  - 로컬 로그인
  - Google 로그인
  - Steam 로그인
  - 게임 세션 토큰 검증
  - 매치 결과 제출
  - 플레이어 누적 통계 조회
- `TestScenarioRunner`
  - MFC QA 버튼용 연속 회귀 테스트 흐름 담당

## 권장 MFC 연결 방식
- `Connect` 버튼 -> `TestClientService::Connect`
- `Register` 버튼 -> `TestClientService::RegisterLocal`
- `Login` 버튼 -> `TestClientService::LoginLocal`
- `Google Login` 버튼 -> `TestClientService::LoginSocial`
- `Steam Login` 버튼 -> `TestClientService::LoginSocial`
- `Validate Token` 버튼 -> `TestClientService::ValidateGameSession`
- `Submit Match` 버튼 -> `TestClientService::SubmitMatchResult`
- `Fetch Stats` 버튼 -> `TestClientService::QueryPlayerStats`
- `Run Regression` 버튼 -> `TestScenarioRunner::RunDefaultRegression`

## 이 구조가 더 나은 이유
- MFC UI 코드는 화면 입력과 출력에만 집중할 수 있습니다.
- 네트워크 처리와 프로토콜 로직을 별도 서비스 계층으로 분리할 수 있습니다.
- 동일한 코어를 유지한 채 MFC 화면만 바꾸거나 테스트 흐름을 확장하기 쉬워집니다.
