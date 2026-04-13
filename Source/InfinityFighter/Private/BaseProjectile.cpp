// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseProjectile.h"

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ProjectileMove_Implementation();
}

void ABaseProjectile::ProjectileMove_Implementation()
{
	FVector CurrentPosition = GetActorLocation();
	const FVector Direction = GetActorForwardVector();
	const float  VT         = GetWorld()->DeltaTimeSeconds;

	CurrentPosition += Direction * 3000 * VT; 

	FHitResult Hit;
	
	const bool bMoved = SetActorLocation(CurrentPosition, true, &Hit);

	if (!bMoved && Hit.bBlockingHit)
	{
		OnProjectileHit(Hit);
		return;
	}
}

void ABaseProjectile::OnProjectileHit_Implementation(const FHitResult& Hit)
{
	// Defer to shared parent logic
	Super::OnProjectileHit_Implementation(Hit);
}

