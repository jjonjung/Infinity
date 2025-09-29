// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DA/ActionRouterDA.h"
#include "DA/ActionStat.h"
#include "GameFramework/Character.h"
#include "Component/InputProxyComponent.h"
#include "Router/CMoveRouter.h"
#include "TimerManager.h" // FTimerHandle을 위해 추가
#include "InfinityFightersGameModeBase.h" // 게임 모드와의 통신을 위해 추가
#include "EnhancedInputComponent.h"
#include "Utility/BattleTypes.h"


#include "CharacterBase.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamaged, float, percent);
UCLASS(Blueprintable, BlueprintType)
class INFINITYFIGHTER_API ACharacterBase : public ACharacter
{
public:
	

private:
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBase();
	/*케릭터 베이스는 몸통 역할
	 *소유해야하는 것
	 *1. 브레인 //PlayerBrain Clsas
	 *2.적이라면 //AIBrainClass
	 *3.액션을 시행하기 위한 라우터 // 두뇌에서 보낸 신호를 라우터가 받고 라우터가 중계 역할로 각자 역할에 맞게 몸통을 움직이게시킴 
	 *4. 액션 컴포넌트(액션 매니저) 
	 *5. 전투스텟 정보 액션 컴포넌트
	 *6. 무브먼트 세팅값 컴포넌트 
	 *7.체력관련 컴포넌트
	 *8.카메라스프링암 컴포넌트
	 *9.카메라 컴포넌트 
	 *10.데이터 에셋 모음(액션, 전투 , 움직임)
	 */

	
#pragma  region 필수 컴포넌트

	// ========== 필수 부품 ==========
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Brain")
	//UPlayerBrainComponent* PlayerBrain; => 플레이어 브레인

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Brain")
	//UAIBrainComponent* AIBrain; => Ai 브레인

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Action")
	class UActionRouter* ActionRouter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Action")
	class UActionComponent* ActionComponent;
#pragma  endregion
#pragma  region 스텟/설정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
	class UCharacterActionStatComponent* ActionStatComp;

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config|Loadout")
	class UActionLoadOutDA* ActionLoadOutDA;
	//
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config|Stats")
	class UActionStat* ActionStatDA;
	//
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Config|Router")
	class UActionRouterDA* ActonRouterDA;

#pragma  endregion
#pragma region 카메라
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	class USpringArmComponent* _SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	class UCameraComponent* Camera;

	FRotator OriginalMeshRotation;
	
#pragma endregion

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="DamagedMontage")
	UAnimMontage* DamagedMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Death")
	UAnimMontage* DeathMontage = nullptr;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	UFUNCTION(BlueprintCallable, Category="Brain")
	void RefreshBrainActivation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="fly")
	class UStaticMeshComponent* FlyingObject; 
	
public:
	
	UFUNCTION(BlueprintCallable, Category="Action")
	void InitActionsFromLoadout();// 이건 리스폰할때 필요없음 처음 생성 될때만
	UFUNCTION(BlueprintCallable, Category="Action")
	void InitStatsFromDA(); // 추후 리스폰해줄때 써줄 수도 있음!
	UFUNCTION(BlueprintCallable, Category="Action")
	void InitActionRouterMapFromDa(); // 추후 리스폰해줄때 써줄 수도 있음!
	UFUNCTION(BlueprintCallable)
	void PlayActionMontage(UAnimMontage* montage);
	//리스폰 버블
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* RespawnProtectionSphere;

	// 벽 붙기 관련 카메라/메시 제어 함수들
	UFUNCTION(BlueprintCallable, Category="Wall")
	void SetWallClimbMode(bool bInWall);

	UFUNCTION(BlueprintCallable, Category="Wall")
	void EnableCameraRotationOnly(); 

	UFUNCTION(BlueprintCallable, Category="Wall") 
	void DisableCameraRotationOnly(); 

    UFUNCTION(BlueprintCallable, Category="Damage") 
    void TakeDamage(float Amount, AActor* Causer);

    // Match identity for killfeed/scoreboard
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Match")
    FName MatchNickname = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Match")
    ETeam MatchTeam = ETeam::None;
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly, Category="Damage")
	FOnDamaged OnDamaged;

private:
	// 벽 붙기 상태 저장
	bool bIsInWallMode = false;
	bool bIsRunningMode = false;
	bool bIsDead = false;
	bool bRespawnRequested = false;
	
	bool bOriginalUsePawnControlRotation = false;
	bool bOriginalOrientRotationToMovement = false;

	FTimerHandle RespawnTimerHandle;

	UFUNCTION()
	void HandleDeath(AActor* DeadActor, AActor* KillerActor);

	UFUNCTION()
	void HandleDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void RequestRespawn();

public:	
	//Move
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UInputProxyComponent* InputProxy;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UCBrainComponent* Brain;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UCMoveRouter* MoveRouter;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;
	UFUNCTION()
	void ExitWallMode();
	UFUNCTION()
	bool GetIsRunning();
	UFUNCTION()
	void SetIsRunning(bool setRunMode);

	//UI
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<class UUserWidget> crosshairUIFactory;
	// UI 인스턴스
	UPROPERTY(EditDefaultsOnly, Category="UI")
	UUserWidget* crosshairUI;
	// AimEnemy UI 위젯 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
	class UWidgetComponent* AimEnemyWidget;  
	
	// AimEnemy 위젯 관련 함수들
	UFUNCTION(BlueprintCallable, Category="UI")
	void SetupAimEnemyWidget();

	UFUNCTION(BlueprintCallable, Category="UI")
	void ShowAimEnemyWidget(bool bShow);

	// 플레이어 시야각 체크 함수들
	UFUNCTION(BlueprintCallable, Category="UI")
	void UpdateAimEnemyWidgetVisibility();

	UFUNCTION(BlueprintCallable, Category="UI")
	bool IsInPlayerFieldOfView(ACharacterBase* PlayerCharacter) const;

	UFUNCTION(BlueprintCallable, Category="UI")
	ACharacterBase* GetNearestPlayerCharacter() const;

	// BP_AimEnemy 위젯 클래스 참조
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TSubclassOf<class UUserWidget> AimEnemyWidgetClass;

	// 시야각 설정 변수들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|FieldOfView")
	float MaxDetectionRange = 2000.0f; // 최대 감지 거리

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|FieldOfView")
	float FieldOfViewAngle = 120.0f; // 시야각 (도)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI|FieldOfView")
	float UpdateFrequency = 0.1f; // 업데이트 주기 (초)


private:
	// 위젯 업데이트 타이머
	FTimerHandle WidgetUpdateTimerHandle;
	

};
