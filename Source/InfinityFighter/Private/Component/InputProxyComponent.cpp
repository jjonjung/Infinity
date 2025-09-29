// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/InputProxyComponent.h"

#include "CharacterActionStatComponent.h"
#include "Component/CBrainComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "InputActionValue.h"
#include "Base/CharacterBase.h"
#include "GameFramework/PlayerController.h"
#include "ActionComponent.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UInputProxyComponent::UInputProxyComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInputProxyComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	Brain = GetOwner() ? GetOwner()->FindComponentByClass<UCBrainComponent>() : nullptr;
	OwnerBase =Cast<ACharacterBase>(GetOwner());
}


// Called every frame
void UInputProxyComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInputProxyComponent::SetupPlayerBindings(UEnhancedInputComponent* EIC)
{
    check(EIC);
    
    if (IA_MoveForward)
	{
    	
		EIC->BindAction(IA_MoveForward,ETriggerEvent::Started,this,&UInputProxyComponent::CheckDoubleFoward);
		EIC->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &UInputProxyComponent::OnMoveForward);
		EIC->BindAction(IA_MoveForward, ETriggerEvent::Completed, this, &UInputProxyComponent::OnMoveForward);
	}
	if (IA_MoveRight)
	{
		EIC->BindAction(IA_MoveRight,   ETriggerEvent::Triggered, this, &UInputProxyComponent::OnMoveRight);
		EIC->BindAction(IA_MoveRight,   ETriggerEvent::Completed, this, &UInputProxyComponent::OnMoveRight);
	}
	if (IA_Look)
	{
		EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &UInputProxyComponent::OnLook);
	}
	if (IA_Reload)
	{
		EIC->BindAction(IA_Reload, ETriggerEvent::Started,this,&UInputProxyComponent::OnReload);
	}
	if (IA_Dodge)
	{
		EIC->BindAction(IA_Dodge,ETriggerEvent::Started,this,&UInputProxyComponent::OnDodge);
	}
	if(IA_Fire)
	{
		EIC->BindAction(IA_Fire,ETriggerEvent::Started,this,&UInputProxyComponent::OnFire);
    }
    if (IA_Jump)
    {
        // Detect double-tap on Jump (space) before emitting single Jump
        EIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &UInputProxyComponent::CheckDoubleJump);
        EIC->BindAction(IA_Jump,ETriggerEvent::Started,this,&UInputProxyComponent::OnJump);
    }
	if (IA_Skill1)
	{
		EIC->BindAction(IA_Skill1,ETriggerEvent::Started,this,&UInputProxyComponent::OnSkill1);
	}
	if (IA_Skill2)
	{
		EIC->BindAction(IA_Skill2,ETriggerEvent::Started,this,&UInputProxyComponent::OnSkill2);
		
	}
	
}

void UInputProxyComponent::RegisterIMCToLocalPlayer(class APlayerController* PC)
{
	if (!PC || !IC_Player) return;

	if (ULocalPlayer* LP = PC->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsys->AddMappingContext(IC_Player, 0);
		}
	}
}

void UInputProxyComponent::OnMoveForward(const FInputActionValue& Value)
{
	if (!Brain) return;
	this->CachedForward = Value.Get<float>();
	if (CachedForward == 0.f)
	{
		isFoward = false;
		OwnerBase->SetIsRunning(false);
		OwnerBase->GetCharacterMovement()->MaxWalkSpeed=600.f;
		return;
	}
	isFoward = true;
	Brain->OnMoveInput(FVector2D(this->CachedRight, this->CachedForward)); // (X=Right, Y=Forward)
}

void UInputProxyComponent::OnMoveRight(const FInputActionValue& Value)
{
	if (!Brain) return;
	this->CachedRight = Value.Get<float>();
	if (CachedRight == 0.f)
	{
		isRight = false;
		return;
	}
	isRight = true;
	if (!isFoward)
	{
		OwnerBase->GetCharacterMovement()->MaxWalkSpeed = 600.f;
		OwnerBase->SetIsRunning(false);
	}
	Brain->OnMoveInput(FVector2D(this->CachedRight, this->CachedForward));
}

void UInputProxyComponent::CheckDoubleFoward(const struct FInputActionValue& Value)
{
	bool isExcuteing = GetWorld()->GetTimerManager().IsTimerActive(DoubleHandle);
	if (isExcuteing)
	{
		GEngine->AddOnScreenDebugMessage(-1,2.F,FColor::Yellow,"Double Forward");
		CurrenTime_Press=0.f;
		EmitIntent(DoublePressed);
		return;
	}
	
	//UE_LOG(LogTemp,Warning,TEXT("타이머 시작"));
	GetWorld()->GetTimerManager().SetTimer(DoubleHandle,this,&UInputProxyComponent::IncreseDoublePressed,DoubleThreshold,false);
}
void UInputProxyComponent::IncreseDoublePressed()
{
	CurrenTime_Press+=DoubleThreshold;
	
}
//누른 순간에 타이머 시작하고 싶다.
//이 타이머가 돌아가는 순간에 키입력이 또 들어오면 더블 클릭 

void UInputProxyComponent::OnLook(const struct FInputActionValue& Value)
{
	if (!Brain) return;
	// 마우스 움직임은 2D 벡터 (X=Yaw좌우, Y=Pitch상하)
	FVector2D LookInput = Value.Get<FVector2D>();
	this->CachedLookX = LookInput.X;  // Yaw (좌우)
	this->CachedLookY = LookInput.Y;  // Pitch (상하)
    
	// Brain에 2D 벡터로 전달 (X=Yaw좌우, Y=Pitch상하)
	Brain->OnLookInput(FVector2D(this->CachedLookX, this->CachedLookY));
}

void UInputProxyComponent::OnFire(const struct FInputActionValue& Value)
{
	UE_LOG(LogTemp,Warning,TEXT("파이어 누름"));
	EmitIntent(FireIntent);
}

void UInputProxyComponent::OnSkill1(const struct FInputActionValue& Value)
{
	UE_LOG(LogTemp,Warning,TEXT("스킬1 누름"));
	EmitIntent(Skill1Intent);
}

void UInputProxyComponent::OnSkill2(const struct FInputActionValue& Value)
{
	EmitIntent(Skill2Intent);
}

void UInputProxyComponent::OnReload(const struct FInputActionValue& Value)
{
	EmitIntent(ReloadIntent);
}

void UInputProxyComponent::OnJump(const struct FInputActionValue& Value)
{
	UE_LOG(LogTemp,Warning,TEXT("점프 누름"));
	EmitIntent(JumpIntent);
}

void UInputProxyComponent::CheckDoubleJump(const struct FInputActionValue& Value)
{
    const bool bTimerActive = GetWorld()->GetTimerManager().IsTimerActive(JumpDoubleHandle);
    if (bTimerActive)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("Double Jump (Space)"));
        }
        CurrenTime_Press = 0.f;
        GetWorld()->GetTimerManager().ClearTimer(JumpDoubleHandle);
        // Toggle: if already flying, finish action (start cooldown); otherwise start flying
        if (OwnerBase && OwnerBase->ActionStatComp && OwnerBase->ActionStatComp->GetInFly())
        {
            if (OwnerBase->ActionComponent)
            {
                UActionBase* FlyAction = OwnerBase->ActionComponent->GetActionByTag(FName("Action.Fly"));
                if (FlyAction)
                {
                    OwnerBase->ActionComponent->FinishActionNow(FName("Action.Fly"), FlyAction);
                }
                else
                {
                    // Fallback: if action not found, cancel to restore walking
                    OwnerBase->ActionComponent->CancelByTag(FName("Action.Fly"));
                }
            }
        }
        else
        {
            EmitIntent(FlyIntent);
        }
        return;
    }

    // Arm timer for double-tap window
    GetWorld()->GetTimerManager().SetTimer(JumpDoubleHandle, this, &UInputProxyComponent::IncreseDoublePressed, DoubleThreshold, false);
}

// Removed OnJumpTriggered: ascend is no longer bound; double-space toggles flight off

void UInputProxyComponent::OnDodge(const struct FInputActionValue& Value)
{
	UE_LOG(LogTemp,Warning,TEXT("닷지 누름"));
	
	EmitIntent(DodgeIntent);
	
}

void UInputProxyComponent::RestoreArcVisibility()
{
	if (ArcComponent)
	{
		ArcComponent->SetHiddenInGame(false, /*bPropagateToChildren=*/true);
		UE_LOG(LogTemp, Warning, TEXT("Arc visible again"));
	}
}


void UInputProxyComponent::EmitIntent(FName IntenSignal)
{
	UE_LOG(LogTemp, Warning, TEXT("EmitIntent 들어옴, %s"), *IntenSignal.ToString());
	if (IntenSignal.IsNone()) return;
	OnIntent.Broadcast(IntenSignal);
}


