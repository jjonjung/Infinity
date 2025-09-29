// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyAvatarCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ALobbyAvatarCharacter::ALobbyAvatarCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Move
	auto* Move = GetCharacterMovement();
	// 이동 벡터 방향으로 회전
	Move->bOrientRotationToMovement = true;
	Move->RotationRate = FRotator(0.f, 720.f, 0.f);

	//controller의 회전유지
	Move->bUseControllerDesiredRotation = true;
	bUseControllerRotationYaw = false; 
	bUseControllerRotationPitch = false;

	//Scale

	// 애니메이션 블루프린트 연결
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent)
	{
		static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPClass(TEXT("/Game/BluePrint/RealCharacter/SpiderMan/ABP_OnlyMove.ABP_OnlyMove_C"));
		if (AnimBPClass.Succeeded())
		{
			MeshComponent->SetAnimInstanceClass(AnimBPClass.Class);
		}
	}

}

// Called when the game starts or when spawned
void ALobbyAvatarCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		SkelMesh-> SetRelativeScale3D(FVector(LoobyCharScale));
		
	}

	
}



// Called to bind functionality to input
void ALobbyAvatarCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ALobbyAvatarCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float Speed = GetVelocity().Size();
	const bool bInAir = GetCharacterMovement() && GetCharacterMovement()->IsFalling();
	const bool  bBaseIdle = (Speed < IdleSpeedThreshold) && !bInAir;
	const bool bEffectiveIdle = bBaseIdle && !bBoredPlaying;

	if (bEffectiveIdle != bWasEffectiveIdle)
	{
		bWasEffectiveIdle = bEffectiveIdle;
		if (bEffectiveIdle) StartIdleWaitTimer(3.f);  
		else                StopIdleWaitTimer();     
	}
}

void ALobbyAvatarCharacter::StartBoredTimer()
{

	GetWorldTimerManager().SetTimer(
		BoredTimer, this, &ALobbyAvatarCharacter::PlayBoredOnce,
		3.0f, true, 3.0f);
}

void ALobbyAvatarCharacter::StopBoredTimer()
{
	GetWorldTimerManager().ClearTimer(BoredTimer);
}

void ALobbyAvatarCharacter::PlayBoredOnce()
{
	
	if (bBoredPlaying) return;

	UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!Anim || !BoredAnim) return;

	
	bBoredPlaying = true;
	StopIdleWaitTimer();

	

	if (UAnimMontage* Dyn = Anim->PlaySlotAnimationAsDynamicMontage(
			BoredAnim,     
			SlotName,     
			BlendIn,       
			BlendOut,     
			1.0f,          
			1,            
			0.3f,          
			0.0f))       
	{
		//Idle이면 다시 3초 타이머 시작
		FOnMontageEnded EndDel;
		EndDel.BindUObject(this, &ALobbyAvatarCharacter::OnBoredMontageEnded);
		Anim->Montage_SetEndDelegate(EndDel, Dyn);
	}
	else
	{
		// 실패 시 즉시 원상복귀
		bBoredPlaying = false;
		StartIdleWaitTimer(3.f);
	}
}

void ALobbyAvatarCharacter::StartIdleWaitTimer(float Seconds)
{
	GetWorldTimerManager().SetTimer(
	  IdleWaitTimer, this, &ALobbyAvatarCharacter::PlayBoredOnce,
	  Seconds,false);
}

void ALobbyAvatarCharacter::StopIdleWaitTimer()
{
	GetWorldTimerManager().ClearTimer(IdleWaitTimer);
}

void ALobbyAvatarCharacter::OnBoredMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bBoredPlaying = false;
	
	const float Speed  = GetVelocity().Size();
	const bool  bInAir = GetCharacterMovement() && GetCharacterMovement()->IsFalling();
	const bool  bBaseIdle = (Speed < IdleSpeedThreshold) && !bInAir;

	if (bBaseIdle)
		StartIdleWaitTimer(3.f);
}
