// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterActionStatComponent.h"

#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h" // CharacterMovementComponent를 위해 추가


#pragma  region Getter/Setter
int32 UCharacterActionStatComponent::GetCurrentBullet() const
{
	return CurrentBullet;
}

void UCharacterActionStatComponent::SetCurrentBullet(int32 _CurrentBullet)
{
	this->CurrentBullet = _CurrentBullet;
}

int32 UCharacterActionStatComponent::Get_CurrentHp() const
{
	return _CurrentHp;
}

void UCharacterActionStatComponent::Set_CurrentHp(int32 CurrentHp)
{
	_CurrentHp = CurrentHp;
}

float UCharacterActionStatComponent::GetAttackDamage() const
{
	return AttackDamage;
}

void UCharacterActionStatComponent::SetAttackDamage(float _AttackDamage)
{
	this->AttackDamage = _AttackDamage;
}

float UCharacterActionStatComponent::GetAttackSpeed() const
{
	return AttackSpeed;
}

void UCharacterActionStatComponent::SetAttackSpeed(float _AttackSpeed)
{
	this->AttackSpeed = _AttackSpeed;
}

int32 UCharacterActionStatComponent::GetMaxBullet() const
{
	return MaxBullet;
}

void UCharacterActionStatComponent::SetMaxBullet(int32 _MaxBullet)
{
	this->MaxBullet = _MaxBullet;
}

int32 UCharacterActionStatComponent::GetBulletBox() const
{
	return BulletBox;
}

void UCharacterActionStatComponent::SetBulletBox(int32 _BulletBox)
{
	this->BulletBox = _BulletBox;
}
#pragma  endregion 

// Sets default values for this component's properties
UCharacterActionStatComponent::UCharacterActionStatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCharacterActionStatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCharacterActionStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCharacterActionStatComponent::SetCurrentBulletBox(int32 bulletBox)
{
	CurrentBulletBox = bulletBox;
}

int32 UCharacterActionStatComponent::GetCurrentBulletBox()
{
	return CurrentBulletBox;
}

float UCharacterActionStatComponent::CalculateHpPercentage()
{
	float currentPercentage = _CurrentHp/_MaxHP;
	float CutedPercentage = FMath::Clamp(currentPercentage, 0.0f, 1.0f);
	

	return 1-CutedPercentage;
}


//데이터 에셋을 받으면 캐릭터 베이스에 한번 초기화 하기 위한 함수 
void UCharacterActionStatComponent::InitFromDa(const UActionStat* BasicStat)
{
	AttackDamage = BasicStat->AttackDamage;
	AttackSpeed= BasicStat->AttackSpeed;// 한발에 총 몇발? 
	MaxBullet= BasicStat->MaxBullet;// 최대로 소유할 수 있는 탄알 수 
	BulletBox = BasicStat->BulletBox;//한 탄창에 쏠 수 있는 탄알 수
	_MaxHP = BasicStat->MaxHP;
	

	SetCurrentBulletBox(BulletBox);
	SetCurrentBullet(MaxBullet);
	Set_CurrentHp(_MaxHP);
}

void UCharacterActionStatComponent::SetInWall(bool InWall)
{
	IsInWall = InWall;
}

bool UCharacterActionStatComponent::GetInWall() const

{
	return IsInWall;
}

bool UCharacterActionStatComponent::GetInFly()
{
	return IsFly;
}

void UCharacterActionStatComponent::SetInFly(bool InFly)
{
	IsFly = InFly;
}

//피격시 데미지 계산
void UCharacterActionStatComponent::ApplyDamage(float Amount, AActor* Causer)
{
	const int32 OldHP = _CurrentHp;
	const int32 Delta = FMath::Max(0,FMath::RoundToInt(Amount));
	_CurrentHp = FMath::Clamp(OldHP - Delta, 0, _MaxHP);
	
	
	if (_CurrentHp <= 0)
	{
		OnDeath.Broadcast(GetOwner(), Causer);

		if (AActor* OwnerActor = GetOwner())
		{
			if (ACharacter* CharacterOwner = Cast<ACharacter>(OwnerActor))
			{
				if (UCharacterMovementComponent* MovementComp = CharacterOwner->GetCharacterMovement())
				{
					// 기존 이동 속도 0.5배
					MovementComp->MaxWalkSpeed *= 0.5f;
					
				}
			}
		}
	}
}

