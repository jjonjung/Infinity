// Fill out your copyright notice in the Description page of Project Settings.


#include "DoctorStrangeProjectile.h"

#include "Base/CharacterBase.h"
#include "Kismet/GameplayStatics.h"

void ADoctorStrangeProjectile::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetTimerManager().SetTimer(
	ProjectileDirHandle,
	this,
	&ADoctorStrangeProjectile::ReadyToBack,
	2.f,      
	false     
	);
	if (firstSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(),firstSound);
	}
	
}

void ADoctorStrangeProjectile::Tick(float DeltaTime)
{
	ProjectileMove_Implementation();
}

void ADoctorStrangeProjectile::FirstMove()
{
	auto p0 = GetActorLocation();
	float  vt = InitialSpeed* GetWorld()->DeltaTimeSeconds;
	FVector Dir = GetActorForwardVector();
	FVector p = p0+vt*Dir;

	FHitResult Hit;
	const bool bMoved = SetActorLocation(p, /*bSweep*/true, &Hit);
	if (!bMoved && Hit.bBlockingHit)
	{
		OnProjectileHit(Hit);
	}
}

void ADoctorStrangeProjectile::ProjectileMove_Implementation()
{
	if (!bIsReturn)
	{
		FirstMove();
		
	}
	SecondMoveBackToOwner();
	
	
}

void ADoctorStrangeProjectile::OnProjectileHit_Implementation(const FHitResult& Hit)
{
	Super::OnProjectileHit_Implementation(Hit);
}

FVector ADoctorStrangeProjectile::GetNextDirection()
{
	if (ProjectileOwner)
	{
		auto TargetLocation = ProjectileOwner-> GetActorLocation();
		auto newDir = TargetLocation - GetActorLocation();
		newDir.Normalize();
		return newDir;
	}
	else
	{
		return -GetActorForwardVector();
		
	}
	
}

void ADoctorStrangeProjectile::SecondMoveBackToOwner()
{
	auto p0 = GetActorLocation();
	float vt = MaxSpeed* GetWorld()->DeltaTimeSeconds;
	FVector p = p0+vt*ToPlayer;

	FHitResult Hit;
	const bool bMoved = SetActorLocation(p, /*bSweep*/true, &Hit);
	if (!bMoved && Hit.bBlockingHit)
	{
		OnProjectileHit(Hit);
	}
}

void ADoctorStrangeProjectile::ReadyToBack()
{
	
	if (ProjectileOwner)
	{
		auto TargetLocation = ProjectileOwner-> GetActorLocation();
		auto newDir = TargetLocation - GetActorLocation();
		newDir.Normalize();
		ToPlayer = newDir;
		bIsReturn = true;
		if (secondSound)
		{
			UGameplayStatics::PlaySound2D(GetWorld(),secondSound);
		}
		
	}
}




