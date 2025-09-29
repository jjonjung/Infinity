// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/Projectile.h"
#include "IronProjectile.generated.h"


/**
 * 
 */
UCLASS()
class INFINITYFIGHTER_API AIronProjectile : public AProjectile
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void ProjectileMove_Implementation() override;
	virtual void OnProjectileHit_Implementation(const FHitResult& Hit) override;
};
