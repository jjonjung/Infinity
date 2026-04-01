# MFC 다이얼로그 골격

## 추가된 파일
- `MfcTestClientSkeleton/MfcTestClientApp.h`
- `MfcTestClientSkeleton/MfcTestClientApp.cpp`
- `MfcTestClientSkeleton/MfcTestClientMainDialog.h`
- `MfcTestClientSkeleton/MfcTestClientMainDialog.cpp`
- `MfcTestClientSkeleton/resource.h`

## 이 문서로 얻는 것
- MFC `CWinApp` 진입 골격
- MFC `CDialogEx` 기반 메인 다이얼로그
- `TestClientService`에 연결된 버튼 핸들러
- `TestScenarioRunner`에 연결된 회귀 테스트 버튼
- 기본 로그 창 갱신 패턴

## 다이얼로그 리소스에 필요한 컨트롤
- host 입력창
- port 입력창
- email 입력창
- password 입력창
- nickname 입력창
- provider token 입력창
- game token 입력창
- user id 입력창
- 여러 줄 로그 입력창
- connect/register/login/google/steam/validate/match/stats/regression 버튼

## 연동 메모
이 문서는 마법사로 생성된 `.rc` 프로젝트 자체가 아니라, MFC 다이얼로그 프로젝트에 넣어 사용할 수 있는 애플리케이션 및 다이얼로그 로직 계층입니다. 실제 리소스 에디터 레이아웃과 연결해서 사용하면 됩니다.
