// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionBase.h"

//현재 액션이 실행 가능한가? (쿨타임인 경우는 실행 불가) 
bool UActionBase::CanActivate_Implementation(ACharacterBase* Owner,FString& FailReason) const
{
	return !bOnCooldown;
}
//액션을 실행할때 소모되는 비용체크 함수 ex 마나
bool UActionBase::PayCost_Implementation(ACharacterBase* Owner,FString& FailReason)
{
	
	return true; 
}

// 액션 실행부 ex) 애니메이션 실행 같은거, 델리게이트는 ui 전송용
void UActionBase::Activate_Implementation(ACharacterBase* Owner)
{
	OnActionStarted.Broadcast(this);
	OnActionEnded.Broadcast(this);
}

//액션이 정상적으로 실행되고 종료 되었을때 실행 되는 함수 
void UActionBase::End_Implementation(ACharacterBase* Owner)
{
	
}

// 무언가의 방해를 받아 액션이 취소되었을때 실행되는 함수
void UActionBase::Cancel_Implementation(ACharacterBase* Owner)
{
	OnActionCanceled.Broadcast(this);
}

// 현재 쿨타임인지 알려주는 함수 
bool UActionBase::IsOnCooldown() const
{ return bOnCooldown; }

void UActionBase::SetActionMontage(UAnimMontage* montage)
{
	ActionMontage = montage;
}

UAnimMontage* UActionBase::GetActionMontage()
{
	return ActionMontage;
}


//현재 스테이트 변경 함수 => 이 액션이 실행중일때 State => Running 스테이트가 된다.
void UActionBase::MarkRunning()
{
	State = EActionState::Running;
}
// 스킬을 사용하고 쿨타임 진행시키는 함수 
void UActionBase::MarkCooldown(float InCooldown)
{
	bOnCooldown = (InCooldown>0.f);
	CooldownSec = FMath::Max(0.f,InCooldown);
	CooldownRemaining = CooldownSec;
	State = bOnCooldown? EActionState::Cooldown : EActionState::Idle;
}

// 쿨타임이 끝나 실행 가능한 상태임으로 되돌리는 함수 
void UActionBase::MarkIdle()
{
	State = EActionState::Idle;
	bOnCooldown = false;
	CooldownRemaining = 0.f;
}

