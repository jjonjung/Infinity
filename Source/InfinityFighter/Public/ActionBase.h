// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/CharacterBase.h"
#include "UObject/NoExportTypes.h"
#include "ActionBase.generated.h"

/**
 * 
 */
UENUM()
enum class EActionState : uint8{Idle,Running,Cooldown};
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, BlueprintType)
class INFINITYFIGHTER_API UActionBase : public UObject
{
    GENERATED_BODY()
public:
	//실행 가능한지 검사(쿨다운/자원/상태 등등)!
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ActionBase")
	bool CanActivate(ACharacterBase* Owner,FString& FailReason) const;
	virtual bool CanActivate_Implementation(ACharacterBase* Owner,FString& FailReason) const;

	//스킬 실행시 소모되는 코스트를 지불하는 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ActionBase")
	bool PayCost(ACharacterBase* Owner,FString& FailReason);
	virtual bool PayCost_Implementation(ACharacterBase* Owner,FString& FailReason);


	//실제 발동(애니메이션/사운드/VFX/SFX)등을 구현해야하는 함수 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ActionBase")
	void Activate(ACharacterBase* Owner);
	virtual void Activate_Implementation(ACharacterBase* Owner);

	//정상 종료 될때 실행되는 함수 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ActionBase")
	void End(ACharacterBase* Owner);
	virtual void End_Implementation(ACharacterBase* Owner);

	// 상태이상 공격을 받아서 강제 취소되는 경우 실행되는 함수 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Action")
	void Cancel(ACharacterBase* Owner);
	virtual void Cancel_Implementation(ACharacterBase* Owner);
public:
	//현재 이 스킬을 사용중인지 안사용중인지 딱 실행 상태만 알 수 있음 쿨타임중인지 그냥 안쓰고 있는건지는 불확실
	UFUNCTION(BlueprintCallable, Category="Action|State")
	bool IsRunning()     const { return State == EActionState::Running; }
	//현재 쿨타임 상태인가를 반환
	UFUNCTION(BlueprintCallable, Category="Action|State")
	bool IsOnCooldown()  const;

	UFUNCTION(BlueprintCallable, Category="Action|State")
	EActionState GetState() const { return State; }
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnActionEvent, UActionBase*);
	FOnActionEvent OnActionStarted;// 이 액션이 시작될때 호출되는 델리게이트
	FOnActionEvent OnActionEnded; // 정상적으로 끝날때 호출되는 델리게이트 
	FOnActionEvent OnActionFailed; // 코스트가 부족해 호출되는 델리게이트 
	FOnActionEvent OnActionCanceled;// 중간에 방해를 받아 호출 실패될때 호출되는 델리게이트

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category="Action|Animation")
	UAnimMontage* ActionMontage=nullptr;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category="Action|Animation")
	UAnimMontage* ActionMontage2=nullptr;
	UFUNCTION(BlueprintCallable, Category="Action|Cooldown")
	void SetActionMontage(UAnimMontage* montage);
	UFUNCTION(BlueprintCallable, Category="Action|Cooldown")
	UAnimMontage* GetActionMontage();
	

	/** (매니저가) 쿨다운 길이 설정/조회 */
	UFUNCTION(BlueprintCallable, Category="Action|Cooldown")
	void  SetCooldown(float InSeconds) { CooldownSec = FMath::Max(0.f, InSeconds); }
	UFUNCTION(BlueprintCallable, Category="Action|Cooldown")
	float GetCooldown() const { return CooldownSec; }
	/** (선택) 남은 쿨다운 값(매니저가 갱신해줄 수 있음) */
	UFUNCTION(BlueprintCallable, Category="Action|Cooldown")
	float GetCooldownRemaining() const { return CooldownRemaining; }
	void  SetCooldownRemaining(float InRemain) { CooldownRemaining = FMath::Max(0.f, InRemain); }

	/** 액션 구분 태그 (예: "Input.Fire", "Input.Skill1") */
    UFUNCTION(BlueprintCallable, Category="Action|Tag")
    FName GetActionTag() const { return ActionTag; }
    void  SetActionTag(FName InTag) { ActionTag = InTag; }


protected:
    // If true, ActionComponent finishes the action immediately after Activate and starts cooldown.
    // If false, the action stays Running until an explicit FinishActionNow call from the owner/skill.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action")
    bool bAutoFinish = true;

    UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly, Category="Action")
    EActionState State = EActionState::Idle;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Action")
	bool bOnCooldown = false;

	/** 총 쿨다운 길이(초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action")
	float CooldownSec = 0.f;

	/** 남은 쿨다운(정보 제공용; 매니저가 갱신 가능) */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Action")
	float CooldownRemaining = 0.f;

	/** 액션 구분 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action")
	FName ActionTag = NAME_None;

	friend class UActionComponent;

	
	void MarkRunning(); // 나 지금 실행중이다 라고 표시하는 함수 
	void MarkCooldown(float InCooldown);// 액션이 끝났고 이제 재사용 대기시간을 시작할 때 호출
	void MarkIdle();// 액션이 다시 준비됨 상태로 복귀 

	
};
