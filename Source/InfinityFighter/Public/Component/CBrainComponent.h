// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Router/ActionRouter.h"
#include "DA/EnemyAIData.h"
#include "CBrainComponent.generated.h"

UENUM(BlueprintType)
enum class EBrainMode : uint8
{
	Player,
	AI
};

class UCMoveRouter;
class UAiBrainComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INFINITYFIGHTER_API UCBrainComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCBrainComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	bool BaseMove();

	// 주기적 재탐색용 타이머
	FTimerHandle TargetRefreshHandle;

	// 재탐색 주기(초)
	UPROPERTY(EditAnywhere, Category="Brain|Target")
	float TargetRefreshInterval = 0.5f;

	UFUNCTION(BlueprintCallable, Category="Brain|Target")
	void RefreshTarget();

	
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	bool bBrainActive = false;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Brain")
	EBrainMode Mode = EBrainMode::Player;

	UPROPERTY(VisibleAnywhere, Transient, Category="AIBrain")
	TObjectPtr<UAiBrainComponent> AIBrain = nullptr;  
	
	// Player input
	void OnMoveInput(const FVector2D& Axis2D);
	void OnLookInput(const FVector2D& LookAxis);

	// AI 경로
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Brain|AI")
	TWeakObjectPtr<AActor> TargetActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Brain|AI")
	float AiMoveScale = 1.0f;

	UPROPERTY()
	UCMoveRouter* MoveRouter;
	UPROPERTY()
	class UActionRouter* ActionRouter;
	UPROPERTY()
	class UInputProxyComponent* InputProxy;

	// 플레이어 InputProxy 참조 (모든 AI가 플레이어 입력 감지용)
	UPROPERTY()
	class UInputProxyComponent* PlayerInputProxy;

	UFUNCTION()
	void RecievedIntent(FName IntentSignal);

	// 플레이어 공격 감지 함수
	UFUNCTION()
	void OnPlayerAttackInput(FName IntentSignal);
	UFUNCTION()//Isplayer 인지 체크
	bool CehckPlayerMode(); 

	// 자동 AI/Player 모드 감지
	UFUNCTION(BlueprintCallable, Category="Brain|Auto")
	void AutoDetectBrainMode();

	// 내부 타이머
	float CurrentTime = 0.0f;
	FVector KnockbackTargetPos;

	UPROPERTY()
	UCMoveRouter* Router;

	UPROPERTY(EditAnywhere, Category="Look")
	float LookYawRate  = 1.0f;
	UPROPERTY(EditAnywhere, Category="Look")
	float LookPitchRate = 1.0f;


};
