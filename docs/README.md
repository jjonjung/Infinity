
# InfinityFighter

<div align="center">
  <img src="../docs/img/infinityMain.png" width="100%" />
  
  <h3>Strategy Racing Shooting Game Portfolio</h3>

  <a href="https://www.unrealengine.com/">
    <img src="https://img.shields.io/badge/Unreal_Engine-5.6.1-white?logo=unrealengine&logoColor=white&color=0E1128"/>
  </a>
  <a href="https://isocpp.org/">
    <img src="https://img.shields.io/badge/C++-17-blue.svg?logo=c%2B%2B"/>
  </a>
  <a href="https://partner.steamgames.com/">
    <img src="https://img.shields.io/badge/Steam-OnlineSubsystem-black?logo=steam"/>
  </a>
</div>

<br>

버블파이터를 오마주한 마블 히어로 배틀 시뮬레이터 프로젝트
- Unreal Engine 5.6 기반으로 멀티플레이 전투, FSM 기반 AI, 인게임 UI, 게임 모드 흐름 제어를 중심으로 설계
- 캐릭터별 전투 스킬과 물리 기반 상호작용을 구현

---

## ✓ Project Overview

<div align="center">

<table border="0" cellspacing="0" cellpadding="8" style="width: 100%; table-layout: fixed;">
  <tr>
    <td style="width: 20%; padding: 8px;"><strong>Project Name</strong></td>
    <td style="padding: 8px;">InfinityFighter</td>
  </tr>
  <tr>
    <td style="padding: 8px;"><strong>Duration</strong></td>
    <td style="padding: 8px;">2025.09</td>
  </tr>
  <tr>
    <td style="padding: 8px;"><strong>Team Size</strong></td>
    <td style="padding: 8px;">3명</td>
  </tr>
  <tr>
    <td style="padding: 8px;"><strong>Engine</strong></td>
    <td style="padding: 8px;">Unreal Engine 5.6</td>
  </tr>
  <tr>
    <td style="padding: 8px;"><strong>Tech</strong></td>
    <td style="padding: 8px;">C++ / FSM / Physical / UMG / Android /Steam Server</td>
  </tr>
  <tr>
    <td style="padding: 8px;"><strong>Purpose</strong></td>
    <td style="padding: 8px;">멀티플레이 전투와 FSM 기반 AI를 결합한 액션 전투 경험 구현</td>
  </tr>
</table>

</div>

---

  - 시연 영상: [InfinityFighter](https://youtu.be/wRN-vNVbVC8?si=RwqurBEoky6E92Ew)

  ## 프로젝트 개요

  - 프로젝트: 버블파이터를 오마주한 마블 히어로 배틀 슈팅 시뮬레이터
  - 목표: 멀티플레이 전투와 FSM 기반 AI를 결합한 액션 전투 경험 구현
  - 기간: 2025.09 
  - 인원: 3명
  - 기술: Unreal Engine 5.6 / C++ / Blueprint / UMG / Steam Server

  ## 기술 핵심

  - 자료구조, AI, 물리, 서버 구조까지 전반을 개선하여 성능(O(n)→O(1)), 안정성(버그 제거), 확장성(구조 분리), 플레이 경험(패턴 다양화)
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

  ## 문제 해결
---

### 자료구조 개선

<div align="center">

| 🚨 Problem | 🧠 Approach | ⚙️ Action | 📊 Result |
|:--|:--|:--|:--|
| 킬 로그 문자열 누적 구조 | 로그/통계 분리 필요<br><sub>대안: 문자열 파싱 O(n)</sub> | `TArray + TMap` 분리<br>최근 N개 제한 | ![O1](https://img.shields.io/badge/Time-O(1)-brightgreen)<br>![Memory](https://img.shields.io/badge/Memory-Fixed-blue)<br><sub>Before: 파싱 필요 → After: 즉시 조회</sub> |
| 시간 스냅샷 로직 혼합 | 책임 분리 구조 필요<br><sub>대안: 단일 함수</sub> | `PushSnapshot` 구조 분리<br>`TDeque` FIFO | ![Stable](https://img.shields.io/badge/Stability-100%25-success)<br><sub>Before: 조건 혼재 → After: 일관 구조</sub> |

</div>

---

### AI & 플레이 경험 개선

<div align="center">

| 🚨 Problem | 🧠 Approach | ⚙️ Action | 📊 Result |
|:--|:--|:--|:--|
| AI 패턴 단일 | FSM 기반 순환 필요<br><sub>대안: 랜덤 이동</sub> | 3패턴 + FSM 조합 | ![Pattern](https://img.shields.io/badge/Pattern-20%2B-blueviolet)<br>![Test](https://img.shields.io/badge/Test-80%25-success)<br><sub>Before: 반복 → After: 다양성</sub> |
| 직선 이동 AI | 수학 기반 이동 필요<br><sub>대안: NavMesh</sub> | Sin/Cos 곡선 이동 | ![Movement](https://img.shields.io/badge/Movement-Nonlinear-orange)<br><sub>Before: 단순 추적 → After: 예측 어려움</sub> |

</div>

---

### 네트워크 & 아키텍처 개선

<div align="center">

| 🚨 Problem | 🧠 Approach | ⚙️ Action | 📊 Result |
|:--|:--|:--|:--|
| 서버 단일 구조 | 역할 분리 필요<br><sub>대안: 통합 서버</sub> | Auth + Game Server 분리 | ![Architecture](https://img.shields.io/badge/Architecture-3Tier-blue)<br><sub>Before: 단일 → After: 계층 분리</sub> |
| 클라이언트 판정 구조 | 서버 권한 필요<br><sub>대안: 클라 신뢰</sub> | Dedicated Server 판정 구조 | ![Security](https://img.shields.io/badge/Security-ServerAuth-critical)<br><sub>Before: 취약 → After: 안전</sub> |

</div>

---

### 물리 & 충돌 시스템 개선

<div align="center">

| 🚨 Problem | 🧠 Approach | ⚙️ Action | 📊 Result |
|:--|:--|:--|:--|
| 충돌 판정 부정확 | 기하 알고리즘 필요<br><sub>대안: 단순 overlap</sub> | Dot / Projection / Sweep 적용 | ![Physics](https://img.shields.io/badge/Physics-Accurate-success)<br><sub>Before: 누락 → After: 안정</sub> |

</div>

---

## 핵심 성과 요약

<div align="center">

![Performance](https://img.shields.io/badge/Performance-O(n)%20→%20O(1)-brightgreen)
![AI](https://img.shields.io/badge/AI-Pattern%2020%2B-blueviolet)
![Architecture](https://img.shields.io/badge/Architecture-Server%20Authoritative-critical)
![Maintainability](https://img.shields.io/badge/Maintainability-High-success)

</div>

- 자료구조 개선 → 성능 최적화 (O(n) → O(1))  
- FSM + 수학 기반 이동 → 전투 다양성 확보  
- 서버 권한 구조 → 치트 방지 및 판정 신뢰성 확보  
- 이벤트 기반 설계 → 유지보수성과 확장성 향상  
- 물리 기반 전투 스킬 시스템 구현 (충돌, 임펄스, 상호작용 포함)
- Sin/Cos 기반 이동 로직을 통해 예측 불가능한 전투 패턴 구현
- FSM 구조 기반으로 유지보수 및 기능 확장 비용 감소
- AI 활용 모션캡처 및 애니메이션, Asset 적용  
---

## ✓ 한 줄 요약
멀티플레이 전투 시스템에서 성능, 구조, 플레이 경험을 동시에 개선한 프로젝트

