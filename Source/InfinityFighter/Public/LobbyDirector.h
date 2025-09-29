
// ReSharper disable once UnrealHeaderToolError
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Utility/BattleTypes.h"
#include "LobbyDirector.generated.h"
//캐릭터 설정
UENUM(BlueprintType)
enum class ELobbyCharacterType : uint8
{
	Player UMETA(DisplayName = "Player"),
	Enemy1 UMETA(DisplayName = "Enemy 1"),
	Enemy2 UMETA(DisplayName = "Enemy 2"),

};
//팀결정
// ETeam moved to Utility/BattleTypes.h


UCLASS()
class INFINITYFIGHTER_API ALobbyDirector : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALobbyDirector();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
protected:
	// 캐릭터별 도착 체크 타이머 (0=Player,1=Enemy1,2=Enemy2)
	FTimerHandle ArrivalCheckHandles[3];

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;



	//camera 고정
public:
	UPROPERTY(EditAnywhere,Category="Camera")
	TArray<AActor*> TargetPoints; // 각 캐릭터 유형별 타겟 포인트
	
	UPROPERTY(EditAnywhere,Category="Camera")
	TArray<APawn*> Characters; // 각 캐릭터 유형별 폰 (플레이어 + 적들)

	UPROPERTY(EditAnywhere,Category="Camera")
	ACameraActor* LobbyCam = nullptr;

	UPROPERTY(EditAnywhere,Category="Camera")
	float BlendTime = 0.0f;

private:
	void ApplyViewTarget();


public:
	UPROPERTY(EditAnywhere,Category="Arrival")
	TSoftObjectPtr<AActor> LobbyCamActor;
	

public:
	// 호출 함수
	UFUNCTION(BlueprintCallable, Category="Lobby")
	void StartCharacterMoveToSeat(ELobbyCharacterType CharacterType);

private:
	void OnCharacterArrival(ELobbyCharacterType CharacterType, FVector GoalLocation);
	

private:
	// 자동보행
	void StartAutoWalkTo(APawn* Character, AActor* Seat);
	//착지타이머 (0=Player, 1=Enemy1, 2=Enemy2)
	FTimerHandle LandingCheckHandles[3];

	// 랜딩 애니 길이
	UPROPERTY(EditAnywhere, Category="Arrival|Timing")
	float LandAnimDelaySec = 1.5f;

	// 착지 감시 간격
	UPROPERTY(EditAnywhere, Category="Arrival|Timing")
	float LandCheckIntervalSec = 0.05f;

	// 캐릭터별 착지 후 이동 시작 유틸
	void StartWhenLanded(ELobbyCharacterType Who);

	// 캐시
	TWeakObjectPtr<class APlayerController> PC;
	FTimerHandle ArrivalCheckHandle;
	FTimerHandle GroundCheck;
	FTimerHandle LandAnimationDelayHandle;
	FTimerHandle EnemyArrivalCheckHandle;

	//팀설정
public:
	// UI 버튼에서 호출
	UFUNCTION(BlueprintCallable, Category="Team")
	void OnTeamSelected(ETeam Team, bool bMirrorOppositeForEnemies = true);
	UPROPERTY(VisibleAnywhere, Category="Team")
	bool bUseTeamAnchors = false;
	UFUNCTION(BlueprintCallable, Category="Team")
	void OnReadySelected();

protected:
	// 팀 앵커
	UPROPERTY(EditAnywhere, Category="Team|Anchors")
	TArray<AActor*> RedTeamAnchor;

	UPROPERTY(EditAnywhere, Category="Team|Anchors")
	TArray<AActor*> BlueTeamAnchor;

	// 인덱스별(0=Player, 1=Enemy1, 2=Enemy2) 포메이션 오프셋 (앵커의 Forward/Right 기준)
	UPROPERTY(EditAnywhere, Category="Team|Formation")
	TArray<FVector> FormationLocalOffsets = {
		FVector(100.f,   0.f,   0.f),  // Player: 앵커 앞 1m
		FVector(150.f, 100.f,   0.f),  // Enemy1: 우측 전방
		FVector(150.f,-100.f,   0.f)   // Enemy2: 좌측 전방
	};

private:
	// 선택 상태 저장
	ETeam CurrentTeam = ETeam::Red;
	bool  bMirrorOppositeForEnemiesDefault = true;

	// 내부 유틸
	bool ComputeTeamDestination(ETeam Team, int32 MemberIdx, FVector& OutNavLoc, bool bOppositeForEnemies) const;
	void StopAllAndReRoute();


public:
	//start 연출
	UPROPERTY(EditAnywhere, Category="Start")
	float StartStepForward = 600.f;     // 앞으로 몇 cm 
	

	UPROPERTY(EditAnywhere, Category="Start")
	float StartToTravelDelay = 5.f;     // 로딩 후 전환까지 대기 시간(초)

	UPROPERTY(EditAnywhere, Category="Start")
	FName NextLevelName = "InGame";   // 전환할 레벨 이름

	UPROPERTY(VisibleAnywhere, Category="Start")
	bool bTeamPlaced = false;
	
	UPROPERTY(VisibleAnywhere, Category="Start")
	int32 TeamArrivedCount = 0;
	UPROPERTY(VisibleAnywhere, Category="Start")
	bool bTeamChosen = false;
	
	
	// UI에서 호출할 함수 (Start 버튼 바인딩용)
	UFUNCTION(BlueprintCallable, Category="Start")
	void OnStartSelected();
	// 내부: 실제 레벨 전환
	void TravelToNextLevel();
};
