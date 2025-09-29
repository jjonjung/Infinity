// GameInstance-backed match player registry and stats manager

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Match/MatchResults.h"
#include "Utility/BattleTypes.h"
#include "ManagerController.generated.h"

UCLASS()
class INFINITYFIGHTER_API UManagerController : public UGameInstance
{
    GENERATED_BODY()
public:
    // 플레이어 등록/해제/초기화
    // RegisterPlayer: 닉/팀으로 등록(이미 있으면 팀 갱신)
    UFUNCTION(BlueprintCallable) void RegisterPlayer(FName Nick, ETeam Team);
    // UnregisterPlayer: 플레이어 삭제
    UFUNCTION(BlueprintCallable) void UnregisterPlayer(FName Nick);
    // ResetAllPlayers: 전체 초기화
    UFUNCTION(BlueprintCallable) void ResetAllPlayers();

    // 팀/닉 수정
    // SetTeam: 플레이어 팀 변경
    UFUNCTION(BlueprintCallable) void SetTeam(FName Nick, ETeam NewTeam);
    // RenamePlayer: 닉네임 변경(키 변경)
    UFUNCTION(BlueprintCallable) void RenamePlayer(FName OldName, FName NewName);

    // 전적 집계(킬/데스)
    // AddKillFor: Killer 킬+1, Victim 데스+1
    UFUNCTION(BlueprintCallable) void AddKillFor(FName Killer, FName Victim);
    // AddKillOnly: 특정 닉 킬+1
    UFUNCTION(BlueprintCallable) void AddKillOnly(FName Nick);
    // AddDeathOnly: 특정 닉 데스+1
    UFUNCTION(BlueprintCallable) void AddDeathOnly(FName Nick);
    // ResetStats: 특정 닉 킬/데스 0으로 초기화
    UFUNCTION(BlueprintCallable) void ResetStats(FName Nick);

    // 조회
    // GetPlayerInfo: 닉으로 정보 조회(성공 시 OutInfo 채움)
    UFUNCTION(BlueprintPure) bool GetPlayerInfo(FName Nick, FPlayerMatchInfo& OutInfo) const;
    // GetAllPlayers: 전체 목록 반환(복사본)
    UFUNCTION(BlueprintPure) TArray<FPlayerMatchInfo> GetAllPlayers() const;
    // GetKills/GetDeaths/GetTeam: 개별 값 조회
    UFUNCTION(BlueprintPure) int32 GetKills(FName Nick) const;
    UFUNCTION(BlueprintPure) int32 GetDeaths(FName Nick) const;
    UFUNCTION(BlueprintPure) ETeam GetTeam(FName Nick) const;

    // 시각화용 팀 색상 반환
    UFUNCTION(BlueprintPure) FLinearColor GetTeamColor(ETeam Team) const;

    UFUNCTION(BlueprintPure) FMatchResult GetMatchResult() const;
    UFUNCTION(BlueprintCallable) void SetMatchResult(const FMatchResult& InResult);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Match") FMatchResult CachedMatchResult;

    virtual void Init() override;
    virtual void Shutdown() override;

private:
    // 닉네임 → 매치 정보 테이블
    UPROPERTY() TMap<FName, FPlayerMatchInfo> PlayerTable;
};

