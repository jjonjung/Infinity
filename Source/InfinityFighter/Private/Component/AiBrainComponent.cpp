// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/AiBrainComponent.h"
#include "Base/CharacterBase.h"
#include "GameFramework/Character.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "ActionComponent.h"
#include "ActionBase.h"
#include "Chatacter/MySpiderMan.h"
#include "Component/InputProxyComponent.h"

UAiBrainComponent::UAiBrainComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	// AI 모드로 강제 설정
	Mode = EBrainMode::AI;
}

void UAiBrainComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// AI 초기화
	InitializeAI();
}

void UAiBrainComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UAiBrainComponent::InitializeAI()
{
	ActionRouter = GetOwner() ? GetOwner() -> FindComponentByClass<UActionRouter>() : nullptr;
	//UE_LOG(LogTemp, Warning, TEXT("UAiBrainComponent"));
	if (AIData)
	{
		CurrentHP = AIData->MaxHP;
		AiMoveScale = AIData->AIMoveScale;
	}
	else
	{
		CurrentHP = 5; // 기본값
		UE_LOG(LogTemp, Warning, TEXT("AiBrainComponent: AIData가 설정되지 않았습니다. 기본값 사용"));
	}

	CurrentTime = 0.0f;

	// 정적 카운터로 패턴 순차 할당 (0, 1, 2, 0, 1, 2...)
	static int32 PatternCounter = 0;
	int32 PatternIndex = PatternCounter % 3;
	CurrentPattern = static_cast<EAIMovementPattern>(PatternIndex);
	PatternCounter++;

	/*UE_LOG(LogTemp, Warning, TEXT("[AIBrainComponent] %s 에게 할당된 패턴: %d (0:혼란이동+Fire, 1:좌우이동+점프+Skill1, 2:엄폐+공격)"),
		*GetOwner()->GetName(), CurrentPattern);*/

	// 타겟 설정 (부모 클래스의 TargetActor 사용)
	if (UGameplayStatics::GetPlayerPawn(GetWorld(), 0) != nullptr)
	{
		TargetActor = Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
		UE_LOG(LogTemp, Warning, TEXT("[AIBrainComponent] 플레이어 타겟 설정 완료: %s"), *TargetActor->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[AIBrainComponent] BeginPlay: 플레이어를 찾을 수 없습니다."));
	}


}

#pragma region AI 상태 변경 FSM
void UAiBrainComponent::UpdateAIStateMachine(EEnemyState NewState)
{
	// 상태 변경만 처리 (재귀 방지)
	if (CurrentEnemyState != NewState)
	{
		CurrentEnemyState = NewState;
		CurrentTime = 0.0f;
		UE_LOG(LogTemp, Warning, TEXT("AI 상태 변경: %s"), *UEnum::GetValueAsString(CurrentEnemyState));
	}
}

void UAiBrainComponent::ExecuteCurrentState(EEnemyState NewState)
{
	// 현재 상태를 화면에 표시 (디버깅용)
	if (GEngine)
	{
		FString StateString = UEnum::GetValueAsString(CurrentEnemyState);
		GEngine->AddOnScreenDebugMessage(0, 0.1f, FColor::Cyan,
			FString::Printf(TEXT("AI State: %s"), *StateString));
	}

	// 상태별 처리
	switch (CurrentEnemyState)
	{
		case EEnemyState::Idle:
			IdleState();
			break;
		case EEnemyState::Move:
			MoveState();
			break;
		case EEnemyState::Attack:
			AttackState();
			break;
		case EEnemyState::Damage:
			DamageState();
			break;
		case EEnemyState::Die:
			DieState();
			break;
	}
}

void UAiBrainComponent::IdleState()
{
	UE_LOG(LogTemp, Warning, TEXT("AI IdleState Start"));
	
	CurrentTime += GetWorld()->DeltaTimeSeconds;

	float IdleTime = AIData ? AIData->IdleDelayTime : 0.5f;
	if (CurrentTime > IdleTime)
	{
		if (TargetActor.IsValid())
		{
			UpdateAIStateMachine(EEnemyState::Move);
		}
		else
		{
			// 타겟 재검색 시도
			if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
			{
				TargetActor = Cast<ACharacter>(PlayerPawn);
				UE_LOG(LogTemp, Warning, TEXT("타겟 재검색 성공: %s"), *TargetActor->GetName());
			}
			CurrentTime = 0.0f;
		}
	}
}

void UAiBrainComponent::MoveState()
{
	//UE_LOG(LogTemp, Warning, TEXT("Ai MoveState Start CurrentPattern: %d"), CurrentPattern);
	
	if (!TargetActor.IsValid())
	{
		UpdateAIStateMachine(EEnemyState::Idle);
		return;
	}

	CurrentTime += GetWorld()->DeltaTimeSeconds;
	AActor* Self = GetOwner();
	ACharacter* Character = Cast<ACharacter>(Self);
	if (!Character) return;

	FVector ToTarget = TargetActor->GetActorLocation() - Self->GetActorLocation();
	ToTarget.Z = 0.f;
	float DistanceToTarget = GetDistanceToTarget();

	// 플레이어 공격 감지 및 회피
	/*if (bIsPlayerAttacking && (CurrentTime - LastDodgeTime) > 0.3f)
	{
		ExecuteDodgeMovement();
		LastDodgeTime = CurrentTime;
		bIsPlayerAttacking = false;
		UE_LOG(LogTemp, Warning, TEXT("플레이어 공격 감지! 회피 실행"));
		return;
	}*/

	int32 PatternIndex = static_cast<int32>(CurrentTime / 4.0f) % 3;
	CurrentPattern = static_cast<EAIMovementPattern>(PatternIndex);

	// 플레이어 방향으로 회전 및 구르기
	FRotator LookRotation = FRotationMatrix::MakeFromX(ToTarget.GetSafeNormal()).Rotator();
	Character->SetActorRotation(FMath::RInterpTo(Character->GetActorRotation(), LookRotation, GetWorld()->DeltaTimeSeconds, 10.0f));
	RecievedIntent(FName("Input.Double"));
	
	// 패턴별 역동적 행동
	switch (CurrentPattern)
	{
		case EAIMovementPattern::ChaoticMovement:
			// 패턴 1: 혼란스럽고 예측 불가능한 이동 + Input.Fire
			ExecuteChaoticMovementPattern(Character, ToTarget, DistanceToTarget);
			break;
		case EAIMovementPattern::StrafingJump:
			// 패턴 2: 좌우 이동 및 점프 + Input.Skill1 +(스파이더맨 당기기)
			ExecuteStrafingJumpPattern(Character, ToTarget, DistanceToTarget);
			break;
		case EAIMovementPattern::CoverAttack:
			// 패턴 3: 엄폐 기반 공격
			ExecuteCoverAttackPattern(Character, ToTarget, DistanceToTarget);
			break;
	}

	if (CurrentTime > 5.0f)
	{
		UpdateAIStateMachine(EEnemyState::Idle);
	}
}

void UAiBrainComponent::AttackState()
{
	if (!TargetActor.IsValid())
	{
		UpdateAIStateMachine(EEnemyState::Idle);
		return;
	}

	CurrentTime += GetWorld()->DeltaTimeSeconds;
	AActor* Self = GetOwner();
	ACharacter* Character = Cast<ACharacter>(Self);
	if (!Character) return;

	FVector Direction = (TargetActor->GetActorLocation() - Self->GetActorLocation());
	float Distance = Direction.Size();

	float IdealAttackRange = AIData ? AIData->AttackRange : 200.0f;
	float MinAttackRange = IdealAttackRange * 0.7f;

	// 플레이어 방향으로 몸통 회전
	FRotator LookRotation = FRotationMatrix::MakeFromX(Direction.GetSafeNormal()).Rotator();
	Character->SetActorRotation(FMath::RInterpTo(Character->GetActorRotation(), LookRotation, GetWorld()->DeltaTimeSeconds, 12.0f));

	// 거리에 따른 이동 전략
	if (Distance > IdealAttackRange * 1.5f)
	{
		// 멀면 빠르게 접근
		Character->AddMovementInput(Direction.GetSafeNormal(), 1.8f);
	}
	else if (Distance < MinAttackRange * 0.8f)
	{
		// 너무 가까우면 백스텝하면서 공격
		FVector SideDirection = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
		float SideMove = FMath::Sin(CurrentTime * 6.0f) * 0.7f;
		FVector BackStep = (-Direction.GetSafeNormal() * 0.5f) + (SideDirection * SideMove);
		Character->AddMovementInput(BackStep.GetSafeNormal(), 1.2f);
	}
	else
	{
		// 적정 거리에서 좌우 움직이며 공격
		FVector SideDirection = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
		float SideMove = FMath::Sin(CurrentTime * 4.5f) * 0.8f;
		FVector CircleMove = (Direction.GetSafeNormal() * 0.3f) + (SideDirection * SideMove);
		Character->AddMovementInput(CircleMove.GetSafeNormal(), 1.4f);
	}

	// 지속적인 공격 (0.5초마다, 거리 제한 완화)
	if (FMath::Fmod(CurrentTime, 0.5f) < 0.1f)
	{
		if (ActionRouter)
		{
			static int32 ComboCount = 0;
			if (ComboCount % 3 == 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("AttackState: ActionRouter 있음, Skill1 공격 시도!"));
				InputProxy->EmitIntent(FName("Input.Skill1"));
				InputProxy->EmitIntent(FName("Input.Fire"));
				UE_LOG(LogTemp, Warning, TEXT("AttackState: ActionRouter 있음, Skill1 공격 성공!"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("AttackState: ActionRouter 있음, Fire 공격 시도!"));
				InputProxy->EmitIntent(FName("Input.Fire"));
			}
			ComboCount++;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AttackState: ActionRouter가 null입니다!"));
		}
	}

	if (CurrentTime > 5.0f)
	{
		UpdateAIStateMachine(EEnemyState::Idle);
	}
}

void UAiBrainComponent::DamageState()
{
	CurrentTime += GetWorld()->DeltaTimeSeconds;

	// 넉백 처리 (선형 보간으로 부드럽게 이동)
	AActor* Self = GetOwner();
	if (Self)
	{
		float LerpSpeed = GetWorld()->DeltaTimeSeconds * 5.0f;
		FVector CurrentPos = Self->GetActorLocation();
		FVector NewPos = FMath::Lerp(CurrentPos, KnockbackTargetPos, LerpSpeed);
		
		float DistanceToTarget = FVector::Dist(NewPos, CurrentPos);
		if (DistanceToTarget > 1.0f)
		{
			Self->SetActorLocation(NewPos, true);
		}
	}

	// 일정 시간 후 대기 상태로 복귀
	float DamageDelay = AIData ? AIData->DamageDelayTime : 1.5f;
	if (CurrentTime >= DamageDelay)
	{
		UpdateAIStateMachine(EEnemyState::Idle);
	}
}

void UAiBrainComponent::DieState()
{
	AActor* Self = GetOwner();
	if (!Self) return;
	
	// 충돌 비활성화
	Self->SetActorEnableCollision(false);
	
	// 아래로 이동 (땅으로 가라앉기)
	FVector CurrentPos = Self->GetActorLocation();
	FVector DownVector = -Self->GetActorUpVector() * 100.0f * GetWorld()->DeltaTimeSeconds;
	Self->SetActorLocation(CurrentPos + DownVector);
	
	// 일정 높이 아래로 가면 오브젝트 파괴
	if (CurrentPos.Z < -80.0f)
	{
		Self->Destroy();
	}
}

#pragma endregion AI 상태 변경 FSM

// ========== 헬퍼 함수들 ==========

bool UAiBrainComponent::IsInAttackRange() const
{
	float Range = AIData ? AIData->AttackRange : 200.0f;
	return GetDistanceToTarget() <= Range;
}

float UAiBrainComponent::GetDistanceToTarget() const
{
	if (!TargetActor.IsValid() || !GetOwner())
		return FLT_MAX;
		
	return FVector::Dist(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation());
}

void UAiBrainComponent::OnDamageProcess(const FVector& HitDirection, float DamageAmount)
{
	CurrentHP -= FMath::RoundToInt(DamageAmount);
	CurrentTime = 0.0f;

	if (CurrentHP > 0)
	{
		// 넉백 위치 계산
		FVector NormalizedDirection = HitDirection;
		NormalizedDirection.Z = 0; // 수평 넉백만
		NormalizedDirection.Normalize();

		float KnockbackForce = AIData ? AIData->KnockbackPower : 300.0f;
		KnockbackTargetPos = GetOwner()->GetActorLocation() + (NormalizedDirection * KnockbackForce);

		UpdateAIStateMachine(EEnemyState::Damage);
		UE_LOG(LogTemp, Warning, TEXT("AI 피해 받음 - 체력: %d"), CurrentHP);
	}
	else
	{
		UpdateAIStateMachine(EEnemyState::Die);
		UE_LOG(LogTemp, Warning, TEXT("AI 사망"));
	}
}

// ========== 회피 및 은신 시스템 ==========
void UAiBrainComponent::OnPlayerAttackDetected()
{
	bIsPlayerAttacking = true;
	UE_LOG(LogTemp, Warning, TEXT("플레이어 공격 감지됨!"));
}

void UAiBrainComponent::ExecuteDodgeMovement()
{
	RecievedIntent(FName("Input.Dodge"));

	UE_LOG(LogTemp, Warning, TEXT("회피 이동 구르기 실행"));
}

bool UAiBrainComponent::FindHidingSpot()
{
	AActor* Self = GetOwner();
	if (!Self || !TargetActor.IsValid()) return false;

	// 플레이어 반대 방향으로 벽 찾기
	FVector ToPlayer = TargetActor->GetActorLocation() - Self->GetActorLocation();
	ToPlayer.Z = 0.f;
	FVector AwayFromPlayer = -ToPlayer.GetSafeNormal();

	// 레이캐스트로 벽 찾기
	FHitResult HitResult;
	FVector StartLocation = Self->GetActorLocation();
	FVector EndLocation = StartLocation + (AwayFromPlayer * 200.0f); // 200유닛 거리까지 검색

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Self);
	QueryParams.AddIgnoredActor(TargetActor.Get());

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_WorldStatic,
		QueryParams
	);

	if (bHit)
	{
		// 벽 뒤쪽 위치 계산
		FVector WallNormal = HitResult.Normal;
		HidePosition = HitResult.Location + (WallNormal * 30.0f); // 벽에서 30유닛 떨어진 위치
		bIsHiding = true;
		UE_LOG(LogTemp, Warning, TEXT("은신 위치 발견: %s"), *HidePosition.ToString());
		return true;
	}

	// 벽이 없으면 플레이어 반대 방향으로 이동
	HidePosition = Self->GetActorLocation() + (AwayFromPlayer * 100.0f);
	bIsHiding = true;
	UE_LOG(LogTemp, Warning, TEXT("벽 없음, 후퇴 위치: %s"), *HidePosition.ToString());
	return false;
}

void UAiBrainComponent::ExecuteHidingBehavior()
{
	AActor* Self = GetOwner();
	ACharacter* Character = Cast<ACharacter>(Self);
	if (!Character || !TargetActor.IsValid()) return;

	// 은신 위치 찾기 (처음 한 번만)
	if (!bIsHiding)
	{
		FindHidingSpot();
	}

	// 은신 위치로 이동
	FVector ToHideSpot = HidePosition - Self->GetActorLocation();
	ToHideSpot.Z = 0.f;

	if (ToHideSpot.Size() > 20.0f)
	{
		// 은신 위치로 이동
		Character->AddMovementInput(ToHideSpot.GetSafeNormal(), 1.8f);
		UE_LOG(LogTemp, Warning, TEXT("은신 위치로 이동 중..."));
	}
	else
	{
		// 은신 위치 도달, 플레이어 방향으로 회전하며 공격
		FVector ToPlayer = TargetActor->GetActorLocation() - Self->GetActorLocation();
		FRotator LookRotation = FRotationMatrix::MakeFromX(ToPlayer.GetSafeNormal()).Rotator();
		Character->SetActorRotation(FMath::RInterpTo(Character->GetActorRotation(), LookRotation, GetWorld()->DeltaTimeSeconds, 12.0f));

		// 은신하며 빠른 연속 공격 (0.4초마다)
		if (FMath::Fmod(CurrentTime, 0.4f) < 0.1f && InputProxy)
		{
			if (CurrentPattern == EAIMovementPattern::ChaoticMovement)
			{
				UE_LOG(LogTemp, Warning, TEXT("은신 중 연속 Input.Fire 공격!"));
				RecievedIntent(FName("Input.Fire"));
				//InputProxy->EmitIntent(FName("Input.Fire"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("은신 중 연속 Input.Skill1 공격!"));
				RecievedIntent(FName("Input.Skill1"));
				//InputProxy->EmitIntent(FName("Input.Skill1"));
			}
		}

		// 추가 공격 (교대로 다른 스킬 사용)
		if (FMath::Fmod(CurrentTime, 0.7f) < 0.1f && InputProxy)
		{
			if (CurrentPattern == EAIMovementPattern::ChaoticMovement)
			{
				UE_LOG(LogTemp, Warning, TEXT("은신 중 추가 Input.Skill1 공격!"));
				RecievedIntent(FName("Input.Skill1"));
				//InputProxy->EmitIntent(FName("Input.Skill1"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("은신 중 추가 Input.Fire 공격!"));
				RecievedIntent(FName("Input.Fire"));
				//InputProxy->EmitIntent(FName("Input.Fire"));
			}
		}

		// 5초 후 은신 해제 (시간 연장)
		if (CurrentTime > 3.0f)
		{
			bIsHiding = false;
			UE_LOG(LogTemp, Warning, TEXT("은신 해제 - 다시 추격 모드"));
		}
	}
}

// ========== 새로운 패턴별 행동 함수들 ==========
void UAiBrainComponent::ExecuteChaoticMovementPattern(ACharacter* Character, const FVector& ToTarget, float Distance)
{
	UE_LOG(LogTemp, Warning, TEXT("패턴1"));
	// 패턴 1: 혼란스럽고 예측 불가능한 이동 + Input.Fire
	// 다중 방향 혼란 이동
	FVector SideDirection = FVector::CrossProduct(ToTarget, FVector::UpVector).GetSafeNormal();

	// 복잡한 수학 함수로 예측 불가능한 움직임 생성
	float Chaos1 = FMath::Sin(CurrentTime * 6.2f) * 1.3f;
	float Chaos2 = FMath::Cos(CurrentTime * 4.7f) * 0.9f;
	float Chaos3 = FMath::Sin(CurrentTime * 3.1f) * FMath::Cos(CurrentTime * 2.3f) * 0.7f;

	// 거리에 따른 기본 방향
	FVector BaseDirection;
	if (Distance > 250.0f)
	{
		BaseDirection = ToTarget.GetSafeNormal() * 0.8f; // 접근
	}
	else if (Distance < 150.0f)
	{
		BaseDirection = -ToTarget.GetSafeNormal() * 0.6f; // 후퇴
	}
	else
	{
		BaseDirection = FVector::ZeroVector; // 유지
	}

	// 혼란스러운 최종 방향 계산
	FVector ChaoticDirection = BaseDirection +
		(SideDirection * Chaos1) +
		(ToTarget.GetSafeNormal() * Chaos2) +
		(FVector::CrossProduct(SideDirection, FVector::UpVector) * Chaos3);

	Character->AddMovementInput(ChaoticDirection.GetSafeNormal(), 2.0f);

	if (FMath::Fmod(CurrentTime, 0.7f) < 0.1f)
	{
		RecievedIntent(FName("Input.Fire"));
		UE_LOG(LogTemp, Warning, TEXT("혼란 이동 중 Fire 공격!"));
	}
}

void UAiBrainComponent::ExecuteStrafingJumpPattern(ACharacter* Character, const FVector& ToTarget, float Distance)
{
	UE_LOG(LogTemp, Warning, TEXT("패턴2"));
	// 패턴 2: 좌우 이동 및 점프 + Input.Skill1

	FVector SideDirection = FVector::CrossProduct(ToTarget, FVector::UpVector).GetSafeNormal();

	// 좌우 스트레이핑 (더 넓은 범위)
	float StrafeIntensity = FMath::Sin(CurrentTime * 2.5f) * 1.5f;

	// 거리 조절 이동
	FVector DistanceMove;
	if (Distance > 300.0f)
	{
		DistanceMove = ToTarget.GetSafeNormal() * 0.7f; // 접근
	}
	else if (Distance < 200.0f)
	{
		DistanceMove = -ToTarget.GetSafeNormal() * 0.4f; // 약간 후퇴
	}
	else
	{
		DistanceMove = FVector::ZeroVector;
	}

	FVector StrafingMove = (SideDirection * StrafeIntensity) + DistanceMove;
	Character->AddMovementInput(StrafingMove.GetSafeNormal(), 1.7f);

	if (FMath::Fmod(CurrentTime, 10.f) < 0.1f)
	{
		UE_LOG(LogTemp, Warning, TEXT("스트레이핑 중 점프!"));
		RecievedIntent(FName("Input.Jump"));
	}
	
	if (FMath::Fmod(CurrentTime, 10.f) < 0.1f)
	{
		if (Character->IsA(AMySpiderMan::StaticClass()))
		{
			// 플레이어 존재 여부 확인
			APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
			if (!IsValid(PlayerPawn))
			{
				UE_LOG(LogTemp, Warning, TEXT("플레이어를 찾을 수 없습니다!"));
				return;
			}
         			
			FVector CurrentLocation = Character->GetActorLocation();
			FVector TargetLocation = PlayerPawn->GetActorLocation();
			FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
                 
			Direction.Z = 0.0f;
			Direction = Direction.GetSafeNormal();
                 
			float TargetDistance = FVector::Dist(CurrentLocation, TargetLocation);
			FRotator LookAtRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
			Character->SetActorRotation(FRotator(0, LookAtRotation.Yaw, 0)); // Yaw만 적용
			RecievedIntent(FName("Input.Skill1"));
			RecievedIntent(FName("Input.Fire"));

			UE_LOG(LogTemp, Warning, TEXT("패턴2: SpiderMan Skill1: Direction(%s), Distance(%.2f)"),
				  *Direction.ToString(), TargetDistance);
		}
		/*else
		{
			RecievedIntent(FName("Input.Skill1"));
			UE_LOG(LogTemp, Warning, TEXT("패턴2: ironMan Skill1"));
		}*/
	}
	
}

void UAiBrainComponent::ExecuteCoverAttackPattern(ACharacter* Character, const FVector& ToTarget, float Distance)
{
	UE_LOG(LogTemp, Warning, TEXT("패턴3"));
	// 패턴 3: 엄폐 기반 공격 (항상 엄폐를 시도)

	// BP_MySpiderMan이 skill을 써서 player를 당길 때 BP_MyIronMan이 스킬 쓰도록 로직 추가
	if (!bIsHiding)
	{
		CheckForSpiderManPullAndTriggerIronManSkill(Character);
		UE_LOG(LogTemp, Warning, TEXT("!bIsHiding"));
		FindHidingSpot();
	}

	// 엄폐 위치로 이동하거나 엄폐 상태 유지
	if (bIsHiding)
	{
		FVector ToHideSpot = HidePosition - Character->GetActorLocation();
		ToHideSpot.Z = 0.f;

		if (ToHideSpot.Size() > 30.0f)
		{
			// 엄폐 위치로 빠르게 이동
			Character->AddMovementInput(ToHideSpot.GetSafeNormal(), 2.2f);
			UE_LOG(LogTemp, Warning, TEXT("엄폐 위치로 신속 이동"));
		}
		else
		{
			// 엄폐 위치에서 집중 공격
			// Fire와 Skill1을 교대로 빠르게 공격
			if (FMath::Fmod(CurrentTime, 0.7f) < 0.1f && InputProxy)
			{
				UE_LOG(LogTemp, Warning, TEXT("엄폐 상태 집중 Fire 공격!"));
				RecievedIntent(FName("Input.Fire"));
				//InputProxy->EmitIntent(FName("Input.Fire"));
			}

			if (FMath::Fmod(CurrentTime, 0.7f) < 0.1f && InputProxy)
			{
				UE_LOG(LogTemp, Warning, TEXT("엄폐 상태 집중 Skill1 공격!"));
				RecievedIntent(FName("Input.Skill1"));
				//InputProxy->EmitIntent(FName("Input.Skill1"));
			}

			// 2초 후 새로운 엄폐 위치 찾기
			if (CurrentTime > 1.0f)
			{
				bIsHiding = false;
				UE_LOG(LogTemp, Warning, TEXT("새로운 엄폐 위치 탐색"));
			}
		}
	}
	else
	{
		// 엄폐 위치를 찾지 못했다면 후퇴하며 공격
		FVector RetreatDirection = -ToTarget.GetSafeNormal();
		FVector SideDirection = FVector::CrossProduct(ToTarget, FVector::UpVector).GetSafeNormal();
		float SideMove = FMath::Sin(CurrentTime * 4.0f) * 0.8f;

		FVector RetreatMove = (RetreatDirection * 0.9f) + (SideDirection * SideMove);
		Character->AddMovementInput(RetreatMove.GetSafeNormal(), 1.5f);

		// 후퇴하며 공격
		if (FMath::Fmod(CurrentTime, 0.7f) < 0.1f && InputProxy)
		{
			UE_LOG(LogTemp, Warning, TEXT("후퇴 중 혼합 공격!"));
			if (FMath::RandBool())
			{
				RecievedIntent(FName("Input.Fire"));
				//InputProxy->EmitIntent(FName("Input.Fire"));
			}
			else
			{
				RecievedIntent(FName("Input.Skill1"));
				//InputProxy->EmitIntent(FName("Input.Skill1"));
			}
		}
	}
}

void UAiBrainComponent::CheckForSpiderManPullAndTriggerIronManSkill(ACharacter* Character)
{
	if (!Character || !GetWorld()) return;

	// 근처의 모든 액터를 탐지하여 SpiderMan과 IronMan을 찾기
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);

	ACharacter* SpiderManActor = nullptr;
	ACharacter* IronManActor = nullptr;

	// SpiderMan과 IronMan 캐릭터 찾기
	for (AActor* Actor : FoundActors)
	{
		if (!Actor) continue;

		FString ActorName = Actor->GetName();
		FString ActorClassName = Actor->GetClass()->GetName();

		// BP_MySpiderMan 찾기 (Blueprint 이름 기반)
		if (ActorName.Contains(TEXT("SpiderMan")) || ActorClassName.Contains(TEXT("SpiderMan")))
		{
			SpiderManActor = Cast<ACharacter>(Actor);
			UE_LOG(LogTemp, Warning, TEXT("SpiderMan 발견: %s"), *ActorName);
		}
		// BP_MyIronMan 찾기 (Blueprint 이름 기반)
		else if (ActorName.Contains(TEXT("IronMan")) || ActorClassName.Contains(TEXT("IronMan")))
		{
			IronManActor = Cast<ACharacter>(Actor);
			UE_LOG(LogTemp, Warning, TEXT("IronMan 발견: %s"), *ActorName);
		}
	}

	// SpiderMan이 있고 플레이어를 당기는 스킬을 사용 중인지 확인
	if (SpiderManActor && IsSpiderManUsingPullSkill(SpiderManActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("SpiderMan이 플레이어를 당기는 중! IronMan 스킬 트리거"));

		// IronMan이 있으면 스킬 사용
		if (IronManActor)
		{
			TriggerIronManSkill(IronManActor);
		}
	}
}

bool UAiBrainComponent::IsSpiderManUsingPullSkill(ACharacter* SpiderManCharacter)
{
	if (!SpiderManCharacter) return false;

	// ActionComponent에서 현재 실행 중인 액션 확인
	UActionComponent* ActionComp = SpiderManCharacter->FindComponentByClass<UActionComponent>();
	if (!ActionComp) return false;

	// Action.Skill1 태그로 액션을 찾아서 실행 중인지 확인
	UActionBase* SpiderManSkill = ActionComp->GetActionByTag(FName("Action.Skill1"));
	if (SpiderManSkill && SpiderManSkill->IsRunning())
	{
		// 추가로 클래스 이름이 SpiderManSwing인지 확인
		if (SpiderManSkill->GetClass()->GetName().Contains(TEXT("Skill1_SpiderManSwing")))
		{
			UE_LOG(LogTemp, Warning, TEXT("SpiderMan이 Swing 스킬 사용 중 확인됨"));
			return true;
		}
	}

	return false;
}

void UAiBrainComponent::TriggerIronManSkill(ACharacter* IronManCharacter)
{
	if (!IronManCharacter) return;

	if (InputProxy)
	{
		RecievedIntent(FName("Input.Skill1"));
		UE_LOG(LogTemp, Warning, TEXT("IronMan에게 Skill1 명령 전송 성공"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("IronMan의 InputProxyComponent를 찾을 수 없습니다"));
	}
}