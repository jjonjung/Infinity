// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/Projectile.h"
#include "DoctorStrangeProjectile.generated.h"

/**
 * 
 */
UCLASS()
class INFINITYFIGHTER_API ADoctorStrangeProjectile : public AProjectile
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	void FirstMove();
	virtual void ProjectileMove_Implementation() override;
	virtual void OnProjectileHit_Implementation(const FHitResult& Hit) override;
	
	bool bIsReturn = false;
	FTimerHandle ProjectileDirHandle;
	FVector ToPlayer;

	FVector GetNextDirection();
	void SecondMoveBackToOwner();
	void ReadyToBack();
	UPROPERTY(EditDefaultsOnly)
	USoundBase* firstSound;
	UPROPERTY(EditDefaultsOnly)
	USoundBase* secondSound;
};
