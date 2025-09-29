// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActionComponent.generated.h"

class UActionBase;
/** 액션 실행 이벤트(UX/HUD/로그용) */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnActionEvent, FName /*Tag*/, UActionBase* /*Action*/);
UCLASS( ClassGroup=(Action), meta=(BlueprintSpawnableComponent) )
class INFINITYFIGHTER_API UActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UActionComponent();
	UFUNCTION(BlueprintCallable, Category = "Action|Regiseter")
	void Register(FName Tag, UActionBase* Action);
	UFUNCTION(BlueprintCallable, Category="Action|Register")
	UActionBase* CreateAndRegister(TSubclassOf<UActionBase> ActionClass, FName Tag);

	UFUNCTION(BlueprintCallable, Category="Action|Run")
	bool TryRunAction(FName Tag,FString& OutFailReason);
	UFUNCTION(BlueprintCallable, Category="Action|Run")
	bool CancelByTag(FName Tag);
	UFUNCTION(BlueprintCallable, Category="Action|CoolDown")
	bool IsOnCoolDown(FName Tag) const;
	UFUNCTION(BlueprintCallable, Category="Action|CoolDown")
	float GetCoolDownRemaining(FName Tag) const;
	
	UFUNCTION(BlueprintCallable, Category="Action|Query")
	UActionBase* GetActionByTag(FName Tag) const { return Actions.FindRef(Tag); }
public :
	FOnActionEvent OnActionStarted;
	FOnActionEvent OnActionEnded;
	FOnActionEvent OnActionFailed;
	FOnActionEvent OnActionCanceled;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere,Instanced,Instanced,Category="Action")
	TMap<FName,UActionBase*> Actions;

private:
	TMap<FName,FTimerHandle> CoolDownTimers;

	/** 내부 유틸: 쿨다운 시작/해제/남은 시간 갱신 */
	void StartCooldown(FName Tag, UActionBase* Action, float Seconds);
	void ClearCooldown(FName Tag, UActionBase* Action);
	void TickCooldownRemaining(FName Tag, UActionBase* Action, float Seconds);

	/** 액션 종료 공통 처리(즉시 종료형에서 사용) */
public:
	void FinishActionNow(FName Tag, UActionBase* Action);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
		
};
