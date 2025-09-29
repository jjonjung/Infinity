// MatchStatsFunctionLibrary.h
// Builds a ceremony-ready match result from raw player stats (single-player friendly)

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Match/MatchResults.h"
#include "MatchStatsFunctionLibrary.generated.h"

UCLASS()
class INFINITYFIGHTER_API UMatchStatsFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/*
	 아주 쉬운 사용 예시 (한글)

	 1) C++에서 사용
	    {
	        TArray<FPlayerMatchStat> Stats;

	        // 각 캐릭터별로 킬/데스 채우기
	        FPlayerMatchStat A; A.DisplayName = FText::FromString(TEXT("아이언맨")); A.Kills = 3; A.Deaths = 1; Stats.Add(A);
	        FPlayerMatchStat B; B.DisplayName = FText::FromString(TEXT("스파이더맨")); B.Kills = 2; B.Deaths = 2; Stats.Add(B);

	        // 시상식용 결과 만들기 (정렬/랭크/TopKiller 자동)
	        const FMatchResult Result = UMatchStatsFunctionLibrary::BuildMatchResultForCeremony(Stats);

	        // 예) 최다 킬 이름/킬 수 사용
	        // Result.TopKillerName.ToString();
	        // Result.TopKills;
	        // 예) 순위표 순회
	        // for (const FPlayerMatchStat& S : Result.PlayerStats) { /* S.DisplayName, S.Rank, S.Kills, S.Deaths, S.KD  }
	    }

	 2) 블루프린트에서 사용
	    - FPlayerMatchStat 배열을 만든다 (DisplayName, Kills, Deaths 채움)
	    - BuildMatchResultForCeremony 노드 호출
	    - 반환된 FMatchResult에서 TopKillerName/TopKills/PlayerStats로 UI 구성
	*/
	UFUNCTION(BlueprintCallable, Category="Match|Stats")
	static FMatchResult BuildMatchResultForCeremony(const TArray<FPlayerMatchStat>& InStats);

	// 인스턴스 매니저(UManagerController)에 저장된 플레이어 목록에서
	// 자동으로 킬/데스 정보를 읽어와 결과를 빌드합니다.
	// 수동으로 배열을 만들 필요가 없는 편의 함수입니다.
	UFUNCTION(BlueprintCallable, Category="Match|Stats", meta=(WorldContext="WorldContextObject"))
	static FMatchResult BuildMatchResultFromManager(const UObject* WorldContextObject);
};
