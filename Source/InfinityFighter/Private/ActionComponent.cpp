// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionComponent.h"

#include "ActionBase.h"
#include "DA/ActionLoadOutDA.h"
#include "UObject/ObjectRename.h"

// Sets default values for this component's properties
UActionComponent::UActionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


//Actions<TMap> 에 내가 사용 가능한 스킬들을 등록하는 함수
void UActionComponent::Register(FName Tag, UActionBase* Action)
{
	if (Tag.IsNone() || !Action) return;

	ResetRuntimeState(Tag);

	if (Actions.Contains(Tag))
	{
		UE_LOG(LogTemp,Warning,TEXT("이미 같은 액션이 들어왔습니다. %s"),*Tag.ToString());
	}
	UE_LOG(LogTemp,Warning,TEXT("등록한다! %s"),*Tag.ToString());
	Action -> SetActionTag(Tag);
	Actions.Add(Tag, Action);
		
}

//class 정보만 넣어주면 만들어서 등록하게 해주는 함수 
UActionBase* UActionComponent::CreateAndRegister(TSubclassOf<UActionBase> ActionClass, FName Tag)
{
	if (!*ActionClass) return nullptr;

	UActionBase* Instance = NewObject<UActionBase>(this,ActionClass);
	if (!Instance) return nullptr;

	Register(Tag,Instance);
	return Instance;
}

//액션 실행키를 눌렀을때 실행되는 함수(액션 실행조건이 되는지 검사하고 된다면 액션 실행) 안된다면 false 리턴
bool UActionComponent::TryRunAction(FName Tag, FString& OutFailReason)
{
	UE_LOG(LogTemp,Warning,TEXT("in TryRunAction"));
	UActionBase*  Action = GetActionByTag(Tag);
	
	if (!Action)
	{
		OutFailReason = TEXT("Action not found");
		OnActionFailed.Broadcast(Tag,nullptr);
		return false;
	}
	
	if (Action->IsOnCooldown())
	{
		OutFailReason = TEXT("On cooldown");
		OnActionFailed.Broadcast(Tag, Action);
		return false;
	}

	{
		FString Reason;
		if (!Action->CanActivate(GetOwner<ACharacterBase>(), Reason))
		{
			OutFailReason = Reason.IsEmpty() ? TEXT("Cannot activate") : Reason;
			OnActionFailed.Broadcast(Tag, Action);
			return false;
		}
	}

	{
		FString Reason;
		if (!Action->PayCost(GetOwner<ACharacterBase>(), Reason))
		{
			OutFailReason = Reason.IsEmpty() ? TEXT("Not enough resource") : Reason;
			OnActionFailed.Broadcast(Tag, Action);
			return false;
		}
	}

	Action->MarkRunning();
	OnActionStarted.Broadcast(Tag, Action);
	Action->Activate(GetOwner<ACharacterBase>());
	/**
	 * NOTE:
	 * - "즉시형 액션"(히트스캔/토글 없는 짧은 동작)은 여기서 곧바로 종료 처리 가능:
	 *     FinishActionNow(Tag, Action);
	 * - "지속형 액션"(캐스팅/채널링/몽타주 기반)은 액션 내부나 애님 노티파이에서 End/Cancel을 호출하고,
	 *   그 타이밍에 매니저가 FinishActionNow를 호출하도록 연결하는 걸 권장.
	 */
	if (Action->bAutoFinish)
	{
		FinishActionNow(Tag, Action);
	}
	return true;
}


// Called when the game starts
void UActionComponent::BeginPlay()
{
	Super::BeginPlay();
	ACharacterBase* Owner = Cast<ACharacterBase>(GetOwner());
	
	// ...
	
}

void UActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (TPair<FName, FActionRuntimeState>& Pair : RuntimeStates)
	{
		Pair.Value.Reset(GetWorld());
	}

	RuntimeStates.Empty();

	for (const TPair<FName, TObjectPtr<UActionBase>>& Pair : Actions)
	{
		if (UActionBase* Action = Pair.Value)
		{
			Action->MarkIdle();
		}
	}

	Super::EndPlay(EndPlayReason);
}


// Called every frame
void UActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Debug %f"), GetCoolDownRemaining("Test")));
	
}


//Tag이름으로 현재 액션을 검사하고 액션을 중지시키는 함수 즉 스킬 타겟팅중 취소할때 실행되는 함수라 이해하면 편안하다. 
bool UActionComponent::CancelByTag(FName Tag)
{
	UActionBase* const Action = GetActionByTag(Tag);
	if (!Action) return false;

	if (Action->IsRunning())
	{
		Action->Cancel(GetOwner<ACharacterBase>());
		OnActionCanceled.Broadcast(Tag, Action);
		// 취소 후 쿨다운 정책: 부분 쿨다운 or 즉시 Idle
		// 여기서는 Idle로 돌리고 싶다면:
		Action->MarkIdle();
		return true;
	}
	return false;
}


//Tag로 검색한 액션이 현재 쿨타임이 돌고있는지 
bool UActionComponent::IsOnCoolDown(FName Tag) const
{
	if (UActionBase* A = Actions.FindRef(Tag))
	{
		return A->IsOnCooldown();
	}
	return false;
}

//쿨타임이 몇초 남았는지 리턴하는 함수 
float UActionComponent::GetCoolDownRemaining(FName Tag) const
{
	if (UActionBase* A = Actions.FindRef(Tag))
	{
		return A->GetCooldownRemaining();
	}
	return 0.f;
}


//그 액션이 끝나면 쿨타임을 진행시키는 함수 
void UActionComponent::StartCooldown(FName Tag, UActionBase* Action, float Seconds)
{
	if (!Action || Seconds <= 0.f)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		ResetRuntimeState(Tag);

		Action->MarkCooldown(Seconds);
		Action->SetCooldownRemaining(Seconds);

		FActionRuntimeState NewRuntimeState;
		FTimerManager& TimerManager = World->GetTimerManager();

		TimerManager.SetTimer(
			NewRuntimeState.CooldownTickHandle,
			FTimerDelegate::CreateWeakLambda(this, [this, Tag, Action, Seconds]()
			{
				TickCooldownRemaining(Tag, Action, Seconds);
			}),
			1.0f, true
		);

		TimerManager.SetTimer(
			NewRuntimeState.CooldownEndHandle,
			FTimerDelegate::CreateWeakLambda(this, [this, Tag, Action]()
			{
				ClearCooldown(Tag, Action);
			}),
			Seconds, false
		);

		RuntimeStates.Add(Tag, MoveTemp(NewRuntimeState));
	}
}

//액션과 tag 정보를 이용하여 쿨다운을 초기화 하는 함수 
void UActionComponent::ClearCooldown(FName Tag, UActionBase* Action)
{
	if (!Action) return;
	ResetRuntimeState(Tag);
	Action->MarkIdle();
	// 필요하면 여기서 "쿨다운 종료" 이벤트를 추가로 방송할 수 있음
}

void UActionComponent::TickCooldownRemaining(FName Tag, UActionBase* Action, float Seconds)
{
	if (!Action)
	{
		return;
	}

	const float NewRemain = FMath::Clamp(Action->GetCooldownRemaining() - 1.f, 0.f, Seconds);
	Action->SetCooldownRemaining(NewRemain);

	if (NewRemain <= 0.f)
	{
		ResetRuntimeState(Tag);
	}
}

void UActionComponent::ResetRuntimeState(FName Tag)
{
	if (FActionRuntimeState* RuntimeState = RuntimeStates.Find(Tag))
	{
		RuntimeState->Reset(GetWorld());
		RuntimeStates.Remove(Tag);
	}
}

//액션이 종료할때 실행하는 함수  쿨타임이 따로 없는 함수면 바로 idle(실행가능상태) 아니라면 쿨타임을 진행 시킨다. 
void UActionComponent::FinishActionNow(FName Tag, UActionBase* Action)
{
	if (!Action) return;

	// 액션 정상 종료 이벤트
	OnActionEnded.Broadcast(Tag, Action);

	// 쿨다운 시작(설정값이 0이면 Idle로 바로 전환됨)
	const float Cool = Action->GetCooldown();
	if (Cool > 0.f)
	{
		StartCooldown(Tag, Action, Cool);
	}
	else
	{
		Action->MarkIdle();
	}
}

