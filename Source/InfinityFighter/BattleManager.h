// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/CharacterBase.h"
#include "GameFramework/Actor.h"
#include "Utility/BattleTypes.h"
#include "BattleManager.generated.h"


// ETeam 과 FPlayerMatchInfo 는 Utility/BattleTypes.h 에 정의되어 있습니다.
UCLASS()
class INFINITYFIGHTER_API ABattleManager : public AActor
{
	GENERATED_BODY()

public:
    // 경기 중 킬/데스 이벤트를 수집하고
    // 킬 로그와 스코어보드를 갱신하는 매니저 액터
    ABattleManager();
public:
	// --- 전투 이벤트 ---
	// 캐릭터가 사망했을 때 호출 (킬/데스 집계 및 로그 추가)
	UFUNCTION(BlueprintCallable)
	void OnCharacterKilled(ACharacterBase* Killer, ACharacterBase*Victim);

	// 캐릭터가 리스폰했을 때 호출 (향후 UI 반영용 훅)
	UFUNCTION(BlueprintCallable)
	void OnCharacterRespawned(ACharacterBase* Character);

	
    // 킬피드에 한 줄 추가하고 내부 로그에 보관
    UFUNCTION(BlueprintCallable)
    void AddKillLog(const FString& KillerName, const FString& VictimName, ETeam KillerTeam, int32 KillerKills, int32 KillerDeaths);

    // 킬 로그 전부 초기화
    UFUNCTION(BlueprintCallable)
    void ClearAllKillLogs();

    UFUNCTION(BlueprintPure, Category="Battle|Query")
    const TArray<FString>& GetRecentKillLogs() const { return KillLogs; }

    UFUNCTION(BlueprintPure, Category="Battle|Query")
    int32 GetHeroKillCount(FName HeroNickname) const;

	
	// 스코어보드 UI 갱신 트리거 (BP에서 구현 권장)
	UFUNCTION(BlueprintCallable)
	void UpdateScoreboard();

	
    // GameInstance(ManagerController)에 킬/데스 결과 보고
    UFUNCTION(BlueprintCallable)
    void ReportKillToGameInstance(ACharacter* Killer, ACharacter* Victim);

public:
    // KillLog UI listeners can bind here
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKillLogAdded, const FKillLogEvent&, Event);

    UPROPERTY(BlueprintAssignable, Category="Battle|Events")
    FOnKillLogAdded OnKillLogAdded;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    void TrimKillLogs();

    // 최근 킬 로그 문자열 목록 (간단한 킬피드 용)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Battle", meta=(AllowPrivateAccess="true"))
    TArray<FString> KillLogs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Battle", meta=(AllowPrivateAccess="true"))
    TMap<FName, int32> HeroKillCounts;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Battle", meta=(AllowPrivateAccess="true", ClampMin="1"))
    int32 MaxKillLogs = 5;
};
