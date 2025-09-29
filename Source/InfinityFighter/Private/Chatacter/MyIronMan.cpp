// Fill out your copyright notice in the Description page of Project Settings.

#include "Chatacter/MyIronMan.h"
#include "ActionComponent.h"
#include "IronManRouter.h"
#include "Component/InputProxyComponent.h"
#include "Router/ActionRouter.h"
#include "Engine/Engine.h"
#include "Skill/Skill1_IronMan.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AMyIronMan::AMyIronMan()
{
	PrimaryActorTick.bCanEverTick = true;

	// 비행을 위해 점프 횟수를 2로 설정 (더블 점프 감지용)
	JumpMaxCount = 2;

	// 비행 중 감속 설정 (값이 높을수록 빠르게 멈춤)
	GetCharacterMovement()->BrakingDecelerationFlying = 6000.f;
}

void AMyIronMan::BeginPlay()
{
	Super::BeginPlay();

	// ActionComponent 초기화 후 아이언맨 액션들 설정
	FTimerHandle SetupTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(SetupTimerHandle, [this]()
	{
		SetupIronManActions();
	}, 0.5f, false); // 0.5초 지연으로 컴포넌트 초기화 대기
}

void AMyIronMan::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 비행 물리 업데이트
	UpdateFlight(DeltaTime);
}

void AMyIronMan::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 비행 중 수직 이동을 위한 축 입력 바인딩
	// 프로젝트 설정 -> 입력에서 "FlyUp"이라는 이름의 축 매핑을 추가해야 합니다.
	// 예: 스페이스바 (Scale 1.0), 왼쪽 Ctrl (Scale -1.0)
	PlayerInputComponent->BindAxis("FlyUp", this, &AMyIronMan::FlyUp);
}

void AMyIronMan::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyIronMan, bIsFlying);
}

void AMyIronMan::OnJumped_Implementation()
{
	// 첫 번째 점프는 일반 점프
	if (JumpCurrentCount == 1)
	{
		Super::OnJumped_Implementation();
	}
	// 두 번째 점프에서 추가 도약
	else if (JumpCurrentCount == 2)
	{
		FVector LaunchVelocity(0.f, 0.f, SuccessiveJumpZVelocity);
		LaunchCharacter(LaunchVelocity, false, true);
	}
}

void AMyIronMan::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// 땅에 착지하면 비행 상태를 강제로 종료
	if (bIsFlying)
	{
		Server_StopFlying();
	}
}

void AMyIronMan::ToggleFlight()
{
	if (bIsFlying)
	{
		Server_StopFlying();
	}
	else
	{
		Server_StartFlying();
	}
}

void AMyIronMan::Server_StartFlying_Implementation()
{
	if (!bIsFlying)
	{
		bIsFlying = true;
		OnRep_IsFlying(); // 서버에서도 즉시 상태 변경 적용
	}
}

void AMyIronMan::Server_StopFlying_Implementation()
{
	if (bIsFlying)
	{
		bIsFlying = false;
		OnRep_IsFlying(); // 서버에서도 즉시 상태 변경 적용
	}
}

void AMyIronMan::OnRep_IsFlying()
{
	if (bIsFlying)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);

		// 비행 애니메이션 몽타주 재생
		if (FlyingAnimMontage)
		{
			if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
			{
				AnimInstance->Montage_Play(FlyingAnimMontage);
			}
		}

		// 발 파티클 이펙트 부착
		if (FootThrusterEffect)
		{
			LeftFootThrusterComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				FootThrusterEffect, GetMesh(), FName("foot_l"), 
				FVector::ZeroVector, FRotator::ZeroRotator, 
				EAttachLocation::SnapToTarget, true);

			RightFootThrusterComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				FootThrusterEffect, GetMesh(), FName("foot_r"), 
				FVector::ZeroVector, FRotator::ZeroRotator, 
				EAttachLocation::SnapToTarget, true);
		}
	}
	else
	{
		// 비행 애니메이션 몽타주 중지
		if (FlyingAnimMontage)
		{
			if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
			{
				AnimInstance->Montage_Stop(0.25f, FlyingAnimMontage);
			}
		}

		// 발 파티클 이펙트 제거
		if (LeftFootThrusterComponent)
		{
			LeftFootThrusterComponent->DestroyComponent();
			LeftFootThrusterComponent = nullptr;
		}
		if (RightFootThrusterComponent)
		{
			RightFootThrusterComponent->DestroyComponent();
			RightFootThrusterComponent = nullptr;
		}

		// 착지 중이 아니라면 떨어지는 상태로, 땅 위라면 걷는 상태로 전환
		if (GetCharacterMovement()->IsFalling())
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		}
		else
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}
	}
}

void AMyIronMan::FlyUp(float Value)
{
	// 비행 중이고, 입력값이 있을 때만 수직 추력 적용
	if (bIsFlying && Value != 0.0f)
	{
		// 위/아래 방향으로 추력 추가 (FlyThrust의 절반 힘으로)
		GetCharacterMovement()->AddForce(FVector::UpVector * Value * FlyThrust * 0.5f);
	}
}

void AMyIronMan::UpdateFlight(float DeltaTime)
{
	// 비행 상태가 아니면 아무것도 하지 않음
	if (!bIsFlying)
	{
		return;
	}

	// 전방 추력 (W키 입력에 따라)
	const float ForwardValue = GetInputAxisValue("MoveForward");
	if (ForwardValue > 0.f)
	{
		// 카메라(컨트롤러)의 전방 벡터를 따라 추력 적용
		const FVector ForwardDirection = GetControlRotation().Vector();
		GetCharacterMovement()->AddForce(ForwardDirection * FlyThrust * ForwardValue);
	}

	// 현재 속도가 최대 비행 속도를 초과하지 않도록 제한
	if (GetCharacterMovement()->Velocity.Size() > MaxFlySpeed)
	{
		GetCharacterMovement()->Velocity = GetCharacterMovement()->Velocity.GetClampedToMaxSize(MaxFlySpeed);
	}
}


void AMyIronMan::SetupIronManActions()
{
	// ActionComponent 유효성 체크
	if (!ActionComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("MyIronMan: ActionComponent가 없습니다."));
		return;
	}

	// Skill1_IronMan 액션 생성 및 등록 (Action.Skill1 태그 사용)
	USkill1_IronMan* IronManSkill1 = Cast<USkill1_IronMan>(
		ActionComponent->CreateAndRegister(USkill1_IronMan::StaticClass(), FName("Action.Skill1"))
	);

	if (IronManSkill1)
	{
		// 기본 설정값들은 블루프린트에서 설정하거나 여기서 추가 설정 가능
		UE_LOG(LogTemp, Log, TEXT("MyIronMan: Skill1_IronMan 액션이 등록되었습니다."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MyIronMan: Skill1_IronMan 액션 생성에 실패했습니다."));
	}

	// ActionRouter에 Input.Skill1 → Action.Skill1 매핑 추가
	if (UActionRouter* ActionRouterComp = FindComponentByClass<UActionRouter>())
	{
		ActionRouterComp->RegisterActionRouter(FName("Input.Skill1"), FName("Action.Skill1"));
		UE_LOG(LogTemp, Log, TEXT("MyIronMan: Input.Skill1 → Action.Skill1 매핑 추가"));
	}

}

void AMyIronMan::OnInputIntent(FName IntentTag)
{
	// ActionComponent를 통해 액션 실행
	if (ActionComponent)
	{
		FString FailReason;
		bool bSuccess = ActionComponent->TryRunAction(IntentTag, FailReason);

		if (bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("MyIronMan: Intent '%s' 실행 성공"), *IntentTag.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("MyIronMan: Intent '%s' 실행 실패: %s"), *IntentTag.ToString(), *FailReason);
		}
	}
}

