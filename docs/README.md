
# InfinityFighter

  버블파이터를 오마주한 마블 히어로 배틀 시뮬레이터 프로젝트입니다.
  Unreal Engine 5.6 기반으로 멀티플레이 전투, FSM 기반 AI, 인게임 UI, 게임 모드 흐름 제어를 중심으로 설계했으며, 캐릭터
  별 전투 스킬과 물리 기반 상호작용을 구현했습니다.

  ## 포트폴리오 링크

  - 시연 영상: [InfinityFighter](https://youtu.be/wRN-vNVbVC8?si=RwqurBEoky6E92Ew)

  ## 프로젝트 개요

  - 프로젝트: 버블파이터를 오마주한 마블 히어로 배틀 슈팅 시뮬레이터
  - 목표: 멀티플레이 전투와 FSM 기반 AI를 결합한 액션 전투 경험 구현
  - 기간: 2025.09 ~ 2025.09
  - 인원: 3명
  - 기술: Unreal Engine 5.6 / C++ / Blueprint / UMG / Steam Server

  ## 기술 핵심

  - FSM 기반 AI 시스템 설계 및 구현
  - 5상태(Idle / Move / Attack / Damage / Die) 구조로 AI 상태 전환 관리
  - 3가지 이동 패턴(Chaotic / Strafing Jump / Cover + Attack) 순환 구조 구현
  - 복합 삼각함수 기반 변칙 이동 로직으로 예측하기 어려운 전투 패턴 구현
  - 게임 모드 중심의 전체 플레이 흐름 제어
  - 스폰, 리스폰, 타이머, 맵 전환까지 전투 루프 전체 관리
  - KillLog, Aim, AimEnemy, 카메라 제어 등 인게임 UI 통합
  - Delegate 기반 이벤트 전달로 UI와 전투 로직 결합도 완화
  - DataAsset 기반 밸런싱 구조로 전투 수치 외부화

  ## 역할 및 기여도

  - 총 기여도: 약 45%
  - 구현 책임: 본인 담당 영역 단독 구현

  ### 담당 범위

  - 아이언맨 구현
  - 스파이더맨 구현
  - AI 시스템 총괄
  - 인게임 시스템 구현

  ### 상세 기여

  - FSM 기반 AI 시스템 총괄
  - 움직임, 스킬, 패턴 전환을 포함한 Enemy AI 로직 설계 및 구현
  - 3가지 이동 패턴 순환 구조와 상태 기반 전투 행동 구현
  - 게임 모드 및 전투 흐름 제어
  - 스폰, 타이머, 맵 전환, 리스폰 보호 로직 구현
  - 인게임 UI 통합
  - 타이머, 킬로그, Aim/AimEnemy, 카메라 제어 UI 구성
  - 전투 정보 시각화 및 디버깅 UI 구현
  - 아이언맨, 스파이더맨 캐릭터 스킬 구현
  - AI 협동 공격 트리거 및 전투 상호작용 로직 구현

  ### 타 팀원 담당

  - 닥터 스트레인지
  - 아웃게임 시스템

 ## 폴더 구성
- 아키텍처: `/docs/architecture`
- MFC 관련 문서: `/docs/mfc`
- 알고리즘: `/docs/algorithms`
- SQL: `/docs/sql`

  ## 문제 해결

  ### 1. AI 움직임의 단조로움

  - 문제: AI가 항상 일정한 패턴으로 움직여 전투가 쉽게 예측되고 반복적으로 느껴짐
  - 해결: 4초 주기로 이동 패턴을 자동 전환하는 FSM 구조를 도입하고, Chaotic / Strafing / Cover 패턴을 순환하도록 설계

  ### 2. 직선 위주의 기계적인 이동

  - 문제: 단순 추적 중심 로직으로 인해 AI 움직임이 부자연스럽고 전투 긴장감이 낮음
  - 해결: `Sin`, `Cos` 기반 복합 삼각함수 조합으로 곡선형 변칙 이동 로직을 구현해 예측 불가능성을 강화

  ### 3. AI 시스템 확장성과 유지보수성 부족

  - 문제: 조건문 중심 구조는 패턴 추가와 튜닝이 어렵고 유지보수 비용이 높음
  - 해결: FSM 상태 분리, DataAsset 기반 수치 외부화, Delegate 기반 이벤트 구조를 적용해 확장성과 재사용성을 확보

  ## 성과

  - AI 활용 모션캡처, 애니메이션, Asset 구현
  - 물리 적용 전투 Skill 구현
  - AI 행동 패턴 20가지 이상 조합 가능 구조 확보
  - 플레이 테스트 만족도 80% 이상 기록
  - FSM 구조를 통한 유지보수성과 확장성 향상
  - 수학적 패턴 설계를 통한 전투 다양성 강화

  ## 핵심 구현 예시

  ### FSM 패턴 전환 로직

  ```cpp
  // MoveState() 내부: CurrentTime이 4초마다 패턴 인덱스를 순환
  int32 PatternIndex = static_cast<int32>(CurrentTime / 4.0f) % 3;
  CurrentPattern = static_cast<EAIMovementPattern>(PatternIndex);

  ### Chaotic Movement 로직

  void UAiBrainComponent::ExecuteChaoticMovementPattern(
      ACharacter* Character, const FVector& ToTarget, float Distance)
  {
      FVector SideDirection = FVector::CrossProduct(ToTarget, FVector::UpVector).GetSafeNormal();

      float Chaos1 = FMath::Sin(CurrentTime * 6.2f) * 1.3f;
      float Chaos2 = FMath::Cos(CurrentTime * 4.7f) * 0.9f;
      float Chaos3 = FMath::Sin(CurrentTime * 3.1f)
                   * FMath::Cos(CurrentTime * 2.3f) * 0.7f;

      FVector ChaoticDirection = BaseDirection
          + (SideDirection * Chaos1)
          + (ToTarget.GetSafeNormal() * Chaos2)
          + (FVector::CrossProduct(SideDirection, FVector::UpVector) * Chaos3);
  }

  ## 기술적 학습

  - FSM 설계 원칙과 상태 전환 구조화 경험 확보
  - 전투 AI에 수학적 변칙성과 확률적 요소를 결합하는 방식 학습
  - UI와 게임 로직을 분리하는 이벤트 기반 구조 설계 경험 축적
  - DataAsset 기반 밸런싱 파이프라인 설계 경험 확보
